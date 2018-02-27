/* 
 * $Id: bfd_msg.h 1.1.2.3 Broadcom SDK $
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
 * File:    bfd_msg.h
 * Purpose: BFD Messages definitions common to SDK and uKernel.
 *
 *          Messages between SDK and uKernel.
 */

#ifndef   _SOC_SHARED_BFD_MSG_H_
#define   _SOC_SHARED_BFD_MSG_H_

#ifdef BCM_UKERNEL
  /* Build for uKernel not SDK */
  #include "sdk_typedefs.h"
#else
  #include <sal/types.h>
#endif

#include <soc/shared/mos_msg_common.h>
#include <shared/bfd.h>


/********************************
 * BFD Control Messages
 *
 *   SDK Host <--> uController
 */

/*
 * BFD Initialization control message
 */
typedef struct shr_bfd_msg_ctrl_init_s {
    uint32  num_sessions;        /* Max number of BFD sessions */
    uint32  encap_size;          /* Total encapsulation table size */
    uint32  num_auth_sha1_keys;  /* Max number of sha1 auth keys */
    uint32  num_auth_sp_keys;    /* Max number of simple pwd auth keys */
    uint32  rx_channel;          /* Local RX DMA channel (0..3) */
} shr_bfd_msg_ctrl_init_t;

/*
 * BFD Encapsulation control message
 *
 * Encapsulation Maximum length
 * L2 + L3 + L4
 * L2 + 2 VLAN + IPV6 + GRE + IPV6 + UDP + BFD + SHA1 + FCS
 * 14 + 8 + 40 + 4 + 40 + 8 + 24 + 28 + 4
 *
 *
 * BFD Encapsulation Table
 *
 * The BFD Encapsulation is maintained in the uKernel
 * It contains the encapsulation for all current BFD sessions.
 *
 * A BFD encapsulation currently includes:
 *     L2
 *     [MPLS]
 *     [IP/UDP]
 *
 * A BFD encapsulation does NOT include the BFD PDU (BFD Control Packet).
 * This is built by the uKernel side.
 *
 * See following definitions for a given 'BFD-x' encapsulation:
 *
 *  |----'encap_offset-x'----+
 *                           |------- 'encap_length-x'--------|
 *                           |
 *  BFD Encapsulation Table  |
 *  +----------+-------------+--------------------------------+--------+
 *  | BFD-1    |    ...      |   BFD-x                        |  ....  |
 *  |          |             | L2 [MPLS..]    .....[IP/UDP]   |        |
 *  +----------+-------------+--------------------------------+--------+
 *                           |                    ^
 *                           |                    |
 *                           +-- 'lkey_offset-x'--+
 *
 * NOTE:
 * The Lookup Key (lkey_offset/lkey_length) is used by the SW to match
 * an initial RX BFD packet to an existing session entry.  An initial
 * BFD packet has 'your_discriminator' set to 0, so HW is not able
 * to make a 'hit' on the corresponding session entry.
 *
 * Note that 'lkey_offset' is the offset relative the start of a
 * given BFD encapsulation (rather than the BFD encapsulation table).
 */
#define SHR_BFD_MAX_ENCAP_LENGTH    114

typedef struct shr_bfd_msg_ctrl_encap_s {
    uint32  offset;    /* Byte offset into Encap table */
    uint32  length;    /* Encapsulation length */
    uint8   data[SHR_BFD_MAX_ENCAP_LENGTH];  /* Encapsulation data */
} shr_bfd_msg_ctrl_encap_t;

/*
 * BFD Session Set control message
 */
typedef struct shr_bfd_msg_ctrl_sess_set_s {
    uint32  sess_id;
    uint32  flags;
    uint8   passive;
    uint8   local_demand;
    uint8   local_diag;
    uint8   local_detect_mult;
    uint32  local_discriminator;
    uint32  remote_discriminator;
    uint32  local_min_tx;
    uint32  local_min_rx;
    uint32  local_min_echo_rx;
    uint8   auth_type;
    uint32  auth_key;
    uint32  xmt_auth_seq;
    uint8   encap_type;    /* Raw, UDP-IPv4/IPv6, used for UDP checksum */
    uint32  encap_offset;  /* BFD encapsulation offset in encap table */
    uint32  encap_length;  /* BFD encapsulation length */
    uint16  lkey_offset;   /* Lookup key offset from 'encap_offset' */
    uint16  lkey_length;   /* Lookup key length */
    uint32  mep_id_offset;
    uint32  mep_id_length;
    uint32  tx_port;
    uint32  tx_cos;
    uint32  tx_pri;
    uint32  tx_qnum;
} shr_bfd_msg_ctrl_sess_set_t;

