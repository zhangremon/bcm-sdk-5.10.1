#ifndef __SBQE2000ELIBMEM_H__
#define __SBQE2000ELIBMEM_H__
/**
 * @file sbQe2000ElibMem.h elib private memory access API
 *
 * <pre>
 * ===============================================
 * ==  sbQe2000Elib.h - elib private memory access API  ==
 * ===============================================
 *
 * WORKING REVISION: $Id: sbQe2000ElibMem.h 1.3.110.1 Broadcom SDK $
 *
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
 * MODULE NAME:
 *
 *     sbQe2000ElibMem.h
 *
 * ABSTRACT:
 *
 *     elib private memory access API
 *
 * LANGUAGE:
 *
 *     C
 *
 * AUTHORS:
 *
 *     David R. Pickett
 *
 * CREATION DATE:
 *
 *     15-September-2004
 *</pre>
 */
#include <glue.h>
#include <sbElibStatus.h>

#define SB_QE2000_ELIB_IND_MEM_TIMEOUT            10000

/*
 * Classifier Memory
 */
#define SB_QE2000_ELIB_CL_MEM_MAX_OFFSET          0x400

/*
 * Bridge Function Memory
 */
#define SB_QE2000_ELIB_BF_MEM_MAX_OFFSET          0x4000

/*
 * Bridging Function PRI Memory
 */
#define SB_QE2000_ELIB_PRI_MEM_MAX_OFFSET         0x190
/*
 * Instruction Processor Memory
 */
#define SB_QE2000_ELIB_IP_MEM_MAX_OFFSET          0x8000

/*
 * Egress Buffer Memory
 */

#define SB_QE2000_ELIB_EB_MEM_MAX_OFFSET          0x7FFF
#define SB_QE2000_ELIB_EB_MVT_ROW_V               (12)

#define SB_QE2000_ELIB_EB_MVT_ROW_GET(_index, _mcgrpsz)    ( SB_QE2000_ELIB_EB_MEM_MAX_OFFSET - \
                                                    ( _index & (( 1 << ( SB_QE2000_ELIB_EB_MVT_ROW_V + _mcgrpsz ))-1)) )

#define SB_QE2000_ELIB_EB_MVT_COL_GET(_index, _mcgrpsz)    ( (_index >> ( SB_QE2000_ELIB_EB_MVT_ROW_V + _mcgrpsz )) & 0x3)

#define SB_QE2000_ELIB_EB_MVT_MIN(_mcgrpsz)                ( (1 << (SB_QE2000_ELIB_EB_MVT_ROW_V + _mcgrpsz)) * 3)
/*
 * Indirect Memory Access functions
 */

/**
 * Read the Classifier Memory
 * If the function returns success, data read is placed in pulData0 &
 * pulData1.
 *
 * @param pEp        A pointer to an SB_QE2000_ELIB_CONTEXT
 * @param ulOffset   Offset into the classifier memory
 * @param ulClrOnRd  Set to 1 to clear memory after the read, (for clearing counters).
 * @param *pulData0  Upon success this field is filled with the 1st longword read
 * @param *pulData1  Upon success this field is filled with the 2nd longword read
 * @return           Status.  0 indicates success - Non-Zero indicates failure
 */
sbElibStatus_et sbQe2000ElibClMemRead( sbhandle HalCtxt, uint32_t ulOffset, uint32_t ulClrOnRd, uint32_t *pulData0, uint32_t *pulData1 );

/**
 * Write the Classifier Memory
 *
 * @param pEp        A pointer to an SB_QE2000_ELIB_CONTEXT
 * @param ulOffset   Offset into the classifier memory
 * @param ulData0    1st longword to place into the classifier memory
 * @param ulData1    2nd longword to place into the classifier memory
 * @return           Status.  0 indicates success - Non-Zero indicates failure
 */
sbElibStatus_et sbQe2000ElibClMemWrite( sbhandle HalCtxt, uint32_t ulOffset, uint32_t ulData0, uint32_t ulData1 );

/**
 * Read the Bridging Function PRI Memory
 * If the function returns success, data read is placed in pulData0 &
 * pulData1.
 *
 * @param pEp        A pointer to an SB_QE2000_ELIB_CONTEXT
 * @param ulOffset   Offset into the bridging function memory
 * @param *pulData   Upon success this field is filled with the longword read
 * @return           Status.  0 indicates success - Non-Zero indicates failure
 */
sbElibStatus_et sbQe2000ElibPriMemRead( sbhandle HalCtxt, uint32_t ulOffset, uint32_t *pulData );

/**
 * Write the Bridging Function PRI Memory
 *
 * @param pEp        A pointer to an SB_QE2000_ELIB_CONTEXT
 * @param ulOffset   Offset into the classifier memory
 * @param ulData     longword placed into the BF memory
 * @return           Status.  0 indicates success - Non-Zero indicates failure
 */
sbElibStatus_et sbQe2000ElibPriMemWrite( sbhandle HalCtxt, uint32_t ulOffset, uint32_t ulData );

