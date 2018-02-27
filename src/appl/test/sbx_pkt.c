/*
 * $Id: sbx_pkt.c 1.11 Broadcom SDK $
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
 * File:        sbx_pkt.c
 * Purpose:     FE2K Packet Class functions
 * Requires:
 */

#ifdef BCM_FE2000_SUPPORT

#include <appl/test/sbx_pkt.h>

int pkt_verbose = 0;

uint32 string_to_int (char *value) {
  unsigned long int lvalue;
  lvalue = strtol(value,0,0);
  return ((int)(lvalue));
}
  
int string_to_mac (char *value, uint8 *mac) {
  int i;
  int j=0;
  int k=0;
  char data[6];
  
  for (i=0; i<32; i++) {
    if (value[i] == '\0') {
       sal_memcpy (data, &value[j], i-j);
       mac[k] = strtol(data,0,16);
       break;
    }
    if (value[i] == ':') {
       sal_memcpy (data, &value[j], i-j);
       mac[k] = strtol(data,0,16);
       j = i+1;
       k++;
    }
  }
  return 0;
}

int stuff_data (raw_data_t *raw_data) {
  int i;

  if (raw_data->mode == 0) {
    for (i=0; i < (MAXLEN/4); i++) {
      raw_data->raw_data[(i*4)+0] = ((raw_data->value + i) >> 24) & 0xff;
      raw_data->raw_data[(i*4)+1] = ((raw_data->value + i) >> 16) & 0xff;
      raw_data->raw_data[(i*4)+2] = ((raw_data->value + i) >> 8) & 0xff;
      raw_data->raw_data[(i*4)+3] = ((raw_data->value + i) >> 0) & 0xff;
    }
  } else {
    for (i=0; i < (MAXLEN/4); i++) {
      raw_data->raw_data[(i*4)+0] = ((raw_data->value) >> 24) & 0xff;
      raw_data->raw_data[(i*4)+1] = ((raw_data->value) >> 16) & 0xff;
      raw_data->raw_data[(i*4)+2] = ((raw_data->value) >> 8) & 0xff;
      raw_data->raw_data[(i*4)+3] = ((raw_data->value) >> 0) & 0xff;
    }
  }
  return 0;
}


#if 0 
Checksum information (to be implemented)

UDP:
Computed as the 16-bit one's complement of the one's complement sum of a pseudo header of information from the IP header, the UDP header, and the data, padded as needed with zero bytes at the end to make a multiple of two bytes. If the checksum is cleared to zero, then checksuming is disabled. If the computed checksum is zero, then this field must be set to 0xFFFF.

TCP:
This is computed as the 16-bit one's complement of the one's complement sum of a pseudo header of information from the IP header, the TCP header, and the data, padded as needed with zero bytes at the end to make a multiple of two bytes. The pseudo header contains the following fields:
  Source IP address
  Destination IP address
  0 + IP Protocol + Total length

#endif

/* ******************************
 * Build headers from the arg vector list
 * ******************************/

int erh_qe_build (erh_qe_t *erh_qe, int arg_start, int arg_end, char **argv) {
  int i;
  erh_qe->entry.type = ERH_QE;
  erh_qe->entry.length = 12;

  erh_qe->ttl       = 0;
  erh_qe->s         = 0;
  erh_qe->rdp       = 0;
  erh_qe->rcos      = 0;
  erh_qe->sid       = 0;
  erh_qe->mc        = 0;
  erh_qe->out_union = 0;
  erh_qe->len_adj   = 0;
  erh_qe->frm_len   = 0;
  erh_qe->test      = 0;
  erh_qe->e         = 0;
  erh_qe->fdp       = 0;
  erh_qe->qid       = 0;

  /* TODO: perform parameter range checks here */
  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-ttl"))) {
      erh_qe->ttl = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-s"))) {
      erh_qe->s = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-rdp"))) {
      erh_qe->rdp = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-rcos"))) {
      erh_qe->rcos = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-sid"))) {
      erh_qe->sid = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-mc"))) {
      erh_qe->mc = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-out_union"))) {
      erh_qe->out_union = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-len_adj"))) {
      erh_qe->len_adj = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-frm_len"))) {
      erh_qe->frm_len = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-test"))) {
      erh_qe->test = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-e"))) {
      erh_qe->e = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-fdp"))) {
      erh_qe->fdp = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-qid"))) {
      erh_qe->qid = string_to_int(argv[i+1]);
    }
  }
  return 0;
}

int erh_ss_build (erh_ss_t *erh_ss, int arg_start, int arg_end, char **argv) {
  int i;
  erh_ss->entry.type = ERH_SS;
  erh_ss->entry.length = 12;

  erh_ss->ttl       = 0;
  erh_ss->s         = 0;
  erh_ss->rdp       = 0;
  erh_ss->rcos      = 0;
  erh_ss->lbid      = 0;
  erh_ss->fcos2     = 0;
  erh_ss->fdp       = 0;
  erh_ss->sid       = 0;
  erh_ss->out_union = 0;
  erh_ss->qid       = 0;
  erh_ss->len_adj   = 0;
  erh_ss->mc        = 0;
  erh_ss->test      = 0;
  erh_ss->ecn       = 0;
  erh_ss->ect       = 0;
  erh_ss->ksop      = 0;

  /* TODO: perform parameter range checks here */
  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-ttl"))) {
      erh_ss->ttl = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-s"))) {
      erh_ss->s = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-rdp"))) {
      erh_ss->rdp = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-rcos"))) {
      erh_ss->rcos = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-lbid"))) {
      erh_ss->lbid = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-fcos2"))) {
      erh_ss->fcos2 = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-fdp"))) {
      erh_ss->fdp = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-sid"))) {
      erh_ss->sid = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-out_union"))) {
      erh_ss->out_union = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-qid"))) {
      erh_ss->qid = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-len_adj"))) {
      erh_ss->len_adj = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-mc"))) {
      erh_ss->mc = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-test"))) {
      erh_ss->test = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-ecn"))) {
      erh_ss->ecn = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-ect"))) {
      erh_ss->ect = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-ksop"))) {
      erh_ss->ksop = string_to_int(argv[i+1]);
    }
  }
  return 0;
}

int erh_qess_build (erh_qess_t *erh_qess, int arg_start, int arg_end, char **argv) {
  int i;
  erh_qess->entry.type = ERH_QESS;
  erh_qess->entry.length = 14;

  erh_qess->ttl       = 0;
  erh_qess->s         = 0; 
  erh_qess->rdp       = 0;
  erh_qess->rcos      = 0;
  erh_qess->lbid      = 0;
  erh_qess->fcos2     = 0;
  erh_qess->sid       = 0;
  erh_qess->out_union = 0;
  erh_qess->zero      = 0;
  erh_qess->ect       = 0;
  erh_qess->mc        = 0;
  erh_qess->len_adj   = 0;
  erh_qess->frm_len   = 0;
  erh_qess->test      = 0;
  erh_qess->ecn       = 0;
  erh_qess->fdp       = 0;
  erh_qess->qid       = 0;

  /* TODO: perform parameter range checks here */
  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-ttl"))) {
      erh_qess->ttl = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-s"))) {
      erh_qess->s = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-rdp"))) {
      erh_qess->rdp = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-rcos"))) {
      erh_qess->rcos = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-lbid"))) {
      erh_qess->lbid = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-fcos2"))) {
      erh_qess->fcos2 = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-sid"))) {
      erh_qess->sid = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-out_union"))) {
      erh_qess->out_union = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-zero"))) {
      erh_qess->zero = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-mc"))) {
      erh_qess->mc = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-len_adj"))) {
      erh_qess->len_adj = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-frm_len"))) {
      erh_qess->frm_len = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-test"))) {
      erh_qess->test = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-ecn"))) {
      erh_qess->ecn = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-fdp"))) {
      erh_qess->fdp = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-qid"))) {
      erh_qess->qid = string_to_int(argv[i+1]);
    }
  }
  return 0;
}

int mac_build (mac_t *mac, int arg_start, int arg_end, char **argv) {
  int i;
  mac->entry.type = MAC;
  mac->entry.length = 12;

  mac->dmac[0]=0; mac->dmac[1]=0x11; mac->dmac[2]=0x22; mac->dmac[3]=0x33; mac->dmac[4]=0x44; mac->dmac[5]=0x55;
  mac->smac[0]=0; mac->smac[1]=0x11; mac->smac[2]=0x22; mac->smac[3]=0x33; mac->smac[4]=0x44; mac->smac[5]=0x66;

  /* TODO: perform parameter range checks here */
  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-dmac"))) {
      string_to_mac(argv[i+1], mac->dmac);
    }
    if (!(sal_strcmp(argv[i], "-smac"))) {
      string_to_mac(argv[i+1], mac->smac);
    }
  }
  return 0;
}

int vlan_build (vlan_t *vlan, int arg_start, int arg_end, char **argv) {
  int i;
  vlan->entry.type = VLAN;
  vlan->entry.length = 4;

  vlan->tpid = 0x8100;
  vlan->vid  = 0xabc;
  vlan->cfi  = 0;
  vlan->pri  = 3;

  /* TODO: perform parameter range checks here */
  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-tpid"))) {
      vlan->tpid = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-vid"))) {
      vlan->vid = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-cfi"))) {
      vlan->cfi = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-pri"))) {
      vlan->pri = string_to_int(argv[i+1]);
    }
  }
  return 0;
}

int stag_build (stag_t *stag, int arg_start, int arg_end, char **argv) {
  int i;
  stag->entry.type = STAG;
  stag->entry.length = 4;

  stag->tpid = 0x88a8;
  stag->vid  = 0xabc;
  stag->dei  = 1;
  stag->pcp  = 3;

  /* TODO: perform parameter range checks here */
  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-tpid"))) {
      stag->tpid = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-vid"))) {
      stag->vid = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-dei"))) {
      stag->dei = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-pcp"))) {
      stag->pcp = string_to_int(argv[i+1]);
    }
  } 
  return 0;
}

