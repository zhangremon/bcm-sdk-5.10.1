/*
 * DO NOT EDIT THIS FILE!
 * This file is auto-generated from the registers file.
 * Edits to this file will be lost when it is regenerated.
 *
 * $Id: driver.h 1.98.2.1 Broadcom SDK $
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
 * File:	driver.h
 * Purpose:	Chip driver declarations
 * Supports:	bcm5690_a0
 *		bcm5670_a0
 *		bcm5673_a0
 *		bcm5674_a0
 *		bcm5665_a0
 *		bcm5665_b0
 *		bcm5650_c0
 *		bcm5695_a0
 *		bcm5675_a0
 *		bcm56601_a0
 *		bcm56601_b0
 *		bcm56601_c0
 *		bcm56602_a0
 *		bcm56602_b0
 *		bcm56602_c0
 *		bcm56504_a0
 *		bcm56504_b0
 *		bcm56304_b0
 *		bcm56314_a0
 *		bcm56102_a0
 *		bcm56112_a0
 *		bcm56800_a0
 *		bcm56218_a0
 *		bcm56514_a0
 *		bcm56624_a0
 *		bcm56624_b0
 *		bcm56680_a0
 *		bcm56680_b0
 *		bcm56224_a0
 *		bcm56224_b0
 *		bcm56820_a0
 *		bcm56725_a0
 *		bcm53314_a0
 *		bcm53324_a0
 *		bcm56634_a0
 *		bcm56634_b0
 *		bcm56524_a0
 *		bcm56524_b0
 *		bcm56685_a0
 *		bcm56685_b0
 *		bcm56334_a0
 *		bcm56334_b0
 *		bcm88230_a0
 *		bcm88230_b0
 *		bcm88230_c0
 *		bcm56840_a0
 *		bcm56840_b0
 *		bcm56142_a0
 *		bcm88732_a0
 *		bcm56440_a0
 */

#ifndef _SOC_DRIVER_H
#define _SOC_DRIVER_H


#include <soc/drv.h>
/****************************************************************
 *
 *    Chip driver declarations
 *
 ****************************************************************/

EXTERN soc_driver_t soc_driver_bcm5690_a0;
EXTERN soc_driver_t soc_driver_bcm5670_a0;
EXTERN soc_driver_t soc_driver_bcm5673_a0;
EXTERN soc_driver_t soc_driver_bcm5674_a0;
EXTERN soc_driver_t soc_driver_bcm5665_a0;
EXTERN soc_driver_t soc_driver_bcm5665_b0;
EXTERN soc_driver_t soc_driver_bcm5650_c0;
EXTERN soc_driver_t soc_driver_bcm5695_a0;
EXTERN soc_driver_t soc_driver_bcm5675_a0;
EXTERN soc_driver_t soc_driver_bcm56601_a0;
EXTERN soc_driver_t soc_driver_bcm56601_b0;
EXTERN soc_driver_t soc_driver_bcm56601_c0;
EXTERN soc_driver_t soc_driver_bcm56602_a0;
EXTERN soc_driver_t soc_driver_bcm56602_b0;
EXTERN soc_driver_t soc_driver_bcm56602_c0;
EXTERN soc_driver_t soc_driver_bcm56504_a0;
EXTERN soc_driver_t soc_driver_bcm56504_b0;
EXTERN soc_driver_t soc_driver_bcm56304_b0;
EXTERN soc_driver_t soc_driver_bcm56314_a0;
EXTERN soc_driver_t soc_driver_bcm56102_a0;
EXTERN soc_driver_t soc_driver_bcm56112_a0;
EXTERN soc_driver_t soc_driver_bcm56800_a0;
EXTERN soc_driver_t soc_driver_bcm56218_a0;
EXTERN soc_driver_t soc_driver_bcm56514_a0;
EXTERN soc_driver_t soc_driver_bcm56624_a0;
EXTERN soc_driver_t soc_driver_bcm56624_b0;
EXTERN soc_driver_t soc_driver_bcm56680_a0;
EXTERN soc_driver_t soc_driver_bcm56680_b0;
EXTERN soc_driver_t soc_driver_bcm56224_a0;
EXTERN soc_driver_t soc_driver_bcm56224_b0;
EXTERN soc_driver_t soc_driver_bcm56820_a0;
EXTERN soc_driver_t soc_driver_bcm56725_a0;
EXTERN soc_driver_t soc_driver_bcm53314_a0;
EXTERN soc_driver_t soc_driver_bcm53324_a0;
EXTERN soc_driver_t soc_driver_bcm56634_a0;
EXTERN soc_driver_t soc_driver_bcm56634_b0;
EXTERN soc_driver_t soc_driver_bcm56524_a0;
EXTERN soc_driver_t soc_driver_bcm56524_b0;
EXTERN soc_driver_t soc_driver_bcm56685_a0;
EXTERN soc_driver_t soc_driver_bcm56685_b0;
EXTERN soc_driver_t soc_driver_bcm56334_a0;
EXTERN soc_driver_t soc_driver_bcm56334_b0;
EXTERN soc_driver_t soc_driver_bcm88230_a0;
EXTERN soc_driver_t soc_driver_bcm88230_b0;
EXTERN soc_driver_t soc_driver_bcm88230_c0;
EXTERN soc_driver_t soc_driver_bcm56840_a0;
EXTERN soc_driver_t soc_driver_bcm56840_b0;
EXTERN soc_driver_t soc_driver_bcm56142_a0;
EXTERN soc_driver_t soc_driver_bcm88732_a0;
EXTERN soc_driver_t soc_driver_bcm56440_a0;
/* The table of all known drivers */
/* Indexed by supported chips */
EXTERN soc_driver_t *soc_base_driver_table[SOC_NUM_SUPPORTED_CHIPS];

EXTERN int soc_chip_type_to_index(soc_chip_types chip);
EXTERN soc_chip_types soc_index_to_chip_type(int chip);


#endif	/* !_SOC_DRIVER_H */