/*
 * BFD Session Get control message
 */
typedef struct shr_bfd_msg_ctrl_sess_get_s {
    uint32   sess_id;
    uint8    enable;
    uint8    passive;
    uint8    local_demand;
    uint8    remote_demand;
    uint8    local_diag;
    uint8    remote_diag;
    uint8    local_sess_state;
    uint8    remote_sess_state;
    uint8    local_detect_mult;
    uint8    remote_detect_mult;
    uint32   local_discriminator;
    uint32   remote_discriminator;
    uint32   local_min_tx;
    uint32   remote_min_tx;
    uint32   local_min_rx;
    uint32   remote_min_rx;
    uint32   local_min_echo_rx;
    uint32   remote_min_echo_rx;
    uint8    auth_type;
    uint32   auth_key;
    uint32   xmt_auth_seq;
    uint32   rcv_auth_seq;
    uint8    encap_type;    /* Raw, UDP-IPv4/IPv6, used for UDP checksum */
    uint32   encap_offset;  /* BFD encapsulation offset in encap table */
    uint32   encap_length;  /* BFD encapsulation length */
    uint16   lkey_offset;   /* Lookup key offset from 'encap_offset' */
    uint16   lkey_length;   /* Lookup key length */
    uint32   mep_id_offset;
    uint32   mep_id_length;
    uint32   tx_port;
    uint32   tx_cos;
    uint32   tx_pri;
    uint32   tx_qnum;
} shr_bfd_msg_ctrl_sess_get_t;

/*
 * BFD SHA1 Authentication control message
 */
typedef struct shr_bfd_msg_ctrl_auth_sha1_s {
    uint32  index;
    uint8   enable;
    uint32  sequence;
    uint8   key[_SHR_BFD_AUTH_SHA1_KEY_LENGTH];
} shr_bfd_msg_ctrl_auth_sha1_t;

/*
 * BFD Simple Password Authentication control message
 *
 * Set length to zero to disable entry.
 */
typedef struct shr_bfd_msg_ctrl_auth_sp_s {
    uint32  index;
    uint8   length;
    uint8   password[_SHR_BFD_AUTH_SIMPLE_PASSWORD_KEY_LENGTH];
} shr_bfd_msg_ctrl_auth_sp_t;

/*
 * BFD Statistics control messages (Request/Reply)
 */
typedef struct shr_bfd_msg_ctrl_stat_req_s {
    uint32  sess_id;    /* BFD session (endpoint) id */
    uint32  clear;      /* Clear stats */
} shr_bfd_msg_ctrl_stat_req_t;

typedef struct shr_bfd_msg_ctrl_stat_reply_s {
    uint32  sess_id;           /* BFD session (endpoint) id */
    uint32  packets_in;        /* Total packets in */
    uint32  packets_out;       /* Total packets out */
    uint32  packets_drop;      /* Total packets drop */
    uint32  packets_auth_drop; /* Packets drop due to authentication failure */
} shr_bfd_msg_ctrl_stat_reply_t;

/*
 * BFD control messages
 */
typedef union shr_bfd_msg_ctrl_s {
    shr_bfd_msg_ctrl_init_t         init;
    shr_bfd_msg_ctrl_encap_t        encap;
    shr_bfd_msg_ctrl_sess_set_t     sess_set;
    shr_bfd_msg_ctrl_sess_get_t     sess_get;
    shr_bfd_msg_ctrl_auth_sp_t      auth_sp;
    shr_bfd_msg_ctrl_auth_sha1_t    auth_sha1;
    shr_bfd_msg_ctrl_stat_req_t     stat_req;
    shr_bfd_msg_ctrl_stat_reply_t   stat_reply;
} shr_bfd_msg_ctrl_t;


/****************************************
 * BFD event message
 */
#define BFD_BTE_EVENT_STATE                      0x0001
#define BFD_BTE_EVENT_REMOTE_STATE_DIAG          0x0002
#define BFD_BTE_EVENT_DISCRIMINATOR              0x0004
#define BFD_BTE_EVENT_AUTHENTICATION             0x0008
#define BFD_BTE_EVENT_PARAMETER                  0x0010
#define BFD_BTE_EVENT_ERROR                      0x0020

/*
 *  The BFD event message is defined as a short message (use mos_msg_data_t).
 *
 *  The fields of mos_msg_data_t are used as followed:
 *      mclass   (uint8)  - MOS_MSG_CLASS_BFD_EVENT
 *      subclass (uint8)  - Unused
 *      len      (uint16) - BFD Session ID
 *      data     (uint32) - Events mask
 */
typedef mos_msg_data_t  bfd_msg_event_t;


#endif /* _SOC_SHARED_BFD_MSG_H_ */