/**
 * Read the Bridging Function Memory
 * If the function returns success, data read is placed in pulData0 &
 * pulData1.  Note: this is exclusive of the PRI memory.
 *
 * @param pEp        A pointer to an SB_QE2000_ELIB_CONTEXT
 * @param ulOffset   Offset into the bridging function memory
 * @param ulClrOnRd  Clear the contents when reading.
 * @param *pulData0  Upon success this field is filled with the 1st longword read
 * @param *pulData1  Upon success this field is filled with the 2nd longword read
 * @return           Status.  0 indicates success - Non-Zero indicates failure
 */
sbElibStatus_et sbQe2000ElibBfMemRead( sbhandle HalCtxt, uint32_t ulOffset, uint32_t ulClrOnRd, uint32_t *pulData0, uint32_t *pulData1 );

/**
 * Write the Bridging Function Memory
 * Note: this is exclusive of the PRI memory.
 *
 * @param pEp        A pointer to an SB_QE2000_ELIB_CONTEXT
 * @param ulOffset   Offset into the bridging function memory
 * @param ulData0    1st longword to place into the bridging function memory
 * @param ulData1    2nd longword to place into the bridging function memory
 * @return           Status.  0 indicates success - Non-Zero indicates failure
 */
sbElibStatus_et sbQe2000ElibBfMemWrite( sbhandle HalCtxt, uint32_t ulOffset, uint32_t ulData0, uint32_t ulData1 );

/**
 * Read the Instruction Processor Memory
 * If the function returns success, data read is placed in pulData0 &
 * pulData1.  Note: this is exclusive of the PRI memory.
 *
 * @param pEp        A pointer to an SB_QE2000_ELIB_CONTEXT
 * @param ulOffset   Offset into the instruction processor memory
 * @param ulClrOnRd  Clear the contents when reading.
 * @param *pulData0  Upon success this field is filled with the 1st longword read
 * @param *pulData1  Upon success this field is filled with the 2nd longword read
 * @return           Status.  0 indicates success - Non-Zero indicates failure
 */
sbElibStatus_et sbQe2000ElibIpMemRead( sbhandle HalCtxt,
                                       uint32_t ulOffset,
                                       uint32_t ulClrOnRd,
                                       uint32_t *pulData0,
                                       uint32_t *pulData1 );

/**
 * Write the Instruction Processor Memory
 * Note: this is exclusive of the PRI memory.
 *
 * @param pEp        A pointer to an SB_QE2000_ELIB_CONTEXT
 * @param ulOffset   Offset into the instruction processor memory
 * @param ulData0    1st longword to place into the instruction processor memory
 * @param ulData1    2nd longword to place into the instruction processor memory
 * @return           Status.  0 indicates success - Non-Zero indicates failure
 */
sbElibStatus_et sbQe2000ElibIpMemWrite( sbhandle HalCtxt, uint32_t ulOffset, uint32_t ulData0, uint32_t ulData1 );

/**
 * Write the Egress Buffer Memory
 *
 * @param pEp        A pointer to an SB_QE2000_ELIB_CONTEXT
 * @param ulOffset   Offset into the egress buffer memory
 * @param ulData     8 longwords to place into the ebuff memory
 * @return           Status.  0 indicates success - Non-Zero indicates failure
 */
sbElibStatus_et sbQe2000ElibEbMemWrite( sbhandle HalCtxt,
                    uint32_t ulOffset,
                    uint32_t ulData[8]);
/**
 * Read the Egress Buffer Memory
 *
 * @param pEp        A pointer to an SB_QE2000_ELIB_CONTEXT
 * @param ulOffset   Offset into the egress buffer memory
 * @param pulData    Upon success these 8 longwords will be filled with the memory read
 * @return           Status.  0 indicates success - Non-Zero indicates failure
 */
sbElibStatus_et sbQe2000ElibEbMemRead( sbhandle HalCtxt,
                   uint32_t ulOffset,
                   uint32_t pulData[8]);

/**
 * Read a line (3 MVT entries) from the Egress Buffer Memory
 *
 * @param pEp        A pointer to an SB_QE2000_ELIB_CONTEXT
 * @param ulIndex    MVT entry index
 * @param pulData    Upon success these 8 longwords will be filled with the memory read
 * @return           Status.  0 indicates success - Non-Zero indicates failure
 */
sbElibStatus_et sbQe2000ElibMvtEntryReadRaw( sbhandle HalCtxt, uint32_t ulIndex, uint32_t pulData[8] );

/**
 * Write a line (3 MVT entries) to the Egress Buffer Memory
 *
 * @param pEp        A pointer to an SB_QE2000_ELIB_CONTEXT
 * @param ulIndex    MVT entry index
 * @param pulData    MVT Row to write (8 longwords)
 * @return           Status.  0 indicates success - Non-Zero indicates failure
 */
sbElibStatus_et sbQe2000ElibMvtEntryWriteRaw( sbhandle HalCtxt, uint32_t ulIndex, uint32_t pulData[8] );


void sbQe2000ElibDelay( int secs, int nsecs );


#endif
