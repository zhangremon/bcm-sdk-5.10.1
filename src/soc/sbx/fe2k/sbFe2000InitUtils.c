/* -*-  Mode:C; c-basic-offset:4 -*- */
/*****************************************************************************
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
 * $Id: sbFe2000InitUtils.c 1.5.186.1 Broadcom SDK $
 *
 * Utilities to facilitate bringup
 * ******************************************************************************/

#include "sbFe2000InitUtils.h"
#include "sbFe2000ImfDriver.h"
#include "soc/drv.h"
#include "hal_ca_auto.h"
#include "sbZfFe2000QmQueueConfigEntryConsole.hx"
#include "sbZfFe2000QmQueueState0EnEntryConsole.hx"
#include "sbFe2000CommonUtil.h"

/* gnats 28299 - track per unit, per map, per port - queue writes */
#define P2Q_NUM_MAPS         7
#define P2Q_NUM_ADDRESSES   64
static uint8_t p2q[SOC_MAX_NUM_DEVICES][P2Q_NUM_MAPS][P2Q_NUM_ADDRESSES];


/* FIX: function removed for sdk but needed for sbFe2000Diags.c */
#define SB_FE2000_QM_FLOW_CONTROL_PORT 0
/* modifed AddQueue method from verification */
void AddQueue(sbhandle userDeviceHandle, uint32_t uInPre, uint32_t uInPort, uint32_t uOutPte,
              uint32_t uOutPort, uint32_t uQueue) {

  /* rgf - May 24 2006 - Write the appropriate PRE memory to map the inbound port to the */
  /* rgf - May 24 2006 - allocated queue. */
  CaPrPortToQueueWrite( userDeviceHandle, uInPre, uInPort, uQueue );

  /* rgf - May 15 2006 - Now enable that queue */
  /* rgf - May 15 2006 - enable=1,min_pages=1,max_pages=100,dropthresh2=80 */
  {

    sbZfFe2000QmQueueConfigEntry_t zQueueConfig;
    sbZfFe2000QmQueueConfigFlowControlPort_t zFlowControlPort;

    sbZfFe2000QmQueueConfigEntry_InitInstance(&zQueueConfig);
    sbZfFe2000QmQueueConfigFlowControlPort_InitInstance(&zFlowControlPort);

    CaQmQueueConfigRead( userDeviceHandle, uQueue, &zQueueConfig);
    zQueueConfig.m_uEnforcePoliceMarkings = 0;
    zQueueConfig.m_uFlowControlEnable = 0;
    zQueueConfig.m_uFlowControlThresh2 = 0;
    zQueueConfig.m_uFlowControlThresh1 = 0;
    zQueueConfig.m_uDropThresh2 = 80;
    zQueueConfig.m_uMaxPages = 100;
    zQueueConfig.m_uMinPages = 1;

    /* rgf - Jul 31 2006 - Associate any flow control with the inbound port/interface */

    zQueueConfig.m_uFlowControlTarget = SB_FE2000_QM_FLOW_CONTROL_PORT;
    zFlowControlPort.m_uFlowControlInterface = uInPre;
    zFlowControlPort.m_uFlowControlPort = uInPort;
    zQueueConfig.m_uFlowControlPort = zFlowControlPort.m_uFlowControlPort;

    /* jts - Oct 30 2006 - zQueueConfig.SetFlowControlInterface(uInPre); */
    /* jts - Oct 30 2006 - zQueueConfig.SetFlowControlPort(uInPort); */

    /* rgf - Jun 14 2006 - Queue is always enabled when added */
    zQueueConfig.m_uEnable = 1;
    CaQmQueueConfigWrite( userDeviceHandle, uQueue, &zQueueConfig);

  }

  /* rgf - May 15 2006 - Now write the enable for queue state 0 */
  {
    /* bms - Nov 08 2006 - Since only QueueState0 is back door and since we are only setting this enable bit before */
    /* bms - Nov 08 2006 - passing any traffic we can safely do a RMW */

    sbZfFe2000QmQueueState0Entry_t zQueueState;
    sbZfFe2000QmQueueState0Entry_InitInstance(&zQueueState);

    CaQmQueueState0Read( userDeviceHandle,uQueue, &zQueueState );
    zQueueState.m_uEnable = 1;
    zQueueState.m_uEmpty = 1;
    CaQmQueueState0Write( userDeviceHandle,uQueue, &zQueueState );
  }

  /* rgf - May 25 2006 - write PT queue to port and port to queue mappings */
  {
    sbZfFe2000PtQueueToPortEntry_t zQueueToPort;
    sbZfFe2000PtQueueToPortEntry_InitInstance(&zQueueToPort);
    zQueueToPort.m_uPpeBound = 0;
    zQueueToPort.m_uPpeEnqueue = 0;
    zQueueToPort.m_uDestInterface = uOutPte;
    zQueueToPort.m_uDestPort = uOutPort;
    zQueueToPort.m_uDestQueue = 0;

    CaPtQueueToPortWrite( userDeviceHandle,uQueue, &zQueueToPort );
    CaPtPortToQueueWrite( userDeviceHandle,uOutPte, uOutPort, uQueue );
  }
}

void
CaP2qShadowWrite(int unit, uint8_t uPte, uint8_t uAddress, uint8_t uQueue)
{
    p2q[unit][uPte][uAddress] = uQueue;
}

void
CaP2qShadowRead(int unit, uint8_t uPte, uint8_t uAddress, uint8_t *pQueue)
{
    *pQueue = p2q[unit][uPte][uAddress];
}


uint8_t CaPtPortToQueueRead( sbhandle userDeviceHandle, uint32_t uPte, uint32_t uAddress, uint32_t *uQueue) {
  int unit = (int) userDeviceHandle;

  if (SOC_UNIT_VALID(unit) && (uPte < P2Q_NUM_MAPS) && 
      (uAddress < P2Q_NUM_ADDRESSES)) 
  {
      uint8_t tmp;
      CaP2qShadowRead(unit, uPte, uAddress, &tmp);
      *uQueue = tmp;
      return TRUE;
  }
  return FALSE;
}

