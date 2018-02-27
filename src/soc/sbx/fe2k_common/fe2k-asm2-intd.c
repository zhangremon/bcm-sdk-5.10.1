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
 * $Id: fe2k-asm2-intd.c 1.7.112.2 Broadcom SDK $
 *
 * fe2k-asm2-intd.c
 *****************************************************************************/

#include <soc/sbx/fe2k_common/fe2k-asm2-intd.h>
#include <soc/sbx/fe2k_common/fe2k-asm2-md5.h>
#include <soc/sbx/fe2k_common/Bsc.h>

#ifdef PRINTDIAG_VN
#include <stdio.h>  /*for the debugging trace */
#endif /*PRINTDIAG_VN*/
  
/******************************************************************************
 * functions definitions for the class fe2kAsm2Nca fe2kAsm2IntD
 *****************************************************************************/

/* initialize interface structure to be empty */
int fe2kAsm2IntD__init(fe2kAsm2IntD *a_p)
{
  if (a_p == NULL) return 1;

  a_p->m_magic      = 0x12345678;

  a_p->m_tver_maj   = 0;
  a_p->m_tver_min   = 0;
  a_p->m_tver_patch = 0;

  a_p->m_uver_maj   = 0;
  a_p->m_uver_min   = 0;
  a_p->m_uver_patch = 0;

  a_p->m_fdin       = 0;
  a_p->m_snum       = 0;
  a_p->m_inum       = 0;
  a_p->m_onum       = 0;
  a_p->m_ncnum      = 0;
  a_p->m_hwid       = 0;

  a_p->m_tver_name = NULL;
  a_p->m_uver_name = NULL;

  MEMSET_VN(a_p->m_md5a, 0, sizeof (a_p->m_md5a));

  a_p->m_ncat = NULL;
  a_p->m_qal = 0;
  a_p->m_qat = NULL;
  a_p->m_tmap = NULL;
  a_p->m_code = NULL;

  a_p->m_fPut = NULL;
  a_p->m_fGet = NULL;
  a_p->m_pv = NULL;

  return 0;
}

/* initialize interface structure to be empty with initialization of pointers to the hw access functions*/
int fe2kAsm2IntD__initF(fe2kAsm2IntD *a_p, HwUcode *a_fPut, HwUcode *a_fGet, void *a_pv)
{
  if (a_p==NULL) return 1;
  a_p->m_fPut = a_fPut;
  a_p->m_fGet = a_fGet;
  a_p->m_pv = a_pv;
  return 0;
}      

/* destroy interface structure to be empty */
int fe2kAsm2IntD__destroy(fe2kAsm2IntD *a_p)
{
  int i=0;

  if (a_p == NULL) return 1;

  if(a_p->m_fdin) {
    for (i=0; i<a_p->m_ncnum; i++) fe2kAsm2Ncae__destroy(&a_p->m_ncat[i]);
    FREE_VN(a_p->m_tver_name);
    FREE_VN(a_p->m_uver_name);
    FREE_VN(a_p->m_ncat);
    FREE_VN(a_p->m_tmap);
    FREE_VN(a_p->m_code);
  }

  if (a_p->m_qat != NULL) {
    for (i=0; i< a_p->m_qal; i++) {
      if (a_p->m_qat[i] != NULL) {
	NcatQae__destroy(a_p->m_qat[i]);
	FREE_VN((void*)a_p->m_qat[i]);
      }
    }
    FREE_VN((void*)a_p->m_qat);
  }

  if (fe2kAsm2IntD__init(a_p)) return 2;

  return 0;
}

/* calculate hash value of the string for the hash table size m_qal */
int fe2kAsm2IndD__calcHash(fe2kAsm2IntD *a_p, char *a_s, unsigned int *a_h)
{
     unsigned char l_rand[] = {
      1,14,110,25,97,174,132,119,138,170,125,118,27,233,140,51,87,197,177,107,
      234,169,56,68,30,7,173,73,188,40,36,65,49,213,104,190,57,211,148,223,48,
      115,15,2,67,186,210,28,12,181,103,70,22,58,75,78,183,167,238,157,124,147,
      172,144,176,161,141,86,60,66,128,83,156,241,79,46,168,198,41,254,178,85,
      253,237,250,154,133,88,35,206,95,116,252,192,54,221,102,218,255,240,82,
      106,158,201,61,3,89,9,42,155,159,93,166,80,50,34,175,195,100,99,26,150,
      16,145,4,33,8,189,121,64,77,72,208,245,130,122,143,55,105,134,29,164,185,
      194,193,239,101,242,5,171,126,11,74,59,137,228,108,191,232,139,6,24,81,
      20,127,17,91,92,251,151,225,207,21,98,113,112,84,226,18,214,199,187,13,
      32,94,220,224,212,247,204,196,43,249,236,45,244,111,182,153,136,129,90,
      217,202,19,165,231,71,230,142,96,227,62,179,246,114,162,53,160,215,205,
      180,47,109,44,38,31,149,135,0,216,52,63,23,37,69,39,117,146,184,163,200,
      222,235,248,243,219,10,152,131,123,229,203,76,120,209
   };

  if (a_p == NULL) return 1;
  if (a_s == NULL) return 2;
  if (a_h == NULL) return 3;

  /* currently hash calculated for the maximum value of table elements = 256 */
  for (*a_h=0; *a_s; a_s++) *a_h ^= l_rand[(unsigned char)*a_s];

  return 0;
}

