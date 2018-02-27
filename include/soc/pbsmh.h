/*
 * $Id: pbsmh.h 1.25.6.3 Broadcom SDK $
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
 * File:        pbsmh.h
 */

#ifndef CMICM_EMBEDDED
#include <soc/defs.h>
#endif

#ifndef _SOC_PBSMH_H
#define _SOC_PBSMH_H

#ifdef BCM_XGS3_SWITCH_SUPPORT

/****************************************************************************
 * PBS MH Header Format
 *
 * On XGS3 devices this is used to send a Packet Byte Stream out a port.
 *
 * ANSI only permits signed or unsigned int for bit field type.  This
 * structure will only work for compilers for which uint32 is unsigned
 * int, and which completely pack bit fields beginning at the MSbit for
 * big-endian machines and at the LSbit for little-endian machines.
 *
 * NOTE: these structures already put the the individual bytes in memory
 * in big endian order for both big- and little-endian machines, so no
 * further swapping is required.
 ***************************************************************************/



typedef struct soc_pbsmh_hdr_s {
#if defined(LE_HOST)
            				/* Byte # */
	uint32	start:8;		/* 0 */
	uint32	_rsvd0:24;		/* 1-3 */
	uint32	_rsvd1;		        /* 4-7 */
	uint32	_rsvd2:16;		/* 8-9 */
	uint32	src_mod:6;              /* 10 */
	uint32	_rsvd3:2;		/* 10 */
	uint32	dst_port:5;             /* 11 */
	uint32	cos:3;                  /* 11 */
#else /* !LE_HOST */
            				/* Byte # */
	uint32	start:8;		/* 0 */
	uint32	_rsvd0:24;		/* 1-3 */
	uint32	_rsvd1;		        /* 4-7 */
	uint32	_rsvd2:16;		/* 8-10 */
	uint32	_rsvd3:2;		/* 10 */
	uint32	src_mod:6;              /* 10 */
	uint32	cos:3;                  /* 11 */
	uint32	dst_port:5;             /* 11 */
#endif /* !LE_HOST */
} soc_pbsmh_hdr_t;

typedef struct soc_pbsmh_v2_hdr_s {
#if defined(LE_HOST)
            				/* Byte # */
	uint32	start:8;		/* 0 */
	uint32	_rsvd0:24;		/* 1-3 */
	uint32	_rsvd1;		        /* 4-7 */
	uint32	_rsvd2:8;		/* 8  */
	uint32	src_mod:8;              /* 9  */
	uint32	cos:4;                  /* 10 */
	uint32	pri:4;                  /* 10 */
	uint32	dst_port:7;             /* 11 */
	uint32	l3pbm_sel:1;            /* 11 */
#else /* !LE_HOST */
            				/* Byte # */
	uint32	start:8;		/* 0 */
	uint32	_rsvd0:24;		/* 1-3 */
	uint32	_rsvd1;		        /* 4-7 */
	uint32	_rsvd2:8;		/* 8  */
	uint32	src_mod:8;              /* 9  */
	uint32	pri:4;                  /* 10 */
	uint32	cos:4;                  /* 10 */
	uint32	l3pbm_sel:1;            /* 11 */
	uint32	dst_port:7;             /* 11 */
#endif /* !LE_HOST */
} soc_pbsmh_v2_hdr_t;

typedef struct soc_pbsmh_v3_hdr_s {
#if defined(LE_HOST)
                                        /* Byte # */
        uint32  start:8;                /* 0 */
        uint32  _rsvd0:24;              /* 1-3 */
        uint32  _rsvd1;                 /* 4-7 */
        uint32  src_mod_hi:4;           /* 8  */ 
        uint32  _rsvd2:4;               /* 8  */
        uint32  pri:4;                  /* 9  */
        uint32  src_mod_lo:4;           /* 9  */
        uint32  cos:6;                  /* 10 */
        uint32  _rsvd3:2;               /* 10 */
        uint32  dst_port:7;             /* 11 */
        uint32  l3pbm_sel:1;            /* 11 */
#else /* !LE_HOST */
                                        /* Byte # */
        uint32  start:8;                /* 0 */
        uint32  _rsvd0:24;              /* 1-3 */
        uint32  _rsvd1;                 /* 4-7 */
        uint32  _rsvd2:4;               /* 8  */
        uint32  src_mod_hi:4;           /* 8  */
        uint32  src_mod_lo:4;           /* 9  */
        uint32  pri:4;                  /* 9  */
        uint32  _rsvd3:2;               /* 10 */
        uint32  cos:6;                  /* 10 */
        uint32  l3pbm_sel:1;            /* 11 */
        uint32  dst_port:7;             /* 11 */
#endif /* !LE_HOST */
} soc_pbsmh_v3_hdr_t;