int etype_build (etype_t *etype, int arg_start, int arg_end, char **argv) {
  int i;
  etype->entry.type = ETYPE;
  etype->entry.length = 2;

  etype->etype = 0x0801;

  /* TODO: perform parameter range checks here */
  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-etype"))) {
      etype->etype = string_to_int(argv[i+1]);
    }
  }
  return 0;
}

int llc_build (llc_t *llc, int arg_start, int arg_end, char **argv) {
  int i;
  llc->entry.type = LLC;
  llc->entry.length = 7;

  llc->len  = 64;
  llc->ssap = 0xaa;
  llc->dsap = 0xaa;
  llc->ctrl = 3;

  /* TODO: perform parameter range checks here */
  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-len"))) {
      llc->len = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-ssap"))) {
      llc->ssap = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-dsap"))) {
      llc->dsap = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-ctrl"))) {
      llc->ctrl = string_to_int(argv[i+1]);
    }
  }
  return 0;
}

int snap_build (snap_t *snap, int arg_start, int arg_end, char **argv) {
  int i;
  snap->entry.type = SNAP;
  snap->entry.length = 3;

  snap->oui = 0;

  /* TODO: perform parameter range checks here */
  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-oui"))) {
      snap->oui = string_to_int(argv[i+1]);
    }
  }
  return 0;
}

int ipv4_build (ipv4_t *ipv4, int arg_start, int arg_end, char **argv) {
  int i;
  ipv4->entry.type = IPV4;
  ipv4->entry.length = 20;

  ipv4->ver      = 4;
  ipv4->ihl      = 5;
  ipv4->tos      = 0;
  ipv4->length   = 0;
  ipv4->id       = 0;
  ipv4->df       = 0;
  ipv4->mf       = 0;
  ipv4->offset   = 0;
  ipv4->ip_opt   = 0;
  ipv4->proto    = 6;
  ipv4->ttl      = 0x40;
  ipv4->checksum = 0;
  ipv4->sa       = 0x01010101;
  ipv4->da       = 0x02020202;

  /* TODO: perform parameter range checks here */
  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-ver"))) {
      ipv4->ver = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-ihl"))) {
      ipv4->ihl = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-tos"))) {
      ipv4->tos = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-length"))) {
      ipv4->length = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-id"))) {
      ipv4->id = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-df"))) {
      ipv4->df = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-mf"))) {
      ipv4->mf = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-offset"))) {
      ipv4->offset = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-ip_opt"))) {
      ipv4->ip_opt = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-proto"))) {
      ipv4->proto = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-ttl"))) {
      ipv4->ttl = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-checksum"))) {
      ipv4->checksum = string_to_int(argv[i+1]);
    }
    
    if (!(sal_strcmp(argv[i], "-sa"))) {
      ipv4->sa = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-da"))) {
      ipv4->da = string_to_int(argv[i+1]);
    }
  }
  return 0;
}

int itag_build (itag_t *itag, int arg_start, int arg_end, char **argv) {
  int i;
  itag->entry.type = ITAG;
  itag->entry.length = 4;

  itag->ipcp = 3;
  itag->idei = 0;
  itag->nca  = 0;
  itag->isid = 0xa5a5a;

  /* TODO: perform parameter range checks here */
  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-ipcp"))) {
      itag->ipcp = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-idei"))) {
      itag->idei = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-nca"))) {
      itag->nca = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-isid"))) {
      itag->isid = string_to_int(argv[i+1]);
    }
  }
  return 0;
}

int mpls_build (mpls_t *mpls, int arg_start, int arg_end, char **argv) {
  int i;
  mpls->entry.type = MPLS;
  mpls->entry.length = 4;

  mpls->label = 0x1234;
  mpls->exp   = 0;
  mpls->s     = 1;
  mpls->ttl   = 0x40;

  /* TODO: perform parameter range checks here */
  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-label"))) {
      mpls->label = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-exp"))) {
      mpls->exp = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-s"))) {
      mpls->s = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-ttl"))) {
      mpls->ttl = string_to_int(argv[i+1]);
    }
  }
  return 0;
}

int udp_build (udp_t *udp, int arg_start, int arg_end, char **argv) {
  int i;
  udp->entry.type = UDP;
  udp->entry.length = 8;

  udp->sport = 1;
  udp->dport = 1;
  udp->len   = 0;  
  udp->checksum = 0; /* 0 means checksum is disabled */

  /* TODO: perform parameter range checks here */
  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-sport"))) {
      udp->sport = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-dport"))) {
      udp->dport = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-len"))) {
      udp->len = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-checksum"))) {
      udp->checksum = string_to_int(argv[i+1]);
    }
  }
  return 0;
}

int tcp_build (tcp_t *tcp, int arg_start, int arg_end, char **argv) {
  int i;
  tcp->entry.type = TCP;

  tcp->sport    = 0;
  tcp->dport    = 0;
  tcp->seqn     = 0;
  tcp->ackn     = 0;
  tcp->dofs     = 0;
  tcp->ecn      = 0;
  tcp->ctrl     = 0;
  tcp->wind     = 0;
  tcp->checksum = 0;
  tcp->urgp     = 0;
  tcp->option   = 0;

  /* TODO: perform parameter range checks here */
  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-sport"))) {
      tcp->sport = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-dport"))) {
      tcp->dport = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-seqn"))) {
      tcp->seqn = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-ackn"))) {
      tcp->ackn = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-dofs"))) {
      tcp->dofs = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-ecn"))) {
      tcp->ecn = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-ctrl"))) {
      tcp->ctrl = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-wind"))) {
      tcp->wind = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-checksum"))) {
      tcp->checksum = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-urgp"))) {
      tcp->urgp = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-option"))) {
      tcp->option = string_to_int(argv[i+1]);
    }
  }
  return 0;
}

int igmp_build (igmp_t *igmp, int arg_start, int arg_end, char **argv) {
  int i;
  igmp->entry.type = IGMP;

  igmp->ver      = 0;
  igmp->type     = 0;
  igmp->checksum = 0;
  igmp->group    = 0;

  /* TODO: perform parameter range checks here */
  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-ver"))) {
      igmp->ver = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-type"))) {
      igmp->type = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-checksum"))) {
      igmp->checksum = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-group"))) {
      igmp->group = string_to_int(argv[i+1]);
    }
  }
  return 0;
}

int raw_data_build (raw_data_t *raw_data, int arg_start, int arg_end, char **argv) {
  int i;
  raw_data->entry.type = RAW_DATA;
  raw_data->entry.length = 40;

  raw_data->value = 0xba53ba11;
  raw_data->mode  = 0;  /* INCREMENT */
  raw_data->flags = 0;

  /* TODO: perform parameter range checks here */
  for (i=arg_start; i < arg_end; i += 2) {
    if (!(sal_strcmp(argv[i], "-value"))) {
      raw_data->value = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-mode"))) {
      raw_data->mode = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-flags"))) {
      raw_data->flags = string_to_int(argv[i+1]);
    }
    if (!(sal_strcmp(argv[i], "-length"))) {
      raw_data->entry.length = string_to_int(argv[i+1]);
    }
  }

  stuff_data (raw_data);
  return 0;
}


/* ******************************
 * print header functions
 * ******************************/
