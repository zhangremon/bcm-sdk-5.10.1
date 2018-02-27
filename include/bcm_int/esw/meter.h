/*
 * $Id: meter.h 1.5 Broadcom SDK $
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
 * This file contains FFP meter definitions internal to the BCM library.
 */

#ifndef _BCM_INT_METER_H
#define _BCM_INT_METER_H

void _bcm_ffp_packet_collect(int unit);
extern void _bcm_esw_meter_size_get(int unit, int *size);

typedef struct _counter_s
{
    uint32	ctr_prev;	/* Last read values of counters */
    uint64	ctr64_prev;	/* Last read values of 64-bit counters */
    uint64	ctr_val;        /* Software accumulated counters */
} _counter_t;

typedef struct _b_meter_s
{
    int		m_size;
    int         m_alloc_count;
    soc_mem_t	m_pic2meter[SOC_MAX_NUM_BLKS];
    soc_mem_t	m_pic2fpc[SOC_MAX_NUM_BLKS];
    SHR_BITDCL	*m_allocated[SOC_MAX_NUM_BLKS];
    _counter_t	*m_counters[SOC_MAX_NUM_BLKS];
} _b_meter_t;

typedef struct _b_ffpcounter_s
{
    int		fc_size;
    int         fc_alloc_count;
    soc_mem_t	fc_pic2fc[SOC_MAX_NUM_BLKS];
    SHR_BITDCL	*fc_allocated[SOC_MAX_NUM_BLKS];
    _counter_t	*fc_counters[SOC_MAX_NUM_BLKS];
} _b_ffpcounter_t;

#define METER_INIT(unit) \
	if (meter_info[unit] == NULL) { return (BCM_E_INIT); }
#define METER_PIC(unit, pic) \
	if (meter_info[unit]->m_allocated[pic] == NULL) \
		{ return (BCM_E_BADID); }
#define METER_ID(unit, id) \
	if (id == 0 || id >= meter_info[unit]->m_size)  \
           { return (BCM_E_BADID); }

#define	METER_ALLOC(unit, blk)	meter_info[unit]->m_allocated[blk]
#define	METER_OK(unit, blk)	(METER_INFO(unit, blk) != NULL)
#define	METER_ISSET(unit, blk, n)	SHR_BITGET(METER_ALLOC(unit, blk), n)
#define	METER_SET(unit, blk, n)	SHR_BITSET(METER_ALLOC(unit, blk), n)
#define	METER_CLR(unit, blk, n)	SHR_BITCLR(METER_ALLOC(unit, blk), n)
#define	METER_ZERO(unit, blk)	sal_memset(METER_ALLOC(unit, blk), 0, \
				SHR_BITALLOCSIZE(meter_info[unit]->m_size))

#define FFPCOUNTER_INIT(unit) \
	if (ffpcounter_info[unit] == NULL) { return (BCM_E_INIT); }
#define FFPCOUNTER_PIC(unit, pic) \
	if (ffpcounter_info[unit]->fc_allocated[pic] == NULL) \
           { return (BCM_E_BADID); }
#define FFPCOUNTER_ID(id) \
	if (id >= ffpcounter_info[unit]->fc_size)  \
           { return (BCM_E_BADID); }

#define	FFPCOUNTER_ALLOC(unit, blk)	\
	ffpcounter_info[unit]->fc_allocated[blk]
#define	FFPCOUNTER_OK(unit, blk)	(FFPCOUNTER_INFO(unit, blk) != NULL)
#define	FFPCOUNTER_ISSET(unit, blk, n)	\
	SHR_BITGET(FFPCOUNTER_ALLOC(unit, blk), n)
#define	FFPCOUNTER_SET(unit, blk, n)	\
	SHR_BITSET(FFPCOUNTER_ALLOC(unit, blk), n)
#define	FFPCOUNTER_CLR(unit, blk, n)	\
	SHR_BITCLR(FFPCOUNTER_ALLOC(unit, blk), n)
#define	FFPCOUNTER_ZERO(unit, blk)	\
	sal_memset(FFPCOUNTER_ALLOC(unit, blk), 0, \
			SHR_BITALLOCSIZE(ffpcounter_info[unit]->fc_size))

#define PIC(_u, _p)  SOC_PORT_BLOCK(_u, _p)

#define COUNTER_ATOMIC_DEF		int
#define COUNTER_ATOMIC_BEGIN(s)		((s) = sal_splhi())
#define COUNTER_ATOMIC_END(s)		(sal_spl(s))

#endif	/* !_BCM_INT_METER_H */
