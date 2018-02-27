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
 * $Id: sbFe2000ImfDriver.h 1.4.214.1 Broadcom SDK $
 *
 ******************************************************************************/

#ifndef SB_FE2000IMFDriver_H
#define SB_FE2000IMFDriver_H

#include <soc/sbx/fe2k/sbFe2000.h>


#define CAD_INT INT 
#define CAD_UINT UINT 
#define CAD_ULLONG ULLONG 
#define CAD_UINT32 UINT32 
typedef enum                                                 
{                                                            
SAND_DRV_CA_STATUS_OK=0,                                     
SAND_DRV_CA_STATUS_ERROR=1,                                  
} sbFe2000ImfDriverStatus_t;                                 



#ifdef CAD_UINT
#undef CAD_UINT
#define CAD_UINT unsigned int
#endif



sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PrPortToQueueAg0Read( CAD_UINT uBaseAddress, 
                                        CAD_UINT uAddress,   
                                        CAD_UINT *pData);    
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PrPortToQueueAg0Write( CAD_UINT uBaseAddress, 
                                         CAD_UINT uAddress,  
                                         CAD_UINT uData);    
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PrPortToQueueAg0Clear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PrPortToQueueAg0FillPattern( CAD_UINT uBaseAddress, 
                                               CAD_UINT uData); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PrPortToQueueAg1Read( CAD_UINT uBaseAddress, 
                                        CAD_UINT uAddress,   
                                        CAD_UINT *pData);    
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PrPortToQueueAg1Write( CAD_UINT uBaseAddress, 
                                         CAD_UINT uAddress,  
                                         CAD_UINT uData);    
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PrPortToQueueAg1Clear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PrPortToQueueAg1FillPattern( CAD_UINT uBaseAddress, 
                                               CAD_UINT uData); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PrPortToQueuePciRead( CAD_UINT uBaseAddress, 
                                        CAD_UINT uAddress,   
                                        CAD_UINT *pData);    
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PrPortToQueuePciWrite( CAD_UINT uBaseAddress, 
                                         CAD_UINT uAddress,  
                                         CAD_UINT uData);    
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PrPortToQueuePciClear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PrPortToQueuePciFillPattern( CAD_UINT uBaseAddress, 
                                               CAD_UINT uData); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PrPortToQueueSr0Read( CAD_UINT uBaseAddress, 
                                        CAD_UINT uAddress,   
                                        CAD_UINT *pData);    
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PrPortToQueueSr0Write( CAD_UINT uBaseAddress, 
                                         CAD_UINT uAddress,  
                                         CAD_UINT uData);    
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PrPortToQueueSr0Clear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PrPortToQueueSr0FillPattern( CAD_UINT uBaseAddress, 
                                               CAD_UINT uData); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PrPortToQueueSr1Read( CAD_UINT uBaseAddress, 
                                        CAD_UINT uAddress,   
                                        CAD_UINT *pData);    
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PrPortToQueueSr1Write( CAD_UINT uBaseAddress, 
                                         CAD_UINT uAddress,  
                                         CAD_UINT uData);    
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PrPortToQueueSr1Clear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PrPortToQueueSr1FillPattern( CAD_UINT uBaseAddress, 
                                               CAD_UINT uData); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PrPortToQueueXg0Read( CAD_UINT uBaseAddress, 
                                        CAD_UINT uAddress,   
                                        CAD_UINT *pData);    
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PrPortToQueueXg0Write( CAD_UINT uBaseAddress, 
                                         CAD_UINT uAddress,  
                                         CAD_UINT uData);    
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PrPortToQueueXg0Clear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PrPortToQueueXg0FillPattern( CAD_UINT uBaseAddress, 
                                               CAD_UINT uData); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PrPortToQueueXg1Read( CAD_UINT uBaseAddress, 
                                        CAD_UINT uAddress,   
                                        CAD_UINT *pData);    
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PrPortToQueueXg1Write( CAD_UINT uBaseAddress, 
                                         CAD_UINT uAddress,  
                                         CAD_UINT uData);    
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PrPortToQueueXg1Clear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PrPortToQueueXg1FillPattern( CAD_UINT uBaseAddress, 
                                               CAD_UINT uData); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PdLrpOutputHeaderCopyBufferRead( CAD_UINT uBaseAddress, 
                                                   CAD_UINT uAddress, 
                                                   CAD_UINT *pData0, 
                                                   CAD_UINT *pData1, 
                                                   CAD_UINT *pData2, 
                                                   CAD_UINT *pData3, 
                                                   CAD_UINT *pData4, 
                                                   CAD_UINT *pData5, 
                                                   CAD_UINT *pData6, 
                                                   CAD_UINT *pData7); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PdLrpOutputHeaderCopyBufferWrite( CAD_UINT uBaseAddress, 
                                                    CAD_UINT uAddress, 
                                                    CAD_UINT uData0, 
                                                    CAD_UINT uData1, 
                                                    CAD_UINT uData2, 
                                                    CAD_UINT uData3, 
                                                    CAD_UINT uData4, 
                                                    CAD_UINT uData5, 
                                                    CAD_UINT uData6, 
                                                    CAD_UINT uData7); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PdLrpOutputHeaderCopyBufferClear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PdLrpOutputHeaderCopyBufferFillPattern( CAD_UINT uBaseAddress, 
                                                          CAD_UINT uData0, 
                                                          CAD_UINT uData1, 
                                                          CAD_UINT uData2, 
                                                          CAD_UINT uData3, 
                                                          CAD_UINT uData4, 
                                                          CAD_UINT uData5, 
                                                          CAD_UINT uData6, 
                                                          CAD_UINT uData7); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PdPdOutputHeaderCopyBufferRead( CAD_UINT uBaseAddress, 
                                                  CAD_UINT uAddress, 
                                                  CAD_UINT *pData0, 
                                                  CAD_UINT *pData1, 
                                                  CAD_UINT *pData2, 
                                                  CAD_UINT *pData3, 
                                                  CAD_UINT *pData4, 
                                                  CAD_UINT *pData5, 
                                                  CAD_UINT *pData6, 
                                                  CAD_UINT *pData7); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PdPdOutputHeaderCopyBufferWrite( CAD_UINT uBaseAddress, 
                                                   CAD_UINT uAddress, 
                                                   CAD_UINT uData0, 
                                                   CAD_UINT uData1, 
                                                   CAD_UINT uData2, 
                                                   CAD_UINT uData3, 
                                                   CAD_UINT uData4, 
                                                   CAD_UINT uData5, 
                                                   CAD_UINT uData6, 
                                                   CAD_UINT uData7); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PdPdOutputHeaderCopyBufferClear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PdPdOutputHeaderCopyBufferFillPattern( CAD_UINT uBaseAddress, 
                                                         CAD_UINT uData0, 
                                                         CAD_UINT uData1, 
                                                         CAD_UINT uData2, 
                                                         CAD_UINT uData3, 
                                                         CAD_UINT uData4, 
                                                         CAD_UINT uData5, 
                                                         CAD_UINT uData6, 
                                                         CAD_UINT uData7); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_LrLrpInstructionMemoryRead( CAD_UINT uBaseAddress, 
                                              CAD_UINT uAddress, 
                                              CAD_UINT *pData0, 
                                              CAD_UINT *pData1, 
                                              CAD_UINT *pData2); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_LrLrpInstructionMemoryWrite( CAD_UINT uBaseAddress, 
                                               CAD_UINT uAddress, 
                                               CAD_UINT uData0, 
                                               CAD_UINT uData1, 
                                               CAD_UINT uData2); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_LrLrpInstructionMemoryClear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_LrLrpInstructionMemoryFillPattern( CAD_UINT uBaseAddress, 
                                                     CAD_UINT uData0, 
                                                     CAD_UINT uData1, 
                                                     CAD_UINT uData2); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmBufferState0Read( CAD_UINT uBaseAddress, 
                                      CAD_UINT uAddress,     
                                      CAD_UINT *pData);      
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmBufferState0Write( CAD_UINT uBaseAddress, 
                                       CAD_UINT uAddress,    
                                       CAD_UINT uData);      
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmBufferState0Clear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmBufferState0FillPattern( CAD_UINT uBaseAddress, 
                                             CAD_UINT uData); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmBufferState1Read( CAD_UINT uBaseAddress, 
                                      CAD_UINT uAddress,     
                                      CAD_UINT *pData0,      
                                      CAD_UINT *pData1);     
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmBufferState1Write( CAD_UINT uBaseAddress, 
                                       CAD_UINT uAddress,    
                                       CAD_UINT uData0,      
                                       CAD_UINT uData1);     
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmBufferState1Clear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmBufferState1FillPattern( CAD_UINT uBaseAddress, 
                                             CAD_UINT uData0, 
                                             CAD_UINT uData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmBufferState2Read( CAD_UINT uBaseAddress, 
                                      CAD_UINT uAddress,     
                                      CAD_UINT *pData);      
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmBufferState2Write( CAD_UINT uBaseAddress, 
                                       CAD_UINT uAddress,    
                                       CAD_UINT uData);      
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmBufferState2Clear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmBufferState2FillPattern( CAD_UINT uBaseAddress, 
                                             CAD_UINT uData); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmBufferState3Read( CAD_UINT uBaseAddress, 
                                      CAD_UINT uAddress,     
                                      CAD_UINT *pData);      
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmBufferState3Write( CAD_UINT uBaseAddress, 
                                       CAD_UINT uAddress,    
                                       CAD_UINT uData);      
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmBufferState3Clear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmBufferState3FillPattern( CAD_UINT uBaseAddress, 
                                             CAD_UINT uData); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmDequeueState0Read( CAD_UINT uBaseAddress, 
                                       CAD_UINT uAddress,    
                                       CAD_UINT *pData0,     
                                       CAD_UINT *pData1);    
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmDequeueState0Write( CAD_UINT uBaseAddress, 
                                        CAD_UINT uAddress,   
                                        CAD_UINT uData0,     
                                        CAD_UINT uData1);    
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmDequeueState0Clear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmDequeueState0FillPattern( CAD_UINT uBaseAddress, 
                                              CAD_UINT uData0, 
                                              CAD_UINT uData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmDequeueState1Read( CAD_UINT uBaseAddress, 
                                       CAD_UINT uAddress,    
                                       CAD_UINT *pData);     
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmDequeueState1Write( CAD_UINT uBaseAddress, 
                                        CAD_UINT uAddress,   
                                        CAD_UINT uData);     
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmDequeueState1Clear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmDequeueState1FillPattern( CAD_UINT uBaseAddress, 
                                              CAD_UINT uData); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmFreePageFifoRead( CAD_UINT uBaseAddress, 
                                      CAD_UINT uAddress,     
                                      CAD_UINT *pData);      
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmFreePageFifoWrite( CAD_UINT uBaseAddress, 
                                       CAD_UINT uAddress,    
                                       CAD_UINT uData);      
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmFreePageFifoClear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmFreePageFifoFillPattern( CAD_UINT uBaseAddress, 
                                             CAD_UINT uData); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmNextBufferRead( CAD_UINT uBaseAddress,   
                                    CAD_UINT uAddress,       
                                    CAD_UINT *pData);        
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmNextBufferWrite( CAD_UINT uBaseAddress,  
                                     CAD_UINT uAddress,      
                                     CAD_UINT uData);        
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmNextBufferClear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmNextBufferFillPattern( CAD_UINT uBaseAddress, 
                                           CAD_UINT uData);  
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmNextPageRead( CAD_UINT uBaseAddress,     
                                  CAD_UINT uAddress,         
                                  CAD_UINT *pData);          
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmNextPageWrite( CAD_UINT uBaseAddress,    
                                   CAD_UINT uAddress,        
                                   CAD_UINT uData);          
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmNextPageClear( CAD_UINT uBaseAddress);   
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmNextPageFillPattern( CAD_UINT uBaseAddress, 
                                         CAD_UINT uData);    
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmQueueConfigRead( CAD_UINT uBaseAddress,  
                                     CAD_UINT uAddress,      
                                     CAD_UINT *pData0,       
                                     CAD_UINT *pData1,       
                                     CAD_UINT *pData2);      
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmQueueConfigWrite( CAD_UINT uBaseAddress, 
                                      CAD_UINT uAddress,     
                                      CAD_UINT uData0,       
                                      CAD_UINT uData1,       
                                      CAD_UINT uData2);      
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmQueueConfigClear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmQueueConfigFillPattern( CAD_UINT uBaseAddress, 
                                            CAD_UINT uData0, 
                                            CAD_UINT uData1, 
                                            CAD_UINT uData2); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmQueueCountersRead( CAD_UINT uBaseAddress, 
                                       CAD_UINT uAddress,    
                                       CAD_UINT *pData0,     
                                       CAD_UINT *pData1);    
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmQueueCountersWrite( CAD_UINT uBaseAddress, 
                                        CAD_UINT uAddress,   
                                        CAD_UINT uData0,     
                                        CAD_UINT uData1);    
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmQueueCountersClear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmQueueCountersFillPattern( CAD_UINT uBaseAddress, 
                                              CAD_UINT uData0, 
                                              CAD_UINT uData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmQueueHeadPtrRead( CAD_UINT uBaseAddress, 
                                      CAD_UINT uAddress,     
                                      CAD_UINT *pData);      
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmQueueHeadPtrWrite( CAD_UINT uBaseAddress, 
                                       CAD_UINT uAddress,    
                                       CAD_UINT uData);      
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmQueueHeadPtrClear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmQueueHeadPtrFillPattern( CAD_UINT uBaseAddress, 
                                             CAD_UINT uData); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmQueueState0Read( CAD_UINT uBaseAddress,  
                                     CAD_UINT uAddress,      
                                     CAD_UINT *pData);       
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmQueueState0Write( CAD_UINT uBaseAddress, 
                                      CAD_UINT uAddress,     
                                      CAD_UINT uData);       
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmQueueState0Clear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmQueueState0FillPattern( CAD_UINT uBaseAddress, 
                                            CAD_UINT uData); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmQueueState0EnRead( CAD_UINT uBaseAddress, 
                                       CAD_UINT uAddress,    
                                       CAD_UINT *pData);     
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmQueueState0EnWrite( CAD_UINT uBaseAddress, 
                                        CAD_UINT uAddress,   
                                        CAD_UINT uData);     
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmQueueState0EnClear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmQueueState0EnFillPattern( CAD_UINT uBaseAddress, 
                                              CAD_UINT uData); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmQueueState1Read( CAD_UINT uBaseAddress,  
                                     CAD_UINT uAddress,      
                                     CAD_UINT *pData);       
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmQueueState1Write( CAD_UINT uBaseAddress, 
                                      CAD_UINT uAddress,     
                                      CAD_UINT uData);       
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmQueueState1Clear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmQueueState1FillPattern( CAD_UINT uBaseAddress, 
                                            CAD_UINT uData); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmReplicationStateRead( CAD_UINT uBaseAddress, 
                                          CAD_UINT uAddress, 
                                          CAD_UINT *pData);  
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmReplicationStateWrite( CAD_UINT uBaseAddress, 
                                           CAD_UINT uAddress, 
                                           CAD_UINT uData);  
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmReplicationStateClear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_QmReplicationStateFillPattern( CAD_UINT uBaseAddress, 
                                                 CAD_UINT uData); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PmCounterMemoryRead( CAD_UINT uBaseAddress, 
                                       CAD_UINT uAddress,    
                                       CAD_UINT bClearOnRead, 
                                       CAD_UINT *pData0,     
                                       CAD_UINT *pData1);    
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PmCounterMemoryWrite( CAD_UINT uBaseAddress, 
                                        CAD_UINT uAddress,   
                                        CAD_UINT uData0,     
                                        CAD_UINT uData1);    
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PmCounterMemoryClear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PmCounterMemoryFillPattern( CAD_UINT uBaseAddress, 
                                              CAD_UINT uData0, 
                                              CAD_UINT uData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PmProfileMemoryRead( CAD_UINT uBaseAddress, 
                                       CAD_UINT uAddress,    
                                       CAD_UINT *pData0,     
                                       CAD_UINT *pData1);    
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PmProfileMemoryWrite( CAD_UINT uBaseAddress, 
                                        CAD_UINT uAddress,   
                                        CAD_UINT uData0,     
                                        CAD_UINT uData1);    
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PmProfileMemoryClear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PmProfileMemoryFillPattern( CAD_UINT uBaseAddress, 
                                              CAD_UINT uData0, 
                                              CAD_UINT uData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Sr0CounterRead( CAD_UINT uBaseAddress,     
                                  CAD_UINT uAddress,         
                                  CAD_UINT bClearOnRead,     
                                  CAD_UINT *pData0,          
                                  CAD_UINT *pData1);         
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Sr0CounterWrite( CAD_UINT uBaseAddress,    
                                   CAD_UINT uAddress,        
                                   CAD_UINT uData0,          
                                   CAD_UINT uData1);         
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Sr0CounterClear( CAD_UINT uBaseAddress);   
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Sr0CounterFillPattern( CAD_UINT uBaseAddress, 
                                         CAD_UINT uData0,    
                                         CAD_UINT uData1);   
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Sr1CounterRead( CAD_UINT uBaseAddress,     
                                  CAD_UINT uAddress,         
                                  CAD_UINT bClearOnRead,     
                                  CAD_UINT *pData0,          
                                  CAD_UINT *pData1);         
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Sr1CounterWrite( CAD_UINT uBaseAddress,    
                                   CAD_UINT uAddress,        
                                   CAD_UINT uData0,          
                                   CAD_UINT uData1);         
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Sr1CounterClear( CAD_UINT uBaseAddress);   
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Sr1CounterFillPattern( CAD_UINT uBaseAddress, 
                                         CAD_UINT uData0,    
                                         CAD_UINT uData1);   
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpAggregateHashBitConfigRead( CAD_UINT uBaseAddress, 
                                                CAD_UINT uAddress, 
                                                CAD_UINT *pData0, 
                                                CAD_UINT *pData1, 
                                                CAD_UINT *pData2, 
                                                CAD_UINT *pData3, 
                                                CAD_UINT *pData4, 
                                                CAD_UINT *pData5, 
                                                CAD_UINT *pData6, 
                                                CAD_UINT *pData7, 
                                                CAD_UINT *pData8, 
                                                CAD_UINT *pData9); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpAggregateHashBitConfigWrite( CAD_UINT uBaseAddress, 
                                                 CAD_UINT uAddress, 
                                                 CAD_UINT uData0, 
                                                 CAD_UINT uData1, 
                                                 CAD_UINT uData2, 
                                                 CAD_UINT uData3, 
                                                 CAD_UINT uData4, 
                                                 CAD_UINT uData5, 
                                                 CAD_UINT uData6, 
                                                 CAD_UINT uData7, 
                                                 CAD_UINT uData8, 
                                                 CAD_UINT uData9); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpAggregateHashBitConfigClear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpAggregateHashBitConfigFillPattern( CAD_UINT uBaseAddress, 
                                                       CAD_UINT uData0, 
                                                       CAD_UINT uData1, 
                                                       CAD_UINT uData2, 
                                                       CAD_UINT uData3, 
                                                       CAD_UINT uData4, 
                                                       CAD_UINT uData5, 
                                                       CAD_UINT uData6, 
                                                       CAD_UINT uData7, 
                                                       CAD_UINT uData8, 
                                                       CAD_UINT uData9); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpAggregateHashByteConfigRead( CAD_UINT uBaseAddress, 
                                                 CAD_UINT uAddress, 
                                                 CAD_UINT *pData0, 
                                                 CAD_UINT *pData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpAggregateHashByteConfigWrite( CAD_UINT uBaseAddress, 
                                                  CAD_UINT uAddress, 
                                                  CAD_UINT uData0, 
                                                  CAD_UINT uData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpAggregateHashByteConfigClear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpAggregateHashByteConfigFillPattern( CAD_UINT uBaseAddress, 
                                                        CAD_UINT uData0, 
                                                        CAD_UINT uData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpCamConfigurationInstance0Read( CAD_UINT uBaseAddress, 
                                                   CAD_UINT uAddress, 
                                                   CAD_UINT *pData0, 
                                                   CAD_UINT *pData1, 
                                                   CAD_UINT *pData2, 
                                                   CAD_UINT *pData3, 
                                                   CAD_UINT *pData4, 
                                                   CAD_UINT *pData5, 
                                                   CAD_UINT *pData6, 
                                                   CAD_UINT *pData7, 
                                                   CAD_UINT *pData8, 
                                                   CAD_UINT *pData9, 
                                                   CAD_UINT *pData10, 
                                                   CAD_UINT *pData11); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpCamConfigurationInstance0Write( CAD_UINT uBaseAddress, 
                                                    CAD_UINT uAddress, 
                                                    CAD_UINT uData0, 
                                                    CAD_UINT uData1, 
                                                    CAD_UINT uData2, 
                                                    CAD_UINT uData3, 
                                                    CAD_UINT uData4, 
                                                    CAD_UINT uData5, 
                                                    CAD_UINT uData6, 
                                                    CAD_UINT uData7, 
                                                    CAD_UINT uData8, 
                                                    CAD_UINT uData9, 
                                                    CAD_UINT uData10, 
                                                    CAD_UINT uData11); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpCamConfigurationInstance0Clear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpCamConfigurationInstance0FillPattern( CAD_UINT uBaseAddress, 
                                                          CAD_UINT uData0, 
                                                          CAD_UINT uData1, 
                                                          CAD_UINT uData2, 
                                                          CAD_UINT uData3, 
                                                          CAD_UINT uData4, 
                                                          CAD_UINT uData5, 
                                                          CAD_UINT uData6, 
                                                          CAD_UINT uData7, 
                                                          CAD_UINT uData8, 
                                                          CAD_UINT uData9, 
                                                          CAD_UINT uData10, 
                                                          CAD_UINT uData11); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpCamConfigurationInstance1Read( CAD_UINT uBaseAddress, 
                                                   CAD_UINT uAddress, 
                                                   CAD_UINT *pData0, 
                                                   CAD_UINT *pData1, 
                                                   CAD_UINT *pData2, 
                                                   CAD_UINT *pData3, 
                                                   CAD_UINT *pData4, 
                                                   CAD_UINT *pData5, 
                                                   CAD_UINT *pData6, 
                                                   CAD_UINT *pData7, 
                                                   CAD_UINT *pData8, 
                                                   CAD_UINT *pData9, 
                                                   CAD_UINT *pData10, 
                                                   CAD_UINT *pData11); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpCamConfigurationInstance1Write( CAD_UINT uBaseAddress, 
                                                    CAD_UINT uAddress, 
                                                    CAD_UINT uData0, 
                                                    CAD_UINT uData1, 
                                                    CAD_UINT uData2, 
                                                    CAD_UINT uData3, 
                                                    CAD_UINT uData4, 
                                                    CAD_UINT uData5, 
                                                    CAD_UINT uData6, 
                                                    CAD_UINT uData7, 
                                                    CAD_UINT uData8, 
                                                    CAD_UINT uData9, 
                                                    CAD_UINT uData10, 
                                                    CAD_UINT uData11); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpCamConfigurationInstance1Clear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpCamConfigurationInstance1FillPattern( CAD_UINT uBaseAddress, 
                                                          CAD_UINT uData0, 
                                                          CAD_UINT uData1, 
                                                          CAD_UINT uData2, 
                                                          CAD_UINT uData3, 
                                                          CAD_UINT uData4, 
                                                          CAD_UINT uData5, 
                                                          CAD_UINT uData6, 
                                                          CAD_UINT uData7, 
                                                          CAD_UINT uData8, 
                                                          CAD_UINT uData9, 
                                                          CAD_UINT uData10, 
                                                          CAD_UINT uData11); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpCamConfigurationInstance2Read( CAD_UINT uBaseAddress, 
                                                   CAD_UINT uAddress, 
                                                   CAD_UINT *pData0, 
                                                   CAD_UINT *pData1, 
                                                   CAD_UINT *pData2, 
                                                   CAD_UINT *pData3, 
                                                   CAD_UINT *pData4, 
                                                   CAD_UINT *pData5, 
                                                   CAD_UINT *pData6, 
                                                   CAD_UINT *pData7, 
                                                   CAD_UINT *pData8, 
                                                   CAD_UINT *pData9, 
                                                   CAD_UINT *pData10, 
                                                   CAD_UINT *pData11); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpCamConfigurationInstance2Write( CAD_UINT uBaseAddress, 
                                                    CAD_UINT uAddress, 
                                                    CAD_UINT uData0, 
                                                    CAD_UINT uData1, 
                                                    CAD_UINT uData2, 
                                                    CAD_UINT uData3, 
                                                    CAD_UINT uData4, 
                                                    CAD_UINT uData5, 
                                                    CAD_UINT uData6, 
                                                    CAD_UINT uData7, 
                                                    CAD_UINT uData8, 
                                                    CAD_UINT uData9, 
                                                    CAD_UINT uData10, 
                                                    CAD_UINT uData11); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpCamConfigurationInstance2Clear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpCamConfigurationInstance2FillPattern( CAD_UINT uBaseAddress, 
                                                          CAD_UINT uData0, 
                                                          CAD_UINT uData1, 
                                                          CAD_UINT uData2, 
                                                          CAD_UINT uData3, 
                                                          CAD_UINT uData4, 
                                                          CAD_UINT uData5, 
                                                          CAD_UINT uData6, 
                                                          CAD_UINT uData7, 
                                                          CAD_UINT uData8, 
                                                          CAD_UINT uData9, 
                                                          CAD_UINT uData10, 
                                                          CAD_UINT uData11); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpCamConfigurationInstance3Read( CAD_UINT uBaseAddress, 
                                                   CAD_UINT uAddress, 
                                                   CAD_UINT *pData0, 
                                                   CAD_UINT *pData1, 
                                                   CAD_UINT *pData2, 
                                                   CAD_UINT *pData3, 
                                                   CAD_UINT *pData4, 
                                                   CAD_UINT *pData5, 
                                                   CAD_UINT *pData6, 
                                                   CAD_UINT *pData7, 
                                                   CAD_UINT *pData8, 
                                                   CAD_UINT *pData9, 
                                                   CAD_UINT *pData10, 
                                                   CAD_UINT *pData11); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpCamConfigurationInstance3Write( CAD_UINT uBaseAddress, 
                                                    CAD_UINT uAddress, 
                                                    CAD_UINT uData0, 
                                                    CAD_UINT uData1, 
                                                    CAD_UINT uData2, 
                                                    CAD_UINT uData3, 
                                                    CAD_UINT uData4, 
                                                    CAD_UINT uData5, 
                                                    CAD_UINT uData6, 
                                                    CAD_UINT uData7, 
                                                    CAD_UINT uData8, 
                                                    CAD_UINT uData9, 
                                                    CAD_UINT uData10, 
                                                    CAD_UINT uData11); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpCamConfigurationInstance3Clear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpCamConfigurationInstance3FillPattern( CAD_UINT uBaseAddress, 
                                                          CAD_UINT uData0, 
                                                          CAD_UINT uData1, 
                                                          CAD_UINT uData2, 
                                                          CAD_UINT uData3, 
                                                          CAD_UINT uData4, 
                                                          CAD_UINT uData5, 
                                                          CAD_UINT uData6, 
                                                          CAD_UINT uData7, 
                                                          CAD_UINT uData8, 
                                                          CAD_UINT uData9, 
                                                          CAD_UINT uData10, 
                                                          CAD_UINT uData11); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpCamRamConfigurationInstance0Read( CAD_UINT uBaseAddress, 
                                                      CAD_UINT uAddress, 
                                                      CAD_UINT *pData0, 
                                                      CAD_UINT *pData1, 
                                                      CAD_UINT *pData2, 
                                                      CAD_UINT *pData3, 
                                                      CAD_UINT *pData4); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpCamRamConfigurationInstance0Write( CAD_UINT uBaseAddress, 
                                                       CAD_UINT uAddress, 
                                                       CAD_UINT uData0, 
                                                       CAD_UINT uData1, 
                                                       CAD_UINT uData2, 
                                                       CAD_UINT uData3, 
                                                       CAD_UINT uData4); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpCamRamConfigurationInstance0Clear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpCamRamConfigurationInstance0FillPattern( CAD_UINT uBaseAddress, 
                                                             CAD_UINT uData0, 
                                                             CAD_UINT uData1, 
                                                             CAD_UINT uData2, 
                                                             CAD_UINT uData3, 
                                                             CAD_UINT uData4); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpCamRamConfigurationInstance1Read( CAD_UINT uBaseAddress, 
                                                      CAD_UINT uAddress, 
                                                      CAD_UINT *pData0, 
                                                      CAD_UINT *pData1, 
                                                      CAD_UINT *pData2, 
                                                      CAD_UINT *pData3, 
                                                      CAD_UINT *pData4); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpCamRamConfigurationInstance1Write( CAD_UINT uBaseAddress, 
                                                       CAD_UINT uAddress, 
                                                       CAD_UINT uData0, 
                                                       CAD_UINT uData1, 
                                                       CAD_UINT uData2, 
                                                       CAD_UINT uData3, 
                                                       CAD_UINT uData4); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpCamRamConfigurationInstance1Clear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpCamRamConfigurationInstance1FillPattern( CAD_UINT uBaseAddress, 
                                                             CAD_UINT uData0, 
                                                             CAD_UINT uData1, 
                                                             CAD_UINT uData2, 
                                                             CAD_UINT uData3, 
                                                             CAD_UINT uData4); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpCamRamConfigurationInstance2Read( CAD_UINT uBaseAddress, 
                                                      CAD_UINT uAddress, 
                                                      CAD_UINT *pData0, 
                                                      CAD_UINT *pData1, 
                                                      CAD_UINT *pData2, 
                                                      CAD_UINT *pData3, 
                                                      CAD_UINT *pData4); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpCamRamConfigurationInstance2Write( CAD_UINT uBaseAddress, 
                                                       CAD_UINT uAddress, 
                                                       CAD_UINT uData0, 
                                                       CAD_UINT uData1, 
                                                       CAD_UINT uData2, 
                                                       CAD_UINT uData3, 
                                                       CAD_UINT uData4); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpCamRamConfigurationInstance2Clear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpCamRamConfigurationInstance2FillPattern( CAD_UINT uBaseAddress, 
                                                             CAD_UINT uData0, 
                                                             CAD_UINT uData1, 
                                                             CAD_UINT uData2, 
                                                             CAD_UINT uData3, 
                                                             CAD_UINT uData4); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpCamRamConfigurationInstance3Read( CAD_UINT uBaseAddress, 
                                                      CAD_UINT uAddress, 
                                                      CAD_UINT *pData0, 
                                                      CAD_UINT *pData1, 
                                                      CAD_UINT *pData2, 
                                                      CAD_UINT *pData3, 
                                                      CAD_UINT *pData4); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpCamRamConfigurationInstance3Write( CAD_UINT uBaseAddress, 
                                                       CAD_UINT uAddress, 
                                                       CAD_UINT uData0, 
                                                       CAD_UINT uData1, 
                                                       CAD_UINT uData2, 
                                                       CAD_UINT uData3, 
                                                       CAD_UINT uData4); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpCamRamConfigurationInstance3Clear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpCamRamConfigurationInstance3FillPattern( CAD_UINT uBaseAddress, 
                                                             CAD_UINT uData0, 
                                                             CAD_UINT uData1, 
                                                             CAD_UINT uData2, 
                                                             CAD_UINT uData3, 
                                                             CAD_UINT uData4); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpHeaderRecordSizeRead( CAD_UINT uBaseAddress, 
                                          CAD_UINT uAddress, 
                                          CAD_UINT *pData);  
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpHeaderRecordSizeWrite( CAD_UINT uBaseAddress, 
                                           CAD_UINT uAddress, 
                                           CAD_UINT uData);  
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpHeaderRecordSizeClear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpHeaderRecordSizeFillPattern( CAD_UINT uBaseAddress, 
                                                 CAD_UINT uData); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpInitialQueueStateRead( CAD_UINT uBaseAddress, 
                                           CAD_UINT uAddress, 
                                           CAD_UINT *pData0, 
                                           CAD_UINT *pData1, 
                                           CAD_UINT *pData2); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpInitialQueueStateWrite( CAD_UINT uBaseAddress, 
                                            CAD_UINT uAddress, 
                                            CAD_UINT uData0, 
                                            CAD_UINT uData1, 
                                            CAD_UINT uData2); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpInitialQueueStateClear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpInitialQueueStateFillPattern( CAD_UINT uBaseAddress, 
                                                  CAD_UINT uData0, 
                                                  CAD_UINT uData1, 
                                                  CAD_UINT uData2); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpPpOutHeaderCopyRead( CAD_UINT uBaseAddress, 
                                         CAD_UINT uAddress,  
                                         CAD_UINT *pData0,   
                                         CAD_UINT *pData1,   
                                         CAD_UINT *pData2,   
                                         CAD_UINT *pData3,   
                                         CAD_UINT *pData4,   
                                         CAD_UINT *pData5,   
                                         CAD_UINT *pData6,   
                                         CAD_UINT *pData7);  
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpPpOutHeaderCopyWrite( CAD_UINT uBaseAddress, 
                                          CAD_UINT uAddress, 
                                          CAD_UINT uData0,   
                                          CAD_UINT uData1,   
                                          CAD_UINT uData2,   
                                          CAD_UINT uData3,   
                                          CAD_UINT uData4,   
                                          CAD_UINT uData5,   
                                          CAD_UINT uData6,   
                                          CAD_UINT uData7);  
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpPpOutHeaderCopyClear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpPpOutHeaderCopyFillPattern( CAD_UINT uBaseAddress, 
                                                CAD_UINT uData0, 
                                                CAD_UINT uData1, 
                                                CAD_UINT uData2, 
                                                CAD_UINT uData3, 
                                                CAD_UINT uData4, 
                                                CAD_UINT uData5, 
                                                CAD_UINT uData6, 
                                                CAD_UINT uData7); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpQueuePriorityGroupRead( CAD_UINT uBaseAddress, 
                                            CAD_UINT uAddress, 
                                            CAD_UINT *pData); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpQueuePriorityGroupWrite( CAD_UINT uBaseAddress, 
                                             CAD_UINT uAddress, 
                                             CAD_UINT uData); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpQueuePriorityGroupClear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpQueuePriorityGroupFillPattern( CAD_UINT uBaseAddress, 
                                                   CAD_UINT uData); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpRxPortDataRead( CAD_UINT uBaseAddress,   
                                    CAD_UINT uAddress,       
                                    CAD_UINT *pData0,        
                                    CAD_UINT *pData1,        
                                    CAD_UINT *pData2,        
                                    CAD_UINT *pData3);       
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpRxPortDataWrite( CAD_UINT uBaseAddress,  
                                     CAD_UINT uAddress,      
                                     CAD_UINT uData0,        
                                     CAD_UINT uData1,        
                                     CAD_UINT uData2,        
                                     CAD_UINT uData3);       
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpRxPortDataClear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PpRxPortDataFillPattern( CAD_UINT uBaseAddress, 
                                           CAD_UINT uData0,  
                                           CAD_UINT uData1,  
                                           CAD_UINT uData2,  
                                           CAD_UINT uData3); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Rc0DataRead( CAD_UINT uBaseAddress,        
                               CAD_UINT uAddress,            
                               CAD_UINT *pData0,             
                               CAD_UINT *pData1,             
                               CAD_UINT *pData2,             
                               CAD_UINT *pData3,             
                               CAD_UINT *pData4,             
                               CAD_UINT *pData5,             
                               CAD_UINT *pData6,             
                               CAD_UINT *pData7,             
                               CAD_UINT *pData8,             
                               CAD_UINT *pData9,             
                               CAD_UINT *pData10,            
                               CAD_UINT *pData11,            
                               CAD_UINT *pData12,            
                               CAD_UINT *pData13,            
                               CAD_UINT *pData14,            
                               CAD_UINT *pData15,            
                               CAD_UINT *pData16,            
                               CAD_UINT *pData17,            
                               CAD_UINT *pData18,            
                               CAD_UINT *pData19,            
                               CAD_UINT *pData20,            
                               CAD_UINT *pData21,            
                               CAD_UINT *pData22,            
                               CAD_UINT *pData23,            
                               CAD_UINT *pData24,            
                               CAD_UINT *pData25,            
                               CAD_UINT *pData26,            
                               CAD_UINT *pData27,            
                               CAD_UINT *pData28,            
                               CAD_UINT *pData29,            
                               CAD_UINT *pData30,            
                               CAD_UINT *pData31,            
                               CAD_UINT *pData32,            
                               CAD_UINT *pData33,            
                               CAD_UINT *pData34,            
                               CAD_UINT *pData35);           
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Rc0DataWrite( CAD_UINT uBaseAddress,       
                                CAD_UINT uAddress,           
                                CAD_UINT uData0,             
                                CAD_UINT uData1,             
                                CAD_UINT uData2,             
                                CAD_UINT uData3,             
                                CAD_UINT uData4,             
                                CAD_UINT uData5,             
                                CAD_UINT uData6,             
                                CAD_UINT uData7,             
                                CAD_UINT uData8,             
                                CAD_UINT uData9,             
                                CAD_UINT uData10,            
                                CAD_UINT uData11,            
                                CAD_UINT uData12,            
                                CAD_UINT uData13,            
                                CAD_UINT uData14,            
                                CAD_UINT uData15,            
                                CAD_UINT uData16,            
                                CAD_UINT uData17,            
                                CAD_UINT uData18,            
                                CAD_UINT uData19,            
                                CAD_UINT uData20,            
                                CAD_UINT uData21,            
                                CAD_UINT uData22,            
                                CAD_UINT uData23,            
                                CAD_UINT uData24,            
                                CAD_UINT uData25,            
                                CAD_UINT uData26,            
                                CAD_UINT uData27,            
                                CAD_UINT uData28,            
                                CAD_UINT uData29,            
                                CAD_UINT uData30,            
                                CAD_UINT uData31,            
                                CAD_UINT uData32,            
                                CAD_UINT uData33,            
                                CAD_UINT uData34,            
                                CAD_UINT uData35);           
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Rc0DataClear( CAD_UINT uBaseAddress);      
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Rc0DataFillPattern( CAD_UINT uBaseAddress, 
                                      CAD_UINT uData0,       
                                      CAD_UINT uData1,       
                                      CAD_UINT uData2,       
                                      CAD_UINT uData3,       
                                      CAD_UINT uData4,       
                                      CAD_UINT uData5,       
                                      CAD_UINT uData6,       
                                      CAD_UINT uData7,       
                                      CAD_UINT uData8,       
                                      CAD_UINT uData9,       
                                      CAD_UINT uData10,      
                                      CAD_UINT uData11,      
                                      CAD_UINT uData12,      
                                      CAD_UINT uData13,      
                                      CAD_UINT uData14,      
                                      CAD_UINT uData15,      
                                      CAD_UINT uData16,      
                                      CAD_UINT uData17,      
                                      CAD_UINT uData18,      
                                      CAD_UINT uData19,      
                                      CAD_UINT uData20,      
                                      CAD_UINT uData21,      
                                      CAD_UINT uData22,      
                                      CAD_UINT uData23,      
                                      CAD_UINT uData24,      
                                      CAD_UINT uData25,      
                                      CAD_UINT uData26,      
                                      CAD_UINT uData27,      
                                      CAD_UINT uData28,      
                                      CAD_UINT uData29,      
                                      CAD_UINT uData30,      
                                      CAD_UINT uData31,      
                                      CAD_UINT uData32,      
                                      CAD_UINT uData33,      
                                      CAD_UINT uData34,      
                                      CAD_UINT uData35);     
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Rc1DataRead( CAD_UINT uBaseAddress,        
                               CAD_UINT uAddress,            
                               CAD_UINT *pData0,             
                               CAD_UINT *pData1,             
                               CAD_UINT *pData2,             
                               CAD_UINT *pData3,             
                               CAD_UINT *pData4,             
                               CAD_UINT *pData5,             
                               CAD_UINT *pData6,             
                               CAD_UINT *pData7,             
                               CAD_UINT *pData8,             
                               CAD_UINT *pData9,             
                               CAD_UINT *pData10,            
                               CAD_UINT *pData11,            
                               CAD_UINT *pData12,            
                               CAD_UINT *pData13,            
                               CAD_UINT *pData14,            
                               CAD_UINT *pData15,            
                               CAD_UINT *pData16,            
                               CAD_UINT *pData17,            
                               CAD_UINT *pData18,            
                               CAD_UINT *pData19,            
                               CAD_UINT *pData20,            
                               CAD_UINT *pData21,            
                               CAD_UINT *pData22,            
                               CAD_UINT *pData23,            
                               CAD_UINT *pData24,            
                               CAD_UINT *pData25,            
                               CAD_UINT *pData26,            
                               CAD_UINT *pData27,            
                               CAD_UINT *pData28,            
                               CAD_UINT *pData29,            
                               CAD_UINT *pData30,            
                               CAD_UINT *pData31,            
                               CAD_UINT *pData32,            
                               CAD_UINT *pData33,            
                               CAD_UINT *pData34,            
                               CAD_UINT *pData35);           
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Rc1DataWrite( CAD_UINT uBaseAddress,       
                                CAD_UINT uAddress,           
                                CAD_UINT uData0,             
                                CAD_UINT uData1,             
                                CAD_UINT uData2,             
                                CAD_UINT uData3,             
                                CAD_UINT uData4,             
                                CAD_UINT uData5,             
                                CAD_UINT uData6,             
                                CAD_UINT uData7,             
                                CAD_UINT uData8,             
                                CAD_UINT uData9,             
                                CAD_UINT uData10,            
                                CAD_UINT uData11,            
                                CAD_UINT uData12,            
                                CAD_UINT uData13,            
                                CAD_UINT uData14,            
                                CAD_UINT uData15,            
                                CAD_UINT uData16,            
                                CAD_UINT uData17,            
                                CAD_UINT uData18,            
                                CAD_UINT uData19,            
                                CAD_UINT uData20,            
                                CAD_UINT uData21,            
                                CAD_UINT uData22,            
                                CAD_UINT uData23,            
                                CAD_UINT uData24,            
                                CAD_UINT uData25,            
                                CAD_UINT uData26,            
                                CAD_UINT uData27,            
                                CAD_UINT uData28,            
                                CAD_UINT uData29,            
                                CAD_UINT uData30,            
                                CAD_UINT uData31,            
                                CAD_UINT uData32,            
                                CAD_UINT uData33,            
                                CAD_UINT uData34,            
                                CAD_UINT uData35);           
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Rc1DataClear( CAD_UINT uBaseAddress);      
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Rc1DataFillPattern( CAD_UINT uBaseAddress, 
                                      CAD_UINT uData0,       
                                      CAD_UINT uData1,       
                                      CAD_UINT uData2,       
                                      CAD_UINT uData3,       
                                      CAD_UINT uData4,       
                                      CAD_UINT uData5,       
                                      CAD_UINT uData6,       
                                      CAD_UINT uData7,       
                                      CAD_UINT uData8,       
                                      CAD_UINT uData9,       
                                      CAD_UINT uData10,      
                                      CAD_UINT uData11,      
                                      CAD_UINT uData12,      
                                      CAD_UINT uData13,      
                                      CAD_UINT uData14,      
                                      CAD_UINT uData15,      
                                      CAD_UINT uData16,      
                                      CAD_UINT uData17,      
                                      CAD_UINT uData18,      
                                      CAD_UINT uData19,      
                                      CAD_UINT uData20,      
                                      CAD_UINT uData21,      
                                      CAD_UINT uData22,      
                                      CAD_UINT uData23,      
                                      CAD_UINT uData24,      
                                      CAD_UINT uData25,      
                                      CAD_UINT uData26,      
                                      CAD_UINT uData27,      
                                      CAD_UINT uData28,      
                                      CAD_UINT uData29,      
                                      CAD_UINT uData30,      
                                      CAD_UINT uData31,      
                                      CAD_UINT uData32,      
                                      CAD_UINT uData33,      
                                      CAD_UINT uData34,      
                                      CAD_UINT uData35);     
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_St0CounterRead( CAD_UINT uBaseAddress,     
                                  CAD_UINT uAddress,         
                                  CAD_UINT bClearOnRead,     
                                  CAD_UINT *pData0,          
                                  CAD_UINT *pData1);         
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_St0CounterWrite( CAD_UINT uBaseAddress,    
                                   CAD_UINT uAddress,        
                                   CAD_UINT uData0,          
                                   CAD_UINT uData1);         
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_St0CounterClear( CAD_UINT uBaseAddress);   
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_St0CounterFillPattern( CAD_UINT uBaseAddress, 
                                         CAD_UINT uData0,    
                                         CAD_UINT uData1);   
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_St1CounterRead( CAD_UINT uBaseAddress,     
                                  CAD_UINT uAddress,         
                                  CAD_UINT bClearOnRead,     
                                  CAD_UINT *pData0,          
                                  CAD_UINT *pData1);         
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_St1CounterWrite( CAD_UINT uBaseAddress,    
                                   CAD_UINT uAddress,        
                                   CAD_UINT uData0,          
                                   CAD_UINT uData1);         
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_St1CounterClear( CAD_UINT uBaseAddress);   
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_St1CounterFillPattern( CAD_UINT uBaseAddress, 
                                         CAD_UINT uData0,    
                                         CAD_UINT uData1);   
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Mm0Internal0MemoryRead( CAD_UINT uBaseAddress, 
                                          CAD_UINT uAddress, 
                                          CAD_UINT *pData0,  
                                          CAD_UINT *pData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Mm0Internal0MemoryWrite( CAD_UINT uBaseAddress, 
                                           CAD_UINT uAddress, 
                                           CAD_UINT uData0,  
                                           CAD_UINT uData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Mm0Internal0MemoryClear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Mm0Internal0MemoryFillPattern( CAD_UINT uBaseAddress, 
                                                 CAD_UINT uData0, 
                                                 CAD_UINT uData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Mm0Internal1MemoryRead( CAD_UINT uBaseAddress, 
                                          CAD_UINT uAddress, 
                                          CAD_UINT *pData0,  
                                          CAD_UINT *pData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Mm0Internal1MemoryWrite( CAD_UINT uBaseAddress, 
                                           CAD_UINT uAddress, 
                                           CAD_UINT uData0,  
                                           CAD_UINT uData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Mm0Internal1MemoryClear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Mm0Internal1MemoryFillPattern( CAD_UINT uBaseAddress, 
                                                 CAD_UINT uData0, 
                                                 CAD_UINT uData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Mm0NarrowPort0MemoryRead( CAD_UINT uBaseAddress, 
                                            CAD_UINT uAddress, 
                                            CAD_UINT *pData0, 
                                            CAD_UINT *pData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Mm0NarrowPort0MemoryWrite( CAD_UINT uBaseAddress, 
                                             CAD_UINT uAddress, 
                                             CAD_UINT uData0, 
                                             CAD_UINT uData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Mm0NarrowPort0MemoryClear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Mm0NarrowPort0MemoryFillPattern( CAD_UINT uBaseAddress, 
                                                   CAD_UINT uData0, 
                                                   CAD_UINT uData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Mm0NarrowPort1MemoryRead( CAD_UINT uBaseAddress, 
                                            CAD_UINT uAddress, 
                                            CAD_UINT *pData0, 
                                            CAD_UINT *pData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Mm0NarrowPort1MemoryWrite( CAD_UINT uBaseAddress, 
                                             CAD_UINT uAddress, 
                                             CAD_UINT uData0, 
                                             CAD_UINT uData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Mm0NarrowPort1MemoryClear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Mm0NarrowPort1MemoryFillPattern( CAD_UINT uBaseAddress, 
                                                   CAD_UINT uData0, 
                                                   CAD_UINT uData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Mm0WidePortMemoryRead( CAD_UINT uBaseAddress, 
                                         CAD_UINT uAddress,  
                                         CAD_UINT *pData0,   
                                         CAD_UINT *pData1);  
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Mm0WidePortMemoryWrite( CAD_UINT uBaseAddress, 
                                          CAD_UINT uAddress, 
                                          CAD_UINT uData0,   
                                          CAD_UINT uData1);  
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Mm0WidePortMemoryClear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Mm0WidePortMemoryFillPattern( CAD_UINT uBaseAddress, 
                                                CAD_UINT uData0, 
                                                CAD_UINT uData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Mm1Internal0MemoryRead( CAD_UINT uBaseAddress, 
                                          CAD_UINT uAddress, 
                                          CAD_UINT *pData0,  
                                          CAD_UINT *pData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Mm1Internal0MemoryWrite( CAD_UINT uBaseAddress, 
                                           CAD_UINT uAddress, 
                                           CAD_UINT uData0,  
                                           CAD_UINT uData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Mm1Internal0MemoryClear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Mm1Internal0MemoryFillPattern( CAD_UINT uBaseAddress, 
                                                 CAD_UINT uData0, 
                                                 CAD_UINT uData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Mm1Internal1MemoryRead( CAD_UINT uBaseAddress, 
                                          CAD_UINT uAddress, 
                                          CAD_UINT *pData0,  
                                          CAD_UINT *pData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Mm1Internal1MemoryWrite( CAD_UINT uBaseAddress, 
                                           CAD_UINT uAddress, 
                                           CAD_UINT uData0,  
                                           CAD_UINT uData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Mm1Internal1MemoryClear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Mm1Internal1MemoryFillPattern( CAD_UINT uBaseAddress, 
                                                 CAD_UINT uData0, 
                                                 CAD_UINT uData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Mm1NarrowPort0MemoryRead( CAD_UINT uBaseAddress, 
                                            CAD_UINT uAddress, 
                                            CAD_UINT *pData0, 
                                            CAD_UINT *pData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Mm1NarrowPort0MemoryWrite( CAD_UINT uBaseAddress, 
                                             CAD_UINT uAddress, 
                                             CAD_UINT uData0, 
                                             CAD_UINT uData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Mm1NarrowPort0MemoryClear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Mm1NarrowPort0MemoryFillPattern( CAD_UINT uBaseAddress, 
                                                   CAD_UINT uData0, 
                                                   CAD_UINT uData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Mm1NarrowPort1MemoryRead( CAD_UINT uBaseAddress, 
                                            CAD_UINT uAddress, 
                                            CAD_UINT *pData0, 
                                            CAD_UINT *pData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Mm1NarrowPort1MemoryWrite( CAD_UINT uBaseAddress, 
                                             CAD_UINT uAddress, 
                                             CAD_UINT uData0, 
                                             CAD_UINT uData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Mm1NarrowPort1MemoryClear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Mm1NarrowPort1MemoryFillPattern( CAD_UINT uBaseAddress, 
                                                   CAD_UINT uData0, 
                                                   CAD_UINT uData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Mm1WidePortMemoryRead( CAD_UINT uBaseAddress, 
                                         CAD_UINT uAddress,  
                                         CAD_UINT *pData0,   
                                         CAD_UINT *pData1);  
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Mm1WidePortMemoryWrite( CAD_UINT uBaseAddress, 
                                          CAD_UINT uAddress, 
                                          CAD_UINT uData0,   
                                          CAD_UINT uData1);  
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Mm1WidePortMemoryClear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Mm1WidePortMemoryFillPattern( CAD_UINT uBaseAddress, 
                                                CAD_UINT uData0, 
                                                CAD_UINT uData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Pb0DataRead( CAD_UINT uBaseAddress,        
                               CAD_UINT uAddress,            
                               CAD_UINT *pData0,             
                               CAD_UINT *pData1,             
                               CAD_UINT *pData2,             
                               CAD_UINT *pData3,             
                               CAD_UINT *pData4,             
                               CAD_UINT *pData5,             
                               CAD_UINT *pData6,             
                               CAD_UINT *pData7,             
                               CAD_UINT *pData8,             
                               CAD_UINT *pData9,             
                               CAD_UINT *pData10,            
                               CAD_UINT *pData11,            
                               CAD_UINT *pData12,            
                               CAD_UINT *pData13,            
                               CAD_UINT *pData14,            
                               CAD_UINT *pData15,            
                               CAD_UINT *pData16,            
                               CAD_UINT *pData17,            
                               CAD_UINT *pData18,            
                               CAD_UINT *pData19,            
                               CAD_UINT *pData20,            
                               CAD_UINT *pData21,            
                               CAD_UINT *pData22,            
                               CAD_UINT *pData23);           
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Pb0DataWrite( CAD_UINT uBaseAddress,       
                                CAD_UINT uAddress,           
                                CAD_UINT uData0,             
                                CAD_UINT uData1,             
                                CAD_UINT uData2,             
                                CAD_UINT uData3,             
                                CAD_UINT uData4,             
                                CAD_UINT uData5,             
                                CAD_UINT uData6,             
                                CAD_UINT uData7,             
                                CAD_UINT uData8,             
                                CAD_UINT uData9,             
                                CAD_UINT uData10,            
                                CAD_UINT uData11,            
                                CAD_UINT uData12,            
                                CAD_UINT uData13,            
                                CAD_UINT uData14,            
                                CAD_UINT uData15,            
                                CAD_UINT uData16,            
                                CAD_UINT uData17,            
                                CAD_UINT uData18,            
                                CAD_UINT uData19,            
                                CAD_UINT uData20,            
                                CAD_UINT uData21,            
                                CAD_UINT uData22,            
                                CAD_UINT uData23);           
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Pb0DataClear( CAD_UINT uBaseAddress);      
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Pb0DataFillPattern( CAD_UINT uBaseAddress, 
                                      CAD_UINT uData0,       
                                      CAD_UINT uData1,       
                                      CAD_UINT uData2,       
                                      CAD_UINT uData3,       
                                      CAD_UINT uData4,       
                                      CAD_UINT uData5,       
                                      CAD_UINT uData6,       
                                      CAD_UINT uData7,       
                                      CAD_UINT uData8,       
                                      CAD_UINT uData9,       
                                      CAD_UINT uData10,      
                                      CAD_UINT uData11,      
                                      CAD_UINT uData12,      
                                      CAD_UINT uData13,      
                                      CAD_UINT uData14,      
                                      CAD_UINT uData15,      
                                      CAD_UINT uData16,      
                                      CAD_UINT uData17,      
                                      CAD_UINT uData18,      
                                      CAD_UINT uData19,      
                                      CAD_UINT uData20,      
                                      CAD_UINT uData21,      
                                      CAD_UINT uData22,      
                                      CAD_UINT uData23);     
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Pb1DataRead( CAD_UINT uBaseAddress,        
                               CAD_UINT uAddress,            
                               CAD_UINT *pData0,             
                               CAD_UINT *pData1,             
                               CAD_UINT *pData2,             
                               CAD_UINT *pData3,             
                               CAD_UINT *pData4,             
                               CAD_UINT *pData5,             
                               CAD_UINT *pData6,             
                               CAD_UINT *pData7,             
                               CAD_UINT *pData8,             
                               CAD_UINT *pData9,             
                               CAD_UINT *pData10,            
                               CAD_UINT *pData11,            
                               CAD_UINT *pData12,            
                               CAD_UINT *pData13,            
                               CAD_UINT *pData14,            
                               CAD_UINT *pData15,            
                               CAD_UINT *pData16,            
                               CAD_UINT *pData17,            
                               CAD_UINT *pData18,            
                               CAD_UINT *pData19,            
                               CAD_UINT *pData20,            
                               CAD_UINT *pData21,            
                               CAD_UINT *pData22,            
                               CAD_UINT *pData23);           
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Pb1DataWrite( CAD_UINT uBaseAddress,       
                                CAD_UINT uAddress,           
                                CAD_UINT uData0,             
                                CAD_UINT uData1,             
                                CAD_UINT uData2,             
                                CAD_UINT uData3,             
                                CAD_UINT uData4,             
                                CAD_UINT uData5,             
                                CAD_UINT uData6,             
                                CAD_UINT uData7,             
                                CAD_UINT uData8,             
                                CAD_UINT uData9,             
                                CAD_UINT uData10,            
                                CAD_UINT uData11,            
                                CAD_UINT uData12,            
                                CAD_UINT uData13,            
                                CAD_UINT uData14,            
                                CAD_UINT uData15,            
                                CAD_UINT uData16,            
                                CAD_UINT uData17,            
                                CAD_UINT uData18,            
                                CAD_UINT uData19,            
                                CAD_UINT uData20,            
                                CAD_UINT uData21,            
                                CAD_UINT uData22,            
                                CAD_UINT uData23);           
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Pb1DataClear( CAD_UINT uBaseAddress);      
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_Pb1DataFillPattern( CAD_UINT uBaseAddress, 
                                      CAD_UINT uData0,       
                                      CAD_UINT uData1,       
                                      CAD_UINT uData2,       
                                      CAD_UINT uData3,       
                                      CAD_UINT uData4,       
                                      CAD_UINT uData5,       
                                      CAD_UINT uData6,       
                                      CAD_UINT uData7,       
                                      CAD_UINT uData8,       
                                      CAD_UINT uData9,       
                                      CAD_UINT uData10,      
                                      CAD_UINT uData11,      
                                      CAD_UINT uData12,      
                                      CAD_UINT uData13,      
                                      CAD_UINT uData14,      
                                      CAD_UINT uData15,      
                                      CAD_UINT uData16,      
                                      CAD_UINT uData17,      
                                      CAD_UINT uData18,      
                                      CAD_UINT uData19,      
                                      CAD_UINT uData20,      
                                      CAD_UINT uData21,      
                                      CAD_UINT uData22,      
                                      CAD_UINT uData23);     
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtMirrorIndexRead( CAD_UINT uBaseAddress,  
                                     CAD_UINT uAddress,      
                                     CAD_UINT *pData);       
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtMirrorIndexWrite( CAD_UINT uBaseAddress, 
                                      CAD_UINT uAddress,     
                                      CAD_UINT uData);       
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtMirrorIndexClear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtMirrorIndexFillPattern( CAD_UINT uBaseAddress, 
                                            CAD_UINT uData); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPortToQueueAg0Read( CAD_UINT uBaseAddress, 
                                        CAD_UINT uAddress,   
                                        CAD_UINT *pData);    
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPortToQueueAg0Write( CAD_UINT uBaseAddress, 
                                         CAD_UINT uAddress,  
                                         CAD_UINT uData);    
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPortToQueueAg0Clear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPortToQueueAg0FillPattern( CAD_UINT uBaseAddress, 
                                               CAD_UINT uData); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPortToQueueAg1Read( CAD_UINT uBaseAddress, 
                                        CAD_UINT uAddress,   
                                        CAD_UINT *pData);    
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPortToQueueAg1Write( CAD_UINT uBaseAddress, 
                                         CAD_UINT uAddress,  
                                         CAD_UINT uData);    
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPortToQueueAg1Clear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPortToQueueAg1FillPattern( CAD_UINT uBaseAddress, 
                                               CAD_UINT uData); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPortToQueuePciRead( CAD_UINT uBaseAddress, 
                                        CAD_UINT uAddress,   
                                        CAD_UINT *pData);    
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPortToQueuePciWrite( CAD_UINT uBaseAddress, 
                                         CAD_UINT uAddress,  
                                         CAD_UINT uData);    
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPortToQueuePciClear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPortToQueuePciFillPattern( CAD_UINT uBaseAddress, 
                                               CAD_UINT uData); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPortToQueueSt0Read( CAD_UINT uBaseAddress, 
                                        CAD_UINT uAddress,   
                                        CAD_UINT *pData);    
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPortToQueueSt0Write( CAD_UINT uBaseAddress, 
                                         CAD_UINT uAddress,  
                                         CAD_UINT uData);    
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPortToQueueSt0Clear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPortToQueueSt0FillPattern( CAD_UINT uBaseAddress, 
                                               CAD_UINT uData); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPortToQueueSt1Read( CAD_UINT uBaseAddress, 
                                        CAD_UINT uAddress,   
                                        CAD_UINT *pData);    
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPortToQueueSt1Write( CAD_UINT uBaseAddress, 
                                         CAD_UINT uAddress,  
                                         CAD_UINT uData);    
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPortToQueueSt1Clear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPortToQueueSt1FillPattern( CAD_UINT uBaseAddress, 
                                               CAD_UINT uData); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPortToQueueXg0Read( CAD_UINT uBaseAddress, 
                                        CAD_UINT uAddress,   
                                        CAD_UINT *pData);    
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPortToQueueXg0Write( CAD_UINT uBaseAddress, 
                                         CAD_UINT uAddress,  
                                         CAD_UINT uData);    
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPortToQueueXg0Clear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPortToQueueXg0FillPattern( CAD_UINT uBaseAddress, 
                                               CAD_UINT uData); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPortToQueueXg1Read( CAD_UINT uBaseAddress, 
                                        CAD_UINT uAddress,   
                                        CAD_UINT *pData);    
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPortToQueueXg1Write( CAD_UINT uBaseAddress, 
                                         CAD_UINT uAddress,  
                                         CAD_UINT uData);    
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPortToQueueXg1Clear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPortToQueueXg1FillPattern( CAD_UINT uBaseAddress, 
                                               CAD_UINT uData); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte0MirrorPortStateRead( CAD_UINT uBaseAddress, 
                                             CAD_UINT uAddress, 
                                             CAD_UINT *pData0, 
                                             CAD_UINT *pData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte0MirrorPortStateWrite( CAD_UINT uBaseAddress, 
                                              CAD_UINT uAddress, 
                                              CAD_UINT uData0, 
                                              CAD_UINT uData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte0MirrorPortStateClear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte0MirrorPortStateFillPattern( CAD_UINT uBaseAddress, 
                                                    CAD_UINT uData0, 
                                                    CAD_UINT uData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte0PortStateRead( CAD_UINT uBaseAddress, 
                                       CAD_UINT uAddress,    
                                       CAD_UINT *pData0,     
                                       CAD_UINT *pData1,     
                                       CAD_UINT *pData2);    
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte0PortStateWrite( CAD_UINT uBaseAddress, 
                                        CAD_UINT uAddress,   
                                        CAD_UINT uData0,     
                                        CAD_UINT uData1,     
                                        CAD_UINT uData2);    
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte0PortStateClear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte0PortStateFillPattern( CAD_UINT uBaseAddress, 
                                              CAD_UINT uData0, 
                                              CAD_UINT uData1, 
                                              CAD_UINT uData2); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte1MirrorPortStateRead( CAD_UINT uBaseAddress, 
                                             CAD_UINT uAddress, 
                                             CAD_UINT *pData0, 
                                             CAD_UINT *pData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte1MirrorPortStateWrite( CAD_UINT uBaseAddress, 
                                              CAD_UINT uAddress, 
                                              CAD_UINT uData0, 
                                              CAD_UINT uData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte1MirrorPortStateClear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte1MirrorPortStateFillPattern( CAD_UINT uBaseAddress, 
                                                    CAD_UINT uData0, 
                                                    CAD_UINT uData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte1PortStateRead( CAD_UINT uBaseAddress, 
                                       CAD_UINT uAddress,    
                                       CAD_UINT *pData0,     
                                       CAD_UINT *pData1,     
                                       CAD_UINT *pData2);    
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte1PortStateWrite( CAD_UINT uBaseAddress, 
                                        CAD_UINT uAddress,   
                                        CAD_UINT uData0,     
                                        CAD_UINT uData1,     
                                        CAD_UINT uData2);    
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte1PortStateClear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte1PortStateFillPattern( CAD_UINT uBaseAddress, 
                                              CAD_UINT uData0, 
                                              CAD_UINT uData1, 
                                              CAD_UINT uData2); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte2MirrorPortStateRead( CAD_UINT uBaseAddress, 
                                             CAD_UINT uAddress, 
                                             CAD_UINT *pData0, 
                                             CAD_UINT *pData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte2MirrorPortStateWrite( CAD_UINT uBaseAddress, 
                                              CAD_UINT uAddress, 
                                              CAD_UINT uData0, 
                                              CAD_UINT uData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte2MirrorPortStateClear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte2MirrorPortStateFillPattern( CAD_UINT uBaseAddress, 
                                                    CAD_UINT uData0, 
                                                    CAD_UINT uData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte2PortStateRead( CAD_UINT uBaseAddress, 
                                       CAD_UINT uAddress,    
                                       CAD_UINT *pData0,     
                                       CAD_UINT *pData1,     
                                       CAD_UINT *pData2);    
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte2PortStateWrite( CAD_UINT uBaseAddress, 
                                        CAD_UINT uAddress,   
                                        CAD_UINT uData0,     
                                        CAD_UINT uData1,     
                                        CAD_UINT uData2);    
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte2PortStateClear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte2PortStateFillPattern( CAD_UINT uBaseAddress, 
                                              CAD_UINT uData0, 
                                              CAD_UINT uData1, 
                                              CAD_UINT uData2); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte3MirrorPortStateRead( CAD_UINT uBaseAddress, 
                                             CAD_UINT uAddress, 
                                             CAD_UINT *pData0, 
                                             CAD_UINT *pData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte3MirrorPortStateWrite( CAD_UINT uBaseAddress, 
                                              CAD_UINT uAddress, 
                                              CAD_UINT uData0, 
                                              CAD_UINT uData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte3MirrorPortStateClear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte3MirrorPortStateFillPattern( CAD_UINT uBaseAddress, 
                                                    CAD_UINT uData0, 
                                                    CAD_UINT uData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte3PortStateRead( CAD_UINT uBaseAddress, 
                                       CAD_UINT uAddress,    
                                       CAD_UINT *pData0,     
                                       CAD_UINT *pData1,     
                                       CAD_UINT *pData2);    
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte3PortStateWrite( CAD_UINT uBaseAddress, 
                                        CAD_UINT uAddress,   
                                        CAD_UINT uData0,     
                                        CAD_UINT uData1,     
                                        CAD_UINT uData2);    
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte3PortStateClear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte3PortStateFillPattern( CAD_UINT uBaseAddress, 
                                              CAD_UINT uData0, 
                                              CAD_UINT uData1, 
                                              CAD_UINT uData2); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte4MirrorPortStateRead( CAD_UINT uBaseAddress, 
                                             CAD_UINT uAddress, 
                                             CAD_UINT *pData0, 
                                             CAD_UINT *pData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte4MirrorPortStateWrite( CAD_UINT uBaseAddress, 
                                              CAD_UINT uAddress, 
                                              CAD_UINT uData0, 
                                              CAD_UINT uData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte4MirrorPortStateClear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte4MirrorPortStateFillPattern( CAD_UINT uBaseAddress, 
                                                    CAD_UINT uData0, 
                                                    CAD_UINT uData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte4PortStateRead( CAD_UINT uBaseAddress, 
                                       CAD_UINT uAddress,    
                                       CAD_UINT *pData0,     
                                       CAD_UINT *pData1,     
                                       CAD_UINT *pData2);    
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte4PortStateWrite( CAD_UINT uBaseAddress, 
                                        CAD_UINT uAddress,   
                                        CAD_UINT uData0,     
                                        CAD_UINT uData1,     
                                        CAD_UINT uData2);    
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte4PortStateClear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte4PortStateFillPattern( CAD_UINT uBaseAddress, 
                                              CAD_UINT uData0, 
                                              CAD_UINT uData1, 
                                              CAD_UINT uData2); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte5MirrorPortStateRead( CAD_UINT uBaseAddress, 
                                             CAD_UINT uAddress, 
                                             CAD_UINT *pData0, 
                                             CAD_UINT *pData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte5MirrorPortStateWrite( CAD_UINT uBaseAddress, 
                                              CAD_UINT uAddress, 
                                              CAD_UINT uData0, 
                                              CAD_UINT uData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte5MirrorPortStateClear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte5MirrorPortStateFillPattern( CAD_UINT uBaseAddress, 
                                                    CAD_UINT uData0, 
                                                    CAD_UINT uData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte5PortStateRead( CAD_UINT uBaseAddress, 
                                       CAD_UINT uAddress,    
                                       CAD_UINT *pData0,     
                                       CAD_UINT *pData1,     
                                       CAD_UINT *pData2);    
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte5PortStateWrite( CAD_UINT uBaseAddress, 
                                        CAD_UINT uAddress,   
                                        CAD_UINT uData0,     
                                        CAD_UINT uData1,     
                                        CAD_UINT uData2);    
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte5PortStateClear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte5PortStateFillPattern( CAD_UINT uBaseAddress, 
                                              CAD_UINT uData0, 
                                              CAD_UINT uData1, 
                                              CAD_UINT uData2); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte6MirrorPortStateRead( CAD_UINT uBaseAddress, 
                                             CAD_UINT uAddress, 
                                             CAD_UINT *pData0, 
                                             CAD_UINT *pData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte6MirrorPortStateWrite( CAD_UINT uBaseAddress, 
                                              CAD_UINT uAddress, 
                                              CAD_UINT uData0, 
                                              CAD_UINT uData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte6MirrorPortStateClear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte6MirrorPortStateFillPattern( CAD_UINT uBaseAddress, 
                                                    CAD_UINT uData0, 
                                                    CAD_UINT uData1); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte6PortStateRead( CAD_UINT uBaseAddress, 
                                       CAD_UINT uAddress,    
                                       CAD_UINT *pData0,     
                                       CAD_UINT *pData1,     
                                       CAD_UINT *pData2);    
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte6PortStateWrite( CAD_UINT uBaseAddress, 
                                        CAD_UINT uAddress,   
                                        CAD_UINT uData0,     
                                        CAD_UINT uData1,     
                                        CAD_UINT uData2);    
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte6PortStateClear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtPte6PortStateFillPattern( CAD_UINT uBaseAddress, 
                                              CAD_UINT uData0, 
                                              CAD_UINT uData1, 
                                              CAD_UINT uData2); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtQueueToPortRead( CAD_UINT uBaseAddress,  
                                     CAD_UINT uAddress,      
                                     CAD_UINT *pData);       
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtQueueToPortWrite( CAD_UINT uBaseAddress, 
                                      CAD_UINT uAddress,     
                                      CAD_UINT uData);       
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtQueueToPortClear( CAD_UINT uBaseAddress); 
sbFe2000ImfDriverStatus_t                                    
sbFe2000ImfDriver_PtQueueToPortFillPattern( CAD_UINT uBaseAddress, 
                                            CAD_UINT uData); 


#endif