uint8_t CaPtPortToQueueWrite( sbhandle userDeviceHandle, uint32_t uPte, uint32_t uAddress, const uint32_t uQueue) {
  uint8_t bSuccess = FALSE;
  int unit = (int)userDeviceHandle;
  sbZfFe2000PtPortToQueueEntry_t zQueueEntry;
  sbZfFe2000PtPortToQueueEntry_InitInstance(&zQueueEntry);

  zQueueEntry.m_uQueue = uQueue;

  if (SOC_UNIT_VALID(unit) && (uPte < P2Q_NUM_MAPS) && 
      (uAddress < P2Q_NUM_ADDRESSES)) 
  {
      CaP2qShadowWrite(unit, uPte, uAddress, uQueue);
  }


  switch( uPte ) {
  case 0:
    bSuccess = CaPtPortToQueueSt0Write( userDeviceHandle, uAddress, &zQueueEntry);
    break;
  case 1:
    bSuccess = CaPtPortToQueueSt1Write( userDeviceHandle, uAddress, &zQueueEntry);
    break;
  case 2:
    bSuccess = CaPtPortToQueueAg0Write( userDeviceHandle, uAddress, &zQueueEntry);
    break;
  case 3:
    bSuccess = CaPtPortToQueueAg1Write( userDeviceHandle, uAddress, &zQueueEntry);
    break;
  case 4:
    bSuccess = CaPtPortToQueueXg0Write( userDeviceHandle, uAddress, &zQueueEntry);
    break;
  case 5:
    bSuccess = CaPtPortToQueueXg1Write( userDeviceHandle, uAddress, &zQueueEntry);
    break;
  case 6:
    bSuccess = CaPtPortToQueuePciWrite( userDeviceHandle, uAddress, &zQueueEntry);
    break;
  default:
    SB_ERROR( "%s: Invalid PTE %d specified.", __PRETTY_FUNCTION__,uPte);
  }
  return bSuccess;
}

uint8_t CaPrPortToQueueRead( sbhandle userDeviceHandle,uint32_t uPre, uint32_t uAddress, uint32_t *uQueue) {
  uint32_t uData[1];
  uint8_t uData_bytes[4];
  sbFe2000ImfDriverStatus_t status;
  sbZfFe2000PrPortToQueueSr0Entry_t zQueueEntry;

  switch( uPre ) {
  case 0:
      status = sbFe2000ImfDriver_PrPortToQueueSr0Read((uint32_t)userDeviceHandle, uAddress, uData);
      break;
  case 1:
      status = sbFe2000ImfDriver_PrPortToQueueSr1Read((uint32_t)userDeviceHandle, uAddress, uData);
      break;
   case 2:
      status = sbFe2000ImfDriver_PrPortToQueueAg0Read((uint32_t)userDeviceHandle, uAddress, uData);
      break;
  case 3:
      status = sbFe2000ImfDriver_PrPortToQueueAg1Read((uint32_t)userDeviceHandle, uAddress, uData);
      break;
  case 4:
      status = sbFe2000ImfDriver_PrPortToQueueXg0Read((uint32_t)userDeviceHandle, uAddress, uData);
      break;
  case 5:
      status = sbFe2000ImfDriver_PrPortToQueueXg1Read((uint32_t)userDeviceHandle, uAddress, uData);
      break;
  case 6:
      status = sbFe2000ImfDriver_PrPortToQueuePciRead((uint32_t)userDeviceHandle, uAddress, uData);
      break;
  default:
    SB_ERROR( "%s: Invalid PRE %d specified.", __PRETTY_FUNCTION__,uPre);
    return FALSE;
  }
  if ( status != SAND_DRV_CA_STATUS_OK ) {
      SB_ERROR("%s:Got bad status 0x%08x", __PRETTY_FUNCTION__,status);
      return FALSE;
  }

  CaSetDataBytes(uData[0],&uData_bytes[0]);
  sbZfFe2000PrPortToQueueSr0Entry_Unpack(&zQueueEntry, uData_bytes, 1);
  *uQueue = zQueueEntry.m_uQueue;
  return TRUE;

}

uint8_t CaPrPortToQueueWrite( sbhandle userDeviceHandle,uint32_t uPre, uint32_t uAddress, const uint32_t uQueue) {
  uint8_t bSuccess = FALSE;
  switch( uPre ) {
  case 0:
    {
      sbZfFe2000PrPortToQueueSr0Entry_t zQueueEntry;
      sbZfFe2000PrPortToQueueSr0Entry_InitInstance(&zQueueEntry);
      zQueueEntry.m_uQueue = uQueue;
      bSuccess = CaPrPortToQueueSr0Write( userDeviceHandle, uAddress, &zQueueEntry);
    }
    break;
  case 1:
    {
      sbZfFe2000PrPortToQueueSr1Entry_t zQueueEntry;
      sbZfFe2000PrPortToQueueSr1Entry_InitInstance(&zQueueEntry);
      zQueueEntry.m_uQueue = uQueue;
      bSuccess = CaPrPortToQueueSr1Write( userDeviceHandle, uAddress, &zQueueEntry);
    }
    break;
  case 2:
    {
      sbZfFe2000PrPortToQueueAg0Entry_t zQueueEntry;
      sbZfFe2000PrPortToQueueAg0Entry_InitInstance(&zQueueEntry);
      zQueueEntry.m_uQueue = uQueue;
      bSuccess = CaPrPortToQueueAg0Write( userDeviceHandle, uAddress, &zQueueEntry);
    }
    break;
  case 3:
    {
      sbZfFe2000PrPortToQueueAg1Entry_t zQueueEntry;
      sbZfFe2000PrPortToQueueAg1Entry_InitInstance(&zQueueEntry);
      zQueueEntry.m_uQueue = uQueue;
      bSuccess = CaPrPortToQueueAg1Write( userDeviceHandle, uAddress, &zQueueEntry);
    }
    break;
  case 4:
    {
      sbZfFe2000PrPortToQueueXg0Entry_t zQueueEntry;
      sbZfFe2000PrPortToQueueXg0Entry_InitInstance(&zQueueEntry);
      zQueueEntry.m_uQueue = uQueue;
      bSuccess = CaPrPortToQueueXg0Write( userDeviceHandle, uAddress, &zQueueEntry);
    }
    break;
  case 5:
    {
      sbZfFe2000PrPortToQueueXg1Entry_t zQueueEntry;
      sbZfFe2000PrPortToQueueXg1Entry_InitInstance(&zQueueEntry);
      zQueueEntry.m_uQueue = uQueue;
      bSuccess = CaPrPortToQueueXg1Write( userDeviceHandle, uAddress, &zQueueEntry);
    }
    break;
  case 6:
    {
      sbZfFe2000PrPortToQueuePciEntry_t zQueueEntry;
      sbZfFe2000PrPortToQueuePciEntry_InitInstance(&zQueueEntry);
      zQueueEntry.m_uQueue = uQueue;
      bSuccess = CaPrPortToQueuePciWrite( userDeviceHandle, uAddress, &zQueueEntry);
    }
    break;
  default:
    SB_ERROR( "%s: Invalid PRE %d specified.", __PRETTY_FUNCTION__,uPre);
  }
  return bSuccess;
}

