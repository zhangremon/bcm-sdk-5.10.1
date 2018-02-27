/*
 * DO NOT EDIT THIS FILE!
 * This file is auto-generated from the registers file.
 * Edits to this file will be lost when it is regenerated.
 *
 * $Id: chip.c 1.24 Broadcom SDK $
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
 * File:	chip.c
 * Purpose:	Chip driver table defined
 * Supports:	bcm5324_a0
 *		bcm5324_a1
 *		bcm5396_a0
 *		bcm5389_a0
 *		bcm5398_a0
 *		bcm5397_a0
 *		bcm5348_a0
 *		bcm5347_a0
 *		bcm5395_a0
 *		bcm53242_a0
 *		bcm53262_a0
 *		bcm53115_a0
 *		bcm53118_a0
 *		bcm53280_a0
 *		bcm53280_b0
 *		bcm53101_a0
 *		bcm53125_a0
 *		bcm53128_a0
 *		bcm53600_a0
 */


#include <soc/defs.h>
#include <soc/mcm/robo/driver.h>

/*
 * A null driver to indicate a given chip is not supported.
 *
 * This assumes that the chip is identified by device/rev id
 * and this will be 0 for the null driver, thus not match.
 */
#if !defined(BCM_ALL_CHIPS)
static soc_driver_t soc_driver_none;
#endif

/*
 * The table of all known drivers
 * Indexed by supported chips
 */
soc_driver_t *soc_robo_base_driver_table[] = {


#if defined(BCM_5324_A0)
    &soc_driver_bcm5324_a0,
#else
    &soc_driver_none,
#endif

#if defined(BCM_5324_A1)
    &soc_driver_bcm5324_a1,
#else
    &soc_driver_none,
#endif

#if defined(BCM_5396_A0)
    &soc_driver_bcm5396_a0,
#else
    &soc_driver_none,
#endif

#if defined(BCM_5389_A0)
    &soc_driver_bcm5389_a0,
#else
    &soc_driver_none,
#endif

#if defined(BCM_5398_A0)
    &soc_driver_bcm5398_a0,
#else
    &soc_driver_none,
#endif

#if defined(BCM_5397_A0)
    &soc_driver_bcm5397_a0,
#else
    &soc_driver_none,
#endif

#if defined(BCM_5348_A0)
    &soc_driver_bcm5348_a0,
#else
    &soc_driver_none,
#endif

#if defined(BCM_5347_A0)
    &soc_driver_bcm5347_a0,
#else
    &soc_driver_none,
#endif

#if defined(BCM_5395_A0)
    &soc_driver_bcm5395_a0,
#else
    &soc_driver_none,
#endif

#if defined(BCM_53242_A0)
    &soc_driver_bcm53242_a0,
#else
    &soc_driver_none,
#endif

#if defined(BCM_53262_A0)
    &soc_driver_bcm53262_a0,
#else
    &soc_driver_none,
#endif

#if defined(BCM_53115_A0)
    &soc_driver_bcm53115_a0,
#else
    &soc_driver_none,
#endif

#if defined(BCM_53118_A0)
    &soc_driver_bcm53118_a0,
#else
    &soc_driver_none,
#endif

#if defined(BCM_53280_A0)
    &soc_driver_bcm53280_a0,
#else
    &soc_driver_none,
#endif

#if defined(BCM_53280_B0)
    &soc_driver_bcm53280_b0,
#else
    &soc_driver_none,
#endif

#if defined(BCM_53101_A0)
    &soc_driver_bcm53101_a0,
#else
    &soc_driver_none,
#endif

#if defined(BCM_53125_A0)
    &soc_driver_bcm53125_a0,
#else
    &soc_driver_none,
#endif

#if defined(BCM_53128_A0)
    &soc_driver_bcm53128_a0,
#else
    &soc_driver_none,
#endif

#if defined(BCM_53600_A0)
    &soc_driver_bcm53600_a0,
#else
    &soc_driver_none,
#endif

};

/* Function to translate SOC_CHIP_ to supported idx*/
int soc_robo_chip_type_to_index(soc_chip_types chip)
{
    switch(chip) {
    case SOC_CHIP_BCM5324_A0:  return 0;
    case SOC_CHIP_BCM5324_A1:  return 1;
    case SOC_CHIP_BCM5396_A0:  return 2;
    case SOC_CHIP_BCM5389_A0:  return 3;
    case SOC_CHIP_BCM5398_A0:  return 4;
    case SOC_CHIP_BCM5397_A0:  return 5;
    case SOC_CHIP_BCM5348_A0:  return 6;
    case SOC_CHIP_BCM5347_A0:  return 7;
    case SOC_CHIP_BCM5395_A0:  return 8;
    case SOC_CHIP_BCM53242_A0:  return 9;
    case SOC_CHIP_BCM53262_A0:  return 10;
    case SOC_CHIP_BCM53115_A0:  return 11;
    case SOC_CHIP_BCM53118_A0:  return 12;
    case SOC_CHIP_BCM53280_A0:  return 13;
    case SOC_CHIP_BCM53280_B0:  return 14;
    case SOC_CHIP_BCM53101_A0:  return 15;
    case SOC_CHIP_BCM53125_A0:  return 16;
    case SOC_CHIP_BCM53128_A0:  return 17;
    case SOC_CHIP_BCM53600_A0:  return 18;
    default:  assert(0);
    }
    return -1;
}

/* Reverse translation of above. */
soc_chip_types soc_robo_index_to_chip_type(int chip)
{
    switch(chip) {
    case 0: return SOC_CHIP_BCM5324_A0;
    case 1: return SOC_CHIP_BCM5324_A1;
    case 2: return SOC_CHIP_BCM5396_A0;
    case 3: return SOC_CHIP_BCM5389_A0;
    case 4: return SOC_CHIP_BCM5398_A0;
    case 5: return SOC_CHIP_BCM5397_A0;
    case 6: return SOC_CHIP_BCM5348_A0;
    case 7: return SOC_CHIP_BCM5347_A0;
    case 8: return SOC_CHIP_BCM5395_A0;
    case 9: return SOC_CHIP_BCM53242_A0;
    case 10: return SOC_CHIP_BCM53262_A0;
    case 11: return SOC_CHIP_BCM53115_A0;
    case 12: return SOC_CHIP_BCM53118_A0;
    case 13: return SOC_CHIP_BCM53280_A0;
    case 14: return SOC_CHIP_BCM53280_B0;
    case 15: return SOC_CHIP_BCM53101_A0;
    case 16: return SOC_CHIP_BCM53125_A0;
    case 17: return SOC_CHIP_BCM53128_A0;
    case 18: return SOC_CHIP_BCM53600_A0;
    default:  assert(0);
    }
    return -1;
}