/* Recover named constant values from hardware */
int fe2kAsm2IndD__recoverNcv(fe2kAsm2IntD *a_p)
{
    unsigned int val, i=0;
    fe2kAsm2Ncae *nc;
    unsigned char *wp;
    Bsc bsc;
    
    if (a_p == NULL) return 1;
    
    if (a_p->m_qat == NULL) return 2; /* accelerator must be present */
    if (a_p->m_fGet == NULL) return 3; /* hardware getter must valid */

    for (i=0; i<a_p->m_qal; i++) {
        NcatQae *root;

        /* Walk the down tree, if *any* entry is in bytecode, 
         * recover it for *all* entries
         *  m_right - items with the same name; 
         *  m_down - items with same hash, diff name
         */
        for (root = a_p->m_qat[i]; root; root = root->m_down) {
            NcatQae *node = root;

            for (nc = &a_p->m_ncat[node->m_ind];
                 node && nc->m_fc == 0; 
                 node = node->m_right, 
                     nc = node ? &a_p->m_ncat[node->m_ind] : NULL)
                ;
            
            if (nc) {
                /* get the entry from byte code using the found nc */
                /* calculate pointer to the first octet of the machine word */
                if (fe2kAsm2IntD__getWordPtr(a_p, nc->m_sn, nc->m_in, &wp)) {
                    return 4;
                }
                
                if (a_p->m_fGet(a_p->m_pv, nc->m_sn, nc->m_in, wp)) {
                    return 5;
                }
                
                /*init bit stream class to the machine word*/
                Bsc__initFromBuf (&bsc, wp, a_p->m_onum*8); 
                
                /*set appropriate bsc attributes*/
                Bsc__setLsbf (&bsc, FALSE_VN); 
                Bsc__setLsof (&bsc, FALSE_VN); 
                
                /* move bit stream to the offset of the field, note then given
                 * offsets based on the 16+ values
                 */ 
                Bsc__moveB (&bsc, 
                            a_p->m_onum * 8 - ((nc->m_bo-16) + nc->m_bl),
                            eBscBeg);
                
                /*put new value to the machine word field*/
                Bsc__getUnsignedB (&bsc, &val, nc->m_bl);

                /* update ALL entries from ROOT */
                for (node = root; node; node = node->m_right) {
                    a_p->m_ncat[node->m_ind].m_va = val;
                }
            }
            /* else, named constant is not in byte code */
        }
    }

    return 0;   
}

/* initialize named constants table access accelerator */
int fe2kAsm2IndD__initQat (fe2kAsm2IntD *a_p)
{
  unsigned int i=0;
  if (a_p == NULL) return 1;

  /* temperary assign hash size to the one octet hash */
  a_p->m_qal=256;

  /* allocate and initialize table */
  a_p->m_qat = (NcatQae**)MALLOC_VN(a_p->m_qal * (sizeof (NcatQae*)), "fe2kAsm2IndD__initQat");
  if (a_p->m_qat == NULL) return 2;
  for (i=0; i<a_p->m_qal; i++) a_p->m_qat[i] = NULL; 

  /* for every element in the NCAT add corresponded element to the QAT */
  for (i=0; i<a_p->m_ncnum; i++) {
    unsigned int lh=0, rf=0; /*variables for hash value and right-flag*/
    char    *lname;          /*name associated with the new element*/
    NcatQae *qe=0, **hc=0;   /*new element and handler for the tree traversing*/

    /* create and init new element */
    qe = (NcatQae *)MALLOC_VN(sizeof (NcatQae), "fe2kAsm2IndD__initQat");
    if (qe == NULL) return 3;
    if (NcatQae__init(qe)) {
        FREE_VN(qe);
        return 4;
    }
    qe->m_ind = i;

    /* hash for the element name and handler of the element in the hash table */
    lname = a_p->m_ncat[i].m_name;
    if (fe2kAsm2IndD__calcHash(a_p, lname, &lh)) {
        FREE_VN(qe);
        return 5;
    }
    hc=&(a_p->m_qat[lh]);

    /*walk down the tree and if there is name match then right along the branch */
    for (rf=0;*hc; hc=rf?&((*hc)->m_right):&((*hc)->m_down))     
      if (!rf && !STRNCMP_VN(a_p->m_ncat[(*hc)->m_ind].m_name, lname, STRMAX_VN))  rf=1;
    
    if (*hc==NULL) *hc = qe; else {
        FREE_VN(qe);
        return 6;
    }
  }
  
  return 0;
}

