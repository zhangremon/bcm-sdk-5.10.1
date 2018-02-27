/*
 * $Id: ethdma.c 1.3.2.16 Broadcom SDK $
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
 * File:    dma.c
 * Purpose:     SOC DMA LLC (Link Layer) driver; used for sending
 *              and receiving packets over MII (and later, the uplink).
 *      Interface via ethernet et module of bcm47xx.
 */
#include <shared/et/osl.h>
#include <shared/et/proto/ethernet.h>
#include <shared/et/bcmenetmib.h>
#include <soc/etc.h>

#include <sal/core/boot.h>
#include <sal/core/libc.h>
#include <shared/alloc.h>

#include <soc/mcm/robo/driver.h>
#include <soc/debug.h>
#include <soc/dma.h>

#include <soc/cm.h>

#if defined(KEYSTONE)
#include <soc/etcgmac.h>
#endif

#if defined(KEYSTONE) || defined(ROBO_4704)

#if defined(KEYSTONE)
static int _soc_eth_dma_oam = -1;
static int _soc_eth_dma_drop_en = -1;

void soc_eth_dma_classify_setup(int unit, int type, int chan);
void soc_eth_dma_default_drop_enable(int unit, int drop_enable);
#endif

#define DV_MAGIC_NUMBER 0xba5eba11

#define DP_FMT  "%sdata[%04x]: "    /* dump line start format */
#define DP_BPL  16          /* dumped bytes per line */

static int eth_dma_occupied[NUMRXQ] = {-1,-1,-1,-1};

void
soc_eth_dma_ether_dump(int unit, char *pfx, uint8 *addr, int len, int offset)
{
    int     i = 0, j;

    if (len >= DP_BPL && (DP_BPL & 1) == 0) {
        char    linebuf[128], *s;
        /* Show first line with MAC addresses in curly braces */
        s = linebuf;
        sal_sprintf(s, DP_FMT "{", pfx, i);
        while (*s != 0) s++;
        for (i = offset; i < offset + 6; i++) {
            sal_sprintf(s, "%02x", addr[i]);
            while (*s != 0) s++;
        }
        sal_sprintf(s, "} {");
        while (*s != 0) s++;
        for (; i < offset + 12; i++) {
            sal_sprintf(s, "%02x", addr[i]);
            while (*s != 0) s++;
        }
        sal_sprintf(s, "}");
        while (*s != 0) s++;
        for (; i < offset + DP_BPL; i += 2) {
            sal_sprintf(s, " %02x%02x", addr[i], addr[i + 1]);
            while (*s != 0) s++;
        }
        soc_cm_print("%s\n", linebuf);
        }

        for (; i < len; i += DP_BPL) {
        char    linebuf[128], *s;
        s = linebuf;
        sal_sprintf(s, DP_FMT, pfx, i);
        while (*s != 0) s++;
        for (j = i; j < i + DP_BPL && j < len; j++) {
            sal_sprintf(s, "%02x%s", addr[j], j & 1 ? " " : "");
            while (*s != 0) s++;
        }
        soc_cm_print("%s\n", linebuf);
    }

}


/*
 * Function:
 *  soc_eth_dma_dump_pkt
 * Purpose:
 *  Dump packet data in human readable form
 * Parameters:
 *  pfx - prefix for all output strings
 *  addr - pointer to data bytes of packet
 *  len - length of data to dump
 * Returns:
 *  Nothing
 */

void
soc_eth_dma_dump_pkt(int unit, char *pfx, uint8 *addr, int len)
{
    int ether_offset;

    COMPILER_REFERENCE(unit);

    if (len == 0 || !addr) {
        soc_cm_print(DP_FMT "<NONE>\n", pfx, 0);
        return;
    }

    ether_offset = 0;

    soc_eth_dma_ether_dump(unit, pfx, addr, len, ether_offset);
}

#undef DP_FMT
#undef DP_BPL

/*
 * Function:
 *      soc_eth_dma_dv_valid
 * Purpose:
 *      Check if a DV is (probably) valid
 * Parameters:
 *      dv - The dv to examine
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Does not guarantee a DV is valid, but will detect
 *      most _invalid_ DVs.
 */

int
soc_eth_dma_dv_valid(eth_dv_t *dv)
{
    if (dv->dv_magic != DV_MAGIC_NUMBER) {
        return FALSE;
    }

    return TRUE;
}


/*
 * Function:
 *  soc_eth_dma_dump_dv
 * Purpose:
 *  Dump a "dv" structure and all the DCB fields.
 * Parameters:
 *  dv_chain - pointer to dv list to dump.
 * Returns:
 *  Nothing.
 */

