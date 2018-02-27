/* -*- mode:c++; c-style:k&r; c-basic-offset:2; indent-tabs-mode: nil; -*- */
/* vi:set expandtab cindent shiftwidth=2 cinoptions=\:0l1(0t0g0: */
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
 * $Id: sbCrc32.c 1.3.112.1 Broadcom SDK $
 *
 * sbCrc32.c: the hash function
 *
 ******************************************************************************/

#include "sbTypes.h"
#include "soc/drv.h"
#include <soc/sbx/fe2k_common/sbCrc32.h>

/************************************************************************/
/* Hash routine to mimic the LRP hash instruction. */

static uint32_t bytetable[] = {
	0x00000000U, 0x04c11db7U, 0x09823b6eU, 0x0d4326d9U,
	0x130476dcU, 0x17c56b6bU, 0x1a864db2U, 0x1e475005U,
	0x2608edb8U, 0x22c9f00fU, 0x2f8ad6d6U, 0x2b4bcb61U,
	0x350c9b64U, 0x31cd86d3U, 0x3c8ea00aU, 0x384fbdbdU,
	0x4c11db70U, 0x48d0c6c7U, 0x4593e01eU, 0x4152fda9U,
	0x5f15adacU, 0x5bd4b01bU, 0x569796c2U, 0x52568b75U,
	0x6a1936c8U, 0x6ed82b7fU, 0x639b0da6U, 0x675a1011U,
	0x791d4014U, 0x7ddc5da3U, 0x709f7b7aU, 0x745e66cdU,
	0x9823b6e0U, 0x9ce2ab57U, 0x91a18d8eU, 0x95609039U, 
	0x8b27c03cU, 0x8fe6dd8bU, 0x82a5fb52U, 0x8664e6e5U,
	0xbe2b5b58U, 0xbaea46efU, 0xb7a96036U, 0xb3687d81U,
	0xad2f2d84U, 0xa9ee3033U, 0xa4ad16eaU, 0xa06c0b5dU,
	0xd4326d90U, 0xd0f37027U, 0xddb056feU, 0xd9714b49U,
	0xc7361b4cU, 0xc3f706fbU, 0xceb42022U, 0xca753d95U,
	0xf23a8028U, 0xf6fb9d9fU, 0xfbb8bb46U, 0xff79a6f1U,
	0xe13ef6f4U, 0xe5ffeb43U, 0xe8bccd9aU, 0xec7dd02dU,
	0x34867077U, 0x30476dc0U, 0x3d044b19U, 0x39c556aeU,
	0x278206abU, 0x23431b1cU, 0x2e003dc5U, 0x2ac12072U,
	0x128e9dcfU, 0x164f8078U, 0x1b0ca6a1U, 0x1fcdbb16U,
	0x018aeb13U, 0x054bf6a4U, 0x0808d07dU, 0x0cc9cdcaU,
	0x7897ab07U, 0x7c56b6b0U, 0x71159069U, 0x75d48ddeU,
	0x6b93dddbU, 0x6f52c06cU, 0x6211e6b5U, 0x66d0fb02U,
	0x5e9f46bfU, 0x5a5e5b08U, 0x571d7dd1U, 0x53dc6066U,
	0x4d9b3063U, 0x495a2dd4U, 0x44190b0dU, 0x40d816baU,
	0xaca5c697U, 0xa864db20U, 0xa527fdf9U, 0xa1e6e04eU,
	0xbfa1b04bU, 0xbb60adfcU, 0xb6238b25U, 0xb2e29692U,
	0x8aad2b2fU, 0x8e6c3698U, 0x832f1041U, 0x87ee0df6U,
	0x99a95df3U, 0x9d684044U, 0x902b669dU, 0x94ea7b2aU,
	0xe0b41de7U, 0xe4750050U, 0xe9362689U, 0xedf73b3eU,
	0xf3b06b3bU, 0xf771768cU, 0xfa325055U, 0xfef34de2U,
	0xc6bcf05fU, 0xc27dede8U, 0xcf3ecb31U, 0xcbffd686U,
	0xd5b88683U, 0xd1799b34U, 0xdc3abdedU, 0xd8fba05aU,
	0x690ce0eeU, 0x6dcdfd59U, 0x608edb80U, 0x644fc637U,
	0x7a089632U, 0x7ec98b85U, 0x738aad5cU, 0x774bb0ebU,
	0x4f040d56U, 0x4bc510e1U, 0x46863638U, 0x42472b8fU,
	0x5c007b8aU, 0x58c1663dU, 0x558240e4U, 0x51435d53U,
	0x251d3b9eU, 0x21dc2629U, 0x2c9f00f0U, 0x285e1d47U,
	0x36194d42U, 0x32d850f5U, 0x3f9b762cU, 0x3b5a6b9bU,
	0x0315d626U, 0x07d4cb91U, 0x0a97ed48U, 0x0e56f0ffU,
	0x1011a0faU, 0x14d0bd4dU, 0x19939b94U, 0x1d528623U,
	0xf12f560eU, 0xf5ee4bb9U, 0xf8ad6d60U, 0xfc6c70d7U,
	0xe22b20d2U, 0xe6ea3d65U, 0xeba91bbcU, 0xef68060bU,
	0xd727bbb6U, 0xd3e6a601U, 0xdea580d8U, 0xda649d6fU,
	0xc423cd6aU, 0xc0e2d0ddU, 0xcda1f604U, 0xc960ebb3U,
	0xbd3e8d7eU, 0xb9ff90c9U, 0xb4bcb610U, 0xb07daba7U,
	0xae3afba2U, 0xaafbe615U, 0xa7b8c0ccU, 0xa379dd7bU,
	0x9b3660c6U, 0x9ff77d71U, 0x92b45ba8U, 0x9675461fU,
	0x8832161aU, 0x8cf30badU, 0x81b02d74U, 0x857130c3U,
	0x5d8a9099U, 0x594b8d2eU, 0x5408abf7U, 0x50c9b640U,
	0x4e8ee645U, 0x4a4ffbf2U, 0x470cdd2bU, 0x43cdc09cU,
	0x7b827d21U, 0x7f436096U, 0x7200464fU, 0x76c15bf8U,
	0x68860bfdU, 0x6c47164aU, 0x61043093U, 0x65c52d24U,
	0x119b4be9U, 0x155a565eU, 0x18197087U, 0x1cd86d30U,
	0x029f3d35U, 0x065e2082U, 0x0b1d065bU, 0x0fdc1becU,
	0x3793a651U, 0x3352bbe6U, 0x3e119d3fU, 0x3ad08088U,
	0x2497d08dU, 0x2056cd3aU, 0x2d15ebe3U, 0x29d4f654U,
	0xc5a92679U, 0xc1683bceU, 0xcc2b1d17U, 0xc8ea00a0U,
	0xd6ad50a5U, 0xd26c4d12U, 0xdf2f6bcbU, 0xdbee767cU,
	0xe3a1cbc1U, 0xe760d676U, 0xea23f0afU, 0xeee2ed18U,
	0xf0a5bd1dU, 0xf464a0aaU, 0xf9278673U, 0xfde69bc4U,
	0x89b8fd09U, 0x8d79e0beU, 0x803ac667U, 0x84fbdbd0U,
	0x9abc8bd5U, 0x9e7d9662U, 0x933eb0bbU, 0x97ffad0cU,
	0xafb010b1U, 0xab710d06U, 0xa6322bdfU, 0xa2f33668U,
	0xbcb4666dU, 0xb8757bdaU, 0xb5365d03U, 0xb1f740b4U,
};

