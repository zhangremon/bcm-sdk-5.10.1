/*
 * $Id: tpacket.c 1.19.310.1 Broadcom SDK $
 * $Copyright: Copyright 2011 Broadcom Corporation.
 * This program is the proprietary software of Broadcom Corporation
 * and/or its licensors, and may only be used, duplicated, modified
 * or distributed pursuant to the terms and conditions of a separate,
 * written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized
 * License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software
 * and all intellectual property rights therein.  IF YOU HAVE
 * NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE
 * IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
 * ALL USE OF THE SOFTWARE.  
 *  
 * Except as expressly set forth in the Authorized License,
 *  
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of
 * Broadcom integrated circuit products.
 *  
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS
 * PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY,
 * OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 * 
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL,
 * INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER
 * ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE OF OR INABILITY
 * TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF
 * THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR USD 1.00,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING
 * ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.$
 *
 * Packet tests that use the BCM api.
 */

#include <sal/types.h>
#include <sal/core/time.h>
#include <sal/appl/sal.h>

#include <soc/util.h>

#include <appl/diag/system.h>
#include <appl/diag/parse.h>

#include <bcm/error.h>
#include <bcm/l2.h>
#include <bcm/mcast.h>
#include <bcm/pkt.h>
#include <bcm/tx.h>

#include "testlist.h"

typedef struct p_s {
    int		p_init;			/* TRUE --> initialized */
    int		p_opt;			/* Destination option */
#   define	P_O_UC_HIT	0	/* Unicast in ARL */
#   define	P_O_UC_MISS	1	/* Unicast not in ARL */
#   define	P_O_MC_HIT	2	/* L2 multicast in MARL/L2 */
#   define	P_O_MC_MISS	3	/* L2 multicast not in MARL/L2 */
#   define	P_O_BC		4	/* L2 Broadcast */
#   define	P_O_PORT	5	/* Send to specific ports */
    int		p_fp;			/* TRUE --> Use fastpath (new only) */
    int		p_tag;			/* TRUE --> tag packet */
    int		p_vid;			/* VLAN id if tagged */
    sal_mac_addr_t p_dst;		/* Dest mac address */
    int		p_array;		/* Array? Bool; false -> list */
    int		ppc;			/* # packets per chain */
    int		chains;		/* # chains */
    int		p_cnt_start;		/* # started */
    volatile int chains_done;		/* # complete */
    int		p_cnt_accum;		/* Accumulated q length all samples */
    pbmp_t	p_pbm;			/* Ports to xmit on */
    pbmp_t	p_upbm;			/* Untabg ports (only if fast) */
    int		len_start;		/* Length start */
    int		len_end;		/* Length end */
    int		p_l_inc;		/* Length increment */
    int		p_ops;			/* # outstanding operations */
    sal_sem_t	p_sem_active;		/* Synchronize # active */
    sal_sem_t	p_sem_done;		/* Completion wait */
    int		p_ops_total;		/* Accumulated operations outstanding */
    enet_hdr_t	*p_tx;			/* Tx - Packet buffer */
    bcm_pkt_t   *p_pkt;                 /* The packet being tx'd */
    bcm_pkt_t   **p_pkt_array;          /* Array of pkt pointers */
} p_t;

static sal_mac_addr_t tp_mac_uc = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
static sal_mac_addr_t tp_mac_mc = {0x01, 0x11, 0x22, 0x33, 0x44, 0x44};
static sal_mac_addr_t tp_mac_bc = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

static p_t	*p_control[SOC_MAX_NUM_DEVICES];

#define	PACKET_MAX_BUFFER	(8 * 1024)	/* 8K max packet */

STATIC void
xmit_chain_done(int unit, bcm_pkt_t *pkt, void *cookie)
{
    p_t		*p = (p_t *)cookie;

    sal_sem_give(p->p_sem_active);	/* Allow next */
    if (++p->chains_done == p->chains) {
	sal_sem_give(p->p_sem_done);
    }
}