void
soc_eth_dma_dump_dv(int unit, char *pfx, eth_dv_t *dv_chain)
{
    char    *op_name;
    int     i;
    eth_dv_t *cur_dv = NULL;

    if (!soc_eth_dma_dv_valid(dv_chain)) {
        soc_cm_print("%sdv@%p appears invalid\n", pfx, (void *)dv_chain);
        return;
    }

    switch(dv_chain->dv_op) {
    case DV_NONE:   op_name = "None";   break;
    case DV_TX:     op_name = "TX";         break;
    case DV_RX:     op_name = "RX";         break;
    default:        op_name = "*ERR*";  break;
    }

    cur_dv = dv_chain;
    while (cur_dv) {
        soc_cm_print("%sdv@%p op=%s vcnt=%d cnt=%d\n",
             pfx, (void *)cur_dv, op_name, cur_dv->dv_vcnt,
             cur_dv->dv_length);
        soc_cm_print("%s    user1 %p. user2 %p. user3 %p. user4 %p\n",
             pfx, cur_dv->dv_public1.ptr, cur_dv->dv_public2.ptr,
                     cur_dv->dv_public3.ptr, cur_dv->dv_public4.ptr);
    
        for (i = 0; i < cur_dv->dv_vcnt; i++) {
            eth_dcb_t    *dcb;
            sal_vaddr_t   addr;

            dcb = (eth_dcb_t *)(&cur_dv->dv_dcb[i]);
            addr = (sal_vaddr_t)(dcb->dcb_vaddr);
            soc_cm_print("%sdcb[%d] @%p: addr=%p, len=%d\n",
                     pfx, i, (void *)dcb, (void *)addr, dcb->len);
            if (soc_cm_debug_check(DK_PACKET)) {
                if (cur_dv->dv_op == DV_TX) {
                    soc_eth_dma_dump_pkt(unit, pfx, (uint8 *) addr, dcb->len);
                } else if (cur_dv->dv_op == DV_RX) {
                    soc_eth_dma_dump_pkt(unit, pfx, (uint8 *) addr, dcb->len);
                }
            }
        }

        cur_dv = cur_dv->dv_next;
    }

}

/*
 * Function:
 *  soc_eth_dma_start
 * Purpose:
 *  Launch a SOC_DMA DMA operation.
 * Parameters:
 *  unit - unit number.
 *  dv_chain - dma request description.
 * Returns:
 *  SOC_E_NONE - operation started.
 *  SOC_E_TIMEOUT - operation failed to be queued.
 */

int
soc_eth_dma_start(int unit, eth_dv_t *dv_chain)
{
    /* If DK_DMA set, dump out info on request, before queued on channel */

    if (soc_cm_debug_check(DK_DMA) && (dv_chain->dv_op == DV_TX)) {
        soc_eth_dma_dump_dv(unit, "dma (before): ", dv_chain);
    }

    switch (dv_chain->dv_op) {
    case DV_TX:
        et_soc_start(unit, dv_chain);
        break;
    case DV_RX:
        et_soc_rx_chain(unit, dv_chain);
        break;
    default:
        soc_cm_print("ERROR: unit %d unknown dma op %d\n", 
            unit, dv_chain->dv_op);
        assert(0);
        return (SOC_E_PARAM);
    }

    return (SOC_E_NONE);
}


int 
soc_eth_dma_rxenable(int unit)
{
    et_soc_rxmon_on(unit);

#if defined(KEYSTONE)
    /* Create a default rule. Do not drop traffics on default dma */
    soc_eth_dma_default_drop_enable(unit, 0);
#endif
    return SOC_E_NONE;
}


int 
soc_eth_dma_rxstop(int unit)
{
    et_soc_rxmon_off(unit);

#if defined(KEYSTONE)
    /* Create a default rule. Drop traffics on default dma */
    soc_eth_dma_default_drop_enable(unit, 1);
#endif
    return SOC_E_NONE;
}




/*
 * Function:
 *  soc_eth_dma_dv_alloc
 * Purpose:
 *  Allocate and initialize a dv struct.
 * Parameters:
 *  op - operations iov requested for.
 *  cnt - number of DCBs required.
 * Notes:
 *  If a DV on the free list will accomodate the request,
 *  satisfy it from there to avoid extra alloc/free calls.
 */

