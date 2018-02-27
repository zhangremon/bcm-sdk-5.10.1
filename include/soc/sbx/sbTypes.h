#ifndef _SAND_TYPES_H_
#define _SAND_TYPES_H_
/* --------------------------------------------------------------------------
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
** sbTypes.h: basic types on which all Sandburst code depends
**
** $Id: sbTypes.h 1.12.156.1 Broadcom SDK $
**
** --------------------------------------------------------------------------*/
#ifdef BCM_SBX_SUPPORT
#include <soc/sbx/sbTypesGlue.h>
#endif

#if defined(SAND_CHIP_USER) || defined(SAND_CHIP_USER_PATH)
#ifndef SAND_CHIP_USER_PATH
#define SAND_CHIP_USER_PATH <chip_user.h>
#endif
#include SAND_CHIP_USER_PATH
#endif

#ifndef SAND_CHIP_USER_INT_TYPES
#if !defined(__NetBSD__)
#if !defined(__linux__)
#if !defined(__sun__) && !defined(VXWORKS)
#if !defined(__uint8_t_defined)
typedef unsigned char uint8_t;
#endif
#if !defined(__uint16_t_defined)
typedef unsigned short uint16_t;
#endif
#if !defined(__uint32_t_defined)
typedef unsigned int uint32_t;
#endif
#endif /* __!defined(__sun__) && !defined(__VXWORKS__) */
#if !defined(__uint64_t_defined)
#ifndef VXWORKS
typedef unsigned long long uint64_t;
#endif /* !defined(VXWORKS) */
#endif
#endif /* !defined(__linux__) */
#endif /* !defined(__NetBSD__) */
#if defined(__NetBSD__) && !defined(uint8_t)
#define uint8_t u_int8_t
#define uint16_t u_int16_t
#define uint32_t u_int32_t
#define uint64_t u_int64_t
#endif /* defined(__NetBSD__) && !defined(uint8_t) */

#if defined(__linux__) && !defined(uint8_t)
#define uint8_t u_int8_t
#define uint16_t u_int16_t
#define uint32_t u_int32_t
#define uint64_t u_int64_t
#endif /* defined(__linux__) && !defined(uint8_t) */

#ifdef VXWORKS
#include "types/vxTypesOld.h"
#else  /* VXWORKS */
#ifndef ZDT_64
typedef uint32_t UINT;
#endif
#endif /* VXWORKS */
#ifndef uint
#define uint unsigned int
#endif /* uint */
#endif /* SAND_CHIP_USER_INT_TYPES */

#if defined(uint8_t)
#define __uint8_t_defined
#endif
#if defined(uint16_t)
#define __uint16_t_defined
#endif
#if defined(uint32_t)
#define __uint32_t_defined
#endif
#if defined(uint64_t)
#define __uint64_t_defined
#endif
#if defined(int8_t)
#define __int8_t_defined
#endif
#if defined(int16_t)
#define __int16_t_defined
#endif
#if defined(int32_t)
#define __int32_t_defined
#endif
#if defined(int64_t)
#define __int64_t_defined
#endif

#ifdef BE_HOST
#define SAND_BIG_ENDIAN_HOST
#endif

#ifdef SAND_BIG_ENDIAN_HOST
#define SAND_HOST_TO_FE_16(x) SAND_SWAP_16(x)
#define SAND_HOST_TO_FE_32(x) SAND_SWAP_32(x)
#define SAND_FE_TO_HOST_16(x) SAND_SWAP_16(x)
#define SAND_FE_TO_HOST_32(x) SAND_SWAP_32(x)
#else
#define SAND_HOST_TO_FE_16(x) (x)
#define SAND_HOST_TO_FE_32(x) (x)
#define SAND_FE_TO_HOST_16(x) (x)
#define SAND_FE_TO_HOST_32(x) (x)
#endif

#ifndef SAND_SWAP_16
static uint16_t sandSwap16(uint16_t x);
static uint16_t (*sandSwap16p)(uint16_t) = sandSwap16; /* fool gcc -Wall */
static uint16_t sandSwap16(uint16_t x)
{
  sandSwap16p = sandSwap16; /* fool gcc -Wall */
  return ((x >> 8) & 0xff) | ((x & 0xff) << 8);
}
#define SAND_SWAP_16(x) sandSwap16((uint16_t) (x))
#endif

#ifndef SAND_SWAP_32
static uint32_t sandSwap32(uint32_t x);
static uint32_t (*sandSwap32p)(uint32_t) = sandSwap32; /* fool gcc -Wall */
static uint32_t sandSwap32(uint32_t x)
{
  sandSwap32p = sandSwap32; /* fool gcc -Wall */
  return ((x >> 24) & 0xff)
    | ((x >> 8) & 0xff00)
    | ((x & 0xff00) << 8)
    | ((x & 0xff) << 24);
}
#define SAND_SWAP_32(x) sandSwap32((uint32_t) (x))
#endif

#ifndef NULL
#define NULL 0
#endif

/**
 * When a MAC address is added, a client supplies a handle that identifies
 * the address later if other entries collide, or the entry becomes too old.
 */

/**
 * Sandburst Library Version Structure
 */
typedef struct sbSwLibVersion_s
{
    uint32_t productCode;      /**< Sandburst Product Code */
    uint32_t majorVersion;     /**< Major Version of the library */
    uint32_t minorVersion;     /**< Minor Version of the library */
    uint32_t patchLevel;       /**< Patch Level of the library */
    char     description[80];  /**< Description of the library */
} sbSwLibVersion_t, *sbSwLibVersion_p_t;

/**
 * Sandburst Microcode Version Structure
 */
typedef struct sbUcodeVersion_s
{
    uint32_t productCode;            /**< Sandburst Product Code of parent library */
    uint32_t libMajorVersion;        /**< Major Version of the parent library */
    uint32_t libMinorVersion;        /**< Minor Version of the parent library */
    uint32_t libPatchLevel;          /**< Patch Level of the parent library */
    uint32_t ucodeMajorVersion;      /**< Major Version of the microcode */
    uint32_t ucodeMinorVersion;      /**< Minor Version of the microcode */
    uint32_t ucodePatchLevel;        /**< Patch Level of the microcode */
    char     packageDescription[80]; /**< Description of the library */
} sbUcodeVersion_t, *sbUcodeVersion_p_t;

#endif
