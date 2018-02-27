/*
 * $Id: sbx_pkt.h 1.4 Broadcom SDK $
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
 * == sbx_pkt.h - SBX Packet Class Functions ==
 */

#ifndef _SBXPKT_H_
#define _SBXPKT_H_

#define MAXARGS 100
#define MAXLEN 2000
#define MAXUSRSTR 4096
#define MAXFIELD 256

#include <sal/types.h>
#include <sal/appl/sal.h>
#include <appl/diag/system.h>
#include <soc/mem.h>

#define printf printk

typedef enum header_type_s {
  PACKET,
  ERH_QE,
  ERH_SS,
  ERH_QESS,
  MAC,
  VLAN,
  STAG,
  ETYPE,
  LLC,
  SNAP,
  IPV4,
  ITAG,
  MPLS,
  UDP,
  TCP,
  IGMP,
  RAW_DATA,
  UNKNOWN,
} header_type_e;

typedef enum op_s {
  CREATE,
  MODIFY,
  PREPEND,
  APPEND,
} op_e;

typedef enum fill_mode_s {
  INCREMENT,
  FIXED,
  RANDOM,
} fill_mode_e;

/*
 * Public: Entry description
 * entry_desc_t
 */
typedef struct entry_desc_s {
    int    type;
    void   *next;
    uint32 length;
} entry_desc_t;

/* 
 * Public: Packet Header
 * sbxpkt_t
 */
typedef struct sbxpkt_s {
  entry_desc_t entry;
  uint32   normalize;
} sbxpkt_t;

/* 
 * Public Functions 
 *
 */ 
int sbxpkt_create (sbxpkt_t *packet, char *char_data);
int sbxpkt_prepend (sbxpkt_t *packet, char *char_data);
int sbxpkt_append (sbxpkt_t *packet, char *char_data);
sbxpkt_t* sbxpkt_alloc (void);
int sbxpkt_free (sbxpkt_t *packet);
int sbxpkt_print (sbxpkt_t *packet);
int sbxpkt_compare (sbxpkt_t *sbxtx_pkt, sbxpkt_t *rx_pkt);
int to_byte (sbxpkt_t *packet, unsigned char *pkt_data);
int from_byte (header_type_e start_type, unsigned char *pkt_data, int length, sbxpkt_t *return_pkt);
int debug_print (unsigned char * pkt_data, int length);

/*
 ***raw_data
 */
typedef struct raw_data_s {
  entry_desc_t entry;
  uint32   value;
  uint32   mode;
  uint32   flags;
  unsigned char raw_data[MAXLEN];
} raw_data_t;

/* 
 ***mac
 */
typedef struct mac_s {
  entry_desc_t entry;
  unsigned char dmac[6];
  unsigned char smac[6];
} mac_t;

/* 
 ***vlan
 */
typedef struct vlan_s {
  entry_desc_t entry;
  uint32 tpid;
  uint32 vid;
  uint32 cfi;
  uint32 pri;
} vlan_t;

/* 
 ***stag
 */
typedef struct stag_s { 
  entry_desc_t entry;
  uint32 tpid;
  uint32 vid;
  uint32 dei;
  uint32 pcp;
} stag_t;

/* 
 ***etype
 */
typedef struct etype_s { 
  entry_desc_t entry;
  uint32 etype;
} etype_t;

/* 
 ***llc 
 */
typedef struct llc_s {
  entry_desc_t entry;
  uint32 len;
  uint32 ssap;
  uint32 dsap;
  uint32 ctrl;
} llc_t;

/* 
 ***snap
 */
typedef struct snap_s {
  entry_desc_t entry;
  uint32 oui;
} snap_t;

/* 
 ***ipv4
 */
typedef struct ipv4_s { 
entry_desc_t entry;
uint32 ver;
uint32 ihl;
uint32 tos;
uint32 length;
uint32 id;
uint32 df;
uint32 mf;
uint32 offset;
uint32 ip_opt;
uint32 proto;
uint32 ttl;
uint32 checksum;
uint32 sa;
uint32 da;
} ipv4_t;


/* 
 ***itag 
 */
typedef struct itag_s {
 entry_desc_t entry;
 uint32 ipcp;
 uint32 idei;
 uint32 nca;
 uint32 isid;
} itag_t;

/* 
 ***mpls 
 */
typedef struct mpls_s{
 entry_desc_t entry;
 uint32 label;
 uint32 exp;
 uint32 s;
 uint32 ttl;
} mpls_t;

/* 
 *** udp
 */
typedef struct udp_s {
 entry_desc_t entry;
 uint32 sport; 
 uint32 dport; 
 uint32 len; 
 uint32 checksum; 
} udp_t;

/* 
 *** tcp 
 */
typedef struct tcp_s {
 entry_desc_t entry;
 uint32 sport;
 uint32 dport;
 uint32 seqn;
 uint32 ackn;
 uint32 dofs;
 uint32 ecn;
 uint32 ctrl;
 uint32 wind;
 uint32 checksum;
 uint32 urgp;
 uint32 option;
} tcp_t;

/* 
 *** igmp
 */
typedef struct igmp_s { 
 entry_desc_t entry;
 uint32 ver; 
 uint32 type; 
 uint32 checksum; 
 uint32 group;
} igmp_t;

/* 
 *** erh_qe
 */
typedef struct erh_qe_s {
 entry_desc_t entry;
 uint32 ttl;
 uint32 s;
 uint32 rdp;
 uint32 rcos;
 uint32 sid;
 uint32 mc;
 uint32 out_union;
 uint32 len_adj;
 uint32 frm_len;
 uint32 test;
 uint32 e;
 uint32 fdp;
 uint32 qid;
} erh_qe_t;

/* 
 *** erh_ss
 */
typedef struct erh_ss_s {
 entry_desc_t entry;
 uint32 ttl;
 uint32 s;
 uint32 rdp;
 uint32 rcos;
 uint32 lbid;
 uint32 fcos2;
 uint32 fdp;
 uint32 sid;
 uint32 out_union;
 uint32 qid;
 uint32 len_adj;
 uint32 mc;
 uint32 test;
 uint32 ecn;
 uint32 ect;
 uint32 ksop;
} erh_ss_t;

/* 
 *** erh_qess
 */
typedef struct erh_qess_s {
 entry_desc_t entry;
 uint32 ttl;
 uint32 s;
 uint32 rdp;
 uint32 rcos;
 uint32 lbid;
 uint32 fcos2;
 uint32 sid;
 uint32 out_union;
 uint32 zero;
 uint32 ect;
 uint32 mc;
 uint32 len_adj;
 uint32 frm_len;
 uint32 test;
 uint32 ecn;
 uint32 fdp;
 uint32 qid;
} erh_qess_t;

#endif /* _SBXPKT_H_ */