uint8_t CaQmQueueState0Read( sbhandle userDeviceHandle,uint32_t uAddress, sbZfFe2000QmQueueState0Entry_t *pZFrame ) {
  uint32_t uData[1] = {0};
  uint8_t uData_bytes[4]={0};
  sbFe2000ImfDriverStatus_t status;
  status = sbFe2000ImfDriver_QmQueueState0Read((uint32_t)userDeviceHandle, uAddress, &uData[0]);
  if ( status == SAND_DRV_CA_STATUS_OK ) {
    /* good */
  } else {
    SB_ERROR( "%s: Got bad status 0x%08x", __PRETTY_FUNCTION__,status);
    return FALSE;
  }
  /* now write into the Zframe */
  CaSetDataBytes(uData[0],&uData_bytes[0]);
  sbZfFe2000QmQueueState0Entry_Unpack(pZFrame,&uData_bytes[0],1);
  return TRUE;
}

uint8_t CaQmQueueState0Write( sbhandle userDeviceHandle,uint32_t uAddress, sbZfFe2000QmQueueState0Entry_t *pZFrame ) {
  uint8_t uData_bytes[4] = {0};
  uint32_t uData[1] = {0};
  sbFe2000ImfDriverStatus_t status;
  /* now read data from the Zframe into the uData buffer */
  sbZfFe2000QmQueueState0Entry_Pack(pZFrame,&uData_bytes[0],1);
  CaSetDataWord(&uData[0],&uData_bytes[0]);
  status = sbFe2000ImfDriver_QmQueueState0Write((uint32_t)userDeviceHandle, uAddress, uData[0]);
  if ( status == SAND_DRV_CA_STATUS_OK ) {
    /* good */
  } else {
    SB_ERROR("%s:Got bad status 0x%08x", __PRETTY_FUNCTION__,status);
    return FALSE;
  }
  return TRUE;
}

uint8_t CaQmQueueState1Read( sbhandle userDeviceHandle,uint32_t uAddress, sbZfFe2000QmQueueState1Entry_t *pZFrame ) {
  uint32_t uData[1] = {0};
  uint8_t uData_bytes[4]={0};
  sbFe2000ImfDriverStatus_t status;
  status = sbFe2000ImfDriver_QmQueueState1Read((uint32_t)userDeviceHandle, uAddress, &uData[0]);
  if ( status == SAND_DRV_CA_STATUS_OK ) {
    /* good */
  } else {
    SB_ERROR( "%s: Got bad status 0x%08x", __PRETTY_FUNCTION__,status);
    return FALSE;
  }
  /* now write into the Zframe */
  CaSetDataBytes(uData[0],&uData_bytes[0]);
  sbZfFe2000QmQueueState1Entry_Unpack(pZFrame,&uData_bytes[0],1);
  return TRUE;
}

uint8_t CaPtQueueToPortRead( sbhandle userDeviceHandle,uint32_t uAddress, sbZfFe2000PtQueueToPortEntry_t *pZFrame ) {
  uint32_t uData[1] = {0};
  uint8_t uData_bytes[4]={0};
  sbFe2000ImfDriverStatus_t status;
  status = sbFe2000ImfDriver_PtQueueToPortRead((uint32_t)userDeviceHandle, uAddress, &uData[0]);
  if ( status == SAND_DRV_CA_STATUS_OK ) {
    /* good */
  } else {
    SB_ERROR( "%s: Got bad status 0x%08x", __PRETTY_FUNCTION__,status);
    return FALSE;
  }
  /* now write into the Zframe */
  CaSetDataBytes(uData[0],&uData_bytes[0]);
  sbZfFe2000PtQueueToPortEntry_Unpack(pZFrame,&uData_bytes[0],1);
  return TRUE;
}

uint8_t CaPtQueueToPortWrite( sbhandle userDeviceHandle,uint32_t uAddress,  sbZfFe2000PtQueueToPortEntry_t *pZFrame ) {
  uint32_t uData[1] = {0};
  uint8_t uData_bytes[4] = {0};
  sbFe2000ImfDriverStatus_t status;
  /* now read data from the Zframe into the uData buffer */
  sbZfFe2000PtQueueToPortEntry_Pack(pZFrame,&uData_bytes[0],1);
  CaSetDataWord(&uData[0],&uData_bytes[0]);
  status = sbFe2000ImfDriver_PtQueueToPortWrite((uint32_t)userDeviceHandle, uAddress, uData[0]);
  if ( status == SAND_DRV_CA_STATUS_OK ) {
    /* good */
  } else {
    SB_ERROR("%s:Got bad status 0x%08x", __PRETTY_FUNCTION__,status);
    return FALSE;
  }
  return TRUE;
}

uint8_t CaQmQueueConfigRead( sbhandle userDeviceHandle,uint32_t uAddress, sbZfFe2000QmQueueConfigEntry_t *pZFrame) {
  uint32_t uData[3]={0};
  uint8_t uData_bytes[12]={0};
  sbFe2000ImfDriverStatus_t status;
  status = sbFe2000ImfDriver_QmQueueConfigRead((uint32_t)userDeviceHandle, uAddress, &uData[0], &uData[1], &uData[2]);
  if ( status == SAND_DRV_CA_STATUS_OK ) {
    /* good */
  } else {
    SB_ERROR( "%s:Got bad status 0x%08x", __PRETTY_FUNCTION__,status);
    return FALSE;
  }
  /* now write into the Zframe */
  CaSetDataBytes(uData[0],&uData_bytes[0]);
  CaSetDataBytes(uData[1],&uData_bytes[4]);
  CaSetDataBytes(uData[2],&uData_bytes[8]);
  sbZfFe2000QmQueueConfigEntry_Unpack(pZFrame,&uData_bytes[0],1);
  return TRUE;
}

uint8_t CaQmQueueConfigWrite( sbhandle userDeviceHandle,uint32_t uAddress, sbZfFe2000QmQueueConfigEntry_t *pZFrame) {
  uint32_t uData[3]={0};
  uint8_t uData_bytes[12]={0};
  sbFe2000ImfDriverStatus_t status;
  /* now read data from the Zframe into the uData buffer */
  sbZfFe2000QmQueueConfigEntry_Pack(pZFrame,&uData_bytes[0],1);
  CaSetDataWord(&uData[0],&uData_bytes[0]);
  CaSetDataWord(&uData[1],&uData_bytes[4]);
  CaSetDataWord(&uData[2],&uData_bytes[8]);
  status = sbFe2000ImfDriver_QmQueueConfigWrite((uint32_t)userDeviceHandle, uAddress, uData[0], uData[1], uData[2]);
  if ( status == SAND_DRV_CA_STATUS_OK ) {
    /* good */
  } else {
    SB_ERROR("%s:Got bad status 0x%08x", __PRETTY_FUNCTION__,status);
    return FALSE;
  }
  return TRUE;
}

