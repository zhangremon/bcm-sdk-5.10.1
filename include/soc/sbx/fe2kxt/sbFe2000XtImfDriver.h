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
 * $Id: sbFe2000XtImfDriver.h 1.3.218.1 Broadcom SDK $
 *
 ******************************************************************************/
#ifndef SB_FE2000XTIMFDriver_H
#define SB_FE2000XTIMFDriver_H

#include <soc/sbx/fe2kxt/sbFe2000Xt.h>

#define CAD_INT INT /* int32_t -> this grows with X64*/      /* Auto-Generated please see imf file*/
#define CAD_UINT UINT /* CAD_UINT -> this actually grows with X64*/ /* Auto-Generated please see imf file*/
#define CAD_ULLONG ULLONG /* uint64_t -> always 64 bits*/    /* Auto-Generated please see imf file*/
#define CAD_UINT32 UINT32 /* used for printing*/             /* Auto-Generated please see imf file*/
#define BOOL unsigned char
typedef enum                                                 /* Auto-Generated please see imf file*/
{                                                            /* Auto-Generated please see imf file*/
SAND_DRV_C2_STATUS_OK=0,                                     /* Auto-Generated please see imf file*/
SAND_DRV_C2_STATUS_ERROR=1,                                  /* Auto-Generated please see imf file*/
} sbFe2000XtImfDriverStatus_t;                                  /* Auto-Generated please see imf file*/
                                                             /* Auto-Generated please see imf file*/
                                                             /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrCc2CamRamRead( CAD_UINT uBaseAddress,     /* Auto-Generated please see imf file*/
                                  CAD_UINT uAddress,         /* Auto-Generated please see imf file*/
                                  CAD_UINT *pData0,          /* Auto-Generated please see imf file*/
                                  CAD_UINT *pData1,          /* Auto-Generated please see imf file*/
                                  CAD_UINT *pData2);         /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrCc2CamRamWrite( CAD_UINT uBaseAddress,    /* Auto-Generated please see imf file*/
                                   CAD_UINT uAddress,        /* Auto-Generated please see imf file*/
                                   CAD_UINT uData0,          /* Auto-Generated please see imf file*/
                                   CAD_UINT uData1,          /* Auto-Generated please see imf file*/
                                   CAD_UINT uData2);         /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrCc2CamRamClear( CAD_UINT uBaseAddress);   /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrCc2CamRamFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                         CAD_UINT uData0,    /* Auto-Generated please see imf file*/
                                         CAD_UINT uData1,    /* Auto-Generated please see imf file*/
                                         CAD_UINT uData2);   /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrCc2QueueRamRead( CAD_UINT uBaseAddress,   /* Auto-Generated please see imf file*/
                                    CAD_UINT uAddress,       /* Auto-Generated please see imf file*/
                                    CAD_UINT *pData);        /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrCc2QueueRamWrite( CAD_UINT uBaseAddress,  /* Auto-Generated please see imf file*/
                                     CAD_UINT uAddress,      /* Auto-Generated please see imf file*/
                                     CAD_UINT uData);        /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrCc2QueueRamClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrCc2QueueRamFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                           CAD_UINT uData);  /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrCc2TcamConfigRead( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                      CAD_UINT uAddress,     /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData0,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData1,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData2,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData3,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData4,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData5,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData6,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData7,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData8,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData9,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData10,     /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData11,     /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData12,     /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData13);    /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrCc2TcamConfigWrite( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                       CAD_UINT uAddress,    /* Auto-Generated please see imf file*/
                                       CAD_UINT uData0,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData1,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData2,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData3,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData4,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData5,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData6,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData7,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData8,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData9,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData10,     /* Auto-Generated please see imf file*/
                                       CAD_UINT uData11,     /* Auto-Generated please see imf file*/
                                       CAD_UINT uData12,     /* Auto-Generated please see imf file*/
                                       CAD_UINT uData13);    /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrCc2TcamConfigClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrCc2TcamConfigFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData1, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData2, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData3, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData4, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData5, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData6, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData7, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData8, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData9, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData10, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData11, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData12, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData13); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrCc3CamRamRead( CAD_UINT uBaseAddress,     /* Auto-Generated please see imf file*/
                                  CAD_UINT uAddress,         /* Auto-Generated please see imf file*/
                                  CAD_UINT *pData0,          /* Auto-Generated please see imf file*/
                                  CAD_UINT *pData1,          /* Auto-Generated please see imf file*/
                                  CAD_UINT *pData2);         /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrCc3CamRamWrite( CAD_UINT uBaseAddress,    /* Auto-Generated please see imf file*/
                                   CAD_UINT uAddress,        /* Auto-Generated please see imf file*/
                                   CAD_UINT uData0,          /* Auto-Generated please see imf file*/
                                   CAD_UINT uData1,          /* Auto-Generated please see imf file*/
                                   CAD_UINT uData2);         /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrCc3CamRamClear( CAD_UINT uBaseAddress);   /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrCc3CamRamFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                         CAD_UINT uData0,    /* Auto-Generated please see imf file*/
                                         CAD_UINT uData1,    /* Auto-Generated please see imf file*/
                                         CAD_UINT uData2);   /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrCc3QueueRamRead( CAD_UINT uBaseAddress,   /* Auto-Generated please see imf file*/
                                    CAD_UINT uAddress,       /* Auto-Generated please see imf file*/
                                    CAD_UINT *pData);        /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrCc3QueueRamWrite( CAD_UINT uBaseAddress,  /* Auto-Generated please see imf file*/
                                     CAD_UINT uAddress,      /* Auto-Generated please see imf file*/
                                     CAD_UINT uData);        /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrCc3QueueRamClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrCc3QueueRamFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                           CAD_UINT uData);  /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrCc3TcamConfigRead( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                      CAD_UINT uAddress,     /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData0,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData1,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData2,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData3,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData4,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData5,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData6,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData7,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData8,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData9,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData10,     /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData11,     /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData12,     /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData13);    /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrCc3TcamConfigWrite( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                       CAD_UINT uAddress,    /* Auto-Generated please see imf file*/
                                       CAD_UINT uData0,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData1,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData2,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData3,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData4,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData5,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData6,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData7,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData8,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData9,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData10,     /* Auto-Generated please see imf file*/
                                       CAD_UINT uData11,     /* Auto-Generated please see imf file*/
                                       CAD_UINT uData12,     /* Auto-Generated please see imf file*/
                                       CAD_UINT uData13);    /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrCc3TcamConfigClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrCc3TcamConfigFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData1, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData2, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData3, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData4, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData5, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData6, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData7, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData8, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData9, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData10, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData11, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData12, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData13); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrCc4CamRamRead( CAD_UINT uBaseAddress,     /* Auto-Generated please see imf file*/
                                  CAD_UINT uAddress,         /* Auto-Generated please see imf file*/
                                  CAD_UINT *pData0,          /* Auto-Generated please see imf file*/
                                  CAD_UINT *pData1,          /* Auto-Generated please see imf file*/
                                  CAD_UINT *pData2);         /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrCc4CamRamWrite( CAD_UINT uBaseAddress,    /* Auto-Generated please see imf file*/
                                   CAD_UINT uAddress,        /* Auto-Generated please see imf file*/
                                   CAD_UINT uData0,          /* Auto-Generated please see imf file*/
                                   CAD_UINT uData1,          /* Auto-Generated please see imf file*/
                                   CAD_UINT uData2);         /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrCc4CamRamClear( CAD_UINT uBaseAddress);   /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrCc4CamRamFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                         CAD_UINT uData0,    /* Auto-Generated please see imf file*/
                                         CAD_UINT uData1,    /* Auto-Generated please see imf file*/
                                         CAD_UINT uData2);   /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrCc4QueueRamRead( CAD_UINT uBaseAddress,   /* Auto-Generated please see imf file*/
                                    CAD_UINT uAddress,       /* Auto-Generated please see imf file*/
                                    CAD_UINT *pData);        /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrCc4QueueRamWrite( CAD_UINT uBaseAddress,  /* Auto-Generated please see imf file*/
                                     CAD_UINT uAddress,      /* Auto-Generated please see imf file*/
                                     CAD_UINT uData);        /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrCc4QueueRamClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrCc4QueueRamFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                           CAD_UINT uData);  /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrCc4TcamConfigRead( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                      CAD_UINT uAddress,     /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData0,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData1,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData2,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData3,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData4,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData5,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData6,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData7,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData8,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData9,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData10,     /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData11,     /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData12,     /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData13);    /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrCc4TcamConfigWrite( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                       CAD_UINT uAddress,    /* Auto-Generated please see imf file*/
                                       CAD_UINT uData0,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData1,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData2,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData3,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData4,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData5,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData6,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData7,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData8,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData9,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData10,     /* Auto-Generated please see imf file*/
                                       CAD_UINT uData11,     /* Auto-Generated please see imf file*/
                                       CAD_UINT uData12,     /* Auto-Generated please see imf file*/
                                       CAD_UINT uData13);    /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrCc4TcamConfigClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrCc4TcamConfigFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData1, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData2, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData3, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData4, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData5, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData6, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData7, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData8, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData9, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData10, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData11, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData12, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData13); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrCc5CamRamRead( CAD_UINT uBaseAddress,     /* Auto-Generated please see imf file*/
                                  CAD_UINT uAddress,         /* Auto-Generated please see imf file*/
                                  CAD_UINT *pData0,          /* Auto-Generated please see imf file*/
                                  CAD_UINT *pData1,          /* Auto-Generated please see imf file*/
                                  CAD_UINT *pData2);         /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrCc5CamRamWrite( CAD_UINT uBaseAddress,    /* Auto-Generated please see imf file*/
                                   CAD_UINT uAddress,        /* Auto-Generated please see imf file*/
                                   CAD_UINT uData0,          /* Auto-Generated please see imf file*/
                                   CAD_UINT uData1,          /* Auto-Generated please see imf file*/
                                   CAD_UINT uData2);         /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrCc5CamRamClear( CAD_UINT uBaseAddress);   /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrCc5CamRamFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                         CAD_UINT uData0,    /* Auto-Generated please see imf file*/
                                         CAD_UINT uData1,    /* Auto-Generated please see imf file*/
                                         CAD_UINT uData2);   /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrCc5QueueRamRead( CAD_UINT uBaseAddress,   /* Auto-Generated please see imf file*/
                                    CAD_UINT uAddress,       /* Auto-Generated please see imf file*/
                                    CAD_UINT *pData);        /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrCc5QueueRamWrite( CAD_UINT uBaseAddress,  /* Auto-Generated please see imf file*/
                                     CAD_UINT uAddress,      /* Auto-Generated please see imf file*/
                                     CAD_UINT uData);        /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrCc5QueueRamClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrCc5QueueRamFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                           CAD_UINT uData);  /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrCc5TcamConfigRead( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                      CAD_UINT uAddress,     /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData0,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData1,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData2,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData3,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData4,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData5,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData6,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData7,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData8,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData9,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData10,     /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData11,     /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData12,     /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData13);    /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrCc5TcamConfigWrite( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                       CAD_UINT uAddress,    /* Auto-Generated please see imf file*/
                                       CAD_UINT uData0,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData1,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData2,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData3,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData4,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData5,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData6,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData7,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData8,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData9,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData10,     /* Auto-Generated please see imf file*/
                                       CAD_UINT uData11,     /* Auto-Generated please see imf file*/
                                       CAD_UINT uData12,     /* Auto-Generated please see imf file*/
                                       CAD_UINT uData13);    /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrCc5TcamConfigClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrCc5TcamConfigFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData1, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData2, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData3, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData4, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData5, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData6, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData7, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData8, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData9, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData10, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData11, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData12, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData13); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrPortToQueueAg0Read( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                       CAD_UINT uAddress,    /* Auto-Generated please see imf file*/
                                       CAD_UINT *pData);     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrPortToQueueAg0Write( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                        CAD_UINT uAddress,   /* Auto-Generated please see imf file*/
                                        CAD_UINT uData);     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrPortToQueueAg0Clear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrPortToQueueAg0FillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                              CAD_UINT uData); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrPortToQueueAg1Read( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                       CAD_UINT uAddress,    /* Auto-Generated please see imf file*/
                                       CAD_UINT *pData);     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrPortToQueueAg1Write( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                        CAD_UINT uAddress,   /* Auto-Generated please see imf file*/
                                        CAD_UINT uData);     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrPortToQueueAg1Clear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrPortToQueueAg1FillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                              CAD_UINT uData); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrPortToQueuePciRead( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                       CAD_UINT uAddress,    /* Auto-Generated please see imf file*/
                                       CAD_UINT *pData);     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrPortToQueuePciWrite( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                        CAD_UINT uAddress,   /* Auto-Generated please see imf file*/
                                        CAD_UINT uData);     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrPortToQueuePciClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrPortToQueuePciFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                              CAD_UINT uData); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrPortToQueueSr0Read( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                       CAD_UINT uAddress,    /* Auto-Generated please see imf file*/
                                       CAD_UINT *pData);     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrPortToQueueSr0Write( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                        CAD_UINT uAddress,   /* Auto-Generated please see imf file*/
                                        CAD_UINT uData);     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrPortToQueueSr0Clear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrPortToQueueSr0FillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                              CAD_UINT uData); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrPortToQueueSr1Read( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                       CAD_UINT uAddress,    /* Auto-Generated please see imf file*/
                                       CAD_UINT *pData);     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrPortToQueueSr1Write( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                        CAD_UINT uAddress,   /* Auto-Generated please see imf file*/
                                        CAD_UINT uData);     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrPortToQueueSr1Clear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrPortToQueueSr1FillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                              CAD_UINT uData); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrPortToQueueXg0Read( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                       CAD_UINT uAddress,    /* Auto-Generated please see imf file*/
                                       CAD_UINT *pData);     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrPortToQueueXg0Write( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                        CAD_UINT uAddress,   /* Auto-Generated please see imf file*/
                                        CAD_UINT uData);     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrPortToQueueXg0Clear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrPortToQueueXg0FillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                              CAD_UINT uData); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrPortToQueueXg1Read( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                       CAD_UINT uAddress,    /* Auto-Generated please see imf file*/
                                       CAD_UINT *pData);     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrPortToQueueXg1Write( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                        CAD_UINT uAddress,   /* Auto-Generated please see imf file*/
                                        CAD_UINT uData);     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrPortToQueueXg1Clear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PrPortToQueueXg1FillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                              CAD_UINT uData); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PdLrpOutputHeaderCopyBufferRead( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                  CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData0, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData1, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData2, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData3, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData4, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData5, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData6, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData7); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PdLrpOutputHeaderCopyBufferWrite( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData1, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData2, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData3, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData4, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData5, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData6, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData7); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PdLrpOutputHeaderCopyBufferClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PdLrpOutputHeaderCopyBufferFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData1, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData2, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData3, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData4, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData5, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData6, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData7); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PdPdOutputHeaderCopyBufferRead( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                 CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                                 CAD_UINT *pData0, /* Auto-Generated please see imf file*/
                                                 CAD_UINT *pData1, /* Auto-Generated please see imf file*/
                                                 CAD_UINT *pData2, /* Auto-Generated please see imf file*/
                                                 CAD_UINT *pData3, /* Auto-Generated please see imf file*/
                                                 CAD_UINT *pData4, /* Auto-Generated please see imf file*/
                                                 CAD_UINT *pData5, /* Auto-Generated please see imf file*/
                                                 CAD_UINT *pData6, /* Auto-Generated please see imf file*/
                                                 CAD_UINT *pData7); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PdPdOutputHeaderCopyBufferWrite( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                  CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                                  CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                                  CAD_UINT uData1, /* Auto-Generated please see imf file*/
                                                  CAD_UINT uData2, /* Auto-Generated please see imf file*/
                                                  CAD_UINT uData3, /* Auto-Generated please see imf file*/
                                                  CAD_UINT uData4, /* Auto-Generated please see imf file*/
                                                  CAD_UINT uData5, /* Auto-Generated please see imf file*/
                                                  CAD_UINT uData6, /* Auto-Generated please see imf file*/
                                                  CAD_UINT uData7); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PdPdOutputHeaderCopyBufferClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PdPdOutputHeaderCopyBufferFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                        CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                                        CAD_UINT uData1, /* Auto-Generated please see imf file*/
                                                        CAD_UINT uData2, /* Auto-Generated please see imf file*/
                                                        CAD_UINT uData3, /* Auto-Generated please see imf file*/
                                                        CAD_UINT uData4, /* Auto-Generated please see imf file*/
                                                        CAD_UINT uData5, /* Auto-Generated please see imf file*/
                                                        CAD_UINT uData6, /* Auto-Generated please see imf file*/
                                                        CAD_UINT uData7); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_LrLrpInstructionMemoryRead( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                             CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                             CAD_UINT *pData0, /* Auto-Generated please see imf file*/
                                             CAD_UINT *pData1, /* Auto-Generated please see imf file*/
                                             CAD_UINT *pData2); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_LrLrpInstructionMemoryWrite( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                              CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                              CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                              CAD_UINT uData1, /* Auto-Generated please see imf file*/
                                              CAD_UINT uData2); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_LrLrpInstructionMemoryClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_LrLrpInstructionMemoryFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                    CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                                    CAD_UINT uData1, /* Auto-Generated please see imf file*/
                                                    CAD_UINT uData2); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_LrLrpInstructionMemoryBank0Read( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                  CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData0, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData1, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData2); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_LrLrpInstructionMemoryBank0Write( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData1, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData2); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_LrLrpInstructionMemoryBank0Clear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_LrLrpInstructionMemoryBank0FillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData1, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData2); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_LrLrpInstructionMemoryBank1Read( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                  CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData0, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData1, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData2); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_LrLrpInstructionMemoryBank1Write( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData1, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData2); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_LrLrpInstructionMemoryBank1Clear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_LrLrpInstructionMemoryBank1FillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData1, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData2); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmBufferState0Read( CAD_UINT uBaseAddress,  /* Auto-Generated please see imf file*/
                                     CAD_UINT uAddress,      /* Auto-Generated please see imf file*/
                                     CAD_UINT *pData);       /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmBufferState0Write( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                      CAD_UINT uAddress,     /* Auto-Generated please see imf file*/
                                      CAD_UINT uData);       /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmBufferState0Clear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmBufferState0FillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                            CAD_UINT uData); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmBufferState1Read( CAD_UINT uBaseAddress,  /* Auto-Generated please see imf file*/
                                     CAD_UINT uAddress,      /* Auto-Generated please see imf file*/
                                     CAD_UINT *pData0,       /* Auto-Generated please see imf file*/
                                     CAD_UINT *pData1);      /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmBufferState1Write( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                      CAD_UINT uAddress,     /* Auto-Generated please see imf file*/
                                      CAD_UINT uData0,       /* Auto-Generated please see imf file*/
                                      CAD_UINT uData1);      /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmBufferState1Clear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmBufferState1FillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                            CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                            CAD_UINT uData1); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmBufferState2Read( CAD_UINT uBaseAddress,  /* Auto-Generated please see imf file*/
                                     CAD_UINT uAddress,      /* Auto-Generated please see imf file*/
                                     CAD_UINT *pData);       /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmBufferState2Write( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                      CAD_UINT uAddress,     /* Auto-Generated please see imf file*/
                                      CAD_UINT uData);       /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmBufferState2Clear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmBufferState2FillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                            CAD_UINT uData); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmBufferState3Read( CAD_UINT uBaseAddress,  /* Auto-Generated please see imf file*/
                                     CAD_UINT uAddress,      /* Auto-Generated please see imf file*/
                                     CAD_UINT *pData);       /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmBufferState3Write( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                      CAD_UINT uAddress,     /* Auto-Generated please see imf file*/
                                      CAD_UINT uData);       /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmBufferState3Clear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmBufferState3FillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                            CAD_UINT uData); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmDequeueState0Read( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                      CAD_UINT uAddress,     /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData0,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData1);     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmDequeueState0Write( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                       CAD_UINT uAddress,    /* Auto-Generated please see imf file*/
                                       CAD_UINT uData0,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData1);     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmDequeueState0Clear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmDequeueState0FillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData1); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmDequeueState1Read( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                      CAD_UINT uAddress,     /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData);      /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmDequeueState1Write( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                       CAD_UINT uAddress,    /* Auto-Generated please see imf file*/
                                       CAD_UINT uData);      /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmDequeueState1Clear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmDequeueState1FillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmFreePageFifoRead( CAD_UINT uBaseAddress,  /* Auto-Generated please see imf file*/
                                     CAD_UINT uAddress,      /* Auto-Generated please see imf file*/
                                     CAD_UINT *pData);       /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmFreePageFifoWrite( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                      CAD_UINT uAddress,     /* Auto-Generated please see imf file*/
                                      CAD_UINT uData);       /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmFreePageFifoClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmFreePageFifoFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                            CAD_UINT uData); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmNextBufferRead( CAD_UINT uBaseAddress,    /* Auto-Generated please see imf file*/
                                   CAD_UINT uAddress,        /* Auto-Generated please see imf file*/
                                   CAD_UINT *pData);         /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmNextBufferWrite( CAD_UINT uBaseAddress,   /* Auto-Generated please see imf file*/
                                    CAD_UINT uAddress,       /* Auto-Generated please see imf file*/
                                    CAD_UINT uData);         /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmNextBufferClear( CAD_UINT uBaseAddress);  /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmNextBufferFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                          CAD_UINT uData);   /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmNextPageRead( CAD_UINT uBaseAddress,      /* Auto-Generated please see imf file*/
                                 CAD_UINT uAddress,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData);           /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmNextPageWrite( CAD_UINT uBaseAddress,     /* Auto-Generated please see imf file*/
                                  CAD_UINT uAddress,         /* Auto-Generated please see imf file*/
                                  CAD_UINT uData);           /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmNextPageClear( CAD_UINT uBaseAddress);    /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmNextPageFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                        CAD_UINT uData);     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmQueueConfigRead( CAD_UINT uBaseAddress,   /* Auto-Generated please see imf file*/
                                    CAD_UINT uAddress,       /* Auto-Generated please see imf file*/
                                    CAD_UINT *pData0,        /* Auto-Generated please see imf file*/
                                    CAD_UINT *pData1,        /* Auto-Generated please see imf file*/
                                    CAD_UINT *pData2);       /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmQueueConfigWrite( CAD_UINT uBaseAddress,  /* Auto-Generated please see imf file*/
                                     CAD_UINT uAddress,      /* Auto-Generated please see imf file*/
                                     CAD_UINT uData0,        /* Auto-Generated please see imf file*/
                                     CAD_UINT uData1,        /* Auto-Generated please see imf file*/
                                     CAD_UINT uData2);       /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmQueueConfigClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmQueueConfigFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                           CAD_UINT uData0,  /* Auto-Generated please see imf file*/
                                           CAD_UINT uData1,  /* Auto-Generated please see imf file*/
                                           CAD_UINT uData2); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmQueueCountersRead( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                      CAD_UINT uAddress,     /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData0,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData1);     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmQueueCountersWrite( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                       CAD_UINT uAddress,    /* Auto-Generated please see imf file*/
                                       CAD_UINT uData0,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData1);     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmQueueCountersClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmQueueCountersFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData1); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmQueueHeadPtrRead( CAD_UINT uBaseAddress,  /* Auto-Generated please see imf file*/
                                     CAD_UINT uAddress,      /* Auto-Generated please see imf file*/
                                     CAD_UINT *pData);       /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmQueueHeadPtrWrite( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                      CAD_UINT uAddress,     /* Auto-Generated please see imf file*/
                                      CAD_UINT uData);       /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmQueueHeadPtrClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmQueueHeadPtrFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                            CAD_UINT uData); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmQueueState0Read( CAD_UINT uBaseAddress,   /* Auto-Generated please see imf file*/
                                    CAD_UINT uAddress,       /* Auto-Generated please see imf file*/
                                    CAD_UINT *pData);        /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmQueueState0Write( CAD_UINT uBaseAddress,  /* Auto-Generated please see imf file*/
                                     CAD_UINT uAddress,      /* Auto-Generated please see imf file*/
                                     CAD_UINT uData);        /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmQueueState0Clear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmQueueState0FillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                           CAD_UINT uData);  /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmQueueState0EnRead( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                      CAD_UINT uAddress,     /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData);      /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmQueueState0EnWrite( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                       CAD_UINT uAddress,    /* Auto-Generated please see imf file*/
                                       CAD_UINT uData);      /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmQueueState0EnClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmQueueState0EnFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmQueueState1Read( CAD_UINT uBaseAddress,   /* Auto-Generated please see imf file*/
                                    CAD_UINT uAddress,       /* Auto-Generated please see imf file*/
                                    CAD_UINT *pData);        /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmQueueState1Write( CAD_UINT uBaseAddress,  /* Auto-Generated please see imf file*/
                                     CAD_UINT uAddress,      /* Auto-Generated please see imf file*/
                                     CAD_UINT uData);        /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmQueueState1Clear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmQueueState1FillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                           CAD_UINT uData);  /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmReplicationStateRead( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                         CAD_UINT uAddress,  /* Auto-Generated please see imf file*/
                                         CAD_UINT *pData);   /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmReplicationStateWrite( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                          CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                          CAD_UINT uData);   /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmReplicationStateClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_QmReplicationStateFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                CAD_UINT uData); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PmCounterMemoryRead( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                      CAD_UINT uAddress,     /* Auto-Generated please see imf file*/
                                      CAD_UINT bClearOnRead, /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData0,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData1);     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PmCounterMemoryWrite( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                       CAD_UINT uAddress,    /* Auto-Generated please see imf file*/
                                       CAD_UINT uData0,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData1);     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PmCounterMemoryClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PmCounterMemoryFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData1); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PmProfileMemoryRead( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                      CAD_UINT uAddress,     /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData0,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData1,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData2);     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PmProfileMemoryWrite( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                       CAD_UINT uAddress,    /* Auto-Generated please see imf file*/
                                       CAD_UINT uData0,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData1,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData2);     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PmProfileMemoryClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PmProfileMemoryFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData1, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData2); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Sr0CounterRead( CAD_UINT uBaseAddress,      /* Auto-Generated please see imf file*/
                                 CAD_UINT uAddress,          /* Auto-Generated please see imf file*/
                                 CAD_UINT bClearOnRead,      /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData0,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData1);          /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Sr0CounterWrite( CAD_UINT uBaseAddress,     /* Auto-Generated please see imf file*/
                                  CAD_UINT uAddress,         /* Auto-Generated please see imf file*/
                                  CAD_UINT uData0,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData1);          /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Sr0CounterClear( CAD_UINT uBaseAddress);    /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Sr0CounterFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                        CAD_UINT uData0,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData1);    /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Sr1CounterRead( CAD_UINT uBaseAddress,      /* Auto-Generated please see imf file*/
                                 CAD_UINT uAddress,          /* Auto-Generated please see imf file*/
                                 CAD_UINT bClearOnRead,      /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData0,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData1);          /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Sr1CounterWrite( CAD_UINT uBaseAddress,     /* Auto-Generated please see imf file*/
                                  CAD_UINT uAddress,         /* Auto-Generated please see imf file*/
                                  CAD_UINT uData0,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData1);          /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Sr1CounterClear( CAD_UINT uBaseAddress);    /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Sr1CounterFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                        CAD_UINT uData0,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData1);    /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpAggregateHashBitConfigRead( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                               CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                               CAD_UINT *pData0, /* Auto-Generated please see imf file*/
                                               CAD_UINT *pData1, /* Auto-Generated please see imf file*/
                                               CAD_UINT *pData2, /* Auto-Generated please see imf file*/
                                               CAD_UINT *pData3, /* Auto-Generated please see imf file*/
                                               CAD_UINT *pData4, /* Auto-Generated please see imf file*/
                                               CAD_UINT *pData5, /* Auto-Generated please see imf file*/
                                               CAD_UINT *pData6, /* Auto-Generated please see imf file*/
                                               CAD_UINT *pData7, /* Auto-Generated please see imf file*/
                                               CAD_UINT *pData8, /* Auto-Generated please see imf file*/
                                               CAD_UINT *pData9); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpAggregateHashBitConfigWrite( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                                CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                                CAD_UINT uData1, /* Auto-Generated please see imf file*/
                                                CAD_UINT uData2, /* Auto-Generated please see imf file*/
                                                CAD_UINT uData3, /* Auto-Generated please see imf file*/
                                                CAD_UINT uData4, /* Auto-Generated please see imf file*/
                                                CAD_UINT uData5, /* Auto-Generated please see imf file*/
                                                CAD_UINT uData6, /* Auto-Generated please see imf file*/
                                                CAD_UINT uData7, /* Auto-Generated please see imf file*/
                                                CAD_UINT uData8, /* Auto-Generated please see imf file*/
                                                CAD_UINT uData9); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpAggregateHashBitConfigClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpAggregateHashBitConfigFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                      CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                                      CAD_UINT uData1, /* Auto-Generated please see imf file*/
                                                      CAD_UINT uData2, /* Auto-Generated please see imf file*/
                                                      CAD_UINT uData3, /* Auto-Generated please see imf file*/
                                                      CAD_UINT uData4, /* Auto-Generated please see imf file*/
                                                      CAD_UINT uData5, /* Auto-Generated please see imf file*/
                                                      CAD_UINT uData6, /* Auto-Generated please see imf file*/
                                                      CAD_UINT uData7, /* Auto-Generated please see imf file*/
                                                      CAD_UINT uData8, /* Auto-Generated please see imf file*/
                                                      CAD_UINT uData9); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpAggregateHashByteConfigRead( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                                CAD_UINT *pData0, /* Auto-Generated please see imf file*/
                                                CAD_UINT *pData1); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpAggregateHashByteConfigWrite( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                 CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                                 CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                                 CAD_UINT uData1); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpAggregateHashByteConfigClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpAggregateHashByteConfigFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                       CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                                       CAD_UINT uData1); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpCamConfigurationInstance0Read( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                  CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData0, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData1, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData2, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData3, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData4, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData5, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData6, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData7, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData8, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData9, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData10, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData11, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData12, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData13); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpCamConfigurationInstance0Write( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData1, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData2, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData3, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData4, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData5, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData6, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData7, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData8, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData9, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData10, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData11, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData12, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData13); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpCamConfigurationInstance0Clear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpCamConfigurationInstance0FillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData1, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData2, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData3, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData4, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData5, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData6, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData7, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData8, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData9, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData10, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData11, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData12, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData13); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpCamConfigurationInstance1Read( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                  CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData0, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData1, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData2, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData3, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData4, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData5, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData6, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData7, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData8, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData9, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData10, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData11, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData12, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData13); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpCamConfigurationInstance1Write( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData1, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData2, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData3, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData4, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData5, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData6, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData7, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData8, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData9, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData10, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData11, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData12, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData13); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpCamConfigurationInstance1Clear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpCamConfigurationInstance1FillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData1, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData2, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData3, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData4, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData5, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData6, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData7, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData8, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData9, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData10, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData11, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData12, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData13); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpCamConfigurationInstance2Read( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                  CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData0, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData1, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData2, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData3, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData4, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData5, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData6, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData7, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData8, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData9, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData10, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData11, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData12, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData13); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpCamConfigurationInstance2Write( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData1, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData2, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData3, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData4, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData5, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData6, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData7, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData8, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData9, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData10, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData11, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData12, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData13); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpCamConfigurationInstance2Clear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpCamConfigurationInstance2FillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData1, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData2, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData3, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData4, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData5, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData6, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData7, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData8, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData9, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData10, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData11, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData12, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData13); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpCamConfigurationInstance3Read( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                  CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData0, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData1, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData2, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData3, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData4, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData5, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData6, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData7, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData8, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData9, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData10, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData11, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData12, /* Auto-Generated please see imf file*/
                                                  CAD_UINT *pData13); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpCamConfigurationInstance3Write( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData1, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData2, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData3, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData4, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData5, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData6, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData7, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData8, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData9, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData10, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData11, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData12, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData13); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpCamConfigurationInstance3Clear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpCamConfigurationInstance3FillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData1, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData2, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData3, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData4, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData5, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData6, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData7, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData8, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData9, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData10, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData11, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData12, /* Auto-Generated please see imf file*/
                                                         CAD_UINT uData13); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpCamRamConfigurationInstance0Read( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                     CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                                     CAD_UINT *pData0, /* Auto-Generated please see imf file*/
                                                     CAD_UINT *pData1, /* Auto-Generated please see imf file*/
                                                     CAD_UINT *pData2, /* Auto-Generated please see imf file*/
                                                     CAD_UINT *pData3, /* Auto-Generated please see imf file*/
                                                     CAD_UINT *pData4, /* Auto-Generated please see imf file*/
                                                     CAD_UINT *pData5); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpCamRamConfigurationInstance0Write( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                      CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                                      CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                                      CAD_UINT uData1, /* Auto-Generated please see imf file*/
                                                      CAD_UINT uData2, /* Auto-Generated please see imf file*/
                                                      CAD_UINT uData3, /* Auto-Generated please see imf file*/
                                                      CAD_UINT uData4, /* Auto-Generated please see imf file*/
                                                      CAD_UINT uData5); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpCamRamConfigurationInstance0Clear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpCamRamConfigurationInstance0FillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                            CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                                            CAD_UINT uData1, /* Auto-Generated please see imf file*/
                                                            CAD_UINT uData2, /* Auto-Generated please see imf file*/
                                                            CAD_UINT uData3, /* Auto-Generated please see imf file*/
                                                            CAD_UINT uData4, /* Auto-Generated please see imf file*/
                                                            CAD_UINT uData5); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpCamRamConfigurationInstance1Read( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                     CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                                     CAD_UINT *pData0, /* Auto-Generated please see imf file*/
                                                     CAD_UINT *pData1, /* Auto-Generated please see imf file*/
                                                     CAD_UINT *pData2, /* Auto-Generated please see imf file*/
                                                     CAD_UINT *pData3, /* Auto-Generated please see imf file*/
                                                     CAD_UINT *pData4, /* Auto-Generated please see imf file*/
                                                     CAD_UINT *pData5); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpCamRamConfigurationInstance1Write( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                      CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                                      CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                                      CAD_UINT uData1, /* Auto-Generated please see imf file*/
                                                      CAD_UINT uData2, /* Auto-Generated please see imf file*/
                                                      CAD_UINT uData3, /* Auto-Generated please see imf file*/
                                                      CAD_UINT uData4, /* Auto-Generated please see imf file*/
                                                      CAD_UINT uData5); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpCamRamConfigurationInstance1Clear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpCamRamConfigurationInstance1FillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                            CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                                            CAD_UINT uData1, /* Auto-Generated please see imf file*/
                                                            CAD_UINT uData2, /* Auto-Generated please see imf file*/
                                                            CAD_UINT uData3, /* Auto-Generated please see imf file*/
                                                            CAD_UINT uData4, /* Auto-Generated please see imf file*/
                                                            CAD_UINT uData5); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpCamRamConfigurationInstance2Read( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                     CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                                     CAD_UINT *pData0, /* Auto-Generated please see imf file*/
                                                     CAD_UINT *pData1, /* Auto-Generated please see imf file*/
                                                     CAD_UINT *pData2, /* Auto-Generated please see imf file*/
                                                     CAD_UINT *pData3, /* Auto-Generated please see imf file*/
                                                     CAD_UINT *pData4, /* Auto-Generated please see imf file*/
                                                     CAD_UINT *pData5); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpCamRamConfigurationInstance2Write( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                      CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                                      CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                                      CAD_UINT uData1, /* Auto-Generated please see imf file*/
                                                      CAD_UINT uData2, /* Auto-Generated please see imf file*/
                                                      CAD_UINT uData3, /* Auto-Generated please see imf file*/
                                                      CAD_UINT uData4, /* Auto-Generated please see imf file*/
                                                      CAD_UINT uData5); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpCamRamConfigurationInstance2Clear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpCamRamConfigurationInstance2FillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                            CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                                            CAD_UINT uData1, /* Auto-Generated please see imf file*/
                                                            CAD_UINT uData2, /* Auto-Generated please see imf file*/
                                                            CAD_UINT uData3, /* Auto-Generated please see imf file*/
                                                            CAD_UINT uData4, /* Auto-Generated please see imf file*/
                                                            CAD_UINT uData5); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpCamRamConfigurationInstance3Read( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                     CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                                     CAD_UINT *pData0, /* Auto-Generated please see imf file*/
                                                     CAD_UINT *pData1, /* Auto-Generated please see imf file*/
                                                     CAD_UINT *pData2, /* Auto-Generated please see imf file*/
                                                     CAD_UINT *pData3, /* Auto-Generated please see imf file*/
                                                     CAD_UINT *pData4, /* Auto-Generated please see imf file*/
                                                     CAD_UINT *pData5); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpCamRamConfigurationInstance3Write( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                      CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                                      CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                                      CAD_UINT uData1, /* Auto-Generated please see imf file*/
                                                      CAD_UINT uData2, /* Auto-Generated please see imf file*/
                                                      CAD_UINT uData3, /* Auto-Generated please see imf file*/
                                                      CAD_UINT uData4, /* Auto-Generated please see imf file*/
                                                      CAD_UINT uData5); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpCamRamConfigurationInstance3Clear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpCamRamConfigurationInstance3FillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                            CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                                            CAD_UINT uData1, /* Auto-Generated please see imf file*/
                                                            CAD_UINT uData2, /* Auto-Generated please see imf file*/
                                                            CAD_UINT uData3, /* Auto-Generated please see imf file*/
                                                            CAD_UINT uData4, /* Auto-Generated please see imf file*/
                                                            CAD_UINT uData5); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpHeaderRecordSizeRead( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                         CAD_UINT uAddress,  /* Auto-Generated please see imf file*/
                                         CAD_UINT *pData);   /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpHeaderRecordSizeWrite( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                          CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                          CAD_UINT uData);   /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpHeaderRecordSizeClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpHeaderRecordSizeFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                CAD_UINT uData); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpInitialQueueStateRead( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                          CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                          CAD_UINT *pData0,  /* Auto-Generated please see imf file*/
                                          CAD_UINT *pData1,  /* Auto-Generated please see imf file*/
                                          CAD_UINT *pData2,  /* Auto-Generated please see imf file*/
                                          CAD_UINT *pData3,  /* Auto-Generated please see imf file*/
                                          CAD_UINT *pData4); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpInitialQueueStateWrite( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                           CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                           CAD_UINT uData0,  /* Auto-Generated please see imf file*/
                                           CAD_UINT uData1,  /* Auto-Generated please see imf file*/
                                           CAD_UINT uData2,  /* Auto-Generated please see imf file*/
                                           CAD_UINT uData3,  /* Auto-Generated please see imf file*/
                                           CAD_UINT uData4); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpInitialQueueStateClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpInitialQueueStateFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                 CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                                 CAD_UINT uData1, /* Auto-Generated please see imf file*/
                                                 CAD_UINT uData2, /* Auto-Generated please see imf file*/
                                                 CAD_UINT uData3, /* Auto-Generated please see imf file*/
                                                 CAD_UINT uData4); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpPpOutHeaderCopyRead( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                        CAD_UINT uAddress,   /* Auto-Generated please see imf file*/
                                        CAD_UINT *pData0,    /* Auto-Generated please see imf file*/
                                        CAD_UINT *pData1,    /* Auto-Generated please see imf file*/
                                        CAD_UINT *pData2,    /* Auto-Generated please see imf file*/
                                        CAD_UINT *pData3,    /* Auto-Generated please see imf file*/
                                        CAD_UINT *pData4,    /* Auto-Generated please see imf file*/
                                        CAD_UINT *pData5,    /* Auto-Generated please see imf file*/
                                        CAD_UINT *pData6,    /* Auto-Generated please see imf file*/
                                        CAD_UINT *pData7);   /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpPpOutHeaderCopyWrite( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                         CAD_UINT uAddress,  /* Auto-Generated please see imf file*/
                                         CAD_UINT uData0,    /* Auto-Generated please see imf file*/
                                         CAD_UINT uData1,    /* Auto-Generated please see imf file*/
                                         CAD_UINT uData2,    /* Auto-Generated please see imf file*/
                                         CAD_UINT uData3,    /* Auto-Generated please see imf file*/
                                         CAD_UINT uData4,    /* Auto-Generated please see imf file*/
                                         CAD_UINT uData5,    /* Auto-Generated please see imf file*/
                                         CAD_UINT uData6,    /* Auto-Generated please see imf file*/
                                         CAD_UINT uData7);   /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpPpOutHeaderCopyClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpPpOutHeaderCopyFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                               CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                               CAD_UINT uData1, /* Auto-Generated please see imf file*/
                                               CAD_UINT uData2, /* Auto-Generated please see imf file*/
                                               CAD_UINT uData3, /* Auto-Generated please see imf file*/
                                               CAD_UINT uData4, /* Auto-Generated please see imf file*/
                                               CAD_UINT uData5, /* Auto-Generated please see imf file*/
                                               CAD_UINT uData6, /* Auto-Generated please see imf file*/
                                               CAD_UINT uData7); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpQueuePriorityGroupRead( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                           CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                           CAD_UINT *pData); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpQueuePriorityGroupWrite( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                            CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                            CAD_UINT uData); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpQueuePriorityGroupClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpQueuePriorityGroupFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                  CAD_UINT uData); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpRxPortDataRead( CAD_UINT uBaseAddress,    /* Auto-Generated please see imf file*/
                                   CAD_UINT uAddress,        /* Auto-Generated please see imf file*/
                                   CAD_UINT *pData0,         /* Auto-Generated please see imf file*/
                                   CAD_UINT *pData1,         /* Auto-Generated please see imf file*/
                                   CAD_UINT *pData2,         /* Auto-Generated please see imf file*/
                                   CAD_UINT *pData3);        /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpRxPortDataWrite( CAD_UINT uBaseAddress,   /* Auto-Generated please see imf file*/
                                    CAD_UINT uAddress,       /* Auto-Generated please see imf file*/
                                    CAD_UINT uData0,         /* Auto-Generated please see imf file*/
                                    CAD_UINT uData1,         /* Auto-Generated please see imf file*/
                                    CAD_UINT uData2,         /* Auto-Generated please see imf file*/
                                    CAD_UINT uData3);        /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpRxPortDataClear( CAD_UINT uBaseAddress);  /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PpRxPortDataFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                          CAD_UINT uData0,   /* Auto-Generated please see imf file*/
                                          CAD_UINT uData1,   /* Auto-Generated please see imf file*/
                                          CAD_UINT uData2,   /* Auto-Generated please see imf file*/
                                          CAD_UINT uData3);  /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc0DataRead( CAD_UINT uBaseAddress,         /* Auto-Generated please see imf file*/
                              CAD_UINT uAddress,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData0,              /* Auto-Generated please see imf file*/
                              CAD_UINT *pData1,              /* Auto-Generated please see imf file*/
                              CAD_UINT *pData2,              /* Auto-Generated please see imf file*/
                              CAD_UINT *pData3,              /* Auto-Generated please see imf file*/
                              CAD_UINT *pData4,              /* Auto-Generated please see imf file*/
                              CAD_UINT *pData5,              /* Auto-Generated please see imf file*/
                              CAD_UINT *pData6,              /* Auto-Generated please see imf file*/
                              CAD_UINT *pData7,              /* Auto-Generated please see imf file*/
                              CAD_UINT *pData8,              /* Auto-Generated please see imf file*/
                              CAD_UINT *pData9,              /* Auto-Generated please see imf file*/
                              CAD_UINT *pData10,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData11,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData12,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData13,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData14,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData15,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData16,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData17,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData18,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData19,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData20,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData21,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData22,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData23,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData24,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData25,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData26,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData27,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData28,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData29,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData30,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData31,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData32,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData33,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData34,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData35,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData36,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData37,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData38,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData39);            /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc0DataWrite( CAD_UINT uBaseAddress,        /* Auto-Generated please see imf file*/
                               CAD_UINT uAddress,            /* Auto-Generated please see imf file*/
                               CAD_UINT uData0,              /* Auto-Generated please see imf file*/
                               CAD_UINT uData1,              /* Auto-Generated please see imf file*/
                               CAD_UINT uData2,              /* Auto-Generated please see imf file*/
                               CAD_UINT uData3,              /* Auto-Generated please see imf file*/
                               CAD_UINT uData4,              /* Auto-Generated please see imf file*/
                               CAD_UINT uData5,              /* Auto-Generated please see imf file*/
                               CAD_UINT uData6,              /* Auto-Generated please see imf file*/
                               CAD_UINT uData7,              /* Auto-Generated please see imf file*/
                               CAD_UINT uData8,              /* Auto-Generated please see imf file*/
                               CAD_UINT uData9,              /* Auto-Generated please see imf file*/
                               CAD_UINT uData10,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData11,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData12,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData13,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData14,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData15,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData16,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData17,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData18,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData19,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData20,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData21,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData22,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData23,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData24,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData25,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData26,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData27,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData28,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData29,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData30,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData31,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData32,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData33,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData34,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData35,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData36,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData37,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData38,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData39);            /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc0DataClear( CAD_UINT uBaseAddress);       /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc0DataFillPattern( CAD_UINT uBaseAddress,  /* Auto-Generated please see imf file*/
                                     CAD_UINT uData0,        /* Auto-Generated please see imf file*/
                                     CAD_UINT uData1,        /* Auto-Generated please see imf file*/
                                     CAD_UINT uData2,        /* Auto-Generated please see imf file*/
                                     CAD_UINT uData3,        /* Auto-Generated please see imf file*/
                                     CAD_UINT uData4,        /* Auto-Generated please see imf file*/
                                     CAD_UINT uData5,        /* Auto-Generated please see imf file*/
                                     CAD_UINT uData6,        /* Auto-Generated please see imf file*/
                                     CAD_UINT uData7,        /* Auto-Generated please see imf file*/
                                     CAD_UINT uData8,        /* Auto-Generated please see imf file*/
                                     CAD_UINT uData9,        /* Auto-Generated please see imf file*/
                                     CAD_UINT uData10,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData11,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData12,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData13,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData14,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData15,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData16,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData17,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData18,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData19,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData20,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData21,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData22,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData23,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData24,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData25,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData26,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData27,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData28,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData29,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData30,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData31,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData32,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData33,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData34,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData35,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData36,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData37,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData38,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData39);      /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc0DataSB0Read( CAD_UINT uBaseAddress,      /* Auto-Generated please see imf file*/
                                 CAD_UINT uAddress,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData0,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData1,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData2,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData3,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData4,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData5,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData6,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData7,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData8,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData9,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData10,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData11,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData12,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData13,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData14,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData15,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData16,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData17,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData18,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData19,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData20,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData21,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData22,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData23,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData24,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData25,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData26,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData27,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData28,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData29,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData30,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData31,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData32,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData33,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData34,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData35,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData36,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData37,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData38,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData39);         /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc0DataSB0Write( CAD_UINT uBaseAddress,     /* Auto-Generated please see imf file*/
                                  CAD_UINT uAddress,         /* Auto-Generated please see imf file*/
                                  CAD_UINT uData0,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData1,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData2,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData3,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData4,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData5,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData6,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData7,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData8,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData9,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData10,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData11,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData12,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData13,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData14,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData15,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData16,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData17,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData18,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData19,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData20,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData21,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData22,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData23,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData24,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData25,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData26,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData27,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData28,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData29,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData30,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData31,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData32,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData33,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData34,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData35,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData36,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData37,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData38,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData39);         /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc0DataSB0Clear( CAD_UINT uBaseAddress);    /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc0DataSB0FillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                        CAD_UINT uData0,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData1,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData2,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData3,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData4,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData5,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData6,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData7,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData8,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData9,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData10,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData11,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData12,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData13,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData14,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData15,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData16,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData17,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData18,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData19,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData20,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData21,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData22,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData23,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData24,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData25,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData26,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData27,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData28,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData29,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData30,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData31,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData32,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData33,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData34,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData35,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData36,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData37,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData38,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData39);   /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc0DataSB1Read( CAD_UINT uBaseAddress,      /* Auto-Generated please see imf file*/
                                 CAD_UINT uAddress,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData0,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData1,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData2,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData3,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData4,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData5,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData6,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData7,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData8,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData9,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData10,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData11,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData12,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData13,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData14,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData15,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData16,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData17,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData18,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData19,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData20,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData21,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData22,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData23,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData24,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData25,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData26,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData27,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData28,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData29,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData30,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData31,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData32,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData33,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData34,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData35,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData36,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData37,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData38,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData39);         /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc0DataSB1Write( CAD_UINT uBaseAddress,     /* Auto-Generated please see imf file*/
                                  CAD_UINT uAddress,         /* Auto-Generated please see imf file*/
                                  CAD_UINT uData0,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData1,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData2,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData3,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData4,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData5,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData6,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData7,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData8,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData9,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData10,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData11,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData12,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData13,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData14,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData15,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData16,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData17,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData18,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData19,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData20,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData21,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData22,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData23,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData24,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData25,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData26,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData27,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData28,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData29,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData30,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData31,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData32,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData33,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData34,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData35,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData36,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData37,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData38,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData39);         /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc0DataSB1Clear( CAD_UINT uBaseAddress);    /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc0DataSB1FillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                        CAD_UINT uData0,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData1,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData2,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData3,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData4,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData5,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData6,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData7,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData8,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData9,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData10,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData11,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData12,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData13,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData14,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData15,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData16,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData17,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData18,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData19,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData20,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData21,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData22,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData23,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData24,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData25,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData26,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData27,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData28,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData29,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData30,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData31,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData32,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData33,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData34,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData35,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData36,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData37,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData38,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData39);   /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc0DataSB2Read( CAD_UINT uBaseAddress,      /* Auto-Generated please see imf file*/
                                 CAD_UINT uAddress,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData0,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData1,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData2,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData3,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData4,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData5,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData6,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData7,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData8,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData9,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData10,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData11,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData12,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData13,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData14,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData15,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData16,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData17,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData18,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData19,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData20,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData21,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData22,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData23,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData24,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData25,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData26,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData27,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData28,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData29,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData30,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData31,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData32,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData33,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData34,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData35,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData36,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData37,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData38,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData39);         /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc0DataSB2Write( CAD_UINT uBaseAddress,     /* Auto-Generated please see imf file*/
                                  CAD_UINT uAddress,         /* Auto-Generated please see imf file*/
                                  CAD_UINT uData0,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData1,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData2,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData3,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData4,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData5,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData6,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData7,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData8,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData9,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData10,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData11,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData12,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData13,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData14,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData15,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData16,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData17,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData18,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData19,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData20,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData21,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData22,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData23,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData24,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData25,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData26,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData27,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData28,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData29,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData30,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData31,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData32,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData33,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData34,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData35,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData36,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData37,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData38,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData39);         /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc0DataSB2Clear( CAD_UINT uBaseAddress);    /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc0DataSB2FillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                        CAD_UINT uData0,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData1,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData2,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData3,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData4,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData5,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData6,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData7,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData8,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData9,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData10,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData11,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData12,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData13,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData14,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData15,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData16,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData17,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData18,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData19,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData20,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData21,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData22,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData23,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData24,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData25,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData26,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData27,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData28,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData29,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData30,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData31,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData32,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData33,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData34,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData35,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData36,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData37,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData38,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData39);   /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc0DataSB3Read( CAD_UINT uBaseAddress,      /* Auto-Generated please see imf file*/
                                 CAD_UINT uAddress,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData0,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData1,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData2,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData3,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData4,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData5,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData6,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData7,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData8,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData9,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData10,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData11,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData12,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData13,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData14,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData15,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData16,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData17,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData18,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData19,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData20,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData21,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData22,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData23,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData24,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData25,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData26,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData27,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData28,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData29,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData30,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData31,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData32,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData33,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData34,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData35,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData36,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData37,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData38,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData39);         /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc0DataSB3Write( CAD_UINT uBaseAddress,     /* Auto-Generated please see imf file*/
                                  CAD_UINT uAddress,         /* Auto-Generated please see imf file*/
                                  CAD_UINT uData0,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData1,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData2,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData3,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData4,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData5,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData6,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData7,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData8,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData9,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData10,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData11,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData12,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData13,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData14,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData15,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData16,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData17,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData18,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData19,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData20,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData21,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData22,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData23,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData24,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData25,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData26,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData27,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData28,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData29,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData30,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData31,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData32,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData33,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData34,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData35,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData36,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData37,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData38,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData39);         /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc0DataSB3Clear( CAD_UINT uBaseAddress);    /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc0DataSB3FillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                        CAD_UINT uData0,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData1,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData2,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData3,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData4,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData5,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData6,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData7,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData8,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData9,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData10,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData11,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData12,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData13,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData14,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData15,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData16,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData17,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData18,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData19,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData20,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData21,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData22,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData23,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData24,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData25,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData26,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData27,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData28,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData29,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData30,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData31,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData32,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData33,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData34,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData35,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData36,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData37,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData38,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData39);   /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc0DataSB4Read( CAD_UINT uBaseAddress,      /* Auto-Generated please see imf file*/
                                 CAD_UINT uAddress,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData0,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData1,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData2,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData3,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData4,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData5,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData6,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData7,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData8,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData9,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData10,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData11,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData12,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData13,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData14,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData15,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData16,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData17,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData18,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData19,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData20,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData21,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData22,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData23,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData24,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData25,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData26,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData27,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData28,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData29,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData30,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData31,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData32,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData33,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData34,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData35,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData36,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData37,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData38,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData39);         /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc0DataSB4Write( CAD_UINT uBaseAddress,     /* Auto-Generated please see imf file*/
                                  CAD_UINT uAddress,         /* Auto-Generated please see imf file*/
                                  CAD_UINT uData0,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData1,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData2,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData3,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData4,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData5,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData6,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData7,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData8,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData9,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData10,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData11,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData12,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData13,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData14,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData15,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData16,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData17,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData18,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData19,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData20,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData21,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData22,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData23,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData24,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData25,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData26,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData27,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData28,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData29,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData30,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData31,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData32,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData33,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData34,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData35,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData36,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData37,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData38,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData39);         /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc0DataSB4Clear( CAD_UINT uBaseAddress);    /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc0DataSB4FillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                        CAD_UINT uData0,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData1,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData2,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData3,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData4,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData5,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData6,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData7,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData8,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData9,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData10,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData11,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData12,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData13,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData14,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData15,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData16,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData17,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData18,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData19,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData20,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData21,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData22,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData23,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData24,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData25,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData26,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData27,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData28,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData29,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData30,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData31,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData32,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData33,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData34,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData35,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData36,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData37,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData38,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData39);   /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc0DataSB5Read( CAD_UINT uBaseAddress,      /* Auto-Generated please see imf file*/
                                 CAD_UINT uAddress,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData0,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData1,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData2,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData3,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData4,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData5,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData6,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData7,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData8,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData9,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData10,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData11,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData12,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData13,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData14,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData15,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData16,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData17,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData18,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData19,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData20,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData21,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData22,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData23,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData24,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData25,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData26,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData27,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData28,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData29,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData30,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData31,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData32,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData33,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData34,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData35,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData36,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData37,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData38,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData39);         /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc0DataSB5Write( CAD_UINT uBaseAddress,     /* Auto-Generated please see imf file*/
                                  CAD_UINT uAddress,         /* Auto-Generated please see imf file*/
                                  CAD_UINT uData0,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData1,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData2,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData3,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData4,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData5,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData6,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData7,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData8,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData9,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData10,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData11,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData12,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData13,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData14,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData15,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData16,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData17,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData18,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData19,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData20,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData21,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData22,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData23,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData24,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData25,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData26,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData27,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData28,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData29,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData30,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData31,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData32,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData33,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData34,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData35,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData36,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData37,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData38,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData39);         /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc0DataSB5Clear( CAD_UINT uBaseAddress);    /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc0DataSB5FillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                        CAD_UINT uData0,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData1,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData2,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData3,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData4,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData5,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData6,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData7,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData8,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData9,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData10,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData11,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData12,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData13,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData14,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData15,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData16,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData17,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData18,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData19,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData20,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData21,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData22,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData23,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData24,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData25,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData26,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData27,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData28,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData29,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData30,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData31,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData32,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData33,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData34,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData35,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData36,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData37,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData38,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData39);   /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc0ResultResolverRead( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                        CAD_UINT uAddress,   /* Auto-Generated please see imf file*/
                                        CAD_UINT *pData);    /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc0ResultResolverWrite( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                         CAD_UINT uAddress,  /* Auto-Generated please see imf file*/
                                         CAD_UINT uData);    /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc0ResultResolverClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc0ResultResolverFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                               CAD_UINT uData); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc0ResultResolverBank0Read( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                             CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                             CAD_UINT *pData); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc0ResultResolverBank0Write( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                              CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                              CAD_UINT uData); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc0ResultResolverBank0Clear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc0ResultResolverBank0FillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                    CAD_UINT uData); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc0ResultResolverBank1Read( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                             CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                             CAD_UINT *pData); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc0ResultResolverBank1Write( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                              CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                              CAD_UINT uData); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc0ResultResolverBank1Clear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc0ResultResolverBank1FillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                    CAD_UINT uData); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc1DataRead( CAD_UINT uBaseAddress,         /* Auto-Generated please see imf file*/
                              CAD_UINT uAddress,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData0,              /* Auto-Generated please see imf file*/
                              CAD_UINT *pData1,              /* Auto-Generated please see imf file*/
                              CAD_UINT *pData2,              /* Auto-Generated please see imf file*/
                              CAD_UINT *pData3,              /* Auto-Generated please see imf file*/
                              CAD_UINT *pData4,              /* Auto-Generated please see imf file*/
                              CAD_UINT *pData5,              /* Auto-Generated please see imf file*/
                              CAD_UINT *pData6,              /* Auto-Generated please see imf file*/
                              CAD_UINT *pData7,              /* Auto-Generated please see imf file*/
                              CAD_UINT *pData8,              /* Auto-Generated please see imf file*/
                              CAD_UINT *pData9,              /* Auto-Generated please see imf file*/
                              CAD_UINT *pData10,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData11,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData12,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData13,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData14,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData15,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData16,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData17,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData18,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData19,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData20,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData21,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData22,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData23,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData24,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData25,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData26,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData27,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData28,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData29,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData30,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData31,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData32,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData33,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData34,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData35,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData36,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData37,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData38,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData39);            /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc1DataWrite( CAD_UINT uBaseAddress,        /* Auto-Generated please see imf file*/
                               CAD_UINT uAddress,            /* Auto-Generated please see imf file*/
                               CAD_UINT uData0,              /* Auto-Generated please see imf file*/
                               CAD_UINT uData1,              /* Auto-Generated please see imf file*/
                               CAD_UINT uData2,              /* Auto-Generated please see imf file*/
                               CAD_UINT uData3,              /* Auto-Generated please see imf file*/
                               CAD_UINT uData4,              /* Auto-Generated please see imf file*/
                               CAD_UINT uData5,              /* Auto-Generated please see imf file*/
                               CAD_UINT uData6,              /* Auto-Generated please see imf file*/
                               CAD_UINT uData7,              /* Auto-Generated please see imf file*/
                               CAD_UINT uData8,              /* Auto-Generated please see imf file*/
                               CAD_UINT uData9,              /* Auto-Generated please see imf file*/
                               CAD_UINT uData10,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData11,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData12,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData13,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData14,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData15,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData16,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData17,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData18,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData19,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData20,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData21,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData22,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData23,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData24,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData25,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData26,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData27,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData28,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData29,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData30,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData31,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData32,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData33,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData34,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData35,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData36,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData37,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData38,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData39);            /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc1DataClear( CAD_UINT uBaseAddress);       /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc1DataFillPattern( CAD_UINT uBaseAddress,  /* Auto-Generated please see imf file*/
                                     CAD_UINT uData0,        /* Auto-Generated please see imf file*/
                                     CAD_UINT uData1,        /* Auto-Generated please see imf file*/
                                     CAD_UINT uData2,        /* Auto-Generated please see imf file*/
                                     CAD_UINT uData3,        /* Auto-Generated please see imf file*/
                                     CAD_UINT uData4,        /* Auto-Generated please see imf file*/
                                     CAD_UINT uData5,        /* Auto-Generated please see imf file*/
                                     CAD_UINT uData6,        /* Auto-Generated please see imf file*/
                                     CAD_UINT uData7,        /* Auto-Generated please see imf file*/
                                     CAD_UINT uData8,        /* Auto-Generated please see imf file*/
                                     CAD_UINT uData9,        /* Auto-Generated please see imf file*/
                                     CAD_UINT uData10,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData11,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData12,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData13,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData14,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData15,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData16,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData17,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData18,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData19,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData20,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData21,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData22,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData23,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData24,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData25,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData26,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData27,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData28,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData29,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData30,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData31,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData32,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData33,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData34,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData35,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData36,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData37,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData38,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData39);      /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc1DataSB0Read( CAD_UINT uBaseAddress,      /* Auto-Generated please see imf file*/
                                 CAD_UINT uAddress,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData0,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData1,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData2,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData3,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData4,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData5,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData6,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData7,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData8,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData9,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData10,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData11,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData12,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData13,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData14,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData15,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData16,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData17,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData18,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData19,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData20,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData21,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData22,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData23,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData24,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData25,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData26,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData27,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData28,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData29,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData30,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData31,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData32,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData33,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData34,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData35,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData36,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData37,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData38,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData39);         /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc1DataSB0Write( CAD_UINT uBaseAddress,     /* Auto-Generated please see imf file*/
                                  CAD_UINT uAddress,         /* Auto-Generated please see imf file*/
                                  CAD_UINT uData0,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData1,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData2,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData3,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData4,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData5,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData6,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData7,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData8,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData9,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData10,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData11,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData12,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData13,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData14,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData15,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData16,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData17,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData18,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData19,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData20,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData21,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData22,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData23,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData24,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData25,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData26,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData27,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData28,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData29,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData30,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData31,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData32,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData33,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData34,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData35,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData36,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData37,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData38,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData39);         /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc1DataSB0Clear( CAD_UINT uBaseAddress);    /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc1DataSB0FillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                        CAD_UINT uData0,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData1,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData2,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData3,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData4,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData5,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData6,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData7,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData8,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData9,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData10,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData11,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData12,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData13,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData14,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData15,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData16,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData17,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData18,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData19,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData20,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData21,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData22,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData23,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData24,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData25,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData26,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData27,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData28,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData29,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData30,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData31,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData32,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData33,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData34,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData35,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData36,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData37,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData38,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData39);   /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc1DataSB1Read( CAD_UINT uBaseAddress,      /* Auto-Generated please see imf file*/
                                 CAD_UINT uAddress,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData0,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData1,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData2,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData3,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData4,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData5,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData6,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData7,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData8,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData9,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData10,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData11,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData12,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData13,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData14,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData15,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData16,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData17,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData18,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData19,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData20,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData21,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData22,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData23,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData24,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData25,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData26,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData27,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData28,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData29,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData30,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData31,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData32,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData33,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData34,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData35,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData36,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData37,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData38,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData39);         /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc1DataSB1Write( CAD_UINT uBaseAddress,     /* Auto-Generated please see imf file*/
                                  CAD_UINT uAddress,         /* Auto-Generated please see imf file*/
                                  CAD_UINT uData0,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData1,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData2,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData3,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData4,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData5,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData6,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData7,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData8,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData9,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData10,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData11,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData12,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData13,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData14,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData15,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData16,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData17,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData18,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData19,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData20,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData21,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData22,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData23,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData24,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData25,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData26,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData27,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData28,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData29,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData30,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData31,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData32,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData33,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData34,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData35,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData36,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData37,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData38,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData39);         /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc1DataSB1Clear( CAD_UINT uBaseAddress);    /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc1DataSB1FillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                        CAD_UINT uData0,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData1,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData2,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData3,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData4,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData5,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData6,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData7,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData8,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData9,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData10,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData11,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData12,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData13,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData14,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData15,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData16,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData17,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData18,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData19,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData20,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData21,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData22,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData23,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData24,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData25,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData26,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData27,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData28,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData29,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData30,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData31,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData32,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData33,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData34,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData35,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData36,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData37,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData38,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData39);   /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc1DataSB2Read( CAD_UINT uBaseAddress,      /* Auto-Generated please see imf file*/
                                 CAD_UINT uAddress,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData0,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData1,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData2,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData3,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData4,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData5,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData6,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData7,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData8,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData9,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData10,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData11,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData12,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData13,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData14,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData15,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData16,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData17,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData18,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData19,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData20,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData21,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData22,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData23,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData24,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData25,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData26,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData27,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData28,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData29,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData30,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData31,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData32,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData33,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData34,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData35,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData36,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData37,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData38,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData39);         /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc1DataSB2Write( CAD_UINT uBaseAddress,     /* Auto-Generated please see imf file*/
                                  CAD_UINT uAddress,         /* Auto-Generated please see imf file*/
                                  CAD_UINT uData0,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData1,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData2,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData3,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData4,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData5,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData6,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData7,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData8,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData9,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData10,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData11,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData12,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData13,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData14,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData15,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData16,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData17,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData18,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData19,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData20,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData21,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData22,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData23,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData24,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData25,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData26,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData27,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData28,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData29,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData30,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData31,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData32,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData33,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData34,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData35,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData36,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData37,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData38,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData39);         /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc1DataSB2Clear( CAD_UINT uBaseAddress);    /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc1DataSB2FillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                        CAD_UINT uData0,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData1,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData2,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData3,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData4,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData5,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData6,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData7,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData8,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData9,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData10,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData11,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData12,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData13,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData14,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData15,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData16,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData17,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData18,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData19,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData20,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData21,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData22,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData23,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData24,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData25,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData26,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData27,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData28,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData29,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData30,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData31,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData32,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData33,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData34,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData35,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData36,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData37,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData38,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData39);   /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc1DataSB3Read( CAD_UINT uBaseAddress,      /* Auto-Generated please see imf file*/
                                 CAD_UINT uAddress,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData0,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData1,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData2,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData3,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData4,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData5,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData6,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData7,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData8,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData9,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData10,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData11,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData12,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData13,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData14,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData15,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData16,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData17,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData18,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData19,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData20,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData21,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData22,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData23,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData24,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData25,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData26,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData27,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData28,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData29,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData30,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData31,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData32,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData33,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData34,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData35,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData36,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData37,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData38,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData39);         /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc1DataSB3Write( CAD_UINT uBaseAddress,     /* Auto-Generated please see imf file*/
                                  CAD_UINT uAddress,         /* Auto-Generated please see imf file*/
                                  CAD_UINT uData0,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData1,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData2,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData3,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData4,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData5,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData6,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData7,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData8,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData9,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData10,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData11,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData12,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData13,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData14,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData15,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData16,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData17,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData18,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData19,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData20,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData21,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData22,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData23,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData24,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData25,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData26,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData27,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData28,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData29,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData30,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData31,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData32,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData33,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData34,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData35,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData36,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData37,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData38,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData39);         /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc1DataSB3Clear( CAD_UINT uBaseAddress);    /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc1DataSB3FillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                        CAD_UINT uData0,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData1,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData2,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData3,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData4,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData5,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData6,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData7,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData8,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData9,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData10,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData11,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData12,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData13,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData14,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData15,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData16,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData17,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData18,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData19,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData20,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData21,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData22,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData23,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData24,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData25,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData26,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData27,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData28,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData29,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData30,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData31,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData32,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData33,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData34,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData35,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData36,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData37,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData38,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData39);   /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc1DataSB4Read( CAD_UINT uBaseAddress,      /* Auto-Generated please see imf file*/
                                 CAD_UINT uAddress,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData0,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData1,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData2,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData3,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData4,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData5,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData6,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData7,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData8,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData9,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData10,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData11,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData12,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData13,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData14,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData15,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData16,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData17,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData18,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData19,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData20,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData21,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData22,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData23,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData24,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData25,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData26,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData27,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData28,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData29,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData30,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData31,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData32,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData33,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData34,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData35,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData36,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData37,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData38,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData39);         /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc1DataSB4Write( CAD_UINT uBaseAddress,     /* Auto-Generated please see imf file*/
                                  CAD_UINT uAddress,         /* Auto-Generated please see imf file*/
                                  CAD_UINT uData0,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData1,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData2,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData3,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData4,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData5,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData6,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData7,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData8,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData9,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData10,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData11,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData12,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData13,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData14,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData15,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData16,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData17,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData18,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData19,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData20,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData21,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData22,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData23,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData24,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData25,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData26,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData27,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData28,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData29,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData30,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData31,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData32,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData33,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData34,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData35,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData36,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData37,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData38,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData39);         /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc1DataSB4Clear( CAD_UINT uBaseAddress);    /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc1DataSB4FillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                        CAD_UINT uData0,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData1,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData2,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData3,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData4,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData5,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData6,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData7,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData8,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData9,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData10,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData11,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData12,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData13,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData14,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData15,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData16,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData17,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData18,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData19,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData20,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData21,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData22,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData23,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData24,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData25,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData26,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData27,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData28,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData29,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData30,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData31,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData32,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData33,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData34,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData35,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData36,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData37,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData38,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData39);   /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc1DataSB5Read( CAD_UINT uBaseAddress,      /* Auto-Generated please see imf file*/
                                 CAD_UINT uAddress,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData0,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData1,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData2,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData3,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData4,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData5,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData6,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData7,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData8,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData9,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData10,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData11,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData12,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData13,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData14,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData15,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData16,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData17,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData18,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData19,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData20,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData21,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData22,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData23,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData24,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData25,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData26,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData27,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData28,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData29,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData30,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData31,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData32,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData33,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData34,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData35,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData36,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData37,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData38,          /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData39);         /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc1DataSB5Write( CAD_UINT uBaseAddress,     /* Auto-Generated please see imf file*/
                                  CAD_UINT uAddress,         /* Auto-Generated please see imf file*/
                                  CAD_UINT uData0,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData1,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData2,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData3,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData4,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData5,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData6,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData7,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData8,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData9,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData10,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData11,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData12,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData13,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData14,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData15,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData16,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData17,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData18,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData19,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData20,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData21,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData22,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData23,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData24,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData25,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData26,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData27,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData28,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData29,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData30,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData31,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData32,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData33,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData34,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData35,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData36,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData37,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData38,          /* Auto-Generated please see imf file*/
                                  CAD_UINT uData39);         /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc1DataSB5Clear( CAD_UINT uBaseAddress);    /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc1DataSB5FillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                        CAD_UINT uData0,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData1,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData2,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData3,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData4,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData5,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData6,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData7,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData8,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData9,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData10,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData11,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData12,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData13,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData14,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData15,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData16,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData17,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData18,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData19,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData20,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData21,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData22,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData23,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData24,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData25,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData26,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData27,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData28,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData29,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData30,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData31,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData32,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData33,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData34,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData35,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData36,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData37,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData38,    /* Auto-Generated please see imf file*/
                                        CAD_UINT uData39);   /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc1ResultResolverRead( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                        CAD_UINT uAddress,   /* Auto-Generated please see imf file*/
                                        CAD_UINT *pData);    /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc1ResultResolverWrite( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                         CAD_UINT uAddress,  /* Auto-Generated please see imf file*/
                                         CAD_UINT uData);    /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc1ResultResolverClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc1ResultResolverFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                               CAD_UINT uData); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc1ResultResolverBank0Read( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                             CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                             CAD_UINT *pData); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc1ResultResolverBank0Write( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                              CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                              CAD_UINT uData); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc1ResultResolverBank0Clear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc1ResultResolverBank0FillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                    CAD_UINT uData); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc1ResultResolverBank1Read( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                             CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                             CAD_UINT *pData); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc1ResultResolverBank1Write( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                              CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                              CAD_UINT uData); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc1ResultResolverBank1Clear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Rc1ResultResolverBank1FillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                    CAD_UINT uData); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_St0CounterRead( CAD_UINT uBaseAddress,      /* Auto-Generated please see imf file*/
                                 CAD_UINT uAddress,          /* Auto-Generated please see imf file*/
                                 CAD_UINT bClearOnRead,      /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData0,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData1);          /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_St0CounterWrite( CAD_UINT uBaseAddress,     /* Auto-Generated please see imf file*/
                                  CAD_UINT uAddress,         /* Auto-Generated please see imf file*/
                                  CAD_UINT uData0,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData1);          /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_St0CounterClear( CAD_UINT uBaseAddress);    /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_St0CounterFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                        CAD_UINT uData0,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData1);    /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_St1CounterRead( CAD_UINT uBaseAddress,      /* Auto-Generated please see imf file*/
                                 CAD_UINT uAddress,          /* Auto-Generated please see imf file*/
                                 CAD_UINT bClearOnRead,      /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData0,           /* Auto-Generated please see imf file*/
                                 CAD_UINT *pData1);          /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_St1CounterWrite( CAD_UINT uBaseAddress,     /* Auto-Generated please see imf file*/
                                  CAD_UINT uAddress,         /* Auto-Generated please see imf file*/
                                  CAD_UINT uData0,           /* Auto-Generated please see imf file*/
                                  CAD_UINT uData1);          /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_St1CounterClear( CAD_UINT uBaseAddress);    /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_St1CounterFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                        CAD_UINT uData0,     /* Auto-Generated please see imf file*/
                                        CAD_UINT uData1);    /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Mm0Internal0MemoryRead( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                         CAD_UINT uAddress,  /* Auto-Generated please see imf file*/
                                         CAD_UINT *pData0,   /* Auto-Generated please see imf file*/
                                         CAD_UINT *pData1);  /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Mm0Internal0MemoryWrite( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                          CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                          CAD_UINT uData0,   /* Auto-Generated please see imf file*/
                                          CAD_UINT uData1);  /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Mm0Internal0MemoryClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Mm0Internal0MemoryFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                                CAD_UINT uData1); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Mm0Internal1MemoryRead( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                         CAD_UINT uAddress,  /* Auto-Generated please see imf file*/
                                         CAD_UINT *pData0,   /* Auto-Generated please see imf file*/
                                         CAD_UINT *pData1);  /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Mm0Internal1MemoryWrite( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                          CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                          CAD_UINT uData0,   /* Auto-Generated please see imf file*/
                                          CAD_UINT uData1);  /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Mm0Internal1MemoryClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Mm0Internal1MemoryFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                                CAD_UINT uData1); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Mm0NarrowPort0MemoryRead( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                           CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                           CAD_UINT *pData0, /* Auto-Generated please see imf file*/
                                           CAD_UINT *pData1); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Mm0NarrowPort0MemoryWrite( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                            CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                            CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                            CAD_UINT uData1); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Mm0NarrowPort0MemoryClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Mm0NarrowPort0MemoryFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                  CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                                  CAD_UINT uData1); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Mm0NarrowPort1MemoryRead( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                           CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                           CAD_UINT *pData0, /* Auto-Generated please see imf file*/
                                           CAD_UINT *pData1); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Mm0NarrowPort1MemoryWrite( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                            CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                            CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                            CAD_UINT uData1); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Mm0NarrowPort1MemoryClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Mm0NarrowPort1MemoryFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                  CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                                  CAD_UINT uData1); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Mm0WidePortMemoryRead( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                        CAD_UINT uAddress,   /* Auto-Generated please see imf file*/
                                        CAD_UINT *pData0,    /* Auto-Generated please see imf file*/
                                        CAD_UINT *pData1);   /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Mm0WidePortMemoryWrite( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                         CAD_UINT uAddress,  /* Auto-Generated please see imf file*/
                                         CAD_UINT uData0,    /* Auto-Generated please see imf file*/
                                         CAD_UINT uData1);   /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Mm0WidePortMemoryClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Mm0WidePortMemoryFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                               CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                               CAD_UINT uData1); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Mm0WidePortMemoryRawRead( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                           CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                           CAD_UINT *pData0, /* Auto-Generated please see imf file*/
                                           CAD_UINT *pData1, /* Auto-Generated please see imf file*/
                                           CAD_UINT *pData2, /* Auto-Generated please see imf file*/
                                           BOOL bBypassEcc); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Mm0WidePortMemoryRawWrite( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                            CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                            CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                            CAD_UINT uData1, /* Auto-Generated please see imf file*/
                                            CAD_UINT uData2, /* Auto-Generated please see imf file*/
                                            BOOL bBypassEcc); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Mm0WidePortMemoryRawClear( CAD_UINT uBaseAddress, BOOL bBypassEcc); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Mm0WidePortMemoryRawFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                  CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                                  CAD_UINT uData1, /* Auto-Generated please see imf file*/
                                                  CAD_UINT uData2, /* Auto-Generated please see imf file*/
                                                  BOOL bBypassEcc); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Mm1Internal0MemoryRead( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                         CAD_UINT uAddress,  /* Auto-Generated please see imf file*/
                                         CAD_UINT *pData0,   /* Auto-Generated please see imf file*/
                                         CAD_UINT *pData1);  /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Mm1Internal0MemoryWrite( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                          CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                          CAD_UINT uData0,   /* Auto-Generated please see imf file*/
                                          CAD_UINT uData1);  /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Mm1Internal0MemoryClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Mm1Internal0MemoryFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                                CAD_UINT uData1); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Mm1Internal1MemoryRead( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                         CAD_UINT uAddress,  /* Auto-Generated please see imf file*/
                                         CAD_UINT *pData0,   /* Auto-Generated please see imf file*/
                                         CAD_UINT *pData1);  /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Mm1Internal1MemoryWrite( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                          CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                          CAD_UINT uData0,   /* Auto-Generated please see imf file*/
                                          CAD_UINT uData1);  /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Mm1Internal1MemoryClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Mm1Internal1MemoryFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                                CAD_UINT uData1); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Mm1NarrowPort0MemoryRead( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                           CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                           CAD_UINT *pData0, /* Auto-Generated please see imf file*/
                                           CAD_UINT *pData1); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Mm1NarrowPort0MemoryWrite( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                            CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                            CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                            CAD_UINT uData1); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Mm1NarrowPort0MemoryClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Mm1NarrowPort0MemoryFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                  CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                                  CAD_UINT uData1); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Mm1NarrowPort1MemoryRead( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                           CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                           CAD_UINT *pData0, /* Auto-Generated please see imf file*/
                                           CAD_UINT *pData1); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Mm1NarrowPort1MemoryWrite( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                            CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                            CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                            CAD_UINT uData1); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Mm1NarrowPort1MemoryClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Mm1NarrowPort1MemoryFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                  CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                                  CAD_UINT uData1); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Mm1WidePortMemoryRead( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                        CAD_UINT uAddress,   /* Auto-Generated please see imf file*/
                                        CAD_UINT *pData0,    /* Auto-Generated please see imf file*/
                                        CAD_UINT *pData1);   /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Mm1WidePortMemoryWrite( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                         CAD_UINT uAddress,  /* Auto-Generated please see imf file*/
                                         CAD_UINT uData0,    /* Auto-Generated please see imf file*/
                                         CAD_UINT uData1);   /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Mm1WidePortMemoryClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Mm1WidePortMemoryFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                               CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                               CAD_UINT uData1); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Mm1WidePortMemoryRawRead( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                           CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                           CAD_UINT *pData0, /* Auto-Generated please see imf file*/
                                           CAD_UINT *pData1, /* Auto-Generated please see imf file*/
                                           CAD_UINT *pData2, /* Auto-Generated please see imf file*/
                                           BOOL bBypassEcc); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Mm1WidePortMemoryRawWrite( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                            CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                            CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                            CAD_UINT uData1, /* Auto-Generated please see imf file*/
                                            CAD_UINT uData2, /* Auto-Generated please see imf file*/
                                            BOOL bBypassEcc); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Mm1WidePortMemoryRawClear( CAD_UINT uBaseAddress, BOOL bBypassEcc); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Mm1WidePortMemoryRawFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                  CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                                  CAD_UINT uData1, /* Auto-Generated please see imf file*/
                                                  CAD_UINT uData2, /* Auto-Generated please see imf file*/
                                                  BOOL bBypassEcc); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Pb0DataRead( CAD_UINT uBaseAddress,         /* Auto-Generated please see imf file*/
                              CAD_UINT uAddress,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData0,              /* Auto-Generated please see imf file*/
                              CAD_UINT *pData1,              /* Auto-Generated please see imf file*/
                              CAD_UINT *pData2,              /* Auto-Generated please see imf file*/
                              CAD_UINT *pData3,              /* Auto-Generated please see imf file*/
                              CAD_UINT *pData4,              /* Auto-Generated please see imf file*/
                              CAD_UINT *pData5,              /* Auto-Generated please see imf file*/
                              CAD_UINT *pData6,              /* Auto-Generated please see imf file*/
                              CAD_UINT *pData7,              /* Auto-Generated please see imf file*/
                              CAD_UINT *pData8,              /* Auto-Generated please see imf file*/
                              CAD_UINT *pData9,              /* Auto-Generated please see imf file*/
                              CAD_UINT *pData10,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData11,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData12,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData13,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData14,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData15,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData16,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData17,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData18,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData19,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData20,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData21,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData22,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData23);            /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Pb0DataWrite( CAD_UINT uBaseAddress,        /* Auto-Generated please see imf file*/
                               CAD_UINT uAddress,            /* Auto-Generated please see imf file*/
                               CAD_UINT uData0,              /* Auto-Generated please see imf file*/
                               CAD_UINT uData1,              /* Auto-Generated please see imf file*/
                               CAD_UINT uData2,              /* Auto-Generated please see imf file*/
                               CAD_UINT uData3,              /* Auto-Generated please see imf file*/
                               CAD_UINT uData4,              /* Auto-Generated please see imf file*/
                               CAD_UINT uData5,              /* Auto-Generated please see imf file*/
                               CAD_UINT uData6,              /* Auto-Generated please see imf file*/
                               CAD_UINT uData7,              /* Auto-Generated please see imf file*/
                               CAD_UINT uData8,              /* Auto-Generated please see imf file*/
                               CAD_UINT uData9,              /* Auto-Generated please see imf file*/
                               CAD_UINT uData10,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData11,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData12,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData13,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData14,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData15,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData16,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData17,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData18,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData19,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData20,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData21,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData22,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData23);            /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Pb0DataClear( CAD_UINT uBaseAddress);       /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Pb0DataFillPattern( CAD_UINT uBaseAddress,  /* Auto-Generated please see imf file*/
                                     CAD_UINT uData0,        /* Auto-Generated please see imf file*/
                                     CAD_UINT uData1,        /* Auto-Generated please see imf file*/
                                     CAD_UINT uData2,        /* Auto-Generated please see imf file*/
                                     CAD_UINT uData3,        /* Auto-Generated please see imf file*/
                                     CAD_UINT uData4,        /* Auto-Generated please see imf file*/
                                     CAD_UINT uData5,        /* Auto-Generated please see imf file*/
                                     CAD_UINT uData6,        /* Auto-Generated please see imf file*/
                                     CAD_UINT uData7,        /* Auto-Generated please see imf file*/
                                     CAD_UINT uData8,        /* Auto-Generated please see imf file*/
                                     CAD_UINT uData9,        /* Auto-Generated please see imf file*/
                                     CAD_UINT uData10,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData11,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData12,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData13,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData14,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData15,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData16,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData17,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData18,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData19,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData20,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData21,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData22,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData23);      /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Pb1DataRead( CAD_UINT uBaseAddress,         /* Auto-Generated please see imf file*/
                              CAD_UINT uAddress,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData0,              /* Auto-Generated please see imf file*/
                              CAD_UINT *pData1,              /* Auto-Generated please see imf file*/
                              CAD_UINT *pData2,              /* Auto-Generated please see imf file*/
                              CAD_UINT *pData3,              /* Auto-Generated please see imf file*/
                              CAD_UINT *pData4,              /* Auto-Generated please see imf file*/
                              CAD_UINT *pData5,              /* Auto-Generated please see imf file*/
                              CAD_UINT *pData6,              /* Auto-Generated please see imf file*/
                              CAD_UINT *pData7,              /* Auto-Generated please see imf file*/
                              CAD_UINT *pData8,              /* Auto-Generated please see imf file*/
                              CAD_UINT *pData9,              /* Auto-Generated please see imf file*/
                              CAD_UINT *pData10,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData11,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData12,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData13,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData14,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData15,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData16,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData17,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData18,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData19,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData20,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData21,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData22,             /* Auto-Generated please see imf file*/
                              CAD_UINT *pData23);            /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Pb1DataWrite( CAD_UINT uBaseAddress,        /* Auto-Generated please see imf file*/
                               CAD_UINT uAddress,            /* Auto-Generated please see imf file*/
                               CAD_UINT uData0,              /* Auto-Generated please see imf file*/
                               CAD_UINT uData1,              /* Auto-Generated please see imf file*/
                               CAD_UINT uData2,              /* Auto-Generated please see imf file*/
                               CAD_UINT uData3,              /* Auto-Generated please see imf file*/
                               CAD_UINT uData4,              /* Auto-Generated please see imf file*/
                               CAD_UINT uData5,              /* Auto-Generated please see imf file*/
                               CAD_UINT uData6,              /* Auto-Generated please see imf file*/
                               CAD_UINT uData7,              /* Auto-Generated please see imf file*/
                               CAD_UINT uData8,              /* Auto-Generated please see imf file*/
                               CAD_UINT uData9,              /* Auto-Generated please see imf file*/
                               CAD_UINT uData10,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData11,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData12,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData13,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData14,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData15,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData16,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData17,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData18,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData19,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData20,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData21,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData22,             /* Auto-Generated please see imf file*/
                               CAD_UINT uData23);            /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Pb1DataClear( CAD_UINT uBaseAddress);       /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_Pb1DataFillPattern( CAD_UINT uBaseAddress,  /* Auto-Generated please see imf file*/
                                     CAD_UINT uData0,        /* Auto-Generated please see imf file*/
                                     CAD_UINT uData1,        /* Auto-Generated please see imf file*/
                                     CAD_UINT uData2,        /* Auto-Generated please see imf file*/
                                     CAD_UINT uData3,        /* Auto-Generated please see imf file*/
                                     CAD_UINT uData4,        /* Auto-Generated please see imf file*/
                                     CAD_UINT uData5,        /* Auto-Generated please see imf file*/
                                     CAD_UINT uData6,        /* Auto-Generated please see imf file*/
                                     CAD_UINT uData7,        /* Auto-Generated please see imf file*/
                                     CAD_UINT uData8,        /* Auto-Generated please see imf file*/
                                     CAD_UINT uData9,        /* Auto-Generated please see imf file*/
                                     CAD_UINT uData10,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData11,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData12,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData13,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData14,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData15,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData16,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData17,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData18,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData19,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData20,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData21,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData22,       /* Auto-Generated please see imf file*/
                                     CAD_UINT uData23);      /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtMirrorIndexRead( CAD_UINT uBaseAddress,   /* Auto-Generated please see imf file*/
                                    CAD_UINT uAddress,       /* Auto-Generated please see imf file*/
                                    CAD_UINT *pData0,        /* Auto-Generated please see imf file*/
                                    CAD_UINT *pData1,        /* Auto-Generated please see imf file*/
                                    CAD_UINT *pData2,        /* Auto-Generated please see imf file*/
                                    CAD_UINT *pData3,        /* Auto-Generated please see imf file*/
                                    CAD_UINT *pData4);       /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtMirrorIndexWrite( CAD_UINT uBaseAddress,  /* Auto-Generated please see imf file*/
                                     CAD_UINT uAddress,      /* Auto-Generated please see imf file*/
                                     CAD_UINT uData0,        /* Auto-Generated please see imf file*/
                                     CAD_UINT uData1,        /* Auto-Generated please see imf file*/
                                     CAD_UINT uData2,        /* Auto-Generated please see imf file*/
                                     CAD_UINT uData3,        /* Auto-Generated please see imf file*/
                                     CAD_UINT uData4);       /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtMirrorIndexClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtMirrorIndexFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                           CAD_UINT uData0,  /* Auto-Generated please see imf file*/
                                           CAD_UINT uData1,  /* Auto-Generated please see imf file*/
                                           CAD_UINT uData2,  /* Auto-Generated please see imf file*/
                                           CAD_UINT uData3,  /* Auto-Generated please see imf file*/
                                           CAD_UINT uData4); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPortToQueueAg0Read( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                       CAD_UINT uAddress,    /* Auto-Generated please see imf file*/
                                       CAD_UINT *pData);     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPortToQueueAg0Write( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                        CAD_UINT uAddress,   /* Auto-Generated please see imf file*/
                                        CAD_UINT uData);     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPortToQueueAg0Clear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPortToQueueAg0FillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                              CAD_UINT uData); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPortToQueueAg1Read( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                       CAD_UINT uAddress,    /* Auto-Generated please see imf file*/
                                       CAD_UINT *pData);     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPortToQueueAg1Write( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                        CAD_UINT uAddress,   /* Auto-Generated please see imf file*/
                                        CAD_UINT uData);     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPortToQueueAg1Clear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPortToQueueAg1FillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                              CAD_UINT uData); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPortToQueuePciRead( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                       CAD_UINT uAddress,    /* Auto-Generated please see imf file*/
                                       CAD_UINT *pData);     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPortToQueuePciWrite( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                        CAD_UINT uAddress,   /* Auto-Generated please see imf file*/
                                        CAD_UINT uData);     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPortToQueuePciClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPortToQueuePciFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                              CAD_UINT uData); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPortToQueueSt0Read( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                       CAD_UINT uAddress,    /* Auto-Generated please see imf file*/
                                       CAD_UINT *pData);     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPortToQueueSt0Write( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                        CAD_UINT uAddress,   /* Auto-Generated please see imf file*/
                                        CAD_UINT uData);     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPortToQueueSt0Clear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPortToQueueSt0FillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                              CAD_UINT uData); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPortToQueueSt1Read( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                       CAD_UINT uAddress,    /* Auto-Generated please see imf file*/
                                       CAD_UINT *pData);     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPortToQueueSt1Write( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                        CAD_UINT uAddress,   /* Auto-Generated please see imf file*/
                                        CAD_UINT uData);     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPortToQueueSt1Clear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPortToQueueSt1FillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                              CAD_UINT uData); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPortToQueueXg0Read( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                       CAD_UINT uAddress,    /* Auto-Generated please see imf file*/
                                       CAD_UINT *pData);     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPortToQueueXg0Write( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                        CAD_UINT uAddress,   /* Auto-Generated please see imf file*/
                                        CAD_UINT uData);     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPortToQueueXg0Clear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPortToQueueXg0FillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                              CAD_UINT uData); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPortToQueueXg1Read( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                       CAD_UINT uAddress,    /* Auto-Generated please see imf file*/
                                       CAD_UINT *pData);     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPortToQueueXg1Write( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                        CAD_UINT uAddress,   /* Auto-Generated please see imf file*/
                                        CAD_UINT uData);     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPortToQueueXg1Clear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPortToQueueXg1FillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                              CAD_UINT uData); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte0MirrorPortStateRead( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                            CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                            CAD_UINT *pData0, /* Auto-Generated please see imf file*/
                                            CAD_UINT *pData1); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte0MirrorPortStateWrite( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                             CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData1); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte0MirrorPortStateClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte0MirrorPortStateFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData1); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte0PortStateRead( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                      CAD_UINT uAddress,     /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData0,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData1,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData2);     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte0PortStateWrite( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                       CAD_UINT uAddress,    /* Auto-Generated please see imf file*/
                                       CAD_UINT uData0,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData1,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData2);     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte0PortStateClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte0PortStateFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData1, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData2); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte1MirrorPortStateRead( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                            CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                            CAD_UINT *pData0, /* Auto-Generated please see imf file*/
                                            CAD_UINT *pData1); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte1MirrorPortStateWrite( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                             CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData1); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte1MirrorPortStateClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte1MirrorPortStateFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData1); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte1PortStateRead( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                      CAD_UINT uAddress,     /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData0,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData1,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData2);     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte1PortStateWrite( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                       CAD_UINT uAddress,    /* Auto-Generated please see imf file*/
                                       CAD_UINT uData0,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData1,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData2);     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte1PortStateClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte1PortStateFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData1, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData2); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte2MirrorPortStateRead( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                            CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                            CAD_UINT *pData0, /* Auto-Generated please see imf file*/
                                            CAD_UINT *pData1); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte2MirrorPortStateWrite( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                             CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData1); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte2MirrorPortStateClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte2MirrorPortStateFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData1); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte2PortStateRead( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                      CAD_UINT uAddress,     /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData0,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData1,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData2);     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte2PortStateWrite( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                       CAD_UINT uAddress,    /* Auto-Generated please see imf file*/
                                       CAD_UINT uData0,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData1,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData2);     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte2PortStateClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte2PortStateFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData1, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData2); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte3MirrorPortStateRead( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                            CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                            CAD_UINT *pData0, /* Auto-Generated please see imf file*/
                                            CAD_UINT *pData1); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte3MirrorPortStateWrite( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                             CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData1); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte3MirrorPortStateClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte3MirrorPortStateFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData1); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte3PortStateRead( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                      CAD_UINT uAddress,     /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData0,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData1,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData2);     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte3PortStateWrite( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                       CAD_UINT uAddress,    /* Auto-Generated please see imf file*/
                                       CAD_UINT uData0,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData1,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData2);     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte3PortStateClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte3PortStateFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData1, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData2); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte4MirrorPortStateRead( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                            CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                            CAD_UINT *pData0, /* Auto-Generated please see imf file*/
                                            CAD_UINT *pData1); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte4MirrorPortStateWrite( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                             CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData1); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte4MirrorPortStateClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte4MirrorPortStateFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData1); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte4PortStateRead( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                      CAD_UINT uAddress,     /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData0,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData1,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData2);     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte4PortStateWrite( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                       CAD_UINT uAddress,    /* Auto-Generated please see imf file*/
                                       CAD_UINT uData0,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData1,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData2);     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte4PortStateClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte4PortStateFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData1, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData2); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte5MirrorPortStateRead( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                            CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                            CAD_UINT *pData0, /* Auto-Generated please see imf file*/
                                            CAD_UINT *pData1); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte5MirrorPortStateWrite( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                             CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData1); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte5MirrorPortStateClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte5MirrorPortStateFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData1); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte5PortStateRead( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                      CAD_UINT uAddress,     /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData0,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData1,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData2);     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte5PortStateWrite( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                       CAD_UINT uAddress,    /* Auto-Generated please see imf file*/
                                       CAD_UINT uData0,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData1,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData2);     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte5PortStateClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte5PortStateFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData1, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData2); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte6MirrorPortStateRead( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                            CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                            CAD_UINT *pData0, /* Auto-Generated please see imf file*/
                                            CAD_UINT *pData1); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte6MirrorPortStateWrite( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                             CAD_UINT uAddress, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData1); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte6MirrorPortStateClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte6MirrorPortStateFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                                   CAD_UINT uData1); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte6PortStateRead( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                      CAD_UINT uAddress,     /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData0,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData1,      /* Auto-Generated please see imf file*/
                                      CAD_UINT *pData2);     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte6PortStateWrite( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                       CAD_UINT uAddress,    /* Auto-Generated please see imf file*/
                                       CAD_UINT uData0,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData1,      /* Auto-Generated please see imf file*/
                                       CAD_UINT uData2);     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte6PortStateClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtPte6PortStateFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData0, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData1, /* Auto-Generated please see imf file*/
                                             CAD_UINT uData2); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtQueueToChannelRead( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                       CAD_UINT uAddress,    /* Auto-Generated please see imf file*/
                                       CAD_UINT *pData);     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtQueueToChannelWrite( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                        CAD_UINT uAddress,   /* Auto-Generated please see imf file*/
                                        CAD_UINT uData);     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtQueueToChannelClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtQueueToChannelFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                              CAD_UINT uData); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtQueueToPortRead( CAD_UINT uBaseAddress,   /* Auto-Generated please see imf file*/
                                    CAD_UINT uAddress,       /* Auto-Generated please see imf file*/
                                    CAD_UINT *pData);        /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtQueueToPortWrite( CAD_UINT uBaseAddress,  /* Auto-Generated please see imf file*/
                                     CAD_UINT uAddress,      /* Auto-Generated please see imf file*/
                                     CAD_UINT uData);        /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtQueueToPortClear( CAD_UINT uBaseAddress); /* Auto-Generated please see imf file*/
sbFe2000XtImfDriverStatus_t                                     /* Auto-Generated please see imf file*/
sbFe2000XtImfDriver_PtQueueToPortFillPattern( CAD_UINT uBaseAddress, /* Auto-Generated please see imf file*/
                                           CAD_UINT uData);  /* Auto-Generated please see imf file*/
                                                             /* Auto-Generated please see imf file*/
                                                             /* Auto-Generated please see imf file*/
                                                             /* Auto-Generated please see imf file*/
#endif