#ifdef PRINTDIAG_VN
/* debugging printing function */
#define PRINT_V1(X)  printf ("%s = \t0x%08x = %u\n", #X, a_p->X, a_p->X);
#define PRINT_S1(X)  printf ("%s = \t\"%s\"\n", #X, a_p->X);
int fe2kAsm2IntD__printf(fe2kAsm2IntD *a_p)
{
  int i=0, sn=0, in=0, on=0;

  if (a_p == NULL) return 1;

  printf ("fe2kAsm2IntD:\n");

#if 1
  PRINT_V1(m_magic);

  PRINT_V1(m_tver_maj);
  PRINT_V1(m_tver_min);
  PRINT_V1(m_tver_patch);
  PRINT_S1(m_tver_name);

  PRINT_V1(m_uver_maj);
  PRINT_V1(m_uver_min);
  PRINT_V1(m_uver_patch);
  PRINT_S1(m_uver_name);

  PRINT_V1(m_fdin);
  PRINT_V1(m_snum);
  PRINT_V1(m_inum);
  PRINT_V1(m_onum);
  PRINT_V1(m_ncnum);
  PRINT_V1(m_hwid);
#endif

  /* print md5 sum */
#if 1
  printf("m_md5a = ");
  for (i=0; i<sizeof(a_p->m_md5a); i++) printf("%02x", a_p->m_md5a[i]);
  printf ("\n");
#endif  

  /* print tmap */
#if 1
  printf("m_tmap =\t{");
  for (i=0; i < (a_p->m_inum+7)/8; i++) printf ("%s 0x%02x", ((i%16)? "":"\n\t\t\t"),a_p->m_tmap[i]);
  printf("\n\t\t}\n");
#endif

  /* print code */
#if 1
  printf("m_code =\t{\n");
  for (sn=0; sn<a_p->m_snum; sn++) {
    if (sn==0) { 
      printf("\t\t\t/*stream = %d*/\n\t\t\t\n", sn);
      for (in=0; in<a_p->m_inum; in++) {
	if (in<10) {
	  printf("\t\t\t");
	  for (on=0; on<a_p->m_onum; on++) {
	    printf("%s0x%02x", (on?",":""), (int)a_p->m_code[(sn*a_p->m_inum+in)*a_p->m_onum+on] );
	  }
	  printf(" /*instr = %d*/\n", in);
	}
      } 
      printf("\n");
    }
  }
  printf("\t\t}\n");
#endif

  /* print ncat */
#if 1
  printf("m_ncat =\t{\n");
  for (i=0; i<a_p->m_ncnum; i++) {
    if (i < 1) {
      fe2kAsm2Ncae *ep = &a_p->m_ncat[i];
      printf("\t\t{\"%s\",%d,%d,0x%x,%d,%d,%d,%d} /*entry = %d*/ \n", ep->m_name, ep->m_fdin, ep->m_fc, ep->m_va, ep->m_sn, ep->m_in, ep->m_bo, ep->m_bl, i);
    }
  }
  printf("\t}\n");
#endif

  return 0;
}
#endif /*PRINTDIAG_VN*/

/* compare 2 interface structure and return rezult in a_e; 
   we do not compare m_fdin - flag indicating dinamic allocation 
   as we no not write it to file and want compate dynamic and static structuries */
int fe2kAsm2IntD__equ(fe2kAsm2IntD *a_a, fe2kAsm2IntD *a_b, int *a_e)
{
  int i=0;
  if (a_a == NULL) return 1;
  if (a_b == NULL) return 2;
  if (a_e == NULL) return 3;
  
  if (   a_a->m_uver_name==0 || a_b->m_uver_name==0 
      || a_a->m_tver_name==0 || a_b->m_tver_name==0
      || a_a->m_ncat==0      || a_b->m_ncat==0 
      || a_a->m_tmap==0      || a_b->m_tmap==0 
      || a_a->m_code==0      || a_b->m_code==0
      ) return 4; /* non initialized buffers error */

  if (   a_a->m_snum>32      || a_b->m_snum>32 
      || a_a->m_inum>0x1000  || a_b->m_inum>0x1000 
      || a_a->m_onum>32      || a_b->m_onum>32 
	 /*  a_a->m_ncnum>0x10000 || a_b->m_ncnum>0x10000 */
      ) return 5; /* clearly invalid values */

  *a_e = (int) (   a_a->m_magic      == a_a->m_magic 
      
		&& a_a->m_tver_maj   == a_a->m_tver_maj    
		&& a_a->m_tver_min   == a_a->m_tver_min    
		&& a_a->m_tver_patch == a_a->m_tver_patch 
		&& STRNCMP_VN (a_a->m_tver_name, a_b->m_tver_name, STRMAX_VN)==0

		&& a_a->m_uver_maj   == a_a->m_uver_maj    
		&& a_a->m_uver_min   == a_a->m_uver_min    
		&& a_a->m_uver_patch == a_a->m_uver_patch 
		&& STRNCMP_VN (a_a->m_uver_name, a_b->m_uver_name, STRMAX_VN)==0

		&& a_a->m_snum	     == a_a->m_snum	      
		&& a_a->m_inum	     == a_a->m_inum	      
		&& a_a->m_onum	     == a_a->m_onum	      
		&& a_a->m_ncnum      == a_a->m_ncnum      
		&& a_a->m_hwid       == a_a->m_hwid      
		);
  if(*a_e == 0) return 0;

  for (i=0; i<(a_a->m_inum/8); i++) {
    *a_e = (int) (a_a->m_tmap[i] == a_b->m_tmap[i]);
    if(*a_e == 0) {
      /* printf ("i=%d, tmap[i]=0x%02x\n"); debug printing*/
      return 0;
    }
  }
  
  for (i=0; i<a_a->m_snum*a_a->m_inum*a_a->m_onum; i++) {
    *a_e = (int) (a_a->m_code[i] == a_b->m_code[i]);
    if(*a_e == 0) {
      /* debug printing 
      printf ("i=%d, stream=%d, instr=%d, octet=%d, instr(a_a)=0x%02x, instr(a_b)=0x%02x\n",
	       i,
	       i/(a_a->m_inum*a_a->m_onum), 
	       (i%(a_a->m_inum*a_a->m_onum))/a_a->m_onum,
	       i%a_a->m_onum, 
	       (int)a_a->m_code[i], 
	       (int)a_b->m_code[i]);*/
	       
      return 0;
    }
  }

  for (i=0; i<a_a->m_ncnum; i++) {
    if (fe2kAsm2Ncae__equ(&a_a->m_ncat[i], &a_b->m_ncat[i], a_e)) return 3;
    if(*a_e == 0) {
      /*printf ("i=%d\n", i); debug printing*/ 
      return 0;
    }
  }
    
  return 0;
}