void print_mac_header (mac_t *mac) {
  printf ("--mac     -dmac %02x:%02x:%02x:%02x:%02x:%02x -smac %02x:%02x:%02x:%02x:%02x:%02x\n", 
    mac->dmac[0], mac->dmac[1], mac->dmac[2], mac->dmac[3], mac->dmac[4], mac->dmac[5], 
    mac->smac[0], mac->smac[1], mac->smac[2], mac->smac[3], mac->smac[4], mac->smac[5]);
}
void print_vlan_header (vlan_t *vlan) {
  printf ("--vlan    -tpid 0x%04x pri 0x%01x -cfi 0x%01x -vid 0x%03x\n", vlan->tpid, vlan->pri, vlan->cfi, vlan->vid);
}
void print_stag_header (stag_t *stag) {
  printf ("--stag    -tpid 0x%04x -pcp 0x%01x -dei 0x%01x -vid 0x%03x\n", stag->tpid, stag->pcp, stag->dei, stag->vid);
}
void print_etype_header (etype_t *etype) {
  printf ("--etype   -etype 0x%04x\n", etype->etype);
}
void print_llc_header (llc_t *llc) {
  printf ("--llc     -len 0x%04x -ssap 0x%04x -dsap 0x%04x -ctrl 0x%04x\n",   
                       llc->len, llc->ssap, llc->dsap, llc->ctrl);
}
void print_snap_header (snap_t *snap) {
  printf ("--snap    -oui=0x%08x\n", snap->oui);
}
void print_ipv4_header (ipv4_t *ipv4) {
  printf ("--ipv4    -ver 0x%01x -ihl 0x%01x -tos 0x%02x -length 0x%04x -id 0x%04x -df 0x%01x\n",  
                        ipv4->ver, ipv4->ihl, ipv4->tos, ipv4->length, ipv4->id, ipv4->df);
  printf ("          -mf 0x%01x -offset 0x%04x -proto 0x%02x -ttl 0x%02x -checksum 0x%04x\n",  
                        ipv4->mf, ipv4->offset, ipv4->proto, ipv4->ttl, ipv4->checksum);
  printf ("          -sa 0x%08x -da 0x%08x\n", 
                        ipv4->sa, ipv4->da);
  if (ipv4->ihl > 5) {
    printf ("          -ip_opt 0x%08x\n", ipv4->ip_opt);
  }
}
void print_itag_header (itag_t *itag) {
  printf ("--itag    -ipcp 0x%01x -idei 0x%01x -ncs 0x%01x -isid 0x%04x\n",   
                        itag->ipcp, itag->idei, itag->nca, itag->isid);
}
void print_mpls_header (mpls_t *mpls) {
  printf ("--mpls    -label 0x%05x -exp 0x%01x -s 0x%01x -ttl 0x%02x\n",    
                        mpls->label, mpls->exp, mpls->s, mpls->ttl);
}
void print_udp_header (udp_t *udp) {
  printf ("--udp     -sport 0x%04x -dport 0x%04x -len 0x%04x -checksum 0x%04x\n",    
                       udp->sport, udp->dport, udp->len, udp->checksum);
}
void print_tcp_header (tcp_t *tcp) {
  printf ("--tcp     -sport 0x%04x -dport 0x%04x -seqn 0x%04x -ackn 0x%04x -dofs 0x%04x\n",  
                       tcp->sport, tcp->dport, tcp->seqn, tcp->ackn, tcp->dofs);
  printf ("          -ecn 0x%01x -ctrl 0x%01x -wind 0x%01x -checksum 0x%04x -urgp 0x%01x\n", 
                       tcp->ecn, tcp->ctrl, tcp->wind, tcp->checksum, tcp->urgp);
  if (tcp->dofs > 5) {
    printf ("          -option=0x%08x\n", tcp->option);
  }
}
void print_igmp_header (igmp_t *igmp) {
  printf ("--igmp    -ver 0x%04x -type 0x%04x -checksum 0x%04x -group 0x%04x\n",
                        igmp->ver, igmp->type, igmp->checksum, igmp->group);
}
void print_erh_qe_header (erh_qe_t *erh_qe) {
  printf ("--erh_qe  -frm_len 0x%04x -test 0x%04x -e 0x%01x -fdp 0x%01x -qid 0x%04x\n",
                    erh_qe->frm_len, erh_qe->test, erh_qe->e, erh_qe->fdp, erh_qe->qid);
  printf ("          -sid 0x%04x -mc 0x%01x -out_union 0x%05x -len_adj 0x%01x\n", 
                    erh_qe->sid, erh_qe->mc, erh_qe->out_union, erh_qe->len_adj);
  printf ("          -ttl 0x%02x -s 0x%01x -rdp 0x%01x -rcos 0x%01x\n",
                    erh_qe->ttl, erh_qe->s, erh_qe->rdp, erh_qe->rcos);
}
void print_erh_ss_header (erh_ss_t *erh_ss) {
  printf ("--erh_ss  -test 0x%04x -ecn 0x%04x -ect 0x%04x -ksop 0x%04x\n",
                    erh_ss->test, erh_ss->ecn, erh_ss->ect, erh_ss->ksop);
  printf ("          -sid 0x%04x -out_union 0x%05x -qid 0x%04x -len_adj 0x%01x -mc 0x%01x\n", 
                    erh_ss->sid, erh_ss->out_union, erh_ss->qid, erh_ss->len_adj, erh_ss->mc);
  printf ("          -ttl 0x%02x -s 0x%01x -rdp 0x%01x -rcos 0x%01x -lbid 0x%04x -fcos2 0x%01x -fdp 0x%01x\n",
                    erh_ss->ttl, erh_ss->s, erh_ss->rdp, erh_ss->rcos, erh_ss->lbid, erh_ss->fcos2, erh_ss->fdp);
}
void print_erh_qess_header (erh_qess_t *erh_qess) {
  printf ("--erh_qess -len_adj 0x%01x -frm_len 0x%04x -test 0x%04x -ecn 0x%04x -fdp 0x%04x -qid 0x%04x\n",
                    erh_qess->len_adj, erh_qess->frm_len, erh_qess->test, erh_qess->ecn, erh_qess->fdp, erh_qess->qid);
  printf ("          -sid 0x%04x -out_union 0x%05x -zero 0x%04x -ect 0x%04x -mc 0x%01x\n", 
                    erh_qess->sid, erh_qess->out_union, erh_qess->zero, erh_qess->ect, erh_qess->mc);
  printf ("          -ttl 0x%02x -s 0x%01x -rdp 0x%01x -rcos 0x%01x -lbid 0x%04x -fcos2 0x%01x\n", 
                    erh_qess->ttl, erh_qess->s, erh_qess->rdp, erh_qess->rcos, erh_qess->lbid, erh_qess->fcos2);
}
void print_raw_data_header (raw_data_t *raw_data) {
  int i;
  int line_len = 16;

  /* 
   * printf ("--raw_data  -length %d  -mode %d -value 0x%08x\n", 
   *             raw_data->entry.length, raw_data->mode, raw_data->value); 
   */

  printf ("--raw_data    -length %d\n", raw_data->entry.length);

  for (i=0; i<raw_data->entry.length; i++) {
    if (i % line_len == 0) {
      if (i !=0) printf("\n");
      printf("%04x  ", i);
    }
    printf ("%02x ", raw_data->raw_data[i]);
  }

  printf ("\n");

} 


/* ******************************
 * test for htype
 * ******************************/
int is_htype(char *arg) { 
  if (*arg != '-') return 0;
  arg++;
  if (*arg != '-') return 0;
  return 1;
}

/* ******************************
 * Main Build header
 * - options:
 *       create new packet
 *       modify fields (not implemented)
 *       prepend fields
 *       append fields
 * ******************************/
int header_build (op_e op, sbxpkt_t* pkt, int argc, char **argv) {
  int i,j;
  int num_fields;
  entry_desc_t *cur_hdr = NULL;
  entry_desc_t *next_hdr = NULL;

  erh_qe_t   *erh_qe;
  erh_ss_t   *erh_ss;
  erh_qess_t *erh_qess;
  mac_t   *mac;
  vlan_t  *vlan;
  stag_t  *stag;
  llc_t   *llc;
  snap_t  *snap;
  etype_t *etype;
  mpls_t  *mpls;
  itag_t  *itag;
  ipv4_t  *ipv4;
  tcp_t   *tcp;
  udp_t   *udp;
  igmp_t  *igmp;

  raw_data_t *raw_data;

  if (op == CREATE) {
    cur_hdr = (entry_desc_t *)pkt;
    cur_hdr->type = PACKET;
    cur_hdr->length = 0;
  } else if (op == PREPEND) {
    cur_hdr = (entry_desc_t *)pkt;
    next_hdr = cur_hdr->next;
  } else if (op == APPEND) {
    cur_hdr = (entry_desc_t *)pkt;
    if ((cur_hdr == NULL) || (cur_hdr->next == NULL)) {
      printf("no header found to append\n");
      return -1;
    }
    next_hdr = cur_hdr->next;
    while ((cur_hdr != NULL) && (next_hdr->type != RAW_DATA)) {
      if (cur_hdr->next == NULL) {
        printf("no header found to append\n");
        return -1;
      }
      if (((entry_desc_t*)cur_hdr->next)->next != NULL) {
        next_hdr = ((entry_desc_t*)cur_hdr->next)->next;
      }
      cur_hdr = cur_hdr->next;
    }
    if (cur_hdr == NULL) {
      /* fatal error */
      printf("expected raw header at end of packet\n");
      return -1;
    }
    /* adjust packet length to remove old data and free old data header  */
    pkt->entry.length -= next_hdr->length;
    sal_free (next_hdr);
  } else {
    printf("unexpected operation %d\n", op);
    return -1;
  }

  for (i=0; i <= argc; ) {
    /* first arg should be a htype */
    if (!(is_htype (argv[i]))) {
      printf("expected htype at %d, got %s\n", i, argv[i]);
      return -1;
    }

    /* find next htype */
    for (j=i+1; j <= argc; j++) {
      if (is_htype (argv[j])) {
        break;
      }
    }

    num_fields = j - i - 1;

    if (!sal_strcmp(argv[i], "--erh_qe")) {
      erh_qe = sal_alloc(sizeof(erh_qe_t), "erh_qe");
      erh_qe_build (erh_qe, i+1, i+num_fields, argv);
      /* print_erh_qe_header (erh_qe); */
      cur_hdr->next = (void *)erh_qe;
      cur_hdr = (entry_desc_t *)erh_qe;
    } else if (!sal_strcmp(argv[i], "--erh_ss")) {
      erh_ss = sal_alloc(sizeof(erh_ss_t),"erh_ss");
      erh_ss_build (erh_ss, i+1, i+num_fields, argv);
      /* print_erh_ss_header (erh_ss); */
      cur_hdr->next = (void *)erh_ss;
      cur_hdr = (entry_desc_t *)erh_ss;
    } else if (!sal_strcmp(argv[i], "--erh_qess")) {
      erh_qess = sal_alloc(sizeof(erh_qess_t),"erh_qess");
      erh_qess_build (erh_qess, i+1, i+num_fields, argv);
      /* print_erh_qess_header (erh_qess); */
      cur_hdr->next = (void *)erh_qess;
      cur_hdr = (entry_desc_t *)erh_qess;
    } else if (!sal_strcmp(argv[i], "--mac")) {
      mac = sal_alloc(sizeof(mac_t),"mac");
      mac_build (mac, i+1, i+num_fields, argv);
      /* print_mac_header (mac); */
      cur_hdr->next = (void *)mac;
      cur_hdr = (entry_desc_t *)mac;
    } else if (!sal_strcmp(argv[i], "--vlan")) {
      vlan = sal_alloc(sizeof(vlan_t),"vlan");
      vlan_build (vlan, i+1, i+num_fields, argv);
      /* print_vlan_header (vlan); */
      cur_hdr->next = (void *)vlan;
      cur_hdr = (entry_desc_t *)vlan;
    } else if (!sal_strcmp(argv[i], "--stag")) {
      stag = sal_alloc(sizeof(stag_t),"stag");
      stag_build (stag, i+1, i+num_fields, argv);
      /* print_stag_header (stag); */
      cur_hdr->next = (void *)stag;
      cur_hdr = (entry_desc_t *)stag;
    } else if (!sal_strcmp(argv[i], "--etype")) {
      etype = sal_alloc(sizeof(etype_t),"etype");
      etype_build (etype, i+1, i+num_fields, argv);
      /* print_etype_header (etype); */
      cur_hdr->next = (void *)etype;
      cur_hdr = (entry_desc_t *)etype;
    } else if (!sal_strcmp(argv[i], "--llc")) {
      llc = sal_alloc(sizeof(llc_t),"llc");
      llc_build (llc, i+1, i+num_fields, argv);
      /* print_llc_header (llc); */
      cur_hdr->next = (void *)llc;
      cur_hdr = (entry_desc_t *)llc;
    } else if (!sal_strcmp(argv[i], "--snap")) {
      snap = sal_alloc(sizeof(snap_t),"snap");
      snap_build (snap, i+1, i+num_fields, argv);
      /* print_snap_header (snap); */
      cur_hdr->next = (void *)snap;
      cur_hdr = (entry_desc_t *)snap;
    } else if (!sal_strcmp(argv[i], "--ipv4")) {
      ipv4 = sal_alloc(sizeof(ipv4_t),"ipv4");
      ipv4_build (ipv4, i+1, i+num_fields, argv);
      /* print_ipv4_header (ipv4); */
      cur_hdr->next = (void *)ipv4;
      cur_hdr = (entry_desc_t *)ipv4;
    } else if (!sal_strcmp(argv[i], "--itag")) {
      itag = sal_alloc(sizeof(itag_t),"itag");
      itag_build (itag, i+1, i+num_fields, argv);
      /* print_itag_header (itag); */
      cur_hdr->next = (void *)itag;
      cur_hdr = (entry_desc_t *)itag;
    } else if (!sal_strcmp(argv[i], "--mpls")) {
      mpls = sal_alloc(sizeof(mpls_t),"mpls");
      mpls_build (mpls, i+1, i+num_fields, argv);
      /* print_mpls_header (mpls); */
      cur_hdr->next = (void *)mpls;
      cur_hdr = (entry_desc_t *)mpls;
    } else if (!sal_strcmp(argv[i], "--udp")) {
      udp = sal_alloc(sizeof(udp_t),"udp");
      udp_build (udp, i+1, i+num_fields, argv);
      /* print_udp_header (udp); */
      cur_hdr->next = (void *)udp;
      cur_hdr = (entry_desc_t *)udp;
    } else if (!sal_strcmp(argv[i], "--tcp")) {
      tcp = sal_alloc(sizeof(tcp_t),"tcp");
      tcp_build (tcp, i+1, i+num_fields, argv);
      /* print_tcp_header (tcp); */
      cur_hdr->next = (void *)tcp;
      cur_hdr = (entry_desc_t *)tcp;
    } else if (!sal_strcmp(argv[i], "--igmp")) {
      igmp = sal_alloc(sizeof(igmp_t),"igmp");
      igmp_build (igmp, i+1, i+num_fields, argv);
      /* print_igmp_header (igmp); */
      cur_hdr->next = (void *)igmp;
      cur_hdr = (entry_desc_t *)igmp;
    } else if (!sal_strcmp(argv[i], "--raw_data")) {
      raw_data = sal_alloc(sizeof(raw_data_t),"raw_data");
      raw_data_build (raw_data, i+1, i+num_fields, argv);
      /* print_raw_data_header (raw_data); */
      cur_hdr->next = (void *)raw_data;
      cur_hdr = (entry_desc_t *)raw_data;
      /* Special case - last header */
      cur_hdr->next = NULL;
    } else {
      printf("header build: unexpected htype %s\n", argv[i]);
    }

    /* printf ("found htype %s and %d fields at %d\n", argv[i], num_fields, i); */
    pkt->entry.length += cur_hdr->length;

    i = j;
  }

  if ((op == CREATE) || (op == APPEND)) {
    /* add RAW_DATA if not specified */
    if (cur_hdr->type != RAW_DATA) {
      raw_data = sal_alloc(sizeof(raw_data_t),"raw_data");
      raw_data->entry.type = RAW_DATA;
      if (pkt->entry.length < 64) {
        raw_data->entry.length = 64 - pkt->entry.length;
        pkt->entry.length = 64;
      } else {
        raw_data->entry.length = 0;
      }
      raw_data->value = 0xba53ba11;
      raw_data->mode = 0;
      stuff_data (raw_data);
      cur_hdr->next = (void *)raw_data;
      cur_hdr = (entry_desc_t *)raw_data;
      cur_hdr->next = NULL;
    }
  }
  if (op == PREPEND) {
    cur_hdr->next = next_hdr;
  }

  return 0;
}