uint8_t CaPtPortToQueueSt0Write( sbhandle userDeviceHandle, uint32_t uAddress, sbZfFe2000PtPortToQueueEntry_t *pZFrame) {
  uint32_t uData[1]={0};
  uint8_t uData_bytes[4]={0};
  sbFe2000ImfDriverStatus_t status;
  /* now read data from the Zframe into the uData buffer */
  sbZfFe2000PtPortToQueueEntry_Pack(pZFrame,&uData_bytes[0],1);
  CaSetDataWord(&uData[0],&uData_bytes[0]);
  status = sbFe2000ImfDriver_PtPortToQueueSt0Write((uint32_t)userDeviceHandle, uAddress, uData[0]);
  if ( status == SAND_DRV_CA_STATUS_OK ) {
    /* good */
  } else {
    SB_ERROR("%s:Got bad status 0x%08x", __PRETTY_FUNCTION__,status);
    return FALSE;
  }
  return TRUE;
}

uint8_t CaPtPortToQueueSt1Write( sbhandle userDeviceHandle, uint32_t uAddress, sbZfFe2000PtPortToQueueEntry_t *pZFrame) {
  uint32_t uData[1]={0};
  uint8_t uData_bytes[4]={0};
  sbFe2000ImfDriverStatus_t status;
  /* now read data from the Zframe into the uData buffer */
  sbZfFe2000PtPortToQueueEntry_Pack(pZFrame,&uData_bytes[0],1);
  CaSetDataWord(&uData[0],&uData_bytes[0]);
  status = sbFe2000ImfDriver_PtPortToQueueSt1Write((uint32_t)userDeviceHandle, uAddress, uData[0]);
  if ( status == SAND_DRV_CA_STATUS_OK ) {
    /* good */
  } else {
    SB_ERROR("%s:Got bad status 0x%08x", __PRETTY_FUNCTION__,status);
    return FALSE;
  }
  return TRUE;
}

uint8_t CaPrPortToQueueSr0Write( sbhandle userDeviceHandle, uint32_t uAddress, sbZfFe2000PrPortToQueueSr0Entry_t *pZFrame) {
  uint32_t uData[1]={0};
  uint8_t uData_bytes[4]={0};
  /* now read data from the Zframe into the uData buffer */
  sbFe2000ImfDriverStatus_t status;
  sbZfFe2000PrPortToQueueSr0Entry_Pack(pZFrame,&uData_bytes[0],1);
  CaSetDataWord(&uData[0],&uData_bytes[0]);
  status = sbFe2000ImfDriver_PrPortToQueueSr0Write((uint32_t)userDeviceHandle, uAddress, uData[0]);
  if ( status == SAND_DRV_CA_STATUS_OK ) {
    /* good */
  } else {
    SB_ERROR("%s:Got bad status 0x%08x", __PRETTY_FUNCTION__,status);
    return FALSE;
  }
  return TRUE;
}

uint8_t CaPrPortToQueueSr1Write( sbhandle userDeviceHandle,uint32_t uAddress,  sbZfFe2000PrPortToQueueSr1Entry_t *pZFrame ) {
  uint32_t uData[1]={0};
  uint8_t uData_bytes[4]={0};
  /* now read data from the Zframe into the uData buffer */
  sbFe2000ImfDriverStatus_t status;
  sbZfFe2000PrPortToQueueSr1Entry_Pack(pZFrame,&uData_bytes[0],1);
  CaSetDataWord(&uData[0],&uData_bytes[0]);
  status = sbFe2000ImfDriver_PrPortToQueueSr1Write((uint32_t)userDeviceHandle, uAddress, uData[0]);
  if ( status == SAND_DRV_CA_STATUS_OK ) {
    /* good */
  } else {
    SB_ERROR("%s:Got bad status 0x%08x", __PRETTY_FUNCTION__,status);
    return FALSE;
  }
  return TRUE;
}

uint8_t CaPrPortToQueueAg0Write( sbhandle userDeviceHandle, uint32_t uAddress, sbZfFe2000PrPortToQueueAg0Entry_t *pZFrame) {
  uint32_t uData[1] = {0};
  uint8_t uData_bytes[4]={0};
  /* now read data from the Zframe into the uData buffer */
  sbFe2000ImfDriverStatus_t status;
  sbZfFe2000PrPortToQueueAg0Entry_Pack(pZFrame,&uData_bytes[0],1);
  CaSetDataWord(&uData[0],&uData_bytes[0]);
  status = sbFe2000ImfDriver_PrPortToQueueAg0Write((uint32_t)userDeviceHandle, uAddress, uData[0]);
  if ( status == SAND_DRV_CA_STATUS_OK ) {
    /* good */
  } else {
    SB_ERROR("%s:Got bad status 0x%08x", __PRETTY_FUNCTION__,status);
    return FALSE;
  }
  return TRUE;
}

uint8_t CaPrPortToQueueAg1Write( sbhandle userDeviceHandle, uint32_t uAddress, sbZfFe2000PrPortToQueueAg1Entry_t *pZFrame ) {
  uint32_t uData[1]={0};
  uint8_t uData_bytes[4]={0};
  sbFe2000ImfDriverStatus_t status;
  /* now read data from the Zframe into the uData buffer */
  sbZfFe2000PrPortToQueueAg1Entry_Pack(pZFrame,&uData_bytes[0],1);
  CaSetDataWord(&uData[0],&uData_bytes[0]);
  status = sbFe2000ImfDriver_PrPortToQueueAg1Write((uint32_t)userDeviceHandle, uAddress, uData[0]);
  if ( status == SAND_DRV_CA_STATUS_OK ) {
    /* good */
  } else {
    SB_ERROR("%s:Got bad status 0x%08x", __PRETTY_FUNCTION__,status);
    return FALSE;
  }
  return TRUE;
}

uint8_t CaPtPortToQueueAg0Write( sbhandle userDeviceHandle,uint32_t uAddress, sbZfFe2000PtPortToQueueEntry_t *pZFrame) {
  uint32_t uData[1]={0};
  uint8_t uData_bytes[4]={0};
  sbFe2000ImfDriverStatus_t status;
  /* now read data from the Zframe into the uData buffer */
  sbZfFe2000PtPortToQueueEntry_Pack(pZFrame,&uData_bytes[0],1);
  CaSetDataWord(&uData[0],&uData_bytes[0]);
  status = sbFe2000ImfDriver_PtPortToQueueAg0Write((uint32_t)userDeviceHandle, uAddress, uData[0]);
  if ( status == SAND_DRV_CA_STATUS_OK ) {
    /* good */
  } else {
    SB_ERROR("%s:Got bad status 0x%08x", __PRETTY_FUNCTION__,status);
    return FALSE;
  }
  return TRUE;
}