/* Note:  i != 0, at least not for linked lists */
#define PKT_PTR_NEXT(p, i, cur) (((p)->ppc > 1) ? \
    ((p)->p_array ? (p)->p_pkt_array[i] : (cur)->next) : NULL)

#define FOREACH_P_PKT(p, i, pkt) \
    for ((i) = 0, (pkt) = (p)->p_pkt; (i) < (p)->ppc; \
        (i)++, (pkt) = PKT_PTR_NEXT(p, i, pkt))

STATIC int
tpacket_setup(int unit, p_t *p)
{
    enet_hdr_t	*enet_hdr;
    uint8 	*fill_addr;
    bcm_pkt_t   *pkt;
    int i;

    p->p_sem_active = sal_sem_create("tpacket_ops", sal_sem_COUNTING,
                                     p->p_ops);
    p->p_sem_done   = sal_sem_create("tpacket_done", sal_sem_BINARY, 0);

    if (p->p_sem_active == NULL || p->p_sem_done == NULL) {
	test_error(unit, "Failed to allocate Tx Buffer\n");
	return(-1);
    }

    if (p->ppc > 1) { /* Multiple packets per chain */
        if (p->p_array) {
             bcm_pkt_blk_alloc(unit, p->ppc,
                               PACKET_MAX_BUFFER, BCM_TX_CRC_REGEN, 
                               &(p->p_pkt_array));
            if (!p->p_pkt_array) {
                test_error(unit, "Failed to allocate pkt array\n");
                return -1;
            }
            p->p_pkt = p->p_pkt_array[0];
        } else { /* Linked list */
            for (i = 0; i < p->ppc; i++) {
                bcm_pkt_alloc(unit, PACKET_MAX_BUFFER,
                              BCM_TX_CRC_REGEN, &pkt);
                if (!pkt) {
                    test_error(unit, "Failed to allocate pkt list\n");
                    return -1;
                }
                pkt->next = p->p_pkt;
                p->p_pkt = pkt;
            }
        }
    } else {
        bcm_pkt_alloc(unit, PACKET_MAX_BUFFER, BCM_TX_CRC_REGEN, &(p->p_pkt));
        if (!p->p_pkt) {
            test_error(unit, "Failed to allocate Tx Buffer\n");
            return(-1);
        }
        p->p_pkt->call_back = xmit_chain_done;
    }

    p->p_tx = (enet_hdr_t *)p->p_pkt->pkt_data[0].data;

    FOREACH_P_PKT(p, i, pkt) {
        if (p->p_fp) {
            pkt->flags |= BCM_TX_FAST_PATH;
        } else if (SOC_IS_XGS3_SWITCH(unit)) {
            pkt->flags |= BCM_TX_ETHER;
        }

        enet_hdr = (enet_hdr_t *)BCM_PKT_DMAC(pkt);

        /* Fill in buffer according to request */
        ENET_SET_MACADDR(enet_hdr->en_dhost, p->p_dst);
        if (p->p_tag) {
            enet_hdr->en_tag_tpid = soc_htons(ENET_DEFAULT_TPID);
            enet_hdr->en_tag_ctrl = soc_htons(VLAN_CTRL(0, 0, p->p_vid));
        }
        fill_addr = (uint8 *)enet_hdr + 2*sizeof(bcm_mac_t) + sizeof(uint32);
        sal_memset(fill_addr, 0xff, p->len_end -
                   (fill_addr - (uint8 *)enet_hdr));
    }

    return(0);
}


/*
 * Function: 	tpacket_test
 * Purpose:	Test basic PCI stuff on StrataSwitch.
 * Parameters:	unit - unit #.
 *		a - pointer to arguments.
 *		pa - ignored cookie.
 * Returns:	0
 */