/* serialize all interface data into the buffer */
int fe2kAsm2IntD__writeBsc(fe2kAsm2IntD *a_p, Bsc *a_pb)
{
  unsigned int 
    tlen=0 
    ,clen=0 
    ,i=0 
    ,pad=0
    ,len=0 /*length of the string portion of the version*/
    ;

  /*validate interface structure parameters*/
   if (a_p == NULL) return 1;
   if (a_pb == NULL) return 2;
   if (a_p->m_ncnum != 0 && a_p->m_ncat == NULL) return 3;
   if (a_p->m_snum == 0) return 4;
   if (a_p->m_inum == 0) return 5;
   if (a_p->m_tmap == NULL) return 6;
   if (a_p->m_code ==NULL) return 7;

  /* calculate md5 checksum for the interface structure */
  if (fe2kAsm2IndD__md5a (a_p, a_p->m_md5a)) return 8;

  Bsc__putUnsignedB (a_pb, &a_p->m_magic, 32);

  /* tool version info */
  Bsc__putUnsignedB (a_pb, &a_p->m_tver_maj, 32);
  Bsc__putUnsignedB (a_pb, &a_p->m_tver_min, 32);
  Bsc__putUnsignedB (a_pb, &a_p->m_tver_patch, 32);
  len = STRLEN_VN(a_p->m_tver_name);  /*calculate length of the string*/
  if (len > STRMAX_VN) return 9;
  Bsc__putUnsignedB(a_pb, &len, 8);         /*length of the string*/
  Bsc__putOctetStr(a_pb, (unsigned char*)a_p->m_tver_name, len); /*string*/

  /* ucode version info */
  Bsc__putUnsignedB (a_pb, &a_p->m_uver_maj, 32);
  Bsc__putUnsignedB (a_pb, &a_p->m_uver_min, 32);
  Bsc__putUnsignedB (a_pb, &a_p->m_uver_patch, 32);
  len = STRLEN_VN(a_p->m_uver_name);  /*calculate length of the string*/
  if (len > STRMAX_VN) return 9;
  Bsc__putUnsignedB(a_pb, &len, 8);         /*length of the string*/
  Bsc__putOctetStr(a_pb, (unsigned char*)a_p->m_uver_name, len); /*string*/

  /* other parameters */
  Bsc__putOctetStr(a_pb, a_p->m_md5a, sizeof (a_p->m_md5a)); /* save md5 checksum */ 

  Bsc__putUnsignedB (a_pb, &a_p->m_snum, 4);
  Bsc__putUnsignedB (a_pb, &a_p->m_inum, 12);
  Bsc__putUnsignedB (a_pb, &a_p->m_onum, 4);
  Bsc__putUnsignedB (a_pb, &a_p->m_ncnum, 20);
  Bsc__putUnsignedB (a_pb, &a_p->m_hwid, 8);

  /*task map*/
  tlen = (a_p->m_inum+7)/8; /* task map length in octets */
  /*printf ("debug printing: write tlen = %d\n", tlen);*/
  Bsc__putOctetStr (a_pb, a_p->m_tmap, tlen);

  /*code*/
  clen=a_p->m_snum * a_p->m_inum * a_p->m_onum ; /* code table length in octets */
  /*printf ("debug printing: write clen = %d\n", clen);*/
  Bsc__putOctetStr (a_pb, a_p->m_code, clen);

  /*NCAT table*/
  for (i=0; i<a_p->m_ncnum; i++) {
    fe2kAsm2Ncae__writeBsc(&a_p->m_ncat[i], a_pb);
    Bsc__putUnsignedB (a_pb, &pad, Bsc__aligningB(a_pb, 8));  /*pad, allign to the octet boundary*/
  }

  return 0;
}