/* ******************************
 * Convert_string
 * ******************************/
int convert_string(char *char_str, int *argc_return, char **argv) {

  char *next_str;
  char *curr_str;
  int  argc;
  int  i;

  char copy_str[MAXUSRSTR];
  for (i=0; i< MAXUSRSTR; i++) {
    copy_str[i] = '\0';
  }

  if (sal_strlen(char_str) > MAXUSRSTR) return -1;
  sal_strcpy (copy_str, char_str);

  curr_str = copy_str;

  /* printf ("convert string \n"); */

  /* initialize */
  for(argc=0; argc < MAXARGS; argc++) {
    argv[argc][0] = '\0';
  }

  /* printf ("String = %s \n", curr_str); */

  /* skip leading white space */
  for (argc=0; argc < MAXARGS && (*curr_str) != '\0';) {
    while(*curr_str == ' ') {
      curr_str++;
    }
    /* find space or end */
    for (next_str = curr_str; *next_str; next_str++) {
      if (*next_str == ' ') {
        break;
      }
    }

    *next_str = '\0';
    /* printf ("argc = %d and string = %s \n", argc, curr_str); */

    sal_strcpy (argv[argc++], curr_str);

    next_str++;
    curr_str = next_str;
  }

  /* empty command line */
  if (argc < 1) {
    argc = 1;
    argv[0][0] = '\0';
  }

  *argc_return = argc - 1;
  return 0;
}

/* ************************
 * Decodes and Encodes
 * ************************/
int decode_erh_qe (erh_qe_t *erh_qe, uint8 *data) {
  uint32 value;
 /* word 0 */
  sal_memcpy (&value, data, 4);
  value = soc_ntohl (value);
  erh_qe->qid  = (value >> 18) & 0x3fff;  /* includes fcos */
  erh_qe->fdp  = (value >> 16) & 0x3;
  erh_qe->e       = (value >> 15) & 0x1;
  erh_qe->test    = (value >> 14) & 0x1;
  erh_qe->frm_len = (value >> 0)  & 0x3ff;

 /* word 1 */
  sal_memcpy (&value, data + 4, 4);
  value = soc_ntohl (value);
  erh_qe->len_adj   = (value >> 28) & 0xf; 
  erh_qe->out_union = (value >>  8) & 0x3ffff;
  erh_qe->mc        = (value >>  7) & 0x1;

 /* word 2 */
  sal_memcpy (&value, data + 8, 4);
  value = soc_ntohl (value);
  erh_qe->sid  = (value >> 18) & 0x3fff;
  
  /* erh_qe->swop = (value >> 16) & 0x3; */
  erh_qe->rcos = (value >> 11) & 0x7;
  erh_qe->rdp  = (value >> 9) & 0x3;
  erh_qe->s    = (value >> 8) & 0x1;
  erh_qe->ttl  = (value >> 0) & 0xff;

  erh_qe->entry.length = 12;
  return 0;
}
int encode_erh_qe (erh_qe_t *erh_qe, uint8 *data) {
  uint32 value;
  uint16 value16;
  uint8  value8;
 /* word 0 */
  value16 = (((erh_qe->qid & 0x3fff) >> 2) |
             ((erh_qe->fdp & 0x3)    >> 0));
  value16 = soc_htons (value16);
  sal_memcpy (data, &value16, 2);

  value16 = (((erh_qe->e       & 0x1)   << 15) |
             ((erh_qe->test    & 0x1)   << 14) |
             ((erh_qe->frm_len & 0x3ff) << 0));
  value16 = soc_htons (value16);
  sal_memcpy (data + 2, &value16, 2);

 /* word 1 */
  value = (((erh_qe->len_adj   & 0xf)     << 28) |
           ((erh_qe->out_union & 0x3ffff) <<  8) |
           ((erh_qe->mc        & 0x1)     <<  7) |
           ((0                 & 0x7f)    <<  0));
  value = soc_htonl (value);
  sal_memcpy (data + 4, &value, 4);

 /* word 2 */
  value16 = (((erh_qe->sid & 0x3fff) << 2) |
             ((0           & 0x3)    << 0));
  value16 = soc_htons (value16);
  sal_memcpy (data + 8, &value16, 2);

  value8 = (((erh_qe->rcos & 0x7) << 3) |
            ((erh_qe->rdp  & 0x3) << 1) |
            ((erh_qe->s    & 0x1) << 0));
  sal_memcpy (data + 10, &value8, 1);

  value8 = (erh_qe->ttl * 0xff);
  sal_memcpy (data + 11, &value8, 1);
  return 0;
}

