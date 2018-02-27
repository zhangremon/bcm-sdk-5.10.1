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
 * $Id: fe2k-asm2-ncae.h 1.4.158.1 Broadcom SDK $
 *
 ******************************************************************************/

/* fe2k-asm2-intd-template.h */

#ifndef FE2K_ASM2_NCAE_H__DEFINED
#define FE2K_ASM2_NCAE_H__DEFINED

#ifndef ASSERT_VN
#ifndef ASSERT 
#ifndef __KERNEL__
#include <stdlib.h>/*for the exit()*/ 
#endif
#define ASSERT_VN(x) assert(x)                                                                    
#else                                                                                                        
#define ASSERT_VN(x) ASSERT(x)                                                                               
#endif /*ASSERT*/ 
#endif /*ASSERT_VN*/                                                                                           


/* temperary for the debugging only */
#include <soc/sbx/fe2k_common/Bsc.h>

/******************************************************************************
 * fe2k asm2 Named Constants Access Entry
 *****************************************************************************/
typedef struct fe2kAsm2Ncae__ {
  char *m_name; /*name associated with the value*/
  unsigned int
    m_fdin,    /*flag indicating dinamic allocation, we do not save it in the buffer / file or read it from there*/

    m_fc,      /*flag which indicate if value is in the bytecode*/
    m_va,      /*value of the constant*/
    
    /*corresponded bytecode parameters, valid only if m_fc is not 0*/
    m_sn,  /*stream number*/
    m_in,  /*instruction number*/ 
    m_bo,  /*offset of the sub-field in the machine word (bits)*/
    m_bl;  /*length of the sub-field in the machine word (bits)*/
} fe2kAsm2Ncae;

/***********=******************************************************************
 * functions declarations for the class fe2kAsm2Ncae
 *****************************************************************************/

/* initialize interface structure to be empty */
int fe2kAsm2Ncae__init(fe2kAsm2Ncae *a_p);

/* compare 2 structure and return rezult in a_e */
int fe2kAsm2Ncae__equ(fe2kAsm2Ncae *a_a, fe2kAsm2Ncae *a_b, int *a_e);

/* destroy interface structure to be empty */
int fe2kAsm2Ncae__destroy(fe2kAsm2Ncae *a_p);

/* serialize all interface data into the buffer */
int fe2kAsm2Ncae__writeBsc(fe2kAsm2Ncae *a_p, Bsc *a_pb);

/* deserialize all interface data from the buffer */
int fe2kAsm2Ncae__readBsc(fe2kAsm2Ncae *a_p, Bsc *a_pb);

/* copy NCA structure a_s to NCA structure a_d with dynamic buffers */
int fe2kAsm2Ncae__copyD(fe2kAsm2Ncae *a_d, fe2kAsm2Ncae *a_s);


/******************************************************************************
 * fe2k asm2 Named Constants Access Entry
 *****************************************************************************/
typedef struct NcatQae__ {
  unsigned int m_ind;
  struct NcatQae__ *m_right, *m_down; /* m_right - items with the same name; m_down - items with same hash, diff name */
} NcatQae;

/******************************************************************************
 * functions declarations for the class fe2kAsm2Ncae
 *****************************************************************************/
int NcatQae__init (NcatQae *a_p);
int NcatQae__destroy (NcatQae *a_p);

/******************************************************************************
 ******************************************************************************
 *****************************************************************************/

/******************************************************************************
 * classes for the new, quick Named Constants Access Interface 
 * supporting strings and arrays
 *****************************************************************************/

/* utility methods */
/* calculates hash value of the string a_s with hash bit length a_bl*/
int fe2kAsm2_hash    (char *a_s, unsigned int *a_h, unsigned int a_bl);

/* Named Constant Ucode Reference */
typedef struct fe2kAsm2Ncur__ {
  unsigned int
    m_sn,  /*stream number*/
    m_in,  /*instruction number*/ 
    m_bo,  /*offset of the sub-field in the machine word (bits)*/
    m_bl;  /*length of the sub-field in the machine word (bits)*/
  struct fe2kAsm2Ncur__ *m_ur; /*pointer to the next object in the linked list*/
} fe2kAsm2Ncur;