typedef struct soc_pbsmh_v4_hdr_s {
#if defined(LE_HOST)
            				/* Byte # */
	uint32	start:8;		/* 0 */
	uint32	_rsvd0:24;		/* 1-3 */
	uint32	_rsvd1;		        /* 4-7 */
	uint32	src_mod_hi:1;           /* 8  */
        uint32  _rsvd2:7;               /* 8  */
	uint32	pri_hi:1;               /* 9 */
	uint32	src_mod_lo:7;           /* 9  */
	uint32	cos:5;                  /* 10 */
	uint32	pri_lo:3;               /* 10 */
	uint32	dst_port:7;             /* 11 */
	uint32	l3pbm_sel:1;            /* 11 */
#else /* !LE_HOST */
            				/* Byte # */
	uint32	start:8;		/* 0 */
	uint32	_rsvd0:24;		/* 1-3 */
	uint32	_rsvd1;		        /* 4-7 */
        uint32  _rsvd2:7;               /* 8  */
	uint32	src_mod_hi:1;           /* 8  */
	uint32	src_mod_lo:7;           /* 9  */
	uint32	pri_hi:1;               /* 9 */
	uint32	pri_lo:3;               /* 10 */
	uint32	cos:5;                  /* 10 */
	uint32	l3pbm_sel:1;            /* 11 */
	uint32	dst_port:7;             /* 11 */
#endif /* !LE_HOST */
} soc_pbsmh_v4_hdr_t;

typedef struct soc_pbsmh_v5_hdr_s {
#if defined(LE_HOST)
                                        /* Byte # */
        uint32  start:8;                /* 0 */
        uint32  _rsvd0:24;              /* 1-3 */
        uint32  ipcf_ptr:8;             /* 4 */
        uint32  spid_override:1;        /* 5 */
        uint32  _rsvd2:3;               /* 5 */
        uint32  tx_ts:1;                /* 5 */
        uint32  _rsvd1:3;               /* 5 */
        uint32  unicast:1;              /* 6 */
        uint32  set_l2bm:1;             /* 6 */
        uint32  set_l3bm:1;             /* 6 */
        uint32  _rsvd3:1;               /* 6 */
        uint32  spap:2;                 /* 6 */
        uint32  spid:2;                 /* 6 */
        uint32  src_mod:8;              /* 7 */
        uint32  input_pri:4;            /* 8 */
        uint32  _rsvd4:4;               /* 8 */
        uint32  queue_num:7;            /* 9 */
        uint32  _rsvd5:1;               /* 9 */
        uint32  cos:4;                  /* 10 */
        uint32  _rsvd6:4;               /* 10 */
        uint32  dst_port:7;             /* 11 */
        uint32  _rsvd7:1;               /* 11 */
#else /* !LE_HOST */
                                        /* Byte # */
        uint32  start:8;                /* 0 */
        uint32  _rsvd0:24;              /* 1-3 */
        uint32  ipcf_ptr:8;             /* 4 */
        uint32  _rsvd1:3;               /* 5 */
        uint32  tx_ts:1;                /* 5 */
        uint32  _rsvd2:3;               /* 5 */
        uint32  spid_override:1;        /* 5 */
        uint32  spid:2;                 /* 6 */
        uint32  spap:2;                 /* 6 */
        uint32  _rsvd3:1;               /* 6 */
        uint32  set_l3bm:1;             /* 6 */
        uint32  set_l2bm:1;             /* 6 */
        uint32  unicast:1;              /* 6 */
        uint32  src_mod:8;              /* 7 */
        uint32  _rsvd4:4;               /* 8 */
        uint32  input_pri:4;            /* 8 */
        uint32  _rsvd5:1;               /* 9 */
        uint32  queue_num:7;            /* 9 */
        uint32  _rsvd6:4;               /* 10 */
        uint32  cos:4;                  /* 10 */
        uint32  _rsvd7:1;               /* 11 */
        uint32  dst_port:7;             /* 11 */
#endif /* !LE_HOST */
} soc_pbsmh_v5_hdr_t;