/* deserialize all interface data from the buffer */
int fe2kAsm2IntD__readBsc(fe2kAsm2IntD *a_p, Bsc *a_pb)
{
  unsigned int 
    magic
    ,tlen=0
    ,clen=0
    ,i=0 
    ,pad=0 
    ,len=0
    ;
  unsigned char lmd5[16];

  if (a_p == NULL) return 1;
  if (a_pb == NULL) return 2;

  /* set structure flag to indicate dynamic allocation of the data buffers */
  a_p->m_fdin = TRUE_VN;

  Bsc__getUnsignedB (a_pb, &magic, 32); /*4 octets long magic number to identify binary form of package*/
  if(magic != a_p->m_magic) return 3;
  
  /* tool version info */
  Bsc__getUnsignedB (a_pb, &a_p->m_tver_maj,  32);/*ucode major version - change of the package format or major functionality*/
  Bsc__getUnsignedB (a_pb, &a_p->m_tver_min,  32);/*ucode minor version - other changes*/
  Bsc__getUnsignedB (a_pb, &a_p->m_tver_patch,32);/*ucode minor version - other changes*/
  Bsc__getUnsignedB (a_pb, &len, 8);         /*length of the string*/
  a_p->m_tver_name = (char*)MALLOC_VN(len+1, "fe2kAsm2IntD__readBsc"); /* allocate buffer for the string */
  if (a_p->m_tver_name == NULL) return 4;
  /*MEMSET_VN (a_p->m_tver_name, 0, len+1);*/
  Bsc__getOctetStr(a_pb, (unsigned char*)a_p->m_tver_name, len); /*name associated with the value, length limit is 128*/
  a_p->m_tver_name[len] = 0;

  /* ucode version info */
  Bsc__getUnsignedB (a_pb, &a_p->m_uver_maj,  32);/*ucode major version - change of the package format or major functionality*/
  Bsc__getUnsignedB (a_pb, &a_p->m_uver_min,  32);/*ucode minor version - other changes*/
  Bsc__getUnsignedB (a_pb, &a_p->m_uver_patch,32);/*ucode minor version - other changes*/
  Bsc__getUnsignedB (a_pb, &len, 8);         /*length of the string*/
  a_p->m_uver_name = (char*)MALLOC_VN(len+1, "fe2kAsm2IntD__readBsc"); /* allocate buffer for the string */
  if (a_p->m_uver_name == NULL) return 4;
  /*MEMSET_VN (a_p->m_uver_name, 0, len+1);*/
  Bsc__getOctetStr(a_pb, (unsigned char*)a_p->m_uver_name, len); /*name associated with the value, length limit is 128*/
  a_p->m_uver_name[len] = 0;

  /* other  parameters */
  Bsc__getOctetStr(a_pb, a_p->m_md5a, sizeof (a_p->m_md5a)); /* get md5 checksum */ 

  Bsc__getUnsignedB (a_pb, &a_p->m_snum, 4);   /*number of streams*/
  if (a_p->m_snum == 0) return 5;

  Bsc__getUnsignedB (a_pb, &a_p->m_inum, 12);  /*number of instructions*/
  if (a_p->m_inum == 0) return 6;

  Bsc__getUnsignedB (a_pb, &a_p->m_onum, 4);   /*number of octets in the machine word*/
  if (a_p->m_onum == 0) return 7;

  Bsc__getUnsignedB (a_pb, &a_p->m_ncnum, 20); /*number of entries in the NCAT table*/
  Bsc__getUnsignedB (a_pb, &a_p->m_hwid, 8); /*hw version*/

  /* calculate task and code tablelength in octets */
  tlen = (a_p->m_inum+7)/8; /* task map length in octets */
  clen=a_p->m_snum * a_p->m_inum * a_p->m_onum ; /* code table length in octets */
  /*printf ("debug printing: write tlen = %d, clen = %d\n", tlen, clen);*/

  /*allocate and read from buffer task map table*/
  a_p->m_tmap = (unsigned char *)MALLOC_VN(tlen, "fe2kAsm2IntD__readBsc");
  if (a_p->m_tmap == NULL) return 8;
  Bsc__getOctetStr (a_pb, a_p->m_tmap, tlen);

  /*allocate and read from buffer code table*/
  a_p->m_code = (unsigned char *)MALLOC_VN(clen, "fe2kAsm2IntD__readBsc");
  if (a_p->m_code == NULL) return 9;
  Bsc__getOctetStr (a_pb, a_p->m_code, clen);

  /*allocate and read NCAT table*/
  a_p->m_ncat = (fe2kAsm2Ncae*)MALLOC_VN(a_p->m_ncnum * sizeof (fe2kAsm2Ncae), "fe2kAsm2IntD__readBsc");
  if (a_p->m_ncnum && a_p->m_ncat == NULL) return 10;
  for (i=0; i<a_p->m_ncnum; i++) {
    if (fe2kAsm2Ncae__readBsc(&a_p->m_ncat[i], a_pb)) return 11;
    Bsc__getUnsignedB (a_pb, &pad, Bsc__aligningB(a_pb, 8));  /*pad, allign to the octet boundary*/
  }

  /* verify md5 sum to make sure important data are not corrupted */
  if (fe2kAsm2IndD__md5a (a_p, lmd5)) return 12;
  for (i=0; i< sizeof (lmd5); i++) if (lmd5[i] != a_p->m_md5a[i]) return 2; /* data are corrupted */

  return 0;
}