/* universal methods for the pseudo class fe2kAsm2Ncur */
int fe2kAsm2Ncur__init     (fe2kAsm2Ncur *a_p); /* default initialization */
int fe2kAsm2Ncur__create   (fe2kAsm2Ncur **a_h); /* default creation */
int fe2kAsm2Ncur__destroy  (fe2kAsm2Ncur *a_p); /* default deinitialization */
int fe2kAsm2Ncur__destroyL (fe2kAsm2Ncur *a_p); /* default deinitialization including linked list*/
int fe2kAsm2Ncur__copy     (fe2kAsm2Ncur *a_p, fe2kAsm2Ncur *a_s); /* copy one object to another */
int fe2kAsm2Ncur__equ      (fe2kAsm2Ncur *a_p, fe2kAsm2Ncur *a_s, unsigned int *a_e);/* compare two objects excluding reference list*/
int fe2kAsm2Ncur__equL     (fe2kAsm2Ncur *a_p, fe2kAsm2Ncur *a_s, unsigned int *a_e);/* compare two objects including referemce list*/
int fe2kAsm2Ncur__writeBsc (fe2kAsm2Ncur *a_p, Bsc *a_bs); /* write (serialize) object to the Bsc bit stream excluding ref list*/
int fe2kAsm2Ncur__writeBscL(fe2kAsm2Ncur *a_p, Bsc *a_bs); /* write (serialize) object to the Bsc bit stream including ref list*/
int fe2kAsm2Ncur__readBsc  (fe2kAsm2Ncur *a_p, Bsc *a_bs); /* read (deserialize) object from Bsc stream excluding ref list*/
int fe2kAsm2Ncur__readBscL (fe2kAsm2Ncur *a_p, Bsc *a_bs); /* read (deserialize) object from Bsc stream insluding ref list*/
int fe2kAsm2Ncur__lenBuf   (fe2kAsm2Ncur *a_p, unsigned int *a_l); /* calc buf len for the class serializing (excluding ref list) */
int fe2kAsm2Ncur__lenBufL  (fe2kAsm2Ncur *a_p, unsigned int *a_l); /* calc buf len for the class serializing (including ref list) */
#ifdef PRINTDIAG_VN 
int fe2kAsm2Ncur__info     (fe2kAsm2Ncur *a_p); /* print diagnostic info about object to stdout */
int fe2kAsm2Ncur__infoL    (fe2kAsm2Ncur *a_p); /* print diagnostic info about object to stdout including linked list*/
#endif /*PRINTDIAG_V*/
int fe2kAsm2Ncur__initVal  (fe2kAsm2Ncur *a_p, unsigned int a_sn, unsigned int a_in, unsigned int a_bo, unsigned int a_bl); /* init all data holding members of the object*/
int fe2kAsm2Ncur__link     (fe2kAsm2Ncur *a_p, fe2kAsm2Ncur *a_n); /* link a_n to the object */

/* enum used to encode operations type for the expressions in the NCAT */
typedef enum fe2kAsm2Ncoe__ {
  fe2kAsm2Ncoe__Ref,     /* this is reference to other named expression in the NCAT table */

  fe2kAsm2Ncoe__OR,
  fe2kAsm2Ncoe__AND,
  fe2kAsm2Ncoe__BIT_OR,
  fe2kAsm2Ncoe__BIT_XOR,
  fe2kAsm2Ncoe__BIT_AND,
  fe2kAsm2Ncoe__EQU,
  fe2kAsm2Ncoe__NEQU,
  fe2kAsm2Ncoe__LESS,
  fe2kAsm2Ncoe__MORE,
  fe2kAsm2Ncoe__LESSEQU,
  fe2kAsm2Ncoe__MOREEQU,
  fe2kAsm2Ncoe__SHIFT_L,
  fe2kAsm2Ncoe__SHIFT_R,
  fe2kAsm2Ncoe__PLUS,
  fe2kAsm2Ncoe__MINUS,
  fe2kAsm2Ncoe__MUL,
  fe2kAsm2Ncoe__DIV,
  fe2kAsm2Ncoe__MOD,
  fe2kAsm2Ncoe__PREF_PP,
  fe2kAsm2Ncoe__PREF_MM,
  fe2kAsm2Ncoe__PREF_M,
  fe2kAsm2Ncoe__PREF_BI,
  fe2kAsm2Ncoe__PREF_N,

  fe2kAsm2Ncoe__NUM,

  fe2kAsm2Ncoe__MaxEnum  /* maximum enum value for the validation of the operation code */
} fe2kAsm2Ncoe;

/* structure for the dependencies referencies list, calculated after reading */
typedef struct fe2kAsm2Ncdr__ {
  struct fe2kAsm2Ncer__ *m_pe;
  struct fe2kAsm2Ncdr__ *m_pn;
} fe2kAsm2Ncdr;

/* Named Constant Expression Reference

   if this is reference to other expresiosn, then:
   m_nm  = name; - is initialized, it is saved in the bin file and used 
              to find reference point 
   m_d   = NULL; - down pointer for the expression arguments  
   m_r   = NULL; - right pointer of the expression arguments
   m_o   = fe2kAsm2Ncoe__Ref; - reference pointer to other entry in the table 
   m_ref - pointer to the reference initialized by init function 
              of the hash table class at the initializaiton time
*/
typedef struct fe2kAsm2Ncer__ {
  enum fe2kAsm2Ncoe__ 
          m_o;   /* operation code */
  char    *m_nm, /* name of the expression which can be a constant */
          *m_sv; /*string value, "" =  \0 for the empty string */
  unsigned int          
          m_iv; /*integer value*/
  struct fe2kAsm2Ncer__ 
          *m_d,   /* down link in the expresison tree */ 
          *m_r,   /* right link in the expression tree */ 
          *m_ref, /* pointer to the reference to other named expression, initialized after loading, 
	       during prepare fase */ 
          *m_pn;  /*pointer to the next value ref in the list with the same hash*/
  struct fe2kAsm2Ncur__ 
          *m_ur; /*ucode ref list, if it is NULL no ucode ref*/
  struct fe2kAsm2Ncdr__ 
          *m_dep; /* dependencies list - list of pointers to the entries using expressions 
				   dependant on this one, initialized during prepare fase */
} fe2kAsm2Ncer;