typedef struct soc_pbsmh_v6_hdr_s {
#if defined(LE_HOST)
 
                                        /* Byte # */
        uint32  start:8;                /* 0 */
        uint32  _rsvd0:13;              /* 1-2 */
        uint32  hdr_offset:8;           /* 2-3 */
        uint32  osts:1;                 /* 3 */
        uint32  regen_upd_chksum:1;     /* 3 */
        uint32  its_sign:1;             /* 3 */
        uint32  ipcf_ptr:8;             /* 4 */
        uint32  tx_ts:1;                /* 5 */
        uint32  _rsvd1:3;               /* 5 */
        uint32  spid_override:1;        /* 5 */
        uint32  _rsvd2:3;               /* 5 */
        uint32  spap:2;                 /* 6 */
        uint32  spid:2;                 /* 6 */
        uint32  unicast:1;              /* 6 */
        uint32  set_l2bm:1;             /* 6 */
        uint32  set_l3bm:1;             /* 6 */
        uint32  _rsvd3:1;               /* 6 */
        uint32  src_mod:8;              /* 7 */
        uint32  input_pri:4;            /* 8 */
        uint32  _rsvd4:2;               /* 8 */
        uint32  queue_num:12;           /* 8-10 */
        uint32  cos:6;                  /* 10 */
        uint32  dst_port:7;             /* 11 */
        uint32  _rsvd7:1;               /* 11 */
#else /* !LE_HOST */
                                        /* Byte # */
        uint32  start:8;                /* 0 */
        uint32  _rsvd0:13;              /* 1-2 */
        uint32  hdr_offset:8;           /* 2-3 */
        uint32  its_sign:1;             /* 3 */
        uint32  regen_upd_chksum:1;     /* 3 */
        uint32  osts:1;                 /* 3 */
        uint32  ipcf_ptr:8;             /* 4 */
        uint32  _rsvd1:3;               /* 5 */
        uint32  tx_ts:1;                /* 5 */
        uint32  _rsvd2:3;               /* 5 */
        uint32  spid_override:1;        /* 5 */
        uint32  spid:2;                 /* 6 */
        uint32  spap:2;                 /* 6 */
        uint32  _rsvd3:1;               /* 6 */
        uint32  set_l3bm:1;             /* 6 */
        uint32  set_l2bm:1;             /* 6 */
        uint32  unicast:1;              /* 6 */
        uint32  src_mod:8;              /* 7 */
        uint32  _rsvd4:2;               /* 8 */
        uint32  input_pri:4;            /* 8 */
        uint32  queue_num:12;           /* 8-10 */
        uint32  cos:6;                  /* 10 */
        uint32  _rsvd5:1;               /* 11 */
        uint32  dst_port:7;             /* 11 */
#endif /* !LE_HOST */
} soc_pbsmh_v6_hdr_t;