eth_dv_t *
soc_eth_dma_dv_alloc(int unit, dvt_t op, int cnt)
{
    soc_control_t   *soc = SOC_CONTROL(unit);
    eth_dv_t   *dv;

    assert(cnt > 0);

    /* Check if we can use one off the free list */

    soc->stat.dv_alloc++;

    dv = sal_alloc(sizeof(eth_dv_t), "soc_dv_alloc");
    if (dv == NULL) {
        soc_cm_print("ERROR: unit %d dv alloc failed.\n", unit);
        return(dv);
    }

    dv->dv_dcb = sal_alloc(sizeof(eth_dcb_t) * cnt, "soc_dcb_alloc");
    if (dv->dv_dcb == NULL) {
        soc_cm_print("ERROR: unit %d dcb alloc failed.\n", unit);
        sal_free(dv);
        return(NULL);
    }
    sal_memset(dv->dv_dcb, 0, sizeof(eth_dcb_t) * cnt);

    switch (op) {
    case DV_TX:
        /* alloc tx specific header and tail crc */
#ifdef BCM_ROBO_SUPPORT
        /* No BRCM type (2 bytes) for BCM53115, BCM53118 */
        if (SOC_IS_ROBO_ARCH_VULCAN(unit)) {
            dv->dv_dmabufhdr = (sal_vaddr_t)
                       soc_cm_salloc(unit, SOC_DMA_TX_HDR_53115, "dma_tx_hdr");
            dv->dv_dmabufcrc = (sal_vaddr_t)
                       soc_cm_salloc(unit, 4, "dma_tx_crc");
        } else if (SOC_IS_TBX(unit)) {
            dv->dv_dmabufhdr = (sal_vaddr_t)
                       soc_cm_salloc(unit, SOC_DMA_TX_HDR_53280, "dma_tx_hdr");
            dv->dv_dmabufcrc = (sal_vaddr_t)
                       soc_cm_salloc(unit, 4, "dma_tx_crc");
        } else {
            dv->dv_dmabufhdr = (sal_vaddr_t)
                       soc_cm_salloc(unit, SOC_DMA_TX_HDR, "dma_tx_hdr");
            dv->dv_dmabufcrc = (sal_vaddr_t)
                       soc_cm_salloc(unit, SOC_DMA_TX_CRC, "dma_tx_crc");
        }
#endif

        /* fail on any of above */
        if (!dv->dv_dmabufhdr || !dv->dv_dmabufcrc) {
                soc_cm_print("ERROR: unit %d tx dmabuf hdr/crc alloc failed.\n",
                 unit);
            if (dv->dv_dmabufhdr) {
                    soc_cm_sfree(unit, (void *)dv->dv_dmabufhdr);
                }

            if (dv->dv_dmabufcrc) {
                    soc_cm_sfree(unit, (void *)dv->dv_dmabufcrc);
                }

            sal_free(dv->dv_dcb);
            sal_free(dv);
            return (NULL);
        }

#ifdef BCM_ROBO_SUPPORT
        /* No BRCM type (2 bytes) for BCM53115, BCM53118 */
        if (SOC_IS_ROBO_ARCH_VULCAN(unit)) {
            sal_memset((void *)dv->dv_dmabufhdr, 0, SOC_DMA_TX_HDR_53115);
            sal_memset((void *)dv->dv_dmabufcrc, 0, 4);
        } else if (SOC_IS_TBX(unit)) {
            sal_memset((void *)dv->dv_dmabufhdr, 0, SOC_DMA_TX_HDR_53280);
            sal_memset((void *)dv->dv_dmabufcrc, 0, 4);
        } else {
            sal_memset((void *)dv->dv_dmabufhdr, 0, SOC_DMA_TX_HDR);
            sal_memset((void *)dv->dv_dmabufcrc, 0, SOC_DMA_TX_CRC);
        }
#endif
        break;

    case DV_RX:
        dv->dv_dmabufhdr = (sal_vaddr_t)NULL;
        dv->dv_dmabufcrc = (sal_vaddr_t)NULL;
        break;

    default:
        soc_cm_print("ERROR: unit %d unknown dma op %d\n", unit, op);
        assert(0);
        return (NULL);
    }

    dv->dv_unit        = unit;
    dv->dv_magic       = DV_MAGIC_NUMBER;
    dv->dv_op          = op;
    dv->dv_length      = 0;
    dv->dv_cnt         = cnt;
    dv->dv_vcnt        = 0;
    dv->dv_dcnt        = 0;
    dv->dv_brcm_tag    = 0;
    dv->dv_brcm_tag2 = 0;
    dv->dv_done_packet = NULL;
    dv->dv_public1.ptr = NULL;
    dv->dv_public2.ptr = NULL;
    dv->dv_public3.ptr = NULL;
    dv->dv_public4.ptr = NULL;
    dv->dv_channel = 0;
    return(dv);
}

/*
 * Function:
 *  soc_eth_dma_dv_free
 * Purpose:
 *  Free a dv struct.
 * Parameters:
 *  dv - pointer to dv to free (NOT a dv chain).
 * Returns:
 *  Nothing.
 */

void
soc_eth_dma_dv_free(int unit, eth_dv_t *dv)
{
    soc_control_t   *soc = SOC_CONTROL(unit);

    soc->stat.dv_free++;
    assert(dv->dv_magic == DV_MAGIC_NUMBER);
    dv->dv_magic = 0;

    if (dv->dv_op == DV_TX) {
        /* free tx extra dma buffer */
        soc_cm_sfree(unit, (void *)dv->dv_dmabufhdr);
        soc_cm_sfree(unit, (void *)dv->dv_dmabufcrc);
    }

    if (dv->dv_dcb) {
        sal_free((void *)dv->dv_dcb);
    }

    sal_free((void *)dv);
}

/*
 * Function:
 *  soc_eth_dma_desc_add
 * Purpose:
 *  Add a DMA descriptor to a DMA chain independent of the
 *  descriptor type.
 * Parameters:
 *  dv - pointer to DMA I/O Vector to be filled in.
 *  addr/cnt - values to add to the DMA chain.
 * Returns:
 *  < 0 - SOC_E_XXXX error code
 *  >= 0 - # entries left that may be filled.
 * Notes:
 *  Calls the specific fastpath routine if it can, defaulting
 *  to a general routine.
 */

int
soc_eth_dma_desc_add(eth_dv_t *dv, sal_vaddr_t addr, uint16 cnt)
{
    int16    dcb = dv->dv_vcnt;
    assert((dv->dv_vcnt >= 0) && (dv->dv_vcnt < dv->dv_cnt));

    dv->dv_dcb[dcb].dcb_vaddr = addr;
    dv->dv_dcb[dcb].len = cnt;
    dv->dv_dcb[dcb].next = NULL;
    if (dcb > 0) {
        dv->dv_dcb[dcb-1].next = (eth_dcb_t *)&dv->dv_dcb[dcb];
    }
    dv->dv_vcnt++;
    dv->dv_length += cnt;
    return(dv->dv_cnt - dv->dv_vcnt);
}

/*
 * Function:
 *  soc_eth_dma_dv_join
 * Purpose:
 *  Append src_chain to the end of dv_chain
 * Parameters:
 *  dv_chain - pointer to DV chain to be appended to.
 *  src_chain - pointer to DV chain to add.
 * Returns:
 *  SOC_E_NONE - success
 *  SOC_E_XXXX - error code.
 * Note:
 *  src_chain is consumed and should not be further referenced.
 *  If the last DCB in the chained list has the S/G
 *  bit set, then the S/G bit is set in the RLD dcb.
 *  The notification routines MUST be the same in all
 *  elements of the list (this condition is asserted)
 */

int
soc_eth_dma_dv_join(eth_dv_t *dv_chain, eth_dv_t *src_chain)
{
    return(SOC_E_UNAVAIL);
}

