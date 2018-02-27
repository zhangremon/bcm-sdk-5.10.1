
/*
 * $Id: sbTypesGlue.h 1.6.196.1 Broadcom SDK $
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
 * Basic types on which all Sandburst code depends
 */

#ifndef _SAND_TYPES_GLUE_H_
#define _SAND_TYPES_GLUE_H_

#ifdef VXWORKS
/* Needed to fix compiler error in vxwork target for types redefinition */
#include "types/vxTypesOld.h"
#else
#ifdef __KERNEL__
#include <linux/types.h>
#else
#include <sys/types.h>
#endif
#endif

#include <sal/types.h>
#include <sal/core/libc.h>

#ifndef soc_htonl
#if defined(LE_HOST)
extern unsigned int _shr_swap32(unsigned int val);
#define	soc_htonl(_l)	_shr_swap32(_l)
#define soc_ntohl(_l)   _shr_swap32(_l)
#else /* BE_HOST */
#define	soc_htonl(_l)	(_l)
#define soc_ntohl(_l)   (_l)
#endif /* BE_HOST */
#endif

#ifdef uint8_t
#undef uint8_t
#endif
#define uint8_t uint8
#ifdef uint16_t
#undef uint16_t
#endif
#define uint16_t uint16
#ifdef uint32_t
#undef uint32_t
#endif
#define uint32_t uint32
#ifdef int8_t
#undef int8_t
#endif
#define int8_t  int8
#ifdef int32_t
#undef int32_t
#endif
#define int32_t int32
#ifdef int64_t
#undef int64_t
#endif
#define int64_t int64
#ifdef UINT
#undef UINT
#endif
#define UINT uint32
#ifdef uint64_t
#undef uint64_t
#endif
#define uint64_t uint64
#ifdef ULLONG
#undef ULLONG
#endif
#define ULLONG uint64
#ifdef uint
#undef uint
#endif
#define uint UINT

typedef uint8_t sbBool_t;
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define SAND_CHIP_USER_INT_TYPES
#include "sbStatus.h"


#ifndef CHIP_USER_SYNC_TOKEN_TYPE
#define CHIP_USER_SYNC_TOKEN_TYPE
typedef int sandCUSyncToken_t;
typedef sandCUSyncToken_t sbSyncToken_t;
#endif

/**
 * The datatype defines the argument which will be returned along with
 * status and type from the callback funtions. For SMAC againg this would
 * return the 8-byte machHandle (Vlan, SMAC) 
 */
typedef uint64_t sbMacEntryHandle_t[2], *sbMacEntryHandle_p_t;
typedef union 
{
    sbMacEntryHandle_t macHandle;     /**< for OLD_{SMAC,DMAC} */
    uint32_t           HbaRecord[32]; /**< For Unloader Isr    */
}sbSpecialArgs_t;

#ifndef _SB_CHIP_T_
#define _SB_CHIP_T_
typedef enum sb_chip_e {
  SB_INVALID = 0,
  SB_IFE_1000,
  SB_EFE_1000,
  SB_QE_1000,
  SB_BME_1600,
  SB_SE_1600,
  SB_PLX_9656,
  SB_BME_3200,
  SB_QE_2000,
  SB_GLUE_FPGA,
  SB_DMA_MEMORY,
  SB_FE_2000
} sb_chip_t; 
#endif

typedef enum sbFeMallocType_e {
  SB_ALLOC_INTERNAL,
  SB_ALLOC_CLS_DMA,
  SB_ALLOC_L2_DMA,
  SB_ALLOC_OTHER_DMA,
  SB_ALLOC_CLS_TO_L2,
  SB_ALLOC_CLS_TO_IPV4,
  SB_ALLOC_IPV4_DMA,
  SB_ALLOC_LPM_DMA,
  SB_ALLOC_COUNTERS
} sbFeMallocType_t;

typedef struct sbDmaMemoryHandle_s {
  void *handle;
} sbDmaMemoryHandle_t;

typedef uint32_t sbreg;		/* register type */
struct sbhandle_s;
typedef struct sbhandle_s *sbhandle; /* handle to device */

/* for backwards compatibility, old apps should be updated to use sbhandle */
typedef struct sbhandle_s *sbregshandle;

typedef enum thin_bus_e {
    THIN_BUS_LE,
    THIN_BUS_BE
} thin_bus_t;

/**
 * Description goes here
 */
typedef struct sbFeAsyncCallbackArgument_s {
  void *clientData;                  /**< Description goes here */
  int type;         /**< Description goes here */
  sbStatus_t status;            /**< Description goes here */
  sbSpecialArgs_t special_args; /**< Description goes here */
} sbFeAsyncCallbackArgument_t, *sbFeAsyncCallbackArgument_p_t;

typedef void (*sbFeAsyncCallback_f_t) 
     (sbFeAsyncCallbackArgument_p_t arg);

typedef void (*sbFeInitAsyncCallback_f_t)
                   (void *initId, sbStatus_t status);


typedef sbregshandle sandCUDevAddr_t;

typedef uint32_t sbFeDmaHostBusAddress_t;

/**
 * ELib FLib MVT Access Semaphore Key.
 */
#define SB_ELIB_FLIB_SEM_ACCESS_KEY  0xF11BE11B;
/***
 * Elib internal counter fetch Semaphore Key.
 */
#define SB_ELIB_COUNTER_GET_SEM_ACCESS_KEY  0xE5E5E11B;

#ifdef SAND_HTONL
#undef SAND_HTONL
#endif
#ifdef SAND_NTOHL
#undef SAND_NTOHL
#endif
#define SAND_HTONL(x) soc_htonl(x)
#define SAND_NTOHL(x) soc_ntohl(x)

#endif