typedef union soc_pbsmh_v7_hdr_s {
#if defined(LE_HOST)
    struct {                            /* Byte # */
        uint32  header_type:6;          /* 0 */
        uint32  start:2;                /* 0 */
        uint32  ep_cpu_reason_code_3:4;   /* 1 ' */
        uint32  _rsvd5:4;                 /* 1 ' */
        uint32  ep_cpu_reason_code_2:8;   /* 2 ' */
        uint32  ep_cpu_reason_code_1:8;   /* 3 ' */
        uint32  oam_replacement_offset:6;   /* 4 */
        uint32  _rsvd2:2;                   /* 4 */
        uint32  set_l3bm:1;             /* 5 */
        uint32  tx_ts:1;                /* 5 */
        uint32  its_sign:1;             /* 5 */
        uint32  regen_udp_checksum:1;   /* 5 */
        uint32  osts:1;                 /* 5 */
        uint32  oam_replacement_type:2; /* 5 */
        uint32  _rsvd1:1;               /* 5 */
        uint32  hdr_offset:8;           /* 6 */
        uint32  dst_port:7;             /* 7 */
        uint32  set_l2bm:1;             /* 7 */
        uint32  int_pri:4;              /* 8 ' */
        uint32  input_pri:4;            /* 8 */
        uint32  queue_num_2:2;          /* 9 */
        uint32  nlf_port:6;             /* 9 ' */
        uint32  queue_num_1:8;          /* 10 */
        uint32  src_mod:8;              /* 11 */
    } overlay1;

    struct {                            /* Byte # */
        uint32  header_type:6;          /* 0 */
        uint32  start:2;                /* 0 */
        uint32  _rsvd3:8;               /* 1 ' */
        uint32  lm_counter_index_2:8;   /* 2 ' */
        uint32  lm_counter_index_1:8;   /* 3 ' */
        uint32  oam_replacement_offset:6;   /* 4 */
        uint32  _rsvd2:2;                   /* 4 */
        uint32  set_l3bm:1;             /* 5 */
        uint32  tx_ts:1;                /* 5 */
        uint32  its_sign:1;             /* 5 */
        uint32  regen_udp_checksum:1;   /* 5 */
        uint32  osts:1;                 /* 5 */
        uint32  oam_replacement_type:2; /* 5 */
        uint32  _rsvd1:1;               /* 5 */
        uint32  hdr_offset:8;           /* 6 */
        uint32  dst_port:7;             /* 7 */
        uint32  set_l2bm:1;             /* 7 */
        uint32  cos:4;                  /* 8 ' */
        uint32  input_pri:4;            /* 8 */
        uint32  queue_num_2:2;          /* 9 */
        uint32  unicast:1;              /* 9 ' */
        uint32  spap:2;                 /* 9 ' */
        uint32  spid:2;                 /* 9 ' */
        uint32  spid_override:1;        /* 9'  */
        uint32  queue_num_1:8;          /* 10 */
        uint32  src_mod:8;              /* 11 */
    } overlay2;
#else /* !LE_HOST */

    struct {                            /* Byte # */
        uint32  start:2;                /* 0 */
        uint32  header_type:6;          /* 0 */
        uint32  _rsvd5:4;               /* 1 ' */
        uint32  ep_cpu_reason_code_3:4; /* 1 ' */
        uint32  ep_cpu_reason_code_2:8; /* 2 ' */
        uint32  ep_cpu_reason_code_1:8; /* 3 ' */
        uint32  _rsvd2:2;                   /* 4 */
        uint32  oam_replacement_offset:6;   /* 4 */
        uint32  _rsvd1:1;               /* 5 */
        uint32  oam_replacement_type:2; /* 5 */
        uint32  osts:1;                 /* 5 */
        uint32  regen_udp_checksum:1;   /* 5 */
        uint32  its_sign:1;             /* 5 */
        uint32  tx_ts:1;                /* 5 */
        uint32  set_l3bm:1;             /* 5 */
        uint32  hdr_offset:8;           /* 6 */
        uint32  set_l2bm:1;             /* 7 */
        uint32  dst_port:7;             /* 7 */
        uint32  input_pri:4;            /* 8 */
        uint32  int_pri:4;              /* 8 ' */
        uint32  nlf_port:6;             /* 9 ' */
        uint32  queue_num_2:2;          /* 9 */
        uint32  queue_num_1:8;          /* 10 */
        uint32  src_mod:8;              /* 11 */
    } overlay1;

    struct {                            /* Byte # */
        uint32  start:2;                /* 0 */
        uint32  header_type:6;          /* 0 */
        uint32  _rsvd3:8;               /* 1 ' */
        uint32  lm_counter_index_2:8;   /* 2 ' */
        uint32  lm_counter_index_1:8;   /* 3 ' */
        uint32  _rsvd2:2;                   /* 4 */
        uint32  oam_replacement_offset:6;   /* 4 */
        uint32  _rsvd1:1;                   /* 5 */
        uint32  oam_replacement_type:2;     /* 5 */
        uint32  osts:1;                 /* 5 */
        uint32  regen_udp_checksum:1;   /* 5 */
        uint32  its_sign:1;             /* 5 */
        uint32  tx_ts:1;                /* 5 */
        uint32  set_l3bm:1;             /* 5 */
        uint32  hdr_offset:8;           /* 6 */
        uint32  set_l2bm:1;             /* 7 */
        uint32  dst_port:7;             /* 7 */
        uint32  input_pri:4;            /* 8 */
        uint32  cos:4;                  /* 8 ' */
        uint32  spid_override:1;        /* 9 ' */
        uint32  spid:2;                 /* 9 ' */
        uint32  spap:2;                 /* 9 ' */
        uint32  unicast:1;              /* 9 ' */
        uint32  queue_num_2:2;          /* 9 */
        uint32  queue_num_1:8;          /* 10 */
        uint32  src_mod:8;              /* 11 */
    } overlay2;
#endif /* !LE_HOST */
} soc_pbsmh_v7_hdr_t;