/*
 * Function:
 *  soc_eth_dma_dv_reset
 * Purpose:
 *  Reinitialize a dv struct to avoid free/alloc to reuse it.
 * Parameters:
 *  op - operation type requested.
 * Returns:
 *  Nothing.
 */

void
soc_eth_dma_dv_reset(dvt_t op, eth_dv_t *dv)
{
    dv->dv_op      = op;
    dv->dv_vcnt    = 0;
    dv->dv_dcnt    = 0;
    /* don't clear all flags */
    dv->dv_flags   = 0;
    dv->dv_public1.ptr = NULL;
    dv->dv_public2.ptr = NULL;
    dv->dv_public3.ptr = NULL;
    dv->dv_public4.ptr = NULL;
}

/*
 * Function:
 *  soc_eth_dma_attach
 * Purpose:
 *  Setup DMA structures whedn a device is attached.
 * Parameters:
 *  unit - StrataSwitch unit #.
 * Returns:
 *  SOC_E_NONE - Attached successful.
 *  SOC_E_xxx  - Attach failed.
 * Notes:
 *  Initializes data structure without regards to the current fields,
 *  calling this routine without detach first may result in memory
 *  leaks.
 */

int
soc_eth_dma_attach(int unit)
{
    int         rv;
    soc_control_t   *soc;

    _soc_robo_device_created(unit);
    soc = SOC_CONTROL(unit);
#ifdef ETH_MII_DEBUG
    SOC_DEBUG_PRINT((DK_PCI, "soc_eth_dma_attach: unit=%d\n", unit));
    /*
     * Attached flag must be true during initialization.
     * If initialization fails, the flag is cleared by soc_detach (below).
     */

    soc->soc_flags |= SOC_F_ATTACHED;

    if (soc_ndev_attached++ == 0) {
        int         chip;

        /* Work to be done before the first SOC device is attached. */
        for (chip = 0; chip < SOC_ROBO_NUM_SUPPORTED_CHIPS; chip++) {
            /* Call each chip driver's init function */
            if (soc_robo_base_driver_table[chip]->init) {
                (soc_robo_base_driver_table[chip]->init)();
            }
        }
    }
#endif
    soc->stat.dv_alloc      = 0;    /* Init Alloc count */
    soc->stat.dv_free       = 0;    /* Init Free count */
    soc->stat.dv_alloc_q    = 0;    /* Init Alloc from Q count */
    if ((rv = et_soc_attach(unit)) != SOC_E_NONE) {
        soc_cm_print("soc_eth_dma_attach: et_soc_attach failed %d\n", rv);
        return rv;
    }
    return et_soc_open(unit);
}

/*
 * Function:
 *  soc_eth_dma_detach
 * Purpose:
 *  Abort DMA active on the specified channel, and free
 *  internal memory associated with DMA on the specified unit.
 *  It is up to the caller to ensure NO more DMAs are started.
 * Parameters:
 *  unit - StrataSwitch Unit number
 * Returns:
 *  SOC_E_TIMEOUT - indicates attempts to abort active
 *      operations failed. Device is detached, but operation
 *      is undefined at this point.
 *  SOC_E_NONE - Operation sucessful.
 */

int
soc_eth_dma_detach(int unit)
{
    et_soc_close(unit);
    return et_soc_detach(unit);
}

/*
 * Reuse pmux field.
 */
#define dv_sem      dv_public4.ptr
#define dv_poll     dv_public4.ptr

/*
 * Function:
 *  soc_eth_dma_dma_wait_done (Internal only)
 * Purpose:
 *  Callout for DMA chain done.
 * Parameters:
 *  unit - StrataSwitch Unit #.
 *  dv_chain - Chain completed.
 * Returns:
 *  Nothing
 */

STATIC void
soc_eth_dma_dma_wait_done(int unit, eth_dv_t *dv_chain)
{
    COMPILER_REFERENCE(unit);
    sal_sem_give((sal_sem_t)dv_chain->dv_sem);
}

/*
 * Function:
 *  soc_eth_dma_wait
 * Purpose:
 *  Start a DMA operation and wait for it's completion.
 * Parameters:
 *  unit - StrataSwitch unit #.
 *  dv_chain - pointer to dv chain to execute.
 *      usec - Time out in microseconds.  Same meanings as sal_sem_take
 * Returns:
 *  SOC_E_XXXX
 */

int
soc_eth_dma_wait_timeout(int unit, eth_dv_t *dv_chain, int usec)
{
    int     rv = SOC_E_NONE;
    sal_usecs_t start_time;
    int diff_time;

    dv_chain->dv_sem = sal_sem_create("dv_sem", sal_sem_BINARY, 0);
    if (!dv_chain->dv_sem) {
        return(SOC_E_MEMORY);
    }
    dv_chain->dv_done_chain = soc_eth_dma_dma_wait_done;

    soc_eth_dma_start(unit, dv_chain);

    start_time = sal_time_usecs();
    diff_time = 0;
    if (sal_sem_take((sal_sem_t)dv_chain->dv_sem, sal_sem_FOREVER)) {
        rv = SOC_E_TIMEOUT;
    }
    sal_sem_destroy((sal_sem_t)dv_chain->dv_sem);

    return(rv);
}

/*
 * Function:
 *  soc_eth_dma_wait
 * Purpose:
 *  Start a DMA operation and wait for it's completion.
 * Parameters:
 *  unit - StrataSwitch unit #.
 *  dv_chain - pointer to dv chain to execute.
 * Returns:
 *  SOC_E_XXXX
 */