static uint32_t bytetable_reverse_index[] = {
 0x00,  0xa9,  0x57,  0xfe,
 0xae,  0x07,  0xf9,  0x50,
 0x59,  0xf0,  0x0e,  0xa7,
 0xf7,  0x5e,  0xa0,  0x09,
 0xb2,  0x1b,  0xe5,  0x4c,
 0x1c,  0xb5,  0x4b,  0xe2,
 0xeb,  0x42,  0xbc,  0x15,
 0x45,  0xec,  0x12,  0xbb,
 0x61,  0xc8,  0x36,  0x9f,
 0xcf,  0x66,  0x98,  0x31,
 0x38,  0x91,  0x6f,  0xc6,
 0x96,  0x3f,  0xc1,  0x68,
 0xd3,  0x7a,  0x84,  0x2d,
 0x7d,  0xd4,  0x2a,  0x83,
 0x8a,  0x23,  0xdd,  0x74,
 0x24,  0x8d,  0x73,  0xda,
 0xc3,  0x6a,  0x94,  0x3d,
 0x6d,  0xc4,  0x3a,  0x93,
 0x9a,  0x33,  0xcd,  0x64,
 0x34,  0x9d,  0x63,  0xca,
 0x71,  0xd8,  0x26,  0x8f,
 0xdf,  0x76,  0x88,  0x21,
 0x28,  0x81,  0x7f,  0xd6,
 0x86,  0x2f,  0xd1,  0x78,
 0xa2,  0x0b,  0xf5,  0x5c,
 0x0c,  0xa5,  0x5b,  0xf2,
 0xfb,  0x52,  0xac,  0x05,
 0x55,  0xfc,  0x02,  0xab,
 0x10,  0xb9,  0x47,  0xee,
 0xbe,  0x17,  0xe9,  0x40,
 0x49,  0xe0,  0x1e,  0xb7,
 0xe7,  0x4e,  0xb0,  0x19,
 0x82,  0x2b,  0xd5,  0x7c,
 0x2c,  0x85,  0x7b,  0xd2,
 0xdb,  0x72,  0x8c,  0x25,
 0x75,  0xdc,  0x22,  0x8b,
 0x30,  0x99,  0x67,  0xce,
 0x9e,  0x37,  0xc9,  0x60,
 0x69,  0xc0,  0x3e,  0x97,
 0xc7,  0x6e,  0x90,  0x39,
 0xe3,  0x4a,  0xb4,  0x1d,
 0x4d,  0xe4,  0x1a,  0xb3,
 0xba,  0x13,  0xed,  0x44,
 0x14,  0xbd,  0x43,  0xea,
 0x51,  0xf8,  0x06,  0xaf,
 0xff,  0x56,  0xa8,  0x01,
 0x08,  0xa1,  0x5f,  0xf6,
 0xa6,  0x0f,  0xf1,  0x58,
 0x41,  0xe8,  0x16,  0xbf,
 0xef,  0x46,  0xb8,  0x11,
 0x18,  0xb1,  0x4f,  0xe6,
 0xb6,  0x1f,  0xe1,  0x48,
 0xf3,  0x5a,  0xa4,  0x0d,
 0x5d,  0xf4,  0x0a,  0xa3,
 0xaa,  0x03,  0xfd,  0x54,
 0x04,  0xad,  0x53,  0xfa,
 0x20,  0x89,  0x77,  0xde,
 0x8e,  0x27,  0xd9,  0x70,
 0x79,  0xd0,  0x2e,  0x87,
 0xd7,  0x7e,  0x80,  0x29,
 0x92,  0x3b,  0xc5,  0x6c,
 0x3c,  0x95,  0x6b,  0xc2,
 0xcb,  0x62,  0x9c,  0x35,
 0x65,  0xcc,  0x32,  0x9b 
};