#define PBS_MH_V7_HDR_TYPE_EP_COPY_TO_CPU       0
#define PBS_MH_V7_HDR_TYPE_FROM_CPU             1
#define PBS_MH_V7_HDR_TYPE_MIM                  2
#define PBS_MH_V7_HDR_TYPE_MPLS_PMP             3
#define PBS_MH_V7_HDR_TYPE_TRILL_NW             4
#define PBS_MH_V7_HDR_TYPE_TRILL_AC             5
#define PBS_MH_V7_HDR_TYPE_WLAN_DECAP           6
#define PBS_MH_V7_HDR_TYPE_WLAN_ENCAP           7
#define PBS_MH_V7_HDR_TYPE_QCN                  8
#define PBS_MH_V7_HDR_TYPE_SM_DPI               9
#define PBS_MH_V7_HDR_TYPE_EP_REDIER            0xA
#define PBS_MH_V7_HDR_TYPE_GENERIC              0xB

/*
 * Fast macros to setup the PBSMH
 */
#if defined(LE_HOST)
#define PBS_MH_W0_START_SET(mh) (((uint32 *)(mh))[0] = 0x000000FF)
#define PBS_MH_W1_RSVD_SET(mh)  (((uint32 *)(mh))[1] = 0x00000000)

#define PBS_MH_V1_W2_SMOD_DPORT_COS_SET(mh, smod, dport, cos, dc1, dc2) \
        (((uint32 *)(mh))[2] = (smod << 16) | (cos << 29) | (dport << 24))

#define PBS_MH_V2_W2_SMOD_DPORT_COS_SET(mh, smod, dport, cos, ipri, l3pbm_sel) \
        (((uint32 *)(mh))[2] = (smod << 8) | (ipri << 20) | (cos << 16) | \
                               (l3pbm_sel << 31) | (dport << 24))

#define PBS_MH_V3_W2_SMOD_DPORT_COS_SET(mh, smod, dport, cos, ipri, l3pbm_sel) \
        (((uint32 *)(mh))[2] = ((smod >> 4) & 0xf) | ((smod & 0xf) << 12) | \
                               (ipri << 8) | (cos << 16) | \
                               (l3pbm_sel << 31) | (dport << 24))

#define PBS_MH_V4_W2_SMOD_DPORT_COS_SET(mh, smod, dport, cos, ipri, l3pbm_sel) \
        (((uint32 *)(mh))[2] = ((smod >> 7) & 0x1) | ((smod & 0x7f) << 9) | \
                               (((ipri >> 3) & 0x1) << 8) | \
                               ((ipri & 0x7) << 21) | (cos << 16) | \
                               (l3pbm_sel << 31) | (dport << 24))

#define PBS_MH_V5_W1_SMOD_SET(mh, smod, unicast, set_l2bm, set_l3bm) \
        (((uint32 *)(mh))[1] = (((smod & 0xff) << 24) | \
                                (unicast ? 0x10000 : 0) | \
                                (set_l2bm ? 0x20000 : 0) | \
                                (set_l3bm ? 0x40000 : 0)))