/* copy interface structure a_b to interface structure a_a with dynamic buffers */
int fe2kAsm2IndD__copyD(fe2kAsm2IntD *a_d, fe2kAsm2IntD *a_s)
{
  unsigned int tlen=0, clen=0, i=0, len=0;

  if (a_d == NULL) return 1;
  if (a_s == NULL) return 2;

  if (fe2kAsm2IntD__init(a_d)) return 3;
  if (a_d->m_magic != a_s->m_magic) return 4;

  /* tool version info */
  a_d->m_tver_maj   = a_s->m_tver_maj;
  a_d->m_tver_min   = a_s->m_tver_min;
  a_d->m_tver_patch = a_s->m_tver_patch;
  /* allocate and copy m_uver_name string */
  len = STRLEN_VN(a_s->m_tver_name);  /*calculate length of the string*/
  if (len > STRMAX_VN) return 5;
  a_d->m_tver_name = (char*)MALLOC_VN(len+1, "fe2kAsm2IndD__copyD"); /* allocate buffer for the string */
  if (a_d->m_tver_name == NULL) return 6;
  STRNCPY_VN(a_d->m_tver_name, a_s->m_tver_name, len+1);

  /* ucode version info */
  a_d->m_uver_maj   = a_s->m_uver_maj;
  a_d->m_uver_min   = a_s->m_uver_min;
  a_d->m_uver_patch = a_s->m_uver_patch;
  /* allocate and copy m_uver_name string */
  len = STRLEN_VN(a_s->m_uver_name);  /*calculate length of the string*/
  if (len > STRMAX_VN) return 5;
  a_d->m_uver_name = (char*)MALLOC_VN(len+1, "fe2kAsm2IndD__copyD"); /* allocate buffer for the string */
  if (a_d->m_uver_name == NULL) return 6;
  STRNCPY_VN(a_d->m_uver_name, a_s->m_uver_name, len+1);

  /* other parameters */
  a_d->m_snum       = a_s->m_snum;
  a_d->m_inum       = a_s->m_inum;
  a_d->m_onum       = a_s->m_onum;
  a_d->m_ncnum      = a_s->m_ncnum;
  a_d->m_hwid       = a_s->m_hwid;
  a_d->m_fdin       = TRUE_VN;
  a_d->m_fPut       = a_s->m_fPut;
  a_d->m_fGet       = a_s->m_fGet;
  a_d->m_pv         = a_s->m_pv;


  /*allocate and copy task map table*/
  tlen = (a_s->m_inum+7)/8; /* task map length in octets */
  a_d->m_tmap = (unsigned char *)MALLOC_VN(tlen, "fe2kAsm2IndD__copyD");
  if (a_d->m_tmap == NULL) return 7;
  for (i=0; i<tlen; i++) a_d->m_tmap[i] = a_s->m_tmap[i];

  /*allocate and copy code table*/
  clen = a_s->m_snum * a_s->m_inum * a_s->m_onum ; /* code table length in octets */
  a_d->m_code = (unsigned char *)MALLOC_VN(clen, "fe2kAsm2IndD__copyD");
  if (a_d->m_code == NULL) return 8;
  for (i=0; i<clen; i++) a_d->m_code[i] = a_s->m_code[i];

  /*calculate size of the NCAT table and allocate it*/
  a_d->m_ncat = (fe2kAsm2Ncae*)MALLOC_VN(a_s->m_ncnum * sizeof (fe2kAsm2Ncae), "fe2kAsm2IndD__copyD");
  if (a_d->m_ncnum && a_d->m_ncat == NULL) return 9;
  
  /* copy ncat table entries */
  for (i=0; i<a_d->m_ncnum; i++)
    if (fe2kAsm2Ncae__copyD(&a_d->m_ncat[i], &a_s->m_ncat[i])) return 10;

  /* calculate and verify md5 sum to make sure important data are not corrupted */
  if (fe2kAsm2IndD__md5a (a_d, a_d->m_md5a)) return 11;
  for (i=0; i< sizeof (a_d->m_md5a); i++) if (a_d->m_md5a[i] != a_s->m_md5a[i]) return 12; /* corrupted data */

  return 0;
}


