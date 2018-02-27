/******************************************************************************
** ========================================================================
** == glue.h -  Lower level OS specific glue layer for Sandburst devices ==
** ========================================================================
**
** WORKING REVISION: $Id: glue_dma.h 1.2.224.1 Broadcom SDK $
**
** $Copyright: Copyright 2011 Broadcom Corporation.
** This program is the proprietary software of Broadcom Corporation
** and/or its licensors, and may only be used, duplicated, modified
** or distributed pursuant to the terms and conditions of a separate,
** written license agreement executed between you and Broadcom
** (an "Authorized License").  Except as set forth in an Authorized
** License, Broadcom grants no license (express or implied), right
** to use, or waiver of any kind with respect to the Software, and
** Broadcom expressly reserves all rights in and to the Software
** and all intellectual property rights therein.  IF YOU HAVE
** NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE
** IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
** ALL USE OF THE SOFTWARE.  
**  
** Except as expressly set forth in the Authorized License,
**  
** 1.     This program, including its structure, sequence and organization,
** constitutes the valuable trade secrets of Broadcom, and you shall use
** all reasonable efforts to protect the confidentiality thereof,
** and to use this information only in connection with your use of
** Broadcom integrated circuit products.
**  
** 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS
** PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
** REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY,
** OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
** DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
** NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
** ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
** CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
** OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
** 
** 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
** BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL,
** INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER
** ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE OF OR INABILITY
** TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE
** POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF
** THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR USD 1.00,
** WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING
** ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.$
**
** MODULE NAME:
**
**     glue_p.h
**
** ABSTRACT:
**
**     Private glue header file.  This file will most likely be unecessary once
** simserver learning interfaces with the QE_NIC driver.
**
** LANGUAGE:
**
**     C
**
** AUTHORS:
**
**     Sean Campbell
**
** CREATION DATE:
**
**     07-Dec-2004
**
******************************************************************************/
#ifndef _THINGLUE_DMA_H_
#define _THINGLUE_DMA_H_

#include "sbTypesGlue.h"

#ifdef SB_LINUX
#include <linux/sbthinioctl.h>

/* For Interfacing with the DMA */
struct dma {
  struct sb_alloc_mem  alloc;  /* alloc information */
  char                *base;   /* Pointer to the Register Base */
  uint32_t             page_size; /* Page Size */
};
#endif /* SB_LINUX */

extern sbhandle thin_dma_handle(sbhandle hdl, uint32_t addr);

extern sbStatus_t thin_getHba(sbDmaMemoryHandle_t mHandle, uint32_t *address, uint32_t words,
	  sbFeDmaHostBusAddress_t *hbaP);

extern sbStatus_t thin_writeSync(sbDmaMemoryHandle_t mHandle, uint32_t *address, uint32_t words);

extern sbStatus_t thin_readSync(sbDmaMemoryHandle_t mHandle, uint32_t *address, uint32_t words);

extern sbSyncToken_t thin_isrSync(void *data);

extern void thin_isrUnsync(void *data, sbSyncToken_t token);

extern void thin_async_cback(sbFeAsyncCallbackArgument_p_t arg);

/* ENHANCE...should probably rename these two to alleviate potential confusion */
extern sbStatus_t thin_wait_for_async(void *clientData, uint32_t cnt);

extern sbStatus_t thin_wait_for_async_type(void *clientData, int type);

#endif /* _THINGLUE_DMA_H_ */