#define PBS_MH_V5_W2_DPORT_COS_SET(mh, dport, cos, ipri) \
        (((uint32 *)(mh))[2] = ((ipri & 0xf) | \
                                ((cos & 0x7f) << 8) | \
                                ((cos & 0xf) << 16) | \
                                ((dport & 0x7f) << 24)))

#define PBS_MH_V6_W2_DPORT_COS_QNUM_SET(mh, dport, cos, qnum, ipri) \
        (((uint32 *)(mh))[2] = (((ipri & 0xf) >> 2) | \
                                ((cos & 0x3f) << 16) | \
                                ((dport & 0x7f) << 24) | \
                                ((qnum & 0x3) << 22) | \
                                ((qnum & 0x3fc) << 8) | \
                                ((qnum & 0xc00) >> 10)))
#define PBS_MH_V6_TS_PKT_SET(mh) (((uint32 *)(mh))[1] |=  (1 << 8))
#define PBS_MH_V6_TS_ONE_STEP_INGRESS_SIGN_PKT_SET(mh) \
                                (((uint32 *)(mh))[0] |=  (1 << 30))
#define PBS_MH_V6_TS_ONE_STEP_HDR_START_REGEN_UDP_CHEKSUM_PKT_SET(mh) \
                                (((uint32 *)(mh))[0] |=  (1 << 29))
#define PBS_MH_V6_TS_ONE_STEP_PKT_SET(mh) \
                                (((uint32 *)(mh))[0] |=  (1 << 28))
#define PBS_MH_V6_TS_ONE_STEP_HDR_START_OFFSET_PKT_SET(mh) \
                                (((uint32 *)(mh))[0] |=  (1 << 20))


#define PBS_MH_V2_TS_PKT_SET(mh) (((uint32 *)(mh))[2] |=  (1 << 4))
#define PBS_MH_V3_TS_PKT_SET(mh) (((uint32 *)(mh))[2] |=  (1 << 4))
#define PBS_MH_V5_TS_PKT_SET(mh) (((uint32 *)(mh))[1] |=  (1 << 12))

#define PBS_MH_V7_W0_START_SET(mh) (((uint32 *)(mh))[0] = 0x00000081)
#define PBS_MH_V7_W1_DPORT_SET(mh, dport) \
        (((uint32 *)(mh))[1] = ((dport & 0x7f) << 24))
#define PBS_MH_V7_TS_PKT_SET(mh) (((uint32 *)(mh))[1] |=  (1 << 9))
#define PBS_MH_V7_TS_ONE_STEP_INGRESS_SIGN_PKT_SET(mh) \
                                (((uint32 *)(mh))[1] |=  (1 << 10))
#define PBS_MH_V7_TS_ONE_STEP_HDR_START_REGEN_UDP_CHEKSUM_PKT_SET(mh) \
                                (((uint32 *)(mh))[1] |=  (1 << 11))
#define PBS_MH_V7_TS_ONE_STEP_PKT_SET(mh) \
                                (((uint32 *)(mh))[1] |=  (1 << 12))
#define PBS_MH_V7_TS_ONE_STEP_HDR_START_OFFSET_PKT_SET(mh) \
                                (((uint32 *)(mh))[1] |=  (1 << 16))

#define PBS_MH_V7_W2_SMOD_COS_QNUM_SET(mh, smod, unicast, cos, qnum, ipri) \
        (((uint32 *)(mh))[2] = (((smod & 0xff) << 24) | \
                                ((qnum & 0xff) << 16) | \
                                (qnum & 0x300) | \
                                (unicast ? 0x400 : 0) | \
                                ((ipri & 0xf) << 4) | \
                                 (cos & 0xf)))

#define PBS_MH_START_IS_SET(mh) \
        (((((uint32 *)(mh))[0]) & 0x000000FF) == 0x000000FF)

#else /* !LE_HOST */
#define PBS_MH_W0_START_SET(mh) (((uint32 *)(mh))[0] = 0xFF000000)
#define PBS_MH_W1_RSVD_SET(mh)  (((uint32 *)(mh))[1] = 0x00000000)

