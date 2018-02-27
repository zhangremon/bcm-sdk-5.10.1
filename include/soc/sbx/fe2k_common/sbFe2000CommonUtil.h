/* -*-  Mode:C; c-basic-offset:4 -*- */
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
 * $Id: sbFe2000CommonUtil.h 1.3.184.1 Broadcom SDK $
 *
 ******************************************************************************/
#ifndef _SB_FE_2000_COMMON_UTIL_H_
#define _SB_FE_2000_COMMON_UTIL_H_

#include "sbTypes.h"
#include <soc/sbx/fe2k_common/sbFe2000Common.h>

/*
 * Generic indirect read function
 *
 * @param userDeviceHandle Glue handle to the FE2000
 * @param raw              Flag to perform no integer interpretation
 * @param descending       Flag indicates data registers are in n->0 order
 * @param ctrlOffset       Offset of _ctrl register
 * @param address          Memory address to access
 * @param words            Number of (consecutive) _data registers for memory
 * @param data             Returns data read
 *
 * @return                 failure flag (TRUE == timeout)
 */
int
sbFe2000UtilReadIndir(sbhandle userDeviceHandle, int raw, int descending,
                      uint32_t ctrlOffset, uint32_t address, uint32_t words,
                      uint32_t *data);

/*
 * Generic indirect write function
 *
 * @param userDeviceHandle Glue handle to the FE2000
 * @param raw              Flag to perform no integer interpretation
 * @param descending       Flag indicates data registers are in n->0 order
 * @param ctrlOffset       Offset of _ctrl register
 * @param address          Memory address to access
 * @param words            Number of (consecutive) _data registers for memory
 * @param data             Data to write
 *
 * @return                 failure flag (TRUE == timeout)
 */
int
sbFe2000UtilWriteIndir(sbhandle userDeviceHandle, int raw, int descending,
                       uint32_t ctrlOffset, uint32_t address, uint32_t words,
                       uint32_t *data);

/*
 * Write data to an integrated Tri-Speed MAC (port specific or general reg)
 *
 * @param userDeviceHandle Glue handle to the FE2000
 * @param nAgmNum          Which AGM to address [0 or 1]
 * @param nPort            Which port to address [0..11 or 12 for gen reg]
 * @param uRegAddr         Register to write
 * @param uData            Data to write.
 *
 * @return                 Status, SB_FE2000_STS_OK_K, or failure code.
 */

uint32_t
sbFe2000UtilAgmWrite(sbhandle userDeviceHandle, int32_t nAgmNum, int32_t nPort,
                 uint32_t uRegAddr, uint32_t uData);

/*
 * Read data from an integrated Tri-Speed MAC (port specific or general reg)
 *
 * @param userDeviceHandle Glue handle to the FE2000
 * @param nAgmNum          Which AGM to address [0 or 1]
 * @param nPort            Which port to address [0..11 or 12 for gen reg]
 * @param uRegAddr         Register to read
 * @param puData           Location in which to place read data.
 *
 * @return                 Status, SB_FE2000_STS_OK_K, or failure code.
 */

uint32_t
sbFe2000UtilAgmRead(sbhandle userDeviceHandle, int32_t nAgmNum, int32_t nPort,
                uint32_t uRegAddr, uint32_t *puData);

void 
sbFe2000UtilApplySoftReset(sbhandle userDeviceHandle);

void 
sbFe2000UtilReleaseSoftReset(sbhandle userDeviceHandle);

uint32_t
sbFe2000UtilAgmMiiWrite(sbhandle userDeviceHandle, uint8_t bInternalPhy,uint32_t uPhyAddr, uint32_t uRegAddr, uint32_t uData);

uint32_t
sbFe2000UtilAgmMiiRead(sbhandle userDeviceHandle, uint8_t bInternalPhy, uint32_t uPhyAddr, uint32_t uRegAddr, uint32_t *puData);

uint32_t
sbFe2000UtilAgmGportWrite(sbhandle userDeviceHandle, uint32_t nAgmNum, uint32_t uPort, 
		      uint32_t uRegAddr, uint32_t uData);
