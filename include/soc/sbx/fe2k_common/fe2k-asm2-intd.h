/******************************************************************************
 *
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
 * $Id: fe2k-asm2-intd.h 1.4.110.1 Broadcom SDK $
 *
 ******************************************************************************/

/* File: fe2k-asm2-intd.h */

#ifndef FE2K_ASM2_INTD_H__DEFINED
#define FE2K_ASM2_INTD_H__DEFINED

#ifndef ASSERT_VN
#ifndef ASSERT
#ifndef __KERNEL__
#include <stdlib.h>/*for the exit()*/
#endif
#include <assert.h>
#define ASSERT_VN(x) assert(x)
#else                                                                                                        
#define ASSERT_VN(x) ASSERT(x)                                                                               
#endif /*ASSERT*/                                                                                            
#endif /*ASSERT_VN*/

/* temperary for the debugging only */
#include <soc/sbx/fe2k_common/Bsc.h>
#include "fe2k-asm2-ncae.h"

/******************************************************************************
 * fe2k asm2 Interface Dinamic Structure
 *****************************************************************************/

typedef int HwUcode (void *a_pv, unsigned int a_sn, unsigned int a_in, unsigned char *a_iw);

typedef struct fe2kAsm2IntD__ {
  unsigned int 
    m_magic,     /*4 octets long magic number to identify binary form of package*/

    m_tver_maj,  /*tool major version - change of the package format or major functionality*/
    m_tver_min,  /*tool minor version - other changes*/
    m_tver_patch,/*tool patch number*/

    m_uver_maj,  /*ucode major version - change of the package format or major functionality*/
    m_uver_min,  /*ucode minor version*/
    m_uver_patch,/*ucode patch number*/

    m_fdin,      /*flag indicating dinamic allocation*/
    m_snum,      /*number of streams*/
    m_inum,      /*number of instructions*/
    m_onum,      /*number of octets in the machine word*/
    m_ncnum,     /*number of entries in the NCAT table*/
    m_hwid       /*hardware identifier, currently used 1 for caladan 1 and 2 for caladan 2*/
    ;

  char  *m_tver_name  /*string portion of tool version*/
       ,*m_uver_name; /*string portion of ucode version*/
    
  unsigned char m_md5a[16]; /* md5 sum for the  m_uver_name, m_uver_maj,  m_uver_min,  m_uver_patch, m_tmap, m_code */

  fe2kAsm2Ncae
    *m_ncat;/*NCAT table*/
  NcatQae 
    **m_qat; /* array for the access hash accelerator for the NCAT */
  unsigned int m_qal; /* number of elements of the quick access table */

  unsigned char
    *m_tmap, /*task map*/
    *m_code; /*code*/

  HwUcode *m_fPut, *m_fGet; /* call back functions to put machine word into hardware and get it from there */
  void *m_pv; /* pointer passed by consumer to call-back hardware acess functions via functions of putNc family*/

} fe2kAsm2IntD;

/******************************************************************************
 * functions declarations for the class fe2kAsm2IntD
 *****************************************************************************/

/* serialize all interface data into the buffer */
int fe2kAsm2IntD__writeBsc(fe2kAsm2IntD *a_p, Bsc *a_pb);

/* deserialize all interface data from the buffer */
int fe2kAsm2IntD__readBsc(fe2kAsm2IntD *a_p, Bsc *a_pb);

/* initialize named constants table access accelerator */
int fe2kAsm2IndD__initQat(fe2kAsm2IntD *a_p);

/* recover the named constant values from hardware, used during a warm boot of a device */
int fe2kAsm2IndD__recoverNcv(fe2kAsm2IntD *a_p);

/* calculate hash value of the string for the hash table size m_qal */
int fe2kAsm2IndD__calcHash(fe2kAsm2IntD *a_p, char *a_s, unsigned int *a_h);

/* update one entry in the named constants table */
int fe2kAsm2IntD__updateNcEntry(fe2kAsm2IntD *a_p, fe2kAsm2Ncae *a_e, unsigned int *a_nc); 

/* get Named Constant value */
int fe2kAsm2IntD__getNc(fe2kAsm2IntD *a_p, char *a_n, unsigned int *a_nc);

/* put Named Constant value */
int fe2kAsm2IntD__putNc(fe2kAsm2IntD *a_p, char *a_n, unsigned int *a_nc);

/* initialize interface structure to be empty */
int fe2kAsm2IntD__init(fe2kAsm2IntD *a_p);

/* initialize interface structure to be empty with initialization of pointers to the hw access functions*/
int fe2kAsm2IntD__initF(fe2kAsm2IntD *a_p, HwUcode *a_fPut, HwUcode *a_fGet, void *a_pv);

/* destroy interface structure to be empty */
int fe2kAsm2IntD__destroy(fe2kAsm2IntD *a_p);

/* calculate length of the buffer which could be used to serialize all interface data */
int fe2kAsm2IntD__lenBuf(fe2kAsm2IntD *a_p, unsigned int *a_l);

/* compare 2 interface structure and return rezult in a_e */
int fe2kAsm2IntD__equ(fe2kAsm2IntD *a_a, fe2kAsm2IntD *a_b, int *a_e);

/* copy interface structure a_s to interface structure a_d with dynamic buffers */
int fe2kAsm2IndD__copyD(fe2kAsm2IntD *a_d, fe2kAsm2IntD *a_s);

/* serialize all interface data into the buffer */
int fe2kAsm2IntD__writeBuf(fe2kAsm2IntD *a_p, unsigned char *a_b, unsigned int a_l);

/* deserialize all interface data from the buffer */
int fe2kAsm2IntD__readBuf(fe2kAsm2IntD *a_p, unsigned char *a_b, unsigned int a_l);

/* get pointer to the first octet of the machine word */
int fe2kAsm2IntD__getWordPtr(fe2kAsm2IntD *a_p, unsigned int a_sn, unsigned int a_in, unsigned char **a_wp);

/* calculate md5a ( for all significunt data fields and arrays ) */
int fe2kAsm2IndD__md5a (fe2kAsm2IntD *a_p, unsigned char a_d[16]);

#ifdef PRINTDIAG_VN
int fe2kAsm2IntD__printf(fe2kAsm2IntD *a_p);
#endif /*PRINTDIAG_VN*/


#endif /*FE2K_ASM2_INTD_H__DEFINED*/
