/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFabWredTableEntry.hx 1.1.48.4 Broadcom SDK $
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
 */


#ifndef SB_ZF_WRED_TABLE_ENTRY_H
#define SB_ZF_WRED_TABLE_ENTRY_H

#define SB_ZF_WRED_TABLE_ENTRY_SIZE_IN_BYTES 24
#define SB_ZF_WRED_TABLE_ENTRY_SIZE 24
#define SB_ZF_WRED_TABLE_ENTRY_M_NDP0_BITS "191:128"
#define SB_ZF_WRED_TABLE_ENTRY_M_NDP1_BITS "127:64"
#define SB_ZF_WRED_TABLE_ENTRY_M_NDP2_BITS "63:0"


/**

 * Copyright (c) Sandburst, Inc. 2005
 * All Rights Reserved.  Unpublished rights reserved under the copyright
 * laws of the United States.
 *
 * The software contained on this media is proprietary to and embodies the
 * confidential technology of Sandburst, Inc. Possession, use, duplication
 * or dissemination of the software and media is authorized only pursuant
 * to a valid written license from Sandburst, Inc.
 *
 * RESTRICTED RIGHTS LEGEND Use, duplication, or disclosure by the U.S.
 * Government is subject to restrictions as set forth in Subparagraph
 * (c) (1) (ii) of DFARS 252.227-7013, or in FAR 52.227-19, as applicable.
 */
typedef struct _sbZfFabWredTableEntry {
  sbZfFabWredParameters_t m_nDp0;
  sbZfFabWredParameters_t m_nDp1;
  sbZfFabWredParameters_t m_nDp2;
} sbZfFabWredTableEntry_t;

uint32_t
sbZfFabWredTableEntry_Pack(sbZfFabWredTableEntry_t *pFrom,
                           uint8_t *pToData,
                           uint32_t nMaxToDataIndex);
void
sbZfFabWredTableEntry_Unpack(sbZfFabWredTableEntry_t *pToStruct,
                             uint8_t *pFromData,
                             uint32_t nMaxToDataIndex);
void
sbZfFabWredTableEntry_InitInstance(sbZfFabWredTableEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
/* No _SET defined for member, m_nDp0, that is a ZFRAME */
/* No _SET defined for member, m_nDp1, that is a ZFRAME */
/* No _SET defined for member, m_nDp2, that is a ZFRAME */
#else
/* No _SET defined for member, m_nDp0, that is a ZFRAME */
/* No _SET defined for member, m_nDp1, that is a ZFRAME */
/* No _SET defined for member, m_nDp2, that is a ZFRAME */
#endif
#ifdef SAND_BIG_ENDIAN_HOST
/* No _SET defined for member, m_nDp0, that is a ZFRAME */
/* No _SET defined for member, m_nDp1, that is a ZFRAME */
/* No _SET defined for member, m_nDp2, that is a ZFRAME */
#else
/* No _SET defined for member, m_nDp0, that is a ZFRAME */
/* No _SET defined for member, m_nDp1, that is a ZFRAME */
/* No _SET defined for member, m_nDp2, that is a ZFRAME */
#endif
#ifdef SAND_BIG_ENDIAN_HOST
/* No _GET defined for member, m_nDp0, that is a ZFRAME */
/* No _GET defined for member, m_nDp1, that is a ZFRAME */
/* No _GET defined for member, m_nDp2, that is a ZFRAME */
#else
/* No _GET defined for member, m_nDp0, that is a ZFRAME */
/* No _GET defined for member, m_nDp1, that is a ZFRAME */
/* No _GET defined for member, m_nDp2, that is a ZFRAME */
#endif
#ifdef SAND_BIG_ENDIAN_HOST
/* No _GET defined for member, m_nDp0, that is a ZFRAME */
/* No _GET defined for member, m_nDp1, that is a ZFRAME */
/* No _GET defined for member, m_nDp2, that is a ZFRAME */
#else
/* No _GET defined for member, m_nDp0, that is a ZFRAME */
/* No _GET defined for member, m_nDp1, that is a ZFRAME */
/* No _GET defined for member, m_nDp2, that is a ZFRAME */
#endif
#endif
/*
 * $Id: sbZfFabWredTableEntry.hx,v 1.1.48.4 2011/05/22 05:37:54 iakramov Exp $
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
 */



#ifdef SB_ZF_INCLUDE_CONSOLE
#ifndef SB_ZF_WRED_TABLE_ENTRY_CONSOLE_H
#define SB_ZF_WRED_TABLE_ENTRY_CONSOLE_H



void
sbZfFabWredTableEntry_Print(sbZfFabWredTableEntry_t *pFromStruct);
int
sbZfFabWredTableEntry_SPrint(sbZfFabWredTableEntry_t *pFromStruct, char *pcToString, uint32_t lStrSize);
int
sbZfFabWredTableEntry_Validate(sbZfFabWredTableEntry_t *pZf);
int
sbZfFabWredTableEntry_SetField(sbZfFabWredTableEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_WRED_TABLE_ENTRY_CONSOLE_H */
#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */
