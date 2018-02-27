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
 * $Id: fe2k-asm2-ncae.c 1.8.112.2 Broadcom SDK $
 *
 * fe2k-asm2-ncae.c
 *****************************************************************************/

#include <soc/sbx/fe2k_common/fe2k-asm2-ncae.h>
#include <soc/sbx/fe2k_common/Bsc.h>

/******************************************************************************
 * functions definitions for the class fe2kAsm2Ncae
 *****************************************************************************/
/* initialize interface structure to be empty */
int fe2kAsm2Ncae__init(fe2kAsm2Ncae *a_p)
{
  if (a_p == NULL) return 1;

  a_p->m_name = 0; /*name associated with the value*/
  a_p->m_va   = 0; /*value of the constant*/
  a_p->m_fc   = 0; /*flag which indicate if value is in the bytecode*/
  a_p->m_fdin = 0;
    
  /*corresponded bytecode parameters, valid only if m_fc is not 0*/
  a_p->m_sn = 0;  /*stream number*/
  a_p->m_in = 0;  /*instruction number*/ 
  a_p->m_bo = 0;  /*offset of the sub-field in the machine word (bits)*/
  a_p->m_bl = 0;  /*length of the sub-field in the machine word (bits)*/

  return 0;
}

/* destroy interface structure to be empty */
int fe2kAsm2Ncae__destroy(fe2kAsm2Ncae *a_p)
{
  if (a_p == NULL) return 1;

  if(a_p->m_fdin) FREE_VN((void*)a_p->m_name); /* if string is dinamically allocated, free the memory*/
  if (fe2kAsm2Ncae__init(a_p)) return 2;

  return 0;
}

/* compare 2 structure and return rezult in a_e; do not check m_fdin - flag indicating dinamic allocation, we do not save it in the buffer / file or read it from there */
int fe2kAsm2Ncae__equ(fe2kAsm2Ncae *a_a, fe2kAsm2Ncae *a_b, int *a_e)
{
  if (a_a == NULL) return 1; 
  if (a_b == NULL) return 2;
  if (a_e == NULL) return 3;
  
  if((a_a->m_name==0) || (a_b->m_name==0)){
     return 4; /* non initialized name error */
  }

  *a_e = (int) (
		STRNCMP_VN (a_a->m_name, a_b->m_name, STRMAX_VN)==0
		&& a_a->m_fc == a_b->m_fc 
		&& a_a->m_va == a_b->m_va 
		&& a_a->m_sn == a_b->m_sn 
		&& a_a->m_in == a_b->m_in 
		&& a_a->m_bo == a_b->m_bo 
		&& a_a->m_bl == a_b->m_bl
		); 

  return 0;
}


int fe2kAsm2Ncae__writeBsc(fe2kAsm2Ncae *a_p, Bsc *a_pb)
{
  unsigned int len=0;

  if (a_p == NULL) return 1;
  if (a_pb == NULL) return 2;
  if (a_p->m_name == NULL) return 3;

  Bsc__putUnsignedB (a_pb, &a_p->m_fc, 1);   /*flag which indicate if value is in the bytecode*/

  len = STRLEN_VN(a_p->m_name);         /*calculate length of the string*/
  if (len > STRMAX_VN) return 4;
  Bsc__putUnsignedB(a_pb, &len, 7);         /*length of the string*/
  Bsc__putOctetStr(a_pb, (unsigned char*)a_p->m_name, len); /*name associated with the value, length limit is 128*/

  Bsc__putUnsignedB   (a_pb, &a_p->m_va, 32);  /*value of the constant*/

  if(a_p->m_fc) {
    Bsc__putUnsignedB (a_pb, &a_p->m_sn, 4);  /*stream number*/
    Bsc__putUnsignedB (a_pb, &a_p->m_in, 12); /*instruction number*/
    Bsc__putUnsignedB (a_pb, &a_p->m_bo, 8);  /*offset of the sub-field in the machine word (bits)*/
    Bsc__putUnsignedB (a_pb, &a_p->m_bl, 8); /*length of the sub-field in the machine word (bits)*/
  }

  return 0;
}

int fe2kAsm2Ncae__readBsc(fe2kAsm2Ncae *a_p, Bsc *a_pb)
{
  unsigned int len=0;

  if (a_p == NULL) return 1;
  if (a_pb == NULL) return 2;

  a_p->m_fdin = TRUE_VN;              /* set flag indicated that string is dinamically allocated*/
  Bsc__getUnsignedB (a_pb, &a_p->m_fc, 1);  /*flag which indicate if value is in the bytecode*/

  Bsc__getUnsignedB (a_pb, &len, 7);         /*length of the string*/
  if (len > STRMAX_VN) return 3;
  a_p->m_name = (char*)MALLOC_VN(len+1, "readBsc"); /* allocate buffer for the string */
  if (a_p->m_name == NULL) return 4;
  /*MEMSET_VN (a_p->m_name, 0, len+1);*/
  Bsc__getOctetStr(a_pb, (unsigned char*)a_p->m_name, len); /*name associated with the value, length limit is 128*/
  a_p->m_name[len] = 0;

  Bsc__getUnsignedB (a_pb, &a_p->m_va, 32);         /*value of the constant*/

  /*corresponded bytecode parameters, valid only if m_fc is not 0*/
  if(a_p->m_fc) {
    Bsc__getUnsignedB (a_pb, &a_p->m_sn, 4);  /*stream number*/
    Bsc__getUnsignedB (a_pb, &a_p->m_in, 12); /*instruction number*/ 
    Bsc__getUnsignedB (a_pb, &a_p->m_bo, 8);  /*offset of the sub-field in the machine word (bits)*/
    Bsc__getUnsignedB (a_pb, &a_p->m_bl, 8); /*length of the sub-field in the machine word (bits)*/
  } else {
    a_p->m_sn = 0;
    a_p->m_in = 0;
    a_p->m_bo = 0;
    a_p->m_bl = 0;
  }

  return 0;
}

/* copy NCA structure a_s to NCA structure a_d with dynamic buffers */
int fe2kAsm2Ncae__copyD(fe2kAsm2Ncae *a_d, fe2kAsm2Ncae * a_s)
{
  unsigned int len=0;

  if (a_d == NULL) return 1;
  if (a_s == NULL) return 2;

  if(fe2kAsm2Ncae__init(a_d)) return 3;

  /* allocate and copy name string */
  len = STRLEN_VN(a_s->m_name);
  if (len > STRMAX_VN) return 4;
  a_d->m_name = (char*)MALLOC_VN(len+1, "copyD"); /* allocate buffer for the string */
  if (a_d->m_name == NULL) return 5;
  STRNCPY_VN(a_d->m_name, a_s->m_name, len+1);
  
  a_d->m_fdin  = TRUE_VN;
  a_d->m_fc = a_s->m_fc;
  a_d->m_va = a_s->m_va;

  /*corresponded bytecode parameters, valid only if m_fc is not 0*/
  if(a_d->m_fc) {
    a_d->m_sn = a_s->m_sn;  /*stream number*/
    a_d->m_in = a_s->m_in; /*instruction number*/ 
    a_d->m_bo = a_s->m_bo;  /*offset of the sub-field in the machine word (bits)*/
    a_d->m_bl = a_s->m_bl; /*length of the sub-field in the machine word (bits)*/
  } else {
    a_d->m_sn = 0;
    a_d->m_in = 0;
    a_d->m_bo = 0;
    a_d->m_bl = 0;
  }

  return 0;
}

/******************************************************************************
 * functions definitions for the class NcatQae 
 *****************************************************************************/
int NcatQae__init (NcatQae *a_p) {
  if (a_p == NULL) return 1;

  a_p->m_ind = 0;
  a_p->m_right = NULL;
  a_p->m_down = NULL;

  return 0;
}

int NcatQae__destroy (NcatQae *a_p)
{
  if (a_p == NULL) return 1;

  if (a_p->m_right) {NcatQae__destroy(a_p->m_right); FREE_VN(a_p->m_right);} 
  if (a_p->m_down)  {NcatQae__destroy(a_p->m_down);  FREE_VN(a_p->m_down); }

  if (NcatQae__init(a_p)) return 2;

  return 0;
}

/******************************************************************************
 ******************************************************************************
 *****************************************************************************/

/******************************************************************************
 * classes for the new, quick Named Constants Access Interface 
 * supporting strings and arrays
 *****************************************************************************/

/******************************************************************************
 * utility functions definitions 
 *****************************************************************************/

/* calculate hash value of the string with the bit size a_bs from 1 to 16*/
int fe2kAsm2__hash    (char *a_s, unsigned int *a_h, unsigned int a_bl)
{
  unsigned char rand[2][256] = {
    {
      1,14,110,25,97,174,132,119,138,170,125,118,27,233,140,51,87,197,177,107,234,169,56,68,30,7,173,73,188,40,36,65,49,213,104,190,57,211,148,223,48,115,15,2,
      67,186,210,28,12,181,103,70,22,58,75,78,183,167,238,157,124,147,172,144,176,161,141,86,60,66,128,83,156,241,79,46,168,198,41,254,178,85,253,237,250,154,
      133,88,35,206,95,116,252,192,54,221,102,218,255,240,82,106,158,201,61,3,89,9,42,155,159,93,166,80,50,34,175,195,100,99,26,150,16,145,4,33,8,189,121,64,
      77,72,208,245,130,122,143,55,105,134,29,164,185,194,193,239,101,242,5,171,126,11,74,59,137,228,108,191,232,139,6,24,81,20,127,17,91,92,251,151,225,207,21,
      98,113,112,84,226,18,214,199,187,13,32,94,220,224,212,247,204,196,43,249,236,45,244,111,182,153,136,129,90,217,202,19,165,231,71,230,142,96,227,62,179,246,
      114,162,53,160,215,205,180,47,109,44,38,31,149,135,0,216,52,63,23,37,69,39,117,146,184,163,200,222,235,248,243,219,10,152,131,123,229,203,76,120,209
    },
    {
      194,193,239,101,242,5,171,126,11,74,59,137,228,108,191,232,139,6,24,81,253,237,250,154,133,88,35,206,95,116,252,192,54,221,102,218,255,240,82,222,235,248,
      243,219,10,152,131,123,229,203,76,120,209,115,15,2,67,186,210,28,12,181,103,70,22,58,75,78,183,167,238,157,124,147,106,158,201,61,3,89,9,42,155,159,93,166,
      80,50,34,175,195,100,99,26,150,1,14,110,25,97,174,132,119,138,170,125,118,27,233,140,51,87,197,177,107,16,145,4,33,8,189,121,64,77,72,208,245,130,122,143,
      55,105,134,29,164,185,20,127,17,91,92,251,151,225,207,21,98,113,112,84,226,18,214,199,187,13,234,169,56,68,30,7,173,73,188,40,36,65,49,213,104,190,57,211,
      148,223,48,32,94,220,224,212,247,204,196,43,249,236,45,244,111,182,153,136,129,90,217,202,19,165,231,71,230,142,96,227,62,179,246,114,162,53,160,215,205,
      180,47,109,44,38,31,149,135,0,216,52,63,23,37,69,39,117,146,184,163,200,172,144,176,161,141,86,60,66,128,83,156,241,79,46,168,198,41,254,178,85
    }
  };
  unsigned int mask[17] = {0,1,3,7,0xf,0x1f,0x3f,0x7f,0xff,0x1ff,0x3ff,0x7ff,0xfff,0x1fff,0x3fff,0x7fff,0xffff};

  if(a_s  == NULL) return 1;
  if(a_h  == NULL) return 2;
  if(a_bl == 0 || a_bl > 16) return 3;
  
  /* currently hash calculated for the maximum value of table elements = 0x1000 = 4096 */
  for (*a_h=0; *a_s; a_s++) *a_h ^= (rand[0][(unsigned char)*a_s] | rand[1][(unsigned char)*a_s]<<8);

  *a_h &= mask[a_bl];

  return 0;
}

/******************************************************************************
 * functions definitions for the class fe2kAsm2Ncur (ucode reference) 
 *****************************************************************************/
/* default initialization */
int fe2kAsm2Ncur__init    (fe2kAsm2Ncur *a_p)
{
  if(a_p == NULL) return 1;
 
  a_p->m_sn = 0;
  a_p->m_in = 0;
  a_p->m_bo = 0;
  a_p->m_bl = 0;
  a_p->m_ur = NULL;

  return 0;
}

/* allocate and default initialize structure */
int fe2kAsm2Ncur__create  (fe2kAsm2Ncur **a_h) /* default creation */
{
  if (a_h == NULL) return 1;

  *a_h = (fe2kAsm2Ncur*) MALLOC_VN (sizeof (fe2kAsm2Ncur), "fe2kAsm2Ncur__create");
  if (*a_h == 0) return 2;
  if (fe2kAsm2Ncur__init(*a_h)) return 3;

  return 0;
}
/* default deinitialization, no linked list deinitialized */ 
int fe2kAsm2Ncur__destroy (fe2kAsm2Ncur *a_p)
{
  if(a_p == NULL) return 1;
  if (fe2kAsm2Ncur__init(a_p) != 0) return 2;
  return 0;
}


/* default deinitialization including attached linked list 
   this function uses reqursive algorithm because of simplicity
*/ 
int fe2kAsm2Ncur__destroyL (fe2kAsm2Ncur *a_p)
{
  if(a_p == NULL) return 1;
 
  if (a_p->m_ur != NULL) {
    if (fe2kAsm2Ncur__destroyL(a_p->m_ur) != 0) return 2;
    FREE_VN(a_p->m_ur);
  }

  if (fe2kAsm2Ncur__destroy(a_p) != 0) return 3;

  return 0;
}


/* copy one object to another except next pointer */
int fe2kAsm2Ncur__copy    (fe2kAsm2Ncur *a_p, fe2kAsm2Ncur *a_s)
{
  if(a_p == NULL) return 1; 
  if(a_s == NULL) return 2;
 
  a_p->m_sn = a_s->m_sn;
  a_p->m_in = a_s->m_in;
  a_p->m_bo = a_s->m_bo;
  a_p->m_bl = a_s->m_bl;
  /*a_p->m_ur = NULL;*/

  return 0;
}

/* compare two objects excluding reference list*/ 
int fe2kAsm2Ncur__equ     (fe2kAsm2Ncur *a_p, fe2kAsm2Ncur *a_s, unsigned int *a_e)
{
  if(a_p == NULL) return 1; 
  if(a_s == NULL) return 2; 
  if(a_e == NULL) return 3;

  *a_e = (unsigned int)
    (   
     a_p->m_sn == a_s->m_sn 
     && a_p->m_in == a_s->m_in
     && a_p->m_bo == a_s->m_bo
     && a_p->m_bl == a_s->m_bl
    );

  return 0;
}

/* compare two objects including referemce list*/ 
int fe2kAsm2Ncur__equL     (fe2kAsm2Ncur *a_p, fe2kAsm2Ncur *a_s, unsigned int *a_e)
{
  if(a_p == NULL) return 1; 
  if(a_s == NULL) return 2; 
  if(a_e == NULL) return 3;
  
  if (fe2kAsm2Ncur__equ(a_p, a_s, a_e)) return 4;
  if (*a_e == 0) return 0;

  *a_e = !((a_p->m_ur==0)^(a_s->m_ur==0));

  if (a_s->m_ur)
    if (fe2kAsm2Ncur__equL(a_p->m_ur, a_s->m_ur, a_e)) return 5;

  return 0;
}


/* write (serialize) object to the Bsc bit stream excluding referencies list*/ 
int fe2kAsm2Ncur__writeBsc(fe2kAsm2Ncur *a_p, Bsc *a_bs)
{
  if(a_p  == NULL) return 1; 
  if(a_bs == NULL) return 2; 

  Bsc__putUnsignedB (a_bs, &a_p->m_sn, 4);  /*stream number*/
  Bsc__putUnsignedB (a_bs, &a_p->m_in, 12); /*instruction number*/
  Bsc__putUnsignedB (a_bs, &a_p->m_bo, 8);  /*offset of the sub-field in the machine word (bits)*/
  Bsc__putUnsignedB (a_bs, &a_p->m_bl, 8);  /*length of the sub-field in the machine word (bits)*/

  return 0;
}

/* write (serialize) object to the Bsc bit stream including referencies list*/ 
int fe2kAsm2Ncur__writeBscL(fe2kAsm2Ncur *a_p, Bsc *a_bs)
{
  unsigned int len = 0;

  if(a_p  == NULL) return 1; 
  if(a_bs == NULL) return 2;

  if (fe2kAsm2Ncur__writeBsc(a_p, a_bs)) return 3;

  /* write ucode referencies list */
  len = a_p->m_ur? 1: 0;
  Bsc__putUnsignedB (a_bs, &len, 8);/*identifier of the ucode reference*/
  if (a_p->m_ur) if (fe2kAsm2Ncur__writeBscL(a_p->m_ur, a_bs)) return 4;

  return 0;
}

/* read (deserialize) object from Bsc stream excluding reverencies list*/ 
int fe2kAsm2Ncur__readBsc (fe2kAsm2Ncur *a_p, Bsc *a_bs)
{
  if(a_p  == NULL) return 1; 
  if(a_bs == NULL) return 2;
 
  Bsc__getUnsignedB (a_bs, &a_p->m_sn, 4);  /*stream number*/
  Bsc__getUnsignedB (a_bs, &a_p->m_in, 12); /*instruction number*/
  Bsc__getUnsignedB (a_bs, &a_p->m_bo, 8);  /*offset of the sub-field in the machine word (bits)*/
  Bsc__getUnsignedB (a_bs, &a_p->m_bl, 8); /*length of the sub-field in the machine word (bits)*/

  return 0;
}

/* read (deserialize) object from Bsc stream including reverencies list*/ 
int fe2kAsm2Ncur__readBscL (fe2kAsm2Ncur *a_p, Bsc *a_bs)
{
  unsigned int len = 0;

  if(a_p  == NULL) return 1; 
  if(a_bs == NULL) return 2;

  fe2kAsm2Ncur__readBsc(a_p, a_bs);

  /* read ucode referencies list */
  Bsc__getUnsignedB (a_bs, &len, 8);/*identifier of the ucode reference*/
  if (len) {
    a_p->m_ur = (fe2kAsm2Ncur*)MALLOC_VN(sizeof (fe2kAsm2Ncur), "fe2kAsm2Ncur__readBscL");
    if (a_p->m_ur == NULL) return 3;
    if (fe2kAsm2Ncur__init(a_p->m_ur)) return 4;
    if (fe2kAsm2Ncur__readBscL(a_p->m_ur, a_bs)) return 5;
  }

  return 0;
}

/* calc buf len for the class serializing (excluding ref list) */
int fe2kAsm2Ncur__lenBuf(fe2kAsm2Ncur *a_p, unsigned int *a_l)
{
  Bsc bsc;

  if (a_p == NULL) return 1;
  if (a_l == NULL) return 2;

  Bsc__initFromBuf(&bsc, 0, 10000000); /* zero pointer to the buf will set nc flag in bsc*/
  if (fe2kAsm2Ncur__writeBsc(a_p, &bsc)) return 3;
  *a_l = Bsc__numProO(&bsc);
  
  return 0;
}

/* calc buf len for the class serializing (including ref list) */
int fe2kAsm2Ncur__lenBufL(fe2kAsm2Ncur *a_p, unsigned int *a_l)
{
  Bsc bsc;

  if (a_p == NULL) return 1;
  if (a_l == NULL) return 2;

  Bsc__initFromBuf(&bsc, 0, 10000000); /* zero pointer to the buf will set nc flag in bsc*/
  if (fe2kAsm2Ncur__writeBscL(a_p, &bsc)) return 3;
  *a_l = Bsc__numProO(&bsc);
  
  return 0;
}


/* print diagnostic info about object as ascii string into the bit stream */
#ifdef PRINTDIAG_VN 
#include <stdio.h>
int fe2kAsm2Ncur__info    (fe2kAsm2Ncur *a_p)
{
  if(a_p == NULL) return 1;
  printf ("{%d:%d:%d:%d->%p}", a_p->m_sn, a_p->m_in, a_p->m_bo, a_p->m_bl, a_p->m_ur);
  return 0;
}

int fe2kAsm2Ncur__infoL    (fe2kAsm2Ncur *a_p)
{
  if(a_p == NULL) return 1;
  if (fe2kAsm2Ncur__info(a_p)) return 1;
  if (a_p->m_ur) {
    printf ("->");
    fe2kAsm2Ncur__infoL(a_p->m_ur);
  }
  
  return 0;
}

#endif /*PRINTDIAG_VN*/ 

/* initialize all data holding members of the object*/ 
int fe2kAsm2Ncur__initVal (fe2kAsm2Ncur *a_p, unsigned int a_sn, unsigned int a_in, unsigned int a_bo, unsigned int a_bl)
{
  /* validate arguments */
  if(a_p  == NULL) return 1; 
  if(a_sn > 32)    return 2; 
  if(a_in > 10000) return 3;
  if(a_bo > 255)   return 4;
  if(a_bl > 256)   return 5;

  /* initialize members */
  a_p->m_sn = a_sn;
  a_p->m_in = a_in;
  a_p->m_bo = a_bo;
  a_p->m_bl = a_bl;
  /*a_p->m_ur = NULL;*/

  return 0;
}

/* link a_n to the object */ 
int fe2kAsm2Ncur__link    (fe2kAsm2Ncur *a_p, fe2kAsm2Ncur *a_n)
{
  if(a_p == NULL) return 1; 
  if(a_n == NULL) return 2;

  a_p->m_ur = a_n;

  return 0;
} 

/******************************************************************************
 * functions definitions for the class fe2kAsm2Ncer (value reference) 
 *****************************************************************************/
/* default initialization */
int fe2kAsm2Ncer__init    (fe2kAsm2Ncer *a_p)
{
  if(a_p  == NULL) return 1;

  /* expression related members */
  a_p->m_o  = fe2kAsm2Ncoe__NUM;
  a_p->m_d  = NULL;
  a_p->m_r  = NULL;
  a_p->m_ref= NULL;

  a_p->m_nm = NULL;
  a_p->m_iv = 0;
  a_p->m_sv = NULL;
  a_p->m_ur = NULL;
  a_p->m_ur = NULL;

  return 0;
}
  
/* default deinitialization; reqursive algorithm used only for the destructur */ 
int fe2kAsm2Ncer__destroy (fe2kAsm2Ncer *a_p)
{
  if (a_p  == NULL) return 1;

  if (a_p->m_r) {if (fe2kAsm2Ncer__destroy(a_p->m_r)) return 2;FREE_VN(a_p->m_r); }  
  if (a_p->m_d) {if (fe2kAsm2Ncer__destroy(a_p->m_d)) return 3;FREE_VN(a_p->m_d); }  
  if (a_p->m_ur){if (fe2kAsm2Ncer__destroy(a_p->m_pn))return 5;FREE_VN (a_p->m_pn);}
  if (a_p->m_ur){if (fe2kAsm2Ncur__destroy(a_p->m_ur))return 4;FREE_VN (a_p->m_ur);}
  if (a_p->m_nm) FREE_VN (a_p->m_nm);
  if (a_p->m_sv) FREE_VN (a_p->m_sv);

  if (fe2kAsm2Ncer__init(a_p)) return 6;

  return 0;
}
  
