/*
 * $Id: filter.h 1.5 Broadcom SDK $
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
 */

#ifndef _SOC_FILTER_H
#define _SOC_FILTER_H

/*
 * Rule definitions
 *
 * NOTE:  In socregs, filter_irule_entry_t and filter_imask_entry_t are
 *	  big-endian formats.  This was done so the filter data would
 *	  align with network packet data.  Unfortunately, the ENTIRE
 *	  structure is reversed, not just the 64-byte field.
 *
 *        Always, in a multi-word memory field represented as an array
 *        of uint32, the LEAST SIGNIFICANT WORD comes first, and within
 *        each word, the MOST SIGNIFICANT BYTE comes first.
 *
 *        Therefore, IRULEf[N-1] must be 0x12345678 in order to match a
 *        packet beginning with the bytes: 0x12 0x34 0x56 0x78, where N
 *        is the number of words in the field (16 or 20).
 *
 *        The above is true regardless of host endian since the lower-
 *        level routines swap appropriately when copying the data out to
 *        the hardware as 32-bit words.
 */

#define FILTER_ACTION_INSERT_PRIO		0x1	/* Also sets COSQ */
#define FILTER_ACTION_SET_COS_QUEUE		0x2	/* No change to pkt */
#define FILTER_ACTION_INSERT_TOSP		0x4	/* IRULE.TOS_P */
#define FILTER_ACTION_COPY_TO_CPU		0x8
#define FILTER_ACTION_DO_NOT_SWITCH		0x10
#define FILTER_ACTION_SET_OUTPUT_PORT		0x20
#define FILTER_ACTION_COPY_TO_MIRROR		0x40

/* Extended Filtering */

#define FILTER_ACTION_INCR_COUNTER		0x80	/* IRULE.COUNTER */
    /* Note: ^^ this action may not be used for IMASK.NOMATCHACTION */
#define FILTER_ACTION_INSERT_PRIO_FROM_TOSP	0x100	/* Also sets COSQ */
#define FILTER_ACTION_INSERT_TOSP_FROM_PRIO	0x200
#define FILTER_ACTION_INSERT_DIFFSERV		0x400	/* IRULE.DIFFSERV */
#define FILTER_ACTION_SET_OUTPUT_PORT_ALL	0x800
#define FILTER_ACTION_DROP_PRECEDENCE		0x1000	/* XGS only */
#define FILTER_ACTION_DO_SWITCH			0x2000	/* Overrides 0x10 */
    /* Note: ^^ this action may not be used for IMASK.NOMATCHACTION */
#define FILTER_ACTION_SET_CLASSIFICN_TAG	0x4000	/* XGS only */
#define FILTER_ACTION_INSERT_VLAN_ID		0x8000	/* XGS only */
#define FILTER_ACTION_EGRESS_MASK		0x10000	/* XGS only */
/* Filter action bits for BCM5695 only */
#define FILTER_DRACO15_ACTION_SET_ECN		0x20000	
#define FILTER_DRACO15_ACTION_SET_PORT_BITMAP	0x40000	


/* Filter action bits for 5665 and variants */
#define FILTER_TUCANA_ACTION_INSERT_DIFFSERV_IPV6	0x800 
                                                        /* IRULE.DSCP */
#define FILTER_TUCANA_ACTION_SET_OUTPUT_PORT_ALL	0x1000
#define FILTER_TUCANA_ACTION_DROP_PRECEDENCE		0x2000
#define FILTER_TUCANA_ACTION_DO_SWITCH			0x4000	
                                                        /* Overrides 0x10 */
    /* Note: ^^ this action may not be used for IMASK.NOMATCHACTION */
#define FILTER_TUCANA_ACTION_DO_NOT_COPY_TO_CPU		0x8000  
                                                        /* Overrides 0x8 */
#define FILTER_TUCANA_ACTION_DO_NOT_SET_OUTPUT_PORT	0x10000
                                                        /* Overrides 0x20 */
    /* Note: vvv these actions may not be used for IMASK.NOMATCHACTION */
#define FILTER_TUCANA_ACTION_SET_CLASSIFICN_TAG	        0x20000
#define FILTER_TUCANA_ACTION_INSERT_VLAN_ID		0x40000
#define FILTER_TUCANA_ACTION_EGRESS_MASK		0x80000
                                                      /* IRULE.PORT_BITMAP */
#define FILTER_TUCANA_ACTION_SET_PORT_BITMAP    	0x100000
                                                      /* IRULE.PORT_BITMAP */
#define FILTER_TUCANA_ACTION_SET_DEST_MAC    	        0x200000
                                                      /* IRULE.DEST_ADDR */