#define PBS_MH_V1_W2_SMOD_DPORT_COS_SET(mh, smod, dport, cos, dc1, dc2) \
        (((uint32 *)(mh))[2] = (smod << 8) | (cos << 5) | (dport << 0))

#define PBS_MH_V2_W2_SMOD_DPORT_COS_SET(mh, smod, dport, cos, ipri, l3pbm_sel) \
        (((uint32 *)(mh))[2] = (smod << 16) | (ipri << 12) | (cos << 8) |      \
                               (l3pbm_sel << 7) | (dport << 0))

#define PBS_MH_V3_W2_SMOD_DPORT_COS_SET(mh, smod, dport, cos, ipri, l3pbm_sel) \
        (((uint32 *)(mh))[2] = (smod << 20) | (ipri << 16) | (cos << 8) |      \
                               (l3pbm_sel << 7) | (dport << 0))

#define PBS_MH_V4_W2_SMOD_DPORT_COS_SET(mh, smod, dport, cos, ipri, l3pbm_sel) \
        (((uint32 *)(mh))[2] = (smod << 17) | (ipri << 13) | (cos << 8) |      \
                               (l3pbm_sel << 7) | (dport << 0))

#define PBS_MH_V5_W1_SMOD_SET(mh, smod, unicast, set_l2bm, set_l3bm) \
        (((uint32 *)(mh))[1] = ((smod & 0xff) | (unicast ? 0x0100 : 0) | \
                                (set_l2bm ? 0x0200 : 0) | \
                                (set_l3bm ? 0x0400 : 0)))

#define PBS_MH_V5_W2_DPORT_COS_SET(mh, dport, cos, ipri) \
        (((uint32 *)(mh))[2] = (((ipri & 0xf) << 24) | \
                                ((cos & 0x7f) << 16) | \
                                ((cos & 0xf) << 8) | \
                                (dport & 0x7f)))

#define PBS_MH_V6_W2_DPORT_COS_QNUM_SET(mh, dport, cos, qnum, ipri) \
        (((uint32 *)(mh))[2] = (((ipri & 0xf) << 26) | \
                                ((qnum & 0xfff) << 14) | \
                                ((cos & 0x3f) << 8) | \
                                (dport & 0x7f)))
#define PBS_MH_V6_TS_PKT_SET(mh) (((uint32 *)(mh))[1] |=  (1 << 11))
#define PBS_MH_V6_TS_ONE_STEP_INGRESS_SIGN_PKT_SET(mh) \
                                (((uint32 *)(mh))[1] |=  (1 << 28))
#define PBS_MH_V6_TS_ONE_STEP_HDR_START_REGEN_UDP_CHEKSUM_PKT_SET(mh) \
                                (((uint32 *)(mh))[1] |=  (1 << 29))
#define PBS_MH_V6_TS_ONE_STEP_PKT_SET(mh) \
                                (((uint32 *)(mh))[1] |=  (1 << 30))
#define PBS_MH_V6_TS_ONE_STEP_HDR_START_OFFSET_PKT_SET(mh) \
                                (((uint32 *)(mh))[1] |=  (1 << 20))

#define PBS_MH_V2_TS_PKT_SET(mh) (((uint32 *)(mh))[2] |=  (1 << 28))
#define PBS_MH_V3_TS_PKT_SET(mh) (((uint32 *)(mh))[2] |=  (1 << 28))
#define PBS_MH_V5_TS_PKT_SET(mh) (((uint32 *)(mh))[1] |=  (1 << 20))

#define PBS_MH_V7_W0_START_SET(mh) (((uint32 *)(mh))[0] = 0x81000000)
#define PBS_MH_V7_W1_DPORT_SET(mh, dport) \
        (((uint32 *)(mh))[1] = (dport & 0x7f))
#define PBS_MH_V7_TS_PKT_SET(mh) (((uint32 *)(mh))[1] |=  (1 << 23))
#define PBS_MH_V7_TS_ONE_STEP_INGRESS_SIGN_PKT_SET(mh) \
                                (((uint32 *)(mh))[1] |=  (1 << 24))