/* copy one object to another except m_ref */ 
int fe2kAsm2Ncer__copy (fe2kAsm2Ncer *a_p, fe2kAsm2Ncer *a_s)
{
  fe2kAsm2Ncur **dh = 0, *sp = 0;
  unsigned int len = 0;

  if(a_p  == NULL) return 1;
  if(a_s  == NULL) return 2;

  /* expression operation code */
  a_p->m_o = a_s->m_o;

  /* down expression branch, use reqursion */
  if (a_s->m_d) { 
    a_p->m_d = (fe2kAsm2Ncer*)MALLOC_VN (sizeof (fe2kAsm2Ncer), "fe2kAsm2Ncer__copy");
    if (a_p->m_d == NULL ) return 3;
    if (fe2kAsm2Ncer__init(a_p->m_d)) return 4;
    if (fe2kAsm2Ncer__copy(a_p->m_d, a_s->m_d)) return 5;
  }

  /* right expression branch, use reqursion */
  if (a_s->m_r) { 
    a_p->m_r = (fe2kAsm2Ncer*)MALLOC_VN (sizeof (fe2kAsm2Ncer), "fe2kAsm2Ncer__copy");
    if (a_p->m_r == NULL ) return 6;
    if (fe2kAsm2Ncer__init(a_p->m_r)) return 7;
    if (fe2kAsm2Ncer__copy(a_p->m_r, a_s->m_r)) return 8;
  }

  /* name str */
  if (a_s->m_nm == NULL) return 9;
  len = STRLEN_VN(a_s->m_nm);
  if (len > STRMAX_VN) return 10;
  a_p->m_nm = MALLOC_VN(len+1, "fe2kAsm2Ncer__copy");
  if (a_p->m_nm == NULL) return 11;
  STRNCPY_VN(a_p->m_nm, a_s->m_nm, len+1);

  /* string value */
  if (a_s->m_sv == NULL) return 12;
  len = STRLEN_VN(a_s->m_sv);
  if (len > STRMAX_VN) return 13;
  a_p->m_sv = MALLOC_VN(len+1, "fe2kAsm2Ncer__copy");
  if (a_p->m_nm == NULL) return 14;
  STRNCPY_VN(a_p->m_sv, a_s->m_sv, len+1);

  /* integer value */
  if (a_p->m_iv != 0) return 15;
  a_p->m_iv = a_s->m_iv;

  /* copy ucode referencies linked list */
  if(a_p->m_ur != NULL) return 16;
  for (dh = &a_p->m_ur, sp = a_s->m_ur; sp != NULL; sp = sp->m_ur, dh = &((*dh)->m_ur)) {
    *dh = MALLOC_VN( sizeof (fe2kAsm2Ncur), "fe2kAsm2Ncer__copy" );
    if (*dh == NULL) return 12;
    if (fe2kAsm2Ncur__init(*dh)) return 17;
    if (fe2kAsm2Ncur__copy(*dh, sp)) return 18;
  }

  if (a_p->m_ur != NULL) return 19;

  return 0;
}
  
/* compare two objects, work even for empty except m_ref*/ 
int fe2kAsm2Ncer__equ (fe2kAsm2Ncer *a_p, fe2kAsm2Ncer *a_s, unsigned int *a_e)
{
  fe2kAsm2Ncur *p=0, *s=0;

  if(a_p == NULL) return 1;
  if(a_s == NULL) return 2;
  if(a_e == NULL) return 3;

  /* expression operation */
  *a_e = (unsigned int)(a_p->m_o == a_s->m_o);

  /* compare down branch using recursion*/
  if (a_p->m_d && a_s->m_d) {if (fe2kAsm2Ncer__equ(a_p->m_d, a_s->m_d, a_e)) return 4;}
  else *a_e = (a_p->m_d == NULL)^(a_s->m_d == NULL);
  if (*a_e == 0) return 0;
  
  /* compare right branch using recursion*/
  if (a_p->m_r && a_s->m_r) {if (fe2kAsm2Ncer__equ(a_p->m_r, a_s->m_r, a_e)) return 4;}
  else *a_e = (a_p->m_r == NULL)^(a_s->m_r == NULL);
  if (*a_e == 0) return 0;

  /* name */
  if (a_p->m_nm == NULL) *a_e = (unsigned int) (a_p->m_nm == a_s->m_nm);
  else *a_e = (unsigned int) !STRNCMP_VN(a_p->m_nm, a_s->m_nm, STRMAX_VN);
  if (*a_e == 0) return 0;

  /* integer value */
  *a_e = (unsigned int)(a_p->m_iv == a_s->m_iv);
  if (*a_e == 0) return 0;

  /* string value */
  if (a_p->m_sv == NULL) *a_e = (unsigned int) (a_p->m_sv == a_s->m_sv);
  else *a_e = (unsigned int) !STRNCMP_VN(a_p->m_sv, a_s->m_sv, STRMAX_VN);
  if (*a_e == 0) return 0;

  /* ucode referencies list */
  if (a_p->m_ur == NULL) *a_e = (unsigned int) (a_p->m_ur == a_s->m_ur);
  else for (p=a_p->m_ur, s=a_s->m_ur; p!=NULL; p=p->m_ur, s=s->m_ur) {
    if(fe2kAsm2Ncur__equ(p, s, a_e)) return 3;
    if (*a_e == 0) break;
  }

  return 0;
}
  
/* write (serialize) object to the Bsc bit stream */ 
int fe2kAsm2Ncer__writeBsc(fe2kAsm2Ncer *a_p, Bsc *a_bs)
{
  unsigned int len = 0;

  if(a_p  == NULL) return 1;
  if(a_bs == NULL) return 2;

  /* write expression operation */
  Bsc__putUnsignedB   (a_bs, (unsigned*)&a_p->m_o, 8);  /*expression operation code*/

  /* write integer value */
  Bsc__putUnsignedB   (a_bs, &a_p->m_iv, 32);  /*value of the constant*/

  /* write name  string*/
  if(a_p->m_nm == NULL) return 3;
  len = STRLEN_VN(a_p->m_nm);
  if(len == 0) return 4;
  if (len > STRMAX_VN) return 5;
  Bsc__putUnsignedB   (a_bs, &len, 8);/*length of the string*/
  Bsc__putOctetStr(a_bs, (unsigned char*)a_p->m_nm, len); /*name associated with the value, length limit is 128*/

  /* write string value */
  if(a_p->m_sv == NULL) return 6;
  len = STRLEN_VN(a_p->m_sv);
  if (len > STRMAX_VN) return 7;
  Bsc__putUnsignedB   (a_bs, &len, 8);/*length of the string*/
  if (len != 0) Bsc__putOctetStr(a_bs, (unsigned char*)a_p->m_sv, len); /*string value*/

  /* write ucode referencies list */
  len = a_p->m_ur? 1: 0;
  Bsc__putUnsignedB (a_bs, &len, 8);/*identifier of the ucode reference*/
  if (a_p->m_ur) if (fe2kAsm2Ncur__writeBsc(a_p->m_ur, a_bs)) return 8;
    
  /* write right list */
  len = a_p->m_r? 1 : 0;
  Bsc__putUnsignedB (a_bs, &len, 8);/*identifier of the down reference*/
  if (a_p->m_r) if (fe2kAsm2Ncer__writeBsc(a_p->m_r, a_bs)) return 9;

  /* write down list */
  len = a_p->m_d? 1 : 0;
  Bsc__putUnsignedB (a_bs, &len, 8);/*identifier of the down reference*/
  if (a_p->m_d) if (fe2kAsm2Ncer__writeBsc(a_p->m_d, a_bs)) return 10;

  return 0;
}
  