uint8_t CaPtPortToQueueAg1Write( sbhandle userDeviceHandle, uint32_t uAddress, sbZfFe2000PtPortToQueueEntry_t *pZFrame) {
  uint32_t uData[1]={0};
  uint8_t uData_bytes[4]={0};
  sbFe2000ImfDriverStatus_t status;
  /* now read data from the Zframe into the uData buffer */
  sbZfFe2000PtPortToQueueEntry_Pack(pZFrame,&uData_bytes[0],1);
  CaSetDataWord(&uData[0],&uData_bytes[0]);
  status = sbFe2000ImfDriver_PtPortToQueueAg1Write((uint32_t)userDeviceHandle, uAddress, uData[0]);
  if ( status == SAND_DRV_CA_STATUS_OK ) {
    /* good */
  } else {
    SB_ERROR("%s:Got bad status 0x%08x", __PRETTY_FUNCTION__,status);
    return FALSE;
  }
  return TRUE;
}

uint8_t CaPtPortToQueueXg0Write( sbhandle userDeviceHandle, uint32_t uAddress, sbZfFe2000PtPortToQueueEntry_t *pZFrame ) {
  uint32_t uData[1]={0};
  uint8_t uData_bytes[4]={0};
  sbFe2000ImfDriverStatus_t status;
  /* now read data from the Zframe into the uData buffer */
  sbZfFe2000PtPortToQueueEntry_Pack(pZFrame,&uData_bytes[0],1);
  CaSetDataWord(&uData[0],&uData_bytes[0]);
  status = sbFe2000ImfDriver_PtPortToQueueXg0Write((uint32_t)userDeviceHandle, uAddress, uData[0]);
  if ( status == SAND_DRV_CA_STATUS_OK ) {
    /* good */
  } else {
    SB_ERROR("%s:Got bad status 0x%08x", __PRETTY_FUNCTION__,status);
    return FALSE;
  }
  return TRUE;
}

uint8_t CaPtPortToQueueXg1Write( sbhandle userDeviceHandle, uint32_t uAddress, sbZfFe2000PtPortToQueueEntry_t *pZFrame ) {
  uint32_t uData[1]={0};
  uint8_t uData_bytes[4]={0};
  sbFe2000ImfDriverStatus_t status;
  /* now read data from the Zframe into the uData buffer */
  sbZfFe2000PtPortToQueueEntry_Pack(pZFrame,&uData_bytes[0],1);
  CaSetDataWord(&uData[0],&uData_bytes[0]);
  status = sbFe2000ImfDriver_PtPortToQueueXg1Write((uint32_t)userDeviceHandle, uAddress, uData[0]);
  if ( status == SAND_DRV_CA_STATUS_OK ) {
    /* good */
  } else {
    SB_ERROR("%s:Got bad status 0x%08x", __PRETTY_FUNCTION__,status);
    return FALSE;
  }
  return TRUE;
}

uint8_t CaPrPortToQueueXg0Write( sbhandle userDeviceHandle, uint32_t uAddress, sbZfFe2000PrPortToQueueXg0Entry_t *pZFrame) {
  uint32_t uData[1]={0};
  uint8_t uData_bytes[4]={0};
  sbFe2000ImfDriverStatus_t status;
  /* now read data from the Zframe into the uData buffer */
  sbZfFe2000PrPortToQueueXg0Entry_Pack(pZFrame,&uData_bytes[0],1);
  CaSetDataWord(&uData[0],&uData_bytes[0]);
  status = sbFe2000ImfDriver_PrPortToQueueXg0Write((uint32_t)userDeviceHandle, uAddress, uData[0]);
  if ( status == SAND_DRV_CA_STATUS_OK ) {
    /* good */
  } else {
    SB_ERROR("%s:Got bad status 0x%08x", __PRETTY_FUNCTION__,status);
    return FALSE;
  }
  return TRUE;
}

uint8_t CaPrPortToQueueXg1Write( sbhandle userDeviceHandle, uint32_t uAddress, sbZfFe2000PrPortToQueueXg1Entry_t *pZFrame) {
  uint32_t uData[1]={0};
  uint8_t uData_bytes[4]={0};
  sbFe2000ImfDriverStatus_t status;
  /* now read data from the Zframe into the uData buffer */
  sbZfFe2000PrPortToQueueXg1Entry_Pack(pZFrame,&uData_bytes[0],1);
  CaSetDataWord(&uData[0],&uData_bytes[0]);
  status = sbFe2000ImfDriver_PrPortToQueueXg1Write((uint32_t)userDeviceHandle, uAddress, uData[0]);
  if ( status == SAND_DRV_CA_STATUS_OK ) {
  } else {
    SB_ERROR("%s:Got bad status 0x%08x", __PRETTY_FUNCTION__,status);
    return FALSE;
  }
  return TRUE;
}


uint8_t CaPtPortToQueuePciWrite( sbhandle userDeviceHandle, uint32_t uAddress, sbZfFe2000PtPortToQueueEntry_t *pZFrame ) {
  uint32_t uData[1]={0};
  uint8_t uData_bytes[4]={0};
  sbFe2000ImfDriverStatus_t status;
  /* now read data from the Zframe into the uData buffer */
  sbZfFe2000PtPortToQueueEntry_Pack(pZFrame,&uData_bytes[0],1);
  CaSetDataWord(&uData[0],&uData_bytes[0]);
  status = sbFe2000ImfDriver_PtPortToQueuePciWrite((uint32_t)userDeviceHandle, uAddress, uData[0]);
  if ( status == SAND_DRV_CA_STATUS_OK ) {
    /* good */
  } else {
    SB_ERROR("%s:Got bad status 0x%08x", __PRETTY_FUNCTION__,status);
    return FALSE;
  }
  return TRUE;
}

uint8_t CaPrPortToQueuePciWrite( sbhandle userDeviceHandle, uint32_t uAddress, sbZfFe2000PrPortToQueuePciEntry_t *pZFrame) {
  uint32_t uData[1]={0};
  uint8_t uData_bytes[4]={0};
  sbFe2000ImfDriverStatus_t status;
  /* now read data from the Zframe into the uData buffer */
  sbZfFe2000PrPortToQueuePciEntry_Pack(pZFrame,&uData_bytes[0],1);
  CaSetDataWord(&uData[0],&uData_bytes[0]);
  status = sbFe2000ImfDriver_PrPortToQueuePciWrite((uint32_t)userDeviceHandle,uAddress, uData[0]);
  if ( status == SAND_DRV_CA_STATUS_OK ) {
    /* good */
  } else {
    SB_ERROR("%s:Got bad status 0x%08x", __PRETTY_FUNCTION__,status);
    return FALSE;
  }
  return TRUE;
}