int
soc_eth_dma_wait(int unit, eth_dv_t *dv_chain)
{
    return soc_eth_dma_wait_timeout(unit, dv_chain, sal_sem_FOREVER);
}

extern et_soc_info_t *et_soc;
extern void et_soc_init(et_soc_info_t *et, bool full);

void
soc_eth_dma_start_channel(int unit, int channel)
{
    struct chops *chops;
    void *ch;

    chops = et_soc->etc->chops;
    ch = et_soc->etc->ch;

    (*chops->rxinit)(ch, channel);
}

void
soc_eth_dma_abort_channel(int unit, int channel)
{
    struct chops *chops;
    void *ch;

    chops = et_soc->etc->chops;
    ch = et_soc->etc->ch;

    (*chops->rxreset)(ch, channel);
}

void
soc_eth_dma_reinit(int unit)
{
    struct chops *chops;
    void *ch;
    int i;
    int test_unit;

    chops = et_soc->etc->chops;
    ch = et_soc->etc->ch;
    ET_SOC_DMA_LOCK(et_soc);
    for (i = 0; i < NUMRXQ; i++) {
        et_soc->rxq_head[i] = NULL;
        et_soc->rxq_tail[i] = NULL;
        et_soc->rxq_cnt[i] = 0;
    }

    if (!(*chops->recover)(ch)) {
        /*
         * If we cannot recover the error using chip's 'recover' call,
         * then do a complete re-init.
         */
        et_soc_init(et_soc, TRUE);
    }

    ET_SOC_DMA_UNLOCK(et_soc);

#if defined(KEYSTONE)
    /* Recover GMAC CFP setting if any */
    if (_soc_eth_dma_oam != -1) {
        soc_eth_dma_classify_setup(unit, 
            socEthDmaClassifyOam, _soc_eth_dma_oam);
    }

    if (_soc_eth_dma_drop_en != -1) {
        soc_eth_dma_default_drop_enable(unit, _soc_eth_dma_drop_en);
    }
#endif
    
    for (i = 0; i < NUMRXQ; i++) {
        soc_eth_dma_occupancy_get(i, &test_unit);
        if (test_unit == unit) {
        } else {
#ifdef BCM_EA_SUPPORT
            if ((test_unit >=0) && SOC_IS_EA(test_unit)) {
                soc_control_t   *soc;
                int init_state;
                soc_ea_oam_ctrl_if_t *ctrl_if;
                soc = SOC_CONTROL(test_unit);
                if (soc->soc_flags & SOC_F_ATTACHED) {
                    ctrl_if = soc->oam_ctrlops;
                    init_state = soc_ea_oam_ctrl_state_get(test_unit);
                    
                    if(init_state == socEaOamCtrlInitFalse){
                        ctrl_if->init(test_unit);
                    }
                }
            }
#endif
        }
    }
}

void
soc_eth_dma_hwrxoff_get(int unit, uint32* hwrxoff)
{
    *hwrxoff = et_soc->etc->hwrxoff;
    return; 
}

void
soc_eth_dma_occupancy_set(int dma_id, int unit)
{
    eth_dma_occupied[dma_id] = unit;
}

void
soc_eth_dma_occupancy_get(int dma_id, int *unit)
{
    *unit = eth_dma_occupied[dma_id];
}

#if defined(KEYSTONE)
#define SET_CFP_FIELD_PARAM(fld_idx, fld_val, ram, l3_fram, s_id, buf_ptr) { \
    buf_ptr->field_idx = fld_idx; buf_ptr->field_value = fld_val; \
    buf_ptr->ram_type = ram; buf_ptr->l3_framing = l3_fram; \
    buf_ptr->slice_id = s_id;}

#define SOC_ETH_CFP_DMA_ENTRY_NUM 2 /*1 */

#ifdef BCM_ROBO_SUPPORT
static void
_soc_eth_cfp_entry_init(ch_t *ch, uint l3_fram, uint slice_id, void *arg)
{
    struct chops *chops;
    int i;
    cfp_ioctl_buf_t *cfp_buf_ptr;

    chops = et_soc->etc->chops;

    cfp_buf_ptr = (cfp_ioctl_buf_t *)arg;

    for (i=0; i < CFP_TCAM_ENTRY_WORD; i++) {
        cfp_buf_ptr->cfp_entry.tcam[i] = 0;
        cfp_buf_ptr->cfp_entry.tcam_mask[i] = 0;
    }
    cfp_buf_ptr->cfp_entry.action= 0;

    /* Create valid entry and drop action */
    cfp_buf_ptr->entry_idx = 0;
    SET_CFP_FIELD_PARAM(CFP_FIELD_IPV4_VALID, 3, CFP_RAM_TYPE_TCAM, 
        CFP_L3_FRAMING_IPV4, slice_id, cfp_buf_ptr);
    chops->cfpfldwr(ch, (void *)(cfp_buf_ptr));

    SET_CFP_FIELD_PARAM(CFP_FIELD_IPV4_VALID, 3, CFP_RAM_TYPE_TCAM_MASK, 
        CFP_L3_FRAMING_IPV4, slice_id, cfp_buf_ptr);
    chops->cfpfldwr(ch, (void *)(cfp_buf_ptr));

    /* Configure the L3 Framming value */
    SET_CFP_FIELD_PARAM(CFP_FIELD_NONIP_L3_FRAMING, l3_fram, 
        CFP_RAM_TYPE_TCAM, CFP_L3_FRAMING_IPV4, slice_id, cfp_buf_ptr);
    chops->cfpfldwr(ch, (void *)(cfp_buf_ptr));
    SET_CFP_FIELD_PARAM(CFP_FIELD_NONIP_L3_FRAMING, 0x3, 
        CFP_RAM_TYPE_TCAM_MASK, CFP_L3_FRAMING_IPV4, slice_id, cfp_buf_ptr);
    chops->cfpfldwr(ch, (void *)(cfp_buf_ptr));

    /* set slice id */
    SET_CFP_FIELD_PARAM(CFP_FIELD_IPV4_SLICE_ID, slice_id, 
            CFP_RAM_TYPE_TCAM, CFP_L3_FRAMING_IPV4, slice_id, cfp_buf_ptr);
    chops->cfpfldwr(ch, (void *)(cfp_buf_ptr));
    SET_CFP_FIELD_PARAM(CFP_FIELD_IPV4_SLICE_ID, 3, 
            CFP_RAM_TYPE_TCAM_MASK, CFP_L3_FRAMING_IPV4, slice_id, cfp_buf_ptr);
    chops->cfpfldwr(ch, (void *)(cfp_buf_ptr));
    
}
#endif