/* read (deserialize) object from Bsc stream */ 
int fe2kAsm2Ncer__readBsc (fe2kAsm2Ncer *a_p, Bsc *a_bs)
{
  unsigned int len = 0;

  if(a_p  == NULL) return 1;
  if(a_bs == NULL) return 2;

  /* read expression operation */
  Bsc__getUnsignedB   (a_bs, (unsigned *)&a_p->m_o, 8);  /*expression operation code*/

  /* read integer value */
  Bsc__getUnsignedB   (a_bs, &a_p->m_iv, 32);  /*value of the constant*/

  /* read name string */
  Bsc__getUnsignedB   (a_bs, &len, 8);/*length of the string*/
  if (len == 0) return 3;
  if (len > STRMAX_VN) return 4;
  a_p->m_nm = (char*)MALLOC_VN(len+1, "fe2kAsm2Ncer__readBsc");
  if (a_p->m_nm == NULL) return 5;
  /*MEMSET_VN(a_p->m_nm, 0, len+1);*/
  Bsc__getOctetStr(a_bs, (unsigned char*)a_p->m_nm, len); /*name associated with the value, length limit is 128*/
  a_p->m_nm[len] = 0;
 
  /* read string value */
  Bsc__getUnsignedB (a_bs, &len, 8);/*length of the string*/
  if (len > STRMAX_VN) return 6;
  a_p->m_sv = (char*)MALLOC_VN(len+1, "fe2kAsm2Ncer__readBsc");
  if (a_p->m_sv == NULL) return 7;
  /*MEMSET_VN(a_p->m_sv, 0, len+1);*/
  if (len) Bsc__getOctetStr(a_bs, (unsigned char*)a_p->m_sv, len);/*string value*/
  a_p->m_sv[len] = 0;

  /* read ucode referencies list */
  Bsc__putUnsignedB (a_bs, &len, 8);/*identifier of the ucode reference*/
  if (len) {
    a_p->m_ur = (fe2kAsm2Ncur*)MALLOC_VN(sizeof (fe2kAsm2Ncur), "fe2kAsm2Ncer__readBsc");
    if (a_p->m_ur == NULL) return 8;
    if (fe2kAsm2Ncur__init(a_p->m_ur)) return 9;
    if (fe2kAsm2Ncur__readBsc(a_p->m_ur, a_bs)) return 10;
  }
  
  /* read right list */
  Bsc__getUnsignedB (a_bs, &len, 8);/*identifier of the right list*/
  if (len) {
    a_p->m_r = (fe2kAsm2Ncer*)MALLOC_VN(sizeof (fe2kAsm2Ncer), "fe2kAsm2Ncer__readBsc");
    if (a_p->m_r == NULL) return 11;
    if (fe2kAsm2Ncer__init(a_p->m_r)) return 12;
    if (fe2kAsm2Ncer__readBsc(a_p->m_r, a_bs)) return 13;
  }
  
  /* read down list */
  Bsc__getUnsignedB (a_bs, &len, 8);/*identifier of the down list*/
  if (len) {
    a_p->m_d = (fe2kAsm2Ncer*)MALLOC_VN(sizeof (fe2kAsm2Ncer), "fe2kAsm2Ncer__readBsc");
    if (a_p->m_d == NULL) return 14;
    if (fe2kAsm2Ncer__init(a_p->m_d)) return 15;
    if (fe2kAsm2Ncer__readBsc(a_p->m_d, a_bs)) return 16;
  }
  
  return 0;
}
  
/* print diagnostic info about object to stdout */ 
#ifdef PRINTDIAG_VN 
int fe2kAsm2Ncer__info (fe2kAsm2Ncer *a_p) 
{
  unsigned int i=10;
  fe2kAsm2Ncur *p = 0;

  if(a_p  == NULL) return 1;
  if(a_p->m_nm == NULL) return 2;
  if(*a_p->m_nm == 0) return 3;

  printf ("[%d\"%s\":%d,\"%s\"]", 
	  a_p->m_o, a_p->m_nm, a_p->m_iv, a_p->m_sv==NULL?"":a_p->m_sv);
  
  for (p=a_p->m_ur; p!=NULL && i--; p=p->m_ur) {
    printf ("->");
    if (fe2kAsm2Ncer__info(a_p)) return 4;
  
  }

  return 0;
}
#endif /*PRINTDIAG_VN*/

/* initialize all data holding members of the object*/
int fe2kAsm2Ncer__initVal (fe2kAsm2Ncer *a_p, char *a_nm, unsigned int a_iv, char *a_sv)
{
  unsigned int len = 0;

  if(a_p  == NULL) return 1;
  if(a_p->m_nm != NULL) return 2;
  if (a_nm == NULL) return 3;
  if (*a_nm == 0) return 4;
  if (a_sv == NULL) return 5;

  len = 0;
  len = STRLEN_VN(a_nm);
  if (len == 0) return 5;
  a_p->m_nm = (char*)MALLOC_VN(len+1, "fe2kAsm2Ncer__initVal");
  if (a_p->m_nm == NULL) return 6;
  STRNCPY_VN(a_p->m_nm, a_nm, len+1);
 
  a_p->m_iv = a_iv;

  if(a_p->m_sv != NULL) return 7;
  len = STRLEN_VN(a_sv);
  if (len > STRMAX_VN) return 8; 
  a_p->m_sv = (char*)MALLOC_VN(len+1, "fe2kAsm2Ncer__initVal");
  if (a_p->m_sv == NULL) return 6;
  STRNCPY_VN(a_p->m_sv, a_sv, len+1);

  if (a_p->m_ur != NULL) return 9;

  return 0;
}
  
/* link a_n to the object */ 
int fe2kAsm2Ncer__link    (fe2kAsm2Ncer *a_p, fe2kAsm2Ncer *a_n)
{
  if(a_p  == NULL) return 1;
  if(a_p->m_ur != NULL) return 2;
  
  a_p->m_pn = a_n;

  return 0;
}
  
/* add ucode reference */ 
int fe2kAxm2Ncer__addUr  (fe2kAsm2Ncer *a_p, fe2kAsm2Ncur *a_ur) 
{
  fe2kAsm2Ncur **dh = 0;
  if(a_p  == NULL) return 1;

  for (dh = &a_p->m_ur;  *dh != NULL; dh = &((*dh)->m_ur)) ;
  *dh = a_ur;

  return 0;
}