int decode_erh_ss (erh_ss_t *erh_ss, uint8 *data) {
  uint32 value;
 /* word 0 */
  sal_memcpy (&value, data, 4);
  value = soc_ntohl (value);
  erh_ss->ksop      = (value >> 24) & 0xff;
  erh_ss->ect       = (value >> 23) & 0x1;
  erh_ss->ecn       = (value >> 22) & 0x1;
  erh_ss->test      = (value >> 21) & 0x1;
  erh_ss->mc        = (value >> 20) & 0x1;
  erh_ss->len_adj   = (value >> 16) & 0xf;
  erh_ss->out_union = (value >> 0)  & 0xffff;

 /* word 1 */
  sal_memcpy (&value, data + 4, 4);
  value = soc_ntohl (value);
  /* erh_ss->rsvd0   = (value >> 30) & 0x3; */
  erh_ss->qid     = (value >> 16) & 0x3fff;  /* includes fcos */
  erh_ss->sid     = (value >>  0) & 0xffff;

 /* word 2 */
  sal_memcpy (&value, data + 8, 4);
  value = soc_ntohl (value);
  /* erh_ss->rsvd1  = (value >> 24) & 0xff; */
  erh_ss->fdp    = (value >> 22) & 0x3;
  erh_ss->fcos2  = (value >> 19) & 0x7;
  erh_ss->lbid   = (value >> 16) & 0x7;
  erh_ss->rcos   = (value >> 12) & 0x7;
  erh_ss->rdp    = (value >>  9) & 0x3;
  erh_ss->s      = (value >>  8) & 0x1;
  erh_ss->ttl    = (value >>  0) & 0xff;

  erh_ss->entry.length = 12;
  return 0;
}
int encode_erh_ss (erh_ss_t *erh_ss, uint8 *data) {
  return 0;
}

int decode_erh_qess (erh_qess_t *erh_qess, uint8 *data) {
  uint32 value;
  uint16 value16;
 /* word 0 */
  sal_memcpy (&value, data, 4);
  value = soc_ntohl (value);
  erh_qess->qid     = (value >> 18) & 0x3fff;  /* includes fcos */
  erh_qess->fdp     = (value >> 16) & 0x3;
  erh_qess->ecn     = (value >> 15) & 0x1;
  erh_qess->test    = (value >> 14) & 0x1;
  erh_qess->frm_len = (value >>  0) & 0x3ff;

 /* word 1 */
  sal_memcpy (&value, data + 4, 4);
  value = soc_ntohl (value);
  erh_qess->len_adj   = (value >> 28) & 0xf;
  erh_qess->out_union = (value >> 14) & 0x3fff;
  erh_qess->mc        = (value >>  0) & 0x1;

 /* word 2 */
  sal_memcpy (&value, data + 8, 4);
  value = soc_ntohl (value);
  erh_qess->sid    = (value >> 16) & 0xffff;
  /* erh_qess->fdp2   = (value >> 14) & 0x3; */
  erh_qess->fcos2  = (value >> 11) & 0x7;
  erh_qess->lbid   = (value >>  8) & 0x7;
  erh_qess->zero   = (value >>  0) & 0xff;

  sal_memcpy (&value16, data + 12, 2);
  value16 = soc_ntohs (value);
  erh_qess->rcos   = (value16 >> 12) & 0x7;
  erh_qess->rdp    = (value16 >>  9) & 0x3;
  erh_qess->s      = (value16 >>  8) & 0x1;
  erh_qess->ttl    = (value16 >>  0) & 0xff;

  erh_qess->entry.length = 14;
  return 0;
}
int encode_erh_qess (erh_qess_t *erh_qess, uint8 *data) {
  return 0;
}

int decode_mac (mac_t *mac, uint8 *data) {
  sal_memcpy (mac->dmac, data, 6);
  sal_memcpy (mac->smac, data + 6, 6);
  mac->entry.length = 12;
  return 0;
}
int encode_mac (mac_t *mac, uint8 *data) {
  sal_memcpy (data, mac->dmac, 6);
  sal_memcpy (data + 6, mac->smac, 6);
  return 0;
}

int decode_vlan (vlan_t *vlan, uint32 value) {
  vlan->tpid = (value >> 16) & 0xffff;
  vlan->pri  = (value >> 13) & 0x7;
  vlan->cfi  = (value >> 12) & 0x1;
  vlan->vid  = (value >>  0) & 0xfff;
  vlan->entry.length = 4;
  return 0;
}
int encode_vlan (vlan_t *vlan, uint8 *data) {
  uint32 value;
  value = (((vlan->tpid & 0xffff) << 16) | 
           ((vlan->pri  & 0x7)    << 13) |
           ((vlan->cfi  & 0x1)    << 12) |
           ((vlan->vid  & 0xfff)  <<  0));
  value = soc_htonl (value);
  sal_memcpy (data, &value, 4);
  return 0;
}

int decode_stag (stag_t *stag, uint32 value) {
  stag->tpid = (value >> 16) & 0xffff;
  stag->pcp  = (value >> 13) & 0x7;
  stag->dei  = (value >> 12) & 0x1;
  stag->vid  = (value >>  0) & 0xfff;
  stag->entry.length = 4;
  return 0;
}
int encode_stag (stag_t *stag, uint8 *data) {
  uint32 value;
  value = (((stag->tpid & 0xffff) << 16) |
           ((stag->pcp  & 0x7)    << 13) |
           ((stag->dei  & 0x1)    << 12) |
           ((stag->vid  & 0xfff)  <<  0));
  value = soc_htonl (value);
  sal_memcpy (data, &value, 4);
  return 0;
}

int decode_etype (etype_t *etype, unsigned int value) {
  etype->etype = (value >> 16) & 0xffff;
  etype->entry.length = 2;
  return 0;
}
int encode_etype (etype_t *etype, uint8 *data) {
  uint16 value;
  value = ((etype->etype & 0xffff)); 
  value = soc_htons (value);
  sal_memcpy (data, &value, 2);
  return 0;
}

int decode_mpls (mpls_t *mpls, uint32 value) {
  mpls->label = (value >> 12) & 0xfffff;
  mpls->exp   = (value >>  9) & 0x7;
  mpls->s     = (value >>  8) & 0x1;
  mpls->ttl   = (value >>  0) & 0xff;
  mpls->entry.length = 4;
  return 0;
}
int encode_mpls (mpls_t *mpls, uint8 *data) {
  uint32 value;
  value = (((mpls->label & 0xfffff)  << 12) |
           ((mpls->exp   &     0x7)  <<  9) |
           ((mpls->s     &     0x1)  <<  8) |
           ((mpls->ttl   &    0xff)  <<  0));
  value = soc_htonl (value);
  sal_memcpy (data, &value, 4);
  return 0;
}

int decode_llc (llc_t *llc, uint8 *data) {
  sal_memcpy (&llc->len, data, 2);
  sal_memcpy (&llc->ssap, data + 2, 2);
  sal_memcpy (&llc->dsap, data + 4, 2);
  sal_memcpy (&llc->ctrl, data + 6, 1);
  llc->entry.length = 7;
  return 0;
}
int encode_llc (llc_t *llc, uint8 *data) {
  sal_memcpy (data, &llc->len,  2);
  sal_memcpy (data + 2, &llc->ssap, 2);
  sal_memcpy (data + 4, &llc->dsap, 2);
  sal_memcpy (data + 6, &llc->ctrl, 1);
  return 0;
}

int decode_snap (snap_t *snap, uint8 *data) {
  sal_memcpy (&snap->oui, data, 3);
  snap->entry.length = 3;
  return 0;
}
int encode_snap (snap_t *snap, uint8 *data) {
  sal_memcpy (data, &snap->oui, 3);
  return 0;
}

int decode_ipv4 (ipv4_t *ipv4, uint8 *data) {
  uint32 value;
 
 /* word 0 */
  sal_memcpy (&value, data, 4);
  value = soc_ntohl (value);
  ipv4->ver = (value >> 28) & 0xf;
  ipv4->ihl = (value >> 24) & 0xf;
  ipv4->tos = (value >> 16) & 0xff;
  ipv4->length = (value >> 0) & 0xffff;

 /* word 1 */
  sal_memcpy (&value, data + 4, 4);
  value = soc_ntohl (value);
  ipv4->id = (value >> 16) & 0xffff;
  ipv4->df = (value >> 14) & 0x1;
  ipv4->mf = (value >> 13) & 0x1;
  ipv4->offset  = (value >> 0) & 0x1fff;

 /* word 2 */
  sal_memcpy (&value, data + 8, 4);
  value = soc_ntohl (value);
  ipv4->ttl = (value >> 24) & 0xff;
  ipv4->proto = (value >> 16) & 0xff;
  ipv4->checksum = (value >> 0) & 0xffff;

 /* word 3 and 4 */
  sal_memcpy (&value, data + 12, 4);
  ipv4->sa = soc_ntohl (value);
  sal_memcpy (&value, data + 16, 4);
  ipv4->da = soc_ntohl (value);

  ipv4->entry.length = 20;
  return 0;
}
int encode_ipv4 (ipv4_t *ipv4, uint8 *data) {
  uint32 value;
  uint16 value16;
  uint8  value8;
 
 /* word 0 */
  value8 = (((ipv4->ver & 0xf) << 4) |
            ((ipv4->ihl & 0xf) << 0));
  sal_memcpy (data, &value8, 1);
  value8 = (ipv4->tos & 0xff);
  sal_memcpy (data + 1, &value8, 1);
  value16 = (ipv4->length & 0xffff);
  sal_memcpy (data + 2, &value16, 2);

 /* word 1 */
  sal_memcpy (data + 4, &ipv4->id, 2);
  value16 = (((0         &    0x1)  << 15) |
             ((ipv4->df  &    0x1)  << 14) |
             ((ipv4->mf  &    0x1)  << 13) |
             ((ipv4->ttl & 0x1fff)  <<  0));
  value16 = soc_htons (value16);
  sal_memcpy (data + 6, &value16, 2);

 /* word 2 */
  value8 = (ipv4->ttl & 0xff);
  sal_memcpy (data + 8, &value8, 1);
  value8 = (ipv4->proto & 0xff);
  sal_memcpy (data + 9, &value8, 1);
  value16 = (ipv4->checksum & 0xffff);
  sal_memcpy (data + 10, &value16, 2);

 /* word 3 and 4 */
  value = soc_htonl (ipv4->sa);
  sal_memcpy (data + 12, &value, 4);
  value = soc_htonl (ipv4->da);
  sal_memcpy (data + 16, &value, 4);

  return 0;
}