static void
_soc_eth_cfp_oam_dma_setup(ch_t *ch, int dma_id)
{
    struct chops *chops;
    cfp_ioctl_buf_t *cfp_buffer;
#ifdef BCM_ROBO_SUPPORT
    uint slice_id, start_entry_idx;
    uint udf_field_idx = 0, field_value;
    uint udf_valid_field_idx = 0;
    uint i, l3_fram;
    uint8 oam_bpdu_addr[6] = {0x01, 0x80, 0xc2, 0x00, 0x00, 0x02};
#endif

    chops = et_soc->etc->chops;

    /* Save a copy of reserved dma id */
    _soc_eth_dma_oam = dma_id;

    if (_soc_eth_dma_oam < 0) {
        soc_cm_debug(DK_WARN, "Keystone CFP: Do not classify OAM packets\n");
    }

    if (!(cfp_buffer = (cfp_ioctl_buf_t *)ET_MALLOC(sizeof(cfp_ioctl_buf_t)))) {
        soc_cm_debug(DK_WARN, "Error : gamc_cfp_promisc() KMALLOC failed!");
        return;
    }

#ifdef BCM_ROBO_SUPPORT
    if (SOC_IS_TBX(soc_mii_unit)) {
        start_entry_idx = CFP_TCAM_NUM - SOC_ETH_CFP_DMA_ENTRY_NUM;
    
        /* Choose the UDF 0, 1, 2 of SLICE 2 to use */
        slice_id = 0;
    
        l3_fram = CFP_L3_FRAMING_NONIP;
        _soc_eth_cfp_entry_init(ch, l3_fram, slice_id, cfp_buffer);
    
        cfp_buffer->entry_idx = start_entry_idx;

        /* BRCM tag(8 bytes)+Packet */

        /* 2. Classify DA: UDF_0_C1/2/3  */
        /* Configure the UDF offset value and base */
        udf_field_idx = CFP_FIELD_NONIP_UDF1;
        udf_valid_field_idx = CFP_FIELD_NONIP_UDF1_VLD;
        for (i= 0; i < 3; i++) {
            cfp_buffer->field_idx = i+1; /* UDF index */
            cfp_buffer->l3_framing = l3_fram;
            cfp_buffer->slice_id= slice_id;
            cfp_buffer->field_value = (2 * i)+ (ENET_BRCM_SHORT_TAG_SIZE*2); /* offset value */
            cfp_buffer->flags = CFP_UDF_OFFSET_BASE_STARTOFFRAME;
            chops->cfpudfwr(ch, (void *)(cfp_buffer));
    
            field_value = ((oam_bpdu_addr[(2 * i)] << 8) | (oam_bpdu_addr[(2 * i) + 1]));
            /* UDF value */
            SET_CFP_FIELD_PARAM(udf_field_idx + i, field_value, 
                CFP_RAM_TYPE_TCAM, l3_fram, slice_id, cfp_buffer);
            chops->cfpfldwr(ch, (void *)(cfp_buffer));
            SET_CFP_FIELD_PARAM(udf_field_idx + i, 0xffff, 
                CFP_RAM_TYPE_TCAM_MASK, l3_fram, slice_id, cfp_buffer);
            chops->cfpfldwr(ch, (void *)(cfp_buffer));
            /* UDF valid */
            SET_CFP_FIELD_PARAM(udf_valid_field_idx + i, 0x1, 
                CFP_RAM_TYPE_TCAM, l3_fram, slice_id, cfp_buffer);
            chops->cfpfldwr(ch, (void *)(cfp_buffer));
            SET_CFP_FIELD_PARAM(udf_valid_field_idx + i, 0x1, 
                CFP_RAM_TYPE_TCAM_MASK, l3_fram, slice_id, cfp_buffer);
            chops->cfpfldwr(ch, (void *)(cfp_buffer));
         }
    
        /* 3. Classify Length/Type: UDF_0_C4 */
        /* Configure the UDF offset value and base */
        cfp_buffer->field_idx = 4; /* UDF index */
        cfp_buffer->l3_framing = l3_fram;
        cfp_buffer->slice_id= slice_id;
        cfp_buffer->field_value = 24; /* offset value */
        cfp_buffer->flags = CFP_UDF_OFFSET_BASE_STARTOFFRAME;
        chops->cfpudfwr(ch, (void *)(cfp_buffer));
    
        /* CFP entries */
        udf_field_idx = CFP_FIELD_NONIP_UDF4;
        udf_valid_field_idx = CFP_FIELD_NONIP_UDF4_VLD;
    
        /* UDF value */
        field_value = 0x8809;
        SET_CFP_FIELD_PARAM(udf_field_idx, field_value, 
            CFP_RAM_TYPE_TCAM, l3_fram, slice_id, cfp_buffer);
        chops->cfpfldwr(ch, (void *)(cfp_buffer));
        SET_CFP_FIELD_PARAM(udf_field_idx, 0xffff, 
            CFP_RAM_TYPE_TCAM_MASK, l3_fram, slice_id, cfp_buffer);
        chops->cfpfldwr(ch, (void *)(cfp_buffer));
    
        /* UDF valid */
        SET_CFP_FIELD_PARAM(udf_valid_field_idx, 0x1, 
            CFP_RAM_TYPE_TCAM, l3_fram, slice_id, cfp_buffer);
        chops->cfpfldwr(ch, (void *)(cfp_buffer));
        SET_CFP_FIELD_PARAM(udf_valid_field_idx, 0x1, 
            CFP_RAM_TYPE_TCAM_MASK, l3_fram, slice_id, cfp_buffer);
        chops->cfpfldwr(ch, (void *)(cfp_buffer));
    
    
        /* 4. Classify Subtype: UDF_0_C5 */
        /* Configure the UDF offset value and base */
        cfp_buffer->field_idx = 5; /* UDF index */
        cfp_buffer->l3_framing = l3_fram;
        cfp_buffer->slice_id= slice_id;
        cfp_buffer->field_value = 26; /* offset value */
        cfp_buffer->flags = CFP_UDF_OFFSET_BASE_STARTOFFRAME;
        chops->cfpudfwr(ch, (void *)(cfp_buffer));
    
        /* CFP entries */
        udf_field_idx = CFP_FIELD_NONIP_UDF5;
        udf_valid_field_idx = CFP_FIELD_NONIP_UDF5_VLD;
    
        /* UDF value */
        field_value = 0x0300;
        SET_CFP_FIELD_PARAM(udf_field_idx, field_value, 
            CFP_RAM_TYPE_TCAM, l3_fram, slice_id, cfp_buffer);
        chops->cfpfldwr(ch, (void *)(cfp_buffer));
        SET_CFP_FIELD_PARAM(udf_field_idx, 0xff01, 
            CFP_RAM_TYPE_TCAM_MASK, l3_fram, slice_id, cfp_buffer);
        chops->cfpfldwr(ch, (void *)(cfp_buffer));
    
        /* UDF valid */
        SET_CFP_FIELD_PARAM(udf_valid_field_idx, 0x1, 
            CFP_RAM_TYPE_TCAM, l3_fram, slice_id, cfp_buffer);
        chops->cfpfldwr(ch, (void *)(cfp_buffer));
        SET_CFP_FIELD_PARAM(udf_valid_field_idx, 0x1, 
            CFP_RAM_TYPE_TCAM_MASK, l3_fram, slice_id, cfp_buffer);
        chops->cfpfldwr(ch, (void *)(cfp_buffer));
    
        /* If dma_id = -1, no action for the rule */
        if (dma_id >= 0) {
            SET_CFP_FIELD_PARAM(CFP_FIELD_ACT_RX_CHANNEL_ID, dma_id, CFP_RAM_TYPE_ACTION, 
                l3_fram, slice_id, cfp_buffer);
            chops->cfpfldwr(ch, (void *)(cfp_buffer));
        }

        /* write this entry to CFP */
        chops->cfpwr(ch, (void *)(cfp_buffer));
    
        /* Add new switch chip with different BRCM tag size or location here. */
    } else {
        soc_cm_debug(DK_WARN, "No Keystone GMAC CFP rule created for OAM packets\n");
    }
#endif

    if (cfp_buffer) {
        ET_MFREE(cfp_buffer, sizeof(cfp_ioctl_buf_t));
    }

}