/* add expression reference */
int fe2kAxm2Ncer__addOp  (fe2kAsm2Ncer *a_p, fe2kAsm2Ncoe a_o, fe2kAsm2Ncer *a_d)
{
  if(a_p  == NULL) return 1;
  if(a_d  == NULL) return 2;

  a_p->m_o = a_o;
  a_p->m_d = a_d;

  return 0;
} 

int fe2kAsm2Ncer__copyVal (fe2kAsm2Ncer *a_p, fe2kAsm2Ncer *a_s)
{
  unsigned int len = 0;

  if(a_p  == NULL) return 1;
  if(a_s  == NULL) return 2;

  /* integer value */
  a_p->m_iv = a_s->m_iv;

  /* string value */
  if (a_s->m_sv == NULL) return 3;
  len = STRLEN_VN(a_s->m_sv);
  if (len > STRMAX_VN) return 4;
  if (a_p->m_sv) FREE_VN(a_p->m_sv);
  a_p->m_sv = MALLOC_VN(len+1, "fe2kAsm2Ncer__copyVal");
  if (a_p->m_nm == NULL) return 5;
  STRNCPY_VN(a_p->m_sv, a_s->m_sv, len+1);

  return 0;
}


/* calculate expression */
int fe2kAsm2Ncer__calcE  (fe2kAsm2Ncer *a_p)
{
  if(a_p  == NULL) return 1;

  switch (a_p->m_o) {
  case fe2kAsm2Ncoe__NUM: break;
  case fe2kAsm2Ncoe__Ref  : 
    if (a_p->m_ref) {
      if (fe2kAsm2Ncer__calcE(a_p->m_ref)) return 2; 
      if (fe2kAsm2Ncer__copyVal(a_p, a_p->m_ref)) return 3;
    } 
    break;
  case fe2kAsm2Ncoe__PLUS :
    if (a_p->m_d == NULL || a_p->m_d->m_r == NULL) return 4;
    if (fe2kAsm2Ncer__calcE(a_p->m_d)) return 5;
    if (fe2kAsm2Ncer__calcE(a_p->m_d->m_r)) return 6;
    a_p->m_iv = a_p->m_d->m_iv + a_p->m_d->m_r->m_iv;
    /* possible strings concatination ? for the future */
    break;
  case fe2kAsm2Ncoe__MINUS: 
    if (a_p->m_d == NULL || a_p->m_d->m_r == NULL) return 7;
    if (fe2kAsm2Ncer__calcE(a_p->m_d)) return 8;
    if (fe2kAsm2Ncer__calcE(a_p->m_d->m_r)) return 9;
    a_p->m_iv = a_p->m_d->m_iv - a_p->m_d->m_r->m_iv;
    /* possible strings concatination ? for the future */
    break;
  default: return 2; break;
  }

  return 0;
} 



/******************************************************************************
 * functions definitions for the class fe2kAsm2Ncqt (value reference) 
 *****************************************************************************/
int fe2kAsm2Ncqt__init    (fe2kAsm2Ncqt *a_p)
{
  if (a_p == NULL) return 1;

  a_p->m_ne = 0;
  a_p->m_bl = 0;
  a_p->m_ht = NULL;

  return 0;
}

int fe2kAsm2Ncqt__destroy (fe2kAsm2Ncqt *a_p)
{
  unsigned int i = 0;

  if (a_p == NULL) return 1;

  if (a_p->m_ne) {
    if (a_p->m_ht == NULL) return 2;
    for (i=0; i<a_p->m_ne; i++) if (a_p->m_ht[i] != NULL) {
	if (fe2kAsm2Ncer__destroy(a_p->m_ht[i])) return 3;
	FREE_VN (a_p->m_ht[i]);
    }
    FREE_VN (a_p->m_ht);
  }

  if (fe2kAsm2Ncqt__init(a_p)) return 3;

  return 0;
}

int fe2kAsm2Ncqt__copy    (fe2kAsm2Ncqt *a_p, fe2kAsm2Ncqt *a_s)
{
  unsigned int i = 0;

  if (a_p == NULL) return 1;
  if (a_s == NULL) return 2;

  if (a_p->m_ne != 0 || a_p->m_ht != NULL) return 3;

  a_p->m_ne = a_s->m_ne;
  a_p->m_bl = a_s->m_bl;
  
  if (a_p->m_ne) {
    if (a_s->m_ht == NULL) return 4;

    a_p->m_ht = (fe2kAsm2Ncer**) MALLOC_VN (a_p->m_ne * sizeof (fe2kAsm2Ncer*), "fe2kAsm2Ncqt__copy" );
    if (a_p->m_ht == NULL) return 5;

    for (i=0; i<a_p->m_ne; i++) if (a_s->m_ht[i] != NULL) {
      a_p->m_ht[i] = (fe2kAsm2Ncer *) MALLOC_VN (sizeof (fe2kAsm2Ncer), "fe2kAsm2Ncqt__copy");
      if (fe2kAsm2Ncer__init(a_p->m_ht[i])) return 6;
      if (fe2kAsm2Ncer__copy(a_p->m_ht[i], a_s->m_ht[i])) return 7;
    }
  }


  return 0;
}

int fe2kAsm2Ncqt__equ     (fe2kAsm2Ncqt *a_p, fe2kAsm2Ncqt *a_s, unsigned int *a_e)
{
  unsigned int i = 0;

  if (a_p == NULL) return 1;
  if (a_s == NULL) return 2;

  *a_e = (unsigned int) (a_p->m_ne == a_s->m_ne);
  if (*a_e == 0) return 0;

  *a_e = (unsigned int) (a_p->m_bl == a_s->m_bl);
  if (*a_e == 0) return 0;

  if (a_p->m_ne == 0) {
    if (a_p->m_ht != NULL) return 3;
    if (a_s->m_ht != NULL) return 4;
  } else {
    if (a_p->m_ht == NULL) return 3;
    if (a_s->m_ht == NULL) return 4;

    for (i=0; i<a_p->m_ne &&  *a_e != 0; i++) {
      *a_e = (unsigned int) ((a_p->m_ht == NULL) ^ (a_s->m_ht == NULL));
      if (*a_e == 0) return 0;
      
      if (a_s->m_ht[i] != NULL) 
	if (fe2kAsm2Ncer__equ(a_p->m_ht[i], a_s->m_ht[i], a_e)) return 5;
    }
  }


  return 0;
}

