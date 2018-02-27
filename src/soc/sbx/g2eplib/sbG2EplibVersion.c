/**  -*-  Mode:C; c-basic-offset:4 -*-
 *  sbG2EplibVersion.cx
 *
 * <pre>
 *
 * THIS IS A GENERATED FILE - DO NOT EDIT
 *
 *
 * ========================================================================
 * == sbG2EplibVersion.cx - Version Information for G2Eplib ==
 * ========================================================================
 *
 * $Id: sbG2EplibVersion.c 1.2.222.2 Broadcom SDK $
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
 * MODULE NAME:
 *
 *     sbG2EplibVersion.cx
 *
 * ABSTRACT:
 *
 *     Version Information For:  G2Eplib, Non Released Test Software
 *     Sandburst Product Code:   10
 *     Major Version:            0
 *     Minor Version:            0
 *     Patch Level:              999999
 *
 * LANGUAGE:
 *
 *     C
 *
 * CREATION DATE:
 *
 *     Tue May  1 12:09:20 2007
 *
 * </pre>
 */

#include "sbTypes.h"
#include "sbStatus.h"
#include "sbWrappers.h"

static const sbSwLibVersion_t sG2EplibLibraryVersion =
{
    productCode:  10,
    majorVersion: 0,
    minorVersion: 0,
    patchLevel:   999999,
    description:  "Non Released Test Software"
};

/*
 * Declare the prototype
 */
sbStatus_t sbG2EplibVersionGet( sbSwLibVersion_p_t pLibVersion );

/**
 *
 * Retrieve the version information for the G2Eplib Library
 *
 * @param pLibVersion  Pointer to an sbSwLibVersion_t.
 *
 * @return             Status, 0 always.
 */
sbStatus_t sbG2EplibVersionGet( sbSwLibVersion_p_t pLibVersion )
{
    SB_ASSERT(pLibVersion);

    *pLibVersion = sG2EplibLibraryVersion;

    return( 0 );
}