static void
_soc_eth_cfp_dma_default_drop_enable(ch_t *ch, int drop_enable)
{
    struct chops *chops;
    cfp_ioctl_buf_t *cfp_buffer;
    uint slice_id, start_entry_idx;
    uint l3_fram;
    int i;
    
    chops = et_soc->etc->chops;

    _soc_eth_dma_drop_en = drop_enable;
    
    if (!(cfp_buffer = (cfp_ioctl_buf_t *)ET_MALLOC(sizeof(cfp_ioctl_buf_t)))) {
        soc_cm_print("Error : gamc_cfp_promisc() KMALLOC failed!");
        return;
    }


    /* Using the last 1 entries */
    start_entry_idx = CFP_TCAM_NUM - 1;

    slice_id = 0;

    for (i=0; i < CFP_TCAM_ENTRY_WORD; i++) {
        cfp_buffer->cfp_entry.tcam[i] = 0;
        cfp_buffer->cfp_entry.tcam_mask[i] = 0;
    }
    cfp_buffer->cfp_entry.action= 0;

    /* Create valid entry and drop action */
    cfp_buffer->entry_idx = 0;
    SET_CFP_FIELD_PARAM(CFP_FIELD_IPV4_VALID, 3, CFP_RAM_TYPE_TCAM, 
        CFP_L3_FRAMING_IPV4, slice_id, cfp_buffer);
    chops->cfpfldwr(ch, (void *)(cfp_buffer));

    SET_CFP_FIELD_PARAM(CFP_FIELD_IPV4_VALID, 3, CFP_RAM_TYPE_TCAM_MASK, 
        CFP_L3_FRAMING_IPV4, slice_id, cfp_buffer);
    chops->cfpfldwr(ch, (void *)(cfp_buffer));

    /* set slice id */
    SET_CFP_FIELD_PARAM(CFP_FIELD_IPV4_SLICE_ID, slice_id, 
            CFP_RAM_TYPE_TCAM, CFP_L3_FRAMING_IPV4, slice_id, cfp_buffer);
    chops->cfpfldwr(ch, (void *)(cfp_buffer));
    SET_CFP_FIELD_PARAM(CFP_FIELD_IPV4_SLICE_ID, 3, 
            CFP_RAM_TYPE_TCAM_MASK, CFP_L3_FRAMING_IPV4, slice_id, cfp_buffer);
    chops->cfpfldwr(ch, (void *)(cfp_buffer));

    cfp_buffer->entry_idx = start_entry_idx;

    /* Drop in-band traffic only.  */
    SET_CFP_FIELD_PARAM(CFP_FIELD_IPV4_SRC_PMAP, 0x1 << et_soc->etc->coreunit, 
        CFP_RAM_TYPE_TCAM, CFP_L3_FRAMING_IPV4, slice_id, cfp_buffer);
    chops->cfpfldwr(ch, (void *)(cfp_buffer));

    SET_CFP_FIELD_PARAM(CFP_FIELD_IPV4_SRC_PMAP, 0x3, 
        CFP_RAM_TYPE_TCAM_MASK, CFP_L3_FRAMING_IPV4, slice_id, cfp_buffer);
    chops->cfpfldwr(ch, (void *)(cfp_buffer));

    l3_fram = CFP_L3_FRAMING_NONIP;

    SET_CFP_FIELD_PARAM(CFP_FIELD_ACT_DROP, drop_enable, CFP_RAM_TYPE_ACTION, 
        l3_fram, slice_id, cfp_buffer);
    chops->cfpfldwr(ch, (void *)(cfp_buffer));

    /* write this entry to CFP */
    chops->cfpwr(ch, (void *)(cfp_buffer));

    if (cfp_buffer) {
        ET_MFREE(cfp_buffer, sizeof(cfp_ioctl_buf_t));
    }

}