int fe2kAsm2IntD__lenBuf(fe2kAsm2IntD *a_p, unsigned int *a_l)
{
  Bsc bsc;

  if (a_p == NULL) return 1;
  if (a_l == NULL) return 2;

  Bsc__initFromBuf(&bsc, 0, 10000000); /* zero pointer to the buf will set nc flag in bsc*/
  if (fe2kAsm2IntD__writeBsc(a_p, &bsc)) return 3;
  *a_l = Bsc__numProO(&bsc);
  
  return 0;
}

int fe2kAsm2IntD__writeBuf(fe2kAsm2IntD *a_p, unsigned char *a_b, unsigned int a_l)
{
  Bsc bsc;

  /*validate interface structure parameters*/
  if (a_p == NULL) return 1;
  if (a_b == NULL) return 2;
  if (a_l == 0) return 3;
  
  Bsc__initFromBuf(&bsc, a_b, a_l*8);
  if (fe2kAsm2IntD__writeBsc(a_p, &bsc)) return 4;

  return 0;
}

int fe2kAsm2IntD__readBuf(fe2kAsm2IntD *a_p, unsigned char *a_b, unsigned int a_l)
{
  Bsc bsc;
  int rcode = 0;

  if (a_p == NULL) return 31;
  if (a_b == NULL) return 32;
  if (a_l == 0) return 33;

  Bsc__initFromBuf(&bsc, a_b, a_l*8);
  rcode = fe2kAsm2IntD__readBsc(a_p, &bsc);

  return rcode;
}

int fe2kAsm2IntD__getNc(fe2kAsm2IntD *a_p, char *a_n, unsigned int *a_nc) 
{
  int i=0;

  if (a_p  == NULL) return 1;
  if (a_n  == NULL) return 2;
  if (a_nc == NULL) return 3;
  if (a_p->m_magic != 0x12345678) return 4;


  for (i=0; i< a_p->m_ncnum; i++)
    if (!STRNCMP_VN(a_p->m_ncat[i].m_name, a_n, STRMAX_VN)) {
      fe2kAsm2Ncae *a_e = &a_p->m_ncat[i];
      *a_nc = a_e->m_va;

      if (a_p->m_fGet && a_p->m_ncat[i].m_fc) { /* if hw access functions are initialized and it is ucode entry, then get value from the microcode */
	unsigned char *wp;
	Bsc bsc;
	if (fe2kAsm2IntD__getWordPtr(a_p, a_e->m_sn, a_e->m_in, &wp)) return 4; /*calculate pointer to the first octet of the machine word*/
	if (a_p->m_fGet(a_p->m_pv, a_e->m_sn, a_e->m_in, wp)) return 5; /*if hardware access function is initialized then get value from hardware word and update data buffer*/
	Bsc__initFromBuf (&bsc, wp, a_p->m_onum*8); /*init bit stream class to the machine word*/
	Bsc__setLsbf (&bsc, FALSE_VN); /*set appropriate bsc attributes*/
	Bsc__setLsof (&bsc, FALSE_VN); /*set appropriate bsc attributes*/
	Bsc__moveB (&bsc, a_p->m_onum*8 - ( (a_e->m_bo-16) + a_e->m_bl), eBscBeg);/*move bit stream to the offset of the field, note then given offsets based on the 16+ values*/ 
	Bsc__getUnsignedB (&bsc, a_nc, a_e->m_bl);/*put new value to the machine word field*/
	if (a_p->m_ncat[i].m_va != *a_nc) { a_p->m_ncat[i].m_va = *a_nc; return 6; }
      } 
     
      return 0;
    }

  return 5;
}

int fe2kAsm2IntD__updateNcEntry(fe2kAsm2IntD *a_p, fe2kAsm2Ncae *a_e, unsigned int *a_nc) 
{
  unsigned char *wp;
  Bsc bsc;

  if (a_p  == NULL) return 1;
  if (a_e  == NULL) return 2;
  if (a_nc == NULL) return 3;

  a_e->m_va = *a_nc;
  if (a_e->m_fc) {
    if (fe2kAsm2IntD__getWordPtr(a_p, a_e->m_sn, a_e->m_in, &wp)) return 4; /*calculate pointer to the first octet of the machine word*/
    if (a_p->m_fGet) a_p->m_fGet(a_p->m_pv, a_e->m_sn, a_e->m_in, wp); /*if hardware access function is initialized then get value from hardware word and update data buffer*/
    Bsc__initFromBuf (&bsc, wp, a_p->m_onum*8); /*init bit stream class to the machine word*/
    Bsc__setLsbf (&bsc, FALSE_VN); /*set appropriate bsc attributes*/
    Bsc__setLsof (&bsc, FALSE_VN); /*set appropriate bsc attributes*/
    Bsc__moveB (&bsc, a_p->m_onum*8 - ( (a_e->m_bo-16) + a_e->m_bl), eBscBeg);/*move bit stream to the offset of the field, note then given offsets based on the 16+ values*/ 
    Bsc__putUnsignedB (&bsc, &a_e->m_va, a_e->m_bl);/*put new value to the machine word field*/
    if (a_p->m_fPut) a_p->m_fPut(a_p->m_pv, a_e->m_sn, a_e->m_in, wp); /*if hardware access function is initialized then put value back to  hardware word from data buffer*/
  }
  return 0;
}