int decode_udp (udp_t *udp, uint8 *data) {
  uint32 value;
  sal_memcpy (&value, data, 4);
  value = soc_ntohl (value);
  udp->sport = (value >> 16) & 0xffff;
  udp->dport = (value >> 0) & 0xffff;

  sal_memcpy (&value, data + 4, 4);
  value = soc_ntohl (value);
  udp->len = (value >> 16) & 0xffff;
  udp->checksum = (value >> 0) & 0xffff;

  udp->entry.length = 8;
  return 0;
}
int encode_udp (udp_t *udp, uint8 *data) {
  sal_memcpy (data, &udp->sport, 2);
  sal_memcpy (data + 2, &udp->dport, 2);
  sal_memcpy (data + 4, &udp->len, 2);
  sal_memcpy (data + 6, &udp->checksum, 2);
  return 0;
}

int decode_tcp (tcp_t *tcp, uint8 *data) {
  uint32 value;
  sal_memcpy (&value, data, 4);
  value = soc_ntohl (value);
  tcp->sport = (value >> 16) & 0xffff;
  tcp->dport = (value >> 0) & 0xffff;

  sal_memcpy (&value, data + 4, 4);
  tcp->seqn = soc_ntohl (value);

  sal_memcpy (&value, data + 8, 4);
  tcp->ackn = soc_ntohl (value);

  sal_memcpy (&value, data + 12, 4);
  value = soc_ntohl (value);
  tcp->dofs = (value >> 28) & 0xf;
  tcp->ecn  = (value >> 22) & 0x7;
  tcp->ctrl = (value >> 16) & 0x3f;
  tcp->wind = (value >>  0) & 0xffff;

  sal_memcpy (&value, data + 16, 4);
  value = soc_ntohl (value);
  tcp->checksum = (value >> 16) & 0xffff;
  tcp->urgp     = (value >>  0) & 0xffff;

  tcp->entry.length = 20;
  return 0;
}
int encode_tcp (tcp_t *tcp, uint8 *data) {
  return 0;
}

int decode_igmp (igmp_t *igmp, uint8 *data) {
  igmp->entry.length = 20;
  return 0;
}
int encode_igmp (igmp_t *igmp, uint8 *data) {
  return 0;
}

int decode_itag (itag_t *itag, uint32 value) {
  itag->ipcp = (value >> 29) & 0x7;
  itag->idei = (value >> 28) & 0x1;
  itag->nca  = (value >> 27) & 0x1;
  itag->isid = (value >>  0) & 0xffffff;
  itag->entry.length = 4;
  return 0;
}
int encode_itag (itag_t *itag, uint8 *data) {
  uint32 value;
  value = (((itag->ipcp & 0x7) >> 29) |
           ((itag->idei & 0x1) >> 28) |
           ((itag->nca  & 0x1) >> 27) |
           ((itag->isid & 0xffffff) >>  0));
  value = soc_htonl (value);
  sal_memcpy (data, &value, 4);
  return 0;
}

int decode_raw_data (raw_data_t *raw_data, uint8 *data) {
  sal_memcpy (raw_data->raw_data, data, raw_data->entry.length);
  return 0;
}
int encode_raw_data (raw_data_t *raw_data, uint8 *data) {
  sal_memcpy (data, raw_data->raw_data, raw_data->entry.length);
  return 0;
} 

int get_value (uint8 *raw_data, int *offset) {
  return ( (raw_data[*offset + 0] << 24) + (raw_data[*offset + 1] << 16) + \
           (raw_data[*offset + 2] <<  8) + (raw_data[*offset + 3] <<  0) );
  return 0;
}

/* ************************
 * allocates header and fills in type
 * ************************/
entry_desc_t* header_alloc (header_type_e hdr_type, uint8 *pkt_data, int length, 
                            int *offset, header_type_e *next_type) {

  unsigned int  value;
  entry_desc_t * hdr_ptr = NULL;

  erh_qe_t   *erh_qe_ptr;
  erh_ss_t   *erh_ss_ptr;
  erh_qess_t *erh_qess_ptr;
  mac_t   *mac_ptr;
  vlan_t  *vlan_ptr;
  stag_t  *stag_ptr;
  llc_t   *llc_ptr;
  snap_t  *snap_ptr;
  etype_t *etype_ptr;
  mpls_t  *mpls_ptr;
  itag_t  *itag_ptr;
  ipv4_t  *ipv4_ptr;
  tcp_t   *tcp_ptr;
  udp_t   *udp_ptr;
  igmp_t  *igmp_ptr;

  raw_data_t *raw_data_ptr;

  value = get_value (pkt_data, offset);

  if (pkt_verbose) {
    printf ("header alloc: %x at offset %d \n", value, *offset);
  }

  if (hdr_type == UNKNOWN) {
    if ((value >> 16) < 1500) {
      hdr_type = LLC;
      printf("llc header type: length= %x\n", value>>16);
    } else {
      switch ((value >> 16) & 0xffff) {
        case 0x8100:
          hdr_type = VLAN;
         break;
        case 0x9100:
        case 0x88A8:
          hdr_type = STAG;
         break;
        case 0x0800:
          hdr_type = ETYPE;
          *next_type = IPV4;
          break;
        case 0x8847:
          hdr_type = ETYPE;
          *next_type = MPLS;
          break;
        case 0x88E7:
          hdr_type = ETYPE;
          *next_type = ITAG;
          break;
        default:
          hdr_type = ETYPE;
          *next_type = RAW_DATA;
        }
     }
  }

  switch (hdr_type) {
    case  ERH_QE:
      erh_qe_ptr = sal_alloc(sizeof(erh_qe_t),"erh_qe");
      erh_qe_ptr->entry.type = ERH_QE;
      decode_erh_qe (erh_qe_ptr, pkt_data + *offset);
      hdr_ptr = (entry_desc_t *)erh_qe_ptr;
      *next_type = MAC;
      break;
    case  ERH_SS:
      erh_ss_ptr = sal_alloc(sizeof(erh_ss_t),"erh_ss");
      erh_ss_ptr->entry.type = ERH_SS;
      decode_erh_ss (erh_ss_ptr, pkt_data + *offset);
      hdr_ptr = (entry_desc_t *)erh_ss_ptr;
      *next_type = MAC;
      break;
    case  ERH_QESS:
      erh_qess_ptr = sal_alloc(sizeof(erh_qess_t),"erh_qess");
      erh_qess_ptr->entry.type = ERH_QESS;
      decode_erh_qess (erh_qess_ptr, pkt_data + *offset);
      hdr_ptr = (entry_desc_t *)erh_qess_ptr;
      *next_type = MAC;
      break;
    case  MAC:
      mac_ptr = sal_alloc(sizeof(mac_t),"mac");
      mac_ptr->entry.type = MAC;
      decode_mac (mac_ptr, pkt_data + *offset);
      hdr_ptr = (entry_desc_t *)mac_ptr;
      *next_type = UNKNOWN;
      break;
    case  VLAN:
      vlan_ptr = sal_alloc(sizeof(vlan_t),"vlan");
      vlan_ptr->entry.type = VLAN;
      decode_vlan (vlan_ptr, value);
      hdr_ptr = (entry_desc_t *)vlan_ptr;
      *next_type = UNKNOWN;
      break;
    case  STAG:
      stag_ptr = sal_alloc(sizeof(stag_t),"stag");
      stag_ptr->entry.type = STAG;
      decode_stag (stag_ptr, value);
      hdr_ptr = (entry_desc_t *)stag_ptr;
      *next_type = UNKNOWN;
      break;
    case  ETYPE:
      etype_ptr = sal_alloc(sizeof(etype_t),"etype");
      etype_ptr->entry.type = ETYPE;
      decode_etype (etype_ptr, value);
      hdr_ptr = (entry_desc_t *)etype_ptr;
      break;
    case  LLC:
      llc_ptr = sal_alloc(sizeof(llc_t),"llc");
      llc_ptr->entry.type = LLC;
      decode_llc (llc_ptr, pkt_data + *offset); 
      hdr_ptr = (entry_desc_t *)llc_ptr;
      if ((llc_ptr->ssap == 0xaa) && (llc_ptr->dsap == 0xaa)) {
        *next_type = SNAP;
      } else {
        *next_type = RAW_DATA;
      }
      break;
    case  SNAP:
      snap_ptr = sal_alloc(sizeof(snap_t),"snap");
      snap_ptr->entry.type = SNAP;
      decode_snap (snap_ptr, pkt_data + *offset);
      hdr_ptr = (entry_desc_t *)snap_ptr;
      *next_type = UNKNOWN;
      break;
    case  IPV4:
      ipv4_ptr = sal_alloc(sizeof(ipv4_t),"ipv4");
      ipv4_ptr->entry.type = IPV4;
      decode_ipv4 (ipv4_ptr, pkt_data + *offset);
      hdr_ptr = (entry_desc_t *)ipv4_ptr;
      if (ipv4_ptr->proto == 0x11) {
        *next_type = UDP;
      } else if (ipv4_ptr->proto == 0x6) {
        *next_type = TCP;
      } else if (ipv4_ptr->proto == 0x2) {
        *next_type = IGMP;
      } else {
        *next_type = RAW_DATA;
      }
      break;
    case  ITAG:
      itag_ptr = sal_alloc(sizeof(itag_t),"itag");
      itag_ptr->entry.type = ITAG;
      decode_itag (itag_ptr, value);
      hdr_ptr = (entry_desc_t *)itag_ptr;
      *next_type = MAC;
      break;
    case  MPLS:
      mpls_ptr = sal_alloc(sizeof(mpls_t),"mpls");
      mpls_ptr->entry.type = MPLS;
      decode_mpls (mpls_ptr, value); 
      hdr_ptr = (entry_desc_t *)mpls_ptr;
      if (mpls_ptr->s == 0) {
        *next_type = MPLS;
      } else {
        *next_type = MAC;
      }
      break;
    case  UDP:
      udp_ptr = sal_alloc(sizeof(udp_t),"udp");
      udp_ptr->entry.type = UDP;
      decode_udp (udp_ptr, pkt_data + *offset);
      hdr_ptr = (entry_desc_t *)udp_ptr;
      *next_type = RAW_DATA;
      break;
    case  TCP:
      tcp_ptr = sal_alloc(sizeof(tcp_t),"tcp");
      tcp_ptr->entry.type = TCP;
      decode_tcp (tcp_ptr, pkt_data + *offset);
      hdr_ptr = (entry_desc_t *)tcp_ptr;
      *next_type = RAW_DATA;
      break;
    case  IGMP:
      igmp_ptr = sal_alloc(sizeof(igmp_t),"igmp");
      igmp_ptr->entry.type = IGMP;
      decode_igmp (igmp_ptr, pkt_data + *offset);
      hdr_ptr = (entry_desc_t *)igmp_ptr;
      *next_type = RAW_DATA;
      break;
    case RAW_DATA:
      raw_data_ptr = sal_alloc(sizeof(raw_data_t),"raw_data");
      raw_data_ptr->entry.type = RAW_DATA;
      raw_data_ptr->entry.next = NULL;
      if (length > *offset) {
        raw_data_ptr->entry.length = length - *offset;
      } else {
        raw_data_ptr->entry.length = 0;
      }
      decode_raw_data (raw_data_ptr, pkt_data + *offset);
      hdr_ptr = (entry_desc_t *)raw_data_ptr;
      break;
    default:
      hdr_ptr = NULL;
      printf("unexpected htype %d\n", hdr_type);
      return (hdr_ptr);
  }

  *offset += hdr_ptr->length;
  return (hdr_ptr);
}