#define PBS_MH_V7_TS_ONE_STEP_HDR_START_REGEN_UDP_CHEKSUM_PKT_SET(mh) \
                                (((uint32 *)(mh))[1] |=  (1 << 25))
#define PBS_MH_V7_TS_ONE_STEP_PKT_SET(mh) \
                                (((uint32 *)(mh))[1] |=  (1 << 26))
#define PBS_MH_V7_TS_ONE_STEP_HDR_START_OFFSET_PKT_SET(mh) \
                                (((uint32 *)(mh))[1] |=  (1 << 16))


#define PBS_MH_V7_W2_SMOD_COS_QNUM_SET(mh, smod, unicast, cos, qnum, ipri) \
        (((uint32 *)(mh))[2] = (((ipri & 0xf) << 28) | \
                                ((cos & 0xf) << 24) | \
                                (unicast ? 0x40000 : 0) | \
                                ((qnum & 0x3ff) << 8) | \
                                (smod & 0xff)))

#define PBS_MH_START_IS_SET(mh) \
        (((((uint32 *)(mh))[0]) & 0xFF000000) == 0xFF000000)

#endif /* !LE_HOST */

#define SOC_PBSMH_HDR_SIZE      ((int) sizeof(soc_pbsmh_hdr_t))

/*
 * PBS MH Field Manipulation
 */

typedef enum {
    /* NOTE: strings in soc_pbsmh_field_names[] must match */
    PBSMH_invalid = -1,
    PBSMH_start = 0,
    PBSMH_src_mod,      /* Source Modid to disable E2E checks */
    PBSMH_dst_port,     /* Destination port */
    PBSMH_cos,          /* COS queue */
    PBSMH_pri,          /* Input Priority */
    PBSMH_l3pbm_sel,    /* Use L3 PBM */
    PBSMH_l2pbm_sel,    /* Use L2 PBM */
    PBSMH_unicast,      /* Unicast */
    PBSMH_tx_ts,        /* Capture transmit timestamp */
    PBSMH_spid_override,
    PBSMH_spid,         /* Service pool ID */
    PBSMH_spap,         /* Service pool priority */
    PBSMH_queue_num,    /* Queue number */
    PBSMH_osts,         /* 1588-TS 1-step */
    PBSMH_its_sign,     /* 1588-TS Ingress timestamp sign (for 1-step) */
    PBSMH_hdr_offset,   /* 1588-TS header start offset (for 1-step) */
    PBSMH_regen_udp_checksum, /* 588-TS UDP checksum update (for 1-step) */
    PBSMH_int_pri,      /* Internal Priority */
    PBSMH_nlf_port,     /* ? */
    PBSMH_lm_ctr_index, /* ? */
    PBSMH_oam_replacement_type, /* See OAM replacement types below */
    PBSMH_oam_replacement_offset, /* OAM insertion point = 14 + (2 * val) */
    PBSMH_ep_cpu_reasons, /* EP Copy to CPU reasons */
    PBSMH_header_type,  /* See PBSMH header types below */
    PBSMH_COUNT
} soc_pbsmh_field_t;

#ifndef CMICM_EMBEDDED

extern soc_pbsmh_field_t soc_pbsmh_name_to_field(int unit, char *name);
extern char *soc_pbsmh_field_to_name(int unit, soc_pbsmh_field_t f);

extern uint32 soc_pbsmh_field_get(int unit, soc_pbsmh_hdr_t *mh,
                                  soc_pbsmh_field_t field);
extern void soc_pbsmh_field_set(int unit, soc_pbsmh_hdr_t *mh,
                                soc_pbsmh_field_t field, uint32 val);

extern void soc_pbsmh_dump(int unit, char *pfx, soc_pbsmh_hdr_t *mh);

#define soc_pbsmh_field_init(unit, mh) \
        sal_memset((mh), 0, sizeof (soc_pbsmh_hdr_t))

#endif

/* PBSMH field default values */
/* default SOP Symbol */
#define SOC_PBSMH_START         0xff /* PBS MH indicator */
#define SOC_PBSMH_START_INTERNAL 0x2 /* PBS MH internal header indicator */

#endif /* BCM_XGS3_SWITCH_SUPPORT */
#endif  /* !_SOC_PBSMH_H */
