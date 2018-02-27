/*
 * $Id: bitop.h 1.12 Broadcom SDK $
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
 * Bit Array Operations
 */

#ifndef _SHR_BITOP_H
#define _SHR_BITOP_H

#include <sal/types.h>

/* Base type for declarations */
#define	SHR_BITDCL		uint32
#define	SHR_BITWID		32

/* (internal) Number of SHR_BITDCLs needed to contain _max bits */
#define	_SHR_BITDCLSIZE(_max)	(((_max) + SHR_BITWID - 1) / SHR_BITWID)

/* Size for giving to malloc and memset to handle _max bits */
#define	SHR_BITALLOCSIZE(_max)	(_SHR_BITDCLSIZE(_max) * sizeof (SHR_BITDCL))

/* Declare bit array _n of size _max bits */
#define	SHR_BITDCLNAME(_n, _max) SHR_BITDCL	_n[_SHR_BITDCLSIZE(_max)]

/* (internal) Generic operation macro on bit array _a, with bit _b */
#define	_SHR_BITOP(_a, _b, _op)	\
        (((_a)[(_b) / SHR_BITWID]) _op (1U << ((_b) % SHR_BITWID)))

/* Specific operations */
#define	SHR_BITGET(_a, _b)	_SHR_BITOP(_a, _b, &)
#define	SHR_BITSET(_a, _b)	_SHR_BITOP(_a, _b, |=)
#define	SHR_BITCLR(_a, _b)	_SHR_BITOP(_a, _b, &= ~)

/* clear _c bits starting from _b in bit array _a */
#define SHR_BITCLR_RANGE(_a, _b, _c)            \
{                                               \
    if ((((_b) % SHR_BITWID) == 0) &&           \
        (((_c) % SHR_BITWID) == 0)) {           \
        sal_memset(&((_a)[(_b) / SHR_BITWID]),  \
                0, SHR_BITALLOCSIZE(_c));       \
    } else {                                    \
        int _i = 0;                             \
        while ((_i) < (_c)) {                   \
            SHR_BITCLR(_a, (_b) + _i);          \
            _i++;                               \
        }                                       \
    }                                           \
}

/* set _c bits starting from _b in bit array _a */
#define SHR_BITSET_RANGE(_a, _b, _c)            \
{                                               \
    if ((((_b) % SHR_BITWID) == 0) &&           \
        (((_c) % SHR_BITWID) == 0)) {           \
        sal_memset(&((_a)[(_b) / SHR_BITWID]),  \
                0xFF, SHR_BITALLOCSIZE(_c));    \
    } else {                                    \
        int _i = 0;                             \
        while ((_i) < (_c)) {                   \
            SHR_BITSET(_a, (_b) + _i);          \
            _i++;                               \
        }                                       \
    }                                           \
}

/*
 * Copy _e bits from bit array _c offset _d to bit array _a offset _b
 * There should be no overlap between source _c and desstination _a
 * _a[_b:_b + _e] = _c[_d:_d + _e]
 */
#define SHR_BITCOPY_RANGE(_a, _b, _c, _d, _e)   \
{                                               \
    if ((((_b) % SHR_BITWID) == 0) &&           \
        (((_d) % SHR_BITWID) == 0) &&           \
        (((_e) % SHR_BITWID) == 0)) {           \
        sal_memcpy(&((_a)[(_b) / SHR_BITWID]),  \
                   &((_c)[(_d) / SHR_BITWID]),  \
                    SHR_BITALLOCSIZE(_e));      \
    } else {                                    \
        int _i = 0;                             \
        while (_i < _e) {                       \
            if (SHR_BITGET(_c, (_d) + _i)) {    \
               SHR_BITSET(_a, (_b) + _i);       \
            } else {                            \
               SHR_BITCLR(_a, (_b) + _i);       \
	    }					\
            _i++;                               \
        }                                       \
    }                                           \
}

/* Result is 0 only if all bits in the range are 0 */

#define SHR_BITTEST_RANGE(_bits, _first, _bit_count, _result) \
    { \
        int _bit_index = 0; \
        _result = 0; \
        while (((_result) == 0) && (_bit_index < (_bit_count))) \
        { \
            _result |= SHR_BITGET(_bits, _first + (_bit_index)); \
            ++_bit_index; \
        } \
    }

#define _SHR_BITOP_RANGE(_bits1, _bits2, _first, _bit_count, _dest, _op) \
    { \
        int _bit_index = 0; \
        while (_bit_index < _bit_count) \
        { \
            if (SHR_BITGET(_bits1, _first + _bit_index) _op \
                SHR_BITGET(_bits2, _first + _bit_index)) \
            { \
                SHR_BITSET(_dest, _first + _bit_index); \
            } \
            else \
            { \
                SHR_BITCLR(_dest, _first + _bit_index); \
            } \
            ++_bit_index; \
        } \
    }

#define SHR_BITAND_RANGE(_bits1, _bits2, _first, _bit_count, _dest) \
    _SHR_BITOP_RANGE(_bits1, _bits2, _first, _bit_count, _dest, &&)

#define SHR_BITOR_RANGE(_bits1, _bits2, _first, _bit_count, _dest) \
    _SHR_BITOP_RANGE(_bits1, _bits2, _first, _bit_count, _dest, ||)

#define SHR_BITXOR_RANGE(_bits1, _bits2, _first, _bit_count, _dest) \
    _SHR_BITOP_RANGE(_bits1, _bits2, _first, _bit_count, _dest, ^)

#define SHR_BITREMOVE_RANGE(_bits1, _bits2, _first, _bit_count, _dest) \
    _SHR_BITOP_RANGE(_bits1, _bits2, _first, _bit_count, _dest, && ~)

#define SHR_BITNEGATE_RANGE(_bits1, _first, _bit_count, _dest) \
    { \
        int _bit_index = 0; \
        while (_bit_index < _bit_count) \
        { \
            if (SHR_BITGET(_bits1, _first + _bit_index)) \
            { \
                SHR_BITCLR(_dest, _first + _bit_index); \
            } \
            else \
            { \
                SHR_BITSET(_dest, _first + _bit_index); \
            } \
            ++_bit_index; \
        } \
    }

extern int shr_bitop_range_null(SHR_BITDCL *bits, int first, int range);
extern int shr_bitop_range_eq(SHR_BITDCL *bits1, SHR_BITDCL *bits2,
                         int first, int range);
extern void shr_bitop_range_count(SHR_BITDCL *bits, int first,
                                 int range, int *count);

#define SHR_BITNULL_RANGE(_bits, _first, _range) \
    (shr_bitop_range_null(_bits, _first, _range))
#define SHR_BITEQ_RANGE(_bits1, _bits2, _first, _range) \
    (shr_bitop_range_eq(_bits1, _bits2, _first, _range))
#define SHR_BITCOUNT_RANGE(_bits, _count, _first, _range) \
    shr_bitop_range_count(_bits, _first, _range, &(_count))

#endif	/* !_SHR_BITOP_H */