int
tpacket_tx_test(int unit, args_t *a, void *pa)
{
    p_t		*p = (p_t *)pa;
    int		i, len, rv;
    sal_usecs_t  time_start, time_end;
    enet_hdr_t  *enet_hdr;
    bcm_pkt_t   *pkt;
    uint32 elapsed_us;
    uint32 bps; /* bytes/sec */
    int ql;
    uint32 pps;  /* Packets per second */
    uint32 tot_pkts;

    COMPILER_REFERENCE(a);

    tot_pkts = p->chains * p->ppc;

    printk("\n"
	   "  Packet | Time   |     Rate       | Average \n"
           "   Size  | (Sec)  |  p/s  |  MB/s  |  Queue  \n"
	   " --------+--------+-------+--------+--------\n");

    for (len = p->len_start; len <= p->len_end; len += p->p_l_inc) {
        FOREACH_P_PKT(p, i, pkt) {
            pkt->pkt_data[0].len = len;
            enet_hdr = (enet_hdr_t *)BCM_PKT_DMAC(pkt);
            if (p->p_tag) {
                enet_hdr->en_tag_len = soc_htons(len);
            } else {
                enet_hdr->en_untagged_len = soc_htons(len);
            }
        }
	p->p_cnt_accum = 0;
	p->chains_done  = 0;

	/* *** Start Timer *** */

	time_start = sal_time_usecs();

	/* *** GO *** */

	for (i = 0; i < p->chains; i++) {
	    if (sal_sem_take(p->p_sem_active, 10 * 1000000) < 0) {
		test_error(unit, "Transmission appears stalled\n");
		return(-1);
	    }
	    p->p_cnt_accum += i - p->chains_done;
            switch(p->p_opt) {
            case P_O_PORT:
		if (p->ppc == 1) { /* Packets per chain */
		    rv = bcm_tx(unit, p->p_pkt, p);
		} else if (p->p_array) {
		    rv = bcm_tx_array(unit, p->p_pkt_array, p->ppc,
				      xmit_chain_done, p);
		} else { /* Linked list, multiple packets per chain */
		    rv = bcm_tx_list(unit, p->p_pkt, xmit_chain_done, p);
		}
		if (rv < 0) {
		    test_error(unit, "packet send failed: %s\n",
			       bcm_errmsg(rv));
		    return(-1);
		}
                break;
            default:
                /* This is already caught elsewhere */
                test_error(unit, "Only PortBitMap option supported "
                           "for bcm_tx\n");
                return -1;
                break;
            }
	}
	if (0 > sal_sem_take(p->p_sem_done, 10 * 1000000)) {
	    test_error(unit, "Timeout waiting for completion\n");
	    return(-1);
	}
	time_end = sal_time_usecs();

	if (time_end == time_start) {
	    printk("   %5d  | ****** | ***** | ******  | ***\n", len);
	} else {
            elapsed_us = SAL_USECS_SUB(time_end, time_start);
            /* Round off error limitting */
            if (tot_pkts > 2000) {
                pps = (tot_pkts * 10000 / elapsed_us) * 100;
            } else {
                pps = tot_pkts * 1000000 / elapsed_us;
            }

            bps = pps * len;
	    ql = (p->p_cnt_accum * 1000) / p->chains;

	    printk("  %5d  | %3d.%02d | %5d | %2d.%03d | %2d.%03d\n",
		   len,
		   elapsed_us / 1000000,
		   (elapsed_us % 1000000) / 10000,
		   pps,
		   bps / 1000000,
		   (bps % 1000000) / 1000,
		   ql / 1000,
		   ql % 1000);
	}
    }
    return(0);
}