uint8_t CaQmQueueState0EnWrite( sbhandle userDeviceHandle, uint32_t uAddress, sbZfFe2000QmQueueState0EnEntry_t *pZFrame ) {
  uint32_t uData[1]={0};
  uint8_t uData_bytes[4]={0};
  sbFe2000ImfDriverStatus_t status;
  sbZfFe2000QmQueueState0EnEntry_Pack(pZFrame,&uData_bytes[0],1);
  CaSetDataWord(&uData[0],&uData_bytes[0]);
  status = sbFe2000ImfDriver_QmQueueState0EnWrite((uint32_t)userDeviceHandle, uAddress, uData[0]);
  if ( status == SAND_DRV_CA_STATUS_OK ) {
    /* good */
  } else {
    SB_ERROR("%s:Got bad status 0x%08x", __PRETTY_FUNCTION__,status);
    return FALSE;
  }
  return TRUE;
}

uint8_t CaPpInitialQueueStateRead( sbhandle userDeviceHandle, uint32_t uAddress, sbZfFe2000PpInitialQueueStateEntry_t *pZFrame) {
  uint32_t uData[3]={0};
  uint8_t uData_bytes[12]={0};
  sbFe2000ImfDriverStatus_t status;
  status = sbFe2000ImfDriver_PpInitialQueueStateRead((uint32_t)userDeviceHandle, uAddress, &uData[0], &uData[1], &uData[2]);
  if ( status == SAND_DRV_CA_STATUS_OK ) {
    /* good */
  } else {
    SB_ERROR( "%s:Got bad status 0x%08x", __PRETTY_FUNCTION__,status);
    return FALSE;
  }
  /* now write into the Zframe */
  CaSetDataBytes(uData[0],&uData_bytes[0]);
  CaSetDataBytes(uData[1],&uData_bytes[4]);
  CaSetDataBytes(uData[2],&uData_bytes[8]);
  sbZfFe2000PpInitialQueueStateEntry_Unpack(pZFrame,&uData_bytes[0],1);
  return TRUE;
}

uint8_t CaPpInitialQueueStateWrite( sbhandle userDeviceHandle, uint32_t uAddress, sbZfFe2000PpInitialQueueStateEntry_t *pZFrame) {
  uint32_t uData[3]={0};
  uint8_t uData_bytes[12]={0};
  sbFe2000ImfDriverStatus_t status;
  /* now read data from the Zframe into the uData buffer */
  sbZfFe2000PpInitialQueueStateEntry_Pack(pZFrame,&uData_bytes[0],1);
  CaSetDataWord(&uData[0],&uData_bytes[0]);
  CaSetDataWord(&uData[1],&uData_bytes[4]);
  CaSetDataWord(&uData[2],&uData_bytes[8]);
  status = sbFe2000ImfDriver_PpInitialQueueStateWrite((uint32_t)userDeviceHandle, uAddress, uData[0], uData[1], uData[2]);
  if ( status == SAND_DRV_CA_STATUS_OK ) {
      /* good */
  } else {
    SB_ERROR("%s:Got bad status 0x%08x", __PRETTY_FUNCTION__,status);
    return FALSE;
  }
  return TRUE;
}

uint8_t CaPpHeaderRecordSizeWrite( sbhandle userDeviceHandle, uint32_t uAddress, sbZfFe2000PpHeaderRecordSizeEntry_t *pZFrame ) {
  uint32_t uData[1]={0};
  uint8_t uData_bytes[4]={0};
  sbFe2000ImfDriverStatus_t status;
  /* now read data from the Zframe into the uData buffer */
  sbZfFe2000PpHeaderRecordSizeEntry_Pack(pZFrame,&uData_bytes[0],1);
  CaSetDataWord(&uData[0],&uData_bytes[0]);
  status = sbFe2000ImfDriver_PpHeaderRecordSizeWrite((uint32_t)userDeviceHandle, uAddress, uData[0]);
  if ( status == SAND_DRV_CA_STATUS_OK ) {
      /* good */
  } else {
      SB_ERROR("%s:Got bad status 0x%08x", __PRETTY_FUNCTION__,status);
      return FALSE;
  }
  return TRUE;
}

uint8_t CaPpQueuePriorityGroupWrite( sbhandle userDeviceHandle, uint32_t uAddress, sbZfFe2000PpQueuePriorityGroupEntry_t *pZFrame ) {
  uint32_t uData[1]={0};
  uint8_t uData_bytes[4]={0};
  sbFe2000ImfDriverStatus_t status;
  /* now read data from the Zframe into the uData buffer */
  sbZfFe2000PpQueuePriorityGroupEntry_Pack(pZFrame,&uData_bytes[0],1);
  CaSetDataWord(&uData[0],&uData_bytes[0]);
  status = sbFe2000ImfDriver_PpQueuePriorityGroupWrite((uint32_t)userDeviceHandle, uAddress, uData[0]);
  if ( status == SAND_DRV_CA_STATUS_OK ) {
      /* good */
  } else {
      SB_ERROR("%s:Got bad status 0x%08x", __PRETTY_FUNCTION__,status);
      return FALSE;
  }
  return TRUE;
}

uint8_t CaPdHeaderConfigWrite(sbhandle userDeviceHandle, uint32_t uIndex, sbZfFe2000PdHeaderConfig_t *pZFrame) {

  uint32_t uData[1]={0};
  uint8_t uData_bytes[4]={0};
/*   rZFrame.CalcBufferUint(&uData,1,CZFrame::ZF_LSB_IN_WORD0_LSB); */
  sbZfFe2000PdHeaderConfig_Pack(pZFrame,&uData_bytes[0],1);
  CaSetDataWord(&uData[0],&uData_bytes[0]);
  SAND_HAL_WRITE_OFFS(userDeviceHandle,CA_PD_HEADER_CONFIG(uIndex),uData[0]);
  return TRUE;
}


uint8_t CaQmQueueConfigClear(sbhandle userDeviceHandle) {
  sbZfFe2000QmQueueConfigEntry_t zClearFrame;
  sbZfFe2000QmQueueConfigEntry_InitInstance(&zClearFrame);
  return CaQmQueueConfigFillPattern(userDeviceHandle,&zClearFrame);
}

uint8_t CaQmQueueConfigFillPattern(sbhandle userDeviceHandle, sbZfFe2000QmQueueConfigEntry_t *pZFrame) {
  uint32_t uData[3] = {0};
  uint8_t uData_bytes[12] = {0};
  sbFe2000ImfDriverStatus_t status;

  sbZfFe2000QmQueueConfigEntry_Pack(pZFrame,&uData_bytes[0],1);
  CaSetDataWord(&uData[0],&uData_bytes[0]);
  CaSetDataWord(&uData[1],&uData_bytes[4]);
  CaSetDataWord(&uData[2],&uData_bytes[8]);
  status = sbFe2000ImfDriver_QmQueueConfigFillPattern((uint32_t)userDeviceHandle,uData[0],uData[1],uData[2]);
  if ( status == SAND_DRV_CA_STATUS_OK ) {
      /* good */
  } else {
      SB_ERROR("%s:Got bad status 0x%08x", __PRETTY_FUNCTION__,status);
      return FALSE;
  }
  return TRUE;
}