/* universal methods for the pseudo class fe2kAsm2Ncer */
int fe2kAsm2Ncer__init    (fe2kAsm2Ncer *a_p); /* default initialization */
int fe2kAsm2Ncer__destroy (fe2kAsm2Ncer *a_p); /* default deinitialization */
int fe2kAsm2Ncer__copy    (fe2kAsm2Ncer *a_p, fe2kAsm2Ncer *a_s); /* copy one object to another */
int fe2kAsm2Ncer__equ     (fe2kAsm2Ncer *a_p, fe2kAsm2Ncer *a_s, unsigned int *a_e); /* compare two objects */
int fe2kAsm2Ncer__writeBsc(fe2kAsm2Ncer *a_p, Bsc *a_bs); /* write (serialize) object to the Bsc bit stream */
int fe2kAsm2Ncer__readBsc (fe2kAsm2Ncer *a_p, Bsc *a_bs); /* read (deserialize) object from Bsc stream */
#ifdef PRINTDIAG_VN 
int fe2kAsm2Ncer__info    (fe2kAsm2Ncer *a_p); /* print diagnostic info about object to stdout */
#endif /*PRINTDIAG_V*/
int fe2kAsm2Ncer__initVal (fe2kAsm2Ncer *a_p, char *a_nm, unsigned int a_iv, char *a_sv); /* initialize all data holding members of the object*/
int fe2kAsm2Ncer__link    (fe2kAsm2Ncer *a_p, fe2kAsm2Ncer *a_n); /* link a_n to the object */
int fe2kAsm2Ncer__addUr   (fe2kAsm2Ncer *a_p, fe2kAsm2Ncur *a_ur); /* add ucode reference */
int fe2kAsm2Ncer__addOp   (fe2kAsm2Ncer *a_p, fe2kAsm2Ncoe a_o, fe2kAsm2Ncer *a_d); /* add expression reference */
int fe2kAsm2Ncer__copyVal (fe2kAsm2Ncer *a_p, fe2kAsm2Ncer *a_s);
int fe2kAsm2Ncer__calcE   (fe2kAsm2Ncer *a_p); /* calculate expression */

/* Named Constant Quick Table */
typedef struct fe2kAsm2Ncqt__ {
  unsigned int 
  m_ne, /*number of elements in the hash table*/
  m_bl; /*maximum number of bits in the number representing table index*/  
  struct fe2kAsm2Ncer__ **m_ht; /* hash index table of the Constant Value Ref*/
} fe2kAsm2Ncqt;

/* methods for the pseudo class fe2kAsm2Ncqt */
int fe2kAsm2Ncqt__init    (fe2kAsm2Ncqt *a_p); /* default initialization */
int fe2kAsm2Ncqt__destroy (fe2kAsm2Ncqt *a_p); /* default deinitialization */
int fe2kAsm2Ncqt__copy    (fe2kAsm2Ncqt *a_p, fe2kAsm2Ncqt *a_s); /* copy one object to another */
int fe2kAsm2Ncqt__equ     (fe2kAsm2Ncqt *a_p, fe2kAsm2Ncqt *a_s, unsigned int *a_e); /* compare two objects */
int fe2kAsm2Ncqt__writeBsc(fe2kAsm2Ncqt *a_p, Bsc *a_bs); /* write (serialize) object to the Bsc bit stream */
int fe2kAsm2Ncqt__readBsc (fe2kAsm2Ncqt *a_p, Bsc *a_bs); /* read (deserialize) object from Bsc stream */
#ifdef PRINTDIAG_VN 
int fe2kAsm2Ncqt__info    (fe2kAsm2Ncqt *a_p); /* print diagnostic info about object to stdout */
#endif /*PRINTDIAG_V*/
int fe2kAsm2Ncqt__initVal (fe2kAsm2Ncqt *a_p, unsigned int a_ne); /* initialize all data holding members of the object*/
int fe2kAsm2Ncqt__addEntry(fe2kAsm2Ncqt *a_p, fe2kAsm2Ncer *a_e); 
int fe2kAsm2Ncqt__addEntryVal(fe2kAsm2Ncqt *a_p, char *a_nm, unsigned int a_iv, char *a_sv); 


#endif /*FE2K_ASM2_NCAE_H__DEFINED*/