#define FILTER_TUCANA_ACTION_SET_VCLABEL    	        0x400000
                                                      /* IRULE.VCLABEL */

#define FILTER_TUCANA_ACTION_RESOURCE_CONFLICTS \
        (FILTER_TUCANA_ACTION_SET_CLASSIFICN_TAG | \
         FILTER_TUCANA_ACTION_SET_PORT_BITMAP | \
         FILTER_TUCANA_ACTION_SET_DEST_MAC | \
         FILTER_TUCANA_ACTION_EGRESS_MASK)

/* Extended Filtering (BCM5605, BCM5615) */
#define FILTER_OUT_ACTION_COPY_TO_CPU		0x1
#define FILTER_OUT_ACTION_DO_NOT_SWITCH		0x2
#define FILTER_OUT_ACTION_INSERT_DIFFSERV	0x4
#define FILTER_OUT_ACTION_DROP_PRECEDENCE	0x8	/* XGS only */
#define FILTER_OUT_ACTION_DO_SWITCH		0x10	/* Overrides 0x2 */

/* Filter out action bits for 5665 and variants */
#define FILTER_TUCANA_OUT_ACTION_INSERT_DIFFSERV_IPV6	0x8
#define FILTER_TUCANA_OUT_ACTION_DROP_PRECEDENCE	0x10
#define FILTER_TUCANA_OUT_ACTION_DO_SWITCH		0x20
                                                        /* Overrides 0x2 */
#define FILTER_TUCANA_OUT_ACTION_DO_NOT_COPY_TO_CPU	0x30
                                                        /* Overrides 0x1 */
/* Filter out action bits for BCM5695 only */
#define FILTER_DRACO15_OUT_ACTION_SET_ECN	0x20	

#define FILTER_OUT_ACTION_ALL(unit)                                   \
	(SOC_IS_TUCANA(unit) ? 0x7f :                                 \
          SOC_IS_DRACO15(unit) ? 0x3f :                               \
           (soc_feature((unit), soc_feature_filter_extended) ? 0x1f : \
           0x0))

/* Extended Filtering (BCM5605, BCM5615, XGS) */

#define FILTER_PKTFORMAT_UNTAGGED_ETH_II	0x1
#define FILTER_PKTFORMAT_UNTAGGED_802_3		0x2	/* type/len < 0x600 */
#define FILTER_PKTFORMAT_TAGGED_ETH_II		0x4
#define FILTER_PKTFORMAT_TAGGED_802_3		0x8	/* type/len < 0x600 */
#define FILTER_PKTFORMAT_ALL			0xf

/* Enhanced format Filtering (BCM5673, BCM5695) */
#define FILTER_ENHANCED_PKTFORMAT_ETH_II	0x01
#define FILTER_ENHANCED_PKTFORMAT_SNAP  	0x02
#define FILTER_ENHANCED_PKTFORMAT_LLC   	0x03
#define FILTER_ENHANCED_PKTFORMAT_INNER_TAGGED	0x04
#define FILTER_ENHANCED_PKTFORMAT_OUTER_TAGGED	0x08
#define FILTER_ENHANCED_PKTFORMAT_IPV4   	0x10
#define FILTER_ENHANCED_PKTFORMAT_IPV6   	0x20
#define FILTER_PKTFORMAT_ALL_LYNX		0x0f
#define FILTER_PKTFORMAT_ALL_DRACO15		0x3f

/* Tucana format Filtering (BCM5665) */
#define FILTER_ENHANCED_PKTFORMAT_TUC_802_3     0x0
#define FILTER_ENHANCED_PKTFORMAT_TUC_ETH_II    0x1
#define FILTER_ENHANCED_PKTFORMAT_TUC_802_1Q    0x2
#define FILTER_ENHANCED_PKTFORMAT_TUC_OUTER_TAG	0x4
#define FILTER_ENHANCED_PKTFORMAT_TUC_IPV4      0x8
#define FILTER_ENHANCED_PKTFORMAT_TUC_IPV6      0x10
#define FILTER_PKTFORMAT_ALL_TUCANA             0x1f

#define FILTER_EPORT_STRATA_PBMP_ZERO		0x3f
#define FILTER_EPORT_STRATA_KNOWN_MC		0x3e

#define FILTER_EPORT_XGS_UNKNOWN_UC		0x1f
#define FILTER_EPORT_XGS_KNOWN_MC		0x1e
#define FILTER_EPORT_XGS_UNKNOWN_MC		0x1d

extern int soc_filter_enable_set(int unit, int enable);
extern int soc_filter_enable_get(int unit, int *enable);
extern int soc_filter_enable_hw_get(int unit, int *enable);

#endif	/* !_SOC_FILTER_H */
