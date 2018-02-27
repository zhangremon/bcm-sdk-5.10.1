#ifndef _SB_STATUS_H_
#define _SB_STATUS_H_
/* --------------------------------------------------------------------------
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
** $Id: sbStatus.h 1.7.156.1 Broadcom SDK $
**
** --------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

typedef enum sbVendorID_es
{
  SB_VENDOR_ID_SANDBURST = 1,
  SB_VENDOR_ID_LAST
} sbVendorID_et;

typedef enum sbModuleID_es
{
  SB_MODULE_ID_FE_ILIB = 1,
  SB_MODULE_ID_QE_ELIB = 2,
  SB_MODULE_ID_FAB_ILIB = 3,
  SB_MODULE_FE1000 = 4,
  SB_MODULE_ID_FLIB2 = 5,
  SB_MODULE_ID_MEM = 6,
  SB_MODULE_ID_FE2000MEM = 7,
  SB_MODULE_ID_FE2000_INIT = 8,
  SB_MODULE_ID_LAST
} sbModuleID_et;

typedef int sbStatus_t;
typedef int* sbStatus_pt;

#define SB_BUILD_ERR_CODE(v, m, e) (( (v & 0x000000ff) << 24) | \
                                    ( (m & 0x000000ff) << 16) | \
                                    ( (e & 0x0000ffff) ) )

#define SB_GET_ERROR_VENDOR(e) ( (e & 0xff000000) >> 24)
#define SB_GET_ERROR_MODULE(e) ( (e & 0x00ff0000) >> 16)
#define SB_GET_ERROR_NUM(e) (e & 0x0000ffff)

/* Generic failure code */
#define SB_FAILED -1

/* NOTE: includes placed after definition of shared MACROS */
#include <soc/sbx/sbFeIlibStatus.h>
#include <soc/sbx/sbFabStatus.h>
#include <soc/sbx/fe2k_common/sbFe2000InitStatus.h>

char* sbGetVendorString(int vendor);
char* sbGetModuleString(int module);
char* sbGetErrorString(int error);

#ifdef __cplusplus
}
#endif
#endif