/*ARGSUSED*/
int
tpacket_tx_done(int unit, void *pa)
/*
 * Function: 	pci_test_done
 * Purpose:	Restore all values to CMIC from soc structure.
 * Parameters:	u - unit #
 *		pa - cookie (Ignored)
 * Returns:	0 - OK
 *		-1 - failed
 */
{
    p_t		*p = p_control[unit];
#ifdef BCM_ESW_SUPPORT
    int		rv;
#endif /* BCM_ESW_SUPPORT */
    bcm_pkt_t   *pkt;
    bcm_pkt_t   *next_pkt;

    if (p == NULL) {
	return 0;
    }

#ifdef BCM_ESW_SUPPORT
    if (0 > (rv = soc_dma_abort(unit))) {
        printk("Warning: soc_dma_abort(%d) failed: %s\n", unit, soc_errmsg(rv));
    }
#endif /* BCM_ESW_SUPPORT */

    if (p->chains > 1) {
        if (p->p_array) {
            bcm_pkt_blk_free(unit, p->p_pkt_array, p->ppc);
        } else {
            pkt = p->p_pkt;
            while (pkt) {
                next_pkt = pkt->next;
                bcm_pkt_free(unit, pkt);
                pkt = next_pkt;
            }
        }
    } else {
        if (p->p_pkt != NULL) {
            bcm_pkt_free(unit, p->p_pkt);
        }
    }

    p->p_pkt_array = NULL;
    p->p_pkt = NULL;
    p->p_tx = NULL;

    if (p->p_sem_active != NULL) {
	sal_sem_destroy(p->p_sem_active);
	p->p_sem_active = NULL;
    }
    if (p->p_sem_done != NULL) {
	sal_sem_destroy(p->p_sem_done);
	p->p_sem_done = NULL;
    }

    /*
     * Don't free the p_control entry,
     * keep it around to save argument state
     */

    return 0;
}

