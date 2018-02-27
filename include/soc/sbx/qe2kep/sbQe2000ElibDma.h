#if !defined(_SBQE2000_ELIB_DMA_H_)
#define _SBQE2000_ELIB_DMA_H_
/* --------------------------------------------------------------------------

$Id: sbQe2000ElibDma.h 1.3.206.1 Broadcom SDK $
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

** --------------------------------------------------------------------------*/
#include "sbTypes.h"

#if !defined(SIM) && defined(SB_LINUX) && !defined(NO_QENIC)
#include <sys/ioctl.h>
#include <linux/sbqeioctl.h>
#include <net/if.h>
#include <errno.h>
#endif

#include "glue.h"
#include "sbQe2000Elib.h"

#if !defined(SIM) && defined(SB_LINUX) && !defined(NO_QENIC)
extern int sbQe2kNicDMA(sbhandle hSbQe, struct sb_qe2000_elib_dma_func_params_s *pDmaArg );
#endif

#if defined(SIM) || defined(NO_QENIC)
extern int sbQe2kDMA(sbhandle hSbQe, struct sb_qe2000_elib_dma_func_params_s *pDmaArg );
#endif

int sbQe2kUserSemCreate(void *pUserSemData);
int sbQe2kUserSemGive(int nSemId, void* pUserData);
int sbQe2kUserSemWaitGet(int nSemId, void* pUserSemData, int nTimeOut);

#endif /* _SBQE2000_ELIB_DMA_H_ */
