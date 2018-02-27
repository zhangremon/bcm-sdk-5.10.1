/*
 * $Id: dcb.c 1.33.4.6 Broadcom SDK $
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
 * File:        dcb.c
 * Purpose:     DCB manipulation routines
 *              Provide a uniform means of manipulation of DMA control blocks
 *              that is independent of the actual DCB format used in any
 *              particular chip.
 */

#include <soc/types.h>
#include <soc/drv.h>
#include <soc/dcb.h>
#include <soc/dcbformats.h>
#include <soc/higig.h>
#include <soc/dma.h>
#include <soc/debug.h>
#include <soc/cm.h>
#include <soc/rx.h>

#if defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT)

/*
 * Short cuts for generating dcb support functions.
 * Most support functions are just setting or getting a field
 * in the appropriate dcb structure or doing a simple expression
 * based on a couple of fields.
 *      GETFUNCFIELD - get a field from DCB
 *      SETFUNCFIELD - set a field in DCB
 *      SETFUNCERR - dummy handler for field that does not exist for
 *                      a descriptor type
 *      GETFUNCERR - dummy handler for field that does not exist for
 *                      a descriptor type
 */
#define GETFUNCEXPR(_dt, _name, _expr)                                  \
        static uint32 dcb##_dt##_##_name##_get(dcb_t *dcb) {            \
                dcb##_dt##_t *d = (dcb##_dt##_t *)dcb;                  \
                return _expr;                                           \
        }
#define GETFUNCFIELD(_dt, _name, _field)                                \
        GETFUNCEXPR(_dt, _name, d->_field)
#define GETFUNCERR(_dt, _name)                                          \
        static uint32 dcb##_dt##_##_name##_get(dcb_t *dcb) {            \
                COMPILER_REFERENCE(dcb);                                \
                dcb0_funcerr(_dt, #_name "_get");                       \
                return 0;                                               \
        }
#define GETFUNCNULL(_dt, _name)                                         \
        static uint32 dcb##_dt##_##_name##_get(dcb_t *dcb) {            \
                COMPILER_REFERENCE(dcb);                                \
                return 0;                                               \
        }
#define GETPTREXPR(_dt, _name, _expr)                                   \
        static uint32 * dcb##_dt##_##_name##_get(dcb_t *dcb) {          \
                dcb##_dt##_t *d = (dcb##_dt##_t *)dcb;                  \
                return _expr;                                           \
        }
#define GETPTRERR(_dt, _name)                                           \
        static uint32 * dcb##_dt##_##_name##_get(dcb_t *dcb) {          \
                COMPILER_REFERENCE(dcb);                                \
                dcb0_funcerr(_dt, #_name "_get");                       \
                return NULL;                                            \
        }
#define GETFUNCUNITEXPR(_dt, _name, _expr)                              \
        static uint32 dcb##_dt##_##_name##_get(int unit, dcb_t *dcb) {  \
                dcb##_dt##_t *d = (dcb##_dt##_t *)dcb;                  \
                COMPILER_REFERENCE(unit);                               \
                return _expr;                                           \
        }
#define GETFUNCUNITFIELD(_dt, _name, _field)                            \
        GETFUNCUNITEXPR(_dt, _name, d->_field)
#define GETFUNCUNITERR(_dt, _name)                                      \
        static uint32 dcb##_dt##_##_name##_get(int unit, dcb_t *dcb) {  \
                COMPILER_REFERENCE(unit);                               \
                COMPILER_REFERENCE(dcb);                                \
                dcb0_funcerr(_dt, #_name "_get");                       \
                return 0;                                               \
        }
#define SETFUNCEXPR(_dt, _name, _arg, _expr)                            \
        static void dcb##_dt##_##_name##_set(dcb_t *dcb, _arg) {        \
                dcb##_dt##_t *d = (dcb##_dt##_t *)dcb;                  \
                _expr;                                                  \
        }
#define SETFUNCFIELD(_dt, _name, _field, _arg, _expr)                   \
        SETFUNCEXPR(_dt, _name, _arg, d->_field = _expr)
#define SETFUNCERR(_dt, _name, _type)                                   \
        static void dcb##_dt##_##_name##_set(dcb_t *dcb, _type val) {   \
                COMPILER_REFERENCE(dcb);                                \
                COMPILER_REFERENCE(val);                                \
                dcb0_funcerr(_dt, #_name "_set");                       \
        }

#define SETFUNCEXPRIGNORE(_dt, _name, _arg, _expr)                      \
        SETFUNCEXPR(_dt, _name, _arg, COMPILER_REFERENCE(d))
#if defined(LE_HOST)
#define GETHGFUNCEXPR(_dt, _name, _expr)                                \
        static uint32 dcb##_dt##_##_name##_get(dcb_t *dcb) {            \
                dcb##_dt##_t *d = (dcb##_dt##_t *)dcb;                  \
                uint32  hgh[3];                                         \
                soc_higig_hdr_t *h = (soc_higig_hdr_t *)&hgh[0];        \
                hgh[0] = soc_htonl(d->mh0);                             \
                hgh[1] = soc_htonl(d->mh1);                             \
                hgh[2] = soc_htonl(d->mh2);                             \
                return _expr;                                           \
        }
#else
#define GETHGFUNCEXPR(_dt, _name, _expr)                                \
        static uint32 dcb##_dt##_##_name##_get(dcb_t *dcb) {            \
                dcb##_dt##_t *d = (dcb##_dt##_t *)dcb;                  \
                soc_higig_hdr_t *h = (soc_higig_hdr_t *)&d->mh0;        \
                return _expr;                                           \
        }
#endif
#define GETHGFUNCFIELD(_dt, _name, _field)                              \
        GETHGFUNCEXPR(_dt, _name, h->hgp_overlay1._field)

#if defined(LE_HOST)
#define GETHG2FUNCEXPR(_dt, _name, _expr)                               \
        static uint32 dcb##_dt##_##_name##_get(dcb_t *dcb) {            \
                dcb##_dt##_t *d = (dcb##_dt##_t *)dcb;                  \
                uint32  hgh[4];                                         \
                soc_higig2_hdr_t *h = (soc_higig2_hdr_t *)&hgh[0];      \
                hgh[0] = soc_htonl(d->mh0);                             \
                hgh[1] = soc_htonl(d->mh1);                             \
                hgh[2] = soc_htonl(d->mh2);                             \
                hgh[3] = soc_htonl(d->mh3);                             \
                return _expr;                                           \
        }
#else
#define GETHG2FUNCEXPR(_dt, _name, _expr)                               \
        static uint32 dcb##_dt##_##_name##_get(dcb_t *dcb) {            \
                dcb##_dt##_t *d = (dcb##_dt##_t *)dcb;                  \
                soc_higig2_hdr_t *h = (soc_higig2_hdr_t *)&d->mh0;      \
                return _expr;                                           \
        }
#endif
#define GETHG2FUNCFIELD(_dt, _name, _field)                             \
        GETHG2FUNCEXPR(_dt, _name, h->ppd_overlay1._field)

#if defined(LE_HOST)
#define GETHGFUNCUNITEXPR(_dt, _name, _expr)                           \
        static uint32 dcb##_dt##_##_name##_get(int unit, dcb_t *dcb) {  \
                dcb##_dt##_t *d = (dcb##_dt##_t *)dcb;                  \
                uint32  hgh[3];                                         \
                soc_higig_hdr_t *h = (soc_higig_hdr_t *)&hgh[0];      \
                COMPILER_REFERENCE(unit);                               \
                hgh[0] = soc_htonl(d->mh0);                             \
                hgh[1] = soc_htonl(d->mh1);                             \
                hgh[2] = soc_htonl(d->mh2);                             \
                return _expr;                                           \
        }
#else
#define GETHGFUNCUNITEXPR(_dt, _name, _expr)                           \
        static uint32 dcb##_dt##_##_name##_get(int unit, dcb_t *dcb) {  \
                dcb##_dt##_t *d = (dcb##_dt##_t *)dcb;                  \
                soc_higig_hdr_t *h = (soc_higig_hdr_t *)&d->mh0;      \
                COMPILER_REFERENCE(unit);                               \
                return _expr;                                           \
        }
#endif

#if defined(LE_HOST)
#define GETHG2FUNCUNITEXPR(_dt, _name, _expr)                           \
        static uint32 dcb##_dt##_##_name##_get(int unit, dcb_t *dcb) {  \
                dcb##_dt##_t *d = (dcb##_dt##_t *)dcb;                  \
                uint32  hgh[4];                                         \
                soc_higig2_hdr_t *h = (soc_higig2_hdr_t *)&hgh[0];      \
                COMPILER_REFERENCE(unit);                               \
                hgh[0] = soc_htonl(d->mh0);                             \
                hgh[1] = soc_htonl(d->mh1);                             \
                hgh[2] = soc_htonl(d->mh2);                             \
                hgh[3] = soc_htonl(d->mh3);                             \
                return _expr;                                           \
        }
#else
#define GETHG2FUNCUNITEXPR(_dt, _name, _expr)                           \
        static uint32 dcb##_dt##_##_name##_get(int unit, dcb_t *dcb) {  \
                dcb##_dt##_t *d = (dcb##_dt##_t *)dcb;                  \
                soc_higig2_hdr_t *h = (soc_higig2_hdr_t *)&d->mh0;      \
                COMPILER_REFERENCE(unit);                               \
                return _expr;                                           \
        }
#endif

/*
 * This is a standard function used to generate a debug message whenever 
 * the code tries to access a field not present in the specific DCB
 */
static void
dcb0_funcerr(int dt, char *name)
{
    soc_cm_debug(DK_ERR, "ERROR: dcb%d_%s called\n", dt, name);
}

/* the addr related functions are the same for all dcb types */
static void
dcb0_addr_set(int unit, dcb_t *dcb, sal_vaddr_t addr)
{
    uint32      *d = (uint32 *)dcb;

    if (addr == 0) {
        *d = 0;
    } else {
        *d = soc_cm_l2p(unit, (void *)addr);
    }
}

static sal_vaddr_t
dcb0_addr_get(int unit, dcb_t *dcb)
{
    uint32      *d = (uint32 *)dcb;

    if (*d == 0) {
        return (sal_vaddr_t)0;
    } else {
        return (sal_vaddr_t)soc_cm_p2l(unit, *d);
    }
}

static sal_paddr_t
dcb0_paddr_get(dcb_t *dcb)
{
    uint32      *d = (uint32 *)dcb;

    return (sal_paddr_t)*d;
}

#if     defined(BCM_HERCULES_SUPPORT)   || defined(BCM_DRACO1_SUPPORT)  || \
        defined(BCM_LYNX_SUPPORT)       || defined(BCM_TUCANA_SUPPORT)  || \
        defined(BCM_DRACO15_SUPPORT)
SETFUNCERR(0, hg, uint32)
GETFUNCERR(0, hg)
SETFUNCERR(0, stat, uint32)
GETFUNCERR(0, stat)
SETFUNCERR(0, purge, uint32)
GETFUNCERR(0, purge)
GETPTRERR(0, mhp)
GETFUNCERR(0, outer_vid)
GETFUNCERR(0, outer_pri)
GETFUNCERR(0, outer_cfi)
GETFUNCERR(0, inner_vid)
GETFUNCERR(0, inner_pri)
GETFUNCERR(0, inner_cfi)
GETFUNCNULL(0, rx_bpdu)
GETFUNCNULL(0, rx_egr_to_cpu)
#endif

/*
 * Function:
 *      soc_dcb_rx_reasons_get
 * Purpose:
 *      Map the hardware reason bits from 'dcb' into the set
 *      of "reasons"
 * Parameters:
 *      unit - device
 *      dcb  - dma control block
 *      reasons - set of "reasons", socRxReason*
 */
void        
soc_dcb_rx_reasons_get(int unit, dcb_t *dcb, soc_rx_reasons_t *reasons)
{
    soc_rx_reason_t *map = SOC_DCB(unit).rx_reason_map;
    uint32 reason = SOC_DCB_RX_REASON_GET(unit, dcb);
    uint32 mask = 1;
    int i;

    if (SOC_DCB_RX_EGR_TO_CPU_GET(unit, dcb) && 
        NULL != SOC_DCB(unit).rx_egr_reason_map) {
        map = SOC_DCB(unit).rx_egr_reason_map;
    }
    SOC_RX_REASON_CLEAR_ALL(*reasons);
    for (i=0; i < 32; i++) {
        if ((mask & reason)) {
            SOC_RX_REASON_SET(*reasons, map[i]);
        }
        mask <<= 1;
    }

    if (soc_feature(unit, soc_feature_dcb_reason_hi)) {
        reason = SOC_DCB_RX_REASON_HI_GET(unit, dcb);
        mask = 1;

        for (i=0; i < 32; i++) {
            if ((mask & reason)) {
                SOC_RX_REASON_SET(*reasons, map[i + 32]);
            }
            mask <<= 1;
        }
    }

    /* BPDU bit should be a reason, paste it in here */
    if (SOC_DCB_RX_BPDU_GET(unit, dcb)) {
        SOC_RX_REASON_SET(*reasons, socRxReasonBpdu);
    }
}

#ifdef  BROADCOM_DEBUG

static char *_dcb_reason_names[socRxReasonCount] = SOC_RX_REASON_NAMES_INITIALIZER;

static void
dcb0_reason_dump(int unit, dcb_t *dcb, char *prefix)
{
    soc_rx_reason_t *map = SOC_DCB(unit).rx_reason_map;
    uint32 reason = SOC_DCB_RX_REASON_GET(unit, dcb);
    uint32 mask = 1;
    int i;
    
    for (i=0; i < 32; i++) {
        if ((mask & reason)) {
            soc_cm_print("%s\treason bit %d: %s\n", prefix, i, 
                         _dcb_reason_names[map[i]]);
        }
        mask <<= 1;
    }
    
    if (soc_feature(unit, soc_feature_dcb_reason_hi)) {
        reason = SOC_DCB_RX_REASON_HI_GET(unit, dcb);
        mask = 1;
    
        for (i=0; i < 32; i++) {
            if ((mask & reason)) {
                soc_cm_print("%s\treason bit %d: %s\n", prefix, i + 32,
                             _dcb_reason_names[map[i + 32]]);
            }
            mask <<= 1;
        }
    }

}


#if     defined(BCM_HERCULES_SUPPORT)   || defined(BCM_DRACO1_SUPPORT)  || \
        defined(BCM_LYNX_SUPPORT)       || defined(BCM_TUCANA_SUPPORT)  || \
        defined(BCM_DRACO15_SUPPORT)
static void
dcb0_dump(int unit, dcb_t *dcb, char *prefix, int tx)
{
    uint32      *p;
    int         i, size;
    char        ps[((DCB_MAX_SIZE/sizeof(uint32))*9)+1];

    p = (uint32 *)dcb;
    size = SOC_DCB_SIZE(unit) / sizeof(uint32);
    for (i = 0; i < size; i++) {
        sal_sprintf(&ps[i*9], "%08x ", p[i]);
    }
    soc_cm_print("%s\t%s\n", prefix, ps);
    soc_cm_print(
        "%s\ttype %d %sdone %ssg %schain %sreload\n",
        prefix,
        SOC_DCB_TYPE(unit),
        SOC_DCB_DONE_GET(unit, dcb) ? "" : "!",
        SOC_DCB_SG_GET(unit, dcb) ? "" : "!",
        SOC_DCB_CHAIN_GET(unit, dcb) ? "" : "!",
        SOC_DCB_RELOAD_GET(unit, dcb) ? "" : "!");
    dcb0_reason_dump(unit, dcb, prefix);
    soc_cm_print(
        "%s\taddr %p reqcount %d xfercount %d\n",
        prefix,
        (void *)SOC_DCB_ADDR_GET(unit, dcb),
        SOC_DCB_REQCOUNT_GET(unit, dcb),
        SOC_DCB_XFERCOUNT_GET(unit, dcb));

    if (tx) {
        soc_cm_print(
            "%s\tl2pbm %x utbpm %x l3bpm %x %scrc cos %d\n",
            prefix,
            SOC_DCB_TX_L2PBM_GET(unit, dcb),
            SOC_DCB_TX_UTPBM_GET(unit, dcb),
            SOC_DCB_TX_L3PBM_GET(unit, dcb),
            SOC_DCB_TX_CRC_GET(unit, dcb) ? "" : "!",
            SOC_DCB_TX_COS_GET(unit, dcb));

        if (SOC_IS_XGS(unit)) {
            if (SOC_IS_XGS_SWITCH(unit)) {
                soc_cm_print(
                    "%s\tdest-modport %d.%d opcode %d\n",
                    prefix,
                    SOC_DCB_TX_DESTMOD_GET(unit, dcb),
                    SOC_DCB_TX_DESTPORT_GET(unit, dcb),
                    SOC_DCB_TX_OPCODE_GET(unit, dcb));
            }
            if (SOC_IS_DRACO15(unit)) {
#ifdef BCM_DRACO15_SUPPORT
                soc_cm_print(
                    "%s\tsrc-modport %d.%d prio %d pfm %d\n",
                    prefix,
                    SOC_DCB_TX_SRCMOD_GET(unit, dcb),
                    SOC_DCB_TX_SRCPORT_GET(unit, dcb),
                    SOC_DCB_TX_PRIO_GET(unit, dcb),
                    SOC_DCB_TX_PFM_GET(unit, dcb));
#endif /* BCM_DRACO15_SUPPORT */
            }
        }
    } else {
        if (SOC_IS_TUCANA(unit)) {
#ifdef  BCM_TUCANA_SUPPORT
            soc_cm_print(
                "%s\t%sstart %send %serror %scrc cos %d [portcos %d] ingport %d\n",
                prefix,
                SOC_DCB_RX_START_GET(unit, dcb) ? "" : "!",
                SOC_DCB_RX_END_GET(unit, dcb) ? "" : "!",
                SOC_DCB_RX_ERROR_GET(unit, dcb) ? "" : "!",
                SOC_DCB_RX_CRC_GET(unit, dcb) ? "" : "!",
                SOC_DCB_RX_COS_GET(unit, dcb),
                ((dcb6_t *)dcb)->s4cos, /* Ugly Tucana only field */
                SOC_DCB_RX_INGPORT_GET(unit, dcb));
#endif /* BCM_TUCANA_SUPPORT */
        } else {
            soc_cm_print(
                "%s\t%sstart %send %serror %scrc cos %d ingport %d reason %#x\n",
                prefix,
                SOC_DCB_RX_START_GET(unit, dcb) ? "" : "!",
                SOC_DCB_RX_END_GET(unit, dcb) ? "" : "!",
                SOC_DCB_RX_ERROR_GET(unit, dcb) ? "" : "!",
                SOC_DCB_RX_CRC_GET(unit, dcb) ? "" : "!",
                SOC_DCB_RX_COS_GET(unit, dcb),
                SOC_DCB_RX_INGPORT_GET(unit, dcb),
                SOC_DCB_RX_REASON_GET(unit, dcb));
        }
        if (SOC_IS_XGS(unit)) {
            soc_cm_print(
                "%s\tsrc-modport %d.%d dest-modport %d.%d\n",
                prefix,
                SOC_DCB_RX_SRCMOD_GET(unit, dcb),
                SOC_DCB_RX_SRCPORT_GET(unit, dcb),
                SOC_DCB_RX_DESTMOD_GET(unit, dcb),
                SOC_DCB_RX_DESTPORT_GET(unit, dcb));
            soc_cm_print(
                "%s\thg-opcode %d prio %d mcast %d\n",
                prefix,
                SOC_DCB_RX_OPCODE_GET(unit, dcb),
                SOC_DCB_RX_PRIO_GET(unit, dcb),
                SOC_DCB_RX_MCAST_GET(unit, dcb));
        }
    }
}
#endif
#endif  /* BROADCOM_DEBUG */

#if defined(BCM_HERCULES_SUPPORT)
/*
 * DCB Type 2 Support
 */

static soc_rx_reason_t
dcb2_rx_reason_map[] = {
    socRxReasonFilterMatch,         /* Offset 0 */
    socRxReasonCpuLearn,            /* Offset 1 */
    socRxReasonSourceRoute,         /* Offset 2 */
    socRxReasonDestLookupFail,      /* Offset 3 */
    socRxReasonControl,             /* Offset 4 */
    socRxReasonIp,                  /* Offset 5 */
    socRxReasonInvalid,             /* Offset 6 */
    socRxReasonIpOptionVersion,     /* Offset 7 */
    socRxReasonIpmc,                /* Offset 8 */
    socRxReasonTtl,                 /* Offset 9 */
    socRxReasonBroadcast,           /* Offset 10 */
    socRxReasonMulticast,           /* Offset 11 */
    socRxReasonIgmp,                /* Offset 12 */
    socRxReasonInvalid,             /* Offset 13 */
    socRxReasonInvalid,             /* Offset 14 */
    socRxReasonInvalid,             /* Offset 15 */
    socRxReasonInvalid,             /* Offset 16 */
    socRxReasonInvalid,             /* Offset 17 */
    socRxReasonInvalid,             /* Offset 18 */
    socRxReasonInvalid,             /* Offset 19 */
    socRxReasonInvalid,             /* Offset 20 */
    socRxReasonInvalid,             /* Offset 21 */
    socRxReasonInvalid,             /* Offset 22 */
    socRxReasonInvalid,             /* Offset 23 */
    socRxReasonInvalid,             /* Offset 24 */
    socRxReasonInvalid,             /* Offset 25 */
    socRxReasonInvalid,             /* Offset 26 */
    socRxReasonInvalid,             /* Offset 27 */
    socRxReasonInvalid,             /* Offset 28 */
    socRxReasonInvalid,             /* Offset 29 */
    socRxReasonInvalid,             /* Offset 30 */
    socRxReasonInvalid,             /* Offset 31 */
};

static void
dcb2_init(dcb_t *dcb)
{
    uint32      *d = (uint32 *)dcb;

    d[0] = d[1] = d[2] = d[3] = d[4] = d[5] = d[6] = d[7] = 0;
}

static int
dcb2_addtx(dv_t *dv, sal_vaddr_t addr, uint32 count,
           pbmp_t l2pbm, pbmp_t utpbm, pbmp_t l3pbm, uint32 flags, uint32 *hgh)
{
    dcb2_t      *d;
    uint32      *di;
    uint32      paddr;

    d = (dcb2_t *)SOC_DCB_IDX2PTR(dv->dv_unit, dv->dv_dcb, dv->dv_vcnt);

    if (addr) {
        paddr = soc_cm_l2p(dv->dv_unit, (void *)addr);
    } else {
        paddr = 0;
    }

    if (dv->dv_vcnt > 0 && (dv->dv_flags & DV_F_COMBINE_DCB) &&
        (d[-1].c_sg != 0) &&
        (d[-1].addr + d[-1].c_count) == paddr &&
        d[-1].c_count + count <= DCB_MAX_REQCOUNT) {
        d[-1].c_count += count;
        return dv->dv_cnt - dv->dv_vcnt;
    }

    if (dv->dv_vcnt >= dv->dv_cnt) {
        return SOC_E_FULL;
    }
    if (dv->dv_vcnt > 0) {      /* chain off previous dcb */
        d[-1].c_chain = 1;
    }

    di = (uint32 *)d;
    di[0] = di[1] = di[2] = di[3] = di[4] = di[5] = di[6] = di[7] = 0;

    d->addr = paddr;
    d->c_count = count;
    d->c_sg = 1;
    d->c_cos = SOC_DMA_COS_GET(flags);
    d->c_crc = SOC_DMA_CRC_GET(flags) ?
                DCB_STRATA_CRC_REGEN : DCB_STRATA_CRC_LEAVE;
    d->l2ports = SOC_PBMP_WORD_GET(l2pbm, 0);
    d->utports = SOC_PBMP_WORD_GET(utpbm, 0);
    d->l3ports = SOC_PBMP_WORD_GET(l3pbm, 0);

    dv->dv_vcnt += 1;
    return dv->dv_cnt - dv->dv_vcnt;
}

static int
dcb2_addrx(dv_t *dv, sal_vaddr_t addr, uint32 count, uint32 flags)
{
    dcb2_t      *d;
    uint32      *di;

    d = (dcb2_t *)SOC_DCB_IDX2PTR(dv->dv_unit, dv->dv_dcb, dv->dv_vcnt);

    if (dv->dv_vcnt > 0) {      /* chain off previous dcb */
        d[-1].c_chain = 1;
    }

    di = (uint32 *)d;
    di[0] = di[1] = di[2] = di[3] = di[4] = di[5] = di[6] = di[7] = 0;

    if (addr) {
        d->addr = soc_cm_l2p(dv->dv_unit, (void *)addr);
    }
    d->c_count = count;
    d->c_sg = 1;
    d->c_cos = SOC_DMA_COS_GET(flags);
    d->c_crc = SOC_DMA_CRC_GET(flags) ?
                DCB_STRATA_CRC_REGEN : DCB_STRATA_CRC_LEAVE;

    dv->dv_vcnt += 1;
    return dv->dv_cnt - dv->dv_vcnt;
}

static uint32
dcb2_intrinfo(int unit, dcb_t *dcb, int tx, uint32 *count)
{
    dcb2_t      *d = (dcb2_t *)dcb;
    uint32      f;

    if (!d->s2valid) {
        return 0;
    }
    f = SOC_DCB_INFO_DONE;
    if (tx) {
        if (!d->c_sg) {
            f |= SOC_DCB_INFO_PKTEND;
        }
    } else {
        if (d->end) {
            f |= SOC_DCB_INFO_PKTEND;
        }
    }
    *count = d->count;
    return f;
}

SETFUNCFIELD(2, reqcount, c_count, uint32 count, count)
GETFUNCFIELD(2, reqcount, c_count)
GETFUNCFIELD(2, xfercount, count)
SETFUNCFIELD(2, done, s2valid, int val, val ? 1 : 0)
GETFUNCFIELD(2, done, s2valid)
SETFUNCFIELD(2, sg, c_sg, int val, val ? 1 : 0)
GETFUNCFIELD(2, sg, c_sg)
SETFUNCFIELD(2, chain, c_chain, int val, val ? 1 : 0)
GETFUNCFIELD(2, chain, c_chain)
SETFUNCFIELD(2, reload, c_reload, int val, val ? 1 : 0)
GETFUNCFIELD(2, reload, c_reload)
SETFUNCFIELD(2, tx_l2pbm, l2ports, pbmp_t pbm, SOC_PBMP_WORD_GET(pbm, 0))
SETFUNCFIELD(2, tx_utpbm, utports, pbmp_t pbm, SOC_PBMP_WORD_GET(pbm, 0))
SETFUNCFIELD(2, tx_l3pbm, l3ports, pbmp_t pbm, SOC_PBMP_WORD_GET(pbm, 0))
SETFUNCFIELD(2, tx_crc, c_crc, int val,
             val ? DCB_STRATA_CRC_REGEN : DCB_STRATA_CRC_LEAVE)
SETFUNCFIELD(2, tx_cos, c_cos, int val, val)
SETFUNCERR(2, tx_destmod, uint32)
SETFUNCERR(2, tx_destport, uint32)
SETFUNCERR(2, tx_opcode, uint32)
SETFUNCERR(2, tx_srcmod, uint32)
SETFUNCERR(2, tx_srcport, uint32)
SETFUNCERR(2, tx_prio, uint32)
SETFUNCERR(2, tx_pfm, uint32)
GETFUNCUNITFIELD(2, rx_untagged, untagged)
GETFUNCEXPR(2, rx_crc, d->crc == DCB_STRATA_CRC_REGEN)
GETFUNCFIELD(2, rx_cos, cos)
GETFUNCERR(2, rx_destmod)
GETFUNCERR(2, rx_destport)
GETFUNCERR(2, rx_opcode)
GETFUNCERR(2, rx_classtag)
GETFUNCFIELD(2, rx_matchrule, match_rule)
GETFUNCFIELD(2, rx_start, start)
GETFUNCFIELD(2, rx_end, end)
GETFUNCEXPR(2, rx_error, d->optim == DCB_STRATA_OPTIM_PURGE)
GETFUNCERR(2, rx_prio)
GETFUNCFIELD(2, rx_reason, reason)
GETFUNCERR(2, rx_reason_hi)
GETFUNCFIELD(2, rx_ingport, srcport)
GETFUNCFIELD(2, rx_srcport, srcport)
GETFUNCERR(2, rx_srcmod)
GETFUNCERR(2, rx_mcast)
GETFUNCERR(2, rx_vclabel)
GETFUNCERR(2, rx_mirror)
GETFUNCERR(2, rx_timestamp)
GETFUNCERR(2, rx_timestamp_upper)

#ifdef BROADCOM_DEBUG
GETFUNCFIELD(2, tx_l2pbm, l2ports)
GETFUNCFIELD(2, tx_utpbm, utports)
GETFUNCFIELD(2, tx_l3pbm, l3ports)
GETFUNCFIELD(2, tx_crc, c_crc)
GETFUNCFIELD(2, tx_cos, c_cos)
GETFUNCERR(2, tx_destmod)
GETFUNCERR(2, tx_destport)
GETFUNCERR(2, tx_opcode)
GETFUNCERR(2, tx_srcmod)
GETFUNCERR(2, tx_srcport)
GETFUNCERR(2, tx_prio)
GETFUNCERR(2, tx_pfm)
#endif /* BROADCOM_DEBUG */

#ifdef  PLISIM          /* these routines are only used by pcid */
static void dcb2_status_init(dcb_t *dcb)
{
    uint32      *d = (uint32 *)dcb;

    d[5] = d[6] = d[7] = 0;
}
SETFUNCFIELD(2, xfercount, count, uint32 count, count)
SETFUNCFIELD(2, rx_start, start, int val, val ? 1 : 0)
SETFUNCFIELD(2, rx_end, end, int val, val ? 1 : 0)
SETFUNCFIELD(2, rx_error, optim, int val, val ? DCB_STRATA_OPTIM_PURGE : 0)
SETFUNCFIELD(2, rx_crc, crc, int val, val ? DCB_STRATA_CRC_REGEN : 0)
#endif  /* PLISIM */

dcb_op_t dcb2_op = {
    2,
    sizeof(dcb2_t),
    dcb2_rx_reason_map,
    NULL,
    dcb2_init,
    dcb2_addtx,
    dcb2_addrx,
    dcb2_intrinfo,
    dcb2_reqcount_set,
    dcb2_reqcount_get,
    dcb2_xfercount_get,
    dcb0_addr_set,
    dcb0_addr_get,
    dcb0_paddr_get,
    dcb2_done_set,
    dcb2_done_get,
    dcb2_sg_set,
    dcb2_sg_get,
    dcb2_chain_set,
    dcb2_chain_get,
    dcb2_reload_set,
    dcb2_reload_get,
    dcb2_tx_l2pbm_set,
    dcb2_tx_utpbm_set,
    dcb2_tx_l3pbm_set,
    dcb2_tx_crc_set,
    dcb2_tx_cos_set,
    dcb2_tx_destmod_set,
    dcb2_tx_destport_set,
    dcb2_tx_opcode_set,
    dcb2_tx_srcmod_set,
    dcb2_tx_srcport_set,
    dcb2_tx_prio_set,
    dcb2_tx_pfm_set,
    dcb2_rx_untagged_get,
    dcb2_rx_crc_get,
    dcb2_rx_cos_get,
    dcb2_rx_destmod_get,
    dcb2_rx_destport_get,
    dcb2_rx_opcode_get,
    dcb2_rx_classtag_get,
    dcb2_rx_matchrule_get,
    dcb2_rx_start_get,
    dcb2_rx_end_get,
    dcb2_rx_error_get,
    dcb2_rx_prio_get,
    dcb2_rx_reason_get,
    dcb2_rx_reason_hi_get,
    dcb2_rx_ingport_get,
    dcb2_rx_srcport_get,
    dcb2_rx_srcmod_get,
    dcb2_rx_mcast_get,
    dcb2_rx_vclabel_get,
    dcb2_rx_mirror_get,
    dcb2_rx_timestamp_get,
    dcb2_rx_timestamp_upper_get,
    dcb0_hg_set,
    dcb0_hg_get,
    dcb0_stat_set,
    dcb0_stat_get,
    dcb0_purge_set,
    dcb0_purge_get,
    dcb0_mhp_get,
    dcb0_outer_vid_get,
    dcb0_outer_pri_get,
    dcb0_outer_cfi_get,
    dcb0_inner_vid_get,
    dcb0_inner_pri_get,
    dcb0_inner_cfi_get,
    dcb0_rx_bpdu_get,
    dcb0_rx_egr_to_cpu_get,
#ifdef  BROADCOM_DEBUG
    dcb2_tx_l2pbm_get,
    dcb2_tx_utpbm_get,
    dcb2_tx_l3pbm_get,
    dcb2_tx_crc_get,
    dcb2_tx_cos_get,
    dcb2_tx_destmod_get,
    dcb2_tx_destport_get,
    dcb2_tx_opcode_get,
    dcb2_tx_srcmod_get,
    dcb2_tx_srcport_get,
    dcb2_tx_prio_get,
    dcb2_tx_pfm_get,

    dcb0_dump,
    dcb0_reason_dump,
#endif /* BROADCOM_DEBUG */
#ifdef  PLISIM
    dcb2_status_init,
    dcb2_xfercount_set,
    dcb2_rx_start_set,
    dcb2_rx_end_set,
    dcb2_rx_error_set,
    dcb2_rx_crc_set,
    NULL,
#endif
};

#endif  /* BCM_HERCULES_SUPPORT */

#if defined(BCM_DRACO1_SUPPORT) || defined(BCM_HERCULES_SUPPORT) || \
    defined(BCM_LYNX_SUPPORT) || defined(BCM_DRACO15_SUPPORT)
static soc_rx_reason_t
dcb3_rx_reason_map[] = {
    socRxReasonUnknownVlan,        /* Offset 0 */
    socRxReasonL2SourceMiss,       /* Offset 1 */
    socRxReasonL2DestMiss,         /* Offset 2 */
    socRxReasonL2Move,             /* Offset 3 */
    socRxReasonL2Cpu,              /* Offset 4 */
    socRxReasonInvalid,            /* Offset 5 */
    socRxReasonInvalid,            /* Offset 6 */
    socRxReasonL3SourceMiss,       /* Offset 7 */
    socRxReasonL3DestMiss,         /* Offset 8 */
    socRxReasonL3SourceMove,       /* Offset 9 */
    socRxReasonMcastMiss,          /* Offset 10 */
    socRxReasonIpMcastMiss,        /* Offset 11 */
    socRxReasonFilterMatch,        /* Offset 12 */
    socRxReasonL3HeaderError,      /* Offset 13 */
    socRxReasonProtocol,           /* Offset 14 */
    socRxReasonInvalid,            /* Offset 15 */
    socRxReasonInvalid,            /* Offset 16 */
    socRxReasonInvalid,            /* Offset 17 */
    socRxReasonInvalid,            /* Offset 18 */
    socRxReasonInvalid,            /* Offset 19 */
    socRxReasonInvalid,            /* Offset 20 */
    socRxReasonInvalid,            /* Offset 21 */
    socRxReasonInvalid,            /* Offset 22 */
    socRxReasonInvalid,            /* Offset 23 */
    socRxReasonInvalid,            /* Offset 24 */
    socRxReasonInvalid,            /* Offset 25 */
    socRxReasonInvalid,            /* Offset 26 */
    socRxReasonInvalid,            /* Offset 27 */
    socRxReasonInvalid,            /* Offset 28 */
    socRxReasonInvalid,            /* Offset 29 */
    socRxReasonInvalid,            /* Offset 30 */
    socRxReasonInvalid,            /* Offset 31 */
};
#endif

#ifdef  BCM_DRACO1_SUPPORT
/*
 * DCB Type 3 Support
 */

static void
dcb3_init(dcb_t *dcb)
{
    uint32      *d = (uint32 *)dcb;

    d[0] = d[1] = d[2] = d[3] = d[4] = d[5] = d[6] = d[7] = 0;
}

static int
dcb3_addtx(dv_t *dv, sal_vaddr_t addr, uint32 count,
           pbmp_t l2pbm, pbmp_t utpbm, pbmp_t l3pbm, uint32 flags, uint32 *hgh)
{
    dcb3_t      *d;
    uint32      *di;
    uint32      paddr, val;

    d = (dcb3_t *)SOC_DCB_IDX2PTR(dv->dv_unit, dv->dv_dcb, dv->dv_vcnt);

    if (addr) {
        paddr = soc_cm_l2p(dv->dv_unit, (void *)addr);
    } else {
        paddr = 0;
    }

    if (dv->dv_vcnt > 0 && (dv->dv_flags & DV_F_COMBINE_DCB) &&
        (d[-1].c_sg != 0) &&
        (d[-1].addr + d[-1].c_count) == paddr &&
        d[-1].c_count + count <= DCB_MAX_REQCOUNT) {
        d[-1].c_count += count;
        return dv->dv_cnt - dv->dv_vcnt;
    }

    if (dv->dv_vcnt >= dv->dv_cnt) {
        return SOC_E_FULL;
    }
    if (dv->dv_vcnt > 0) {      /* chain off previous dcb */
        d[-1].c_chain = 1;
    }

    di = (uint32 *)d;
    di[0] = di[1] = di[2] = di[3] = di[4] = di[5] = di[6] = di[7] = 0;

    d->addr = paddr;
    d->c_count = count;
    d->c_sg = 1;
    d->c_cos = SOC_DMA_COS_GET(flags);
    d->c_crc = SOC_DMA_CRC_GET(flags) ? DCB_XGS_CRC_REGEN : DCB_XGS_CRC_LEAVE;
    d->l2ports = SOC_PBMP_WORD_GET(l2pbm, 0);
    d->utports = SOC_PBMP_WORD_GET(utpbm, 0);
    d->l3ports = SOC_PBMP_WORD_GET(l3pbm, 0);

    /* tx_destmod_set, tx_destport_set, tx_opcode_set */
    d->c_destmod = SOC_DMA_DMOD_GET(flags);
    val = SOC_DMA_DPORT_GET(flags);
    d->c_destport_lo = val;
    d->c_destport_hi = val>>4;
    val = SOC_DMA_MHOP_GET(flags);
    d->c_opcode0 = val & 1;
    d->c_opcode1 = (val>>1) & 1;
    d->c_opcode2 = (val>>2) & 1;

    dv->dv_vcnt += 1;
    return dv->dv_cnt - dv->dv_vcnt;
}

static int
dcb3_addrx(dv_t *dv, sal_vaddr_t addr, uint32 count, uint32 flags)
{
    dcb3_t      *d;
    uint32      *di;

    d = (dcb3_t *)SOC_DCB_IDX2PTR(dv->dv_unit, dv->dv_dcb, dv->dv_vcnt);

    if (dv->dv_vcnt > 0) {      /* chain off previous dcb */
        d[-1].c_chain = 1;
    }

    di = (uint32 *)d;
    di[0] = di[1] = di[2] = di[3] = di[4] = di[5] = di[6] = di[7] = 0;

    if (addr) {
        d->addr = soc_cm_l2p(dv->dv_unit, (void *)addr);
    }
    d->c_count = count;
    d->c_sg = 1;
    d->c_cos = SOC_DMA_COS_GET(flags);
    d->c_crc = SOC_DMA_CRC_GET(flags) ? DCB_XGS_CRC_REGEN : DCB_XGS_CRC_LEAVE;

    dv->dv_vcnt += 1;
    return dv->dv_cnt - dv->dv_vcnt;
}

static uint32
dcb3_intrinfo(int unit, dcb_t *dcb, int tx, uint32 *count)
{
    dcb3_t      *d = (dcb3_t *)dcb;
    uint32      f;

    if (!d->s2valid) {
        return 0;
    }
    f = SOC_DCB_INFO_DONE;
    if (tx) {
        if (!d->c_sg) {
            f |= SOC_DCB_INFO_PKTEND;
        }
    } else {
        if (d->end) {
            f |= SOC_DCB_INFO_PKTEND;
        }
    }
    *count = d->count;
    return f;
}

SETFUNCFIELD(3, reqcount, c_count, uint32 count, count)
GETFUNCFIELD(3, reqcount, c_count)
GETFUNCFIELD(3, xfercount, count)
SETFUNCFIELD(3, done, s2valid, int val, val ? 1 : 0)
GETFUNCFIELD(3, done, s2valid)
SETFUNCFIELD(3, sg, c_sg, int val, val ? 1 : 0)
GETFUNCFIELD(3, sg, c_sg)
SETFUNCFIELD(3, chain, c_chain, int val, val ? 1 : 0)
GETFUNCFIELD(3, chain, c_chain)
SETFUNCFIELD(3, reload, c_reload, int val, val ? 1 : 0)
GETFUNCFIELD(3, reload, c_reload)
SETFUNCFIELD(3, tx_l2pbm, l2ports, pbmp_t pbm, SOC_PBMP_WORD_GET(pbm, 0))
SETFUNCFIELD(3, tx_utpbm, utports, pbmp_t pbm, SOC_PBMP_WORD_GET(pbm, 0))
SETFUNCFIELD(3, tx_l3pbm, l3ports, pbmp_t pbm, SOC_PBMP_WORD_GET(pbm, 0))
SETFUNCFIELD(3, tx_crc, c_crc, int val,
             val ? DCB_XGS_CRC_REGEN : DCB_XGS_CRC_LEAVE)
SETFUNCFIELD(3, tx_cos, c_cos, int val, val)
SETFUNCFIELD(3, tx_destmod, c_destmod, uint32 val, val)
SETFUNCEXPR(3, tx_destport, uint32 val,
        d->c_destport_lo = val; d->c_destport_hi = val>>4)
SETFUNCEXPR(3, tx_opcode, uint32 val,
        d->c_opcode0 = val&1; d->c_opcode1 = (val>>1)&1; d->c_opcode2 = (val>>2)&1)
SETFUNCERR(3, tx_srcmod, uint32)
SETFUNCERR(3, tx_srcport, uint32)
SETFUNCERR(3, tx_prio, uint32)
SETFUNCERR(3, tx_pfm, uint32)
GETFUNCUNITEXPR(3, rx_untagged, !d->tagged)
GETFUNCFIELD(3, rx_crc, crc)
GETFUNCFIELD(3, rx_cos, cos)
GETFUNCEXPR(3, rx_destmod, d->destmod_lo)
GETFUNCFIELD(3, rx_destport, destport)
GETFUNCFIELD(3, rx_opcode, opcode)
GETFUNCFIELD(3, rx_classtag, class_tag)
GETFUNCFIELD(3, rx_matchrule, match_rule)
GETFUNCFIELD(3, rx_start, start)
GETFUNCFIELD(3, rx_end, end)
GETFUNCFIELD(3, rx_error, error)
GETFUNCEXPR(3, rx_prio, ((d->prio_hi<<1) | d->prio_lo))
GETFUNCFIELD(3, rx_reason, reason)
GETFUNCERR(3, rx_reason_hi)
GETFUNCFIELD(3, rx_ingport, ingport)
GETFUNCEXPR(3, rx_srcport, ((d->srcport_hi<<5) | d->srcport_lo))
GETFUNCFIELD(3, rx_srcmod, srcmod)
GETFUNCEXPR(3, rx_mcast, ((d->destmod_lo<<5) | d->destport))
GETFUNCERR(3, rx_vclabel)
GETFUNCERR(3, rx_mirror)
GETFUNCERR(3, rx_timestamp)
GETFUNCERR(3, rx_timestamp_upper)
#ifdef BROADCOM_DEBUG
GETFUNCFIELD(3, tx_l2pbm, l2ports)
GETFUNCFIELD(3, tx_utpbm, utports)
GETFUNCFIELD(3, tx_l3pbm, l3ports)
GETFUNCFIELD(3, tx_crc, c_crc)
GETFUNCFIELD(3, tx_cos, c_cos)
GETFUNCFIELD(3, tx_destmod, c_destmod)
GETFUNCEXPR(3, tx_destport, ((d->c_destport_hi<<4) | d->c_destport_lo))
GETFUNCEXPR(3, tx_opcode, ((d->c_opcode2<<2) | (d->c_opcode1<<1) | d->c_opcode0))
GETFUNCERR(3, tx_srcmod)
GETFUNCERR(3, tx_srcport)
GETFUNCERR(3, tx_prio)
GETFUNCERR(3, tx_pfm)
#endif /* BROADCOM_DEBUG */

#ifdef  PLISIM          /* these routines are only used by pcid */
static void dcb3_status_init(dcb_t *dcb)
{
    uint32      *d = (uint32 *)dcb;

    d[5] = d[6] = d[7] = 0;
}
SETFUNCFIELD(3, xfercount, count, uint32 count, count)
SETFUNCFIELD(3, rx_start, start, int val, val ? 1 : 0)
SETFUNCFIELD(3, rx_end, end, int val, val ? 1 : 0)
SETFUNCFIELD(3, rx_error, error, int val, val ? 1 : 0)
SETFUNCFIELD(3, rx_crc, crc, int val, val ? 1 : 0)
#endif  /* PLISIM */

dcb_op_t dcb3_op = {
    3,
    sizeof(dcb3_t),
    dcb3_rx_reason_map,
    NULL,
    dcb3_init,
    dcb3_addtx,
    dcb3_addrx,
    dcb3_intrinfo,
    dcb3_reqcount_set,
    dcb3_reqcount_get,
    dcb3_xfercount_get,
    dcb0_addr_set,
    dcb0_addr_get,
    dcb0_paddr_get,
    dcb3_done_set,
    dcb3_done_get,
    dcb3_sg_set,
    dcb3_sg_get,
    dcb3_chain_set,
    dcb3_chain_get,
    dcb3_reload_set,
    dcb3_reload_get,
    dcb3_tx_l2pbm_set,
    dcb3_tx_utpbm_set,
    dcb3_tx_l3pbm_set,
    dcb3_tx_crc_set,
    dcb3_tx_cos_set,
    dcb3_tx_destmod_set,
    dcb3_tx_destport_set,
    dcb3_tx_opcode_set,
    dcb3_tx_srcmod_set,
    dcb3_tx_srcport_set,
    dcb3_tx_prio_set,
    dcb3_tx_pfm_set,
    dcb3_rx_untagged_get,
    dcb3_rx_crc_get,
    dcb3_rx_cos_get,
    dcb3_rx_destmod_get,
    dcb3_rx_destport_get,
    dcb3_rx_opcode_get,
    dcb3_rx_classtag_get,
    dcb3_rx_matchrule_get,
    dcb3_rx_start_get,
    dcb3_rx_end_get,
    dcb3_rx_error_get,
    dcb3_rx_prio_get,
    dcb3_rx_reason_get,
    dcb3_rx_reason_hi_get,
    dcb3_rx_ingport_get,
    dcb3_rx_srcport_get,
    dcb3_rx_srcmod_get,
    dcb3_rx_mcast_get,
    dcb3_rx_vclabel_get,
    dcb3_rx_mirror_get,
    dcb3_rx_timestamp_get,
    dcb3_rx_timestamp_upper_get,
    dcb0_hg_set,
    dcb0_hg_get,
    dcb0_stat_set,
    dcb0_stat_get,
    dcb0_purge_set,
    dcb0_purge_get,
    dcb0_mhp_get,
    dcb0_outer_vid_get,
    dcb0_outer_pri_get,
    dcb0_outer_cfi_get,
    dcb0_inner_vid_get,
    dcb0_inner_pri_get,
    dcb0_inner_cfi_get,
    dcb0_rx_bpdu_get,
    dcb0_rx_egr_to_cpu_get,
#ifdef  BROADCOM_DEBUG
    dcb3_tx_l2pbm_get,
    dcb3_tx_utpbm_get,
    dcb3_tx_l3pbm_get,
    dcb3_tx_crc_get,
    dcb3_tx_cos_get,
    dcb3_tx_destmod_get,
    dcb3_tx_destport_get,
    dcb3_tx_opcode_get,
    dcb3_tx_srcmod_get,
    dcb3_tx_srcport_get,
    dcb3_tx_prio_get,
    dcb3_tx_pfm_get,

    dcb0_dump,
    dcb0_reason_dump,
#endif /* BROADCOM_DEBUG */
#ifdef  PLISIM
    dcb3_status_init,
    dcb3_xfercount_set,
    dcb3_rx_start_set,
    dcb3_rx_end_set,
    dcb3_rx_error_set,
    dcb3_rx_crc_set,
    NULL,
#endif
};

#endif  /* BCM_DRACO1_SUPPORT */

#ifdef  BCM_HERCULES_SUPPORT
/*
 * DCB Type 4 Support
 * The trick here is that the tx related functions use the dcb2
 * variations because hercules has a split personality.  Type 2 to
 * transmit but type 4 to receive.  We take advantage of the fact both
 * types are the same length.
 */

static int
dcb4_addrx(dv_t *dv, sal_vaddr_t addr, uint32 count, uint32 flags)
{
    dcb4_t      *d;
    uint32      *di;

    d = (dcb4_t *)SOC_DCB_IDX2PTR(dv->dv_unit, dv->dv_dcb, dv->dv_vcnt);

    if (dv->dv_vcnt > 0) {      /* chain off previous dcb */
        d[-1].c_chain = 1;
    }

    di = (uint32 *)d;
    di[0] = di[1] = di[2] = di[3] = di[4] = di[5] = di[6] = di[7] = 0;

    if (addr) {
        d->addr = soc_cm_l2p(dv->dv_unit, (void *)addr);
    }
    d->c_count = count;
    d->c_sg = 1;
    d->c_cos = SOC_DMA_COS_GET(flags);
    d->c_crc = SOC_DMA_CRC_GET(flags) ? DCB_XGS_CRC_REGEN : DCB_XGS_CRC_LEAVE;

    dv->dv_vcnt += 1;
    return dv->dv_cnt - dv->dv_vcnt;
}

static uint32
dcb4_intrinfo(int unit, dcb_t *dcb, int tx, uint32 *count)
{
    dcb4_t      *d = (dcb4_t *)dcb;
    uint32      f;

    if (!d->s2valid) {
        return 0;
    }
    f = SOC_DCB_INFO_DONE;
    if (tx) {
        if (!d->c_sg) {
            f |= SOC_DCB_INFO_PKTEND;
        }
    } else {
        if (d->end) {
            f |= SOC_DCB_INFO_PKTEND;
        }
    }
    *count = d->count;
    return f;
}

GETFUNCUNITERR(4, rx_untagged)
GETFUNCFIELD(4, rx_crc, crc)
GETFUNCFIELD(4, rx_cos, cos)
GETFUNCEXPR(4, rx_destmod, d->destmod_lo)
GETFUNCFIELD(4, rx_destport, destport)
GETFUNCFIELD(4, rx_opcode, opcode)
GETFUNCFIELD(4, rx_classtag, class_tag)
GETFUNCFIELD(4, rx_matchrule, match_rule)
GETFUNCFIELD(4, rx_start, start)
GETFUNCFIELD(4, rx_end, end)
GETFUNCFIELD(4, rx_error, error)
GETFUNCEXPR(4, rx_prio, ((d->prio_hi<<1) | d->prio_lo))
GETFUNCFIELD(4, rx_reason, reason)
GETFUNCERR(4, rx_reason_hi)
GETFUNCFIELD(4, rx_ingport, ingport)
GETFUNCERR(4, rx_srcport)
GETFUNCFIELD(4, rx_srcmod, srcmod)
GETFUNCEXPR(4, rx_mcast, ((d->destmod_lo<<5) | d->destport))
GETFUNCERR(4, rx_vclabel)
GETFUNCERR(4, rx_mirror)
GETFUNCERR(4, rx_timestamp)
GETFUNCERR(4, rx_timestamp_upper)

#ifdef  PLISIM          /* these routines are only used by pcid */
static void dcb4_status_init(dcb_t *dcb)
{
    uint32      *d = (uint32 *)dcb;

    d[5] = d[6] = d[7] = 0;
}
SETFUNCFIELD(4, rx_start, start, int val, val ? 1 : 0)
SETFUNCFIELD(4, rx_end, end, int val, val ? 1 : 0)
SETFUNCFIELD(4, rx_error, error, int val, val ? 1 : 0)
SETFUNCFIELD(4, rx_crc, crc, int val, val ? 1 : 0)
#endif  /* PLISIM */

dcb_op_t dcb4_op = {
    4,
    sizeof(dcb4_t),
    dcb3_rx_reason_map,
    NULL,
    dcb2_init,
    dcb2_addtx,
    dcb4_addrx,
    dcb4_intrinfo,
    dcb2_reqcount_set,          /* 2 and 4 are the same */
    dcb2_reqcount_get,          /* 2 and 4 are the same */
    dcb2_xfercount_get,         /* 2 and 4 are the same */
    dcb0_addr_set,
    dcb0_addr_get,
    dcb0_paddr_get,
    dcb2_done_set,              /* 2 and 4 are the same */
    dcb2_done_get,              /* 2 and 4 are the same */
    dcb2_sg_set,                /* 2 and 4 are the same */
    dcb2_sg_get,                /* 2 and 4 are the same */
    dcb2_chain_set,             /* 2 and 4 are the same */
    dcb2_chain_get,             /* 2 and 4 are the same */
    dcb2_reload_set,            /* 2 and 4 are the same */
    dcb2_reload_get,            /* 2 and 4 are the same */
    dcb2_tx_l2pbm_set,
    dcb2_tx_utpbm_set,
    dcb2_tx_l3pbm_set,
    dcb2_tx_crc_set,
    dcb2_tx_cos_set,
    dcb2_tx_destmod_set,
    dcb2_tx_destport_set,
    dcb2_tx_opcode_set,
    dcb2_tx_srcmod_set,
    dcb2_tx_srcport_set,
    dcb2_tx_prio_set,
    dcb2_tx_pfm_set,
    dcb4_rx_untagged_get,
    dcb4_rx_crc_get,
    dcb4_rx_cos_get,
    dcb4_rx_destmod_get,
    dcb4_rx_destport_get,
    dcb4_rx_opcode_get,
    dcb4_rx_classtag_get,
    dcb4_rx_matchrule_get,
    dcb4_rx_start_get,
    dcb4_rx_end_get,
    dcb4_rx_error_get,
    dcb4_rx_prio_get,
    dcb4_rx_reason_get,
    dcb4_rx_reason_hi_get,
    dcb4_rx_ingport_get,
    dcb4_rx_srcport_get,
    dcb4_rx_srcmod_get,
    dcb4_rx_mcast_get,
    dcb4_rx_vclabel_get,
    dcb4_rx_mirror_get,
    dcb4_rx_timestamp_get,
    dcb4_rx_timestamp_upper_get,
    dcb0_hg_set,
    dcb0_hg_get,
    dcb0_stat_set,
    dcb0_stat_get,
    dcb0_purge_set,
    dcb0_purge_get,
    dcb0_mhp_get,
    dcb0_outer_vid_get,
    dcb0_outer_pri_get,
    dcb0_outer_cfi_get,
    dcb0_inner_vid_get,
    dcb0_inner_pri_get,
    dcb0_inner_cfi_get,
    dcb0_rx_bpdu_get,
    dcb0_rx_egr_to_cpu_get,
#ifdef  BROADCOM_DEBUG
    dcb2_tx_l2pbm_get,
    dcb2_tx_utpbm_get,
    dcb2_tx_l3pbm_get,
    dcb2_tx_crc_get,
    dcb2_tx_cos_get,
    dcb2_tx_destmod_get,
    dcb2_tx_destport_get,
    dcb2_tx_opcode_get,
    dcb2_tx_srcmod_get,
    dcb2_tx_srcport_get,
    dcb2_tx_prio_get,
    dcb2_tx_pfm_get,

    dcb0_dump,
    dcb0_reason_dump,
#endif /* BROADCOM_DEBUG */
#ifdef  PLISIM
    dcb4_status_init,
    dcb2_xfercount_set,         /* 2 and 4 are the same */
    dcb4_rx_start_set,
    dcb4_rx_end_set,
    dcb4_rx_error_set,
    dcb4_rx_crc_set,
    NULL,
#endif
};

#endif  /* BCM_HERCULES_SUPPORT */

#ifdef  BCM_LYNX_SUPPORT
/*
 * DCB Type 5 Support
 */

static void
dcb5_init(dcb_t *dcb)
{
    uint32      *d = (uint32 *)dcb;

    d[0] = d[1] = d[2] = d[3] = d[4] = d[5] = d[6] = d[7] = d[8] = 0;
}

static int
dcb5_addtx(dv_t *dv, sal_vaddr_t addr, uint32 count,
           pbmp_t l2pbm, pbmp_t utpbm, pbmp_t l3pbm, uint32 flags, uint32 *hgh)
{
    dcb5_t      *d;
    uint32      *di;
    uint32      paddr, val;

    d = (dcb5_t *)SOC_DCB_IDX2PTR(dv->dv_unit, dv->dv_dcb, dv->dv_vcnt);

    if (addr) {
        paddr = soc_cm_l2p(dv->dv_unit, (void *)addr);
    } else {
        paddr = 0;
    }

    if (dv->dv_vcnt > 0 && (dv->dv_flags & DV_F_COMBINE_DCB) &&
        (d[-1].c_sg != 0) &&
        (d[-1].addr + d[-1].c_count) == paddr &&
        d[-1].c_count + count <= DCB_MAX_REQCOUNT) {
        d[-1].c_count += count;
        return dv->dv_cnt - dv->dv_vcnt;
    }

    if (dv->dv_vcnt >= dv->dv_cnt) {
        return SOC_E_FULL;
    }
    if (dv->dv_vcnt > 0) {      /* chain off previous dcb */
        d[-1].c_chain = 1;
    }

    di = (uint32 *)d;
    di[0] = di[1] = di[2] = di[3] = di[4] = di[5] = di[6] = di[7] = di[8] = 0;

    d->addr = paddr;
    d->c_count = count;
    d->c_sg = 1;
    d->c_cos = SOC_DMA_COS_GET(flags);
    d->c_crc = SOC_DMA_CRC_GET(flags) ? DCB_XGS_CRC_REGEN : DCB_XGS_CRC_LEAVE;
    d->l2ports = SOC_PBMP_WORD_GET(l2pbm, 0);
    d->utports = SOC_PBMP_WORD_GET(utpbm, 0);
    d->l3ports = SOC_PBMP_WORD_GET(l3pbm, 0);

    /* tx_destmod_set, tx_destport_set, tx_opcode_set */
    d->c_destmod = SOC_DMA_DMOD_GET(flags);
    val = SOC_DMA_DPORT_GET(flags);
    d->c_destport_lo = val;
    d->c_destport_hi = val>>4;
    val = SOC_DMA_MHOP_GET(flags);
    d->c_opcode0 = val & 1;
    d->c_opcode1 = (val>>1) & 1;
    d->c_opcode2 = (val>>2) & 1;

    dv->dv_vcnt += 1;
    return dv->dv_cnt - dv->dv_vcnt;
}

static int
dcb5_addrx(dv_t *dv, sal_vaddr_t addr, uint32 count, uint32 flags)
{
    dcb5_t      *d;
    uint32      *di;

    d = (dcb5_t *)SOC_DCB_IDX2PTR(dv->dv_unit, dv->dv_dcb, dv->dv_vcnt);

    if (dv->dv_vcnt > 0) {      /* chain off previous dcb */
        d[-1].c_chain = 1;
    }

    di = (uint32 *)d;
    di[0] = di[1] = di[2] = di[3] = di[4] = di[5] = di[6] = di[7] = di[8] = 0;

    if (addr) {
        d->addr = soc_cm_l2p(dv->dv_unit, (void *)addr);
    }
    d->c_count = count;
    d->c_sg = 1;
    d->c_cos = SOC_DMA_COS_GET(flags);
    d->c_crc = SOC_DMA_CRC_GET(flags) ? DCB_XGS_CRC_REGEN : DCB_XGS_CRC_LEAVE;

    dv->dv_vcnt += 1;
    return dv->dv_cnt - dv->dv_vcnt;
}

static uint32
dcb5_intrinfo(int unit, dcb_t *dcb, int tx, uint32 *count)
{
    dcb5_t      *d = (dcb5_t *)dcb;
    uint32      f;

    if (!d->s3valid) {
        return 0;
    }
    f = SOC_DCB_INFO_DONE;
    if (tx) {
        if (!d->c_sg) {
            f |= SOC_DCB_INFO_PKTEND;
        }
    } else {
        if (d->end) {
            f |= SOC_DCB_INFO_PKTEND;
        }
    }
    *count = d->count;
    return f;
}

SETFUNCFIELD(5, reqcount, c_count, uint32 count, count)
GETFUNCFIELD(5, reqcount, c_count)
GETFUNCFIELD(5, xfercount, count)
SETFUNCFIELD(5, done, s3valid, int val, val ? 1 : 0)
GETFUNCFIELD(5, done, s3valid)
SETFUNCFIELD(5, sg, c_sg, int val, val ? 1 : 0)
GETFUNCFIELD(5, sg, c_sg)
SETFUNCFIELD(5, chain, c_chain, int val, val ? 1 : 0)
GETFUNCFIELD(5, chain, c_chain)
SETFUNCFIELD(5, reload, c_reload, int val, val ? 1 : 0)
GETFUNCFIELD(5, reload, c_reload)
SETFUNCFIELD(5, tx_l2pbm, l2ports, pbmp_t pbm, SOC_PBMP_WORD_GET(pbm, 0))
SETFUNCFIELD(5, tx_utpbm, utports, pbmp_t pbm, SOC_PBMP_WORD_GET(pbm, 0))
SETFUNCFIELD(5, tx_l3pbm, l3ports, pbmp_t pbm, SOC_PBMP_WORD_GET(pbm, 0))
SETFUNCFIELD(5, tx_crc, c_crc, int val,
             val ? DCB_XGS_CRC_REGEN : DCB_XGS_CRC_LEAVE)
SETFUNCFIELD(5, tx_cos, c_cos, int val, val)
SETFUNCFIELD(5, tx_destmod, c_destmod, uint32 val, val)
SETFUNCEXPR(5, tx_destport, uint32 val,
        d->c_destport_lo = val; d->c_destport_hi = val>>4)
SETFUNCEXPR(5, tx_opcode, uint32 val,
        d->c_opcode0 = val&1; d->c_opcode1 = (val>>1)&1; d->c_opcode2 = (val>>2)&1)
SETFUNCERR(5, tx_srcmod, uint32)
SETFUNCERR(5, tx_srcport, uint32)
SETFUNCERR(5, tx_prio, uint32)
SETFUNCERR(5, tx_pfm, uint32)
GETFUNCUNITEXPR(5, rx_untagged, !d->tagged)
GETFUNCFIELD(5, rx_crc, crc)
GETFUNCFIELD(5, rx_cos, cos)
GETFUNCEXPR(5, rx_destmod, d->destmod_lo)
GETFUNCFIELD(5, rx_destport, destport)
GETFUNCFIELD(5, rx_opcode, opcode)
GETFUNCFIELD(5, rx_classtag, class_tag)
GETFUNCFIELD(5, rx_matchrule, match_rule)
GETFUNCFIELD(5, rx_start, start)
GETFUNCFIELD(5, rx_end, end)
GETFUNCFIELD(5, rx_error, error)
GETFUNCEXPR(5, rx_prio, ((d->prio_hi<<1) | d->prio_lo))
GETFUNCFIELD(5, rx_reason, reason)
GETFUNCERR(5, rx_reason_hi)
GETFUNCFIELD(5, rx_ingport, ingport)
GETFUNCEXPR(5, rx_srcport, ((d->srcport_hi<<5) | d->srcport_lo))
GETFUNCFIELD(5, rx_srcmod, srcmod)
GETFUNCEXPR(5, rx_mcast, ((d->destmod_lo<<5) | d->destport))
GETFUNCERR(5, rx_vclabel)
GETFUNCERR(5, rx_mirror)
GETFUNCERR(5, rx_timestamp)
GETFUNCERR(5, rx_timestamp_upper)

#ifdef BROADCOM_DEBUG
GETFUNCFIELD(5, tx_l2pbm, l2ports)
GETFUNCFIELD(5, tx_utpbm, utports)
GETFUNCFIELD(5, tx_l3pbm, l3ports)
GETFUNCFIELD(5, tx_crc, c_crc)
GETFUNCFIELD(5, tx_cos, c_cos)
GETFUNCFIELD(5, tx_destmod, c_destmod)
GETFUNCEXPR(5, tx_destport, ((d->c_destport_hi<<4) | d->c_destport_lo))
GETFUNCEXPR(5, tx_opcode, ((d->c_opcode2<<2) | (d->c_opcode1<<1) | d->c_opcode0))
GETFUNCERR(5, tx_srcmod)
GETFUNCERR(5, tx_srcport)
GETFUNCERR(5, tx_prio)
GETFUNCERR(5, tx_pfm)
#endif /* BROADCOM_DEBUG */

#ifdef  PLISIM          /* these routines are only used by pcid */
static void dcb5_status_init(dcb_t *dcb)
{
    uint32      *d = (uint32 *)dcb;

    d[5] = d[6] = d[7] = d[8] = 0;
}
SETFUNCFIELD(5, xfercount, count, uint32 count, count)
SETFUNCFIELD(5, rx_start, start, int val, val ? 1 : 0)
SETFUNCFIELD(5, rx_end, end, int val, val ? 1 : 0)
SETFUNCFIELD(5, rx_error, error, int val, val ? 1 : 0)
SETFUNCFIELD(5, rx_crc, crc, int val, val ? 1 : 0)
#endif  /* PLISIM */

dcb_op_t dcb5_op = {
    5,
    sizeof(dcb5_t),
    dcb3_rx_reason_map,
    NULL,
    dcb5_init,
    dcb5_addtx,
    dcb5_addrx,
    dcb5_intrinfo,
    dcb5_reqcount_set,
    dcb5_reqcount_get,
    dcb5_xfercount_get,
    dcb0_addr_set,
    dcb0_addr_get,
    dcb0_paddr_get,
    dcb5_done_set,
    dcb5_done_get,
    dcb5_sg_set,
    dcb5_sg_get,
    dcb5_chain_set,
    dcb5_chain_get,
    dcb5_reload_set,
    dcb5_reload_get,
    dcb5_tx_l2pbm_set,
    dcb5_tx_utpbm_set,
    dcb5_tx_l3pbm_set,
    dcb5_tx_crc_set,
    dcb5_tx_cos_set,
    dcb5_tx_destmod_set,
    dcb5_tx_destport_set,
    dcb5_tx_opcode_set,
    dcb5_tx_srcmod_set,
    dcb5_tx_srcport_set,
    dcb5_tx_prio_set,
    dcb5_tx_pfm_set,
    dcb5_rx_untagged_get,
    dcb5_rx_crc_get,
    dcb5_rx_cos_get,
    dcb5_rx_destmod_get,
    dcb5_rx_destport_get,
    dcb5_rx_opcode_get,
    dcb5_rx_classtag_get,
    dcb5_rx_matchrule_get,
    dcb5_rx_start_get,
    dcb5_rx_end_get,
    dcb5_rx_error_get,
    dcb5_rx_prio_get,
    dcb5_rx_reason_get,
    dcb5_rx_reason_hi_get,
    dcb5_rx_ingport_get,
    dcb5_rx_srcport_get,
    dcb5_rx_srcmod_get,
    dcb5_rx_mcast_get,
    dcb5_rx_vclabel_get,
    dcb5_rx_mirror_get,
    dcb5_rx_timestamp_get,
    dcb5_rx_timestamp_upper_get,
    dcb0_hg_set,
    dcb0_hg_get,
    dcb0_stat_set,
    dcb0_stat_get,
    dcb0_purge_set,
    dcb0_purge_get,
    dcb0_mhp_get,
    dcb0_outer_vid_get,
    dcb0_outer_pri_get,
    dcb0_outer_cfi_get,
    dcb0_inner_vid_get,
    dcb0_inner_pri_get,
    dcb0_inner_cfi_get,
    dcb0_rx_bpdu_get,
    dcb0_rx_egr_to_cpu_get,
#ifdef  BROADCOM_DEBUG
    dcb5_tx_l2pbm_get,
    dcb5_tx_utpbm_get,
    dcb5_tx_l3pbm_get,
    dcb5_tx_crc_get,
    dcb5_tx_cos_get,
    dcb5_tx_destmod_get,
    dcb5_tx_destport_get,
    dcb5_tx_opcode_get,
    dcb5_tx_srcmod_get,
    dcb5_tx_srcport_get,
    dcb5_tx_prio_get,
    dcb5_tx_pfm_get,

    dcb0_dump,
    dcb0_reason_dump,
#endif /* BROADCOM_DEBUG */
#ifdef  PLISIM
    dcb5_status_init,
    dcb5_xfercount_set,
    dcb5_rx_start_set,
    dcb5_rx_end_set,
    dcb5_rx_error_set,
    dcb5_rx_crc_set,
    NULL,
#endif
};

#endif  /* BCM_LYNX_SUPPORT */

#ifdef  BCM_TUCANA_SUPPORT
/*
 * DCB Type 6 Support
 */

static soc_rx_reason_t
dcb6_rx_reason_map[] = {
    socRxReasonUnknownVlan,        /* Offset 0 */
    socRxReasonL2SourceMiss,       /* Offset 1 */ 
    socRxReasonL2DestMiss,         /* Offset 2 */
    socRxReasonL2Move,             /* Offset 3 */
    socRxReasonL2Cpu,              /* Offset 4 */
    socRxReasonInvalid,            /* Offset 5 */
    socRxReasonInvalid,            /* Offset 6 */
    socRxReasonL3SourceMiss,       /* Offset 7 */ 
    socRxReasonL3DestMiss,         /* Offset 8 */
    socRxReasonL3SourceMove,       /* Offset 9 */ 
    socRxReasonMcastMiss,          /* Offset 10 */
    socRxReasonIpMcastMiss,        /* Offset 11 */
    socRxReasonFilterMatch,        /* Offset 12 */
    socRxReasonL3HeaderError,      /* Offset 13 */
    socRxReasonProtocol,           /* Offset 14 */
    socRxReasonVcLabelMiss,        /* Offset 15 */
    socRxReasonInvalid,            /* Offset 16 */
    socRxReasonInvalid,            /* Offset 17 */
    socRxReasonInvalid,            /* Offset 18 */
    socRxReasonInvalid,            /* Offset 19 */
    socRxReasonInvalid,            /* Offset 20 */
    socRxReasonInvalid,            /* Offset 21 */
    socRxReasonInvalid,            /* Offset 22 */
    socRxReasonInvalid,            /* Offset 23 */
    socRxReasonInvalid,            /* Offset 24 */
    socRxReasonInvalid,            /* Offset 25 */
    socRxReasonInvalid,            /* Offset 26 */
    socRxReasonInvalid,            /* Offset 27 */
    socRxReasonInvalid,            /* Offset 28 */
    socRxReasonInvalid,            /* Offset 29 */
    socRxReasonInvalid,            /* Offset 30 */
    socRxReasonInvalid,            /* Offset 31 */
};

static void
dcb6_init(dcb_t *dcb)
{
    uint32      *d = (uint32 *)dcb;

    d[0] = d[1] = d[2] = d[3] = d[4] = d[5] = d[6] = d[7] = 0;
    d[8] = d[9] = d[10] = d[11] = d[12] = d[13] = d[14] = 0;
}

static int
dcb6_addtx(dv_t *dv, sal_vaddr_t addr, uint32 count,
           pbmp_t l2pbm, pbmp_t utpbm, pbmp_t l3pbm, uint32 flags, uint32 *hgh)
{
    dcb6_t      *d;
    uint32      *di;
    uint32      paddr, val;

    d = (dcb6_t *)SOC_DCB_IDX2PTR(dv->dv_unit, dv->dv_dcb, dv->dv_vcnt);

    if (addr) {
        paddr = soc_cm_l2p(dv->dv_unit, (void *)addr);
    } else {
        paddr = 0;
    }

    if (dv->dv_vcnt > 0 && (dv->dv_flags & DV_F_COMBINE_DCB) &&
        (d[-1].c_sg != 0) &&
        (d[-1].addr + d[-1].c_count) == paddr &&
        d[-1].c_count + count <= DCB_MAX_REQCOUNT) {
        d[-1].c_count += count;
        return dv->dv_cnt - dv->dv_vcnt;
    }

    if (dv->dv_vcnt >= dv->dv_cnt) {
        return SOC_E_FULL;
    }
    if (dv->dv_vcnt > 0) {      /* chain off previous dcb */
        d[-1].c_chain = 1;
    }

    di = (uint32 *)d;
    di[0] = di[1] = di[2] = di[3] = di[4] = di[5] = di[6] = di[7] = 0;
    di[8] = di[9] = di[10] = di[11] = di[12] = di[13] = di[14] = 0;

    d->addr = paddr;
    d->c_count = count;
    d->c_sg = 1;
    d->c_cos = SOC_DMA_COS_GET(flags);
    d->c_crc = SOC_DMA_CRC_GET(flags) ? DCB_XGS_CRC_REGEN : DCB_XGS_CRC_LEAVE;
    d->l2ports_mod0 = SOC_PBMP_WORD_GET(l2pbm, 0);
    d->l2ports_mod1 = SOC_PBMP_WORD_GET(l2pbm, 1);
    d->utports_mod0 = SOC_PBMP_WORD_GET(utpbm, 0);
    d->utports_mod1 = SOC_PBMP_WORD_GET(utpbm, 1);
    d->l3ports_mod0 = SOC_PBMP_WORD_GET(l3pbm, 0);
    d->l3ports_mod1 = SOC_PBMP_WORD_GET(l3pbm, 1);

    /* tx_destmod_set, tx_destport_set, tx_opcode_set */
    d->c_destmod = SOC_DMA_DMOD_GET(flags);
    val = SOC_DMA_DPORT_GET(flags);
    d->c_destport_lo = val;
    d->c_destport_hi = val>>4;
    d->c_opcode = SOC_DMA_MHOP_GET(flags);

    dv->dv_vcnt += 1;
    return dv->dv_cnt - dv->dv_vcnt;
}

static int
dcb6_addrx(dv_t *dv, sal_vaddr_t addr, uint32 count, uint32 flags)
{
    dcb6_t      *d;
    uint32      *di;

    d = (dcb6_t *)SOC_DCB_IDX2PTR(dv->dv_unit, dv->dv_dcb, dv->dv_vcnt);

    if (dv->dv_vcnt > 0) {      /* chain off previous dcb */
        d[-1].c_chain = 1;
    }

    di = (uint32 *)d;
    di[0] = di[1] = di[2] = di[3] = di[4] = di[5] = di[6] = di[7] = 0;
    di[8] = di[9] = di[10] = di[11] = di[12] = di[13] = di[14] = 0;

    if (addr) {
        d->addr = soc_cm_l2p(dv->dv_unit, (void *)addr);
    }
    d->c_count = count;
    d->c_sg = 1;
    d->c_cos = SOC_DMA_COS_GET(flags);
    d->c_crc = SOC_DMA_CRC_GET(flags) ? DCB_XGS_CRC_REGEN : DCB_XGS_CRC_LEAVE;

    dv->dv_vcnt += 1;
    return dv->dv_cnt - dv->dv_vcnt;
}

static uint32
dcb6_intrinfo(int unit, dcb_t *dcb, int tx, uint32 *count)
{
    dcb6_t      *d = (dcb6_t *)dcb;
    uint32      f;

    if (!d->s4valid) {
        return 0;
    }
    f = SOC_DCB_INFO_DONE;
    if (tx) {
        if (!d->c_sg) {
            f |= SOC_DCB_INFO_PKTEND;
        }
    } else {
        if (d->end) {
            f |= SOC_DCB_INFO_PKTEND;
        }
    }
    *count = d->count;
    return f;
}

SETFUNCFIELD(6, reqcount, c_count, uint32 count, count)
GETFUNCFIELD(6, reqcount, c_count)
GETFUNCFIELD(6, xfercount, count)
SETFUNCFIELD(6, done, s4valid, int val, val ? 1 : 0)
GETFUNCFIELD(6, done, s4valid)
SETFUNCFIELD(6, sg, c_sg, int val, val ? 1 : 0)
GETFUNCFIELD(6, sg, c_sg)
SETFUNCFIELD(6, chain, c_chain, int val, val ? 1 : 0)
GETFUNCFIELD(6, chain, c_chain)
SETFUNCFIELD(6, reload, c_reload, int val, val ? 1 : 0)
GETFUNCFIELD(6, reload, c_reload)
SETFUNCEXPR(6, tx_l2pbm, pbmp_t pbm,
        d->l2ports_mod0 = SOC_PBMP_WORD_GET(pbm, 0);
        d->l2ports_mod1 = SOC_PBMP_WORD_GET(pbm, 1))
SETFUNCEXPR(6, tx_utpbm, pbmp_t pbm,
        d->utports_mod0 = SOC_PBMP_WORD_GET(pbm, 0);
        d->utports_mod1 = SOC_PBMP_WORD_GET(pbm, 1))
SETFUNCEXPR(6, tx_l3pbm, pbmp_t pbm,
        d->l3ports_mod0 = SOC_PBMP_WORD_GET(pbm, 0);
        d->l3ports_mod1 = SOC_PBMP_WORD_GET(pbm, 1))
SETFUNCFIELD(6, tx_crc, c_crc, int val,
             val ? DCB_XGS_CRC_REGEN : DCB_XGS_CRC_LEAVE)
SETFUNCFIELD(6, tx_cos, c_cos, int val, val)
SETFUNCFIELD(6, tx_destmod, c_destmod, uint32 val, val)
SETFUNCEXPR(6, tx_destport, uint32 val,
        d->c_destport_lo = val; d->c_destport_hi = val>>4)
SETFUNCFIELD(6, tx_opcode, c_opcode, uint32 val, val)
SETFUNCERR(6, tx_srcmod, uint32)
SETFUNCERR(6, tx_srcport, uint32)
SETFUNCERR(6, tx_prio, uint32)
SETFUNCERR(6, tx_pfm, uint32)
GETFUNCUNITEXPR(6, rx_untagged, !d->tagged)
GETFUNCFIELD(6, rx_crc, crc)
GETFUNCFIELD(6, rx_cos, cos)
GETFUNCEXPR(6, rx_destmod, ((d->destmod_hi<<3) | d->destmod_lo))
GETFUNCFIELD(6, rx_destport, destport)
GETFUNCFIELD(6, rx_opcode, opcode)
GETFUNCFIELD(6, rx_classtag, class_tag)
GETFUNCEXPR(6, rx_matchrule, ((d->match_rule_hi<<7) | d->match_rule))
GETFUNCFIELD(6, rx_start, start)
GETFUNCFIELD(6, rx_end, end)
GETFUNCFIELD(6, rx_error, error)
GETFUNCEXPR(6, rx_prio, ((d->prio_hi<<1) | d->prio_lo))
GETFUNCFIELD(6, rx_reason, reason)
GETFUNCERR(6, rx_reason_hi)
GETFUNCEXPR(6, rx_ingport, 
        ((d->s4srcmod1<<5) | (d->ingport_hi<<4) | d->ingport_lo))
GETFUNCEXPR(6, rx_srcport, ((d->srcport_hi<<5) | d->srcport_lo))
GETFUNCFIELD(6, rx_srcmod, srcmod)
GETFUNCEXPR(6, rx_mcast, ((d->destmod_lo<<5) | d->destport))
GETFUNCEXPR(6, rx_vclabel, ((d->vclabel_hi<<16) | d->class_tag))
GETFUNCERR(6, rx_mirror)
GETFUNCERR(6, rx_timestamp)
GETFUNCERR(6, rx_timestamp_upper)

#ifdef BROADCOM_DEBUG
GETFUNCFIELD(6, tx_l2pbm, l2ports_mod0)
GETFUNCFIELD(6, tx_utpbm, utports_mod0)
GETFUNCFIELD(6, tx_l3pbm, l3ports_mod0)
GETFUNCFIELD(6, tx_crc, c_crc)
GETFUNCFIELD(6, tx_cos, c_cos)
GETFUNCFIELD(6, tx_destmod, c_destmod)
GETFUNCEXPR(6, tx_destport, ((d->c_destport_hi<<4) | d->c_destport_lo))
GETFUNCFIELD(6, tx_opcode, c_opcode)
GETFUNCERR(6, tx_srcmod)
GETFUNCERR(6, tx_srcport)
GETFUNCERR(6, tx_prio)
GETFUNCERR(6, tx_pfm)
#endif /* BROADCOM_DEBUG */

#ifdef  PLISIM          /* these routines are only used by pcid */
static void dcb6_status_init(dcb_t *dcb)
{
    uint32      *d = (uint32 *)dcb;

    d[10] = d[11] = d[12] = d[13] = d[14] = 0;
}
SETFUNCFIELD(6, xfercount, count, uint32 count, count)
SETFUNCFIELD(6, rx_start, start, int val, val ? 1 : 0)
SETFUNCFIELD(6, rx_end, end, int val, val ? 1 : 0)
SETFUNCFIELD(6, rx_error, error, int val, val ? 1 : 0)
SETFUNCFIELD(6, rx_crc, crc, int val, val ? 1 : 0)
#endif  /* PLISIM */

dcb_op_t dcb6_op = {
    6,
    sizeof(dcb6_t),
    dcb6_rx_reason_map,
    NULL,
    dcb6_init,
    dcb6_addtx,
    dcb6_addrx,
    dcb6_intrinfo,
    dcb6_reqcount_set,
    dcb6_reqcount_get,
    dcb6_xfercount_get,
    dcb0_addr_set,
    dcb0_addr_get,
    dcb0_paddr_get,
    dcb6_done_set,
    dcb6_done_get,
    dcb6_sg_set,
    dcb6_sg_get,
    dcb6_chain_set,
    dcb6_chain_get,
    dcb6_reload_set,
    dcb6_reload_get,
    dcb6_tx_l2pbm_set,
    dcb6_tx_utpbm_set,
    dcb6_tx_l3pbm_set,
    dcb6_tx_crc_set,
    dcb6_tx_cos_set,
    dcb6_tx_destmod_set,
    dcb6_tx_destport_set,
    dcb6_tx_opcode_set,
    dcb6_tx_srcmod_set,
    dcb6_tx_srcport_set,
    dcb6_tx_prio_set,
    dcb6_tx_pfm_set,
    dcb6_rx_untagged_get,
    dcb6_rx_crc_get,
    dcb6_rx_cos_get,
    dcb6_rx_destmod_get,
    dcb6_rx_destport_get,
    dcb6_rx_opcode_get,
    dcb6_rx_classtag_get,
    dcb6_rx_matchrule_get,
    dcb6_rx_start_get,
    dcb6_rx_end_get,
    dcb6_rx_error_get,
    dcb6_rx_prio_get,
    dcb6_rx_reason_get,
    dcb6_rx_reason_hi_get,
    dcb6_rx_ingport_get,
    dcb6_rx_srcport_get,
    dcb6_rx_srcmod_get,
    dcb6_rx_mcast_get,
    dcb6_rx_vclabel_get,
    dcb6_rx_mirror_get,
    dcb6_rx_timestamp_get,
    dcb6_rx_timestamp_upper_get,
    dcb0_hg_set,
    dcb0_hg_get,
    dcb0_stat_set,
    dcb0_stat_get,
    dcb0_purge_set,
    dcb0_purge_get,
    dcb0_mhp_get,
    dcb0_outer_vid_get,
    dcb0_outer_pri_get,
    dcb0_outer_cfi_get,
    dcb0_inner_vid_get,
    dcb0_inner_pri_get,
    dcb0_inner_cfi_get,
    dcb0_rx_bpdu_get,
    dcb0_rx_egr_to_cpu_get,
#ifdef  BROADCOM_DEBUG
    dcb6_tx_l2pbm_get,
    dcb6_tx_utpbm_get,
    dcb6_tx_l3pbm_get,
    dcb6_tx_crc_get,
    dcb6_tx_cos_get,
    dcb6_tx_destmod_get,
    dcb6_tx_destport_get,
    dcb6_tx_opcode_get,
    dcb6_tx_srcmod_get,
    dcb6_tx_srcport_get,
    dcb6_tx_prio_get,
    dcb6_tx_pfm_get,

    dcb0_dump,
    dcb0_reason_dump,
#endif /* BROADCOM_DEBUG */
#ifdef  PLISIM
    dcb6_status_init,
    dcb6_xfercount_set,
    dcb6_rx_start_set,
    dcb6_rx_end_set,
    dcb6_rx_error_set,
    dcb6_rx_crc_set,
    NULL,
#endif
};

#endif  /* BCM_TUCANA_SUPPORT */

#ifdef  BCM_DRACO15_SUPPORT
/*
 * DCB Type 7 Support
 */

static void
dcb7_init(dcb_t *dcb)
{
    uint32      *d = (uint32 *)dcb;

    d[0] = d[1] = d[2] = d[3] = d[4] = 0;
    d[5] = d[6] = d[7] = d[8] = d[9] = 0;
}

static int
dcb7_addtx(dv_t *dv, sal_vaddr_t addr, uint32 count,
           pbmp_t l2pbm, pbmp_t utpbm, pbmp_t l3pbm, uint32 flags, uint32 *hgh)
{
    dcb7_t      *d;
    uint32      *di;
    uint32      paddr;

    d = (dcb7_t *)SOC_DCB_IDX2PTR(dv->dv_unit, dv->dv_dcb, dv->dv_vcnt);

    if (addr) {
        paddr = soc_cm_l2p(dv->dv_unit, (void *)addr);
    } else {
        paddr = 0;
    }

    if (dv->dv_vcnt > 0 && (dv->dv_flags & DV_F_COMBINE_DCB) &&
        (d[-1].c_sg != 0) &&
        (d[-1].addr + d[-1].c_count) == paddr &&
        d[-1].c_count + count <= DCB_MAX_REQCOUNT) {
        d[-1].c_count += count;
        return dv->dv_cnt - dv->dv_vcnt;
    }

    if (dv->dv_vcnt >= dv->dv_cnt) {
        return SOC_E_FULL;
    }
    if (dv->dv_vcnt > 0) {      /* chain off previous dcb */
        d[-1].c_chain = 1;
    }

    di = (uint32 *)d;
    di[0] = di[1] = di[2] = di[3] = di[4] = 0;
    di[5] = di[6] = di[7] = di[8] = di[9] = 0;

    d->addr = paddr;
    d->c_count = count;
    d->c_sg = 1;
    d->c_cos = SOC_DMA_COS_GET(flags);
    d->c_crc = SOC_DMA_CRC_GET(flags) ? DCB_XGS_CRC_REGEN : DCB_XGS_CRC_LEAVE;
    d->l2ports = SOC_PBMP_WORD_GET(l2pbm, 0);
    d->utports = SOC_PBMP_WORD_GET(utpbm, 0);
    d->l3ports = SOC_PBMP_WORD_GET(l3pbm, 0);

    /* tx_destmod_set, tx_destport_set, tx_opcode_set */
    d->c_destmod = SOC_DMA_DMOD_GET(flags);
    d->c_destport = SOC_DMA_DPORT_GET(flags);
    d->c_opcode = SOC_DMA_MHOP_GET(flags);
    d->c_srcmod = SOC_DEFAULT_DMA_SRCMOD_GET(dv->dv_unit);
    d->c_srcport = SOC_DEFAULT_DMA_SRCPORT_GET(dv->dv_unit);
    d->c_pfm = SOC_DEFAULT_DMA_PFM_GET(dv->dv_unit);

    dv->dv_vcnt += 1;
    return dv->dv_cnt - dv->dv_vcnt;
}

static int
dcb7_addrx(dv_t *dv, sal_vaddr_t addr, uint32 count, uint32 flags)
{
    dcb7_t      *d;
    uint32      *di;

    d = (dcb7_t *)SOC_DCB_IDX2PTR(dv->dv_unit, dv->dv_dcb, dv->dv_vcnt);

    if (dv->dv_vcnt > 0) {      /* chain off previous dcb */
        d[-1].c_chain = 1;
    }

    di = (uint32 *)d;
    di[0] = di[1] = di[2] = di[3] = di[4] = 0;
    di[5] = di[6] = di[7] = di[8] = di[9] = 0;

    if (addr) {
        d->addr = soc_cm_l2p(dv->dv_unit, (void *)addr);
    }
    d->c_count = count;
    d->c_sg = 1;
    d->c_cos = SOC_DMA_COS_GET(flags);
    d->c_crc = SOC_DMA_CRC_GET(flags) ? DCB_XGS_CRC_REGEN : DCB_XGS_CRC_LEAVE;

    dv->dv_vcnt += 1;
    return dv->dv_cnt - dv->dv_vcnt;
}

static uint32
dcb7_intrinfo(int unit, dcb_t *dcb, int tx, uint32 *count)
{
    dcb7_t      *d = (dcb7_t *)dcb;
    uint32      f;

    if (!d->s3valid) {
        return 0;
    }
    f = SOC_DCB_INFO_DONE;
    if (tx) {
        if (!d->c_sg) {
            f |= SOC_DCB_INFO_PKTEND;
        }
    } else {
        if (d->end) {
            f |= SOC_DCB_INFO_PKTEND;
        }
    }
    *count = d->count;
    return f;
}

SETFUNCFIELD(7, reqcount, c_count, uint32 count, count)
GETFUNCFIELD(7, reqcount, c_count)
GETFUNCFIELD(7, xfercount, count)
SETFUNCFIELD(7, done, s3valid, int val, val ? 1 : 0)
GETFUNCFIELD(7, done, s3valid)
SETFUNCFIELD(7, sg, c_sg, int val, val ? 1 : 0)
GETFUNCFIELD(7, sg, c_sg)
SETFUNCFIELD(7, chain, c_chain, int val, val ? 1 : 0)
GETFUNCFIELD(7, chain, c_chain)
SETFUNCFIELD(7, reload, c_reload, int val, val ? 1 : 0)
GETFUNCFIELD(7, reload, c_reload)
SETFUNCFIELD(7, tx_l2pbm, l2ports, pbmp_t pbm, SOC_PBMP_WORD_GET(pbm, 0))
SETFUNCFIELD(7, tx_utpbm, utports, pbmp_t pbm, SOC_PBMP_WORD_GET(pbm, 0))
SETFUNCFIELD(7, tx_l3pbm, l3ports, pbmp_t pbm, SOC_PBMP_WORD_GET(pbm, 0))
SETFUNCFIELD(7, tx_crc, c_crc, int val,
             val ? DCB_XGS_CRC_REGEN : DCB_XGS_CRC_LEAVE)
SETFUNCFIELD(7, tx_cos, c_cos, int val, val)
SETFUNCFIELD(7, tx_destmod, c_destmod, uint32 val, val)
SETFUNCFIELD(7, tx_destport, c_destport, uint32 val, val)
SETFUNCFIELD(7, tx_opcode, c_opcode, uint32 val, val)
SETFUNCFIELD(7, tx_srcmod, c_srcmod, uint32 val, val)
SETFUNCFIELD(7, tx_srcport, c_srcport, uint32 val, val)
SETFUNCFIELD(7, tx_prio, c_prio, uint32 val, val)
SETFUNCFIELD(7, tx_pfm, c_pfm, uint32 val, val)
GETFUNCUNITEXPR(7, rx_untagged, !d->tagged)
GETFUNCFIELD(7, rx_crc, crc)
GETFUNCFIELD(7, rx_cos, cos)
GETFUNCFIELD(7, rx_destmod, destmod)
GETFUNCFIELD(7, rx_destport, destport)
GETFUNCFIELD(7, rx_opcode, opcode)
GETFUNCFIELD(7, rx_classtag, class_tag)
GETFUNCFIELD(7, rx_matchrule, match_rule)
GETFUNCFIELD(7, rx_start, start)
GETFUNCFIELD(7, rx_end, end)
GETFUNCFIELD(7, rx_error, error)
GETFUNCFIELD(7, rx_prio, prio)
GETFUNCFIELD(7, rx_reason, reason)
GETFUNCERR(7, rx_reason_hi)
GETFUNCFIELD(7, rx_ingport, ingport)
GETFUNCFIELD(7, rx_srcport, srcport)
GETFUNCFIELD(7, rx_srcmod, srcmod)
GETFUNCEXPR(7, rx_mcast, ((d->destmod<<6) | d->destport))
GETFUNCERR(7, rx_vclabel)
GETFUNCERR(7, rx_mirror)
GETFUNCERR(7, rx_timestamp)
GETFUNCERR(7, rx_timestamp_upper)

#ifdef BROADCOM_DEBUG
GETFUNCFIELD(7, tx_l2pbm, l2ports)
GETFUNCFIELD(7, tx_utpbm, utports)
GETFUNCFIELD(7, tx_l3pbm, l3ports)
GETFUNCFIELD(7, tx_crc, c_crc)
GETFUNCFIELD(7, tx_cos, c_cos)
GETFUNCFIELD(7, tx_destmod, c_destmod)
GETFUNCFIELD(7, tx_destport, c_destport)
GETFUNCFIELD(7, tx_opcode, c_opcode)
GETFUNCFIELD(7, tx_srcmod, c_srcmod)
GETFUNCFIELD(7, tx_srcport, c_srcport)
GETFUNCFIELD(7, tx_prio, c_prio)
GETFUNCFIELD(7, tx_pfm, c_pfm)
#endif /* BROADCOM_DEBUG */

#ifdef  PLISIM          /* these routines are only used by pcid */
static void dcb7_status_init(dcb_t *dcb)
{
    uint32      *d = (uint32 *)dcb;

    d[6] = d[7] = d[8] = d[9] = 0;
}
SETFUNCFIELD(7, xfercount, count, uint32 count, count)
SETFUNCFIELD(7, rx_start, start, int val, val ? 1 : 0)
SETFUNCFIELD(7, rx_end, end, int val, val ? 1 : 0)
SETFUNCFIELD(7, rx_error, error, int val, val ? 1 : 0)
SETFUNCFIELD(7, rx_crc, crc, int val, val ? 1 : 0)
#endif  /* PLISIM */

dcb_op_t dcb7_op = {
    7,
    sizeof(dcb7_t),
    dcb3_rx_reason_map,
    NULL,
    dcb7_init,
    dcb7_addtx,
    dcb7_addrx,
    dcb7_intrinfo,
    dcb7_reqcount_set,
    dcb7_reqcount_get,
    dcb7_xfercount_get,
    dcb0_addr_set,
    dcb0_addr_get,
    dcb0_paddr_get,
    dcb7_done_set,
    dcb7_done_get,
    dcb7_sg_set,
    dcb7_sg_get,
    dcb7_chain_set,
    dcb7_chain_get,
    dcb7_reload_set,
    dcb7_reload_get,
    dcb7_tx_l2pbm_set,
    dcb7_tx_utpbm_set,
    dcb7_tx_l3pbm_set,
    dcb7_tx_crc_set,
    dcb7_tx_cos_set,
    dcb7_tx_destmod_set,
    dcb7_tx_destport_set,
    dcb7_tx_opcode_set,
    dcb7_tx_srcmod_set,
    dcb7_tx_srcport_set,
    dcb7_tx_prio_set,
    dcb7_tx_pfm_set,
    dcb7_rx_untagged_get,
    dcb7_rx_crc_get,
    dcb7_rx_cos_get,
    dcb7_rx_destmod_get,
    dcb7_rx_destport_get,
    dcb7_rx_opcode_get,
    dcb7_rx_classtag_get,
    dcb7_rx_matchrule_get,
    dcb7_rx_start_get,
    dcb7_rx_end_get,
    dcb7_rx_error_get,
    dcb7_rx_prio_get,
    dcb7_rx_reason_get,
    dcb7_rx_reason_hi_get,
    dcb7_rx_ingport_get,
    dcb7_rx_srcport_get,
    dcb7_rx_srcmod_get,
    dcb7_rx_mcast_get,
    dcb7_rx_vclabel_get,
    dcb7_rx_mirror_get,
    dcb7_rx_timestamp_get,
    dcb7_rx_timestamp_upper_get,
    dcb0_hg_set,
    dcb0_hg_get,
    dcb0_stat_set,
    dcb0_stat_get,
    dcb0_purge_set,
    dcb0_purge_get,
    dcb0_mhp_get,
    dcb0_outer_vid_get,
    dcb0_outer_pri_get,
    dcb0_outer_cfi_get,
    dcb0_inner_vid_get,
    dcb0_inner_pri_get,
    dcb0_inner_cfi_get,
    dcb0_rx_bpdu_get,
    dcb0_rx_egr_to_cpu_get,
#ifdef  BROADCOM_DEBUG
    dcb7_tx_l2pbm_get,
    dcb7_tx_utpbm_get,
    dcb7_tx_l3pbm_get,
    dcb7_tx_crc_get,
    dcb7_tx_cos_get,
    dcb7_tx_destmod_get,
    dcb7_tx_destport_get,
    dcb7_tx_opcode_get,
    dcb7_tx_srcmod_get,
    dcb7_tx_srcport_get,
    dcb7_tx_prio_get,
    dcb7_tx_pfm_get,

    dcb0_dump,
    dcb0_reason_dump,
#endif /* BROADCOM_DEBUG */
#ifdef  PLISIM
    dcb7_status_init,
    dcb7_xfercount_set,
    dcb7_rx_start_set,
    dcb7_rx_end_set,
    dcb7_rx_error_set,
    dcb7_rx_crc_set,
    NULL,
#endif
};

#endif  /* BCM_DRACO15_SUPPORT */

#if defined(BCM_XGS3_SWITCH_SUPPORT) || defined(BCM_SIRIUS_SUPPORT)

#ifdef BCM_XGS3_SWITCH_SUPPORT
/*
 * DCB Type 9 Support
 */

#ifdef BCM_FIREBOLT_SUPPORT
static soc_rx_reason_t
dcb9_rx_reason_map[] = {
    socRxReasonUnknownVlan,        /* Offset 0 */
    socRxReasonL2SourceMiss,       /* Offset 1 */
    socRxReasonL2DestMiss,         /* Offset 2 */
    socRxReasonL2Move,             /* Offset 3 */
    socRxReasonL2Cpu,              /* Offset 4 */
    socRxReasonSampleSource,       /* Offset 5 */
    socRxReasonSampleDest,         /* Offset 6 */
    socRxReasonL3SourceMiss,       /* Offset 7 */
    socRxReasonL3DestMiss,         /* Offset 8 */
    socRxReasonL3SourceMove,       /* Offset 9 */
    socRxReasonMcastMiss,          /* Offset 10 */
    socRxReasonIpMcastMiss,        /* Offset 11 */
    socRxReasonFilterMatch,        /* Offset 12 */
    socRxReasonL3HeaderError,      /* Offset 13 */
    socRxReasonProtocol,           /* Offset 14 */
    socRxReasonDosAttack,          /* Offset 15 */
    socRxReasonMartianAddr,        /* Offset 16 */
    socRxReasonTunnelError,        /* Offset 17 */
    socRxReasonL2MtuFail,          /* Offset 18 */
    socRxReasonIcmpRedirect,       /* Offset 19 */
    socRxReasonL3Slowpath,         /* Offset 20 */
    socRxReasonInvalid,            /* Offset 21 */
    socRxReasonInvalid,            /* Offset 22 */
    socRxReasonInvalid,            /* Offset 23 */
    socRxReasonInvalid,            /* Offset 24 */
    socRxReasonInvalid,            /* Offset 25 */
    socRxReasonInvalid,            /* Offset 26 */
    socRxReasonInvalid,            /* Offset 27 */
    socRxReasonInvalid,            /* Offset 28 */
    socRxReasonInvalid,            /* Offset 29 */
    socRxReasonInvalid,            /* Offset 30 */
    socRxReasonInvalid             /* Offset 31 */
};
#endif /* BCM_FIREBOLT_SUPPORT */

static void
dcb9_init(dcb_t *dcb)
{
    uint32      *d = (uint32 *)dcb;

    d[0] = d[1] = d[2] = d[3] = d[4] = 0;
    d[5] = d[6] = d[7] = d[8] = d[9] = d[10] = 0;
}

static int
dcb9_addtx(dv_t *dv, sal_vaddr_t addr, uint32 count,
           pbmp_t l2pbm, pbmp_t utpbm, pbmp_t l3pbm, uint32 flags, uint32 *hgh)
{
    dcb9_t      *d;     /* DCB */
    uint32      *di;    /* DCB integer pointer */
    uint32      paddr;  /* Packet buffer physical address */

    d = (dcb9_t *)SOC_DCB_IDX2PTR(dv->dv_unit, dv->dv_dcb, dv->dv_vcnt);

    if (addr) {
        paddr = soc_cm_l2p(dv->dv_unit, (void *)addr);
    } else {
        paddr = 0;
    }

    if (dv->dv_vcnt > 0 && (dv->dv_flags & DV_F_COMBINE_DCB) &&
        (d[-1].c_sg != 0) &&
        (d[-1].addr + d[-1].c_count) == paddr &&
        d[-1].c_count + count <= DCB_MAX_REQCOUNT) {
        d[-1].c_count += count;
        return dv->dv_cnt - dv->dv_vcnt;
    }

    if (dv->dv_vcnt >= dv->dv_cnt) {
        return SOC_E_FULL;
    }
    if (dv->dv_vcnt > 0) {      /* chain off previous dcb */
        d[-1].c_chain = 1;
    }

    di = (uint32 *)d;
    di[0] = di[1] = di[2] = di[3] = di[4] = 0;
    di[5] = di[6] = di[7] = di[8] = di[9] = di[10] = 0;

    d->addr = paddr;
    d->c_count = count;
    d->c_sg = 1;

    d->c_stat = 1;
    d->c_purge = SOC_DMA_PURGE_GET(flags);
    if (SOC_DMA_HG_GET(flags)) {
#if defined(BCM_BRADLEY_SUPPORT) || defined(BCM_RAPTOR_SUPPORT)
        soc_higig_hdr_t *mh = (soc_higig_hdr_t *)hgh;
        if (mh->overlay1.start == SOC_HIGIG2_START) {
#if defined(BCM_BRADLEY_SUPPORT)
            dcb11_t *d11 = (dcb11_t *)d;
#elif defined(BCM_RAPTOR_SUPPORT)
            dcb12_t *d11 = (dcb12_t *)d;
#endif
            d11->mh3 = soc_ntohl(hgh[3]);
        }
#endif  /* BCM_BRADLEY_SUPPORT || BCM_RAPTOR_SUPPORT */
        d->c_hg = 1;
        d->mh0 = soc_ntohl(hgh[0]);
        d->mh1 = soc_ntohl(hgh[1]);
        d->mh2 = soc_ntohl(hgh[2]);
    }

    dv->dv_vcnt += 1;
    return dv->dv_cnt - dv->dv_vcnt;
}

static int
dcb9_addrx(dv_t *dv, sal_vaddr_t addr, uint32 count, uint32 flags)
{
    dcb9_t      *d;     /* DCB */
    uint32      *di;    /* DCB integer pointer */

    d = (dcb9_t *)SOC_DCB_IDX2PTR(dv->dv_unit, dv->dv_dcb, dv->dv_vcnt);

    if (dv->dv_vcnt > 0) {      /* chain off previous dcb */
        d[-1].c_chain = 1;
    }

    di = (uint32 *)d;
    di[0] = di[1] = di[2] = di[3] = di[4] = 0;
    di[5] = di[6] = di[7] = di[8] = di[9] = di[10] = 0;

    if (addr) {
        d->addr = soc_cm_l2p(dv->dv_unit, (void *)addr);
    }
    d->c_count = count;
    d->c_sg = 1;

    dv->dv_vcnt += 1;
    return dv->dv_cnt - dv->dv_vcnt;
}

static uint32
dcb9_intrinfo(int unit, dcb_t *dcb, int tx, uint32 *count)
{
    dcb9_t      *d = (dcb9_t *)dcb;     /*  DCB */
    uint32      f;                      /* SOC_DCB_INFO_* flags */

    if (!d->done) {
        return 0;
    }
    f = SOC_DCB_INFO_DONE;
    if (tx) {
        if (!d->c_sg) {
            f |= SOC_DCB_INFO_PKTEND;
        }
    } else {
        if (d->end) {
            f |= SOC_DCB_INFO_PKTEND;
        }
    }
    *count = d->count;
    return f;
}

SETFUNCFIELD(9, reqcount, c_count, uint32 count, count)
GETFUNCFIELD(9, reqcount, c_count)
GETFUNCFIELD(9, xfercount, count)
/* addr_set, addr_get, paddr_get - Same as DCB 0 */
SETFUNCFIELD(9, done, done, int val, val ? 1 : 0)
GETFUNCFIELD(9, done, done)
SETFUNCFIELD(9, sg, c_sg, int val, val ? 1 : 0)
GETFUNCFIELD(9, sg, c_sg)
SETFUNCFIELD(9, chain, c_chain, int val, val ? 1 : 0)
GETFUNCFIELD(9, chain, c_chain)
SETFUNCFIELD(9, reload, c_reload, int val, val ? 1 : 0)
GETFUNCFIELD(9, reload, c_reload)
SETFUNCERR(9, tx_l2pbm, pbmp_t)
SETFUNCERR(9, tx_utpbm, pbmp_t)
SETFUNCERR(9, tx_l3pbm, pbmp_t)
SETFUNCERR(9, tx_crc, int)
SETFUNCERR(9, tx_cos, int)
SETFUNCERR(9, tx_destmod, uint32)
SETFUNCERR(9, tx_destport, uint32)
SETFUNCERR(9, tx_opcode, uint32)
SETFUNCERR(9, tx_srcmod, uint32)
SETFUNCERR(9, tx_srcport, uint32)
SETFUNCERR(9, tx_prio, uint32)
SETFUNCERR(9, tx_pfm, uint32)
GETFUNCFIELD(9, rx_start, start)
GETFUNCFIELD(9, rx_end, end)
GETFUNCFIELD(9, rx_error, error)
/* Fields extracted from MH/PBI */
GETFUNCFIELD(9, rx_cos, cpu_cos)
GETHGFUNCEXPR(9, rx_destmod, (h->overlay1.dst_mod |
                             (h->hgp_overlay1.dst_mod_5 << 5)))
GETHGFUNCFIELD(9, rx_destport, dst_port)
GETHGFUNCFIELD(9, rx_opcode, opcode)
GETHGFUNCFIELD(9, rx_prio, vlan_pri) /* outer_pri */

/* 
 * When Firebolt/Firebolt_2 are in double tagging mode:
 * - If incoming front-panel packet is double tagged, d->ingress_untagged = 0,
 *   h->hgp_overlay1.ingress_tagged = 1, d->add_vid = 0.
 * - If incoming front-panel packet is single outer tagged, d->ingress_untagged = 0,
 *   h->hgp_overlay1.ingress_tagged = 0, d->add_vid = 0.
 * - If incoming front-panel packet is single inner tagged, d->ingress_untagged = 0,
 *   h->hgp_overlay1.ingress_tagged = 1, d->add_vid = 1. 
 * - If incoming front-panel packet is untagged, d->ingress_untagged = 1,
 *   h->hgp_overlay1.ingress_tagged = 0, d->add_vid = 1. 
 * - For an incoming Higig packet, d->ingress_untagged = 0,
 *   h->hgp_overlay1.ingress_tagged = 1 if the packet contains an inner tag, 0 otherwise,
 *   d->add_vid = 1.
 *
 * In single tagging mode:
 * - If incoming front-panel packet is tagged, d->ingress_untagged = 0,
 *   h->hgp_overlay1.ingress_tagged = 1, d->add_vid = 0.
 * - If incoming front-panel packet is untagged, d->ingress_untagged = 1,
 *   h->hgp_overlay1.ingress_tagged = 0, d->add_vid = 1. 
 * - For an incoming Higig packet, d->ingress_untagged = 0,
 *   h->hgp_overlay1.ingress_tagged = 1 if the packet originally ingressed the
 *   ingress chip's front-panel port with a tag, 0 otherwise,
 *   d->add_vid = 1.
 */
GETHGFUNCUNITEXPR(9, rx_untagged, (SOC_DT_MODE(unit) ?
                                   (IS_HG_PORT(unit, d->srcport) ? 
                                    (h->hgp_overlay1.ingress_tagged ? 0 : 2) :
                                    (h->hgp_overlay1.ingress_tagged ?
                                     (d->add_vid ? 1 : 0) :
                                     (d->add_vid ? 3 : 2))) :
                                   (h->hgp_overlay1.ingress_tagged ? 2 : 3)))

#ifdef BCM_FIREBOLT_SUPPORT
GETFUNCFIELD(9, rx_matchrule, match_rule)
GETFUNCFIELD(9, rx_reason, reason)
GETFUNCERR(9, rx_reason_hi)
GETFUNCFIELD(9, rx_ingport, srcport)
GETFUNCERR(9, rx_vclabel)
GETFUNCEXPR(9, rx_mirror, ((d->imirror) | (d->emirror)))
GETFUNCERR(9, rx_timestamp)
GETFUNCERR(9, rx_timestamp_upper)
GETFUNCFIELD(9, outer_vid, outer_vid)
GETFUNCFIELD(9, outer_pri, outer_pri)
GETFUNCFIELD(9, outer_cfi, outer_cfi)
#endif /* BCM_FIREBOLT_SUPPORT */
#ifdef BCM_EASYRIDER_SUPPORT
GETFUNCFIELD(10, rx_matchrule, match_rule)
GETFUNCEXPR(10, rx_reason, ((d->reason_hi << 6) | (d->reason_lo)))
GETFUNCERR(10, rx_reason_hi)
GETFUNCFIELD(10, rx_ingport, srcport)
GETFUNCERR(10, rx_vclabel)
GETFUNCEXPR(10, rx_mirror, ((d->imirror) | (d->emirror)))
GETFUNCERR(10, rx_timestamp)
GETFUNCERR(10, rx_timestamp_upper)
GETFUNCNULL(10, outer_vid)
GETFUNCNULL(10, outer_pri)
GETFUNCNULL(10, outer_cfi)
#endif /* BCM_EASYRIDER_SUPPORT */
GETHGFUNCFIELD(9, rx_srcport, src_port)
GETHGFUNCEXPR(9, rx_srcmod, (h->overlay1.src_mod |
                             (h->hgp_overlay1.src_mod_5 << 5)))
/* DCB 9 specific fields */
SETFUNCFIELD(9, hg, c_hg, uint32 hg, hg)
GETFUNCFIELD(9, hg, c_hg)
SETFUNCFIELD(9, stat, c_stat, uint32 stat, stat)
GETFUNCFIELD(9, stat, c_stat)
SETFUNCFIELD(9, purge, c_purge, uint32 purge, purge)
GETFUNCFIELD(9, purge, c_purge)
GETPTREXPR(9, mhp, &(d->mh0))
GETFUNCNULL(9, inner_vid)
GETFUNCNULL(9, inner_pri)
GETFUNCNULL(9, inner_cfi)
GETFUNCFIELD(9, rx_bpdu, bpdu)
GETFUNCNULL(9, rx_egr_to_cpu)


#ifdef BCM_BRADLEY_SUPPORT
GETFUNCFIELD(11, rx_cos, cpu_cos)
GETHG2FUNCFIELD(11, rx_destmod, dst_mod)
GETHG2FUNCFIELD(11, rx_destport, dst_port)
GETHG2FUNCEXPR(11, rx_srcport, h->ppd_overlay1.src_port |
                               ((h->ppd_overlay1.ppd_type <= 1) ?
                                (h->ppd_overlay1.src_t << 5) : 0))
GETHG2FUNCFIELD(11, rx_srcmod, src_mod)
GETHG2FUNCFIELD(11, rx_opcode, opcode)
GETHG2FUNCFIELD(11, rx_prio, vlan_pri) /* outer_pri */
GETFUNCFIELD(11, rx_matchrule, match_rule)
GETHG2FUNCUNITEXPR(11, rx_untagged, !h->ppd_overlay1.ingress_tagged)
GETFUNCEXPR(11, rx_reason, d->reason)
GETFUNCERR(11, rx_reason_hi)
GETFUNCFIELD(11, rx_ingport, srcport)
GETFUNCEXPR(11, rx_mirror, ((d->imirror) | (d->emirror)))
GETFUNCERR(11, rx_timestamp)
GETFUNCERR(11, rx_timestamp_upper)
SETFUNCERR(11, stat, uint32)
GETFUNCEXPR(11, stat, d->c_hg ^ d->c_hg) /* 0 */
SETFUNCERR(11, purge, uint32)
GETFUNCEXPR(11, purge, d->c_hg ^ d->c_hg) /* 0 */
GETFUNCFIELD(11, outer_vid, outer_vid)
GETFUNCFIELD(11, outer_pri, outer_pri)
GETFUNCFIELD(11, outer_cfi, outer_cfi)
GETFUNCFIELD(11, rx_bpdu, bpdu)
GETFUNCNULL(11, rx_egr_to_cpu)
#endif /* BCM_BRADLEY_SUPPORT */

#if defined(BCM_RAPTOR_SUPPORT) 
GETHG2FUNCFIELD(12, rx_destmod, dst_mod)
GETHG2FUNCFIELD(12, rx_destport, dst_port)
GETHG2FUNCFIELD(12, rx_opcode, opcode)
GETHG2FUNCFIELD(12, rx_prio, vlan_pri) /* outer_pri */
GETHG2FUNCUNITEXPR(12, rx_untagged, !h->ppd_overlay1.ingress_tagged)
GETHG2FUNCFIELD(12, rx_srcport, src_port)
GETHG2FUNCFIELD(12, rx_srcmod, src_mod)
GETFUNCFIELD(12, rx_ingport, srcport)
GETFUNCFIELD(12, rx_crc, regen_crc)
GETFUNCFIELD(12, rx_cos, cpu_cos)
GETFUNCFIELD(12, rx_matchrule, match_rule)
GETFUNCFIELD(12, rx_reason, reason)
GETFUNCERR(12, rx_reason_hi)
GETHG2FUNCEXPR(12, rx_mcast, ((h->ppd_overlay1.dst_mod << 8) |
                              (h->ppd_overlay1.dst_port)))
GETHG2FUNCEXPR(12, rx_vclabel, ((h->ppd_overlay1.vc_label_19_16 << 16) |
                              (h->ppd_overlay1.vc_label_15_8 << 8) |
                              (h->ppd_overlay1.vc_label_7_0)))
GETHG2FUNCEXPR(12, rx_classtag, (h->ppd_overlay1.ppd_type != 1 ? 0 :
                                 (h->ppd_overlay2.ctag_hi << 8) |
                                 (h->ppd_overlay2.ctag_lo)))
GETFUNCEXPR(12, rx_mirror, ((d->imirror) | (d->emirror)))
GETFUNCERR(12, rx_timestamp)
GETFUNCERR(12, rx_timestamp_upper)
GETFUNCFIELD(12, outer_vid, outer_vid)
GETFUNCFIELD(12, outer_pri, outer_pri)
GETFUNCFIELD(12, outer_cfi, outer_cfi)
GETFUNCFIELD(12, rx_bpdu, bpdu)
GETFUNCNULL(12, rx_egr_to_cpu)
#ifdef  PLISIM
SETFUNCFIELD(12, rx_crc, regen_crc, int val, val ? 1 : 0)
SETFUNCFIELD(12, rx_ingport, srcport, int val, val)
#endif /* PLISIM */
GETHG2FUNCFIELD(15, rx_destmod, dst_mod)
GETHG2FUNCFIELD(15, rx_destport, dst_port)
GETHG2FUNCFIELD(15, rx_opcode, opcode)
GETHG2FUNCFIELD(15, rx_prio, vlan_pri) /* outer_pri */
GETHG2FUNCUNITEXPR(15, rx_untagged, !h->ppd_overlay1.ingress_tagged)
GETHG2FUNCFIELD(15, rx_srcport, src_port)
GETHG2FUNCFIELD(15, rx_srcmod, src_mod)
GETFUNCFIELD(15, rx_ingport, srcport)
GETFUNCFIELD(15, rx_crc, regen_crc)
GETFUNCFIELD(15, rx_cos, cpu_cos)
GETFUNCFIELD(15, rx_matchrule, match_rule)
GETFUNCFIELD(15, rx_reason, reason)
GETFUNCERR(15, rx_reason_hi)
GETHG2FUNCEXPR(15, rx_mcast, ((h->ppd_overlay1.dst_mod << 8) |
                              (h->ppd_overlay1.dst_port)))
GETHG2FUNCEXPR(15, rx_vclabel, ((h->ppd_overlay1.vc_label_19_16 << 16) |
                              (h->ppd_overlay1.vc_label_15_8 << 8) |
                              (h->ppd_overlay1.vc_label_7_0)))
GETHG2FUNCEXPR(15, rx_classtag, (h->ppd_overlay1.ppd_type != 1 ? 0 :
                                 (h->ppd_overlay2.ctag_hi << 8) |
                                 (h->ppd_overlay2.ctag_lo)))
GETFUNCEXPR(15, rx_mirror, ((d->imirror) | (d->emirror)))
GETFUNCERR(15, rx_timestamp)
GETFUNCERR(15, rx_timestamp_upper)
GETFUNCFIELD(15, outer_vid, outer_vid)
GETFUNCFIELD(15, outer_pri, outer_pri)
GETFUNCFIELD(15, outer_cfi, outer_cfi)
#ifdef  PLISIM
SETFUNCFIELD(15, rx_crc, regen_crc, int val, val ? 1 : 0)
SETFUNCFIELD(15, rx_ingport, srcport, int val, val)
#endif /* PLISIM */
#endif  /* BCM_RAPTOR_SUPPORT */

#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_FIREBOLT_SUPPORT)
GETFUNCFIELD(13, outer_vid, outer_vid)
GETFUNCFIELD(13, outer_pri, outer_pri)
GETFUNCFIELD(13, outer_cfi, outer_cfi)

/* For an explanation of how rx_untagged is derived for Firebolt2, see the
 * comments for the derivation of rx_untagged field for Firebolt.
 */ 
GETHGFUNCUNITEXPR(13, rx_untagged, (SOC_DT_MODE(unit) ?
                                   (IS_HG_PORT(unit, d->srcport) ? 
                                    (h->hgp_overlay1.ingress_tagged ? 0 : 2) :
                                    (h->hgp_overlay1.ingress_tagged ?
                                     (d->add_vid ? 1 : 0) :
                                     (d->add_vid ? 3 : 2))) :
                                   (h->hgp_overlay1.ingress_tagged ? 2 : 3)))

#endif /* BCM_FIREBOLT2_SUPPORT || BCM_FIREBOLT_SUPPORT */

#if defined(BCM_HAWKEYE_SUPPORT)
GETFUNCFIELD(17, rx_srcport, srcport)
GETFUNCFIELD(17, rx_reason, reason)
GETFUNCUNITFIELD(17, rx_untagged, ingress_untagged)
GETFUNCFIELD(17, rx_crc, regen_crc)
GETFUNCFIELD(17, rx_cos, cpu_cos)
GETFUNCNULL(17, rx_destmod)
GETFUNCNULL(17, rx_destport)
GETFUNCNULL(17, rx_opcode)
GETFUNCNULL(17, rx_classtag)
GETFUNCFIELD(17, rx_prio, outer_pri)
GETFUNCNULL(17, rx_srcmod)
GETFUNCNULL(17, rx_mcast)
GETFUNCNULL(17, rx_vclabel)
GETHG2FUNCEXPR(17, rx_timestamp, ((uint32 *)h)[1])
GETFUNCERR(17, rx_timestamp_upper)
#endif /* BCM_HAWKEYE_SUPPORT */

#ifdef BROADCOM_DEBUG
GETFUNCERR(9, tx_l2pbm) 
GETFUNCERR(9, tx_utpbm) 
GETFUNCERR(9, tx_l3pbm)
GETFUNCERR(9, tx_crc) 
GETFUNCERR(9, tx_cos)
GETFUNCERR(9, tx_destmod)
GETFUNCERR(9, tx_destport)
GETFUNCERR(9, tx_opcode) 
GETFUNCERR(9, tx_srcmod)
GETFUNCERR(9, tx_srcport)
GETFUNCERR(9, tx_prio)
GETFUNCERR(9, tx_pfm)
#endif /* BROADCOM_DEBUG */

static uint32 dcb9_rx_crc_get(dcb_t *dcb) {
    return 0;
}

#if defined(LE_HOST)
static uint32 dcb9_rx_classtag_get(dcb_t *dcb)
{
    dcb9_t *d = (dcb9_t *)dcb;
    uint32  hgh[3];
    soc_higig_hdr_t *h = (soc_higig_hdr_t *)&hgh[0];
    hgh[0] = soc_htonl(d->mh0);
    hgh[1] = soc_htonl(d->mh1);
    hgh[2] = soc_htonl(d->mh2);
    if (h->overlay1.hdr_format != 1) { /* return 0 if not overlay 2 format */
        return 0;
    }
    return((h->overlay2.ctag_hi << 8) | (h->overlay2.ctag_lo << 0));
}

static uint32 dcb9_rx_mcast_get(dcb_t *dcb)
{
    dcb9_t *d = (dcb9_t *)dcb;
    uint32  hgh[3];
    soc_higig_hdr_t *h = (soc_higig_hdr_t *)&hgh[0];
    hgh[0] = soc_htonl(d->mh0);
    hgh[1] = soc_htonl(d->mh1);
    hgh[2] = soc_htonl(d->mh2);
    return((h->overlay1.dst_mod << 5) | (h->overlay1.dst_port << 0));
}
#else
static uint32 dcb9_rx_classtag_get(dcb_t *dcb)
{
    dcb9_t *d = (dcb9_t *)dcb;
    soc_higig_hdr_t *h = (soc_higig_hdr_t *)&d->mh0;
    if (h->overlay1.hdr_format != 1) { /* return 0 if not overlay 2 format */
        return 0;
    }
    return((h->overlay2.ctag_hi << 8) | (h->overlay2.ctag_lo << 0));
}

static uint32 dcb9_rx_mcast_get(dcb_t *dcb)
{
    dcb9_t *d = (dcb9_t *)dcb;
    soc_higig_hdr_t *h = (soc_higig_hdr_t *)&d->mh0;
    return((h->overlay1.dst_mod << 5) | (h->overlay1.dst_port << 0));
}
#endif

#ifdef  PLISIM          /* these routines are only used by pcid */
static void dcb9_status_init(dcb_t *dcb)
{
    uint32      *d = (uint32 *)dcb;

    d[5] = d[6] = d[7] = d[8] = d[9] = d[10] = 0;
}
SETFUNCFIELD(9, xfercount, count, uint32 count, count)
SETFUNCFIELD(9, rx_start, start, int val, val ? 1 : 0)
SETFUNCFIELD(9, rx_end, end, int val, val ? 1 : 0)
SETFUNCFIELD(9, rx_error, error, int val, val ? 1 : 0)
SETFUNCEXPRIGNORE(9, rx_crc, int val, ignore)
#ifdef BCM_FIREBOLT_SUPPORT
SETFUNCFIELD(9, rx_ingport, srcport, int val, val)
#endif /* BCM_FIREBOLT_SUPPORT */
#ifdef BCM_EASYRIDER_SUPPORT
SETFUNCFIELD(10, rx_ingport, srcport, int val, val)
#endif /* BCM_EASYRIDER_SUPPORT */
#ifdef BCM_BRADLEY_SUPPORT
SETFUNCFIELD(11, rx_ingport, srcport, int val, val)
#endif /* BCM_BRADLEY_SUPPORT */
#endif  /* PLISIM */

#ifdef BROADCOM_DEBUG
static void
dcb9_dump(int unit, dcb_t *dcb, char *prefix, int tx)
{
    uint32      *p;
    int         i, size;
    dcb9_t *d = (dcb9_t *)dcb;
    char        ps[((DCB_MAX_SIZE/sizeof(uint32))*9)+1];
#if defined(LE_HOST)
    uint32  hgh[4];
    uint8 *h = (uint8 *)&hgh[0];

#if defined(BCM_BRADLEY_SUPPORT) || defined(BCM_RAPTOR_SUPPORT)
#if defined(BCM_BRADLEY_SUPPORT)
            dcb11_t *d11 = (dcb11_t *)d;
#elif defined(BCM_RAPTOR_SUPPORT)
            dcb12_t *d11 = (dcb12_t *)d;
#endif
    hgh[3] = soc_htonl(d11->mh3);
#endif
    hgh[0] = soc_htonl(d->mh0);
    hgh[1] = soc_htonl(d->mh1);
    hgh[2] = soc_htonl(d->mh2);
#else
    uint8 *h = (uint8 *)&d->mh0;
#endif

    p = (uint32 *)dcb;
    size = SOC_DCB_SIZE(unit) / sizeof(uint32);
    for (i = 0; i < size; i++) {
        sal_sprintf(&ps[i*9], "%08x ", p[i]);
    }
    soc_cm_print("%s\t%s\n", prefix, ps);
    if ((SOC_DCB_HG_GET(unit, dcb)) || (SOC_DCB_RX_START_GET(unit, dcb))) {
        soc_dma_higig_dump(unit, prefix, h, 0, 0, NULL);
    }
    soc_cm_print(
        "%s\ttype %d %ssg %schain %sreload %shg %sstat %spause %spurge\n",
        prefix,
        SOC_DCB_TYPE(unit),
        SOC_DCB_SG_GET(unit, dcb) ? "" : "!",
        SOC_DCB_CHAIN_GET(unit, dcb) ? "" : "!",
        SOC_DCB_RELOAD_GET(unit, dcb) ? "" : "!",
        SOC_DCB_HG_GET(unit, dcb) ? "" : "!",
        SOC_DCB_STAT_GET(unit, dcb) ? "" : "!",
        d->c_pause ? "" : "!",
        SOC_DCB_PURGE_GET(unit, dcb) ? "" : "!");
    soc_cm_print(
        "%s\taddr %p reqcount %d xfercount %d\n",
        prefix,
        (void *)SOC_DCB_ADDR_GET(unit, dcb),
        SOC_DCB_REQCOUNT_GET(unit, dcb),
        SOC_DCB_XFERCOUNT_GET(unit, dcb));
    if (!tx) {
        soc_cm_print(
            "%s\t%sdone %sstart %send %serror\n",
            prefix,
            SOC_DCB_DONE_GET(unit, dcb) ? "" : "!",
            SOC_DCB_RX_START_GET(unit, dcb) ? "" : "!",
            SOC_DCB_RX_END_GET(unit, dcb) ? "" : "!",
            SOC_DCB_RX_ERROR_GET(unit, dcb) ? "" : "!");
    }
#ifdef BCM_FIREBOLT_SUPPORT
    if ((!tx) && (SOC_DCB_RX_START_GET(unit, dcb)) &&
        (SOC_DCB_TYPE(unit) == 9)) {
        dcb0_reason_dump(unit, dcb, prefix);
        soc_cm_print(
            "%s  %sadd_vid %sbpdu %scell_error %schg_tos %semirror %simirror\n",
            prefix,
            d->add_vid ? "" : "!",
            d->bpdu ? "" : "!",
            d->cell_error ? "" : "!",
            d->chg_tos ? "" : "!",
            d->emirror ? "" : "!",
            d->imirror ? "" : "!"
            );
        soc_cm_print(
            "%s  %sl3ipmc %sl3only %sl3uc %spkt_aged %spurge_cell %ssrc_hg\n",
            prefix,
            d->l3ipmc ? "" : "!",
            d->l3only ? "" : "!",
            d->l3uc ? "" : "!",
            d->pkt_aged ? "" : "!",
            d->purge_cell ? "" : "!",
            d->src_hg ? "" : "!"
            );
        soc_cm_print(
            "%s  %sswitch_pkt %sregen_crc %sdecap_iptunnel %sing_untagged\n",
            prefix,
            d->switch_pkt ? "" : "!",
            d->regen_crc ? "" : "!",
            d->decap_iptunnel ? "" : "!",
            d->ingress_untagged ? "" : "!"
            );
        soc_cm_print(
            "%s  cpu_cos=%d cos=%d l3_intf=%d mtp_index=%d reason=%08x\n",
            prefix,
            d->cpu_cos,
            d->cos,
            d->l3_intf,
            (d->mtp_index_hi << 2) | d->mtp_index_lo,
            d->reason
            );
        soc_cm_print(
            "%s  match_rule=%d nh_index=%d\n",
            prefix,
            d->match_rule,
            d->nh_index
            );
        soc_cm_print(
            "%s  srcport=%d dscp=%d outer_pri=%d outer_cfi=%d outer_vid=%d\n",
            prefix,
            d->srcport,
            (d->dscp_hi << 4) | d->dscp_lo,
            d->outer_pri,
            d->outer_cfi,
            d->outer_vid
            );
    }
#endif /* BCM_FIREBOLT_SUPPORT */
#ifdef BCM_EASYRIDER_SUPPORT
    if ((!tx) && (SOC_DCB_RX_START_GET(unit, dcb)) &&
        (SOC_DCB_TYPE(unit) == 10)) {
        dcb10_t *d = (dcb10_t *)dcb;
        dcb0_reason_dump(unit, dcb, prefix);
        soc_cm_print(
            "%s  %sadd_vid %sbpdu %scell_error %schg_tos %semirror %simirror\n",
            prefix,
            d->add_vid ? "" : "!",
            d->bpdu ? "" : "!",
            d->cell_error ? "" : "!",
            d->chg_tos ? "" : "!",
            d->emirror ? "" : "!",
            d->imirror ? "" : "!"
            );
        soc_cm_print(
            "%s  %sl3ipmc %sl3only %sl3uc %spkt_aged %spurge_cell %ssrc_hg\n",
            prefix,
            d->l3ipmc ? "" : "!",
            d->l3only ? "" : "!",
            d->l3uc ? "" : "!",
            d->pkt_aged ? "" : "!",
            d->purge_cell ? "" : "!",
            d->src_hg ? "" : "!"
            );
        soc_cm_print(
            "%s  %sswitch_pkt %sregen_crc %spkt_not_changed\n",
            prefix,
            d->switch_pkt ? "" : "!",
            d->regen_crc ? "" : "!",
            d->pkt_not_changed ? "" : "!"
            );
        soc_cm_print(
            "%s  cpu_cos=%d cos=%d l3_intf=%d mtp_index=%d reason=%08x\n",
            prefix,
            d->cpu_cos,
            d->cos,
            d->l3_intf,
            d->mtp_index,
            ((d->reason_hi << 6) | (d->reason_lo))
            );
        soc_cm_print(
            "%s  decap_iptunnel=%d match_rule=%d nh_index=%d\n",
            prefix,
            d->decap_iptunnel,
            d->match_rule,
            ((d->nh_index_hi << 1) | (d->nh_index_lo))
            );
        soc_cm_print(
            "%s  srcport=%d dscp=%d ecn=%d %sing_untagged\n",
            prefix,
            d->srcport,
            d->dscp,
            d->ecn,
            d->ingress_untagged ? "" : "!"
            );
    }
#endif  /* BCM_EASYRIDER_SUPPORT */
#ifdef BCM_BRADLEY_SUPPORT
    if ((!tx) && (SOC_DCB_RX_START_GET(unit, dcb)) &&
        (SOC_DCB_TYPE(unit) == 11)) {
        dcb11_t *d = (dcb11_t *)dcb;
        dcb0_reason_dump(unit, dcb, prefix);
        soc_cm_print(
            "%s  %sadd_vid %sbpdu %scell_error %schg_tos %semirror %simirror\n",
            prefix,
            d->add_vid ? "" : "!",
            d->bpdu ? "" : "!",
            d->cell_error ? "" : "!",
            d->chg_tos ? "" : "!",
            d->emirror ? "" : "!",
            d->imirror ? "" : "!"
            );
        soc_cm_print(
            "%s  %sl3ipmc %sl3only %sl3uc %spkt_aged %spurge_cell %ssrc_hg\n",
            prefix,
            d->l3ipmc ? "" : "!",
            d->l3only ? "" : "!",
            d->l3uc ? "" : "!",
            d->pkt_aged ? "" : "!",
            d->purge_cell ? "" : "!",
            d->src_hg ? "" : "!"
            );
        soc_cm_print(
            "%s  %sswitch_pkt %sregen_crc %sdecap_iptunnel %sing_untagged\n",
            prefix,
            d->switch_pkt ? "" : "!",
            d->regen_crc ? "" : "!",
            d->decap_iptunnel ? "" : "!",
            d->ingress_untagged ? "" : "!"
            );
        soc_cm_print(
            "%s  cpu_cos=%d cos=%d l3_intf=%d mtp_index=%d reason=%08x\n",
            prefix,
            d->cpu_cos,
            d->cos,
            d->l3_intf,
            d->mtp_index,
            d->reason
            );
        soc_cm_print(
            "%s  match_rule=%d nh_index=%d hg_type=%d\n",
            prefix,
            d->match_rule,
            d->nh_index,
            d->hg_type
            );
        soc_cm_print(
            "%s  srcport=%d dscp=%d outer_pri=%d outer_cfi=%d outer_vid=%d\n",
            prefix,
            d->srcport,
            d->dscp,
            d->outer_pri,
            d->outer_cfi,
            d->outer_vid
            );
    }
#endif /* BCM_BRADLEY_SUPPORT */
#if defined(BCM_RAPTOR_SUPPORT)
    if ((!tx) && (SOC_DCB_RX_START_GET(unit, dcb)) &&
        (SOC_DCB_TYPE(unit) == 12)) {
        dcb12_t *d = (dcb12_t *)dcb;
        dcb0_reason_dump(unit, dcb, prefix);
        soc_cm_print(
            "%s  %sdo_not_change_ttl %sadd_vid %sbpdu %scell_error %schg_tos\n",
            prefix,
            d->do_not_change_ttl ? "" : "!",
            d->add_vid ? "" : "!",
            d->bpdu ? "" : "!",
            d->cell_error ? "" : "!",
            d->chg_tos ? "" : "!"
            );
        soc_cm_print(
            "%s  %sl3uc %spkt_aged %spurge_cell %ssrc_hg %semirror %simirror\n",
            prefix,
            d->l3uc ? "" : "!",
            d->pkt_aged ? "" : "!",
            d->purge_cell ? "" : "!",
            d->src_hg ? "" : "!",
            d->emirror ? "" : "!",
            d->imirror ? "" : "!"
            );
        soc_cm_print(
            "%s  %sswitch_pkt %sregen_crc %sdecap_iptunnel %sing_untagged\n",
            prefix,
            d->switch_pkt ? "" : "!",
            d->regen_crc ? "" : "!",
            d->decap_iptunnel ? "" : "!",
            d->ingress_untagged ? "" : "!"
            );
        soc_cm_print(
            "%s  cpu_cos=%d cos=%d mtp_index=%d reason=%08x\n",
            prefix,
            d->cpu_cos,
            d->cos,
            d->mtp_index,
            d->reason
            );
        soc_cm_print(
            "%s  match_rule=%d nh_index=%d\n",
            prefix,
            d->match_rule,
            d->nh_index
            );
        soc_cm_print(
            "%s  srcport=%d dscp=%d outer_pri=%d outer_cfi=%d outer_vid=%d\n",
            prefix,
            d->srcport,
            d->dscp,
            d->outer_pri,
            d->outer_cfi,
            d->outer_vid
            );
    }
#endif /* BCM_RAPTOR_SUPPORT */
#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_FIREBOLT_SUPPORT)
    if ((!tx) && (SOC_DCB_RX_START_GET(unit, dcb)) &&
        (SOC_DCB_TYPE(unit) == 13)) {
        dcb13_t *d = (dcb13_t *)dcb;
        dcb0_reason_dump(unit, dcb, prefix);
        soc_cm_print(
            "%s  %sadd_vid %sbpdu %scell_error %schg_tos %semirror %simirror\n",
            prefix,
            d->add_vid ? "" : "!",
            d->bpdu ? "" : "!",
            d->cell_error ? "" : "!",
            d->chg_tos ? "" : "!",
            d->emirror ? "" : "!",
            d->imirror ? "" : "!"
            );
        soc_cm_print(
            "%s  %sl3ipmc %sl3only %sl3uc %spkt_aged %spurge_cell %ssrc_hg\n",
            prefix,
            d->l3ipmc ? "" : "!",
            d->l3only ? "" : "!",
            d->l3uc ? "" : "!",
            d->pkt_aged ? "" : "!",
            d->purge_cell ? "" : "!",
            d->src_hg ? "" : "!"
            );
        soc_cm_print(
            "%s  %sswitch_pkt %sregen_crc %sdecap_iptunnel %sing_untagged\n",
            prefix,
            d->switch_pkt ? "" : "!",
            d->regen_crc ? "" : "!",
            d->decap_iptunnel ? "" : "!",
            d->ingress_untagged ? "" : "!"
            );
        soc_cm_print(
            "%s  %sivlan_add\n",
            prefix,
            d->inner_vlan_add ? "" : "!"
            );
        soc_cm_print(
            "%s  cpu_cos=%d cos=%d l3_intf=%d mtp_index=%d reason=%08x\n",
            prefix,
            d->cpu_cos,
            d->cos,
            d->l3_intf,
            (d->mtp_index_hi << 2) | d->mtp_index_lo,
            d->reason
            );
        soc_cm_print(
            "%s  match_rule=%d nh_index=%d\n",
            prefix,
            d->match_rule,
            d->nh_index
            );
        soc_cm_print(
            "%s  srcport=%d dscp=%d outer_pri=%d outer_cfi=%d outer_vid=%d\n",
            prefix,
            d->srcport,
            (d->dscp_hi << 6) | d->dscp_lo,
            d->outer_pri,
            d->outer_cfi,
            d->outer_vid
            );
    }
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_FIREBOLT_SUPPORT */
#ifdef BCM_RAPTOR_SUPPORT
    if ((!tx) && (SOC_DCB_RX_START_GET(unit, dcb)) &&
        (SOC_DCB_TYPE(unit) == 15)) {
        dcb15_t *d = (dcb15_t *)dcb;
        dcb0_reason_dump(unit, dcb, prefix);
        soc_cm_print(
            "%s  %sadd_vid %sbpdu %scell_error %schg_tos %semirror %simirror\n",
            prefix,
            d->add_vid ? "" : "!",
            d->bpdu ? "" : "!",
            d->cell_error ? "" : "!",
            d->chg_tos ? "" : "!",
            d->emirror ? "" : "!",
            d->imirror ? "" : "!"
            );
        soc_cm_print(
            "%s  %sl3ipmc %sl3only %sl3uc %spkt_aged %spurge_cell %ssrc_hg\n",
            prefix,
            d->l3ipmc ? "" : "!",
            d->l3only ? "" : "!",
            d->l3uc ? "" : "!",
            d->pkt_aged ? "" : "!",
            d->purge_cell ? "" : "!",
            d->src_hg ? "" : "!"
            );
        soc_cm_print(
            "%s  %sswitch_pkt %sregen_crc %sdecap_iptunnel %sing_untagged\n",
            prefix,
            d->switch_pkt ? "" : "!",
            d->regen_crc ? "" : "!",
            d->decap_iptunnel ? "" : "!",
            d->ingress_untagged ? "" : "!"
            );
        soc_cm_print(
            "%s  cpu_cos=%d cos=%d l3_intf=%d mtp_index=%d reason=%08x\n",
            prefix,
            d->cpu_cos,
            d->cos,
            d->l3_intf,
            d->mtp_index,
            d->reason
            );
        soc_cm_print(
            "%s  match_rule=%d nh_index=%d\n",
            prefix,
            d->match_rule,
            d->nh_index
            );
        soc_cm_print(
            "%s  srcport=%d dscp=%d outer_pri=%d outer_cfi=%d outer_vid=%d\n",
            prefix,
            d->srcport,
            d->dscp,
            d->outer_pri,
            d->outer_cfi,
            d->outer_vid
            );
    }
#endif /* BCM_RAPTOR_SUPPORT */
#ifdef BCM_SCORPION_SUPPORT
    if ((!tx) && (SOC_DCB_RX_START_GET(unit, dcb)) &&
        (SOC_DCB_TYPE(unit) == 16)) {
        dcb16_t *d = (dcb16_t *)dcb;
        dcb0_reason_dump(unit, dcb, prefix);
        soc_cm_print(
            "%s  %sbpdu %scell_error %schg_tos %semirror %simirror\n",
            prefix,
            d->bpdu ? "" : "!",
            d->cell_error ? "" : "!",
            d->chg_tos ? "" : "!",
            d->emirror ? "" : "!",
            d->imirror ? "" : "!"
            );
        soc_cm_print(
            "%s  %sl3ipmc %sl3only %sl3uc %spkt_aged %spurge_cell %ssrc_hg\n",
            prefix,
            d->l3ipmc ? "" : "!",
            d->l3only ? "" : "!",
            d->l3uc ? "" : "!",
            d->pkt_aged ? "" : "!",
            d->purge_cell ? "" : "!",
            d->src_hg ? "" : "!"
            );
        soc_cm_print(
            "%s  %sswitch_pkt %sregen_crc %sdecap_iptunnel %sing_untagged\n",
            prefix,
            d->switch_pkt ? "" : "!",
            d->regen_crc ? "" : "!",
            d->decap_iptunnel ? "" : "!",
            d->ingress_untagged ? "" : "!"
            );
        soc_cm_print(
            "%s  cpu_cos=%d cos=%d l3_intf=%d mtp_index=%d reason=%08x\n",
            prefix,
            d->cpu_cos,
            d->cos,
            d->l3_intf,
            d->mtp_index,
            d->reason
            );
        soc_cm_print(
            "%s  match_rule=%d nh_index=%d hg_type=%d\n",
            prefix,
            d->match_rule,
            d->nh_index,
            d->hg_type
            );
        soc_cm_print(
            "%s  srcport=%d dscp=%d outer_pri=%d outer_cfi=%d outer_vid=%d\n",
            prefix,
            d->srcport,
            d->dscp,
            d->outer_pri,
            d->outer_cfi,
            d->outer_vid
            );
    }
#endif /* BCM_SCORPION_SUPPORT */
#ifdef BCM_HAWKEYE_SUPPORT
    if ((!tx) && (SOC_DCB_RX_START_GET(unit, dcb)) &&
        (SOC_DCB_TYPE(unit) == 17)) {
        dcb17_t *d = (dcb17_t *)dcb;
        dcb0_reason_dump(unit, dcb, prefix);
        soc_cm_print(
            "%s  %sadd_vid %sbpdu %scell_error %schg_tos %semirror %simirror\n",
            prefix,
            d->add_vid ? "" : "!",
            d->bpdu ? "" : "!",
            d->cell_error ? "" : "!",
            d->chg_tos ? "" : "!",
            d->emirror ? "" : "!",
            d->imirror ? "" : "!"
            );
        soc_cm_print(
            "%s  %sl3ipmc %sl3only %sl3uc %spkt_aged %spurge_cell %ssrc_hg\n",
            prefix,
            d->l3ipmc ? "" : "!",
            d->l3only ? "" : "!",
            d->l3uc ? "" : "!",
            d->pkt_aged ? "" : "!",
            d->purge_cell ? "" : "!",
            d->src_hg ? "" : "!"
            );
        soc_cm_print(
            "%s  %sswitch_pkt %sregen_crc %sdecap_iptunnel %sing_untagged\n",
            prefix,
            d->switch_pkt ? "" : "!",
            d->regen_crc ? "" : "!",
            d->decap_iptunnel ? "" : "!",
            d->ingress_untagged ? "" : "!"
            );
        soc_cm_print(
            "%s  cpu_cos=%d l3_intf=%d mtp_index=%d reason=%08x\n",
            prefix,
            d->cpu_cos,
            d->l3_intf,
            d->mtp_index,
            d->reason
            );
        soc_cm_print(
            "%s  match_rule=%d nh_index=%d\n",
            prefix,
            d->match_rule,
            d->nh_index
            );
        soc_cm_print(
            "%s  srcport=%d dscp=%d outer_pri=%d outer_cfi=%d outer_vid=%d\n",
            prefix,
            d->srcport,
            d->dscp,
            d->outer_pri,
            d->outer_cfi,
            d->outer_vid
            );
    }
#endif /* BCM_HAWKEYE_SUPPORT */
#ifdef BCM_RAPTOR_SUPPORT
    if ((!tx) && (SOC_DCB_RX_START_GET(unit, dcb)) &&
        (SOC_DCB_TYPE(unit) == 18)) {
        dcb18_t *d = (dcb18_t *)dcb;
        dcb0_reason_dump(unit, dcb, prefix);
        soc_cm_print(
            "%s  %sadd_vid %sbpdu %scell_error %schg_tos %semirror %simirror\n",
            prefix,
            d->add_vid ? "" : "!",
            d->bpdu ? "" : "!",
            d->cell_error ? "" : "!",
            d->chg_tos ? "" : "!",
            d->emirror ? "" : "!",
            d->imirror ? "" : "!"
            );
        soc_cm_print(
            "%s  %sl3ipmc %sl3only %sl3uc %spkt_aged %spurge_cell %ssrc_hg\n",
            prefix,
            d->l3ipmc ? "" : "!",
            d->l3only ? "" : "!",
            d->l3uc ? "" : "!",
            d->pkt_aged ? "" : "!",
            d->purge_cell ? "" : "!",
            d->src_hg ? "" : "!"
            );
        soc_cm_print(
            "%s  %sswitch_pkt %sregen_crc %sdecap_iptunnel %sing_untagged\n",
            prefix,
            d->switch_pkt ? "" : "!",
            d->regen_crc ? "" : "!",
            d->decap_iptunnel ? "" : "!",
            d->ingress_untagged ? "" : "!"
            );
        soc_cm_print(
            "%s  cpu_cos=%d cos=%d l3_intf=%d mtp_index=%d reason=%08x\n",
            prefix,
            d->cpu_cos,
            d->cos,
            d->l3_intf,
            d->mtp_index,
            d->reason
            );
        soc_cm_print(
            "%s  match_rule=%d nh_index=%d\n",
            prefix,
            d->match_rule,
            d->nh_index
            );
        soc_cm_print(
            "%s  srcport=%d dscp=%d outer_pri=%d outer_cfi=%d outer_vid=%d\n",
            prefix,
            d->srcport,
            d->dscp,
            d->outer_pri,
            d->outer_cfi,
            d->outer_vid
            );
    }
#endif /* BCM_RAPTOR_SUPPORT */
}
#endif /* BROADCOM_DEBUG */

#ifdef BCM_FIREBOLT_SUPPORT
dcb_op_t dcb9_op = {
    9,
    sizeof(dcb9_t),
    dcb9_rx_reason_map,
    NULL,
    dcb9_init,
    dcb9_addtx,
    dcb9_addrx,
    dcb9_intrinfo,
    dcb9_reqcount_set,
    dcb9_reqcount_get,
    dcb9_xfercount_get,
    dcb0_addr_set,
    dcb0_addr_get,
    dcb0_paddr_get,
    dcb9_done_set,
    dcb9_done_get,
    dcb9_sg_set,
    dcb9_sg_get,
    dcb9_chain_set,
    dcb9_chain_get,
    dcb9_reload_set,
    dcb9_reload_get,
    dcb9_tx_l2pbm_set,
    dcb9_tx_utpbm_set,
    dcb9_tx_l3pbm_set,
    dcb9_tx_crc_set,
    dcb9_tx_cos_set,
    dcb9_tx_destmod_set,
    dcb9_tx_destport_set,
    dcb9_tx_opcode_set,
    dcb9_tx_srcmod_set,
    dcb9_tx_srcport_set,
    dcb9_tx_prio_set,
    dcb9_tx_pfm_set,
    dcb9_rx_untagged_get,
    dcb9_rx_crc_get,
    dcb9_rx_cos_get,
    dcb9_rx_destmod_get,
    dcb9_rx_destport_get,
    dcb9_rx_opcode_get,
    dcb9_rx_classtag_get,
    dcb9_rx_matchrule_get,
    dcb9_rx_start_get,
    dcb9_rx_end_get,
    dcb9_rx_error_get,
    dcb9_rx_prio_get,
    dcb9_rx_reason_get,
    dcb9_rx_reason_hi_get,
    dcb9_rx_ingport_get,
    dcb9_rx_srcport_get,
    dcb9_rx_srcmod_get,
    dcb9_rx_mcast_get,
    dcb9_rx_vclabel_get,
    dcb9_rx_mirror_get,
    dcb9_rx_timestamp_get,
    dcb9_rx_timestamp_upper_get,
    dcb9_hg_set,
    dcb9_hg_get,
    dcb9_stat_set,
    dcb9_stat_get,
    dcb9_purge_set,
    dcb9_purge_get,
    dcb9_mhp_get,
    dcb9_outer_vid_get,
    dcb9_outer_pri_get,
    dcb9_outer_cfi_get,
    dcb9_inner_vid_get,
    dcb9_inner_pri_get,
    dcb9_inner_cfi_get,
    dcb9_rx_bpdu_get,
    dcb9_rx_egr_to_cpu_get,
#ifdef  BROADCOM_DEBUG
    dcb9_tx_l2pbm_get,
    dcb9_tx_utpbm_get,
    dcb9_tx_l3pbm_get,
    dcb9_tx_crc_get,
    dcb9_tx_cos_get,
    dcb9_tx_destmod_get,
    dcb9_tx_destport_get,
    dcb9_tx_opcode_get,
    dcb9_tx_srcmod_get,
    dcb9_tx_srcport_get,
    dcb9_tx_prio_get,
    dcb9_tx_pfm_get,

    dcb9_dump,
    dcb0_reason_dump,
#endif /* BROADCOM_DEBUG */
#ifdef  PLISIM
    dcb9_status_init,
    dcb9_xfercount_set,
    dcb9_rx_start_set,
    dcb9_rx_end_set,
    dcb9_rx_error_set,
    dcb9_rx_crc_set,
    dcb9_rx_ingport_set,
#endif /* PLISIM */
};
#endif /* BCM_FIREBOLT_SUPPORT */

#ifdef BCM_EASYRIDER_SUPPORT
static soc_rx_reason_t
dcb10_rx_reason_map[] = {
    socRxReasonUnknownVlan,        /* Offset 0 */
    socRxReasonL2SourceMiss,       /* Offset 1 */
    socRxReasonL2DestMiss,         /* Offset 2 */
    socRxReasonL2Move,             /* Offset 3 */
    socRxReasonL2Cpu,              /* Offset 4 */
    socRxReasonSampleSource,       /* Offset 5 */
    socRxReasonSampleDest,         /* Offset 6 */
    socRxReasonL3SourceMiss,       /* Offset 7 */
    socRxReasonL3DestMiss,         /* Offset 8 */
    socRxReasonL3SourceMove,       /* Offset 9 */
    socRxReasonMcastMiss,          /* Offset 10 */
    socRxReasonIpMcastMiss,        /* Offset 11 */
    socRxReasonFilterMatch,        /* Offset 12 */
    socRxReasonL3HeaderError,      /* Offset 13 */
    socRxReasonProtocol,           /* Offset 14 */
    socRxReasonDosAttack,          /* Offset 15 */
    socRxReasonMartianAddr,        /* Offset 16 */
    socRxReasonTunnelError,        /* Offset 17 */
    socRxReasonL2MtuFail,          /* Offset 18 */
    socRxReasonIcmpRedirect,       /* Offset 19 */
    socRxReasonL3Slowpath,         /* Offset 20 */
    socRxReasonIngressFilter,      /* Offset 21 */
    socRxReasonGreChecksum,        /* Offset 22 */
    socRxReasonGreSourceRoute,     /* Offset 23 */
    socRxReasonUdpChecksum,        /* Offset 24 */
    socRxReasonMplsError,          /* Offset 25 */
    socRxReasonInvalid,            /* Offset 26 */
    socRxReasonInvalid,            /* Offset 27 */
    socRxReasonInvalid,            /* Offset 28 */
    socRxReasonInvalid,            /* Offset 29 */
    socRxReasonInvalid,            /* Offset 30 */
    socRxReasonInvalid             /* Offset 31 */
};

dcb_op_t dcb10_op = {
    10,
    sizeof(dcb10_t),
    dcb10_rx_reason_map,
    NULL,
    dcb9_init,
    dcb9_addtx,
    dcb9_addrx,
    dcb9_intrinfo,
    dcb9_reqcount_set,
    dcb9_reqcount_get,
    dcb9_xfercount_get,
    dcb0_addr_set,
    dcb0_addr_get,
    dcb0_paddr_get,
    dcb9_done_set,
    dcb9_done_get,
    dcb9_sg_set,
    dcb9_sg_get,
    dcb9_chain_set,
    dcb9_chain_get,
    dcb9_reload_set,
    dcb9_reload_get,
    dcb9_tx_l2pbm_set,
    dcb9_tx_utpbm_set,
    dcb9_tx_l3pbm_set,
    dcb9_tx_crc_set,
    dcb9_tx_cos_set,
    dcb9_tx_destmod_set,
    dcb9_tx_destport_set,
    dcb9_tx_opcode_set,
    dcb9_tx_srcmod_set,
    dcb9_tx_srcport_set,
    dcb9_tx_prio_set,
    dcb9_tx_pfm_set,
    dcb9_rx_untagged_get,
    dcb9_rx_crc_get,
    dcb9_rx_cos_get,
    dcb9_rx_destmod_get,
    dcb9_rx_destport_get,
    dcb9_rx_opcode_get,
    dcb9_rx_classtag_get,
    dcb10_rx_matchrule_get,
    dcb9_rx_start_get,
    dcb9_rx_end_get,
    dcb9_rx_error_get,
    dcb9_rx_prio_get,
    dcb10_rx_reason_get,
    dcb10_rx_reason_hi_get,
    dcb10_rx_ingport_get,
    dcb9_rx_srcport_get,
    dcb9_rx_srcmod_get,
    dcb9_rx_mcast_get,
    dcb10_rx_vclabel_get,
    dcb10_rx_mirror_get,
    dcb10_rx_timestamp_get,
    dcb10_rx_timestamp_upper_get,
    dcb9_hg_set,
    dcb9_hg_get,
    dcb9_stat_set,
    dcb9_stat_get,
    dcb9_purge_set,
    dcb9_purge_get,
    dcb9_mhp_get,
    dcb10_outer_vid_get,
    dcb10_outer_pri_get,
    dcb10_outer_cfi_get,
    dcb9_inner_vid_get,
    dcb9_inner_pri_get,
    dcb9_inner_cfi_get,
    dcb9_rx_bpdu_get,
    dcb9_rx_egr_to_cpu_get,
#ifdef  BROADCOM_DEBUG
    dcb9_tx_l2pbm_get,
    dcb9_tx_utpbm_get,
    dcb9_tx_l3pbm_get,
    dcb9_tx_crc_get,
    dcb9_tx_cos_get,
    dcb9_tx_destmod_get,
    dcb9_tx_destport_get,
    dcb9_tx_opcode_get,
    dcb9_tx_srcmod_get,
    dcb9_tx_srcport_get,
    dcb9_tx_prio_get,
    dcb9_tx_pfm_get,

    dcb9_dump,
    dcb0_reason_dump,
#endif /* BROADCOM_DEBUG */
#ifdef  PLISIM
    dcb9_status_init,
    dcb9_xfercount_set,
    dcb9_rx_start_set,
    dcb9_rx_end_set,
    dcb9_rx_error_set,
    dcb9_rx_crc_set,
    dcb10_rx_ingport_set,
#endif /* PLISIM */
};
#endif /* BCM_EASYRIDER_SUPPORT */

#ifdef BCM_BRADLEY_SUPPORT
static soc_rx_reason_t
dcb11_rx_reason_map[] = {
    socRxReasonUnknownVlan,        /* Offset 0 */
    socRxReasonL2SourceMiss,       /* Offset 1 */
    socRxReasonL2DestMiss,         /* Offset 2 */
    socRxReasonL2Move,             /* Offset 3 */
    socRxReasonL2Cpu,              /* Offset 4 */
    socRxReasonSampleSource,       /* Offset 5 */
    socRxReasonSampleDest,         /* Offset 6 */
    socRxReasonL3SourceMiss,       /* Offset 7 */
    socRxReasonL3DestMiss,         /* Offset 8 */
    socRxReasonL3SourceMove,       /* Offset 9 */
    socRxReasonMcastMiss,          /* Offset 10 */
    socRxReasonIpMcastMiss,        /* Offset 11 */
    socRxReasonFilterMatch,        /* Offset 12 */
    socRxReasonL3HeaderError,      /* Offset 13 */
    socRxReasonProtocol,           /* Offset 14 */
    socRxReasonDosAttack,          /* Offset 15 */
    socRxReasonMartianAddr,        /* Offset 16 */
    socRxReasonTunnelError,        /* Offset 17 */
    socRxReasonL2MtuFail,          /* Offset 18 */
    socRxReasonIcmpRedirect,       /* Offset 19 */
    socRxReasonL3Slowpath,         /* Offset 20 */
    socRxReasonParityError,        /* Offset 21 */
    socRxReasonL3MtuFail,          /* Offset 22 */
    socRxReasonHigigHdrError,      /* Offset 23 */
    socRxReasonMcastIdxError,      /* Offset 24 */
    socRxReasonInvalid,            /* Offset 25 */
    socRxReasonInvalid,            /* Offset 26 */
    socRxReasonInvalid,            /* Offset 27 */
    socRxReasonInvalid,            /* Offset 28 */
    socRxReasonInvalid,            /* Offset 29 */
    socRxReasonInvalid,            /* Offset 30 */
    socRxReasonInvalid             /* Offset 31 */
};

dcb_op_t dcb11_op = {
    11,
    sizeof(dcb11_t),
    dcb11_rx_reason_map,
    NULL,
    dcb9_init,
    dcb9_addtx,
    dcb9_addrx,
    dcb9_intrinfo,
    dcb9_reqcount_set,
    dcb9_reqcount_get,
    dcb9_xfercount_get,
    dcb0_addr_set,
    dcb0_addr_get,
    dcb0_paddr_get,
    dcb9_done_set,
    dcb9_done_get,
    dcb9_sg_set,
    dcb9_sg_get,
    dcb9_chain_set,
    dcb9_chain_get,
    dcb9_reload_set,
    dcb9_reload_get,
    dcb9_tx_l2pbm_set,
    dcb9_tx_utpbm_set,
    dcb9_tx_l3pbm_set,
    dcb9_tx_crc_set,
    dcb9_tx_cos_set,
    dcb9_tx_destmod_set,
    dcb9_tx_destport_set,
    dcb9_tx_opcode_set,
    dcb9_tx_srcmod_set,
    dcb9_tx_srcport_set,
    dcb9_tx_prio_set,
    dcb9_tx_pfm_set,
    dcb11_rx_untagged_get,
    dcb9_rx_crc_get,
    dcb11_rx_cos_get,
    dcb11_rx_destmod_get,
    dcb11_rx_destport_get,
    dcb11_rx_opcode_get,
    dcb9_rx_classtag_get,
    dcb11_rx_matchrule_get,
    dcb9_rx_start_get,
    dcb9_rx_end_get,
    dcb9_rx_error_get,
    dcb11_rx_prio_get,
    dcb11_rx_reason_get,
    dcb11_rx_reason_hi_get,
    dcb11_rx_ingport_get,
    dcb11_rx_srcport_get,
    dcb11_rx_srcmod_get,
    dcb9_rx_mcast_get,
    dcb9_rx_vclabel_get,
    dcb11_rx_mirror_get,
    dcb11_rx_timestamp_get,
    dcb11_rx_timestamp_upper_get,
    dcb9_hg_set,
    dcb9_hg_get,
    dcb11_stat_set,
    dcb11_stat_get,
    dcb11_purge_set,
    dcb11_purge_get,
    dcb9_mhp_get,
    dcb11_outer_vid_get,
    dcb11_outer_pri_get,
    dcb11_outer_cfi_get,
    dcb9_inner_vid_get,
    dcb9_inner_pri_get,
    dcb9_inner_cfi_get,
    dcb11_rx_bpdu_get,
    dcb11_rx_egr_to_cpu_get,
#ifdef  BROADCOM_DEBUG
    dcb9_tx_l2pbm_get,
    dcb9_tx_utpbm_get,
    dcb9_tx_l3pbm_get,
    dcb9_tx_crc_get,
    dcb9_tx_cos_get,
    dcb9_tx_destmod_get,
    dcb9_tx_destport_get,
    dcb9_tx_opcode_get,
    dcb9_tx_srcmod_get,
    dcb9_tx_srcport_get,
    dcb9_tx_prio_get,
    dcb9_tx_pfm_get,

    dcb9_dump,
    dcb0_reason_dump,
#endif /* BROADCOM_DEBUG */
#ifdef  PLISIM
    dcb9_status_init,
    dcb9_xfercount_set,
    dcb9_rx_start_set,
    dcb9_rx_end_set,
    dcb9_rx_error_set,
    dcb9_rx_crc_set,
    dcb11_rx_ingport_set,
#endif /* PLISIM */
};
#endif /* BCM_BRADLEY_SUPPORT */

#if defined(BCM_RAPTOR_SUPPORT)
static soc_rx_reason_t
dcb12_rx_reason_map[] = {
    socRxReasonUnknownVlan,        /* Offset 0 */
    socRxReasonL2SourceMiss,       /* Offset 1 */
    socRxReasonL2DestMiss,         /* Offset 2 */
    socRxReasonL2Move,             /* Offset 3 */
    socRxReasonL2Cpu,              /* Offset 4 */
    socRxReasonSampleSource,       /* Offset 5 */
    socRxReasonSampleDest,         /* Offset 6 */
    socRxReasonL3SourceMiss,       /* Offset 7 */
    socRxReasonL3DestMiss,         /* Offset 8 */
    socRxReasonL3SourceMove,       /* Offset 9 */
    socRxReasonMcastMiss,          /* Offset 10 */
    socRxReasonIpMcastMiss,        /* Offset 11 */
    socRxReasonFilterMatch,        /* Offset 12 */
    socRxReasonL3HeaderError,      /* Offset 13 */
    socRxReasonProtocol,           /* Offset 14 */
    socRxReasonDosAttack,          /* Offset 15 */
    socRxReasonMartianAddr,        /* Offset 16 */
    socRxReasonTunnelError,        /* Offset 17 */
    socRxReasonL2MtuFail,          /* Offset 18 */
    socRxReasonIcmpRedirect,       /* Offset 19 */
    socRxReasonL3Slowpath,         /* Offset 20 */
    socRxReasonParityError,        /* Offset 21 */
    socRxReasonL3MtuFail,          /* Offset 22 */
    socRxReasonHigigHdrError,      /* Offset 23 */
    socRxReasonMcastIdxError,      /* Offset 24 */
    socRxReasonL2LearnLimit,       /* Offset 25 */
    socRxReasonInvalid,            /* Offset 26 */
    socRxReasonInvalid,            /* Offset 27 */
    socRxReasonInvalid,            /* Offset 28 */
    socRxReasonInvalid,            /* Offset 29 */
    socRxReasonInvalid,            /* Offset 30 */
    socRxReasonInvalid             /* Offset 31 */
};

dcb_op_t dcb12_op = {
    12,
    sizeof(dcb12_t),
    dcb12_rx_reason_map,
    NULL,
    dcb9_init,
    dcb9_addtx,
    dcb9_addrx,
    dcb9_intrinfo,
    dcb9_reqcount_set,
    dcb9_reqcount_get,
    dcb9_xfercount_get,
    dcb0_addr_set,
    dcb0_addr_get,
    dcb0_paddr_get,
    dcb9_done_set,
    dcb9_done_get,
    dcb9_sg_set,
    dcb9_sg_get,
    dcb9_chain_set,
    dcb9_chain_get,
    dcb9_reload_set,
    dcb9_reload_get,
    dcb9_tx_l2pbm_set,
    dcb9_tx_utpbm_set,
    dcb9_tx_l3pbm_set,
    dcb9_tx_crc_set,
    dcb9_tx_cos_set,
    dcb9_tx_destmod_set,
    dcb9_tx_destport_set,
    dcb9_tx_opcode_set,
    dcb9_tx_srcmod_set,
    dcb9_tx_srcport_set,
    dcb9_tx_prio_set,
    dcb9_tx_pfm_set,
    dcb12_rx_untagged_get,
    dcb12_rx_crc_get,
    dcb12_rx_cos_get,
    dcb12_rx_destmod_get,
    dcb12_rx_destport_get,
    dcb12_rx_opcode_get,
    dcb12_rx_classtag_get,
    dcb12_rx_matchrule_get,
    dcb9_rx_start_get,
    dcb9_rx_end_get,
    dcb9_rx_error_get,
    dcb12_rx_prio_get,
    dcb12_rx_reason_get,
    dcb12_rx_reason_hi_get,
    dcb12_rx_ingport_get,
    dcb12_rx_srcport_get,
    dcb12_rx_srcmod_get,
    dcb12_rx_mcast_get,
    dcb12_rx_vclabel_get,
    dcb12_rx_mirror_get,
    dcb12_rx_timestamp_get,
    dcb12_rx_timestamp_upper_get,
    dcb9_hg_set,
    dcb9_hg_get,
    dcb9_stat_set,
    dcb9_stat_get,
    dcb9_purge_set,
    dcb9_purge_get,
    dcb9_mhp_get,
    dcb12_outer_vid_get,
    dcb12_outer_pri_get,
    dcb12_outer_cfi_get,
    dcb9_inner_vid_get,
    dcb9_inner_pri_get,
    dcb9_inner_cfi_get,
    dcb12_rx_bpdu_get,
    dcb12_rx_egr_to_cpu_get,
#ifdef  BROADCOM_DEBUG
    dcb9_tx_l2pbm_get,
    dcb9_tx_utpbm_get,
    dcb9_tx_l3pbm_get,
    dcb9_tx_crc_get,
    dcb9_tx_cos_get,
    dcb9_tx_destmod_get,
    dcb9_tx_destport_get,
    dcb9_tx_opcode_get,
    dcb9_tx_srcmod_get,
    dcb9_tx_srcport_get,
    dcb9_tx_prio_get,
    dcb9_tx_pfm_get,

    dcb9_dump,
    dcb0_reason_dump,
#endif /* BROADCOM_DEBUG */
#ifdef  PLISIM
    dcb9_status_init,
    dcb9_xfercount_set,
    dcb9_rx_start_set,
    dcb9_rx_end_set,
    dcb9_rx_error_set,
    dcb12_rx_crc_set,
    dcb12_rx_ingport_set,
#endif /* PLISIM */
};
#endif /* BCM_RAPTOR_SUPPORT */

#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_FIREBOLT_SUPPORT)
static soc_rx_reason_t
dcb13_rx_reason_map[] = {
    socRxReasonUnknownVlan,        /* Offset 0 */
    socRxReasonL2SourceMiss,       /* Offset 1 */
    socRxReasonL2DestMiss,         /* Offset 2 */
    socRxReasonL2Move,             /* Offset 3 */
    socRxReasonL2Cpu,              /* Offset 4 */
    socRxReasonSampleSource,       /* Offset 5 */
    socRxReasonSampleDest,         /* Offset 6 */
    socRxReasonL3SourceMiss,       /* Offset 7 */
    socRxReasonL3DestMiss,         /* Offset 8 */
    socRxReasonL3SourceMove,       /* Offset 9 */
    socRxReasonMcastMiss,          /* Offset 10 */
    socRxReasonIpMcastMiss,        /* Offset 11 */
    socRxReasonFilterMatch,        /* Offset 12 */
    socRxReasonL3HeaderError,      /* Offset 13 */
    socRxReasonProtocol,           /* Offset 14 */
    socRxReasonDosAttack,          /* Offset 15 */
    socRxReasonMartianAddr,        /* Offset 16 */
    socRxReasonTunnelError,        /* Offset 17 */
    socRxReasonL2MtuFail,          /* Offset 18 */
    socRxReasonIcmpRedirect,       /* Offset 19 */
    socRxReasonL3Slowpath,         /* Offset 20 */
    socRxReasonParityError,        /* Offset 21 */
    socRxReasonL3MtuFail,          /* Offset 22 */
    socRxReasonHigigHdrError,      /* Offset 23 */
    socRxReasonVlanFilterMatch,    /* Offset 24 */
    socRxReasonInvalid,            /* Offset 25 */
    socRxReasonInvalid,            /* Offset 26 */
    socRxReasonInvalid,            /* Offset 27 */
    socRxReasonInvalid,            /* Offset 28 */
    socRxReasonInvalid,            /* Offset 29 */
    socRxReasonInvalid,            /* Offset 30 */
    socRxReasonInvalid             /* Offset 31 */
};

GETFUNCERR(13, rx_timestamp)
GETFUNCERR(13, rx_timestamp_upper)

dcb_op_t dcb13_op = {
    13,
    sizeof(dcb13_t),
    dcb13_rx_reason_map,
    NULL,
    dcb9_init,
    dcb9_addtx,
    dcb9_addrx,
    dcb9_intrinfo,
    dcb9_reqcount_set,
    dcb9_reqcount_get,
    dcb9_xfercount_get,
    dcb0_addr_set,
    dcb0_addr_get,
    dcb0_paddr_get,
    dcb9_done_set,
    dcb9_done_get,
    dcb9_sg_set,
    dcb9_sg_get,
    dcb9_chain_set,
    dcb9_chain_get,
    dcb9_reload_set,
    dcb9_reload_get,
    dcb9_tx_l2pbm_set,
    dcb9_tx_utpbm_set,
    dcb9_tx_l3pbm_set,
    dcb9_tx_crc_set,
    dcb9_tx_cos_set,
    dcb9_tx_destmod_set,
    dcb9_tx_destport_set,
    dcb9_tx_opcode_set,
    dcb9_tx_srcmod_set,
    dcb9_tx_srcport_set,
    dcb9_tx_prio_set,
    dcb9_tx_pfm_set,
    dcb13_rx_untagged_get,
    dcb9_rx_crc_get,
    dcb9_rx_cos_get,
    dcb9_rx_destmod_get,
    dcb9_rx_destport_get,
    dcb9_rx_opcode_get,
    dcb9_rx_classtag_get,
    dcb9_rx_matchrule_get,
    dcb9_rx_start_get,
    dcb9_rx_end_get,
    dcb9_rx_error_get,
    dcb9_rx_prio_get,
    dcb9_rx_reason_get,
    dcb9_rx_reason_hi_get,
    dcb9_rx_ingport_get,
    dcb9_rx_srcport_get,
    dcb9_rx_srcmod_get,
    dcb9_rx_mcast_get,
    dcb9_rx_vclabel_get,
    dcb9_rx_mirror_get,
    dcb13_rx_timestamp_get,
    dcb13_rx_timestamp_upper_get,
    dcb9_hg_set,
    dcb9_hg_get,
    dcb9_stat_set,
    dcb9_stat_get,
    dcb9_purge_set,
    dcb9_purge_get,
    dcb9_mhp_get,
    dcb13_outer_vid_get,
    dcb13_outer_pri_get,
    dcb13_outer_cfi_get,
    dcb9_inner_vid_get,
    dcb9_inner_pri_get,
    dcb9_inner_cfi_get,
    dcb9_rx_bpdu_get,
    dcb9_rx_egr_to_cpu_get,
#ifdef  BROADCOM_DEBUG
    dcb9_tx_l2pbm_get,
    dcb9_tx_utpbm_get,
    dcb9_tx_l3pbm_get,
    dcb9_tx_crc_get,
    dcb9_tx_cos_get,
    dcb9_tx_destmod_get,
    dcb9_tx_destport_get,
    dcb9_tx_opcode_get,
    dcb9_tx_srcmod_get,
    dcb9_tx_srcport_get,
    dcb9_tx_prio_get,
    dcb9_tx_pfm_get,

    dcb9_dump,
    dcb0_reason_dump,
#endif /* BROADCOM_DEBUG */
#ifdef  PLISIM
    dcb9_status_init,
    dcb9_xfercount_set,
    dcb9_rx_start_set,
    dcb9_rx_end_set,
    dcb9_rx_error_set,
    dcb9_rx_crc_set,
    dcb9_rx_ingport_set,
#endif /* PLISIM */
};
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_FIREBOLT_SUPPORT */

#if defined(BCM_TRIUMPH_SUPPORT) || defined(BCM_SCORPION_SUPPORT)
/*
 * DCB Type 14 Support
 */

static soc_rx_reason_t
dcb14_rx_reason_map[] = {
    socRxReasonUnknownVlan,        /* Offset 0 */
    socRxReasonL2SourceMiss,       /* Offset 1 */
    socRxReasonL2DestMiss,         /* Offset 2 */
    socRxReasonL2Move,             /* Offset 3 */
    socRxReasonL2Cpu,              /* Offset 4 */
    socRxReasonSampleSource,       /* Offset 5 */
    socRxReasonSampleDest,         /* Offset 6 */
    socRxReasonL3SourceMiss,       /* Offset 7 */
    socRxReasonL3DestMiss,         /* Offset 8 */
    socRxReasonL3SourceMove,       /* Offset 9 */
    socRxReasonMcastMiss,          /* Offset 10 */
    socRxReasonIpMcastMiss,        /* Offset 11 */
    socRxReasonFilterMatch,        /* Offset 12 */
    socRxReasonL3HeaderError,      /* Offset 13 */
    socRxReasonProtocol,           /* Offset 14 */
    socRxReasonDosAttack,          /* Offset 15 */
    socRxReasonMartianAddr,        /* Offset 16 */
    socRxReasonTunnelError,        /* Offset 17 */
    socRxReasonL2MtuFail,          /* Offset 18 */
    socRxReasonIcmpRedirect,       /* Offset 19 */
    socRxReasonL3Slowpath,         /* Offset 20 */
    socRxReasonParityError,        /* Offset 21 */
    socRxReasonL3MtuFail,          /* Offset 22 */
    socRxReasonHigigHdrError,      /* Offset 23 */
    socRxReasonMcastIdxError,      /* Offset 24 */
    socRxReasonVlanFilterMatch,    /* Offset 25 */
    socRxReasonClassBasedMove,     /* Offset 26 */
    socRxReasonL2LearnLimit,       /* Offset 27 */
    socRxReasonMplsLabelMiss,      /* Offset 28 */
    socRxReasonMplsInvalidAction,  /* Offset 29 */
    socRxReasonMplsInvalidPayload, /* Offset 30 */
    socRxReasonMplsTtl,            /* Offset 31 */
    socRxReasonMplsSequenceNumber, /* Offset 32 */
    socRxReasonL2NonUnicastMiss,   /* Offset 33 */
    socRxReasonNhop,               /* Offset 34 */
    socRxReasonInvalid,            /* Offset 35 */
    socRxReasonInvalid,            /* Offset 36 */
    socRxReasonInvalid,            /* Offset 37 */
    socRxReasonInvalid,            /* Offset 38 */
    socRxReasonInvalid,            /* Offset 39 */
    socRxReasonInvalid,            /* Offset 40 */
    socRxReasonInvalid,            /* Offset 41 */
    socRxReasonInvalid,            /* Offset 42 */
    socRxReasonInvalid,            /* Offset 43 */
    socRxReasonInvalid,            /* Offset 44 */
    socRxReasonInvalid,            /* Offset 45 */
    socRxReasonInvalid,            /* Offset 46 */
    socRxReasonInvalid,            /* Offset 47 */
    socRxReasonInvalid,            /* Offset 48 */
    socRxReasonInvalid,            /* Offset 49 */
    socRxReasonInvalid,            /* Offset 50 */
    socRxReasonInvalid,            /* Offset 51 */
    socRxReasonInvalid,            /* Offset 52 */
    socRxReasonInvalid,            /* Offset 53 */
    socRxReasonInvalid,            /* Offset 54 */
    socRxReasonInvalid,            /* Offset 55 */
    socRxReasonInvalid,            /* Offset 56 */
    socRxReasonInvalid,            /* Offset 57 */
    socRxReasonInvalid,            /* Offset 58 */
    socRxReasonInvalid,            /* Offset 59 */
    socRxReasonInvalid,            /* Offset 60 */
    socRxReasonInvalid,            /* Offset 61 */
    socRxReasonInvalid,            /* Offset 62 */
    socRxReasonInvalid             /* Offset 63 */
};

static void
dcb14_init(dcb_t *dcb)
{
    uint32      *d = (uint32 *)dcb;

    d[0] = d[1] = d[2] = d[3] = d[4] = 0;
    d[5] = d[6] = d[7] = d[8] = d[9] = d[10] = 0;
    d[11] = d[12] = d[13] = d[14] = d[15] = 0;
}

static int
dcb14_addtx(dv_t *dv, sal_vaddr_t addr, uint32 count,
            pbmp_t l2pbm, pbmp_t utpbm, pbmp_t l3pbm, uint32 flags, uint32 *hgh)
{
    dcb14_t     *d;     /* DCB */
    uint32      *di;    /* DCB integer pointer */
    uint32      paddr;  /* Packet buffer physical address */
    int         unaligned;
    int         unaligned_bytes;
    uint8       *unaligned_buffer;
    uint8       *aligned_buffer;

    d = (dcb14_t *)SOC_DCB_IDX2PTR(dv->dv_unit, dv->dv_dcb, dv->dv_vcnt);

    if (addr) {
        paddr = soc_cm_l2p(dv->dv_unit, (void *)addr);
    } else {
        paddr = 0;
    }

    if (dv->dv_vcnt > 0 && (dv->dv_flags & DV_F_COMBINE_DCB) &&
        (d[-1].c_sg != 0) &&
        (d[-1].addr + d[-1].c_count) == paddr &&
        d[-1].c_count + count <= DCB_MAX_REQCOUNT) {
        d[-1].c_count += count;
        return dv->dv_cnt - dv->dv_vcnt;
    }

    /*
     * A few chip revisions do not support 128 byte PCI bursts
     * correctly if the address is not word-aligned. In case
     * we encounter an unaligned address, we consume an extra
     * DCB to fixup the alignment.
     */
    do {
        if (dv->dv_vcnt >= dv->dv_cnt) {
            return SOC_E_FULL;
        }
        if (dv->dv_vcnt > 0) {      /* chain off previous dcb */
            d[-1].c_chain = 1;
        }

        di = (uint32 *)d;
        di[0] = di[1] = di[2] = di[3] = di[4] = 0;
        di[5] = di[6] = di[7] = di[8] = di[9] = di[10] = 0;
        di[11] = di[12] = di[13] = di[14] = di[15] = 0;

        d->addr = paddr;
        d->c_count = count;
        d->c_sg = 1;

        d->c_stat = 1;
        d->c_purge = SOC_DMA_PURGE_GET(flags);
        if (SOC_DMA_HG_GET(flags)) {
            soc_higig_hdr_t *mh = (soc_higig_hdr_t *)hgh;
            if (mh->overlay1.start == SOC_HIGIG2_START) {
                d->mh3 = soc_ntohl(hgh[3]);
            }
            d->c_hg = 1;
            d->mh0 = soc_ntohl(hgh[0]);
            d->mh1 = soc_ntohl(hgh[1]);
            d->mh2 = soc_ntohl(hgh[2]);
            d->mh3 = soc_ntohl(hgh[3]);
        }

        unaligned = 0;
        if (soc_feature(dv->dv_unit, soc_feature_pkt_tx_align)) {
            if (paddr & 0x3) {
                unaligned_bytes = 4 - (paddr & 0x3);
                unaligned_buffer = (uint8 *)addr;
                aligned_buffer = SOC_DV_TX_ALIGN(dv, dv->dv_vcnt);
                aligned_buffer[0] = unaligned_buffer[0];
                aligned_buffer[1] = unaligned_buffer[1];
                aligned_buffer[2] = unaligned_buffer[2];
                d->addr = soc_cm_l2p(dv->dv_unit, aligned_buffer);
                if (count > 3) {
                    d->c_count = unaligned_bytes;
                    paddr += unaligned_bytes;
                    count -= unaligned_bytes;
                    unaligned = 1;
                }
            }
        }

        dv->dv_vcnt += 1;

        d = (dcb14_t *)SOC_DCB_IDX2PTR(dv->dv_unit, dv->dv_dcb, dv->dv_vcnt);
    } while (unaligned);

    return dv->dv_cnt - dv->dv_vcnt;
}

static int
dcb14_addrx(dv_t *dv, sal_vaddr_t addr, uint32 count, uint32 flags)
{
    dcb14_t     *d;     /* DCB */
    uint32      *di;    /* DCB integer pointer */

    d = (dcb14_t *)SOC_DCB_IDX2PTR(dv->dv_unit, dv->dv_dcb, dv->dv_vcnt);

    if (dv->dv_vcnt > 0) {      /* chain off previous dcb */
        d[-1].c_chain = 1;
    }

    di = (uint32 *)d;
    di[0] = di[1] = di[2] = di[3] = di[4] = 0;
    di[5] = di[6] = di[7] = di[8] = di[9] = di[10] = 0;
    di[11] = di[12] = di[13] = di[14] = di[15] = 0;

    if (addr) {
        d->addr = soc_cm_l2p(dv->dv_unit, (void *)addr);
    }
    d->c_count = count;
    d->c_sg = 1;

    dv->dv_vcnt += 1;
    return dv->dv_cnt - dv->dv_vcnt;
}

static uint32
dcb14_intrinfo(int unit, dcb_t *dcb, int tx, uint32 *count)
{
    dcb14_t      *d = (dcb14_t *)dcb;     /*  DCB */
    uint32      f;                      /* SOC_DCB_INFO_* flags */

    if (!d->done) {
        return 0;
    }
    f = SOC_DCB_INFO_DONE;
    if (tx) {
        if (!d->c_sg) {
            f |= SOC_DCB_INFO_PKTEND;
        }
    } else {
        if (d->end) {
            f |= SOC_DCB_INFO_PKTEND;
        }
    }
    *count = d->count;
    return f;
}

SETFUNCFIELD(14, reqcount, c_count, uint32 count, count)
GETFUNCFIELD(14, reqcount, c_count)
GETFUNCFIELD(14, xfercount, count)
/* addr_set, addr_get, paddr_get - Same as DCB 0 */
SETFUNCFIELD(14, done, done, int val, val ? 1 : 0)
GETFUNCFIELD(14, done, done)
SETFUNCFIELD(14, sg, c_sg, int val, val ? 1 : 0)
GETFUNCFIELD(14, sg, c_sg)
SETFUNCFIELD(14, chain, c_chain, int val, val ? 1 : 0)
GETFUNCFIELD(14, chain, c_chain)
SETFUNCFIELD(14, reload, c_reload, int val, val ? 1 : 0)
GETFUNCFIELD(14, reload, c_reload)
SETFUNCERR(14, tx_l2pbm, pbmp_t)
SETFUNCERR(14, tx_utpbm, pbmp_t)
SETFUNCERR(14, tx_l3pbm, pbmp_t)
SETFUNCERR(14, tx_crc, int)
SETFUNCERR(14, tx_cos, int)
SETFUNCERR(14, tx_destmod, uint32)
SETFUNCERR(14, tx_destport, uint32)
SETFUNCERR(14, tx_opcode, uint32)
SETFUNCERR(14, tx_srcmod, uint32)
SETFUNCERR(14, tx_srcport, uint32)
SETFUNCERR(14, tx_prio, uint32)
SETFUNCERR(14, tx_pfm, uint32)
GETFUNCFIELD(14, rx_start, start)
GETFUNCFIELD(14, rx_end, end)
GETFUNCFIELD(14, rx_error, error)
GETFUNCFIELD(14, rx_cos, cpu_cos)
/* Fields extracted from MH/PBI */
GETHG2FUNCFIELD(14, rx_destmod, dst_mod)
GETHG2FUNCFIELD(14, rx_destport, dst_port)
GETHG2FUNCFIELD(14, rx_srcmod, src_mod)
GETHG2FUNCFIELD(14, rx_srcport, src_port)
GETHG2FUNCFIELD(14, rx_opcode, opcode)
GETHG2FUNCFIELD(14, rx_prio, vlan_pri) /* outer_pri */
GETHG2FUNCEXPR(14, rx_mcast, ((h->ppd_overlay1.dst_mod << 8) |
                              (h->ppd_overlay1.dst_port)))
GETHG2FUNCEXPR(14, rx_vclabel, ((h->ppd_overlay1.vc_label_19_16 << 16) |
                              (h->ppd_overlay1.vc_label_15_8 << 8) |
                              (h->ppd_overlay1.vc_label_7_0)))
GETHG2FUNCEXPR(14, rx_classtag, (h->ppd_overlay1.ppd_type != 1 ? 0 :
                                 (h->ppd_overlay2.ctag_hi << 8) |
                                 (h->ppd_overlay2.ctag_lo)))

GETFUNCUNITEXPR(14, rx_untagged, (IS_HG_PORT(unit, d->srcport) ?
                                  ((d->itag_status) ? 0 : 2) :
                                  ((d->itag_status & 0x2) ?
                                   ((d->itag_status & 0x1) ? 0 : 2) :
                                   ((d->itag_status & 0x1) ? 1 : 3))))
GETFUNCFIELD(14, rx_matchrule, match_rule)
GETFUNCFIELD(14, rx_reason, reason)
GETFUNCFIELD(14, rx_reason_hi, reason_hi)
GETFUNCFIELD(14, rx_ingport, srcport)
GETFUNCEXPR(14, rx_mirror, ((d->imirror) | (d->emirror)))
GETFUNCERR(14, rx_timestamp)
GETFUNCERR(14, rx_timestamp_upper)
SETFUNCFIELD(14, hg, c_hg, uint32 hg, hg)
GETFUNCFIELD(14, hg, c_hg)
SETFUNCFIELD(14, stat, c_stat, uint32 stat, stat)
GETFUNCFIELD(14, stat, c_stat)
SETFUNCFIELD(14, purge, c_purge, uint32 purge, purge)
GETFUNCFIELD(14, purge, c_purge)
GETFUNCFIELD(14, outer_vid, outer_vid)
GETFUNCFIELD(14, outer_pri, outer_pri)
GETFUNCFIELD(14, outer_cfi, outer_cfi)
GETFUNCFIELD(14, rx_bpdu, bpdu)
GETFUNCNULL(14, rx_egr_to_cpu)

#ifdef BROADCOM_DEBUG
GETFUNCERR(14, tx_l2pbm) 
GETFUNCERR(14, tx_utpbm) 
GETFUNCERR(14, tx_l3pbm)
GETFUNCERR(14, tx_crc) 
GETFUNCERR(14, tx_cos)
GETFUNCERR(14, tx_destmod)
GETFUNCERR(14, tx_destport)
GETFUNCERR(14, tx_opcode) 
GETFUNCERR(14, tx_srcmod)
GETFUNCERR(14, tx_srcport)
GETFUNCERR(14, tx_prio)
GETFUNCERR(14, tx_pfm)
#endif /* BROADCOM_DEBUG */

static uint32 dcb14_rx_crc_get(dcb_t *dcb) {
    return 0;
}

#ifdef  PLISIM          /* these routines are only used by pcid */
static void dcb14_status_init(dcb_t *dcb)
{
    uint32      *d = (uint32 *)dcb;

    d[5] = d[6] = d[7] = d[8] = d[9] = d[10] = 0;
    d[11] = d[12] = d[13] = d[14] = d[15] = 0;
}
SETFUNCFIELD(14, xfercount, count, uint32 count, count)
SETFUNCFIELD(14, rx_start, start, int val, val ? 1 : 0)
SETFUNCFIELD(14, rx_end, end, int val, val ? 1 : 0)
SETFUNCFIELD(14, rx_error, error, int val, val ? 1 : 0)
SETFUNCEXPRIGNORE(14, rx_crc, int val, ignore)
SETFUNCFIELD(14, rx_ingport, srcport, int val, val)
#endif  /* PLISIM */

#ifdef BROADCOM_DEBUG
static void
dcb14_dump(int unit, dcb_t *dcb, char *prefix, int tx)
{
    uint32      *p;
    int         i, size;
    dcb14_t *d = (dcb14_t *)dcb;
    char        ps[((DCB_MAX_SIZE/sizeof(uint32))*9)+1];
#if defined(LE_HOST)
    uint32  hgh[4];
    uint8 *h = (uint8 *)&hgh[0];

    hgh[0] = soc_htonl(d->mh0);
    hgh[1] = soc_htonl(d->mh1);
    hgh[2] = soc_htonl(d->mh2);
    hgh[3] = soc_htonl(d->mh3);
#else
    uint8 *h = (uint8 *)&d->mh0;
#endif

    p = (uint32 *)dcb;
    size = SOC_DCB_SIZE(unit) / sizeof(uint32);
    for (i = 0; i < size; i++) {
        sal_sprintf(&ps[i*9], "%08x ", p[i]);
    }
    soc_cm_print("%s\t%s\n", prefix, ps);
    if ((SOC_DCB_HG_GET(unit, dcb)) || (SOC_DCB_RX_START_GET(unit, dcb))) {
        soc_dma_higig_dump(unit, prefix, h, 0, 0, NULL);
    }
    soc_cm_print(
        "%s\ttype %d %ssg %schain %sreload %shg %sstat %spause %spurge\n",
        prefix,
        SOC_DCB_TYPE(unit),
        SOC_DCB_SG_GET(unit, dcb) ? "" : "!",
        SOC_DCB_CHAIN_GET(unit, dcb) ? "" : "!",
        SOC_DCB_RELOAD_GET(unit, dcb) ? "" : "!",
        SOC_DCB_HG_GET(unit, dcb) ? "" : "!",
        SOC_DCB_STAT_GET(unit, dcb) ? "" : "!",
        d->c_pause ? "" : "!",
        SOC_DCB_PURGE_GET(unit, dcb) ? "" : "!");
    soc_cm_print(
        "%s\taddr %p reqcount %d xfercount %d\n",
        prefix,
        (void *)SOC_DCB_ADDR_GET(unit, dcb),
        SOC_DCB_REQCOUNT_GET(unit, dcb),
        SOC_DCB_XFERCOUNT_GET(unit, dcb));
    if (!tx) {
        soc_cm_print(
            "%s\t%sdone %sstart %send %serror\n",
            prefix,
            SOC_DCB_DONE_GET(unit, dcb) ? "" : "!",
            SOC_DCB_RX_START_GET(unit, dcb) ? "" : "!",
            SOC_DCB_RX_END_GET(unit, dcb) ? "" : "!",
            SOC_DCB_RX_ERROR_GET(unit, dcb) ? "" : "!");
    }
    if ((!tx) && (SOC_DCB_RX_START_GET(unit, dcb))) {
        dcb0_reason_dump(unit, dcb, prefix);
        soc_cm_print(
            "%s  %sdo_not_change_ttl %sbpdu %scell_error %schg_tos %semirror %simirror\n",
            prefix,
            d->do_not_change_ttl ? "" : "!",
            d->bpdu ? "" : "!",
            d->cell_error ? "" : "!",
            d->chg_tos ? "" : "!",
            d->emirror ? "" : "!",
            d->imirror ? "" : "!"
            );
        soc_cm_print(
            "%s  %sl3ipmc %sl3only %sl3uc %spkt_aged %spurge_cell %ssrc_hg\n",
            prefix,
            d->l3ipmc ? "" : "!",
            d->l3only ? "" : "!",
            d->l3uc ? "" : "!",
            d->pkt_aged ? "" : "!",
            d->purge_cell ? "" : "!",
            d->src_hg ? "" : "!"
            );
        soc_cm_print(
            "%s  %sswitch_pkt %sregen_crc %sdecap_iptunnel %sing_untagged\n",
            prefix,
            d->switch_pkt ? "" : "!",
            d->regen_crc ? "" : "!",
            d->decap_iptunnel ? "" : "!",
            d->ingress_untagged ? "" : "!"
            );
        soc_cm_print(
            "%s  cpu_cos=%d cos=%d l3_intf=%d mtp_index=%d reason=%08x_%08x\n",
            prefix,
            d->cpu_cos,
            d->cos,
            d->l3_intf,
            d->mtp_index,
            d->reason_hi,
            d->reason
            );
        soc_cm_print(
            "%s  match_rule=%d nh_index=%d hg_type=%d em_mtp_index=%d\n",
            prefix,
            d->match_rule,
            d->nh_index,
            d->hg_type,
            d->em_mtp_index
            );
        soc_cm_print(
            "%s  srcport=%d dscp=%d outer_pri=%d outer_cfi=%d outer_vid=%d\n",
            prefix,
            d->srcport,
            d->dscp,
            d->outer_pri,
            d->outer_cfi,
            d->outer_vid
            );
        soc_cm_print(
            "%s  hgi=%d itag_status=%d otag_action=%d itag_action=%d\n",
            prefix,
            d->hgi,
            d->itag_status,
            d->otag_action,
            d->itag_action
            );
    }
}
#endif /* BROADCOM_DEBUG */
dcb_op_t dcb14_op = {
    14,
    sizeof(dcb14_t),
    dcb14_rx_reason_map,
    NULL,
    dcb14_init,
    dcb14_addtx,
    dcb14_addrx,
    dcb14_intrinfo,
    dcb14_reqcount_set,
    dcb14_reqcount_get,
    dcb14_xfercount_get,
    dcb0_addr_set,
    dcb0_addr_get,
    dcb0_paddr_get,
    dcb14_done_set,
    dcb14_done_get,
    dcb14_sg_set,
    dcb14_sg_get,
    dcb14_chain_set,
    dcb14_chain_get,
    dcb14_reload_set,
    dcb14_reload_get,
    dcb14_tx_l2pbm_set,
    dcb14_tx_utpbm_set,
    dcb14_tx_l3pbm_set,
    dcb14_tx_crc_set,
    dcb14_tx_cos_set,
    dcb14_tx_destmod_set,
    dcb14_tx_destport_set,
    dcb14_tx_opcode_set,
    dcb14_tx_srcmod_set,
    dcb14_tx_srcport_set,
    dcb14_tx_prio_set,
    dcb14_tx_pfm_set,
    dcb14_rx_untagged_get,
    dcb14_rx_crc_get,
    dcb14_rx_cos_get,
    dcb14_rx_destmod_get,
    dcb14_rx_destport_get,
    dcb14_rx_opcode_get,
    dcb14_rx_classtag_get,
    dcb14_rx_matchrule_get,
    dcb14_rx_start_get,
    dcb14_rx_end_get,
    dcb14_rx_error_get,
    dcb14_rx_prio_get,
    dcb14_rx_reason_get,
    dcb14_rx_reason_hi_get,
    dcb14_rx_ingport_get,
    dcb14_rx_srcport_get,
    dcb14_rx_srcmod_get,
    dcb14_rx_mcast_get,
    dcb14_rx_vclabel_get,
    dcb14_rx_mirror_get,
    dcb14_rx_timestamp_get,
    dcb14_rx_timestamp_upper_get,
    dcb14_hg_set,
    dcb14_hg_get,
    dcb14_stat_set,
    dcb14_stat_get,
    dcb14_purge_set,
    dcb14_purge_get,
    dcb9_mhp_get,
    dcb14_outer_vid_get,
    dcb14_outer_pri_get,
    dcb14_outer_cfi_get,
    dcb9_inner_vid_get,
    dcb9_inner_pri_get,
    dcb9_inner_cfi_get,
    dcb14_rx_bpdu_get,
    dcb14_rx_egr_to_cpu_get,
#ifdef  BROADCOM_DEBUG
    dcb14_tx_l2pbm_get,
    dcb14_tx_utpbm_get,
    dcb14_tx_l3pbm_get,
    dcb14_tx_crc_get,
    dcb14_tx_cos_get,
    dcb14_tx_destmod_get,
    dcb14_tx_destport_get,
    dcb14_tx_opcode_get,
    dcb14_tx_srcmod_get,
    dcb14_tx_srcport_get,
    dcb14_tx_prio_get,
    dcb14_tx_pfm_get,

    dcb14_dump,
    dcb0_reason_dump,
#endif /* BROADCOM_DEBUG */
#ifdef  PLISIM
    dcb14_status_init,
    dcb14_xfercount_set,
    dcb14_rx_start_set,
    dcb14_rx_end_set,
    dcb14_rx_error_set,
    dcb14_rx_crc_set,
    dcb14_rx_ingport_set,
#endif /* PLISIM */
};
#endif /* BCM_TRIUMPH_SUPPORT */

#if defined(BCM_RAPTOR_SUPPORT)
dcb_op_t dcb15_op = {
    15,
    sizeof(dcb15_t),
    dcb12_rx_reason_map,
    NULL,
    dcb9_init,
    dcb9_addtx,
    dcb9_addrx,
    dcb9_intrinfo,
    dcb9_reqcount_set,
    dcb9_reqcount_get,
    dcb9_xfercount_get,
    dcb0_addr_set,
    dcb0_addr_get,
    dcb0_paddr_get,
    dcb9_done_set,
    dcb9_done_get,
    dcb9_sg_set,
    dcb9_sg_get,
    dcb9_chain_set,
    dcb9_chain_get,
    dcb9_reload_set,
    dcb9_reload_get,
    dcb9_tx_l2pbm_set,
    dcb9_tx_utpbm_set,
    dcb9_tx_l3pbm_set,
    dcb9_tx_crc_set,
    dcb9_tx_cos_set,
    dcb9_tx_destmod_set,
    dcb9_tx_destport_set,
    dcb9_tx_opcode_set,
    dcb9_tx_srcmod_set,
    dcb9_tx_srcport_set,
    dcb9_tx_prio_set,
    dcb9_tx_pfm_set,
    dcb15_rx_untagged_get,
    dcb15_rx_crc_get,
    dcb15_rx_cos_get,
    dcb15_rx_destmod_get,
    dcb15_rx_destport_get,
    dcb15_rx_opcode_get,
    dcb15_rx_classtag_get,
    dcb15_rx_matchrule_get,
    dcb9_rx_start_get,
    dcb9_rx_end_get,
    dcb9_rx_error_get,
    dcb15_rx_prio_get,
    dcb15_rx_reason_get,
    dcb15_rx_reason_hi_get,
    dcb15_rx_ingport_get,
    dcb15_rx_srcport_get,
    dcb15_rx_srcmod_get,
    dcb15_rx_mcast_get,
    dcb15_rx_vclabel_get,
    dcb15_rx_mirror_get,
    dcb15_rx_timestamp_get,
    dcb15_rx_timestamp_upper_get,
    dcb9_hg_set,
    dcb9_hg_get,
    dcb9_stat_set,
    dcb9_stat_get,
    dcb9_purge_set,
    dcb9_purge_get,
    dcb9_mhp_get,
    dcb15_outer_vid_get,
    dcb15_outer_pri_get,
    dcb15_outer_cfi_get,
    dcb9_inner_vid_get,
    dcb9_inner_pri_get,
    dcb9_inner_cfi_get,
    dcb12_rx_bpdu_get,
    dcb12_rx_egr_to_cpu_get,
#ifdef  BROADCOM_DEBUG
    dcb9_tx_l2pbm_get,
    dcb9_tx_utpbm_get,
    dcb9_tx_l3pbm_get,
    dcb9_tx_crc_get,
    dcb9_tx_cos_get,
    dcb9_tx_destmod_get,
    dcb9_tx_destport_get,
    dcb9_tx_opcode_get,
    dcb9_tx_srcmod_get,
    dcb9_tx_srcport_get,
    dcb9_tx_prio_get,
    dcb9_tx_pfm_get,

    dcb9_dump,
    dcb0_reason_dump,
#endif /* BROADCOM_DEBUG */
#ifdef  PLISIM
    dcb9_status_init,
    dcb9_xfercount_set,
    dcb9_rx_start_set,
    dcb9_rx_end_set,
    dcb9_rx_error_set,
    dcb15_rx_crc_set,
    dcb15_rx_ingport_set,
#endif /* PLISIM */
};
#endif /* BCM_RAPTOR_SUPPORT */

#ifdef BCM_SCORPION_SUPPORT
static soc_rx_reason_t
dcb16_rx_reason_map[] = {
    socRxReasonUnknownVlan,        /* Offset 0 */
    socRxReasonL2SourceMiss,       /* Offset 1 */
    socRxReasonL2DestMiss,         /* Offset 2 */
    socRxReasonL2Move,             /* Offset 3 */
    socRxReasonL2Cpu,              /* Offset 4 */
    socRxReasonSampleSource,       /* Offset 5 */
    socRxReasonSampleDest,         /* Offset 6 */
    socRxReasonL3SourceMiss,       /* Offset 7 */
    socRxReasonL3DestMiss,         /* Offset 8 */
    socRxReasonL3SourceMove,       /* Offset 9 */
    socRxReasonMcastMiss,          /* Offset 10 */
    socRxReasonIpMcastMiss,        /* Offset 11 */
    socRxReasonFilterMatch,        /* Offset 12 */
    socRxReasonL3HeaderError,      /* Offset 13 */
    socRxReasonProtocol,           /* Offset 14 */
    socRxReasonDosAttack,          /* Offset 15 */
    socRxReasonMartianAddr,        /* Offset 16 */
    socRxReasonTunnelError,        /* Offset 17 */
    socRxReasonL2MtuFail,          /* Offset 18 */
    socRxReasonIcmpRedirect,       /* Offset 19 */
    socRxReasonL3Slowpath,         /* Offset 20 */
    socRxReasonParityError,        /* Offset 21 */
    socRxReasonL3MtuFail,          /* Offset 22 */
    socRxReasonHigigHdrError,      /* Offset 23 */
    socRxReasonMcastIdxError,      /* Offset 24 */
    socRxReasonVlanFilterMatch,    /* Offset 25 */
    socRxReasonClassBasedMove,     /* Offset 26 */
    socRxReasonL2LearnLimit,       /* Offset 27 */
    socRxReasonL2NonUnicastMiss,   /* Offset 28 */
    socRxReasonNhop,               /* Offset 29 */
    socRxReasonInvalid,            /* Offset 30 */
    socRxReasonInvalid             /* Offset 31 */
};

GETFUNCERR(16, rx_timestamp)
GETFUNCERR(16, rx_timestamp_upper)

dcb_op_t dcb16_op = {
    16,
    sizeof(dcb16_t),
    dcb16_rx_reason_map,
    NULL,
    dcb14_init,
    dcb14_addtx,
    dcb14_addrx,
    dcb14_intrinfo,
    dcb14_reqcount_set,
    dcb14_reqcount_get,
    dcb14_xfercount_get,
    dcb0_addr_set,
    dcb0_addr_get,
    dcb0_paddr_get,
    dcb14_done_set,
    dcb14_done_get,
    dcb14_sg_set,
    dcb14_sg_get,
    dcb14_chain_set,
    dcb14_chain_get,
    dcb14_reload_set,
    dcb14_reload_get,
    dcb14_tx_l2pbm_set,
    dcb14_tx_utpbm_set,
    dcb14_tx_l3pbm_set,
    dcb14_tx_crc_set,
    dcb14_tx_cos_set,
    dcb14_tx_destmod_set,
    dcb14_tx_destport_set,
    dcb14_tx_opcode_set,
    dcb14_tx_srcmod_set,
    dcb14_tx_srcport_set,
    dcb14_tx_prio_set,
    dcb14_tx_pfm_set,
    dcb14_rx_untagged_get,
    dcb14_rx_crc_get,
    dcb14_rx_cos_get,
    dcb14_rx_destmod_get,
    dcb14_rx_destport_get,
    dcb14_rx_opcode_get,
    dcb9_rx_classtag_get,
    dcb14_rx_matchrule_get,
    dcb14_rx_start_get,
    dcb14_rx_end_get,
    dcb14_rx_error_get,
    dcb14_rx_prio_get,
    dcb14_rx_reason_get,
    dcb9_rx_reason_hi_get,
    dcb14_rx_ingport_get,
    dcb14_rx_srcport_get,
    dcb14_rx_srcmod_get,
    dcb9_rx_mcast_get,
    dcb14_rx_vclabel_get,
    dcb14_rx_mirror_get,
    dcb16_rx_timestamp_get,
    dcb16_rx_timestamp_upper_get,
    dcb14_hg_set,
    dcb14_hg_get,
    dcb14_stat_set,
    dcb14_stat_get,
    dcb14_purge_set,
    dcb14_purge_get,
    dcb9_mhp_get,
    dcb14_outer_vid_get,
    dcb14_outer_pri_get,
    dcb14_outer_cfi_get,
    dcb9_inner_vid_get,
    dcb9_inner_pri_get,
    dcb9_inner_cfi_get,
    dcb14_rx_bpdu_get,
    dcb14_rx_egr_to_cpu_get,
#ifdef  BROADCOM_DEBUG
    dcb14_tx_l2pbm_get,
    dcb14_tx_utpbm_get,
    dcb14_tx_l3pbm_get,
    dcb14_tx_crc_get,
    dcb14_tx_cos_get,
    dcb14_tx_destmod_get,
    dcb14_tx_destport_get,
    dcb14_tx_opcode_get,
    dcb14_tx_srcmod_get,
    dcb14_tx_srcport_get,
    dcb14_tx_prio_get,
    dcb14_tx_pfm_get,

    dcb14_dump,
    dcb0_reason_dump,
#endif /* BROADCOM_DEBUG */
#ifdef  PLISIM
    dcb14_status_init,
    dcb14_xfercount_set,
    dcb14_rx_start_set,
    dcb14_rx_end_set,
    dcb14_rx_error_set,
    dcb14_rx_crc_set,
    dcb14_rx_ingport_set,
#endif /* PLISIM */
};
#endif /* BCM_SCORPION_SUPPORT */

#if defined(BCM_HAWKEYE_SUPPORT)

static soc_rx_reason_t
dcb17_rx_reason_map[] = {
    socRxReasonUnknownVlan,        /* Offset 0 */
    socRxReasonL2SourceMiss,       /* Offset 1 */
    socRxReasonL2DestMiss,         /* Offset 2 */
    socRxReasonL2Move,             /* Offset 3 */
    socRxReasonL2Cpu,              /* Offset 4 */
    socRxReasonSampleSource,       /* Offset 5 */
    socRxReasonSampleDest,         /* Offset 6 */
    socRxReasonL3SourceMiss,       /* Offset 7 */
    socRxReasonL3DestMiss,         /* Offset 8 */
    socRxReasonL3SourceMove,       /* Offset 9 */
    socRxReasonMcastMiss,          /* Offset 10 */
    socRxReasonIpMcastMiss,        /* Offset 11 */
    socRxReasonFilterMatch,        /* Offset 12 */
    socRxReasonL3HeaderError,      /* Offset 13 */
    socRxReasonProtocol,           /* Offset 14 */
    socRxReasonDosAttack,          /* Offset 15 */
    socRxReasonMartianAddr,        /* Offset 16 */
    socRxReasonTunnelError,        /* Offset 17 */
    socRxReasonL2MtuFail,          /* Offset 18 */
    socRxReasonIcmpRedirect,       /* Offset 19 */
    socRxReasonL3Slowpath,         /* Offset 20 */
    socRxReasonParityError,        /* Offset 21 */
    socRxReasonL3MtuFail,          /* Offset 22 */
    socRxReasonHigigHdrError,      /* Offset 23 */
    socRxReasonMcastIdxError,      /* Offset 24 */
    socRxReasonL2LearnLimit,       /* Offset 25 */
    socRxReasonTimeSync,           /* Offset 26 */
    socRxReasonEAVData,            /* Offset 27 */
    socRxReasonInvalid,            /* Offset 28 */
    socRxReasonInvalid,            /* Offset 29 */
    socRxReasonInvalid,            /* Offset 30 */
    socRxReasonInvalid             /* Offset 31 */
};

dcb_op_t dcb17_op = {
    17,
    sizeof(dcb17_t),
    dcb17_rx_reason_map,
    NULL,
    dcb9_init,
    dcb9_addtx,
    dcb9_addrx,
    dcb9_intrinfo,
    dcb9_reqcount_set,
    dcb9_reqcount_get,
    dcb9_xfercount_get,
    dcb0_addr_set,
    dcb0_addr_get,
    dcb0_paddr_get,
    dcb9_done_set,
    dcb9_done_get,
    dcb9_sg_set,
    dcb9_sg_get,
    dcb9_chain_set,
    dcb9_chain_get,
    dcb9_reload_set,
    dcb9_reload_get,
    dcb9_tx_l2pbm_set,
    dcb9_tx_utpbm_set,
    dcb9_tx_l3pbm_set,
    dcb9_tx_crc_set,
    dcb9_tx_cos_set,
    dcb9_tx_destmod_set,
    dcb9_tx_destport_set,
    dcb9_tx_opcode_set,
    dcb9_tx_srcmod_set,
    dcb9_tx_srcport_set,
    dcb9_tx_prio_set,
    dcb9_tx_pfm_set,
    dcb17_rx_untagged_get,
    dcb17_rx_crc_get,
    dcb17_rx_cos_get,
    dcb17_rx_destmod_get,
    dcb17_rx_destport_get,
    dcb17_rx_opcode_get,
    dcb17_rx_classtag_get,
    dcb15_rx_matchrule_get,
    dcb9_rx_start_get,
    dcb9_rx_end_get,
    dcb9_rx_error_get,
    dcb17_rx_prio_get,
    dcb17_rx_reason_get,
    dcb15_rx_reason_hi_get,
    dcb15_rx_ingport_get,
    dcb17_rx_srcport_get,
    dcb17_rx_srcmod_get,
    dcb17_rx_mcast_get,
    dcb17_rx_vclabel_get,
    dcb15_rx_mirror_get,
    dcb17_rx_timestamp_get,
    dcb17_rx_timestamp_upper_get,
    dcb9_hg_set,
    dcb9_hg_get,
    dcb9_stat_set,
    dcb9_stat_get,
    dcb9_purge_set,
    dcb9_purge_get,
    dcb9_mhp_get,
    dcb15_outer_vid_get,
    dcb15_outer_pri_get,
    dcb15_outer_cfi_get,
    dcb9_inner_vid_get,
    dcb9_inner_pri_get,
    dcb9_inner_cfi_get,
    dcb12_rx_bpdu_get,
    dcb12_rx_egr_to_cpu_get,
#ifdef  BROADCOM_DEBUG
    dcb9_tx_l2pbm_get,
    dcb9_tx_utpbm_get,
    dcb9_tx_l3pbm_get,
    dcb9_tx_crc_get,
    dcb9_tx_cos_get,
    dcb9_tx_destmod_get,
    dcb9_tx_destport_get,
    dcb9_tx_opcode_get,
    dcb9_tx_srcmod_get,
    dcb9_tx_srcport_get,
    dcb9_tx_prio_get,
    dcb9_tx_pfm_get,

    dcb9_dump,
    dcb0_reason_dump,
#endif /* BROADCOM_DEBUG */
#ifdef  PLISIM
    dcb9_status_init,
    dcb9_xfercount_set,
    dcb9_rx_start_set,
    dcb9_rx_end_set,
    dcb9_rx_error_set,
    dcb15_rx_crc_set,
    dcb15_rx_ingport_set,
#endif /* PLISIM */
};
#endif /* BCM_HAWKEYE_SUPPORT */

#if defined(BCM_RAPTOR_SUPPORT)
GETFUNCERR(18, rx_timestamp)
GETFUNCERR(18, rx_timestamp_upper)
GETFUNCFIELD(18, rx_matchrule, match_rule)
dcb_op_t dcb18_op = {
    18,
    sizeof(dcb18_t),
    dcb12_rx_reason_map,
    NULL,
    dcb9_init,
    dcb9_addtx,
    dcb9_addrx,
    dcb9_intrinfo,
    dcb9_reqcount_set,
    dcb9_reqcount_get,
    dcb9_xfercount_get,
    dcb0_addr_set,
    dcb0_addr_get,
    dcb0_paddr_get,
    dcb9_done_set,
    dcb9_done_get,
    dcb9_sg_set,
    dcb9_sg_get,
    dcb9_chain_set,
    dcb9_chain_get,
    dcb9_reload_set,
    dcb9_reload_get,
    dcb9_tx_l2pbm_set,
    dcb9_tx_utpbm_set,
    dcb9_tx_l3pbm_set,
    dcb9_tx_crc_set,
    dcb9_tx_cos_set,
    dcb9_tx_destmod_set,
    dcb9_tx_destport_set,
    dcb9_tx_opcode_set,
    dcb9_tx_srcmod_set,
    dcb9_tx_srcport_set,
    dcb9_tx_prio_set,
    dcb9_tx_pfm_set,
    dcb15_rx_untagged_get,
    dcb15_rx_crc_get,
    dcb15_rx_cos_get,
    dcb15_rx_destmod_get,
    dcb15_rx_destport_get,
    dcb15_rx_opcode_get,
    dcb15_rx_classtag_get,
    dcb18_rx_matchrule_get,
    dcb9_rx_start_get,
    dcb9_rx_end_get,
    dcb9_rx_error_get,
    dcb15_rx_prio_get,
    dcb15_rx_reason_get,
    dcb15_rx_reason_hi_get,
    dcb15_rx_ingport_get,
    dcb15_rx_srcport_get,
    dcb15_rx_srcmod_get,
    dcb15_rx_mcast_get,
    dcb15_rx_vclabel_get,
    dcb15_rx_mirror_get,
    dcb18_rx_timestamp_get,
    dcb18_rx_timestamp_upper_get,
    dcb9_hg_set,
    dcb9_hg_get,
    dcb9_stat_set,
    dcb9_stat_get,
    dcb9_purge_set,
    dcb9_purge_get,
    dcb9_mhp_get,
    dcb15_outer_vid_get,
    dcb15_outer_pri_get,
    dcb15_outer_cfi_get,
    dcb9_inner_vid_get,
    dcb9_inner_pri_get,
    dcb9_inner_cfi_get,
    dcb12_rx_bpdu_get,
    dcb12_rx_egr_to_cpu_get,
#ifdef  BROADCOM_DEBUG
    dcb9_tx_l2pbm_get,
    dcb9_tx_utpbm_get,
    dcb9_tx_l3pbm_get,
    dcb9_tx_crc_get,
    dcb9_tx_cos_get,
    dcb9_tx_destmod_get,
    dcb9_tx_destport_get,
    dcb9_tx_opcode_get,
    dcb9_tx_srcmod_get,
    dcb9_tx_srcport_get,
    dcb9_tx_prio_get,
    dcb9_tx_pfm_get,

    dcb9_dump,
    dcb0_reason_dump,
#endif /* BROADCOM_DEBUG */
#ifdef  PLISIM
    dcb9_status_init,
    dcb9_xfercount_set,
    dcb9_rx_start_set,
    dcb9_rx_end_set,
    dcb9_rx_error_set,
    dcb15_rx_crc_set,
    dcb15_rx_ingport_set,
#endif /* PLISIM */
};
#endif /* BCM_RAPTOR_SUPPORT */
#endif /* BCM_XGS3_SWITCH_SUPPORT */

#if defined(BCM_TRIUMPH2_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) \
    || defined(BCM_SHADOW_SUPPORT)
/*
 * DCB Type 19 Support
 */
static soc_rx_reason_t
dcb19_rx_reason_map[] = {
    socRxReasonUnknownVlan,        /* Offset 0 */
    socRxReasonL2SourceMiss,       /* Offset 1 */
    socRxReasonL2DestMiss,         /* Offset 2 */
    socRxReasonL2Move,             /* Offset 3 */
    socRxReasonL2Cpu,              /* Offset 4 */
    socRxReasonSampleSource,       /* Offset 5 */
    socRxReasonSampleDest,         /* Offset 6 */
    socRxReasonL3SourceMiss,       /* Offset 7 */
    socRxReasonL3DestMiss,         /* Offset 8 */
    socRxReasonL3SourceMove,       /* Offset 9 */
    socRxReasonMcastMiss,          /* Offset 10 */
    socRxReasonIpMcastMiss,        /* Offset 11 */
    socRxReasonFilterMatch,        /* Offset 12 */
    socRxReasonL3HeaderError,      /* Offset 13 */
    socRxReasonProtocol,           /* Offset 14 */
    socRxReasonDosAttack,          /* Offset 15 */
    socRxReasonMartianAddr,        /* Offset 16 */
    socRxReasonTunnelError,        /* Offset 17 */
    socRxReasonL2MtuFail,          /* Offset 18 */
    socRxReasonIcmpRedirect,       /* Offset 19 */
    socRxReasonL3Slowpath,         /* Offset 20 */
    socRxReasonParityError,        /* Offset 21 */
    socRxReasonL3MtuFail,          /* Offset 22 */
    socRxReasonHigigHdrError,      /* Offset 23 */
    socRxReasonMcastIdxError,      /* Offset 24 */
    socRxReasonVlanFilterMatch,    /* Offset 25 */
    socRxReasonClassBasedMove,     /* Offset 26 */
    socRxReasonL2LearnLimit,       /* Offset 27 */
    socRxReasonMplsLabelMiss,      /* Offset 28 */
    socRxReasonMplsInvalidAction,  /* Offset 29 */
    socRxReasonMplsInvalidPayload, /* Offset 30 */
    socRxReasonMplsTtl,            /* Offset 31 */
    socRxReasonMplsSequenceNumber, /* Offset 32 */
    socRxReasonL2NonUnicastMiss,   /* Offset 33 */
    socRxReasonNhop,               /* Offset 34 */
    socRxReasonMplsCtrlWordError,  /* Offset 35 */
    socRxReasonIpfixRateViolation, /* Offset 36 */
    socRxReasonWlanDot1xDrop,      /* Offset 37 */
    socRxReasonWlanSlowpath,       /* Offset 38 */
    socRxReasonWlanClientError,    /* Offset 39 */
    socRxReasonEncapHigigError,    /* Offset 40 */
    socRxReasonTimeSync,           /* Offset 41 */
    socRxReasonOAMSlowpath,        /* Offset 42 */
    socRxReasonOAMError,           /* Offset 43 */
    socRxReasonL3AddrBindFail,     /* Offset 44 */
    socRxReasonInvalid,            /* Offset 45 */
    socRxReasonInvalid,            /* Offset 46 */
    socRxReasonInvalid,            /* Offset 47 */
    socRxReasonInvalid,            /* Offset 48 */
    socRxReasonInvalid,            /* Offset 49 */
    socRxReasonInvalid,            /* Offset 50 */
    socRxReasonInvalid,            /* Offset 51 */
    socRxReasonInvalid,            /* Offset 52 */
    socRxReasonInvalid,            /* Offset 53 */
    socRxReasonInvalid,            /* Offset 54 */
    socRxReasonInvalid,            /* Offset 55 */
    socRxReasonInvalid,            /* Offset 56 */
    socRxReasonInvalid,            /* Offset 57 */
    socRxReasonInvalid,            /* Offset 58 */
    socRxReasonInvalid,            /* Offset 59 */
    socRxReasonInvalid,            /* Offset 60 */
    socRxReasonInvalid,            /* Offset 61 */
    socRxReasonInvalid,            /* Offset 62 */
    socRxReasonInvalid             /* Offset 63 */
};

static soc_rx_reason_t
dcb19_rx_egr_reason_map[] = {
    socRxReasonUnknownVlan,        /* Offset 0 */
    socRxReasonStp,                /* Offset 1 */
    socRxReasonVlanTranslate,      /* Offset 2 new */
    socRxReasonTunnelError,        /* Offset 3 */
    socRxReasonIpmc,               /* Offset 4 */
    socRxReasonL3HeaderError,      /* Offset 5 */
    socRxReasonTtl,                /* Offset 6 */
    socRxReasonL2MtuFail,          /* Offset 7 */
    socRxReasonHigigHdrError,      /* Offset 8 */
    socRxReasonSplitHorizon,       /* Offset 9 */
    socRxReasonL2Move,             /* Offset 10 */
    socRxReasonFilterMatch,        /* Offset 11 */
    socRxReasonL2SourceMiss,       /* Offset 12 */
    socRxReasonInvalid,            /* Offset 13 */
    socRxReasonInvalid,            /* Offset 14 */
    socRxReasonInvalid,            /* Offset 15 */
    socRxReasonInvalid,            /* Offset 16 */
    socRxReasonInvalid,            /* Offset 17 */
    socRxReasonInvalid,            /* Offset 18 */
    socRxReasonInvalid,            /* Offset 19 */
    socRxReasonInvalid,            /* Offset 20 */
    socRxReasonInvalid,            /* Offset 21 */
    socRxReasonInvalid,            /* Offset 22 */
    socRxReasonInvalid,            /* Offset 23 */
    socRxReasonInvalid,            /* Offset 24 */
    socRxReasonInvalid,            /* Offset 25 */
    socRxReasonInvalid,            /* Offset 26 */
    socRxReasonInvalid,            /* Offset 27 */
    socRxReasonInvalid,            /* Offset 28 */
    socRxReasonInvalid,            /* Offset 29 */
    socRxReasonInvalid,            /* Offset 30 */
    socRxReasonInvalid,            /* Offset 31 */
    socRxReasonInvalid,            /* Offset 32 */
    socRxReasonInvalid,            /* Offset 33 */
    socRxReasonInvalid,            /* Offset 34 */
    socRxReasonInvalid,            /* Offset 35 */
    socRxReasonInvalid,            /* Offset 36 */
    socRxReasonInvalid,            /* Offset 37 */
    socRxReasonInvalid,            /* Offset 38 */
    socRxReasonInvalid,            /* Offset 39 */
    socRxReasonInvalid,            /* Offset 40 */
    socRxReasonInvalid,            /* Offset 41 */
    socRxReasonInvalid,            /* Offset 42 */
    socRxReasonInvalid,            /* Offset 43 */
    socRxReasonInvalid,            /* Offset 44 */
    socRxReasonInvalid,            /* Offset 45 */
    socRxReasonInvalid,            /* Offset 46 */
    socRxReasonInvalid,            /* Offset 47 */
    socRxReasonInvalid,            /* Offset 48 */
    socRxReasonInvalid,            /* Offset 49 */
    socRxReasonInvalid,            /* Offset 50 */
    socRxReasonInvalid,            /* Offset 51 */
    socRxReasonInvalid,            /* Offset 52 */
    socRxReasonInvalid,            /* Offset 53 */
    socRxReasonInvalid,            /* Offset 54 */
    socRxReasonInvalid,            /* Offset 55 */
    socRxReasonInvalid,            /* Offset 56 */
    socRxReasonInvalid,            /* Offset 57 */
    socRxReasonInvalid,            /* Offset 58 */
    socRxReasonInvalid,            /* Offset 59 */
    socRxReasonInvalid,            /* Offset 60 */
    socRxReasonInvalid,            /* Offset 61 */
    socRxReasonInvalid,            /* Offset 62 */
    socRxReasonInvalid             /* Offset 63 */
};

static void
dcb19_init(dcb_t *dcb)
{
    uint32      *d = (uint32 *)dcb;

    d[0] = d[1] = d[2] = d[3] = d[4] = 0;
    d[5] = d[6] = d[7] = d[8] = d[9] = d[10] = 0;
    d[11] = d[12] = d[13] = d[14] = d[15] = 0;
}

static int
dcb19_addtx(dv_t *dv, sal_vaddr_t addr, uint32 count,
            pbmp_t l2pbm, pbmp_t utpbm, pbmp_t l3pbm, uint32 flags, uint32 *hgh)
{
    dcb19_t     *d;     /* DCB */
    uint32      *di;    /* DCB integer pointer */
    uint32      paddr;  /* Packet buffer physical address */
    int         unaligned;
    int         unaligned_bytes;
    uint8       *unaligned_buffer;
    uint8       *aligned_buffer;

    d = (dcb19_t *)SOC_DCB_IDX2PTR(dv->dv_unit, dv->dv_dcb, dv->dv_vcnt);

    if (addr) {
        paddr = soc_cm_l2p(dv->dv_unit, (void *)addr);
    } else {
        paddr = 0;
    }

    if (dv->dv_vcnt > 0 && (dv->dv_flags & DV_F_COMBINE_DCB) &&
        (d[-1].c_sg != 0) &&
        (d[-1].addr + d[-1].c_count) == paddr &&
        d[-1].c_count + count <= DCB_MAX_REQCOUNT) {
        d[-1].c_count += count;
        return dv->dv_cnt - dv->dv_vcnt;
    }

    /*
     * A few chip revisions do not support 128 byte PCI bursts
     * correctly if the address is not word-aligned. In case
     * we encounter an unaligned address, we consume an extra
     * DCB to fixup the alignment.
     */
    do {
        if (dv->dv_vcnt >= dv->dv_cnt) {
            return SOC_E_FULL;
        }
        if (dv->dv_vcnt > 0) {      /* chain off previous dcb */
            d[-1].c_chain = 1;
        }

        di = (uint32 *)d;
        di[0] = di[1] = di[2] = di[3] = di[4] = 0;
        di[5] = di[6] = di[7] = di[8] = di[9] = di[10] = 0;
        di[11] = di[12] = di[13] = di[14] = di[15] = 0;

        d->addr = paddr;
        d->c_count = count;
        d->c_sg = 1;

        d->c_stat = 1;
        d->c_purge = SOC_DMA_PURGE_GET(flags);
        if (SOC_DMA_HG_GET(flags)) {
            soc_higig_hdr_t *mh = (soc_higig_hdr_t *)hgh;
            if (mh->overlay1.start == SOC_HIGIG2_START) {
                d->mh3 = soc_ntohl(hgh[3]);
            }
            d->c_hg = 1;
            d->mh0 = soc_ntohl(hgh[0]);
            d->mh1 = soc_ntohl(hgh[1]);
            d->mh2 = soc_ntohl(hgh[2]);
            d->mh3 = soc_ntohl(hgh[3]);
        }

        unaligned = 0;
        if (soc_feature(dv->dv_unit, soc_feature_pkt_tx_align)) {
            if (paddr & 0x3) {
                unaligned_bytes = 4 - (paddr & 0x3);
                unaligned_buffer = (uint8 *)addr;
                aligned_buffer = SOC_DV_TX_ALIGN(dv, dv->dv_vcnt);
                aligned_buffer[0] = unaligned_buffer[0];
                aligned_buffer[1] = unaligned_buffer[1];
                aligned_buffer[2] = unaligned_buffer[2];
                d->addr = soc_cm_l2p(dv->dv_unit, aligned_buffer);
                if (count > 3) {
                    d->c_count = unaligned_bytes;
                    paddr += unaligned_bytes;
                    count -= unaligned_bytes;
                    unaligned = 1;
                }
            }
        }

        dv->dv_vcnt += 1;

        d = (dcb19_t *)SOC_DCB_IDX2PTR(dv->dv_unit, dv->dv_dcb, dv->dv_vcnt);

    } while (unaligned);

    return dv->dv_cnt - dv->dv_vcnt;
}

static int
dcb19_addrx(dv_t *dv, sal_vaddr_t addr, uint32 count, uint32 flags)
{
    dcb19_t     *d;     /* DCB */
    uint32      *di;    /* DCB integer pointer */

    d = (dcb19_t *)SOC_DCB_IDX2PTR(dv->dv_unit, dv->dv_dcb, dv->dv_vcnt);

    if (dv->dv_vcnt > 0) {      /* chain off previous dcb */
        d[-1].c_chain = 1;
    }

    di = (uint32 *)d;
    di[0] = di[1] = di[2] = di[3] = di[4] = 0;
    di[5] = di[6] = di[7] = di[8] = di[9] = di[10] = 0;
    di[11] = di[12] = di[13] = di[14] = di[15] = 0;

    if (addr) {
        d->addr = soc_cm_l2p(dv->dv_unit, (void *)addr);
    }
    d->c_count = count;
    d->c_sg = 1;

    dv->dv_vcnt += 1;
    return dv->dv_cnt - dv->dv_vcnt;
}

static uint32
dcb19_intrinfo(int unit, dcb_t *dcb, int tx, uint32 *count)
{
    dcb19_t      *d = (dcb19_t *)dcb;     /*  DCB */
    uint32      f;                      /* SOC_DCB_INFO_* flags */

    if (!d->done) {
        return 0;
    }
    f = SOC_DCB_INFO_DONE;
    if (tx) {
        if (!d->c_sg) {
            f |= SOC_DCB_INFO_PKTEND;
        }
    } else {
        if (d->end) {
            f |= SOC_DCB_INFO_PKTEND;
        }
    }
    *count = d->count;
    return f;
}

SETFUNCFIELD(19, reqcount, c_count, uint32 count, count)
GETFUNCFIELD(19, reqcount, c_count)
GETFUNCFIELD(19, xfercount, count)
/* addr_set, addr_get, paddr_get - Same as DCB 0 */
SETFUNCFIELD(19, done, done, int val, val ? 1 : 0)
GETFUNCFIELD(19, done, done)
SETFUNCFIELD(19, sg, c_sg, int val, val ? 1 : 0)
GETFUNCFIELD(19, sg, c_sg)
SETFUNCFIELD(19, chain, c_chain, int val, val ? 1 : 0)
GETFUNCFIELD(19, chain, c_chain)
SETFUNCFIELD(19, reload, c_reload, int val, val ? 1 : 0)
GETFUNCFIELD(19, reload, c_reload)
SETFUNCERR(19, tx_l2pbm, pbmp_t)
SETFUNCERR(19, tx_utpbm, pbmp_t)
SETFUNCERR(19, tx_l3pbm, pbmp_t)
SETFUNCERR(19, tx_crc, int)
SETFUNCERR(19, tx_cos, int)
SETFUNCERR(19, tx_destmod, uint32)
SETFUNCERR(19, tx_destport, uint32)
SETFUNCERR(19, tx_opcode, uint32)
SETFUNCERR(19, tx_srcmod, uint32)
SETFUNCERR(19, tx_srcport, uint32)
SETFUNCERR(19, tx_prio, uint32)
SETFUNCERR(19, tx_pfm, uint32)
GETFUNCFIELD(19, rx_start, start)
GETFUNCFIELD(19, rx_end, end)
GETFUNCFIELD(19, rx_error, error)
GETFUNCFIELD(19, rx_cos, cpu_cos)
/* Fields extracted from MH/PBI */
GETHG2FUNCFIELD(19, rx_destmod, dst_mod)
GETHG2FUNCFIELD(19, rx_destport, dst_port)
GETHG2FUNCFIELD(19, rx_srcmod, src_mod)
GETHG2FUNCFIELD(19, rx_srcport, src_port)
GETHG2FUNCFIELD(19, rx_opcode, opcode)
GETHG2FUNCFIELD(19, rx_prio, vlan_pri) /* outer_pri */
GETHG2FUNCEXPR(19, rx_mcast, ((h->ppd_overlay1.dst_mod << 8) |
                              (h->ppd_overlay1.dst_port)))
GETHG2FUNCEXPR(19, rx_vclabel, ((h->ppd_overlay1.vc_label_19_16 << 16) |
                              (h->ppd_overlay1.vc_label_15_8 << 8) |
                              (h->ppd_overlay1.vc_label_7_0)))
GETHG2FUNCEXPR(19, rx_classtag, (h->ppd_overlay1.ppd_type != 1 ? 0 :
                                 (h->ppd_overlay2.ctag_hi << 8) |
                                 (h->ppd_overlay2.ctag_lo)))
GETFUNCUNITEXPR(19, rx_untagged, (IS_HG_PORT(unit, d->srcport) ?
                                  ((d->itag_status) ? 0 : 2) :
                                  ((d->itag_status & 0x2) ?
                                   ((d->itag_status & 0x1) ? 0 : 2) :
                                   ((d->itag_status & 0x1) ? 1 : 3))))
GETFUNCFIELD(19, rx_matchrule, match_rule)
GETFUNCFIELD(19, rx_reason, reason)
GETFUNCFIELD(19, rx_reason_hi, reason_hi)
GETFUNCFIELD(19, rx_ingport, srcport)
GETFUNCEXPR(19, rx_mirror, ((d->imirror) | (d->emirror)))
GETFUNCFIELD(19, rx_timestamp, timestamp)
GETFUNCERR(19, rx_timestamp_upper)
SETFUNCFIELD(19, hg, c_hg, uint32 hg, hg)
GETFUNCFIELD(19, hg, c_hg)
SETFUNCFIELD(19, stat, c_stat, uint32 stat, stat)
GETFUNCFIELD(19, stat, c_stat)
SETFUNCFIELD(19, purge, c_purge, uint32 purge, purge)
GETFUNCFIELD(19, purge, c_purge)
GETPTREXPR(19, mhp, &(d->mh0))
GETFUNCFIELD(19, outer_vid, outer_vid)
GETFUNCFIELD(19, outer_pri, outer_pri)
GETFUNCFIELD(19, outer_cfi, outer_cfi)
GETFUNCFIELD(19, inner_vid, inner_vid)
GETFUNCFIELD(19, inner_pri, inner_pri)
GETFUNCFIELD(19, inner_cfi, inner_cfi)
GETFUNCFIELD(19, rx_bpdu, bpdu)
GETFUNCFIELD(19, rx_egr_to_cpu, egr_cpu_copy)

#ifdef BROADCOM_DEBUG
GETFUNCERR(19, tx_l2pbm) 
GETFUNCERR(19, tx_utpbm) 
GETFUNCERR(19, tx_l3pbm)
GETFUNCERR(19, tx_crc) 
GETFUNCERR(19, tx_cos)
GETFUNCERR(19, tx_destmod)
GETFUNCERR(19, tx_destport)
GETFUNCERR(19, tx_opcode) 
GETFUNCERR(19, tx_srcmod)
GETFUNCERR(19, tx_srcport)
GETFUNCERR(19, tx_prio)
GETFUNCERR(19, tx_pfm)
#endif /* BROADCOM_DEBUG */

static uint32 dcb19_rx_crc_get(dcb_t *dcb) {
    return 0;
}

#ifdef  PLISIM          /* these routines are only used by pcid */
static void dcb19_status_init(dcb_t *dcb)
{
    uint32      *d = (uint32 *)dcb;

    d[5] = d[6] = d[7] = d[8] = d[9] = d[10] = 0;
    d[11] = d[12] = d[13] = d[14] = d[15] = 0;
}
SETFUNCFIELD(19, xfercount, count, uint32 count, count)
SETFUNCFIELD(19, rx_start, start, int val, val ? 1 : 0)
SETFUNCFIELD(19, rx_end, end, int val, val ? 1 : 0)
SETFUNCFIELD(19, rx_error, error, int val, val ? 1 : 0)
SETFUNCEXPRIGNORE(19, rx_crc, int val, ignore)
SETFUNCFIELD(19, rx_ingport, srcport, int val, val)
#endif  /* PLISIM */

#ifdef BROADCOM_DEBUG
static void
dcb19_dump(int unit, dcb_t *dcb, char *prefix, int tx)
{
    uint32      *p;
    int         i, size;
    dcb19_t *d = (dcb19_t *)dcb;
    char        ps[((DCB_MAX_SIZE/sizeof(uint32))*9)+1];
#if defined(LE_HOST)
    uint32  hgh[4];
    uint8 *h = (uint8 *)&hgh[0];

    hgh[0] = soc_htonl(d->mh0);
    hgh[1] = soc_htonl(d->mh1);
    hgh[2] = soc_htonl(d->mh2);
    hgh[3] = soc_htonl(d->mh3);
#else
    uint8 *h = (uint8 *)&d->mh0;
#endif

    p = (uint32 *)dcb;
    size = SOC_DCB_SIZE(unit) / sizeof(uint32);
    for (i = 0; i < size; i++) {
        sal_sprintf(&ps[i*9], "%08x ", p[i]);
    }
    soc_cm_print("%s\t%s\n", prefix, ps);
    if ((SOC_DCB_HG_GET(unit, dcb)) || (SOC_DCB_RX_START_GET(unit, dcb))) {
        soc_dma_higig_dump(unit, prefix, h, 0, 0, NULL);
    }
    soc_cm_print(
        "%s\ttype %d %ssg %schain %sreload %shg %sstat %spause %spurge\n",
        prefix,
        SOC_DCB_TYPE(unit),
        SOC_DCB_SG_GET(unit, dcb) ? "" : "!",
        SOC_DCB_CHAIN_GET(unit, dcb) ? "" : "!",
        SOC_DCB_RELOAD_GET(unit, dcb) ? "" : "!",
        SOC_DCB_HG_GET(unit, dcb) ? "" : "!",
        SOC_DCB_STAT_GET(unit, dcb) ? "" : "!",
        d->c_pause ? "" : "!",
        SOC_DCB_PURGE_GET(unit, dcb) ? "" : "!");
    soc_cm_print(
        "%s\taddr %p reqcount %d xfercount %d\n",
        prefix,
        (void *)SOC_DCB_ADDR_GET(unit, dcb),
        SOC_DCB_REQCOUNT_GET(unit, dcb),
        SOC_DCB_XFERCOUNT_GET(unit, dcb));
    if (!tx) {
        soc_cm_print(
            "%s\t%sdone %sstart %send %serror\n",
            prefix,
            SOC_DCB_DONE_GET(unit, dcb) ? "" : "!",
            SOC_DCB_RX_START_GET(unit, dcb) ? "" : "!",
            SOC_DCB_RX_END_GET(unit, dcb) ? "" : "!",
            SOC_DCB_RX_ERROR_GET(unit, dcb) ? "" : "!");
    }
    if ((!tx) && (SOC_DCB_RX_START_GET(unit, dcb))) {
        dcb0_reason_dump(unit, dcb, prefix);
        soc_cm_print(
            "%s  %sdo_not_change_ttl %sbpdu %sl3routed %schg_tos %semirror %simirror\n",
            prefix,
            d->do_not_change_ttl ? "" : "!",
            d->bpdu ? "" : "!",
            d->l3routed ? "" : "!",
            d->chg_tos ? "" : "!",
            d->emirror ? "" : "!",
            d->imirror ? "" : "!"
            );
        soc_cm_print(
            "%s  %sreplicated %sl3only %soam_pkt %segr_cpu_copy %strue_egr_mirror %ssrc_hg\n",
            prefix,
            d->replicated ? "" : "!",
            d->l3only ? "" : "!",
            d->oam_pkt ? "" : "!",
            d->egr_cpu_copy ? "" : "!",
            d->true_egr_mirror ? "" : "!",
            d->src_hg ? "" : "!"
            );
        soc_cm_print(
            "%s  %sswitch_pkt %sregen_crc %sservice_tag %sing_untagged\n",
            prefix,
            d->switch_pkt ? "" : "!",
            d->regen_crc ? "" : "!",
            d->service_tag ? "" : "!",
            !(((soc_higig2_hdr_t *)h)->ppd_overlay1.ingress_tagged) ? "" : "!"
            );
        soc_cm_print(
            "%s  cpu_cos=%d cos=%d l3_intf=%d mtp_index=%d reason=%08x_%08x\n",
            prefix,
            d->cpu_cos,
            d->cos,
            d->l3_intf,
            d->mtp_index,
            d->reason_hi,
            d->reason
            );
        soc_cm_print(
            "%s  match_rule=%d hg_type=%d mtp_index=%d decap_tunnel_type=%d\n",
            prefix,
            d->match_rule,
            d->hg_type,
            d->mtp_index,
            d->decap_tunnel_type
            );
        soc_cm_print(
            "%s  srcport=%d dscp=%d outer_pri=%d outer_cfi=%d outer_vid=%d\n",
            prefix,
            d->srcport,
            d->dscp,
            d->outer_pri,
            d->outer_cfi,
            d->outer_vid
            );
        soc_cm_print(
            "%s  orig_dstport=%d inner_pri=%d inner_cfi=%d inner_vid=%d\n",
            prefix,
            d->orig_dstport,
            d->inner_pri,
            d->inner_cfi,
            d->inner_vid
            );
        soc_cm_print(
            "%s  hgi=%d itag_status=%d otag_action=%d itag_action=%d\n",
            prefix,
            d->hgi,
            d->itag_status,
            d->otag_action,
            d->itag_action
            );
    }
}
#endif /* BROADCOM_DEBUG */

dcb_op_t dcb19_op = {
    19,
    sizeof(dcb19_t),
    dcb19_rx_reason_map,
    dcb19_rx_egr_reason_map,
    dcb19_init,
    dcb19_addtx,
    dcb19_addrx,
    dcb19_intrinfo,
    dcb19_reqcount_set,
    dcb19_reqcount_get,
    dcb19_xfercount_get,
    dcb0_addr_set,
    dcb0_addr_get,
    dcb0_paddr_get,
    dcb19_done_set,
    dcb19_done_get,
    dcb19_sg_set,
    dcb19_sg_get,
    dcb19_chain_set,
    dcb19_chain_get,
    dcb19_reload_set,
    dcb19_reload_get,
    dcb19_tx_l2pbm_set,
    dcb19_tx_utpbm_set,
    dcb19_tx_l3pbm_set,
    dcb19_tx_crc_set,
    dcb19_tx_cos_set,
    dcb19_tx_destmod_set,
    dcb19_tx_destport_set,
    dcb19_tx_opcode_set,
    dcb19_tx_srcmod_set,
    dcb19_tx_srcport_set,
    dcb19_tx_prio_set,
    dcb19_tx_pfm_set,
    dcb19_rx_untagged_get,
    dcb19_rx_crc_get,
    dcb19_rx_cos_get,
    dcb19_rx_destmod_get,
    dcb19_rx_destport_get,
    dcb19_rx_opcode_get,
    dcb19_rx_classtag_get,
    dcb19_rx_matchrule_get,
    dcb19_rx_start_get,
    dcb19_rx_end_get,
    dcb19_rx_error_get,
    dcb19_rx_prio_get,
    dcb19_rx_reason_get,
    dcb19_rx_reason_hi_get,
    dcb19_rx_ingport_get,
    dcb19_rx_srcport_get,
    dcb19_rx_srcmod_get,
    dcb19_rx_mcast_get,
    dcb19_rx_vclabel_get,
    dcb19_rx_mirror_get,
    dcb19_rx_timestamp_get,
    dcb19_rx_timestamp_upper_get,
    dcb19_hg_set,
    dcb19_hg_get,
    dcb19_stat_set,
    dcb19_stat_get,
    dcb19_purge_set,
    dcb19_purge_get,
    dcb19_mhp_get,
    dcb19_outer_vid_get,
    dcb19_outer_pri_get,
    dcb19_outer_cfi_get,
    dcb19_inner_vid_get,
    dcb19_inner_pri_get,
    dcb19_inner_cfi_get,
    dcb19_rx_bpdu_get,
    dcb19_rx_egr_to_cpu_get,
#ifdef  BROADCOM_DEBUG
    dcb19_tx_l2pbm_get,
    dcb19_tx_utpbm_get,
    dcb19_tx_l3pbm_get,
    dcb19_tx_crc_get,
    dcb19_tx_cos_get,
    dcb19_tx_destmod_get,
    dcb19_tx_destport_get,
    dcb19_tx_opcode_get,
    dcb19_tx_srcmod_get,
    dcb19_tx_srcport_get,
    dcb19_tx_prio_get,
    dcb19_tx_pfm_get,

    dcb19_dump,
    dcb0_reason_dump,
#endif /* BROADCOM_DEBUG */
#ifdef  PLISIM
    dcb19_status_init,
    dcb19_xfercount_set,
    dcb19_rx_start_set,
    dcb19_rx_end_set,
    dcb19_rx_error_set,
    dcb19_rx_crc_set,
    dcb19_rx_ingport_set,
#endif /* PLISIM */
};
#endif /* BCM_TRIUMPH2_SUPPORT || BCM_SIRIUS_SUPPORT */

#if defined(BCM_ENDURO_SUPPORT)
/*
 * DCB Type 20 Support
 */

static soc_rx_reason_t
dcb20_rx_reason_map[] = {
    socRxReasonUnknownVlan,        /* Offset 0 */
    socRxReasonL2SourceMiss,       /* Offset 1 */
    socRxReasonL2DestMiss,         /* Offset 2 */
    socRxReasonL2Move,             /* Offset 3 */
    socRxReasonL2Cpu,              /* Offset 4 */
    socRxReasonSampleSource,       /* Offset 5 */
    socRxReasonSampleDest,         /* Offset 6 */
    socRxReasonL3SourceMiss,       /* Offset 7 */
    socRxReasonL3DestMiss,         /* Offset 8 */
    socRxReasonL3SourceMove,       /* Offset 9 */
    socRxReasonMcastMiss,          /* Offset 10 */
    socRxReasonIpMcastMiss,        /* Offset 11 */
    socRxReasonFilterMatch,        /* Offset 12 */
    socRxReasonL3HeaderError,      /* Offset 13 */
    socRxReasonProtocol,           /* Offset 14 */
    socRxReasonDosAttack,          /* Offset 15 */
    socRxReasonMartianAddr,        /* Offset 16 */
    socRxReasonTunnelError,        /* Offset 17 */
    socRxReasonL2MtuFail,          /* Offset 18 */
    socRxReasonIcmpRedirect,       /* Offset 19 */
    socRxReasonL3Slowpath,         /* Offset 20 */
    socRxReasonParityError,        /* Offset 21 */
    socRxReasonL3MtuFail,          /* Offset 22 */
    socRxReasonHigigHdrError,      /* Offset 23 */
    socRxReasonMcastIdxError,      /* Offset 24 */
    socRxReasonVlanFilterMatch,    /* Offset 25 */
    socRxReasonClassBasedMove,     /* Offset 26 */
    socRxReasonL2LearnLimit,       /* Offset 27 */
    socRxReasonMplsLabelMiss,      /* Offset 28 */
    socRxReasonMplsInvalidAction,  /* Offset 29 */
    socRxReasonMplsInvalidPayload, /* Offset 30 */
    socRxReasonMplsTtl,            /* Offset 31 */
    socRxReasonMplsSequenceNumber, /* Offset 32 */
    socRxReasonL2NonUnicastMiss,   /* Offset 33 */
    socRxReasonNhop,               /* Offset 34 */
    socRxReasonMplsCtrlWordError,  /* Offset 35 */
    socRxReasonTimeSync,           /* Offset 36 */
    socRxReasonOAMSlowpath,        /* Offset 37 */
    socRxReasonOAMError,           /* Offset 38 */
    socRxReasonOAMLMDM,            /* Offset 39 */
    socRxReasonInvalid,            /* Offset 40 */
    socRxReasonInvalid,            /* Offset 41 */
    socRxReasonInvalid,            /* Offset 42 */
    socRxReasonInvalid,            /* Offset 43 */
    socRxReasonInvalid,            /* Offset 44 */
    socRxReasonInvalid,            /* Offset 45 */
    socRxReasonInvalid,            /* Offset 46 */
    socRxReasonInvalid,            /* Offset 47 */
    socRxReasonInvalid,            /* Offset 48 */
    socRxReasonInvalid,            /* Offset 49 */
    socRxReasonInvalid,            /* Offset 50 */
    socRxReasonInvalid,            /* Offset 51 */
    socRxReasonInvalid,            /* Offset 52 */
    socRxReasonInvalid,            /* Offset 53 */
    socRxReasonInvalid,            /* Offset 54 */
    socRxReasonInvalid,            /* Offset 55 */
    socRxReasonInvalid,            /* Offset 56 */
    socRxReasonInvalid,            /* Offset 57 */
    socRxReasonInvalid,            /* Offset 58 */
    socRxReasonInvalid,            /* Offset 59 */
    socRxReasonInvalid,            /* Offset 60 */
    socRxReasonInvalid,            /* Offset 61 */
    socRxReasonInvalid,            /* Offset 62 */
    socRxReasonInvalid             /* Offset 63 */
};


#ifdef BROADCOM_DEBUG
static void
dcb20_dump(int unit, dcb_t *dcb, char *prefix, int tx)
{
    uint32      *p;
    int         i, size;
    dcb20_t *d = (dcb20_t *)dcb;
    char        ps[((DCB_MAX_SIZE/sizeof(uint32))*9)+1];
#if defined(LE_HOST)
    uint32  hgh[4];
    uint8 *h = (uint8 *)&hgh[0];

    hgh[0] = soc_htonl(d->mh0);
    hgh[1] = soc_htonl(d->mh1);
    hgh[2] = soc_htonl(d->mh2);
    hgh[3] = soc_htonl(d->mh3);
#else
    uint8 *h = (uint8 *)&d->mh0;
#endif

    p = (uint32 *)dcb;
    size = SOC_DCB_SIZE(unit) / sizeof(uint32);
    for (i = 0; i < size; i++) {
        sal_sprintf(&ps[i*9], "%08x ", p[i]);
    }
    soc_cm_print("%s\t%s\n", prefix, ps);
    if ((SOC_DCB_HG_GET(unit, dcb)) || (SOC_DCB_RX_START_GET(unit, dcb))) {
        soc_dma_higig_dump(unit, prefix, h, 0, 0, NULL);
    }
    soc_cm_print(
        "%s\ttype %d %ssg %schain %sreload %shg %sstat %spause %spurge\n",
        prefix,
        SOC_DCB_TYPE(unit),
        SOC_DCB_SG_GET(unit, dcb) ? "" : "!",
        SOC_DCB_CHAIN_GET(unit, dcb) ? "" : "!",
        SOC_DCB_RELOAD_GET(unit, dcb) ? "" : "!",
        SOC_DCB_HG_GET(unit, dcb) ? "" : "!",
        SOC_DCB_STAT_GET(unit, dcb) ? "" : "!",
        d->c_pause ? "" : "!",
        SOC_DCB_PURGE_GET(unit, dcb) ? "" : "!");
    soc_cm_print(
        "%s\taddr %p reqcount %d xfercount %d\n",
        prefix,
        (void *)SOC_DCB_ADDR_GET(unit, dcb),
        SOC_DCB_REQCOUNT_GET(unit, dcb),
        SOC_DCB_XFERCOUNT_GET(unit, dcb));
    if (!tx) {
        soc_cm_print(
            "%s\t%sdone %sstart %send %serror\n",
            prefix,
            SOC_DCB_DONE_GET(unit, dcb) ? "" : "!",
            SOC_DCB_RX_START_GET(unit, dcb) ? "" : "!",
            SOC_DCB_RX_END_GET(unit, dcb) ? "" : "!",
            SOC_DCB_RX_ERROR_GET(unit, dcb) ? "" : "!");
    }
    if ((!tx) && (SOC_DCB_RX_START_GET(unit, dcb))) {
        dcb0_reason_dump(unit, dcb, prefix);
        soc_cm_print(
            "%s  %sdo_not_change_ttl %sbpdu %sl3routed %schg_tos %semirror %simirror\n",
            prefix,
            d->do_not_change_ttl ? "" : "!",
            d->bpdu ? "" : "!",
            d->l3routed ? "" : "!",
            d->chg_tos ? "" : "!",
            d->emirror ? "" : "!",
            d->imirror ? "" : "!"
            );
        soc_cm_print(
            "%s  %sreplicated %sl3only %ssrc_hg im_mtp_index=%d em_mtp_index=%d\n",
            prefix,
            d->replicated ? "" : "!",
            d->l3only ? "" : "!",
            d->src_hg ? "" : "!",
            d->im_mtp_index,
            d->em_mtp_index
            );
        soc_cm_print(
            "%s  %sswitch_pkt %sregen_crc %sservice_tag %sing_untagged\n",
            prefix,
            d->switch_pkt ? "" : "!",
            d->regen_crc ? "" : "!",
            d->service_tag ? "" : "!",
            !(((soc_higig2_hdr_t *)h)->ppd_overlay1.ingress_tagged) ? "" : "!"
            );
        soc_cm_print(
            "%s  cpu_cos=%d cos=%d l3_intf=%d reason=%08x_%08x\n",
            prefix,
            d->cpu_cos,
            d->cos,
            d->nhop_index,
            d->reason_hi,
            d->reason
            );
        soc_cm_print(
            "%s  match_rule=%d hg_type=%d decap_tunnel_type=%d\n",
            prefix,
            d->match_rule,
            d->hg_type,
            d->decap_tunnel_type
            );
        soc_cm_print(
            "%s  srcport=%d dscp=%d outer_pri=%d outer_cfi=%d outer_vid=%d\n",
            prefix,
            d->srcport,
            d->dscp,
            d->outer_pri,
            d->outer_cfi,
            d->outer_vid
            );
        soc_cm_print(
            "%s  inner_pri=%d inner_cfi=%d inner_vid=%d\n",
            prefix,
            d->inner_pri,
            d->inner_cfi,
            d->inner_vid
            );
        soc_cm_print(
            "%s  hgi=%d itag_status=%d otag_action=%d itag_action=%d\n",
            prefix,
            d->hgi,
            d->itag_status,
            d->otag_action,
            d->itag_action
            );
    }
}
#endif /* BROADCOM_DEBUG */

GETFUNCFIELD(20, rx_matchrule, match_rule)
GETFUNCFIELD(20, rx_timestamp, timestamp)
GETFUNCFIELD(20, rx_timestamp_upper, timestamp_upper)

dcb_op_t dcb20_op = {
    20,
    sizeof(dcb20_t),
    dcb20_rx_reason_map,
    NULL,
    dcb19_init,
    dcb19_addtx,
    dcb19_addrx,
    dcb19_intrinfo,
    dcb19_reqcount_set,
    dcb19_reqcount_get,
    dcb19_xfercount_get,
    dcb0_addr_set,
    dcb0_addr_get,
    dcb0_paddr_get,
    dcb19_done_set,
    dcb19_done_get,
    dcb19_sg_set,
    dcb19_sg_get,
    dcb19_chain_set,
    dcb19_chain_get,
    dcb19_reload_set,
    dcb19_reload_get,
    dcb19_tx_l2pbm_set,
    dcb19_tx_utpbm_set,
    dcb19_tx_l3pbm_set,
    dcb19_tx_crc_set,
    dcb19_tx_cos_set,
    dcb19_tx_destmod_set,
    dcb19_tx_destport_set,
    dcb19_tx_opcode_set,
    dcb19_tx_srcmod_set,
    dcb19_tx_srcport_set,
    dcb19_tx_prio_set,
    dcb19_tx_pfm_set,
    dcb19_rx_untagged_get,
    dcb19_rx_crc_get,
    dcb19_rx_cos_get,
    dcb19_rx_destmod_get,
    dcb19_rx_destport_get,
    dcb19_rx_opcode_get,
    dcb19_rx_classtag_get,
    dcb20_rx_matchrule_get,
    dcb19_rx_start_get,
    dcb19_rx_end_get,
    dcb19_rx_error_get,
    dcb19_rx_prio_get,
    dcb19_rx_reason_get,
    dcb19_rx_reason_hi_get,
    dcb19_rx_ingport_get,
    dcb19_rx_srcport_get,
    dcb19_rx_srcmod_get,
    dcb19_rx_mcast_get,
    dcb19_rx_vclabel_get,
    dcb19_rx_mirror_get,
    dcb20_rx_timestamp_get,
    dcb20_rx_timestamp_upper_get,
    dcb19_hg_set,
    dcb19_hg_get,
    dcb19_stat_set,
    dcb19_stat_get,
    dcb19_purge_set,
    dcb19_purge_get,
    dcb19_mhp_get,
    dcb19_outer_vid_get,
    dcb19_outer_pri_get,
    dcb19_outer_cfi_get,
    dcb19_inner_vid_get,
    dcb19_inner_pri_get,
    dcb19_inner_cfi_get,
    dcb19_rx_bpdu_get,
    dcb9_rx_egr_to_cpu_get,
#ifdef  BROADCOM_DEBUG
    dcb19_tx_l2pbm_get,
    dcb19_tx_utpbm_get,
    dcb19_tx_l3pbm_get,
    dcb19_tx_crc_get,
    dcb19_tx_cos_get,
    dcb19_tx_destmod_get,
    dcb19_tx_destport_get,
    dcb19_tx_opcode_get,
    dcb19_tx_srcmod_get,
    dcb19_tx_srcport_get,
    dcb19_tx_prio_get,
    dcb19_tx_pfm_get,

    dcb20_dump,
    dcb0_reason_dump,
#endif /* BROADCOM_DEBUG */
#ifdef  PLISIM
    dcb19_status_init,
    dcb19_xfercount_set,
    dcb19_rx_start_set,
    dcb19_rx_end_set,
    dcb19_rx_error_set,
    dcb19_rx_crc_set,
    dcb19_rx_ingport_set,
#endif /* PLISIM */
};
#endif /* BCM_ENDURO_SUPPORT */

#if defined(BCM_TRIDENT_SUPPORT)
/*
 * DCB Type 21 Support
 */

static soc_rx_reason_t
dcb21_rx_reason_map[] = {
    socRxReasonUnknownVlan,        /* Offset 0 */
    socRxReasonL2SourceMiss,       /* Offset 1 */
    socRxReasonL2DestMiss,         /* Offset 2 */
    socRxReasonL2Move,             /* Offset 3 */
    socRxReasonL2Cpu,              /* Offset 4 */
    socRxReasonSampleSource,       /* Offset 5 */
    socRxReasonSampleDest,         /* Offset 6 */
    socRxReasonL3SourceMiss,       /* Offset 7 */
    socRxReasonL3DestMiss,         /* Offset 8 */
    socRxReasonL3SourceMove,       /* Offset 9 */
    socRxReasonMcastMiss,          /* Offset 10 */
    socRxReasonIpMcastMiss,        /* Offset 11 */
    socRxReasonFilterMatch,        /* Offset 12 */
    socRxReasonL3HeaderError,      /* Offset 13 */
    socRxReasonProtocol,           /* Offset 14 */
    socRxReasonDosAttack,          /* Offset 15 */
    socRxReasonMartianAddr,        /* Offset 16 */
    socRxReasonTunnelError,        /* Offset 17 */
    socRxReasonInvalid,             
    socRxReasonIcmpRedirect,       /* Offset 19 */
    socRxReasonL3Slowpath,         /* Offset 20 */
    socRxReasonParityError,        /* Offset 21 */
    socRxReasonL3MtuFail,          /* Offset 22 */
    socRxReasonHigigHdrError,      /* Offset 23 */
    socRxReasonMcastIdxError,      /* Offset 24 */
    socRxReasonVlanFilterMatch,    /* Offset 25 */
    socRxReasonClassBasedMove,     /* Offset 26 */
    socRxReasonL3AddrBindFail,     /* Offset 27 */
    socRxReasonMplsLabelMiss,      /* Offset 28 */
    socRxReasonMplsInvalidAction,  /* Offset 29 */
    socRxReasonMplsInvalidPayload, /* Offset 30 */
    socRxReasonMplsTtl,            /* Offset 31 */
    socRxReasonMplsSequenceNumber, /* Offset 32 */
    socRxReasonL2NonUnicastMiss,   /* Offset 33 */
    socRxReasonNhop,               /* Offset 34 */
    socRxReasonMplsCtrlWordError,  /* Offset 35 */
    socRxReasonInvalid,            
    socRxReasonInvalid,            
    socRxReasonInvalid,            
    socRxReasonInvalid,            
    socRxReasonEncapHigigError,    /* Offset 40 */
    socRxReasonTimeSync,           /* Offset 41 */
    socRxReasonOAMSlowpath,        /* Offset 42 */
    socRxReasonOAMError,           /* Offset 43 */
    socRxReasonInvalid,            
    socRxReasonInvalid,            
    socRxReasonInvalid,            
    socRxReasonInvalid,            /* Offset 47 */
    socRxReasonInvalid,            /* Offset 48 */
    socRxReasonInvalid,            /* Offset 49 */
    socRxReasonInvalid,            /* Offset 50 */
    socRxReasonInvalid,            /* Offset 51 */
    socRxReasonInvalid,            /* Offset 52 */
    socRxReasonInvalid,            /* Offset 53 */
    socRxReasonInvalid,            /* Offset 54 */
    socRxReasonInvalid,            /* Offset 55 */
    socRxReasonInvalid,            /* Offset 56 */
    socRxReasonInvalid,            /* Offset 57 */
    socRxReasonInvalid,            /* Offset 58 */
    socRxReasonInvalid,            /* Offset 59 */
    socRxReasonInvalid,            /* Offset 60 */
    socRxReasonInvalid,            /* Offset 61 */
    socRxReasonInvalid,            /* Offset 62 */
    socRxReasonInvalid             /* Offset 63 */
};

GETFUNCEXPR(21, rx_mirror, (d->reason & (1 << 18)))

#ifdef BROADCOM_DEBUG
static void
dcb21_dump(int unit, dcb_t *dcb, char *prefix, int tx)
{
    uint32      *p;
    int         i, size;
    dcb21_t     *d = (dcb21_t *)dcb;
    char        ps[((DCB_MAX_SIZE/sizeof(uint32))*9)+1];
#if defined(LE_HOST)
    uint32      hgh[4];
    uint8       *h = (uint8 *)&hgh[0];

    hgh[0] = soc_htonl(d->mh0);
    hgh[1] = soc_htonl(d->mh1);
    hgh[2] = soc_htonl(d->mh2);
    hgh[3] = soc_htonl(d->mh3);
#else
    uint8       *h = (uint8 *)&d->mh0;
#endif

    p = (uint32 *)dcb;
    size = SOC_DCB_SIZE(unit) / sizeof(uint32);
    for (i = 0; i < size; i++) {
        sal_sprintf(&ps[i*9], "%08x ", p[i]);
    }
    soc_cm_print("%s\t%s\n", prefix, ps);
    if ((SOC_DCB_HG_GET(unit, dcb)) || (SOC_DCB_RX_START_GET(unit, dcb))) {
        soc_dma_higig_dump(unit, prefix, h, 0, 0, NULL);
    }
    soc_cm_print("%s\ttype %d %schain %ssg %sreload %shg %sstat %spause "
                 " %spurge\n",
                 prefix,
                 SOC_DCB_TYPE(unit),
                 SOC_DCB_CHAIN_GET(unit, dcb) ? "" : "!",
                 SOC_DCB_SG_GET(unit, dcb) ? "" : "!",
                 SOC_DCB_RELOAD_GET(unit, dcb) ? "" : "!",
                 SOC_DCB_HG_GET(unit, dcb) ? "" : "!",
                 SOC_DCB_STAT_GET(unit, dcb) ? "" : "!",
                 d->c_pause ? "" : "!",
                 SOC_DCB_PURGE_GET(unit, dcb) ? "" : "!");
    soc_cm_print("%s\taddr %p reqcount %d xfercount %d\n",
                 prefix,
                 (void *)SOC_DCB_ADDR_GET(unit, dcb),
                 SOC_DCB_REQCOUNT_GET(unit, dcb),
                 SOC_DCB_XFERCOUNT_GET(unit, dcb));
    if (!tx) {
        soc_cm_print("%s\t%sdone %serror %sstart %send\n",
                     prefix,
                     SOC_DCB_DONE_GET(unit, dcb) ? "" : "!",
                     SOC_DCB_RX_ERROR_GET(unit, dcb) ? "" : "!",
                     SOC_DCB_RX_START_GET(unit, dcb) ? "" : "!",
                     SOC_DCB_RX_END_GET(unit, dcb) ? "" : "!");
    }

    if (tx || !SOC_DCB_RX_START_GET(unit, dcb)) {
        return;
    }

    dcb0_reason_dump(unit, dcb, prefix);
    soc_cm_print("%s\t%schg_tos %sregen_crc %schg_ecn %smcq %svfi_valid"
                 " %sdvp_nhi_sel\n",
                 prefix,
                 d->chg_tos ? "" : "!",
                 d->regen_crc ? "" : "!",
                 d->chg_ecn ? "" : "!",
                 d->mcq ? "" : "!",
                 d->vfi_valid ? "" : "!",
                 d->dvp_nhi_sel ? "" : "!");
    soc_cm_print("%s\t%sservice_tag %sswitch_pkt %shg_type %ssrc_hg\n",
                 prefix,
                 d->service_tag ? "" : "!",
                 d->switch_pkt ? "" : "!",
                 d->hg_type ? "" : "!",
                 d->src_hg ? "" : "!");
    soc_cm_print("%s\t%sl3routed %sl3only %sreplicated %sdo_not_change_ttl"
                 " %sbpdu\n",
                 prefix,
                 d->l3routed ? "" : "!",
                 d->l3only ? "" : "!",
                 d->replicated ? "" : "!",
                 d->do_not_change_ttl ? "" : "!",
                 d->bpdu ? "" : "!");
    soc_cm_print("%s\t%soam_pkt %seh_tm %shg2_ext_hdr\n",
                 prefix,
                 d->oam_pkt ? "" : "!",
                 d->eh_tm ? "" : "!",
                 d->hg2_ext_hdr ? "" : "!");
    soc_cm_print("%s\treason=%08x_%08x timestamp=%d srcport=%d\n",
                 prefix,
                 d->reason_hi,
                 d->reason,
                 d->timestamp,
                 d->srcport);
    soc_cm_print("%s\tcpu_cos=%d uc_cos=%d mc_cos1=%d mc_cos2=%d hgi=%d\n",
                 prefix,
                 d->cpu_cos,
                 d->uc_cos,
                 d->mc_cos1,
                 d->mc_cos2,
                 d->hgi);
    soc_cm_print("%s\touter_vid=%d outer_cfi=%d outer_pri=%d otag_action=%d"
                 " vntag_action=%d\n",
                 prefix,
                 d->outer_vid,
                 d->outer_cfi,
                 d->outer_pri,
                 d->otag_action,
                 d->vntag_action);
    soc_cm_print("%s\tinner_vid=%d inner_cfi=%d inner_pri=%d itag_action=%d"
                 " itag_status=%d\n",
                 prefix,
                 d->inner_vid,
                 d->inner_cfi,
                 d->inner_pri,
                 d->itag_action,
                 d->itag_status);
    soc_cm_print("%s\tdscp=%d ecn=%d \n",
                 prefix,
                 d->dscp,
                 d->ecn);
    soc_cm_print("%s\tnext_pass_cp=%d eh_seg_sel=%d eh_tag_type=%d "
                 " eh_queue_tag=%d\n",
                 prefix,
                 d->next_pass_cp,
                 d->eh_seg_sel,
                 d->eh_tag_type,
                 d->eh_queue_tag);
    soc_cm_print("%s\tdecap_tunnel_type=%d vfi=%d l3_intf=%d match_rule=%d"
                 " mtp_ind=%d\n",
                 prefix,
                 d->decap_tunnel_type,
                 d->vfi,
                 d->l3_intf,
                 d->match_rule,
                 d->mtp_index);
}
#endif /* BROADCOM_DEBUG */

dcb_op_t dcb21_op = {
    21,
    sizeof(dcb21_t),
    dcb21_rx_reason_map,
    NULL,
    dcb19_init,
    dcb19_addtx,
    dcb19_addrx,
    dcb19_intrinfo,
    dcb19_reqcount_set,
    dcb19_reqcount_get,
    dcb19_xfercount_get,
    dcb0_addr_set,
    dcb0_addr_get,
    dcb0_paddr_get,
    dcb19_done_set,
    dcb19_done_get,
    dcb19_sg_set,
    dcb19_sg_get,
    dcb19_chain_set,
    dcb19_chain_get,
    dcb19_reload_set,
    dcb19_reload_get,
    dcb19_tx_l2pbm_set,
    dcb19_tx_utpbm_set,
    dcb19_tx_l3pbm_set,
    dcb19_tx_crc_set,
    dcb19_tx_cos_set,
    dcb19_tx_destmod_set,
    dcb19_tx_destport_set,
    dcb19_tx_opcode_set,
    dcb19_tx_srcmod_set,
    dcb19_tx_srcport_set,
    dcb19_tx_prio_set,
    dcb19_tx_pfm_set,
    dcb19_rx_untagged_get,
    dcb19_rx_crc_get,
    dcb19_rx_cos_get,
    dcb19_rx_destmod_get,
    dcb19_rx_destport_get,
    dcb19_rx_opcode_get,
    dcb19_rx_classtag_get,
    dcb19_rx_matchrule_get,
    dcb19_rx_start_get,
    dcb19_rx_end_get,
    dcb19_rx_error_get,
    dcb19_rx_prio_get,
    dcb19_rx_reason_get,
    dcb19_rx_reason_hi_get,
    dcb19_rx_ingport_get,
    dcb19_rx_srcport_get,
    dcb19_rx_srcmod_get,
    dcb19_rx_mcast_get,
    dcb19_rx_vclabel_get,
    dcb21_rx_mirror_get,
    dcb19_rx_timestamp_get,
    dcb19_rx_timestamp_upper_get,
    dcb19_hg_set,
    dcb19_hg_get,
    dcb19_stat_set,
    dcb19_stat_get,
    dcb19_purge_set,
    dcb19_purge_get,
    dcb19_mhp_get,
    dcb19_outer_vid_get,
    dcb19_outer_pri_get,
    dcb19_outer_cfi_get,
    dcb19_inner_vid_get,
    dcb19_inner_pri_get,
    dcb19_inner_cfi_get,
    dcb19_rx_bpdu_get,
    dcb9_rx_egr_to_cpu_get,
#ifdef  BROADCOM_DEBUG
    dcb19_tx_l2pbm_get,
    dcb19_tx_utpbm_get,
    dcb19_tx_l3pbm_get,
    dcb19_tx_crc_get,
    dcb19_tx_cos_get,
    dcb19_tx_destmod_get,
    dcb19_tx_destport_get,
    dcb19_tx_opcode_get,
    dcb19_tx_srcmod_get,
    dcb19_tx_srcport_get,
    dcb19_tx_prio_get,
    dcb19_tx_pfm_get,

    dcb21_dump,
    dcb0_reason_dump,
#endif /* BROADCOM_DEBUG */
#ifdef  PLISIM
    dcb19_status_init,
    dcb19_xfercount_set,
    dcb19_rx_start_set,
    dcb19_rx_end_set,
    dcb19_rx_error_set,
    dcb19_rx_crc_set,
    dcb19_rx_ingport_set,
#endif /* PLISIM */
};
#endif /* BCM_TRIDENT_SUPPORT */

#if defined(BCM_SHADOW_SUPPORT)
/*
 * DCB Type 22 Support
 */

static soc_rx_reason_t
dcb22_rx_reason_map[] = {
    socRxReasonUnknownVlan,        /* Offset 0 */
    socRxReasonFilterMatch,        /* Offset 1 */
    socRxReasonL3HeaderError,      /* Offset 2 */
    socRxReasonProtocol,           /* Offset 3 */
    socRxReasonDosAttack,          /* Offset 4 */
    socRxReasonMartianAddr,        /* Offset 5 */
    socRxReasonParityError,        /* Offset 6 */
    socRxReasonHigigHdrError,      /* Offset 7 */
    socRxReasonTimeSync,           /* Offset 8 */
    socRxReasonHigigHdrError,      /* Offset 9 */
    socRxReasonInvalid,            /* Offset 10 */
    socRxReasonInvalid,            /* Offset 11 */
    socRxReasonInvalid,            /* Offset 12 */
    socRxReasonInvalid,            /* Offset 13 */
    socRxReasonInvalid,            /* Offset 14 */
    socRxReasonInvalid,            /* Offset 15 */
    socRxReasonInvalid,            /* Offset 16 */
    socRxReasonInvalid,            /* Offset 17 */
    socRxReasonInvalid,            /* Offset 18 */
    socRxReasonInvalid,            /* Offset 19 */
    socRxReasonInvalid,            /* Offset 20 */
    socRxReasonInvalid,            /* Offset 21 */
    socRxReasonInvalid,            /* Offset 22 */
    socRxReasonInvalid,            /* Offset 23 */
    socRxReasonInvalid,            /* Offset 24 */
    socRxReasonInvalid,            /* Offset 25 */
    socRxReasonInvalid,            /* Offset 26 */
    socRxReasonInvalid,            /* Offset 27 */
    socRxReasonInvalid,            /* Offset 28 */
    socRxReasonInvalid,            /* Offset 29 */
    socRxReasonInvalid,            /* Offset 30 */
    socRxReasonInvalid,            /* Offset 31 */
    socRxReasonInvalid,            /* Offset 32 */
    socRxReasonInvalid,            /* Offset 33 */
    socRxReasonInvalid,            /* Offset 34 */
    socRxReasonInvalid,            /* Offset 35 */
    socRxReasonInvalid,            /* Offset 36 */
    socRxReasonInvalid,            /* Offset 37 */
    socRxReasonInvalid,            /* Offset 38 */
    socRxReasonInvalid,            /* Offset 39 */
    socRxReasonInvalid,            /* Offset 40 */
    socRxReasonInvalid,            /* Offset 41 */
    socRxReasonInvalid,            /* Offset 42 */
    socRxReasonInvalid,            /* Offset 43 */
    socRxReasonInvalid,            /* Offset 44 */
    socRxReasonInvalid,            /* Offset 45 */
    socRxReasonInvalid,            /* Offset 46 */
    socRxReasonInvalid,            /* Offset 47 */
    socRxReasonInvalid,            /* Offset 48 */
    socRxReasonInvalid,            /* Offset 49 */
    socRxReasonInvalid,            /* Offset 50 */
    socRxReasonInvalid,            /* Offset 51 */
    socRxReasonInvalid,            /* Offset 52 */
    socRxReasonInvalid,            /* Offset 53 */
    socRxReasonInvalid,            /* Offset 54 */
    socRxReasonInvalid,            /* Offset 55 */
    socRxReasonInvalid,            /* Offset 56 */
    socRxReasonInvalid,            /* Offset 57 */
    socRxReasonInvalid,            /* Offset 58 */
    socRxReasonInvalid,            /* Offset 59 */
    socRxReasonInvalid,            /* Offset 60 */
    socRxReasonInvalid,            /* Offset 61 */
    socRxReasonInvalid,            /* Offset 62 */
    socRxReasonInvalid             /* Offset 63 */
};


#ifdef BROADCOM_DEBUG
static void
dcb22_dump(int unit, dcb_t *dcb, char *prefix, int tx)
{
    uint32      *p;
    int         i, size;
    dcb22_t *d = (dcb22_t *)dcb;
    char        ps[((DCB_MAX_SIZE/sizeof(uint32))*9)+1];
#if defined(LE_HOST)
    uint32  hgh[4];
    uint8 *h = (uint8 *)&hgh[0];

    hgh[0] = soc_htonl(d->mh0);
    hgh[1] = soc_htonl(d->mh1);
    hgh[2] = soc_htonl(d->mh2);
    hgh[3] = soc_htonl(d->mh3);
#else
    uint8 *h = (uint8 *)&d->mh0;
#endif

    p = (uint32 *)dcb;
    size = SOC_DCB_SIZE(unit) / sizeof(uint32);
    for (i = 0; i < size; i++) {
        sal_sprintf(&ps[i*9], "%08x ", p[i]);
    }
    soc_cm_print("%s\t%s\n", prefix, ps);
    if ((SOC_DCB_HG_GET(unit, dcb)) || (SOC_DCB_RX_START_GET(unit, dcb))) {
        soc_dma_higig_dump(unit, prefix, h, 0, 0, NULL);
    }
    soc_cm_print(
        "%s\ttype %d %ssg %schain %sreload %shg %sstat %spause %spurge\n",
        prefix,
        SOC_DCB_TYPE(unit),
        SOC_DCB_SG_GET(unit, dcb) ? "" : "!",
        SOC_DCB_CHAIN_GET(unit, dcb) ? "" : "!",
        SOC_DCB_RELOAD_GET(unit, dcb) ? "" : "!",
        SOC_DCB_HG_GET(unit, dcb) ? "" : "!",
        SOC_DCB_STAT_GET(unit, dcb) ? "" : "!",
        d->c_pause ? "" : "!",
        SOC_DCB_PURGE_GET(unit, dcb) ? "" : "!");
    soc_cm_print(
        "%s\taddr %p reqcount %d xfercount %d\n",
        prefix,
        (void *)SOC_DCB_ADDR_GET(unit, dcb),
        SOC_DCB_REQCOUNT_GET(unit, dcb),
        SOC_DCB_XFERCOUNT_GET(unit, dcb));
    if (!tx) {
        soc_cm_print(
            "%s\t%sdone %sstart %send %serror\n",
            prefix,
            SOC_DCB_DONE_GET(unit, dcb) ? "" : "!",
            SOC_DCB_RX_START_GET(unit, dcb) ? "" : "!",
            SOC_DCB_RX_END_GET(unit, dcb) ? "" : "!",
            SOC_DCB_RX_ERROR_GET(unit, dcb) ? "" : "!");
    }
    if ((!tx) && (SOC_DCB_RX_START_GET(unit, dcb))) {
        dcb0_reason_dump(unit, dcb, prefix);
    }
}
#endif /* BROADCOM_DEBUG */

GETFUNCFIELD(22, rx_matchrule, match_rule)
GETFUNCFIELD(22, rx_timestamp_lo, timestamp_lo)
GETFUNCFIELD(22, rx_timestamp_hi, timestamp_hi)
GETFUNCNULL(22, rx_egr_to_cpu)

dcb_op_t dcb22_op = {
    22,
    sizeof(dcb22_t),
    dcb22_rx_reason_map,
    NULL,
    dcb19_init,
    dcb19_addtx,
    dcb19_addrx,
    dcb19_intrinfo,
    dcb19_reqcount_set,
    dcb19_reqcount_get,
    dcb19_xfercount_get,
    dcb0_addr_set,
    dcb0_addr_get,
    dcb0_paddr_get,
    dcb19_done_set,
    dcb19_done_get,
    dcb19_sg_set,
    dcb19_sg_get,
    dcb19_chain_set,
    dcb19_chain_get,
    dcb19_reload_set,
    dcb19_reload_get,
    dcb19_tx_l2pbm_set,
    dcb19_tx_utpbm_set,
    dcb19_tx_l3pbm_set,
    dcb19_tx_crc_set,
    dcb19_tx_cos_set,
    dcb19_tx_destmod_set,
    dcb19_tx_destport_set,
    dcb19_tx_opcode_set,
    dcb19_tx_srcmod_set,
    dcb19_tx_srcport_set,
    dcb19_tx_prio_set,
    dcb19_tx_pfm_set,
    dcb19_rx_untagged_get,
    dcb19_rx_crc_get,
    dcb19_rx_cos_get,
    dcb19_rx_destmod_get,
    dcb19_rx_destport_get,
    dcb19_rx_opcode_get,
    dcb19_rx_classtag_get,
    dcb22_rx_matchrule_get,
    dcb19_rx_start_get,
    dcb19_rx_end_get,
    dcb19_rx_error_get,
    dcb19_rx_prio_get,
    dcb19_rx_reason_get,
    dcb19_rx_reason_hi_get,
    dcb19_rx_ingport_get,
    dcb19_rx_srcport_get,
    dcb19_rx_srcmod_get,
    dcb19_rx_mcast_get,
    dcb19_rx_vclabel_get,
    dcb19_rx_mirror_get,
    dcb22_rx_timestamp_lo_get,
    dcb22_rx_timestamp_hi_get,
    dcb19_hg_set,
    dcb19_hg_get,
    dcb19_stat_set,
    dcb19_stat_get,
    dcb19_purge_set,
    dcb19_purge_get,
    dcb19_mhp_get,
    dcb19_outer_vid_get,
    dcb19_outer_pri_get,
    dcb19_outer_cfi_get,
    dcb19_inner_vid_get,
    dcb19_inner_pri_get,
    dcb19_inner_cfi_get,
    dcb19_rx_bpdu_get,
    dcb22_rx_egr_to_cpu_get, 
#ifdef  BROADCOM_DEBUG
    dcb19_tx_l2pbm_get,
    dcb19_tx_utpbm_get,
    dcb19_tx_l3pbm_get,
    dcb19_tx_crc_get,
    dcb19_tx_cos_get,
    dcb19_tx_destmod_get,
    dcb19_tx_destport_get,
    dcb19_tx_opcode_get,
    dcb19_tx_srcmod_get,
    dcb19_tx_srcport_get,
    dcb19_tx_prio_get,
    dcb19_tx_pfm_get,

    dcb22_dump,
    dcb0_reason_dump,
#endif /* BROADCOM_DEBUG */
#ifdef  PLISIM
    dcb19_status_init,
    dcb19_xfercount_set,
    dcb19_rx_start_set,
    dcb19_rx_end_set,
    dcb19_rx_error_set,
    dcb19_rx_crc_set,
    dcb19_rx_ingport_set,
#endif /* PLISIM */
};
#endif /* BCM_SHADOW_SUPPORT */
#if defined(BCM_KATANA_SUPPORT)
/*
 * DCB Type 23 Support
 */
static soc_rx_reason_t
dcb23_rx_reason_map_ip_0[] = { /* IP Overlay 0 */
    socRxReasonUnknownVlan,        /* Offset 0 */
    socRxReasonL2SourceMiss,       /* Offset 1 */
    socRxReasonL2DestMiss,         /* Offset 2 */
    socRxReasonL2Move,             /* Offset 3 */
    socRxReasonL2Cpu,              /* Offset 4 */
    socRxReasonSampleSource,       /* Offset 5 */
    socRxReasonSampleDest,         /* Offset 6 */
    socRxReasonL3SourceMiss,       /* Offset 7 */
    socRxReasonL3DestMiss,         /* Offset 8 */
    socRxReasonL3SourceMove,       /* Offset 9 */
    socRxReasonMcastMiss,          /* Offset 10 */
    socRxReasonIpMcastMiss,        /* Offset 11 */
    socRxReasonL3HeaderError,      /* Offset 12 */
    socRxReasonProtocol,           /* Offset 13 */
    socRxReasonDosAttack,          /* Offset 14 */
    socRxReasonMartianAddr,        /* Offset 15 */
    socRxReasonTunnelError,        /* Offset 16 */
    socRxReasonMirror,             /* Offset 17 */ 
    socRxReasonIcmpRedirect,       /* Offset 18 */
    socRxReasonL3Slowpath,         /* Offset 19 */
    socRxReasonL3MtuFail,          /* Offset 20 */
    socRxReasonMcastIdxError,      /* Offset 21 */
    socRxReasonVlanFilterMatch,    /* Offset 22 */
    socRxReasonClassBasedMove,     /* Offset 23 */
    socRxReasonL3AddrBindFail,     /* Offset 24 */
    socRxReasonMplsLabelMiss,      /* Offset 25 */
    socRxReasonMplsInvalidAction,  /* Offset 26 */
    socRxReasonMplsInvalidPayload, /* Offset 27 */
    socRxReasonMplsTtl,            /* Offset 28 */
    socRxReasonMplsSequenceNumber, /* Offset 29 */
    socRxReasonL2NonUnicastMiss,   /* Offset 30 */
    socRxReasonNhop,               /* Offset 31 */
    socRxReasonMplsCtrlWordError,  /* Offset 32 */
    socRxReasonStation,            /* Offset 33 */
    socRxReasonVlanTranslate,      /* Offset 34 */
    socRxReasonTimeSync,           /* Offset 35 */
    socRxReasonOAMSlowpath,        /* Offset 36 */
    socRxReasonOAMError,           /* Offset 37 */
    socRxReasonIpfixRateViolation, /* Offset 38 */
    socRxReasonL2LearnLimit,       /* Offset 39 */
    socRxReasonEncapHigigError,    /* Offset 40 */
    socRxReasonFlowTracker,        
    socRxReasonOAMLMDM,            /* Offset 42 */
    socRxReasonBfd,                /* Offset 43 */
    socRxReasonBfdSlowpath,        /* Offset 44 */
    socRxReasonProtectionDrop,     
    socRxReasonTrillNickname,      
    socRxReasonTrillTtl,           /* Offset 47 */
    socRxReasonTrillCoreIsIs,      /* Offset 48 */
    socRxReasonTrillSlowpath,      /* Offset 49 */
    socRxReasonTrillRpfFail,       /* Offset 50 */
    socRxReasonTrillMiss,          /* Offset 51 */
    socRxReasonTrillInvalid,       /* Offset 52 */
    socRxReasonNivUntagDrop,       /* Offset 53 */
    socRxReasonNivTagDrop,         /* Offset 54 */
    socRxReasonNivTagInvalid,      /* Offset 55 */
    socRxReasonNivRpfFail,         /* Offset 56 */
    socRxReasonNivInterfaceMiss,   /* Offset 57 */
    socRxReasonNivPrioDrop,        /* Offset 58 */
    socRxReasonParityError,        /* Offset 59 */
    socRxReasonHigigHdrError,      /* Offset 60 */
    socRxReasonFilterMatch,        /* Offset 61 */
    socRxReasonInvalid,            /* Offset 62 */
    socRxReasonInvalid             /* Offset 63 */
};

#ifdef  BROADCOM_DEBUG
static soc_rx_reason_t
dcb23_rx_reason_map_ip_1[] = { /* IP Overlay 1 */
    socRxReasonUnknownVlan,        /* Offset 0 */
    socRxReasonL2SourceMiss,       /* Offset 1 */
    socRxReasonL2DestMiss,         /* Offset 2 */
    socRxReasonL2Move,             /* Offset 3 */
    socRxReasonL2Cpu,              /* Offset 4 */
    socRxReasonSampleSource,       /* Offset 5 */
    socRxReasonSampleDest,         /* Offset 6 */
    socRxReasonL3SourceMiss,       /* Offset 7 */
    socRxReasonL3DestMiss,         /* Offset 8 */
    socRxReasonL3SourceMove,       /* Offset 9 */
    socRxReasonMcastMiss,          /* Offset 10 */
    socRxReasonIpMcastMiss,        /* Offset 11 */
    socRxReasonL3HeaderError,      /* Offset 12 */
    socRxReasonProtocol,           /* Offset 13 */
    socRxReasonDosAttack,          /* Offset 14 */
    socRxReasonMartianAddr,        /* Offset 15 */
    socRxReasonTunnelError,        /* Offset 16 */
    socRxReasonMirror,             /* Offset 17 */ 
    socRxReasonIcmpRedirect,       /* Offset 18 */
    socRxReasonL3Slowpath,         /* Offset 19 */
    socRxReasonL3MtuFail,          /* Offset 20 */
    socRxReasonMcastIdxError,      /* Offset 21 */
    socRxReasonVlanFilterMatch,    /* Offset 22 */
    socRxReasonClassBasedMove,     /* Offset 23 */
    socRxReasonL3AddrBindFail,     /* Offset 24 */
    socRxReasonMplsLabelMiss,      /* Offset 25 */
    socRxReasonMplsInvalidAction,  /* Offset 26 */
    socRxReasonMplsInvalidPayload, /* Offset 27 */
    socRxReasonMplsTtl,            /* Offset 28 */
    socRxReasonMplsSequenceNumber, /* Offset 29 */
    socRxReasonL2NonUnicastMiss,   /* Offset 30 */
    socRxReasonNhop,               /* Offset 31 */
    socRxReasonMplsCtrlWordError,  /* Offset 32 */
    socRxReasonStation,            /* Offset 33 */
    socRxReasonVlanTranslate,      /* Offset 34 */
    socRxReasonTimeSync,           /* Offset 35 */
    socRxReasonOAMSlowpath,        /* Offset 36 */
    socRxReasonOAMError,           /* Offset 37 */
    socRxReasonIpfixRateViolation, /* Offset 38 */
    socRxReasonL2LearnLimit,       /* Offset 39 */
    socRxReasonEncapHigigError,    /* Offset 40 */
    socRxReasonFlowTracker,        
    socRxReasonOAMLMDM,            /* Offset 42 */
    socRxReasonBfd,                /* Offset 43 */
    socRxReasonBfdSlowpath,        /* Offset 44 */
    socRxReasonProtectionDrop,     
    socRxReasonWlanSlowpathKeepalive, /* Offset 46 */
    socRxReasonWlanShimHdrError,   
    socRxReasonWlanSlowpath,       /* Offset 48 */
    socRxReasonWlanDot1xDrop,      /* Offset 49 */
    socRxReasonMplsEntropyLabelRange, 
    socRxReasonQcnCnmPrp,          
    socRxReasonQcnCnmPrpDlf,       
    socRxReasonIcnm,               
    socRxReasonMplsUnknownAch,     
    socRxReasonMplsLookupsExceeded, 
    socRxReasonMplsReservedLabel,  
    socRxReasonMplsAlertLabel,     
    socRxReasonInvalid,            /* Offset 58 */
    socRxReasonParityError,        /* Offset 59 */
    socRxReasonHigigHdrError,      /* Offset 60 */
    socRxReasonFilterMatch,        /* Offset 61 */
    socRxReasonInvalid,            /* Offset 62 */
    socRxReasonInvalid             /* Offset 63 */
};

static soc_rx_reason_t
dcb23_rx_reason_map_ep[] = {
    socRxReasonUnknownVlan,        /* Offset 0 */
    socRxReasonStp,                /* Offset 1 */
    socRxReasonVlanTranslate,      /* Offset 2 new */
    socRxReasonTunnelError,        /* Offset 3 */
    socRxReasonIpmc,               /* Offset 4 */
    socRxReasonL3HeaderError,      /* Offset 5 */
    socRxReasonTtl,                /* Offset 6 */
    socRxReasonL2MtuFail,          /* Offset 7 */
    socRxReasonHigigHdrError,      /* Offset 8 */
    socRxReasonSplitHorizon,       /* Offset 9 */
    socRxReasonNivPrune,           
    socRxReasonSplitHorizonPrune,  
    socRxReasonFilterMatch,        /* Offset 12 */
    socRxReasonMultiDest,          
    socRxReasonTrillPortMismatch,  /* Offset 14 */
    socRxReasonInvalid,            /* Offset 15 */
    socRxReasonInvalid,            /* Offset 16 */
    socRxReasonInvalid,            /* Offset 17 */
    socRxReasonInvalid,            /* Offset 18 */
    socRxReasonInvalid,            /* Offset 19 */
    socRxReasonInvalid,            /* Offset 20 */
    socRxReasonInvalid,            /* Offset 21 */
    socRxReasonInvalid,            /* Offset 22 */
    socRxReasonInvalid,            /* Offset 23 */
    socRxReasonInvalid,            /* Offset 24 */
    socRxReasonInvalid,            /* Offset 25 */
    socRxReasonInvalid,            /* Offset 26 */
    socRxReasonInvalid,            /* Offset 27 */
    socRxReasonInvalid,            /* Offset 28 */
    socRxReasonInvalid,            /* Offset 29 */
    socRxReasonInvalid,            /* Offset 30 */
    socRxReasonInvalid,            /* Offset 31 */
    socRxReasonInvalid,            /* Offset 32 */
    socRxReasonInvalid,            /* Offset 33 */
    socRxReasonInvalid,            /* Offset 34 */
    socRxReasonInvalid,            /* Offset 35 */
    socRxReasonInvalid,            /* Offset 36 */
    socRxReasonInvalid,            /* Offset 37 */
    socRxReasonInvalid,            /* Offset 38 */
    socRxReasonInvalid,            /* Offset 39 */
    socRxReasonInvalid,            /* Offset 40 */
    socRxReasonInvalid,            /* Offset 41 */
    socRxReasonInvalid,            /* Offset 42 */
    socRxReasonInvalid,            /* Offset 43 */
    socRxReasonInvalid,            /* Offset 44 */
    socRxReasonInvalid,            /* Offset 45 */
    socRxReasonInvalid,            /* Offset 46 */
    socRxReasonInvalid,            /* Offset 47 */
    socRxReasonInvalid,            /* Offset 48 */
    socRxReasonInvalid,            /* Offset 49 */
    socRxReasonInvalid,            /* Offset 50 */
    socRxReasonInvalid,            /* Offset 51 */
    socRxReasonInvalid,            /* Offset 52 */
    socRxReasonInvalid,            /* Offset 53 */
    socRxReasonInvalid,            /* Offset 54 */
    socRxReasonInvalid,            /* Offset 55 */
    socRxReasonInvalid,            /* Offset 56 */
    socRxReasonInvalid,            /* Offset 57 */
    socRxReasonInvalid,            /* Offset 58 */
    socRxReasonInvalid,            /* Offset 59 */
    socRxReasonInvalid,            /* Offset 60 */
    socRxReasonInvalid,            /* Offset 61 */
    socRxReasonInvalid,            /* Offset 62 */
    socRxReasonInvalid             /* Offset 63 */
};

static soc_rx_reason_t
dcb23_rx_reason_map_nlf[] = {
    socRxReasonSignatureMatch,     /* Offset 0 */
    socRxReasonWlanClientMove,     /* Offset 1 */
    socRxReasonWlanSourcePortMiss, /* Offset 2 */
    socRxReasonWlanDatabaseError,  /* Offset 3 */
    socRxReasonWlanSourceMiss,     /* Offset 4 */
    socRxReasonWlanDestMiss,       /* Offset 5 */
    socRxReasonWlanMtu,            /* Offset 6 */
    socRxReasonInvalid,            /* Offset 7 */
    socRxReasonInvalid,            /* Offset 8 */
    socRxReasonInvalid,            /* Offset 9 */
    socRxReasonInvalid,            /* Offset 10 */
    socRxReasonInvalid,            /* Offset 11 */
    socRxReasonInvalid,            /* Offset 12 */
    socRxReasonInvalid,            /* Offset 13 */
    socRxReasonInvalid,            /* Offset 14 */
    socRxReasonInvalid,            /* Offset 15 */
    socRxReasonInvalid,            /* Offset 16 */
    socRxReasonInvalid,            /* Offset 17 */
    socRxReasonInvalid,            /* Offset 18 */
    socRxReasonInvalid,            /* Offset 19 */
    socRxReasonInvalid,            /* Offset 20 */
    socRxReasonInvalid,            /* Offset 21 */
    socRxReasonInvalid,            /* Offset 22 */
    socRxReasonInvalid,            /* Offset 23 */
    socRxReasonInvalid,            /* Offset 24 */
    socRxReasonInvalid,            /* Offset 25 */
    socRxReasonInvalid,            /* Offset 26 */
    socRxReasonInvalid,            /* Offset 27 */
    socRxReasonInvalid,            /* Offset 28 */
    socRxReasonInvalid,            /* Offset 29 */
    socRxReasonInvalid,            /* Offset 30 */
    socRxReasonInvalid,            /* Offset 31 */
    socRxReasonInvalid,            /* Offset 32 */
    socRxReasonInvalid,            /* Offset 33 */
    socRxReasonInvalid,            /* Offset 34 */
    socRxReasonInvalid,            /* Offset 35 */
    socRxReasonInvalid,            /* Offset 36 */
    socRxReasonInvalid,            /* Offset 37 */
    socRxReasonInvalid,            /* Offset 38 */
    socRxReasonInvalid,            /* Offset 39 */
    socRxReasonInvalid,            /* Offset 40 */
    socRxReasonInvalid,            /* Offset 41 */
    socRxReasonInvalid,            /* Offset 42 */
    socRxReasonInvalid,            /* Offset 43 */
    socRxReasonInvalid,            /* Offset 44 */
    socRxReasonInvalid,            /* Offset 45 */
    socRxReasonInvalid,            /* Offset 46 */
    socRxReasonInvalid,            /* Offset 47 */
    socRxReasonInvalid,            /* Offset 48 */
    socRxReasonInvalid,            /* Offset 49 */
    socRxReasonInvalid,            /* Offset 50 */
    socRxReasonInvalid,            /* Offset 51 */
    socRxReasonInvalid,            /* Offset 52 */
    socRxReasonInvalid,            /* Offset 53 */
    socRxReasonInvalid,            /* Offset 54 */
    socRxReasonInvalid,            /* Offset 55 */
    socRxReasonInvalid,            /* Offset 56 */
    socRxReasonInvalid,            /* Offset 57 */
    socRxReasonInvalid,            /* Offset 58 */
    socRxReasonInvalid,            /* Offset 59 */
    socRxReasonInvalid,            /* Offset 60 */
    socRxReasonInvalid,            /* Offset 61 */
    socRxReasonInvalid,            /* Offset 62 */
    socRxReasonInvalid             /* Offset 63 */
};

static void
dcb23_reason_dump(int unit, dcb_t *dcb, char *prefix)
{
    soc_rx_reason_t *map;
    uint32 reason = SOC_DCB_RX_REASON_GET(unit, dcb);
    uint32 mask = 1;
    int i;
    dcb23_t     *d = (dcb23_t *)dcb;

    switch (d->word4.overlay1.cpu_opcode_type) {
    case SOC_CPU_OPCODE_TYPE_IP_0:
        map = dcb23_rx_reason_map_ip_0;
        break;
    case SOC_CPU_OPCODE_TYPE_IP_1:
        map = dcb23_rx_reason_map_ip_1;
        break;
    case SOC_CPU_OPCODE_TYPE_EP:
        map = dcb23_rx_reason_map_ep;
        break;
    case SOC_CPU_OPCODE_TYPE_NLF:
        map = dcb23_rx_reason_map_nlf;
        break;
    default:
        soc_cm_print("%s\tUnknown reason type!\n", prefix);
        return;        
    }
    
    for (i=0; i < 32; i++) {
        if ((mask & reason)) {
            soc_cm_print("%s\treason bit %d: %s\n", prefix, i, 
                         _dcb_reason_names[map[i]]);
        }
        mask <<= 1;
    }
    
    if (soc_feature(unit, soc_feature_dcb_reason_hi)) {
        reason = SOC_DCB_RX_REASON_HI_GET(unit, dcb);
        mask = 1;
    
        for (i=0; i < 32; i++) {
            if ((mask & reason)) {
                soc_cm_print("%s\treason bit %d: %s\n", prefix, i + 32,
                             _dcb_reason_names[map[i + 32]]);
            }
            mask <<= 1;
        }
    }

}
#endif /* BROADCOM_DEBUG */

GETFUNCFIELD(23, xfercount, count)
GETFUNCFIELD(23, rx_cos, word4.overlay1.queue_num)

/* Fields extracted from MH/PBI */
GETHG2FUNCFIELD(23, rx_destmod, dst_mod)
GETHG2FUNCFIELD(23, rx_destport, dst_port)
GETHG2FUNCFIELD(23, rx_srcmod, src_mod)
GETHG2FUNCFIELD(23, rx_srcport, src_port)
GETHG2FUNCFIELD(23, rx_opcode, opcode)
GETHG2FUNCFIELD(23, rx_prio, vlan_pri) /* outer_pri */
GETHG2FUNCEXPR(23, rx_mcast, ((h->ppd_overlay1.dst_mod << 8) |
                              (h->ppd_overlay1.dst_port)))
GETHG2FUNCEXPR(23, rx_vclabel, ((h->ppd_overlay1.vc_label_19_16 << 16) |
                              (h->ppd_overlay1.vc_label_15_8 << 8) |
                              (h->ppd_overlay1.vc_label_7_0)))
GETHG2FUNCEXPR(23, rx_classtag, (h->ppd_overlay1.ppd_type != 1 ? 0 :
                                 (h->ppd_overlay2.ctag_hi << 8) |
                                 (h->ppd_overlay2.ctag_lo)))
GETFUNCUNITEXPR(23, rx_untagged, (IS_HG_PORT(unit, d->srcport) ?
                                  ((d->itag_status) ? 0 : 2) :
                                  ((d->itag_status & 0x2) ?
                                   ((d->itag_status & 0x1) ? 0 : 2) :
                                   ((d->itag_status & 0x1) ? 1 : 3))))

GETFUNCFIELD(23, rx_matchrule, match_rule)
GETFUNCFIELD(23, rx_reason, reason)
GETFUNCFIELD(23, rx_reason_hi, reason_hi)
GETFUNCFIELD(23, rx_ingport, srcport)
GETFUNCEXPR(23, rx_mirror, ((SOC_CPU_OPCODE_TYPE_IP_0 ==
                            d->word4.overlay1.cpu_opcode_type) ?
                            (d->reason & (1 << 17)) : 0))
GETFUNCFIELD(23, rx_timestamp, word12.overlay1.timestamp)
GETFUNCFIELD(23, rx_timestamp_upper, word14.overlay1.timestamp_hi)
GETPTREXPR(23, mhp, &(d->mh0))
GETFUNCFIELD(23, outer_vid, word4.overlay1.outer_vid)
GETFUNCFIELD(23, outer_pri, word11.overlay1.outer_pri)
GETFUNCFIELD(23, outer_cfi, word11.overlay1.outer_cfi)
GETFUNCFIELD(23, inner_vid, word11.overlay1.inner_vid)
GETFUNCFIELD(23, inner_pri, inner_pri)
GETFUNCFIELD(23, inner_cfi, word11.overlay1.inner_cfi)
GETFUNCFIELD(23, rx_bpdu, bpdu)

#ifdef BROADCOM_DEBUG
static void
dcb23_dump(int unit, dcb_t *dcb, char *prefix, int tx)
{
    uint32      *p;
    int         i, size;
    dcb19_t     *dtx = (dcb19_t *)dcb;  /* Fake out for different TX MH */
    dcb23_t     *d = (dcb23_t *)dcb;
    char        ps[((DCB_MAX_SIZE/sizeof(uint32))*9)+1];
    uint8       *h;
#if defined(LE_HOST)
    uint32      hgh[4];
    h = (uint8 *)&hgh[0];

    if (tx) {
        hgh[0] = soc_htonl(dtx->mh0);
        hgh[1] = soc_htonl(dtx->mh1);
        hgh[2] = soc_htonl(dtx->mh2);
        hgh[3] = soc_htonl(dtx->mh3);
    } else {
        hgh[0] = soc_htonl(d->mh0);
        hgh[1] = soc_htonl(d->mh1);
        hgh[2] = soc_htonl(d->mh2);
        hgh[3] = soc_htonl(d->mh3);
    }
#else
    if (tx) {
        h = (uint8 *)&dtx->mh0;
    } else {
        h = (uint8 *)&d->mh0;
    }
#endif

    p = (uint32 *)dcb;
    size = SOC_DCB_SIZE(unit) / sizeof(uint32);
    for (i = 0; i < size; i++) {
        sal_sprintf(&ps[i*9], "%08x ", p[i]);
    }
    soc_cm_print("%s\t%s\n", prefix, ps);
    if ((SOC_DCB_HG_GET(unit, dcb)) || (SOC_DCB_RX_START_GET(unit, dcb))) {
        soc_dma_higig_dump(unit, prefix, h, 0, 0, NULL);
    }
    soc_cm_print("%s\ttype %d %schain %ssg %sreload %shg %sstat %spause "
                 " %spurge\n",
                 prefix,
                 SOC_DCB_TYPE(unit),
                 SOC_DCB_CHAIN_GET(unit, dcb) ? "" : "!",
                 SOC_DCB_SG_GET(unit, dcb) ? "" : "!",
                 SOC_DCB_RELOAD_GET(unit, dcb) ? "" : "!",
                 SOC_DCB_HG_GET(unit, dcb) ? "" : "!",
                 SOC_DCB_STAT_GET(unit, dcb) ? "" : "!",
                 d->c_pause ? "" : "!",
                 SOC_DCB_PURGE_GET(unit, dcb) ? "" : "!");
    soc_cm_print("%s\taddr %p reqcount %d xfercount %d\n",
                 prefix,
                 (void *)SOC_DCB_ADDR_GET(unit, dcb),
                 SOC_DCB_REQCOUNT_GET(unit, dcb),
                 SOC_DCB_XFERCOUNT_GET(unit, dcb));
    if (!tx) {
        soc_cm_print("%s\t%sdone %serror %sstart %send\n",
                     prefix,
                     SOC_DCB_DONE_GET(unit, dcb) ? "" : "!",
                     SOC_DCB_RX_ERROR_GET(unit, dcb) ? "" : "!",
                     SOC_DCB_RX_START_GET(unit, dcb) ? "" : "!",
                     SOC_DCB_RX_END_GET(unit, dcb) ? "" : "!");
    }

    if (tx || !SOC_DCB_RX_START_GET(unit, dcb)) {
        return;
    }

    dcb23_reason_dump(unit, dcb, prefix);
    soc_cm_print("%s\t%schg_tos %sregen_crc %schg_ecn %svfi_valid"
                 " %sdvp_nhi_sel\n",
                 prefix,
                 d->word11.overlay1.chg_tos ? "" : "!",
                 d->regen_crc ? "" : "!",
                 d->chg_ecn ? "" : "!",
                 d->vfi_valid ? "" : "!",
                 d->dvp_nhi_sel ? "" : "!");
    soc_cm_print("%s\t%sservice_tag %sswitch_pkt %shg_type %ssrc_hg\n",
                 prefix,
                 d->service_tag ? "" : "!",
                 d->switch_pkt ? "" : "!",
                 d->hg_type ? "" : "!",
                 d->src_hg ? "" : "!");
    soc_cm_print("%s\t%sl3routed %sl3only %sreplicated %sdo_not_change_ttl"
                 " %sbpdu\n",
                 prefix,
                 d->l3routed ? "" : "!",
                 d->l3only ? "" : "!",
                 d->replicated ? "" : "!",
                 d->do_not_change_ttl ? "" : "!",
                 d->bpdu ? "" : "!");
    soc_cm_print("%s\t%shg2_ext_hdr\n",
                 prefix,
                 d->hg2_ext_hdr ? "" : "!");
    if (d->hg2_ext_hdr) {
        soc_cm_print("%s\tHigig2 Extension: queue_tag=%04x %stm tag_type=%d"
                     " seg_sel=%d\n",
                     prefix,
                     d->word14.overlay2.eh_queue_tag,
                     (d->word14.overlay2.eh_tm) ? "" : "!",
                     d->word14.overlay2.eh_tag_type,
                     d->word14.overlay2.eh_seg_sel);
    }
    soc_cm_print("%s\treason_type=%d reason=%08x_%08x ts_type=%d"
                 " timestamp=%08x_%08x",
                 prefix,
                 d->word4.overlay1.cpu_opcode_type,
                 d->reason_hi,
                 d->reason,
                 d->timestamp_type,
                 d->word14.overlay1.timestamp_hi,
                 d->word12.overlay1.timestamp);
    soc_cm_print("%s\tsrcport=%d cpu_cos=%d hgi=%d lb_pkt_type=%d"
                 " repl_nhi=%05x\n",
                 prefix,
                 d->word4.overlay1.queue_num,
                 d->hgi,
                 d->loopback_pkt_type,
                 d->srcport,
                 d->repl_nhi);
    soc_cm_print("%s\touter_vid=%d outer_cfi=%d outer_pri=%d otag_action=%d"
                 " vntag_action=%d\n",
                 prefix,
                 d->word4.overlay1.outer_vid,
                 d->word11.overlay1.outer_cfi,
                 d->word11.overlay1.outer_pri,
                 d->otag_action,
                 d->word11.overlay1.vntag_action);
    soc_cm_print("%s\tinner_vid=%d inner_cfi=%d inner_pri=%d itag_action=%d"
                 " itag_status=%d\n",
                 prefix,
                 d->word11.overlay1.inner_vid,
                 d->word11.overlay1.inner_cfi,
                 d->inner_pri,
                 d->itag_action,
                 d->itag_status);
    soc_cm_print("%s\tdscp=%d ecn=%d %sspecial_pkt %sall_switch_drop\n",
                 prefix,
                 d->word11.overlay1.dscp,
                 d->ecn,
                 d->special_pkt ? "" : "!",
                 d->all_switch_drop ? "" : "!");
    soc_cm_print("%s\tdecap_tunnel_type=%d vfi=%d match_rule=%d"
                 " mtp_ind=%d\n",
                 prefix,
                 d->word11.overlay1.decap_tunnel_type,
                 d->word12.overlay2.vfi,
                 d->match_rule,
                 d->mtp_index);
}
#endif /* BROADCOM_DEBUG */

dcb_op_t dcb23_op = {
    23,
    sizeof(dcb23_t),
    dcb23_rx_reason_map_ip_0,
    NULL,
    dcb19_init,
    dcb19_addtx,
    dcb19_addrx,
    dcb19_intrinfo,
    dcb19_reqcount_set,
    dcb19_reqcount_get,
    dcb23_xfercount_get,
    dcb0_addr_set,
    dcb0_addr_get,
    dcb0_paddr_get,
    dcb19_done_set,
    dcb19_done_get,
    dcb19_sg_set,
    dcb19_sg_get,
    dcb19_chain_set,
    dcb19_chain_get,
    dcb19_reload_set,
    dcb19_reload_get,
    dcb19_tx_l2pbm_set,
    dcb19_tx_utpbm_set,
    dcb19_tx_l3pbm_set,
    dcb19_tx_crc_set,
    dcb19_tx_cos_set,
    dcb19_tx_destmod_set,
    dcb19_tx_destport_set,
    dcb19_tx_opcode_set,
    dcb19_tx_srcmod_set,
    dcb19_tx_srcport_set,
    dcb19_tx_prio_set,
    dcb19_tx_pfm_set,
    dcb23_rx_untagged_get,
    dcb19_rx_crc_get,
    dcb23_rx_cos_get,
    dcb23_rx_destmod_get,
    dcb23_rx_destport_get,
    dcb23_rx_opcode_get,
    dcb23_rx_classtag_get,
    dcb23_rx_matchrule_get,
    dcb19_rx_start_get,
    dcb19_rx_end_get,
    dcb19_rx_error_get,
    dcb23_rx_prio_get,
    dcb23_rx_reason_get,
    dcb23_rx_reason_hi_get,
    dcb23_rx_ingport_get,
    dcb23_rx_srcport_get,
    dcb23_rx_srcmod_get,
    dcb23_rx_mcast_get,
    dcb23_rx_vclabel_get,
    dcb23_rx_mirror_get,
    dcb23_rx_timestamp_get,
    dcb23_rx_timestamp_upper_get,
    dcb19_hg_set,
    dcb19_hg_get,
    dcb19_stat_set,
    dcb19_stat_get,
    dcb19_purge_set,
    dcb19_purge_get,
    dcb23_mhp_get,
    dcb23_outer_vid_get,
    dcb23_outer_pri_get,
    dcb23_outer_cfi_get,
    dcb23_inner_vid_get,
    dcb23_inner_pri_get,
    dcb23_inner_cfi_get,
    dcb23_rx_bpdu_get,
    dcb9_rx_egr_to_cpu_get,
#ifdef  BROADCOM_DEBUG
    dcb19_tx_l2pbm_get,
    dcb19_tx_utpbm_get,
    dcb19_tx_l3pbm_get,
    dcb19_tx_crc_get,
    dcb19_tx_cos_get,
    dcb19_tx_destmod_get,
    dcb19_tx_destport_get,
    dcb19_tx_opcode_get,
    dcb19_tx_srcmod_get,
    dcb19_tx_srcport_get,
    dcb19_tx_prio_get,
    dcb19_tx_pfm_get,

    dcb23_dump,
    dcb23_reason_dump,
#endif /* BROADCOM_DEBUG */
#ifdef  PLISIM
    dcb19_status_init,
    dcb19_xfercount_set,
    dcb19_rx_start_set,
    dcb19_rx_end_set,
    dcb19_rx_error_set,
    dcb19_rx_crc_set,
    dcb19_rx_ingport_set,
#endif /* PLISIM */
};
#endif /* BCM_KATANA_SUPPORT */

#if defined(BCM_KATANA_SUPPORT)
/*
 * DCB Type 24 Support
 */

static soc_rx_reason_t
dcb24_rx_reason_map[] = {
    socRxReasonUnknownVlan,        /* Offset 0 */
    socRxReasonL2SourceMiss,       /* Offset 1 */
    socRxReasonL2DestMiss,         /* Offset 2 */
    socRxReasonL2Move,             /* Offset 3 */
    socRxReasonL2Cpu,              /* Offset 4 */
    socRxReasonSampleSource,       /* Offset 5 */
    socRxReasonSampleDest,         /* Offset 6 */
    socRxReasonL3SourceMiss,       /* Offset 7 */
    socRxReasonL3DestMiss,         /* Offset 8 */
    socRxReasonL3SourceMove,       /* Offset 9 */
    socRxReasonMcastMiss,          /* Offset 10 */
    socRxReasonIpMcastMiss,        /* Offset 11 */
    socRxReasonFilterMatch,        /* Offset 12 */
    socRxReasonL3HeaderError,      /* Offset 13 */
    socRxReasonProtocol,           /* Offset 14 */
    socRxReasonDosAttack,          /* Offset 15 */
    socRxReasonMartianAddr,        /* Offset 16 */
    socRxReasonTunnelError,        /* Offset 17 */
    socRxReasonMirror,             /* Offset 18 */ 
    socRxReasonIcmpRedirect,       /* Offset 19 */
    socRxReasonL3Slowpath,         /* Offset 20 */
    socRxReasonParityError,        /* Offset 21 */
    socRxReasonL3MtuFail,          /* Offset 22 */
    socRxReasonHigigHdrError,      /* Offset 23 */
    socRxReasonMcastIdxError,      /* Offset 24 */
    socRxReasonVlanFilterMatch,    /* Offset 25 */
    socRxReasonClassBasedMove,     /* Offset 26 */
    socRxReasonL3AddrBindFail,     /* Offset 27 */
    socRxReasonMplsLabelMiss,      /* Offset 28 */
    socRxReasonMplsInvalidAction,  /* Offset 29 */
    socRxReasonMplsInvalidPayload, /* Offset 30 */
    socRxReasonMplsTtl,            /* Offset 31 */
    socRxReasonMplsSequenceNumber, /* Offset 32 */
    socRxReasonL2NonUnicastMiss,   /* Offset 33 */
    socRxReasonNhop,               /* Offset 34 */
    socRxReasonBfdSlowpath,        /* Offset 35 */
    socRxReasonStation,            /* Offset 36 */   
    socRxReasonNiv,                /* Offset 37 */
    socRxReasonNiv,                /* Offset 38 */
    socRxReasonNiv,                /* Offset 39 */
    socRxReasonVlanTranslate,      /* Offset 40 */
    socRxReasonTimeSync,           /* Offset 41 */ 
    socRxReasonOAMSlowpath,        /* Offset 42 */
    socRxReasonOAMError,           /* Offset 43 */
    socRxReasonOAMLMDM,            /* Offset 44 */ 
    socRxReasonL2LearnLimit,       /* Offset 45 */
    socRxReasonBfd,                /* Offset 46 */   
    socRxReasonInvalid,            /* Offset 47 */
    socRxReasonInvalid,            /* Offset 48 */
    socRxReasonInvalid,            /* Offset 49 */
    socRxReasonInvalid,            /* Offset 50 */
    socRxReasonInvalid,            /* Offset 51 */
    socRxReasonInvalid,            /* Offset 52 */
    socRxReasonInvalid,            /* Offset 53 */
    socRxReasonInvalid,            /* Offset 54 */
    socRxReasonInvalid,            /* Offset 55 */
    socRxReasonInvalid,            /* Offset 56 */
    socRxReasonInvalid,            /* Offset 57 */
    socRxReasonInvalid,            /* Offset 58 */
    socRxReasonInvalid,            /* Offset 59 */
    socRxReasonInvalid,            /* Offset 60 */
    socRxReasonInvalid,            /* Offset 61 */
    socRxReasonInvalid,            /* Offset 62 */
    socRxReasonInvalid             /* Offset 63 */
};

static int
dcb24_addrx(dv_t *dv, sal_vaddr_t addr, uint32 count, uint32 flags)
{
    dcb24_t     *d;     /* DCB */
    uint32      *di;    /* DCB integer pointer */

    d = (dcb24_t *)SOC_DCB_IDX2PTR(dv->dv_unit, dv->dv_dcb, dv->dv_vcnt);

    if (dv->dv_vcnt > 0) {      /* chain off previous dcb */
        d[-1].c_chain = 1;
    }

    di = (uint32 *)d;
    di[0] = di[1] = di[2] = di[3] = di[4] = 0;
    di[5] = di[6] = di[7] = di[8] = di[9] = di[10] = 0;
    di[11] = di[12] = di[13] = di[14] = di[15] = 0;

    if (addr) {
        d->addr = soc_cm_l2p(dv->dv_unit, (void *)addr);
    }
    d->c_count = count;
    d->c_sg = 1;

    dv->dv_vcnt += 1;
    return dv->dv_cnt - dv->dv_vcnt;
}

GETFUNCEXPR(24, rx_mirror, (d->reason & (1 << 18)))

#ifdef BROADCOM_DEBUG
static void
dcb24_dump(int unit, dcb_t *dcb, char *prefix, int tx)
{
    uint32      *p;
    int         i, size;
    dcb19_t     *dtx = (dcb19_t *)dcb;
    dcb24_t     *drx = (dcb24_t *)dcb;
    char        ps[((DCB_MAX_SIZE/sizeof(uint32))*9)+1];
    uint8       *h;
#if defined(LE_HOST)
    uint32      hgh[4];
    h = (uint8 *)&hgh[0];

    if (tx) {
        hgh[0] = soc_htonl(dtx->mh0);
        hgh[1] = soc_htonl(dtx->mh1);
        hgh[2] = soc_htonl(dtx->mh2);
        hgh[3] = soc_htonl(dtx->mh3);
    } else {
        hgh[0] = soc_htonl(drx->mh0);
        hgh[1] = soc_htonl(drx->mh1);
        hgh[2] = soc_htonl(drx->mh2);
        hgh[3] = soc_htonl(drx->mh3);
    }
#else
    if (tx) {
        h = (uint8 *)&dtx->mh0;
    } else {
        h = (uint8 *)&drx->mh0;
    }
#endif

    p = (uint32 *)dcb;
    size = SOC_DCB_SIZE(unit) / sizeof(uint32);
    for (i = 0; i < size; i++) {
        sal_sprintf(&ps[i*9], "%08x ", p[i]);
    }
    soc_cm_print("%s\t%s\n", prefix, ps);
    if ((SOC_DCB_HG_GET(unit, dcb)) || (SOC_DCB_RX_START_GET(unit, dcb))) {
        soc_dma_higig_dump(unit, prefix, h, 0, 0, NULL);
    }
    soc_cm_print("%s\ttype %d %schain %ssg %sreload %shg %sstat %spause "
                 " %spurge\n",
                 prefix,
                 SOC_DCB_TYPE(unit),
                 SOC_DCB_CHAIN_GET(unit, dcb) ? "" : "!",
                 SOC_DCB_SG_GET(unit, dcb) ? "" : "!",
                 SOC_DCB_RELOAD_GET(unit, dcb) ? "" : "!",
                 SOC_DCB_HG_GET(unit, dcb) ? "" : "!",
                 SOC_DCB_STAT_GET(unit, dcb) ? "" : "!",
                 dtx->c_pause ? "" : "!",
                 SOC_DCB_PURGE_GET(unit, dcb) ? "" : "!");
    soc_cm_print("%s\taddr %p reqcount %d xfercount %d\n",
                 prefix,
                 (void *)SOC_DCB_ADDR_GET(unit, dcb),
                 SOC_DCB_REQCOUNT_GET(unit, dcb),
                 SOC_DCB_XFERCOUNT_GET(unit, dcb));
    if (!tx) {
        soc_cm_print("%s\t%sdone %serror %sstart %send\n",
                     prefix,
                     SOC_DCB_DONE_GET(unit, dcb) ? "" : "!",
                     SOC_DCB_RX_ERROR_GET(unit, dcb) ? "" : "!",
                     SOC_DCB_RX_START_GET(unit, dcb) ? "" : "!",
                     SOC_DCB_RX_END_GET(unit, dcb) ? "" : "!");
    }

    if (tx || !SOC_DCB_RX_START_GET(unit, dcb)) {
        return;
    }
    
    dcb0_reason_dump(unit, dcb, prefix);
    soc_cm_print("%s\t%schg_tos %sregen_crc %schg_ecn %svfi_valid\n",
                 prefix,
                 drx->word11.overlay1.chg_tos ? "" : "!",
                 drx->regen_crc ? "" : "!",
                 drx->chg_ecn ? "" : "!",
                 drx->vfi_valid ? "" : "!");
    soc_cm_print("%s\t%sservice_tag %sswitch_pkt %shg_type %ssrc_hg\n",
                 prefix,
                 drx->service_tag ? "" : "!",
                 drx->switch_pkt ? "" : "!",
                 drx->hg_type ? "" : "!",
                 drx->src_hg ? "" : "!");
    soc_cm_print("%s\t%sl3routed %sl3only %sreplicated %sdo_not_change_ttl"
                 " %sbpdu\n",
                 prefix,
                 drx->l3routed ? "" : "!",
                 drx->l3only ? "" : "!",
                 drx->replicated ? "" : "!",
                 drx->do_not_change_ttl ? "" : "!",
                 drx->bpdu ? "" : "!");
    soc_cm_print("%s\t%shg2_ext_hdr\n",
                 prefix,
                 drx->hg2_ext_hdr ? "" : "!");
    if (drx->hg2_ext_hdr) {
        soc_cm_print("%s\tHigig2 Extension: queue_tag=%04x %stm tag_type=%d"
                     " seg_sel=%d\n",
                     prefix,
                     drx->word14.overlay2.eh_queue_tag,
                     (drx->word14.overlay2.eh_tm) ? "" : "!",
                     drx->word14.overlay2.eh_tag_type,
                     drx->word14.overlay2.eh_seg_sel);
    }
    soc_cm_print("%s\treason=%08x_%08x ts_type=%d"
                 " timestamp=%08x_%08x",
                 prefix,
                 drx->reason_hi,
                 drx->reason,
                 drx->timestamp_type,
                 drx->word14.overlay1.timestamp_hi,
                 drx->timestamp);
    soc_cm_print("%s\tsrcport=%d queue_num=%d hgi=%d lb_pkt_type=%d"
                 " repl_nhi=%05x\n",
                 prefix,
                 drx->srcport,
                 drx->word4.overlay1.queue_num,
                 drx->hgi,
                 drx->loopback_pkt_type,
                 drx->repl_nhi);
    soc_cm_print("%s\touter_vid=%d outer_cfi=%d outer_pri=%d otag_action=%d"
                 " vntag_action=%d\n",
                 prefix,
                 drx->word4.overlay1.outer_vid,
                 drx->word11.overlay1.outer_cfi,
                 drx->word11.overlay1.outer_pri,
                 drx->otag_action,
                 drx->word11.overlay1.vntag_action);
    soc_cm_print("%s\tinner_vid=%d inner_cfi=%d inner_pri=%d itag_action=%d"
                 " itag_status=%d\n",
                 prefix,
                 drx->word11.overlay1.inner_vid,
                 drx->word11.overlay1.inner_cfi,
                 drx->inner_pri,
                 drx->itag_action,
                 drx->itag_status);
    soc_cm_print("%s\tdscp=%d ecn=%d %sspecial_pkt %sall_switch_drop\n",
                 prefix,
                 drx->word11.overlay1.dscp,
                 drx->ecn,
                 drx->oam_pkt ? "" : "!",
                 drx->all_switch_drop ? "" : "!");
    soc_cm_print("%s\tdecap_tunnel_type=%d vfi=%d match_rule=%d"
                 " mtp_ind=%d\n",
                 prefix,
                 drx->word11.overlay1.decap_tunnel_type,
                 drx->timestamp & 0x3fff,
                 drx->match_rule,
                 drx->mtp_index);
}
#endif /* BROADCOM_DEBUG */

dcb_op_t dcb24_op = {
    24,
    sizeof(dcb24_t),
    dcb24_rx_reason_map,
    NULL,
    dcb19_init,
    dcb19_addtx,
    dcb24_addrx,
    dcb19_intrinfo,
    dcb19_reqcount_set,
    dcb19_reqcount_get,
    dcb23_xfercount_get,
    dcb0_addr_set,
    dcb0_addr_get,
    dcb0_paddr_get,
    dcb19_done_set,
    dcb19_done_get,
    dcb19_sg_set,
    dcb19_sg_get,
    dcb19_chain_set,
    dcb19_chain_get,
    dcb19_reload_set,
    dcb19_reload_get,
    dcb19_tx_l2pbm_set,
    dcb19_tx_utpbm_set,
    dcb19_tx_l3pbm_set,
    dcb19_tx_crc_set,
    dcb19_tx_cos_set,
    dcb19_tx_destmod_set,
    dcb19_tx_destport_set,
    dcb19_tx_opcode_set,
    dcb19_tx_srcmod_set,
    dcb19_tx_srcport_set,
    dcb19_tx_prio_set,
    dcb19_tx_pfm_set,
    dcb23_rx_untagged_get,
    dcb19_rx_crc_get,
    dcb23_rx_cos_get,
    dcb23_rx_destmod_get,
    dcb23_rx_destport_get,
    dcb23_rx_opcode_get,
    dcb23_rx_classtag_get,
    dcb23_rx_matchrule_get,
    dcb19_rx_start_get,
    dcb19_rx_end_get,
    dcb19_rx_error_get,
    dcb23_rx_prio_get,
    dcb23_rx_reason_get,
    dcb23_rx_reason_hi_get,
    dcb23_rx_ingport_get,
    dcb23_rx_srcport_get,
    dcb23_rx_srcmod_get,
    dcb23_rx_mcast_get,
    dcb23_rx_vclabel_get,
    dcb24_rx_mirror_get,
    dcb23_rx_timestamp_get,
    dcb23_rx_timestamp_upper_get,
    dcb19_hg_set,
    dcb19_hg_get,
    dcb19_stat_set,
    dcb19_stat_get,
    dcb19_purge_set,
    dcb19_purge_get,
    dcb23_mhp_get,
    dcb23_outer_vid_get,
    dcb23_outer_pri_get,
    dcb23_outer_cfi_get,
    dcb23_inner_vid_get,
    dcb23_inner_pri_get,
    dcb23_inner_cfi_get,
    dcb23_rx_bpdu_get,
    dcb9_rx_egr_to_cpu_get,
#ifdef  BROADCOM_DEBUG
    dcb19_tx_l2pbm_get,
    dcb19_tx_utpbm_get,
    dcb19_tx_l3pbm_get,
    dcb19_tx_crc_get,
    dcb19_tx_cos_get,
    dcb19_tx_destmod_get,
    dcb19_tx_destport_get,
    dcb19_tx_opcode_get,
    dcb19_tx_srcmod_get,
    dcb19_tx_srcport_get,
    dcb19_tx_prio_get,
    dcb19_tx_pfm_get,

    dcb24_dump,
    dcb0_reason_dump,
#endif /* BROADCOM_DEBUG */
#ifdef  PLISIM
    dcb19_status_init,
    dcb19_xfercount_set,
    dcb19_rx_start_set,
    dcb19_rx_end_set,
    dcb19_rx_error_set,
    dcb19_rx_crc_set,
    dcb19_rx_ingport_set,
#endif /* PLISIM */
};
#endif /* BCM_KATANA_SUPPORT */


#if defined(BCM_SIRIUS_SUPPORT)
/*
 * DCB Type 27 Support
 */
static soc_rx_reason_t
dcb27_rx_reason_map[] = {
    socRxReasonInvalid,            /* Offset 0 */
    socRxReasonInvalid,            /* Offset 1 */
    socRxReasonInvalid,            /* Offset 2 */
    socRxReasonInvalid,            /* Offset 3 */
    socRxReasonInvalid,            /* Offset 4 */
    socRxReasonInvalid,            /* Offset 5 */
    socRxReasonInvalid,            /* Offset 6 */
    socRxReasonInvalid,            /* Offset 7 */
    socRxReasonInvalid,            /* Offset 8 */
    socRxReasonInvalid,            /* Offset 9 */
    socRxReasonInvalid,            /* Offset 10 */
    socRxReasonInvalid,            /* Offset 11 */
    socRxReasonInvalid,            /* Offset 12 */
    socRxReasonInvalid,            /* Offset 13 */
    socRxReasonInvalid,            /* Offset 14 */
    socRxReasonInvalid,            /* Offset 15 */
    socRxReasonInvalid,            /* Offset 16 */
    socRxReasonInvalid,            /* Offset 17 */
    socRxReasonInvalid,            /* Offset 18 */
    socRxReasonInvalid,            /* Offset 19 */
    socRxReasonInvalid,            /* Offset 20 */
    socRxReasonInvalid,            /* Offset 21 */
    socRxReasonInvalid,            /* Offset 22 */
    socRxReasonInvalid,            /* Offset 23 */
    socRxReasonInvalid,            /* Offset 24 */
    socRxReasonInvalid,            /* Offset 25 */
    socRxReasonInvalid,            /* Offset 26 */
    socRxReasonInvalid,            /* Offset 27 */
    socRxReasonInvalid,            /* Offset 28 */
    socRxReasonInvalid,            /* Offset 29 */
    socRxReasonInvalid,            /* Offset 30 */
    socRxReasonInvalid,            /* Offset 31 */
    socRxReasonInvalid,            /* Offset 32 */
    socRxReasonInvalid,            /* Offset 33 */
    socRxReasonInvalid,            /* Offset 34 */
    socRxReasonInvalid,            /* Offset 35 */
    socRxReasonInvalid,            /* Offset 36 */
    socRxReasonInvalid,            /* Offset 37 */
    socRxReasonInvalid,            /* Offset 38 */
    socRxReasonInvalid,            /* Offset 39 */
    socRxReasonInvalid,            /* Offset 40 */
    socRxReasonInvalid,            /* Offset 41 */
    socRxReasonInvalid,            /* Offset 42 */
    socRxReasonInvalid,            /* Offset 43 */
    socRxReasonInvalid,            /* Offset 44 */
    socRxReasonInvalid,            /* Offset 45 */
    socRxReasonInvalid,            /* Offset 46 */
    socRxReasonInvalid,            /* Offset 47 */
    socRxReasonInvalid,            /* Offset 48 */
    socRxReasonInvalid,            /* Offset 49 */
    socRxReasonInvalid,            /* Offset 50 */
    socRxReasonInvalid,            /* Offset 51 */
    socRxReasonInvalid,            /* Offset 52 */
    socRxReasonInvalid,            /* Offset 53 */
    socRxReasonInvalid,            /* Offset 54 */
    socRxReasonInvalid,            /* Offset 55 */
    socRxReasonInvalid,            /* Offset 56 */
    socRxReasonInvalid,            /* Offset 57 */
    socRxReasonInvalid,            /* Offset 58 */
    socRxReasonInvalid,            /* Offset 59 */
    socRxReasonInvalid,            /* Offset 60 */
    socRxReasonInvalid,            /* Offset 61 */
    socRxReasonInvalid,            /* Offset 62 */
    socRxReasonInvalid             /* Offset 63 */
};

GETFUNCERR(27, rx_cos)
GETFUNCERR(27, rx_prio)
GETFUNCERR(27, rx_mcast)
GETFUNCERR(27, rx_vclabel)
GETFUNCERR(27, rx_classtag)
GETFUNCUNITERR(27, rx_untagged)
GETFUNCERR(27, rx_matchrule)
GETFUNCERR(27, rx_reason)
GETFUNCERR(27, rx_reason_hi)
GETFUNCERR(27, rx_ingport)
GETFUNCERR(27, rx_mirror)
GETFUNCERR(27, rx_timestamp)
GETFUNCERR(27, rx_timestamp_upper)
GETFUNCERR(27, outer_vid)
GETFUNCERR(27, outer_pri)
GETFUNCERR(27, outer_cfi)
GETFUNCERR(27, inner_vid)
GETFUNCERR(27, inner_pri)
GETFUNCERR(27, inner_cfi)
GETFUNCERR(27, rx_bpdu)
GETFUNCERR(27, rx_egr_to_cpu)

#ifdef BROADCOM_DEBUG
static void
dcb27_dump(int unit, dcb_t *dcb, char *prefix, int tx)
{
    uint32      *p;
    int         i, size;
    dcb27_t *d = (dcb27_t *)dcb;
    char        ps[((DCB_MAX_SIZE/sizeof(uint32))*9)+1];
#if defined(LE_HOST)
    uint32  hgh[4];
    uint8 *h = (uint8 *)&hgh[0];

    hgh[0] = soc_htonl(d->mh0);
    hgh[1] = soc_htonl(d->mh1);
    hgh[2] = soc_htonl(d->mh2);
    hgh[3] = soc_htonl(d->mh3);
#else
    uint8 *h = (uint8 *)&d->mh0;
#endif

    p = (uint32 *)dcb;
    size = SOC_DCB_SIZE(unit) / sizeof(uint32);
    for (i = 0; i < size; i++) {
        sal_sprintf(&ps[i*9], "%08x ", p[i]);
    }
    soc_cm_print("%s\t%s\n", prefix, ps);
    if ((SOC_DCB_HG_GET(unit, dcb)) || (SOC_DCB_RX_START_GET(unit, dcb))) {
        soc_dma_higig_dump(unit, prefix, h, 0, 0, NULL);
    }
    soc_cm_print(
        "%s\ttype %d %ssg %schain %sreload %shg %sstat %spause %spurge\n",
        prefix,
        SOC_DCB_TYPE(unit),
        SOC_DCB_SG_GET(unit, dcb) ? "" : "!",
        SOC_DCB_CHAIN_GET(unit, dcb) ? "" : "!",
        SOC_DCB_RELOAD_GET(unit, dcb) ? "" : "!",
        SOC_DCB_HG_GET(unit, dcb) ? "" : "!",
        SOC_DCB_STAT_GET(unit, dcb) ? "" : "!",
        d->c_pause ? "" : "!",
        SOC_DCB_PURGE_GET(unit, dcb) ? "" : "!");
    soc_cm_print(
        "%s\taddr %p reqcount %d xfercount %d\n",
        prefix,
        (void *)SOC_DCB_ADDR_GET(unit, dcb),
        SOC_DCB_REQCOUNT_GET(unit, dcb),
        SOC_DCB_XFERCOUNT_GET(unit, dcb));
    if (!tx) {
        soc_cm_print(
            "%s\t%sdone %sstart %send %serror\n",
            prefix,
            SOC_DCB_DONE_GET(unit, dcb) ? "" : "!",
            SOC_DCB_RX_START_GET(unit, dcb) ? "" : "!",
            SOC_DCB_RX_END_GET(unit, dcb) ? "" : "!",
            SOC_DCB_RX_ERROR_GET(unit, dcb) ? "" : "!");
    }
}
#endif /* BROADCOM_DEBUG */

dcb_op_t dcb27_op = {
    27,
    sizeof(dcb27_t),
    dcb27_rx_reason_map,
    NULL,
    dcb19_init,
    dcb19_addtx,
    dcb19_addrx,
    dcb19_intrinfo,
    dcb19_reqcount_set,
    dcb19_reqcount_get,
    dcb19_xfercount_get,
    dcb0_addr_set,
    dcb0_addr_get,
    dcb0_paddr_get,
    dcb19_done_set,
    dcb19_done_get,
    dcb19_sg_set,
    dcb19_sg_get,
    dcb19_chain_set,
    dcb19_chain_get,
    dcb19_reload_set,
    dcb19_reload_get,
    dcb19_tx_l2pbm_set,
    dcb19_tx_utpbm_set,
    dcb19_tx_l3pbm_set,
    dcb19_tx_crc_set,
    dcb19_tx_cos_set,
    dcb19_tx_destmod_set,
    dcb19_tx_destport_set,
    dcb19_tx_opcode_set,
    dcb19_tx_srcmod_set,
    dcb19_tx_srcport_set,
    dcb19_tx_prio_set,
    dcb19_tx_pfm_set,
    dcb27_rx_untagged_get,
    dcb19_rx_crc_get,
    dcb27_rx_cos_get,
    dcb19_rx_destmod_get,
    dcb19_rx_destport_get,
    dcb19_rx_opcode_get,
    dcb27_rx_classtag_get,
    dcb27_rx_matchrule_get,
    dcb19_rx_start_get,
    dcb19_rx_end_get,
    dcb19_rx_error_get,
    dcb27_rx_prio_get,
    dcb27_rx_reason_get,
    dcb27_rx_reason_hi_get,
    dcb27_rx_ingport_get,
    dcb19_rx_srcport_get,
    dcb19_rx_srcmod_get,
    dcb27_rx_mcast_get,
    dcb27_rx_vclabel_get,
    dcb27_rx_mirror_get,
    dcb27_rx_timestamp_get,
    dcb27_rx_timestamp_upper_get,
    dcb19_hg_set,
    dcb19_hg_get,
    dcb19_stat_set,
    dcb19_stat_get,
    dcb19_purge_set,
    dcb19_purge_get,
    dcb19_mhp_get,
    dcb27_outer_vid_get,
    dcb27_outer_pri_get,
    dcb27_outer_cfi_get,
    dcb27_inner_vid_get,
    dcb27_inner_pri_get,
    dcb27_inner_cfi_get,
    dcb27_rx_bpdu_get,
    dcb27_rx_egr_to_cpu_get,
#ifdef  BROADCOM_DEBUG
    dcb19_tx_l2pbm_get,
    dcb19_tx_utpbm_get,
    dcb19_tx_l3pbm_get,
    dcb19_tx_crc_get,
    dcb19_tx_cos_get,
    dcb19_tx_destmod_get,
    dcb19_tx_destport_get,
    dcb19_tx_opcode_get,
    dcb19_tx_srcmod_get,
    dcb19_tx_srcport_get,
    dcb19_tx_prio_get,
    dcb19_tx_pfm_get,

    dcb27_dump,
    dcb0_reason_dump,
#endif /* BROADCOM_DEBUG */
#ifdef  PLISIM
    dcb19_status_init,
    dcb19_xfercount_set,
    dcb19_rx_start_set,
    dcb19_rx_end_set,
    dcb19_rx_error_set,
    dcb19_rx_crc_set,
    dcb19_rx_ingport_set,
#endif /* PLISIM */
};
#endif /* BCM_SIRIUS_SUPPORT */

#endif  /* BCM_XGS3_SWITCH_SUPPORT || BCM_SIRIUS_SUPPORT */


/*
 * Function:
 *      soc_dcb_unit_init
 * Purpose:
 *      Select the appropriate dcb operations set and load it into
 *      SOC_CONTROL
 * Parameters:
 *      unit - device
 */
void
soc_dcb_unit_init(int unit)
{
    soc_control_t       *soc;

    COMPILER_REFERENCE(dcb0_funcerr);
    soc = SOC_CONTROL(unit);

#ifdef  BCM_STRATA1_SUPPORT
    if (soc_feature(unit, soc_feature_dcb_type1)) {
        soc->dcb_op = dcb1_op;
        return;
    }
#endif  /* BCM_STRATA1_SUPPORT */

#ifdef  BCM_DRACO1_SUPPORT
    if (soc_feature(unit, soc_feature_dcb_type3)) {
        soc->dcb_op = dcb3_op;
        return;
    }
#endif  /* BCM_DRACO1_SUPPORT */

#ifdef  BCM_HERCULES_SUPPORT
    if (soc_feature(unit, soc_feature_dcb_type4)) {
        soc->dcb_op = dcb4_op;
        return;
    }
#endif  /* BCM_HERCULES_SUPPORT */

#ifdef  BCM_LYNX_SUPPORT
    if (soc_feature(unit, soc_feature_dcb_type5)) {
        soc->dcb_op = dcb5_op;
        return;
    }
#endif  /* BCM_LYNX_SUPPORT */

#ifdef  BCM_TUCANA_SUPPORT
    if (soc_feature(unit, soc_feature_dcb_type6)) {
        soc->dcb_op = dcb6_op;
        return;
    }
#endif  /* BCM_TUCANA_SUPPORT */

#ifdef  BCM_DRACO15_SUPPORT
    if (soc_feature(unit, soc_feature_dcb_type7)) {
        soc->dcb_op = dcb7_op;
        return;
    }
#endif  /* BCM_DRACO15_SUPPORT */

#ifdef  BCM_XGS3_SWITCH_SUPPORT
#ifdef BCM_FIREBOLT_SUPPORT
    if (soc_feature(unit, soc_feature_dcb_type9)) {
        soc->dcb_op = dcb9_op;
        return;
    }
#endif
#ifdef BCM_EASYRIDER_SUPPORT
    if (soc_feature(unit, soc_feature_dcb_type10)) {
        soc->dcb_op = dcb10_op;
        return;
    }
#endif
#ifdef BCM_BRADLEY_SUPPORT
    if (soc_feature(unit, soc_feature_dcb_type11)) {
        soc->dcb_op = dcb11_op;
        return;
    }
#endif
#if defined(BCM_RAPTOR_SUPPORT)
    if (soc_feature(unit, soc_feature_dcb_type12)) {
        soc->dcb_op = dcb12_op;
        return;
    }
#endif
#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_FIREBOLT_SUPPORT)
    if (soc_feature(unit, soc_feature_dcb_type13)) {
        soc->dcb_op = dcb13_op;
        return;
    }
#endif
#if defined(BCM_TRIUMPH_SUPPORT)
    if (soc_feature(unit, soc_feature_dcb_type14)) {
        soc->dcb_op = dcb14_op;
        return;
    }
#endif
#if defined(BCM_RAPTOR_SUPPORT)
    if (soc_feature(unit, soc_feature_dcb_type15)) {
        soc->dcb_op = dcb15_op;
        return;
    }
#endif
#if defined(BCM_SCORPION_SUPPORT)
    if (soc_feature(unit, soc_feature_dcb_type16)) {
        soc->dcb_op = dcb16_op;
        return;
    }
#endif
#if defined(BCM_HAWKEYE_SUPPORT)
    if (soc_feature(unit, soc_feature_dcb_type17)) {
        soc->dcb_op = dcb17_op;
        return;
    }
#endif
#if defined(BCM_RAPTOR_SUPPORT)
    if (soc_feature(unit, soc_feature_dcb_type18)) {
        soc->dcb_op = dcb18_op;
        return;
    }
#endif
#if defined(BCM_TRIUMPH2_SUPPORT)
    if (soc_feature(unit, soc_feature_dcb_type19)) {
        soc->dcb_op = dcb19_op;
        return;
    }
#endif
#ifdef BCM_ENDURO_SUPPORT
    if (soc_feature(unit, soc_feature_dcb_type20)) {
        soc->dcb_op = dcb20_op;
        return;
    }
#endif
#if defined(BCM_TRIDENT_SUPPORT)
    if (soc_feature(unit, soc_feature_dcb_type21)) {
        soc->dcb_op = dcb21_op;
        return;
    }
#endif
#ifdef BCM_SHADOW_SUPPORT
    if (soc_feature(unit, soc_feature_dcb_type22)) {
        soc->dcb_op = dcb22_op;
        return;
    }
#endif
#if defined(BCM_KATANA_SUPPORT)
    if (soc_feature(unit, soc_feature_dcb_type24)) {
        soc->dcb_op = dcb24_op;
        return;
    }
#endif
#endif  /* BCM_XGS3_SWITCH_SUPPORT */
#if defined(BCM_SIRIUS_SUPPORT)
    if (soc_feature(unit, soc_feature_dcb_type27)) {
	/* defined but not used for now, pending more testing */
        soc->dcb_op = dcb27_op;
        return;
    } else if (soc_feature(unit, soc_feature_dcb_type19)) {
        soc->dcb_op = dcb19_op;
        return;
    }
#endif
    soc_cm_debug(DK_ERR, "unit %d has unknown dcb type\n", unit);
    assert(0);
}

#endif /* defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) */
