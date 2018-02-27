/*
$Id: KaminoDriver.h 1.2.298.1 Broadcom SDK $
$Copyright: Copyright 2011 Broadcom Corporation.
This program is the proprietary software of Broadcom Corporation
and/or its licensors, and may only be used, duplicated, modified
or distributed pursuant to the terms and conditions of a separate,
written license agreement executed between you and Broadcom
(an "Authorized License").  Except as set forth in an Authorized
License, Broadcom grants no license (express or implied), right
to use, or waiver of any kind with respect to the Software, and
Broadcom expressly reserves all rights in and to the Software
and all intellectual property rights therein.  IF YOU HAVE
NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE
IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
ALL USE OF THE SOFTWARE.  
 
Except as expressly set forth in the Authorized License,
 
1.     This program, including its structure, sequence and organization,
constitutes the valuable trade secrets of Broadcom, and you shall use
all reasonable efforts to protect the confidentiality thereof,
and to use this information only in connection with your use of
Broadcom integrated circuit products.
 
2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS
PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY,
OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
OUT OF USE OR PERFORMANCE OF THE SOFTWARE.

3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL,
INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER
ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE OF OR INABILITY
TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF
THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR USD 1.00,
WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING
ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.$
*/

#ifndef AB_KaminoDriver_H
#define AB_KaminoDriver_H


#include <soc/sbx/sbTypesGlue.h>

#define SAND_DRV_KA_STATUS_OK     0 
#define SAND_DRV_KA_STATUS_ERROR -1


int sandDrvKaSetDefaultProfile(uint32_t unit);
int sandDrvKaVerifyProfile(uint32_t unit);

int sandDrvKaQsRankWrite(uint32_t unit, uint32_t nAddress, uint32_t *pData);
int sandDrvKaQsRankRead(uint32_t unit,  uint32_t nAddress, uint32_t *pData);

int sandDrvKaQsRandWrite(uint32_t unit, uint32_t nAddress, uint32_t *pData);
int sandDrvKaQsRandRead(uint32_t unit,  uint32_t nAddress, uint32_t *pData);

int sandDrvKaQsLnaRankWrite(uint32_t unit, uint32_t nAddress, uint32_t *pData);
int sandDrvKaQsLnaRankRead(uint32_t unit,  uint32_t nAddress, uint32_t *pData);

int sandDrvKaQsLnaRandWrite(uint32_t unit, uint32_t nAddress, uint32_t *pData);
int sandDrvKaQsLnaRandRead(uint32_t unit,  uint32_t nAddress, uint32_t *pData);

int sandDrvKaQsMemWrite(uint32_t unit, uint32_t nTableId, uint32_t nAddress, uint32_t *pData);
int sandDrvKaQsMemRead(uint32_t unit,  uint32_t nTableId, uint32_t nAddress, uint32_t *pData);

int sandDrvKaQsMemLnaWrite(uint32_t unit, uint32_t nTableId, uint32_t nAddress, uint32_t *pData);
int sandDrvKaQsMemLnaRead(uint32_t unit,  uint32_t nTableId, uint32_t nAddress, uint32_t *pData);

int sandDrvKaQmMemWrite(uint32_t unit, uint32_t nTableId, uint32_t nAddress,  uint32_t *pData);
int sandDrvKaQmMemRead(uint32_t unit,  uint32_t nTableId, uint32_t nAddress,  uint32_t *pData);

int sandDrvKaQmFbCacheFifoWrite(uint32_t unit, uint32_t *pData);
int sandDrvKaQmFbCacheFifoRead(uint32_t unit,  uint32_t *pData);

int sandDrvKaPmMemWrite(uint32_t unit, uint32_t nAddress, uint32_t *pData);
int sandDrvKaPmMemRead(uint32_t unit,  uint32_t nAddress, uint32_t *pData);

int sandDrvKaSfMemWrite(uint32_t unit, uint32_t nInstance, uint32_t nAddress, uint32_t *pData);
int sandDrvKaSfMemRead(uint32_t unit,  uint32_t nInstance, uint32_t nAddress, uint32_t *pData);

int sandDrvKaEgMemWrite(uint32_t unit, uint32_t nTableId, uint32_t nAddress, uint32_t *pData);
int sandDrvKaEgMemRead(uint32_t unit,  uint32_t nTableId, uint32_t nAddress, uint32_t *pData);

int sandDrvKaEiMemWrite(uint32_t unit, uint32_t nTableId, uint32_t nAddress, uint32_t *pData0);
int sandDrvKaEiMemRead(uint32_t unit,  uint32_t nTableId, uint32_t nAddress, uint32_t *pData0);

int sandDrvKaEbMemWrite(uint32_t unit, uint32_t nAddress,  uint32_t pData[8]);
int sandDrvKaEbMemRead(uint32_t unit, uint32_t nAddress,  uint32_t pData[8]);

int sandDrvKaEpAmClMemWrite(uint32_t unit, uint32_t nAddress, uint32_t *pData);
int sandDrvKaEpAmClMemRead(uint32_t unit, uint32_t nAddress,  uint32_t *pData);

int sandDrvKaEpBmBfMemWrite(uint32_t unit, uint32_t nAddress, uint32_t *pData);
int sandDrvKaEpBmBfMemRead(uint32_t unit,  uint32_t nAddress, uint32_t *pData);

int sandDrvKaEpMmIpMemWrite(uint32_t unit, uint32_t nAddress, uint32_t *pData);
int sandDrvKaEpMmIpMemRead(uint32_t unit, uint32_t nAddress,  uint32_t *pData);

int sandDrvKaEpMmBfMemWrite(uint32_t unit, uint32_t nAddress, uint32_t *pData);
int sandDrvKaEpMmBfMemRead(uint32_t unit, uint32_t nAddress,  uint32_t *pData);

int sandDrvKaRbPolMemWrite(uint32_t unit, uint32_t nTableId, uint32_t nAddress, uint32_t *pData);
int sandDrvKaRbPolMemRead(uint32_t unit,  uint32_t nTableId, uint32_t nAddress, uint32_t *pData);

int sandDrvKaRbClassMemWrite(uint32_t unit, uint32_t nSpi, uint32_t nTableId, uint32_t nAddress, uint32_t *pData);
int sandDrvKaRbClassMemRead(uint32_t unit,  uint32_t nSpi, uint32_t nTableId, uint32_t nAddress, uint32_t *pData);

int sandDrvKaPmDllLutWrite(uint32_t unit, uint32_t nTableId, uint32_t nAddress, uint32_t *pData);
int sandDrvKaPmDllLutRead(uint32_t unit,  uint32_t nTableId, uint32_t nAddress, uint32_t *pData);

#ifdef __cplusplus
}
#endif



#endif