uint8_t CaPtPortToQueueAg0Clear(sbhandle userDeviceHandle) {
  sbZfFe2000PtPortToQueueEntry_t zClearFrame;
  sbZfFe2000PtPortToQueueEntry_InitInstance(&zClearFrame);
  return CaPtPortToQueueAg0FillPattern(userDeviceHandle,&zClearFrame);
}

uint8_t CaPtPortToQueueAg0FillPattern(sbhandle userDeviceHandle, sbZfFe2000PtPortToQueueEntry_t *pZFrame) {
  uint32_t uData[1] = {0};
  uint8_t uData_bytes[4] = {0};
  sbFe2000ImfDriverStatus_t status;

  sbZfFe2000PtPortToQueueEntry_Pack(pZFrame,&uData_bytes[0],1);
  CaSetDataWord(&uData[0],&uData_bytes[0]);
  status = sbFe2000ImfDriver_PtPortToQueueAg0FillPattern((uint32_t)userDeviceHandle,uData[0]);
  if ( status == SAND_DRV_CA_STATUS_OK ) {
      /* good */
  } else {
      SB_ERROR("%s:Got bad status 0x%08x", __PRETTY_FUNCTION__,status);
      return FALSE;
  }
  return TRUE;
}

uint8_t CaPtPortToQueueAg1Clear(sbhandle userDeviceHandle) {
  sbZfFe2000PtPortToQueueEntry_t zClearFrame;
  sbZfFe2000PtPortToQueueEntry_InitInstance(&zClearFrame);
  return CaPtPortToQueueAg1FillPattern(userDeviceHandle,&zClearFrame);
}

uint8_t CaPtPortToQueueAg1FillPattern(sbhandle userDeviceHandle, sbZfFe2000PtPortToQueueEntry_t *pZFrame) {
  uint32_t uData[1] = {0};
  uint8_t uData_bytes[4] = {0};
  sbFe2000ImfDriverStatus_t status;

  sbZfFe2000PtPortToQueueEntry_Pack(pZFrame,&uData_bytes[0],1);
  CaSetDataWord(&uData[0],&uData_bytes[0]);
  status = sbFe2000ImfDriver_PtPortToQueueAg1FillPattern((uint32_t)userDeviceHandle,uData[0]);
  if ( status == SAND_DRV_CA_STATUS_OK ) {
      /* good */
  } else {
      SB_ERROR("%s:Got bad status 0x%08x", __PRETTY_FUNCTION__,status);
      return FALSE;
  }
  return TRUE;
}

uint8_t CaQmQueueState0Clear(sbhandle userDeviceHandle) {
  sbZfFe2000QmQueueState0Entry_t zClearFrame;
  sbZfFe2000QmQueueState0Entry_InitInstance(&zClearFrame);
  return CaQmQueueState0FillPattern(userDeviceHandle,&zClearFrame);
}

uint8_t CaQmQueueState0FillPattern(sbhandle userDeviceHandle, sbZfFe2000QmQueueState0Entry_t *pZFrame) {
  uint32_t uData[1] = {0};
  uint8_t uData_bytes[4] = {0};
  sbFe2000ImfDriverStatus_t status;

  sbZfFe2000QmQueueState0Entry_Pack(pZFrame,&uData_bytes[0],1);
  CaSetDataWord(&uData[0],&uData_bytes[0]);
  status = sbFe2000ImfDriver_QmQueueState0FillPattern((uint32_t)userDeviceHandle,uData[0]);
  if ( status == SAND_DRV_CA_STATUS_OK ) {
      /* good */
  } else {
      SB_ERROR("%s:Got bad status 0x%08x", __PRETTY_FUNCTION__,status);
      return FALSE;
  }
  return TRUE;

}

uint8_t CaQmQueueState1Clear(sbhandle userDeviceHandle) {
  sbZfFe2000QmQueueState1Entry_t zClearFrame;
  sbZfFe2000QmQueueState1Entry_InitInstance(&zClearFrame);
  return CaQmQueueState1FillPattern(userDeviceHandle,&zClearFrame);
}

uint8_t CaQmQueueState1FillPattern(sbhandle userDeviceHandle, sbZfFe2000QmQueueState1Entry_t *pZFrame) {
  uint32_t uData[1] = {0};
  uint8_t uData_bytes[4] = {0};
  sbFe2000ImfDriverStatus_t status;

  sbZfFe2000QmQueueState1Entry_Pack(pZFrame,&uData_bytes[0],1);
  CaSetDataWord(&uData[0],&uData_bytes[0]);
  status = sbFe2000ImfDriver_QmQueueState1FillPattern((uint32_t)userDeviceHandle,uData[0]);
  if ( status == SAND_DRV_CA_STATUS_OK ) {
      /* good */
  } else {
      SB_ERROR("%s:Got bad status 0x%08x", __PRETTY_FUNCTION__,status);
      return FALSE;
  }
  return TRUE;

}

uint8_t CaMm0Internal1MemoryWrite(sbhandle userDeviceHandle, uint32_t uAddress, sbZfFe2000MmInternal1MemoryEntry_t *pZFrame) {
  uint32_t uData[2] = {0};
  uint8_t uData_bytes[8] = {0};
  sbFe2000ImfDriverStatus_t status;
  sbZfFe2000MmInternal1MemoryEntry_Pack(pZFrame,&uData_bytes[0],1);
  CaSetDataWord(&uData[0],&uData_bytes[0]);
  CaSetDataWord(&uData[1],&uData_bytes[4]);
  status = sbFe2000ImfDriver_Mm0Internal1MemoryWrite((uint32_t)userDeviceHandle,uAddress,uData[0],uData[1]);
  if ( status == SAND_DRV_CA_STATUS_OK ) {
      /* good */
  } else {
      SB_ERROR("%s:Got bad status 0x%08x", __PRETTY_FUNCTION__,status);
      return FALSE;
  }
  return TRUE;
}