int
tpacket_tx_init(int unit, args_t *a, void **pa)
/*
 * Function: 	packet_test_init
 * Purpose:	Save all the current PCI Config registers to write
 *		on completion.
 * Parameters:	u - unit #
 *		a - pointer to args
 *		pa - Pointer to cookie
 * Returns:	0 - success, -1 - failed.
 */
{
    static char 	*opt_list[] = {
	"UCHit", "UCMiss", "MCHit", "MCMiss", "Bcast", "PortBitMap"
    };
    p_t			*p;
    parse_table_t	pt;
    bcm_mcast_addr_t	mc;
    bcm_l2_addr_t	l2;
    int			rv;

    p = p_control[unit];
    if (p == NULL) {
	p = sal_alloc(sizeof(p_t), "tpacket");
	if (p == NULL) {
	    test_error(unit, "ERROR: cannot allocate memory\n");
	    return -1;
	}
	sal_memset(p, 0, sizeof(p_t));
	p_control[unit] = p;
    }

    if (!p->p_init) {			/* Init defaults first time */
	p->len_start = 64;
	p->len_end = 64;
	p->p_l_inc = 64;
	p->p_init  = TRUE;
	p->chains  = 1000;
	p->ppc     = 1;
	p->p_vid   = 1;
	p->p_ops   = 1;
	p->p_tag   = 1;
	p->p_opt   = P_O_PORT;
	p->p_array = 1;
        SOC_PBMP_CLEAR(p->p_pbm);
        SOC_PBMP_CLEAR(p->p_upbm);
        SOC_PBMP_OR(p->p_pbm, PBMP_PORT_ALL(unit));
    }

    parse_table_init(unit, &pt);
    parse_table_add(&pt, "FastPath", PQ_DFL|PQ_BOOL, 0, &p->p_fp, 0);
    parse_table_add(&pt, "DestMac", PQ_DFL|PQ_MULTI, 0, &p->p_opt, opt_list);
    parse_table_add(&pt, "Tagged", PQ_DFL|PQ_BOOL, 0, &p->p_tag, 0);
    parse_table_add(&pt, "VlanId", PQ_DFL|PQ_INT, 0, &p->p_vid, 0);
    parse_table_add(&pt, "LengthStart", PQ_DFL|PQ_INT, 0, &p->len_start, 0);
    parse_table_add(&pt, "LengthEnd", PQ_DFL|PQ_INT, 0, &p->len_end, 0);
    parse_table_add(&pt, "LengthInc", PQ_DFL|PQ_INT, 0, &p->p_l_inc, 0);
    parse_table_add(&pt, "PortBitMap", PQ_DFL|PQ_PBMP, 0, &p->p_pbm, 0);
    parse_table_add(&pt, "UntagPortBitMap", PQ_DFL|PQ_PBMP, 0, &p->p_upbm, 0);
    parse_table_add(&pt, "PktsPerChain", PQ_DFL|PQ_INT, 0, &p->ppc, 0);
    parse_table_add(&pt, "Chains", PQ_DFL|PQ_INT, 0, &p->chains, 0);
    parse_table_add(&pt, "Array", PQ_DFL|PQ_BOOL, 0, &p->p_array, 0);
    parse_table_add(&pt, "Operations", PQ_DFL|PQ_INT, 0, &p->p_ops, 0);

    if (parse_arg_eq(a, &pt) < 0 || ARG_CNT(a) != 0) {
	test_error(unit, "%s: Invalid option: %s\n",
		   ARG_CMD(a), ARG_CUR(a) ? ARG_CUR(a) : "*");
	parse_arg_eq_done(&pt);
	return(-1);
    }
    parse_arg_eq_done(&pt);

    if (p->p_ops > 1000) {
	test_error(unit, "Operations must be 1 <= # <= 1000\n");
	return(-1);
    }

    if (p->p_opt != P_O_PORT) {
        test_error(unit, "Only PortBitMap option supported for bcm_tx\n");
        return -1;
    }

    if (p->len_end > PACKET_MAX_BUFFER) {
        test_error(unit, "Max supported pkt size is %d\n", PACKET_MAX_BUFFER);
        p->len_end = PACKET_MAX_BUFFER;
        return -1;
    }

    if (tpacket_setup(unit, p) < 0) {
	(void)tpacket_tx_done(unit, p);
	return(-1);
    }

    /* Insert ARL/MARL entry */

    switch(p->p_opt) {
    case P_O_UC_HIT:
	bcm_l2_addr_t_init(&l2, tp_mac_uc, p->p_vid);
	l2.flags |= BCM_L2_STATIC;
	if (0 > (rv = bcm_l2_addr_add(unit, &l2))) {
	    test_error(unit, "bcm_l2_addr_add failed: %s\n", bcm_errmsg(rv));
	    return(-1);
	}
	ENET_SET_MACADDR(p->p_dst, tp_mac_uc);
	break;
    case P_O_UC_MISS:
	if (0 > (rv = bcm_l2_addr_delete(unit, tp_mac_uc, p->p_vid))) {
	    test_error(unit, "bcm_l2_addr_delete failed: %s\n", bcm_errmsg(rv));
	    return(-1);
	}
	ENET_SET_MACADDR(p->p_dst, tp_mac_uc);
	break;
    case P_O_MC_HIT:
	bcm_mcast_addr_t_init(&mc, tp_mac_mc, p->p_vid);
	if (0 > (rv = bcm_mcast_addr_add(unit, &mc))) {
	    test_error(unit, "bcm_mcast_addr_add failed: %s\n", bcm_errmsg(rv));
	    return(-1);
	}
	ENET_SET_MACADDR(p->p_dst, tp_mac_mc);
	break;
    case P_O_MC_MISS:
	if (0 > (rv = bcm_mcast_addr_remove(unit, tp_mac_mc, p->p_vid))) {
	    if (rv != BCM_E_NOT_FOUND) {
		test_error(unit, "bcm_mcast_addr_remove failed: %s\n", bcm_errmsg(rv));
		return(-1);
	    }
	}
	ENET_SET_MACADDR(p->p_dst, tp_mac_mc);
	break;
    case P_O_BC:
	ENET_SET_MACADDR(p->p_dst, tp_mac_bc);
	break;
    case P_O_PORT:
	ENET_SET_MACADDR(p->p_dst, tp_mac_bc);
	break;
    default:
	return(-1);
    }

    *pa = (void *)p;
    return(0);
}