/* ***********************
 * From Byte - convert network byte string to Packet linked list
 * ***********************/
int from_byte (header_type_e start_type, uint8 *pkt_data, int length, sbxpkt_t *return_pkt) {

  int offset=0;
  header_type_e next_type;
  entry_desc_t *hdr_ptr;
  entry_desc_t *next_hdr_ptr;

  /* printf ("from byte %x %x %x %x for %d \n", pkt_data[0], pkt_data[1], pkt_data[2], pkt_data[3], length); */
  /* alloc and fill first_hdr_t  */
  hdr_ptr = header_alloc (start_type, pkt_data, length, &offset, &next_type);

  /* link first_hdr_t to sbxpkt_t  */
  return_pkt->entry.next = hdr_ptr;

  /* printf ("header pointer %x  type %x next %x at offset %d\n", hdr_ptr, hdr_ptr->type, hdr_ptr->next, offset); */

  /* determine next header type or exit  */
  while (offset < length) { 
    /* alloc and fill next_hdr_t */
    next_hdr_ptr = header_alloc (next_type, pkt_data, length, &offset, &next_type);

    hdr_ptr->next = next_hdr_ptr;

    /*determine next heder type or exit */
    hdr_ptr = next_hdr_ptr;
    if (hdr_ptr->type == RAW_DATA) break;
    
    /* printf ("header pointer %x  type %x next %x at offset %d\n", hdr_ptr, hdr_ptr->type, hdr_ptr->next, offset);  */
  }

  if (hdr_ptr->type != RAW_DATA) {
    printf ("unexpected packet length - terminating decoding headers \n");
    hdr_ptr->next = NULL;
  }

  return_pkt->entry.length = offset;

  return 0;
}

/* ***********************
 * To Byte - convert a packet linked list to network byte string
 * ***********************/
int to_byte (sbxpkt_t *packet, uint8 *pkt_data) {
 
  entry_desc_t *hdr_ptr;

  erh_qe_t   *erh_qe_ptr;
  erh_ss_t   *erh_ss_ptr;
  erh_qess_t *erh_qess_ptr;
  mac_t   *mac_ptr;
  vlan_t  *vlan_ptr;
  stag_t  *stag_ptr;
  llc_t   *llc_ptr;
  snap_t  *snap_ptr;
  etype_t *etype_ptr;
  mpls_t  *mpls_ptr;
  itag_t  *itag_ptr;
  ipv4_t  *ipv4_ptr;
  tcp_t   *tcp_ptr;
  udp_t   *udp_ptr;
  igmp_t  *igmp_ptr;

  raw_data_t *raw_data_ptr;

  if (packet->normalize) {
    /* normalize_packet (packet); */
  }

  hdr_ptr = packet->entry.next;

  while (hdr_ptr) {
    switch (hdr_ptr->type) {
      case  ERH_QE:
        erh_qe_ptr = (erh_qe_t *)hdr_ptr;
        encode_erh_qe (erh_qe_ptr, pkt_data);
        pkt_data += 12;
        break;
      case  ERH_SS:
        erh_ss_ptr = (erh_ss_t *)hdr_ptr;
        /* encode_erh_ss_header (erh_ss_ptr); */
        pkt_data += 12;
        break;
      case  ERH_QESS:
        erh_qess_ptr = (erh_qess_t *)hdr_ptr;
        /* encode_erh_qess_header (erh_qess_ptr); */
        pkt_data += 14;
        break;
      case  MAC:
        mac_ptr = (mac_t *)hdr_ptr;
        encode_mac (mac_ptr, pkt_data);
        pkt_data += 12;
        break;
      case  VLAN:
        vlan_ptr = (vlan_t *)hdr_ptr;
        encode_vlan (vlan_ptr, pkt_data);
        pkt_data += 4;
        break;
      case  STAG:
        stag_ptr = (stag_t *)hdr_ptr;
        encode_stag (stag_ptr, pkt_data);
       pkt_data += 4;
        break;
      case  ETYPE:
        etype_ptr = (etype_t *)hdr_ptr;
        encode_etype (etype_ptr, pkt_data);
        pkt_data += 2;
        break;
      case  LLC:
        llc_ptr = (llc_t *)hdr_ptr;
        encode_llc (llc_ptr, pkt_data);
        pkt_data += 5;
        break;
      case  SNAP:
        snap_ptr = (snap_t *)hdr_ptr;
        encode_snap (snap_ptr, pkt_data);
        pkt_data += 3;
        break;
      case  IPV4:
        ipv4_ptr = (ipv4_t *)hdr_ptr;
        encode_ipv4 (ipv4_ptr, pkt_data);
        pkt_data += 20;
        break;
      case  ITAG:
        itag_ptr = (itag_t *)hdr_ptr;
        encode_itag (itag_ptr, pkt_data);
        pkt_data += 4;
        break;
      case  MPLS:
        mpls_ptr = (mpls_t *)hdr_ptr;
        encode_mpls (mpls_ptr, pkt_data);
        pkt_data += 4;
        break;
      case  UDP:
        udp_ptr = (udp_t *)hdr_ptr;
        encode_udp (udp_ptr, pkt_data);
        pkt_data += 8;
        break;
      case  TCP:
        tcp_ptr = (tcp_t *)hdr_ptr;
        encode_tcp (tcp_ptr, pkt_data);
        pkt_data += 20;
        break;
      case  IGMP:
        igmp_ptr = (igmp_t *)hdr_ptr;
        encode_igmp (igmp_ptr, pkt_data);
        break;
      case RAW_DATA:
        raw_data_ptr = (raw_data_t *)hdr_ptr;
        encode_raw_data (raw_data_ptr, pkt_data);
        break;
      default:
       printf("header print: unexpected htype %d\n", hdr_ptr->type);
    }

   hdr_ptr = hdr_ptr->next;
  }
  
 /* printf ("to byte: packet at %x for %d\n", packet, packet->entry.length);  */
  return 0;
}

/* ********************************
 * Allocate and Free Packet
 * ********************************/
sbxpkt_t* sbxpkt_alloc (void) {
  sbxpkt_t *packet;
  packet = sal_alloc(sizeof(sbxpkt_t),"pkt");
  if (packet == NULL) {
    printf ("unable to allocate memory for packet\n");
  } else {
    sal_memset(packet, 0, sizeof(*packet));
  }
  return (packet);
}

int sbxpkt_free (sbxpkt_t *packet) {
  entry_desc_t *cur_hdr;
  entry_desc_t *next_hdr;

  if (packet == NULL) return -1;

  cur_hdr = packet->entry.next;
  sal_free (packet);

  while (cur_hdr) {
   next_hdr = cur_hdr->next;
   sal_free (cur_hdr);
   cur_hdr = next_hdr;
  }
  return 0;
}