int fe2kAsm2Ncqt__writeBsc(fe2kAsm2Ncqt *a_p, Bsc *a_bs)
{
  unsigned int len = 0, i = 0;

  if(a_p  == NULL) return 1;
  if(a_bs == NULL) return 2;

  /* write bit length of the number representing length of the hash table */
  Bsc__putUnsignedB   (a_bs, &a_p->m_bl, 8);  /*length of the hash table*/

  /* write hash table */
  for (i=0; i<a_p->m_ne; i++) {
    if (a_p->m_ht[i] == NULL) {
      len = 0; 
      Bsc__putUnsignedB (a_bs, &len, 8);/*identifier for the table entry*/
    } else {
      len = 1; 
      Bsc__putUnsignedB (a_bs, &len, 8);/*identifier for the table entry*/
      if (fe2kAsm2Ncer__writeBsc (a_p->m_ht[i], a_bs)) return 3;
    }
  }

  return 0;
}

int fe2kAsm2Ncqt__readBsc (fe2kAsm2Ncqt *a_p, Bsc *a_bs)
{
  unsigned int len = 0, i = 0;

  if(a_p  == NULL) return 1;
  if(a_bs == NULL) return 2;

  /* read length of the hash table */
  Bsc__getUnsignedB   (a_bs, &a_p->m_bl, 8);  /*length of the hash table*/
  if (a_p->m_bl == 0) return 0;

  /* calc m_ne */
  for (a_p->m_ne=1, i=0; i<a_p->m_bl; a_p->m_ne <<= 1) i++;

  /* allocate hash table */
  a_p->m_ht = (fe2kAsm2Ncer**) MALLOC_VN (a_p->m_ne * sizeof (fe2kAsm2Ncer*), "fe2kAsm2Ncqt__readBsc" );
  if (a_p->m_ht == NULL) return 3;

  /* read hash table */
  for (i=0; i<a_p->m_ne; i++) {
    /* get identifier for the table entry */
    Bsc__putUnsignedB (a_bs, &len, 8);/*identifier for the table entry*/
    
    /* read table entry */
    if (len == 1) {
      a_p->m_ht[i] = (fe2kAsm2Ncer *) MALLOC_VN (sizeof (fe2kAsm2Ncer), "fe2kAsm2Ncqt__readBsc");
      if (a_p->m_ht[i] == NULL) return 4;
      if (fe2kAsm2Ncer__init(a_p->m_ht[i])) return 5;
      if (fe2kAsm2Ncer__readBsc (a_p->m_ht[i], a_bs)) return 6;

    }
  }

  return 0;
}

#ifdef PRINTDIAG_VN 
int fe2kAsm2Ncqt__info    (fe2kAsm2Ncqt *a_p) 
{
  unsigned int i=0, maxi=10;

  if (a_p == NULL) return 1;

  printf ("\nNamed constants Quick Table");
  printf ("\n\tm_bl = %d, m_ne = 0x%08x", a_p->m_bl, a_p->m_ne);

  for (i=0; i<a_p->m_ne && i< maxi; i++) {
    printf ("\n\t table entry %d", i);
    if (fe2kAsm2Ncer__info(a_p->m_ht[i])) return 2;
  }

  return 0;
}
#endif /*PRINTDIAG_V*/

int fe2kAsm2Ncqt__initVal (fe2kAsm2Ncqt *a_p, unsigned int a_ne) 
{
  if (a_p == NULL) return 1;

  if (a_p->m_ne != 0) return 2;
  if (a_p->m_ht != 0) return 3;
  if (a_ne > 0xffff) return 4;

  /* calculate table elements number as smallest power of 2 lager or equal to a_ne */
  for (a_p->m_ne = 1, a_p->m_bl=0; a_p->m_ne < a_ne; a_p->m_ne <<= 1, a_p->m_bl++) 
    if (a_p->m_ne > 0x10000) return 5;

  /* allocate hash table */
  a_p->m_ht = (fe2kAsm2Ncer**) MALLOC_VN (a_p->m_ne * sizeof (fe2kAsm2Ncer*), "fe2kAsm2Ncqt__initVal" );
  if (a_p->m_ht == NULL) return 6;

  return 0;
}

int fe2kAsm2Ncqt__addEntry(fe2kAsm2Ncqt *a_p, fe2kAsm2Ncer *a_e) 
{
  fe2kAsm2Ncer **he = 0;
  unsigned int hi = 0;

  if (a_p == NULL) return 1;
  if (a_p->m_ne == 0) return 2;
  if (a_p->m_ht == NULL) return 3;
  if (a_e == NULL) return 4;

  /* calculate index in the hash table */
  if (fe2kAsm2__hash(a_e->m_nm, &hi, a_p->m_bl)) return 4;

  /* find last entry in the list */
  for (he=&a_p->m_ht[hi]; *he; he=&(*he)->m_pn) if (!STRNCMP_VN((*he)->m_nm, a_e->m_nm, STRMAX_VN)) return  4;

  /* initialize entry */
  *he = a_e;

  return 0;
}


int fe2kAsm2Ncqt__addEntryVal(fe2kAsm2Ncqt *a_p, char *a_nm, unsigned int a_iv, char *a_sv) 
{
  fe2kAsm2Ncer *pe = 0;

  if (a_p == NULL) return 1;

  /* allocate and initialize entry */
  pe = (fe2kAsm2Ncer *) MALLOC_VN (sizeof (fe2kAsm2Ncer), "fe2kAsm2Ncqt__addEntryVal");
  if (pe == NULL) return 4;
  if (fe2kAsm2Ncer__init(pe)) {
      FREE_VN(pe);
      return 5;
  }
  if (fe2kAsm2Ncer__initVal(pe, a_nm, a_iv, a_sv)) {
      FREE_VN(pe);
      return 6;
  }

  /* add entry to the hash table */
  if (fe2kAsm2Ncqt__addEntry(a_p, pe)) {
      FREE_VN(pe);
      return 7;
  }

  return 0;
}


  

/******************************************************************************
 * end of file fe2k-as2-ncae.c
 *****************************************************************************/