void
soc_eth_dma_classify_setup(int unit, int type, int chan)
{
    void *ch;
    int max_channels = N_DMA_CHAN;

    if (chan >= max_channels) {
        soc_cm_debug(DK_ERR, \
            "soc_eth_dma_classify_setup: invalid dma channel\n");
        return;
    }

    ch = et_soc->etc->ch;

    ET_SOC_DMA_LOCK(et_soc);
    switch (type) {
        case socEthDmaClassifyOam:
            _soc_eth_cfp_oam_dma_setup(ch, chan);
            break;
        default:
            soc_cm_debug(DK_WARN, \
            "soc_eth_dma_classify_setup: Unknown type for DMA classfication\n");
            break;
    }
    ET_SOC_DMA_UNLOCK(et_soc);

    return;
}

void
soc_eth_dma_default_drop_enable(int unit, int drop_enable)
{
    void *ch;

    ch = et_soc->etc->ch;

    ET_SOC_DMA_LOCK(et_soc);
    _soc_eth_cfp_dma_default_drop_enable(ch, drop_enable);
    ET_SOC_DMA_UNLOCK(et_soc);

    return;
}
#endif

#else

eth_dv_t *
soc_eth_dma_dv_alloc(int unit, dvt_t op, int cnt)
{
    return NULL;
}

void
soc_eth_dma_dv_free(int unit, eth_dv_t *dv)
{
    return;
}

void
soc_eth_dma_dump_dv(int unit, char *pfx, eth_dv_t *dv_chain)
{
    return;
}

void
soc_eth_dma_dump_pkt(int unit, char *pfx, uint8 *addr, int len)
{
    return;
}

void
soc_eth_dma_ether_dump(int unit, char *pfx, uint8 *addr, int len, int offset)
{
    return;
}

int
soc_eth_dma_desc_add(eth_dv_t *dv, sal_vaddr_t addr, uint16 cnt)
{
    return SOC_E_UNAVAIL;
}

int
soc_eth_dma_start(int unit, eth_dv_t *dv_chain)
{
    return SOC_E_UNAVAIL;
}

void
soc_eth_dma_dv_reset(dvt_t op, eth_dv_t *dv)
{
    return;
}

void
soc_eth_dma_hwrxoff_get(int unit, uint32* hwrxoff)
{
    return; 
}

void
soc_eth_dma_reinit(int unit)
{
    return; 
}

void
soc_eth_dma_occupancy_set(int dma_id, int unit)
{
    return; 
}

void
soc_eth_dma_occupancy_get(int dma_id, int *unit)
{
    return; 
}

int
soc_eth_dma_rxenable(int unit)
{
    return SOC_E_UNAVAIL;
}

int
soc_eth_dma_rxstop(int unit)
{
    return SOC_E_UNAVAIL;
}

int
soc_eth_dma_dv_valid(eth_dv_t *dv)
{
    return FALSE;
}

int
soc_eth_dma_wait_timeout(int unit, eth_dv_t *dv_chain, int usec)
{
     return SOC_E_UNAVAIL;
}

int
soc_eth_dma_wait(int unit, eth_dv_t *dv_chain)
{
    return SOC_E_UNAVAIL;
}

int
soc_eth_dma_attach(int unit)
{
    return SOC_E_UNAVAIL;
}

int
soc_eth_dma_detach(int unit)
{
    return SOC_E_UNAVAIL;
}

void
soc_eth_dma_start_channel(int unit, int channel)
{
    return; 
}

void
soc_eth_dma_abort_channel(int unit, int channel)
{
    return; 
}


#endif /* defined(KEYSTONE) || defined(ROBO_4704) */