/* -------------------------------------------------------------------
 * Perform a hash on a 32bit value and return the result. The hash
 * algorithm is designed to operate identically to the microcode
 * instruction in the LRPs
 * -------------------------------------------------------------------*/
uint32_t
sbCrc32(uint32_t x)
{
  uint32_t i;

  for (i = 0; i < 4; i++)
    x = (x << 8) ^ bytetable[x >> 24];
    return x;
}

/* -------------------------------------------------------------------
 * Perform a "reverse" hash on a 32bit value and return the result. 
 * -------------------------------------------------------------------*/
uint32_t
sbCrc32_reverse(uint32_t x)
{
  uint32_t      key;
  unsigned char key_byte[4];
  int           index_a, index_b, index_c, index_d;
  
  index_a = x & 0xff;
  index_a = bytetable_reverse_index[index_a];
  
  index_b = ((x & 0xff00) >> 8) ^ 
            ((bytetable[index_a] & 0xff00) >> 8);
  index_b = bytetable_reverse_index[index_b];

  index_c = ((x & 0xff0000) >> 16) ^
            ((bytetable[index_a] & 0xff0000) >> 16) ^
            ((bytetable[index_b] & 0xff00) >> 8);
  index_c = bytetable_reverse_index[index_c];

  index_d = ((x & 0xff000000) >> 24) ^
            ((bytetable[index_a] & 0xff000000) >> 24) ^
            ((bytetable[index_b] & 0xff0000) >> 16) ^
            ((bytetable[index_c] & 0xff00) >> 8);
  index_d = bytetable_reverse_index[index_d];

  /* 
   * soc_cm_print("indexes %x %x %x %x \n", index_a, index_b, index_c, index_d);
   */

  key_byte[0] = index_d;
  key_byte[1] = index_c ^ ((bytetable[index_d] & 0xff000000) >> 24);
  key_byte[2] = index_b ^ ((bytetable[index_d] & 0xff0000) >> 16) ^
                          ((bytetable[index_c] & 0xff000000) >> 24);
  key_byte[3] = index_a ^ ((bytetable[index_d] & 0xff00) >> 8) ^
                          ((bytetable[index_c] & 0xff0000) >> 16) ^
                          ((bytetable[index_b] & 0xff000000) >> 24);

  key = (key_byte[0] << 24) + (key_byte[1] << 16) + (key_byte[2] << 8) + key_byte[3];
  return key;
}

/************************************************************************/
/* Routine to mimic the LRP popcnt instruction. */

static uint8_t poptbl[256] = {
        0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
        1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
        1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
        1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
        3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
        1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
        3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
        3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
        3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
        4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
};

/* -------------------------------------------------------------------
 * The microcode in the LRPs has a popcnt instruction. This function
 * emulates that instruction.
 *
 * The input to popcnt is a 32bit number. Popcnt returns the number of
 * bits set to one.
 * -------------------------------------------------------------------*/
uint32_t
sbPopcnt(uint32_t x)
{
        return  poptbl[(x >>  0) & 0xff] +
                poptbl[(x >>  8) & 0xff] +
                poptbl[(x >> 16) & 0xff] +
                poptbl[(x >> 24) & 0xff];
}