int fe2kAsm2IntD__putNc(fe2kAsm2IntD *a_p, char *a_n, unsigned int *a_nc) 
{
  int lret=1, i=0;
  unsigned int lh=0;
  NcatQae *qe=0;

  if (a_p  == NULL) return 1;
  if (a_n  == NULL) return 2;
  if (a_nc == NULL) return 3;
  if (a_p->m_magic != 0x12345678) return 4;

  /* if quick access table is not initialized use just slow main named constants table access */
  if (a_p->m_qat == NULL) { 
    for (i=0; i< a_p->m_ncnum; i++) 
      if (!STRNCMP_VN(a_p->m_ncat[i].m_name, a_n, STRMAX_VN)) 
	lret = fe2kAsm2IntD__updateNcEntry(a_p, &a_p->m_ncat[i], a_nc);
    
  } else {/* if quick access table is initialized, use it for the fast access */
    /*calculate hash and get correct element on the tree*/
    if (fe2kAsm2IndD__calcHash(a_p, a_n, &lh)) return 2;
    for (qe = a_p->m_qat[lh]; qe; qe = qe->m_down) 
      if (!STRNCMP_VN(a_p->m_ncat[qe->m_ind].m_name, a_n, STRMAX_VN)) break;
    if (qe == NULL) return 3;

    /*make changes for all elements in the right list*/
    for (; qe; qe=qe->m_right) 
      lret = fe2kAsm2IntD__updateNcEntry(a_p, &a_p->m_ncat[qe->m_ind], a_nc);
  }

  return lret;
}

int fe2kAsm2IntD__getWordPtr(fe2kAsm2IntD *a_p, unsigned int a_sn, unsigned int a_in, unsigned char **a_wp)
{
  if (a_p  == NULL) return 1;
  if (a_wp == NULL) return 2;
  if (a_p->m_magic != 0x12345678) return 3;
  if (a_sn >= a_p->m_snum) return 4;
  if (a_in >= a_p->m_inum) return 5;

  *a_wp = &a_p->m_code[a_sn*a_p->m_inum*a_p->m_onum + a_in*a_p->m_onum];

  return 0;
}

/* calculate md5a ( for all significunt data fields and arrays ) */
int fe2kAsm2IndD__md5a (fe2kAsm2IntD *a_p, unsigned char a_d[16])
{
  /* local variables and objects */
  Md5c ctx;
  Bsc bs;
  unsigned char buf[4];
  unsigned int len = 0;

  /* validate function arguments */
  if (a_p == NULL) return 1;
  if (a_d == NULL) return 2;

  /* initialize used pseudo-classes */
  Bsc__initFromBuf(&bs, buf, 8* sizeof (buf)); 
  if (Md5c__init( &ctx )) return 3;

  /* calculate md5 sum for the magic field */
  Bsc__putUnsigned(&bs, a_p->m_magic, 8*sizeof (buf)); 
  if (Md5c__update( &ctx, Bsc__ptrPduO(&bs), sizeof (buf))) return 4; 

  /* ucode version info */
  /* calculate md5 sum for the m_uver_maj field*/ 
  Bsc__putUnsigned(&bs, a_p->m_uver_maj, 8*sizeof (buf)); 
  if (Md5c__update( &ctx, Bsc__ptrPduO(&bs), sizeof (buf))) return 7; 

  /* calculate md5 sum for the m_uver_min field*/ 
  Bsc__putUnsigned(&bs, a_p->m_uver_min, 8*sizeof (buf)); 
  if (Md5c__update( &ctx, Bsc__ptrPduO(&bs), sizeof (buf))) return 8; 

  /* calculate md5 sum for the m_uver_patch field*/ 
  Bsc__putUnsigned(&bs, a_p->m_uver_patch, 8*sizeof (buf)); 
  if (Md5c__update( &ctx, Bsc__ptrPduO(&bs), sizeof (buf))) return 9; 

  /* calculate md5 sum for the m_uver_name field*/   
  if (a_p->m_uver_name) {
    len = STRLEN_VN(a_p->m_uver_name);
    if (len > STRMAX_VN) return 10;
    if (Md5c__update( &ctx, (unsigned char*)a_p->m_uver_name,  len+1)) return 11;
  }

  /* calculate md5 sum for the task map*/ 
  if (Md5c__update( &ctx, a_p->m_tmap, (a_p->m_inum+7)/8)) return 12;

  /* calculate md5 sum for the code */ 
  if (Md5c__update( &ctx, a_p->m_code, a_p->m_snum * a_p->m_inum * a_p->m_onum)) return 13;

  /* finalize calculation of the md5 check-sum */
  if (Md5c__finish( &ctx, a_d )) return 14;

  return 0;
}

/******************************************************************************
 * end of file fe2k-asm2-intd.c
 *****************************************************************************/