uint32_t
sbFe2000UtilAgmGportRead(sbhandle userDeviceHandle, uint32_t nAgmNum, uint32_t uPort, 
		     uint32_t uRegAddr);
unsigned int
sbFe2000UtilIICRead(	int unit,
			unsigned int slave_dev_addr,
			unsigned int reg_index,
			unsigned int *data);

unsigned int
sbFe2000UtilIICWrite(	int unit,
			unsigned int slave_dev_addr,
			unsigned int reg_index,
			unsigned int data);

unsigned int
sbFe2000UtilPciIICRead(int unit, unsigned int uRegAddr, unsigned int *puData);

unsigned int
sbFe2000UtilPciIICWrite(int unit, unsigned int uRegAddr, unsigned int uData);

uint32_t 
sbFe2000UtilAgmMacErrorMaskCalcAddr(uint16_t nNum, uint16_t nPort,sbhandle userDeviceHandle);

void
sbFe2000UtilSetupCmSegment(  sbhandle userDeviceHandle, uint32_t uCmSegment, uint32_t uCntrType, uint32_t uMmuDestBank, uint32_t uEjectLocation,
			     uint32_t uMmuBankBase, uint32_t uLimit, uint32_t uMemAddrConsumed );

/*
 * Write data to an integrated 10G MAC
 *
 * @param userDeviceHandle Glue handle to the FE2000
 * @param nAgmNum          Which AGM to address [0 or 1]
 * @param nPort            Which port to address [0..11 or 12 for gen reg]
 * @param uRegAddr         Register to write
 * @param uDataHi          Data to write (bits [63:32]).
 * @param uDataLo          Data to write (bits [31:0]).
 *
 * @return                 Status, SB_FE2000_STS_OK_K, or failure code.
 */

uint32_t
sbFe2000UtilXgmWrite(sbhandle userDeviceHandle, int32_t nXgmNum, uint32_t uRegAddr,
                 uint32_t uDataHi, uint32_t uDataLo);

/*
 * Read data from an integrated 10G MAC
 *
 * @param userDeviceHandle Glue handle to the FE2000
 * @param nAgmNum          Which AGM to address [0 or 1]
 * @param nPort            Which port to address [0..11 or 12 for gen reg]
 * @param uRegAddr         Register to write
 * @param puDataHi         Location in which to place read data (bits [63:32]).
 * @param puDataLo         Location in which to place read data (bits [31:0]).
 *
 * @return                 Status, SB_FE2000_STS_OK_K, or failure code.
 */
uint32_t
sbFe2000UtilXgmRead(sbhandle userDeviceHandle, int32_t nXgmNum,
                uint32_t uRegAddr, uint32_t *puDataHi, uint32_t *puDataLo);
uint32_t
sbFe2000UtilXgmBigMacRead(sbhandle userDeviceHandle,uint32_t uXgmNum, uint32_t uRegAddr,
		      uint32_t *pDataHi, uint32_t *pDataLo);
uint32_t
sbFe2000UtilXgmBigMacWrite(sbhandle userDeviceHandle,uint32_t uXgmNum, uint32_t uRegAddr, 
		       uint32_t uDataHi, uint32_t uDataLo);
uint32_t
sbFe2000UtilXgmMiimRead(sbhandle userDeviceHandle, uint8_t bInternalPhy, uint8_t bClause45,
			uint32_t uDevAddr, uint32_t uPhyOrPortAddr, uint32_t uRegAddr,
			uint32_t *pData);

uint32_t
sbFe2000UtilXgmMiimWrite(sbhandle userDeviceHandle,uint8_t bInternalPhy, uint8_t bClause45,
			 uint32_t uDevAddr, uint32_t uPhyOrPortAddr, uint32_t uRegAddr, 
			 uint32_t uData);

/* helper routines */
void CaSetDataWord(uint32_t * pWord, uint8_t *pDataBytes);
void CaSetDataBytes( uint32_t uData, uint8_t *pBytes);
#endif /* _SB_FE_2000_UTIL_H_ */