int header_print (entry_desc_t *hdr_ptr) {

  erh_qe_t   *erh_qe_ptr;
  erh_ss_t   *erh_ss_ptr;
  erh_qess_t *erh_qess_ptr;
  mac_t   *mac_ptr;
  vlan_t  *vlan_ptr;
  stag_t  *stag_ptr;
  llc_t   *llc_ptr;
  snap_t  *snap_ptr;
  etype_t *etype_ptr;
  mpls_t  *mpls_ptr;
  itag_t  *itag_ptr;
  ipv4_t  *ipv4_ptr;
  tcp_t   *tcp_ptr;
  udp_t   *udp_ptr;
  igmp_t  *igmp_ptr;

  raw_data_t *raw_data_ptr;

  switch (hdr_ptr->type) {
    case  ERH_QE:
      erh_qe_ptr = (erh_qe_t *)hdr_ptr;
      print_erh_qe_header (erh_qe_ptr);
      break;
    case  ERH_SS:
      erh_ss_ptr = (erh_ss_t *)hdr_ptr;
      print_erh_ss_header (erh_ss_ptr);
      break;
    case  ERH_QESS:
      erh_qess_ptr = (erh_qess_t *)hdr_ptr;
      print_erh_qess_header (erh_qess_ptr);
      break;
    case  MAC:
      mac_ptr = (mac_t *)hdr_ptr;
      print_mac_header (mac_ptr);
      break;
    case  VLAN:
      vlan_ptr = (vlan_t *)hdr_ptr;
      print_vlan_header (vlan_ptr);
      break;
    case  STAG:
      stag_ptr = (stag_t *)hdr_ptr;
      print_stag_header (stag_ptr);
      break;
    case  ETYPE:
      etype_ptr = (etype_t *)hdr_ptr;
      print_etype_header (etype_ptr);
      break;
    case  LLC:
      llc_ptr = (llc_t *)hdr_ptr;
      print_llc_header (llc_ptr);
      break;
    case  SNAP:
      snap_ptr = (snap_t *)hdr_ptr;
      print_snap_header (snap_ptr);
      break;
    case  IPV4:
      ipv4_ptr = (ipv4_t *)hdr_ptr;
      print_ipv4_header (ipv4_ptr);
      break;
    case  ITAG:
      itag_ptr = (itag_t *)hdr_ptr;
      print_itag_header (itag_ptr);
      break;
    case  MPLS:
      mpls_ptr = (mpls_t *)hdr_ptr;
      print_mpls_header (mpls_ptr);
      break;
    case  UDP:
      udp_ptr = (udp_t *)hdr_ptr;
      print_udp_header (udp_ptr);
      break;
    case  TCP:
      tcp_ptr = (tcp_t *)hdr_ptr;
      print_tcp_header (tcp_ptr);
      break;
    case  IGMP:
      igmp_ptr = (igmp_t *)hdr_ptr;
      print_igmp_header (igmp_ptr);
      break;
    case RAW_DATA:
      raw_data_ptr = (raw_data_t *)hdr_ptr;
      print_raw_data_header (raw_data_ptr);
      break;
    default:
     printf("header print: unexpected htype %d\n", hdr_ptr->type);
  }
  return 0;
}

int sbxpkt_print (sbxpkt_t *packet) {
  entry_desc_t *cur_hdr;
  entry_desc_t *next_hdr;

  if (packet == NULL) return -1;

  /* printf ("packet at %x - type %x next %x \n", packet, packet->entry.type, packet->entry.next); */
  printf ("Packet Information:  len=%d (0x%x)\n", packet->entry.length, packet->entry.length);

  cur_hdr = packet->entry.next;

  while (cur_hdr) {
   next_hdr = cur_hdr->next;

   /* printf ("header at %x - type %x next %x \n", cur_hdr, cur_hdr->type, cur_hdr->next); */
   header_print (cur_hdr);

   cur_hdr = next_hdr;
  }
  return 0;
}

int debug_print (uint8 * pkt_data, int length) {
  int i;
  int line_len = 16;

  printf ("\n packet dump for length = %d\n", length);

  /* if (length > 128) length = 128;  */

  for (i=0; i<length; i++) { 
    if (i % line_len == 0) {
      if (i !=0) printf("\n");
      printf("%04x  ", i); 
    }
    printf ("%02x ", pkt_data[i]);
  }

  printf ("\n\n");
  return 0;
}

int sbxpkt_compare (sbxpkt_t *tx_pkt, sbxpkt_t *rx_pkt) {
  int i;
  uint8 tx_pkt_data[2000];
  uint8 rx_pkt_data[2000];

  if ((tx_pkt == NULL) || (rx_pkt == NULL)) return -1;

  printf ("Packet Compare: \n");

  if (tx_pkt->entry.length == rx_pkt->entry.length) {
    to_byte (tx_pkt, tx_pkt_data);
    to_byte (rx_pkt, rx_pkt_data);
    for (i=0; i< (tx_pkt->entry.length); i++) {
      if (tx_pkt_data[i] != rx_pkt_data[i]) {
        printf ("Expected Packet: \n");
        sbxpkt_print(tx_pkt);
        debug_print(tx_pkt_data, tx_pkt->entry.length);
        printf ("Received Packet: \n");
        sbxpkt_print(rx_pkt);
        debug_print(rx_pkt_data, rx_pkt->entry.length);
        return -1;
      }
    }
    printf ("okay \n");
    return 0;
  }

  printf ("Expected Packet: \n");
  sbxpkt_print(tx_pkt);
  printf ("Received Packet: \n");
  sbxpkt_print(rx_pkt);

  return -1;
}

int p_build (op_e op, sbxpkt_t *packet, char *char_data) {
  int rc;
  int i;
  int argc;
  char **argv;

  if (packet == NULL) return -1;

  /* allocate array of pointers */
  argv = sal_alloc(MAXFIELD * MAXARGS * sizeof(argv),"argv");

  /* allocate storage for input */
  for (i=0; i<MAXFIELD; i++) {
    argv[i] = sal_alloc(MAXFIELD * sizeof(char),"argv");
  }

  convert_string (char_data, &argc, argv);

  rc = header_build (op, packet, argc, argv);

  for (i=0; i<MAXFIELD; i++) {
    sal_free (argv[i]);
  }
  sal_free (argv);

  return rc;
}

int sbxpkt_create (sbxpkt_t *packet, char *char_data) {
  int rc;

  rc = p_build (CREATE, packet, char_data);

  return rc;
}

int sbxpkt_prepend (sbxpkt_t *packet, char *char_data) {
  int rc;

  rc = p_build (PREPEND, packet, char_data);

  return rc;
}

int sbxpkt_append (sbxpkt_t *packet, char *char_data) {
  int rc;

  rc = p_build (APPEND, packet, char_data);

  return rc;
}

int sbxpkt_test_main(void) {

  sbxpkt_t *pkt;
  sbxpkt_t *i_pkt;
  uint8 pkt_data[2000];

  uint8 eth_data[128] = {0,1,2,3,4,5,0,1,2,3,4,6,0x81,0x00,0,3,
                         0x88,0x47,0xab,0xcd,0xe1,0x40,0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 
                         0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0x08, 0x11, 
                         0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55};
  uint8 eth_data2[128] = {0x00, 0xe0, 0xfc, 0x00, 0x44, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
                     0x08, 0x00, 0x45, 0x00, 0x00, 0x6a, 0x00, 0x00, 0x00, 0x00, 0x40, 0x11, 
                     0x74, 0x7c, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 
                     0x03, 0x04, 0x00, 0x04, 0x01, 0x00, 0x56, 0xf1, 0x39, 0x00, 0x00, 
                     0x11, 0x22, 0x33, 0x44, 0x55, 0x66};

  uint8 erh_data[128] = {0x01,0x23,0,0,0,0,0,0,0,0,0,0,0,1,2,3,4,5,0,1,2,3,4,6,0x81,0x00,0,3,
                         0x88,0x47,0xab,0xcd,0xe1,0x40,1,2,3,4,5,6,7,8,9};

  printf ("main \n");

  printf ("\n*************** test 1 *******************\n");

  pkt = sbxpkt_alloc();

  sbxpkt_create (pkt, "--mac -smac 00:11:22:33:44:55 --stag --vlan -pri 5 -vid 18 --etype -etype 0x09bb --ipv4");

  sbxpkt_print(pkt);

  to_byte (pkt, pkt_data);
  debug_print(pkt_data, pkt->entry.length);

  sbxpkt_append (pkt, "--udp -sport 0x1111 -dport 0x2222");
  sbxpkt_print(pkt);

  to_byte (pkt, pkt_data);
  debug_print(pkt_data, pkt->entry.length);

  sbxpkt_free(pkt);

  printf ("\n*************** test 2 *******************\n");

  pkt = sbxpkt_alloc();

  sbxpkt_create (pkt, "--mac -dmac 00:00:ab:cd:ef:ff --etype -etype 0x8847 --mpls");

  sbxpkt_print(pkt);

  to_byte (pkt, pkt_data);
  debug_print(pkt_data, pkt->entry.length);

  sbxpkt_prepend (pkt, "--mac --vlan -vid 0xabc --etype -etype 0x88e7 --itag -isid 0x12345");

  sbxpkt_print(pkt);

  to_byte (pkt, pkt_data);
  debug_print(pkt_data, pkt->entry.length);

  sbxpkt_free(pkt);

  printf ("\n*************** test 3 *******************\n");

  pkt = sbxpkt_alloc();

  sbxpkt_create (pkt, "--erh_qe --mac --vlan --etype --raw_data -length 100");

  sbxpkt_print(pkt);

  to_byte (pkt, pkt_data);
  debug_print(pkt_data, pkt->entry.length);

  sbxpkt_free(pkt);

  printf ("\n*************** test 4 *******************\n");

  pkt = sbxpkt_alloc();
  pkt->entry.type = PACKET;
  from_byte (MAC, eth_data, 64, pkt);

  i_pkt = sbxpkt_alloc();
  sbxpkt_create (i_pkt, "--mac -dmac 00:01:02:03:04:05 -smac 00:01:02:03:04:06 --vlan -pri 0 -vid 3 \
                    --etype -etype 0x8847 --mpls -label 0xabcde -s 1 -ttl 0x40 \
                    --mac -dmac 00:aa:aa:aa:aa:aa -smac 00:bb:bb:bb:bb:bb --etype -etype 0x811 \
                    --raw_data -length 28 -value 0x55555555 -mode 1");
  sbxpkt_compare (i_pkt, pkt);

  sbxpkt_free(i_pkt);
  sbxpkt_free(pkt);


  printf ("\n*************** test 5 *******************\n");

  pkt = sbxpkt_alloc();
  pkt->entry.type = PACKET;
  from_byte (MAC, eth_data2, 0, pkt);
  from_byte (MAC, eth_data2, 20, pkt);
  from_byte (MAC, eth_data2, 120, pkt);

  sbxpkt_print(pkt);
  sbxpkt_free(pkt);

  printf ("\n*************** test 6 *******************\n");

  pkt = sbxpkt_alloc();
  pkt->entry.type = PACKET;
  from_byte (ERH_QE, erh_data, 64, pkt);

  sbxpkt_print(pkt);
  sbxpkt_free(pkt);

  return 0;
}

#else
int _src_appl_test_sbx_pkt_c_not_empty;
#endif /* BCM_FE2000_SUPPORT */
