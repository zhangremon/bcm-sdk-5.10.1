/*
 * $Id: bm3200_mem_access.h 1.3 Broadcom SDK $
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
 * This file contains definitions for BM3200 memory access.
 */

#ifndef _SOC_SBX_BM3200_MEM_ACCESS_H
#define _SOC_SBX_BM3200_MEM_ACCESS_H


#include <soc/sbx/sbTypesGlue.h>
#include <soc/sbx/sbWrappers.h>

#define BM3200_STATUS_OK         0
#define BM3200_STATUS_BAD_UNIT  -1
#define BM3200_STATUS_BAD_READ  -2
#define BM3200_STATUS_BAD_WRITE -3
#define BM3200_STATUS_BAD_FIELD -4


typedef struct bm3200MemConfigRec_s {
      char memname[30];
      int  memindex;
      int  rangemax;
      int  instances;
} bm3200MemConfigRec;

extern void sbBm3200ShowMemNames(void);

/*
 * include other specific structs defined for zframes
 */
#include "sbZfFabBm3200BwBwpEntry.hx"
#include "sbZfFabBm3200BwDstEntry.hx"
#include "sbZfFabBm3200BwLthrEntry.hx"
#include "sbZfFabBm3200BwNPC2QEntry.hx"
#include "sbZfFabBm3200BwPrtEntry.hx"
#include "sbZfFabBm3200BwQ2NPCEntry.hx"
#include "sbZfFabBm3200BwQlopEntry.hx"
#include "sbZfFabBm3200BwQltEntry.hx"
#include "sbZfFabBm3200BwWatEntry.hx"
#include "sbZfFabBm3200BwWctEntry.hx"
#include "sbZfFabBm3200BwWdtEntry.hx"
#include "sbZfFabBm3200BwWstEntry.hx"
#include "sbZfFabBm3200NextPriMemEntry.hx"
#include "sbZfFabBm3200NmEmapEntry.hx"
#include "sbZfFabBm3200PriMemEntry.hx"

enum {
    BM3200_PTBWBWP=0,
    BM3200_PTBWPRT,
    BM3200_PTBWWCT,
    BM3200_PTBWWDT,
    BM3200_PTBWWST,
    BM3200_PTNEXTPRIMEM,
    BM3200_PTNMEMAP,
    BM3200_PTPRIMEM,
    BM3200_MEM_MAX_INDEX,
    /* Cmode only memories, no need to support */
    BM3200_PTBWDST,
    BM3200_PTBWLTHR,
    BM3200_PTBWNPC2Q,
    BM3200_PTBWQ2NPC,
    BM3200_PTBWQLOP,
    BM3200_PTBWQLT,
    BM3200_PTBWWAT,
};

#endif  /* !_SOC_SBX_BM3200_MEM_ACCESS_H */