uint8_t CaMm1Internal0MemoryWrite(sbhandle userDeviceHandle, uint32_t uAddress, sbZfFe2000MmInternal0MemoryEntry_t *pZFrame) {
  uint32_t uData[2] = {0};
  uint8_t uData_bytes[8] = {0};
  sbFe2000ImfDriverStatus_t status;
  sbZfFe2000MmInternal0MemoryEntry_Pack(pZFrame,&uData_bytes[0],1);
  CaSetDataWord(&uData[0],&uData_bytes[0]);
  CaSetDataWord(&uData[1],&uData_bytes[4]);
  status = sbFe2000ImfDriver_Mm1Internal0MemoryWrite((uint32_t)userDeviceHandle,uAddress,uData[0],uData[1]);
  if ( status == SAND_DRV_CA_STATUS_OK ) {
      /* good */
  } else {
      SB_ERROR("%s:Got bad status 0x%08x", __PRETTY_FUNCTION__,status);
      return FALSE;
  }
  return TRUE;
}

uint8_t CaMm1Internal1MemoryWrite(sbhandle userDeviceHandle, uint32_t uAddress, sbZfFe2000MmInternal1MemoryEntry_t *pZFrame) {
  uint32_t uData[2] = {0};
  uint8_t uData_bytes[8] = {0};
  sbFe2000ImfDriverStatus_t status;
  sbZfFe2000MmInternal1MemoryEntry_Pack(pZFrame,&uData_bytes[0],1);
  CaSetDataWord(&uData[0],&uData_bytes[0]);
  CaSetDataWord(&uData[1],&uData_bytes[4]);
  status = sbFe2000ImfDriver_Mm1Internal1MemoryWrite((uint32_t)userDeviceHandle,uAddress,uData[0],uData[1]);
  if ( status == SAND_DRV_CA_STATUS_OK ) {
      /* good */
  } else {
      SB_ERROR("%s:Got bad status 0x%08x", __PRETTY_FUNCTION__,status);
      return FALSE;
  }
  return TRUE;
}

uint8_t CaMm0Internal0MemoryRead(sbhandle userDeviceHandle, uint32_t uAddress, sbZfFe2000MmInternal0MemoryEntry_t *pZFrame) {
  uint32_t uData[2] = {0};
  uint8_t uData_bytes[8] = {0};
  sbFe2000ImfDriverStatus_t status;
  status = sbFe2000ImfDriver_Mm0Internal0MemoryRead((uint32_t)userDeviceHandle,uAddress,&uData[0],&uData[1]);
  if ( status == SAND_DRV_CA_STATUS_OK ) {
    /* good */
  } else {
    SB_ERROR("%s:Got bad status 0x%08x", __PRETTY_FUNCTION__,status);
    return FALSE;
  }
  /* now write into the Zframe */
  CaSetDataBytes(uData[0],&uData_bytes[0]);
  CaSetDataBytes(uData[1],&uData_bytes[4]);
  sbZfFe2000MmInternal0MemoryEntry_Unpack(pZFrame,&uData_bytes[0],1);
  return TRUE;
}

uint8_t CaMm0Internal1MemoryRead(sbhandle userDeviceHandle, uint32_t uAddress, sbZfFe2000MmInternal1MemoryEntry_t *pZFrame) {
  uint32_t uData[2] = {0};
  uint8_t uData_bytes[8] = {0};
  sbFe2000ImfDriverStatus_t status;
  status = sbFe2000ImfDriver_Mm0Internal1MemoryRead((uint32_t)userDeviceHandle,uAddress,&uData[0],&uData[1]);
  if ( status == SAND_DRV_CA_STATUS_OK ) {
    /* good */
  } else {
    SB_ERROR("%s:Got bad status 0x%08x", __PRETTY_FUNCTION__,status);
    return FALSE;
  }
  /* now write into the Zframe */
  CaSetDataBytes(uData[0],&uData_bytes[0]);
  CaSetDataBytes(uData[1],&uData_bytes[4]);
  sbZfFe2000MmInternal1MemoryEntry_Unpack(pZFrame,&uData_bytes[0],1);
  return TRUE;
}

uint8_t CaMm1Internal0MemoryRead(sbhandle userDeviceHandle, uint32_t uAddress, sbZfFe2000MmInternal0MemoryEntry_t *pZFrame) {
  uint32_t uData[2] = {0};
  uint8_t uData_bytes[8] = {0};
  sbFe2000ImfDriverStatus_t status;
  status = sbFe2000ImfDriver_Mm1Internal0MemoryRead((uint32_t)userDeviceHandle,uAddress,&uData[0],&uData[1]);
  if ( status == SAND_DRV_CA_STATUS_OK ) {
    /* good */
  } else {
    SB_ERROR("%s:Got bad status 0x%08x", __PRETTY_FUNCTION__,status);
    return FALSE;
  }
  /* now write into the Zframe */
  CaSetDataBytes(uData[0],&uData_bytes[0]);
  CaSetDataBytes(uData[1],&uData_bytes[4]);
  sbZfFe2000MmInternal0MemoryEntry_Unpack(pZFrame,&uData_bytes[0],1);
  return TRUE;
}

uint8_t CaMm1Internal1MemoryRead(sbhandle userDeviceHandle, uint32_t uAddress, sbZfFe2000MmInternal1MemoryEntry_t *pZFrame) {
  uint32_t uData[2] = {0};
  uint8_t uData_bytes[8] = {0};
  sbFe2000ImfDriverStatus_t status;
  status = sbFe2000ImfDriver_Mm1Internal1MemoryRead((uint32_t)userDeviceHandle,uAddress,&uData[0],&uData[1]);
  if ( status == SAND_DRV_CA_STATUS_OK ) {
    /* good */
  } else {
    SB_ERROR("%s:Got bad status 0x%08x", __PRETTY_FUNCTION__,status);
    return FALSE;
  }
  /* now write into the Zframe */
  CaSetDataBytes(uData[0],&uData_bytes[0]);
  CaSetDataBytes(uData[1],&uData_bytes[4]);
  sbZfFe2000MmInternal1MemoryEntry_Unpack(pZFrame,&uData_bytes[0],1);
  return TRUE;
}


uint8_t CaMm0Internal0MemoryWrite(sbhandle userDeviceHandle, uint32_t uAddress, sbZfFe2000MmInternal0MemoryEntry_t *pZFrame) {
  uint32_t uData[2] = {0};
  uint8_t uData_bytes[8] = {0};
  sbFe2000ImfDriverStatus_t status;
  sbZfFe2000MmInternal0MemoryEntry_Pack(pZFrame,&uData_bytes[0],1);
  CaSetDataWord(&uData[0],&uData_bytes[0]);
  CaSetDataWord(&uData[1],&uData_bytes[4]);
  status = sbFe2000ImfDriver_Mm0Internal0MemoryWrite((uint32_t)userDeviceHandle,uAddress,uData[0],uData[1]);
  if ( status == SAND_DRV_CA_STATUS_OK ) {
      /* good */
  } else {
      SB_ERROR("%s:Got bad status 0x%08x", __PRETTY_FUNCTION__,status);
      return FALSE;
  }
  return TRUE;
}
