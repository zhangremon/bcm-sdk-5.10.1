/*
 * $Id: phy84740.c 1.30.2.11 Broadcom SDK $
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
 * File:        phy84740.c
 * Purpose:    Phys Driver support for Broadcom 84740 40Gig
 *             transceiver :XLAUI system side and PPI Line interface.
 * Note:       
 */

#include <sal/types.h>

#include <soc/drv.h>
#include <soc/debug.h>
#include <soc/error.h>
#include <soc/phyreg.h>

#include <soc/phy.h>
#include <soc/phy/phyctrl.h>
#include <soc/phy/drv.h>

#include "phydefs.h"      /* Must include before other phy related includes */

#if defined(INCLUDE_PHY_84740)
#include "phyconfig.h"    /* Must be the first phy include after phydefs.h */
#include "phyident.h"
#include "phyreg.h"
#include "phynull.h"
#include "phyxehg.h"
#include "phy84740.h"
/*
 * SPI-ROM Program related defs
 */
#define SPI_CTRL_1_L        0xC000
#define SPI_CTRL_1_H        0xC002
#define SPI_CTRL_2_L        0xC400
#define SPI_CTRL_2_H        0xC402
#define SPI_TXFIFO          0xD000
#define SPI_RXFIFO          0xD400
#define WR_CPU_CTRL_REGS    0x11
#define RD_CPU_CTRL_REGS    0xEE
#define WR_CPU_CTRL_FIFO    0x66

/*
 * SPI Controller Commands(Messages).
 */
#define MSGTYPE_HWR          0x40
#define MSGTYPE_HRD          0x80
#define WRSR_OPCODE          0x01
#define WR_OPCODE            0x02
#define WRDI_OPCODE          0x04
#define RDSR_OPCODE          0x05
#define WREN_OPCODE          0x06
#define WR_BLOCK_SIZE        0x40
#define TOTAL_WR_BYTE        0x4000
                                                                                
#define WR_TIMEOUT   1000000

/* CL73 autoneg */
#define AN_ADVERT_40GCR4 (1 << 9)
#define AN_ADVERT_40GKR4 (1 << 8)
#define AN_ADVERT_10GKR  (1 << 7)
#define AN_ADVERT_1GKX   (1 << 5)

/* PRBS */
#define PHY84740_FORTYG_PRBS_PATTERN_TESTING_CONTROL_STATUS         0x0135
#define PHY84740_40G_PRBS31             (1 << 7)               
#define PHY84740_40G_PRBS9              (1 << 6) 
#define PHY84740_40G_PRBS_TX_ENABLE     (1 << 3) 
#define PHY84740_40G_PRBS_RX_ENABLE     (1 << 0) 
#define PHY84740_40G_PRBS_ENABLE     (PHY84740_40G_PRBS_RX_ENABLE | \
	PHY84740_40G_PRBS_TX_ENABLE)

#define PHY84740_FORTYG_PRBS_RECEIVE_ERROR_COUNTER_LANE0            0x0140 
#define PHY84740_USER_PRBS_CONTROL_0_REGISTER                       0xCD14 
#define PHY84740_USER_PRBS_TX_INVERT   (1 << 4)
#define PHY84740_USER_PRBS_RX_INVERT   (1 << 15)
#define PHY84740_USER_PRBS_INVERT      (1 << 4 | 1 << 15)
#define PHY84740_USER_PRBS_ENABLE      (1 << 7)
#define PHY84740_USER_PRBS_TYPE_MASK   (0x7)

#define PHY84740_USER_PRBS_STATUS_0_REGISTER                        0xCD15 
#define PHY84740_GENSIG_8071_REGISTER                               0xCD16 
#define PHY84740_RESET_CONTROL_REGISTER                             0xCD17 

/* Polarity config */
#define POL_CONFIG_LANE_WIDTH     0x4
#define POL_CONFIG_LANE_MASK(_ln)   (0xf << ((_ln) * POL_CONFIG_LANE_WIDTH))
#define POL_CONFIG_ALL_LANES    1 

/* I2C related defines */
#define PHY84740_BSC_XFER_MAX     0x1F9
#define PHY84740_BSC_WR_MAX       16
#define PHY84740_WRITE_START_ADDR 0x8007
#define PHY84740_READ_START_ADDR  0x8007
#define PHY84740_WR_FREQ_400KHZ   0x0100
#define PHY84740_2W_STAT          0x000C
#define PHY84740_2W_STAT_IDLE     0x0000
#define PHY84740_2W_STAT_COMPLETE 0x0004
#define PHY84740_2W_STAT_IN_PRG   0x0008
#define PHY84740_2W_STAT_FAIL     0x000C
#define PHY84740_BSC_WRITE_OP     0x22
#define PHY84740_BSC_READ_OP      0x2
#define PHY84740_I2CDEV_WRITE     0x1
#define PHY84740_I2CDEV_READ      0x0
#define PHY84740_I2C_8BIT         0
#define PHY84740_I2C_16BIT        1
#define PHY84740_I2C_TEMP_RAM     0xE
#define PHY84740_I2C_OP_TYPE(access_type,data_type) \
        ((access_type) | ((data_type) << 8))
#define PHY84740_I2C_ACCESS_TYPE(op_type) ((op_type) & 0xff)
#define PHY84740_I2C_DATA_TYPE(op_type)   (((op_type) >> 8) & 0xff)

/* preemphasis control */
#define PHY84740_PREEMPH_CTRL_FORCE_SHFT       15
#define PHY84740_PREEMPH_GET_FORCE(_val)  \
	(((_val) >> PHY84740_PREEMPH_CTRL_FORCE_SHFT) & 1)
#define PHY84740_PREEMPH_CTRL_POST_TAP_SHFT    10
#define PHY84740_PREEMPH_GET_POST_TAP(_val) \
	(((_val) >> PHY84740_PREEMPH_CTRL_POST_TAP_SHFT) & 0x1f)
#define PHY84740_PREEMPH_CTRL_MAIN_TAP_SHFT    4
#define PHY84740_PREEMPH_GET_MAIN_TAP(_val) \
	(((_val) >> PHY84740_PREEMPH_CTRL_MAIN_TAP_SHFT) & 0x3f) 
#define PHY84740_PREEMPH_REG_FORCE_MASK     (1 << 15)
#define PHY84740_PREEMPH_REG_POST_TAP_SHFT  4
#define PHY84740_PREEMPH_REG_POST_TAP_MASK  (0xf << PHY84740_PREEMPH_REG_POST_TAP_SHFT)
#define PHY84740_PREEMPH_REG_MAIN_TAP_SHFT  11   
#define PHY84740_PREEMPH_REG_MAIN_TAP_MASK  (0x1f << PHY84740_PREEMPH_REG_MAIN_TAP_SHFT)
#define PHY84740_PREEMPH_REG_TX_PWRDN_MASK  (0x1 << 10)

/* Misc. */
#define PHY84740_UCODE_DEVID_ADDR              0x7FF9

#define PHY84740_SINGLE_PORT_MODE(_pc)  ((_pc)->speed_max >= 40000)
#define PHY84740_EDC_MODE_MASK          0xff
#define PHY84740_ID_84740 0x84740
#define PHY84740_ID_84064 0x84064
#define PHY84740_ID_84754 0x84754
#define PHY84740_ID_84753 0x84753

#define NUM_LANES  4

/* typedefs */
typedef struct {
    int rx_los;
    int rx_los_invert;
    int mod_abs;
    int mod_abs_invert;
    int p2l_map[NUM_LANES];  /* index: physical lane, array element: logical lane */
    int pcs_repeater;
    int reverse_mode;
} PHY84740_DEV_DESC_t; 

/* device field macros */
#define RX_LOS(_pc) (((PHY84740_DEV_DESC_t *)((_pc) + 1))->rx_los)
#define RX_LOS_INVERT(_pc) (((PHY84740_DEV_DESC_t *)((_pc) + 1))->rx_los_invert)
#define MOD_ABS(_pc) (((PHY84740_DEV_DESC_t *)((_pc) + 1))->mod_abs)
#define MOD_ABS_INVERT(_pc) (((PHY84740_DEV_DESC_t *)((_pc) + 1))->mod_abs_invert)
#define P2L_MAP(_pc,_ix) (((PHY84740_DEV_DESC_t *)((_pc) + 1))->p2l_map[(_ix)])
#define PCS_REPEATER(_pc) (((PHY84740_DEV_DESC_t *)((_pc) + 1))->pcs_repeater)
#define REVERSE_MODE(_pc) (((PHY84740_DEV_DESC_t *)((_pc) + 1))->reverse_mode)

/* callback to deliever the firmware by application 
 * return value: TRUE  successfully delivered
 *               FALSE fail to deliver
 *
 * Example:
 *
 * int user_firmware_send (int unit,int port,int flag,unsigned char **data,int *len)
 * {
 *     *data = phy84740_ucode_bin;
 *     *len  = phy84740_ucode_bin_len;
 *     soc_cm_debug(DK_WARN,"port %d: user firmware hook routine called\n",port);
 *     return TRUE;
 * }
 * Then initialize the function pointer before the BCM port initialization: 
 * phy_84740_uc_firmware_hook = user_firmware_send;
 *
 */
int (*phy_84740_uc_firmware_hook)(
       int unit, /* switch number */
       int port, /* port number */
       int flag, /* any information need to ba passed. Not used for now*/
       unsigned char **addr,  /* starting address of delivered firmware */
       int *len               /* length of the firmware */
       ) = NULL;          

static int write_message(int unit, phy_ctrl_t *pc, uint16 wrdata, 
                         uint16 *rddata);
                                                                                

STATIC int phy_84740_speed_get(int, soc_port_t, int *);
STATIC int
_phy_84740_firmware_to_rom(int unit, int port, int offset, uint8 *array,int datalen);
STATIC int
_phy84740_mdio_lane_firmware_download(int unit, int port, phy_ctrl_t *pc,
                                uint8 *new_fw, uint32 fw_length);

extern unsigned char phy84740_ucode_bin[];
extern unsigned int phy84740_ucode_bin_len;
extern unsigned char phy84754_ucode_bin[];
extern unsigned int phy84754_ucode_bin_len;
extern unsigned char phy84753_ucode_bin[];
extern unsigned int phy84753_ucode_bin_len;
extern unsigned char phy84064_ucode_bin[];
extern unsigned int phy84064_ucode_bin_len;

static char *dev_name_84740 = "BCM84740";
static char *dev_name_84753 = "BCM84753";
static char *dev_name_84754 = "BCM84754";
static char *dev_name_84064 = "BCM84064";

STATIC int
_phy_84740_config_devid(int unit,soc_port_t port, uint32 *devid)
{
    if (soc_property_port_get(unit, port,spn_PHY_84740, FALSE)) {
        *devid = PHY84740_ID_84740;
    } else if (soc_property_port_get(unit, port,spn_PHY_84753, FALSE)) {
        *devid = PHY84740_ID_84753;
    } else if (soc_property_port_get(unit, port,spn_PHY_84754, FALSE)) {
        *devid = PHY84740_ID_84754;
    } else if (soc_property_port_get(unit, port,spn_PHY_84064, FALSE)) {
        *devid = PHY84740_ID_84064;
    } else {
        *devid = 0;
    }
    return SOC_E_NONE;
}

STATIC int
_phy_84740_single_to_quad_mode(int unit,soc_port_t port, phy_ctrl_t *pc)
{
    int            ix;
    uint16         saved_phy_addr; 
    uint16 mask16;
    uint16 data16;
    uint32 devid;

    if (pc == NULL) {
        pc = EXT_PHY_SW_STATE(unit, port);
    }

    /* probe routine should block any invalid devids. 
     * devids should be always valid here
     */
    SOC_IF_ERROR_RETURN(
        _phy_84740_config_devid(pc->unit,pc->port, &devid));

    mask16 = PHY84740_PMAD_DAC_MODE_MASK | PHY84740_PMAD_BKPLANE_MODE_MASK;
    if (devid == PHY84740_ID_84064) {
        data16 = PHY84740_PMAD_BKPLANE_MODE_MASK;
    } else {
        data16 = 0;
    }

    /* config DAC mode/backplane mode first. This is a bcst register in single mode */
    SOC_IF_ERROR_RETURN(
        MODIFY_PHY84740_MMF_PMA_PMD_REG(pc->unit, pc, PHY84740_PMAD_CHIP_MODE_REG,
              data16, mask16));

    /* then configure quad chip mode */
    SOC_IF_ERROR_RETURN(
        MODIFY_PHY84740_MMF_PMA_PMD_REG(pc->unit, pc, PHY84740_PMAD_CHIP_MODE_REG,
              0, PHY84740_PMAD_CHIP_MODE_MASK));

    /* once this mode register is configured with quad mode, each lane is
     * accessable by the lane specific MDIO address
     */
    saved_phy_addr = pc->phy_id;
    /* Reset all 4 lanes. Only after reset, the mode is actually switched */
    for (ix = 0; ix < 4; ix++) {
        pc->phy_id = (saved_phy_addr & (~0x3)) + ix;
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY84740_PMA_PMD_CTRLr(pc->unit, pc,
                              MII_CTRL_RESET, MII_CTRL_RESET));
    }
    pc->phy_id = saved_phy_addr;

    return SOC_E_NONE;
}

STATIC int
_phy_84740_quad_to_single_mode(int unit,soc_port_t port)
{
    int            ix;
    uint16         saved_phy_addr;
    phy_ctrl_t     *pc;
    uint16 mask16;
    uint16 data16;
    uint32 devid;

    pc = EXT_PHY_SW_STATE(unit, port);

    SOC_IF_ERROR_RETURN(
        _phy_84740_config_devid(pc->unit,pc->port, &devid));

    saved_phy_addr = pc->phy_id;
    /* configure all four lanes to bcst mode */
    for (ix = 0; ix < 4; ix++) {
        pc->phy_id = (saved_phy_addr & (~0x3)) + ix;
        SOC_IF_ERROR_RETURN
            (WRITE_PHY84740_MMF_PMA_PMD_REG(pc->unit, pc,
                              PHY84740_PMAD_BCST_REG, 0xffff));
    }
    pc->phy_id = saved_phy_addr;

    /* then configure single mode */
    SOC_IF_ERROR_RETURN(
        MODIFY_PHY84740_MMF_PMA_PMD_REG(pc->unit, pc, PHY84740_PMAD_CHIP_MODE_REG,
              PHY84740_PMAD_MODE_40G, PHY84740_PMAD_CHIP_MODE_MASK));

    mask16 = PHY84740_PMAD_DAC_MODE_MASK | PHY84740_PMAD_BKPLANE_MODE_MASK;
    if (devid == PHY84740_ID_84064) {
        data16 = PHY84740_PMAD_BKPLANE_MODE_MASK;
    } else {
        data16 = 0;
    }

    /* config DAC mode and backplane mode for LR4/SR4 */
    SOC_IF_ERROR_RETURN(
        MODIFY_PHY84740_MMF_PMA_PMD_REG(pc->unit, pc, PHY84740_PMAD_CHIP_MODE_REG,
              data16, mask16));

    /* do a soft reset to switch to the configured mode
     * The reset should also clear the bcst register configuration
     */
    SOC_IF_ERROR_RETURN
        (MODIFY_PHY84740_PMA_PMD_CTRLr(pc->unit, pc,
                          MII_CTRL_RESET, MII_CTRL_RESET));

    return SOC_E_NONE;
}
STATIC int
_phy84740_mdio_firmware_download(int unit, int port, phy_ctrl_t *pc,
                                uint8 *new_fw, uint32 fw_length)
{
    uint16 data16;
    uint16 mask16;
    int lane;
    int rv;

    if (new_fw == NULL || fw_length == 0) {
        SOC_DEBUG_PRINT((DK_WARN, "u=%d p=%d MDIO firmware download: invalid firmware\n",
                         unit, port));
        return SOC_E_NONE;
    }

    if (PHY84740_SINGLE_PORT_MODE(pc)) {
        /* point to lane 0 */
        SOC_IF_ERROR_RETURN
            (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc, PHY84740_PMAD_AER_ADDR_REG, 
                                        0));
    }

    /* clear SPA ctrl reg bit 15 and bit 13. 
     * bit 15, 0-use MDIO download to SRAM, 1 SPI-ROM download to SRAM 
     * bit 13, 0 clear download done status, 1 skip download 
     */
    mask16 = (1 << 13) | (1 << 15);
    data16 = 0;
    SOC_IF_ERROR_RETURN
        (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit, pc, PHY84740_PMAD_SPA_CTRL_REG, 
                                        data16,mask16));

    /* set SPA ctrl reg bit 14, 1 RAM boot, 0 internal ROM boot */
    mask16 = 1 << 14;
    data16 = mask16;
    SOC_IF_ERROR_RETURN
        (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit, pc, PHY84740_PMAD_SPA_CTRL_REG, 
                                        data16,mask16));

    /* misc_ctrl1 reg bit 3 to 1 for 32k downloading size */
    if (PHY84740_SINGLE_PORT_MODE(pc)) {
        for (lane = 0; lane < NUM_LANES; lane++) {
            SOC_IF_ERROR_RETURN
            (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc, PHY84740_PMAD_AER_ADDR_REG, lane));
            mask16 = 1 << 3;
            data16 = mask16;
            SOC_IF_ERROR_RETURN
                (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit, pc, PHY84740_PMAD_MISC_CTRL1_REG, 
                                           data16,mask16));
        }
    } else {
        mask16 = 1 << 3;
        data16 = mask16;
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit, pc, PHY84740_PMAD_MISC_CTRL1_REG, 
                                           data16,mask16));
    }
    if (PHY84740_SINGLE_PORT_MODE(pc)) {
        /* apply bcst Reset to start download code from MDIO */
        SOC_IF_ERROR_RETURN
            (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc, PHY84740_PMAD_AER_ADDR_REG, 0));
    }
    SOC_IF_ERROR_RETURN
        (WRITE_PHY84740_PMA_PMD_CTRLr(unit, pc, 0x8000));

    /* wait for 2ms for M8051 start and 5ms to initialize the RAM */
    sal_usleep(10000); /* Wait for 10ms */


    if (PHY84740_SINGLE_PORT_MODE(pc)) {
        for (lane = 0; lane < NUM_LANES; lane++) {
            SOC_IF_ERROR_RETURN
            (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc, PHY84740_PMAD_AER_ADDR_REG, lane));
            /* download the firmware to the specific lane */
            rv = _phy84740_mdio_lane_firmware_download(unit,port,pc,new_fw,fw_length);
        }

        /* point back to lane0 */
        SOC_IF_ERROR_RETURN
            (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc, PHY84740_PMAD_AER_ADDR_REG, 0));
    } else {
        SOC_IF_ERROR_RETURN
            (_phy84740_mdio_lane_firmware_download(unit,port,pc,new_fw,fw_length));
    }
    return SOC_E_NONE;
}

STATIC int
_phy84740_mdio_lane_firmware_download(int unit, int port, phy_ctrl_t *pc,
                                uint8 *new_fw, uint32 fw_length)

{
    uint16 data16;
    int j;
    uint16 num_words;


    /* Write Starting Address, where the Code will reside in SRAM */
    data16 = 0x8000;
    SOC_IF_ERROR_RETURN
        (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc, PHY84740_PMAD_M8051_MSGIN_REG, data16));

    /* make sure address word is read by the micro */
    sal_udelay(10); /* Wait for 10us */

    /* Write SPI SRAM Count Size */
    data16 = (fw_length)/2;
    SOC_IF_ERROR_RETURN
        (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc, PHY84740_PMAD_M8051_MSGIN_REG, data16));

    /* make sure read by the micro */
    sal_udelay(10); /* Wait for 10us */

    /* Fill in the SRAM */
    num_words = (fw_length - 1);
    for (j = 0; j < num_words; j+=2) {
        /* Make sure the word is read by the Micro */
        sal_udelay(10);

        data16 = (new_fw[j] << 8) | new_fw[j+1];
        SOC_IF_ERROR_RETURN
            (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc, PHY84740_PMAD_M8051_MSGIN_REG, 
                 data16));
    }

    /* make sure last code word is read by the micro */
    sal_udelay(20);

    /* Read Hand-Shake message (Done) from Micro */
    SOC_IF_ERROR_RETURN
        (READ_PHY84740_MMF_PMA_PMD_REG(unit, pc,PHY84740_PMAD_M8051_MSGOUT_REG, &data16));

    /* Download done message */
    SOC_DEBUG_PRINT((DK_PHY, "u=%d p=%d MDIO firmware download done message: 0x%x\n",
                         unit, port,data16));

    /* Clear LASI status */
    SOC_IF_ERROR_RETURN
        (READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0x9003, &data16));

    /* Wait for LASI to be asserted when M8051 writes checksum to MSG_OUTr */
    sal_udelay(100); /* Wait for 100 usecs */

    SOC_IF_ERROR_RETURN
        (READ_PHY84740_MMF_PMA_PMD_REG(unit, pc,PHY84740_PMAD_M8051_MSGOUT_REG, &data16));

    /* Need to check if checksum is correct */
    SOC_IF_ERROR_RETURN
        (READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0xCA1C, &data16));

    if (data16 != 0x600D) {
        /* Bad CHECKSUM */
        soc_cm_print("MDIO Firmware downlad failure:"
                     "Incorrect Checksum %x\n", data16);
        return SOC_E_FAIL;
    }

    /* read Rev-ID */
    SOC_IF_ERROR_RETURN
        (READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0xCA1A, &data16));
    SOC_DEBUG_PRINT((DK_PHY, "u=%d p=%d MDIO Firmware download revID: 0x%x\n",
                         unit, port,data16));

    return SOC_E_NONE;
}

STATIC int
_phy84740_rom_firmware_download(int unit, int port, phy_ctrl_t *pc)
{
    int lane;
    uint16 data16;
    uint16 mask16;
    if (PHY84740_SINGLE_PORT_MODE(pc)) {
        for (lane = 3; lane >= 0; lane--) {
            SOC_IF_ERROR_RETURN
                (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc, 
                                           PHY84740_PMAD_AER_ADDR_REG, lane));

            /* 0xca85[3]=1, 32K download */ 
            mask16 = (1 << 3);
            data16 = mask16;
            SOC_IF_ERROR_RETURN
                (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0xCA85, data16,mask16));
        }

        /* 0xc848[15]=1, SPI-ROM downloading to RAM, 0xc848[14]=1, serial boot */
        mask16 = (1 << 15) | (1 << 14);
        data16 = mask16;

        /* 0xc848[13]=0, SPI-ROM downloading not done, 0xc848[2]=0, spi port enable */
        mask16 |= (1 << 13) | (1 << 2);
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0xc848, data16, mask16));

        /* apply software reset to download code from SPI-ROM */
        SOC_IF_ERROR_RETURN
            (WRITE_PHY84740_PMA_PMD_CTRLr(unit, pc, MII_CTRL_RESET));

        for (lane = 0; lane < NUM_LANES; lane++) {
            SOC_IF_ERROR_RETURN
                (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc, 
                                           PHY84740_PMAD_AER_ADDR_REG, lane));
            /* wait for at least 50ms */
            sal_usleep(100000); 
            SOC_IF_ERROR_RETURN
                (READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0xCA13, &data16));
            SOC_DEBUG_PRINT((DK_PHY, "u=%d p=%d lane%d SPI-ROM download done msg 0x%x\n",
                         unit, port,lane,data16));
            SOC_IF_ERROR_RETURN
                (READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0xCA1C, &data16));
            SOC_IF_ERROR_RETURN
                (READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0xCA1A, &data16));
        }
        SOC_IF_ERROR_RETURN
            (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc, 
                                           PHY84740_PMAD_AER_ADDR_REG, 0));
    } else { /* single lane */
        /* 0xca85[3]=1, 32K download */
        mask16 = (1 << 3);
        data16 = mask16;
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0xCA85, data16,mask16));

        /* 0xc848[15]=1, SPI-ROM downloading to RAM, 0xc848[14]=1, serial boot */
        mask16 = (1 << 15) | (1 << 14);
        data16 = mask16;
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0xc848, data16, mask16));

        /* 0xc848[13]=0, SPI-ROM downloading not done, 0xc848[2]=0, spi port enable */
        mask16 = (1 << 13) | (1 << 2);
        data16 = 0;
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0xc848, data16, mask16));

        /* apply software reset to download code from SPI-ROM */
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY84740_PMA_PMD_CTRLr(unit, pc,
                                  MII_CTRL_RESET, MII_CTRL_RESET));

        /* wait for at least 50ms */
        sal_usleep(100000);
        SOC_IF_ERROR_RETURN
            (READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0xCA13, &data16));
        SOC_DEBUG_PRINT((DK_PHY, "u=%d p=%d SPI-ROM download done msg 0x%x\n",
                   unit, port,data16));

    }
    return SOC_E_NONE;
}

/*
 * Flip the tx/rx polarities based on the configuration.
 */ 
STATIC int
_phy_84740_polarity_flip_set(int unit,soc_port_t port)
{
    int            lane;
    phy_ctrl_t     *pc;
    uint16 tx_pol,rx_pol;
    uint16 data16,mask16;

    pc = EXT_PHY_SW_STATE(unit, port);


    tx_pol = 0;
    rx_pol = 0;

    /* get the reset default value. The 84740 device has the TX polarity flipped
     * by default. Construct the init default value 
     */

    if ((PHY84740_SINGLE_PORT_MODE(pc))) {
        for (lane = 0; lane < NUM_LANES; lane++) { 
            /* point to the each lane starting at lane0 */
            SOC_IF_ERROR_RETURN
                (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc,
                                PHY84740_PMAD_AER_ADDR_REG, P2L_MAP(pc,lane)));

            /* read both TX/RX polarities */
            SOC_IF_ERROR_RETURN(
                READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, PHY84740_DIG_CTRL_REG,
                           &data16));
            if (data16 & PHY84740_TX_POLARITY_FLIP_MASK) {
                tx_pol |= POL_CONFIG_LANE_MASK(lane); 
            } 
            if (data16 & PHY84740_RX_POLARITY_FLIP_MASK) {
                rx_pol |= POL_CONFIG_LANE_MASK(lane); 
            } 
        }
    } else {  /* quad mode */
        SOC_IF_ERROR_RETURN(
            READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, PHY84740_DIG_CTRL_REG,
                            &data16));
        if (data16 & PHY84740_TX_POLARITY_FLIP_MASK) {
            tx_pol = POL_CONFIG_ALL_LANES;
        } 
        if (data16 & PHY84740_RX_POLARITY_FLIP_MASK) {
            rx_pol = POL_CONFIG_ALL_LANES;
        } 
    }

    tx_pol = soc_property_port_get(unit, port,
                                    spn_PHY_PCS_TX_POLARITY_FLIP, tx_pol);
    rx_pol = soc_property_port_get(unit, port,
                                    spn_PHY_PCS_RX_POLARITY_FLIP, rx_pol);

    mask16 = PHY84740_TX_POLARITY_FLIP_MASK | 
             PHY84740_RX_POLARITY_FLIP_MASK;

    if ((PHY84740_SINGLE_PORT_MODE(pc))) {
        for (lane = 0; lane < NUM_LANES; lane++) { 
            data16 = 0;

            /* point to the each lane starting at lane0 */
            SOC_IF_ERROR_RETURN
                (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc,
                            PHY84740_PMAD_AER_ADDR_REG, P2L_MAP(pc,lane)));

            if ((tx_pol == POL_CONFIG_ALL_LANES) || 
                ((tx_pol & POL_CONFIG_LANE_MASK(lane)) ==
                POL_CONFIG_LANE_MASK(lane)) ) {  
                data16 = PHY84740_TX_POLARITY_FLIP_MASK;
            }
            if ((rx_pol == POL_CONFIG_ALL_LANES) || 
                ((rx_pol & POL_CONFIG_LANE_MASK(lane)) ==
                POL_CONFIG_LANE_MASK(lane)) ) {  
                data16 |= PHY84740_RX_POLARITY_FLIP_MASK;
            }
            SOC_IF_ERROR_RETURN(
                MODIFY_PHY84740_MMF_PMA_PMD_REG(unit,pc, 
                                     PHY84740_DIG_CTRL_REG,
                                     data16,mask16));
        }
    } else {  /* quad-port mode */
        data16 = 0;
        if (tx_pol) {
            data16 = PHY84740_TX_POLARITY_FLIP_MASK;
        }
        if (rx_pol) {
            data16 |= PHY84740_RX_POLARITY_FLIP_MASK;
        }
        SOC_IF_ERROR_RETURN(
            MODIFY_PHY84740_MMF_PMA_PMD_REG(unit,pc, PHY84740_DIG_CTRL_REG,
                                 data16,mask16));
    }
   
    return SOC_E_NONE;
}

/* Function:
 *    phy_84740_init
 * Purpose:    
 *    Initialize 84740 phys
 * Parameters:
 *    unit - StrataSwitch unit #.
 *    port - StrataSwitch port #. 
 * Returns:    
 *    SOC_E_NONE
 */

STATIC int
phy_84740_init(int unit, soc_port_t port)
{
    soc_timeout_t  to;
    int            rv = SOC_E_NONE;
    uint16         data16,ucode_ver;
    uint16         chip_mode;
    phy_ctrl_t     *pc;
    int            ix;
    uint32         phy_ext_rom_boot = 0; 
    uint16 mask16;
    uint32 devid;
    unsigned char *pdata;
    int data_len;
    int l2p;

    pc = EXT_PHY_SW_STATE(unit, port);
    SOC_DEBUG_PRINT((DK_PHY,
              "PHY84740 init: u=%d p=%d desc=0x%x\n",
                             unit, port,PTR_TO_INT(pc + 1)));
    SOC_IF_ERROR_RETURN(
        _phy_84740_config_devid(pc->unit,pc->port, &devid));

    /* initialize default p2l map */
    for (ix = 0; ix < NUM_LANES; ix++) {
        P2L_MAP(pc,ix) = ix;
    }

    if ((PHYCTRL_INIT_STATE(pc) == PHYCTRL_INIT_STATE_PASS1) ||
        (PHYCTRL_INIT_STATE(pc) == PHYCTRL_INIT_STATE_DEFAULT)) {
        PHY_FLAGS_SET(unit, port,  PHY_FLAGS_FIBER | PHY_FLAGS_C45);
        PCS_REPEATER(pc) = FALSE;

        if (PHYCTRL_INIT_STATE(pc) == PHYCTRL_INIT_STATE_PASS1) {
            SOC_DEBUG_PRINT((DK_PHY,
                  "PHY84740 init pass1: u=%d p=%d\n", unit, port));

            /* check if to use the bcst mode for RAM uC download */
            phy_ext_rom_boot = soc_property_port_get(unit, port, 
                                            spn_PHY_EXT_ROM_BOOT, 1);
            if (!phy_ext_rom_boot) {
                pc->flags |= PHYCTRL_MDIO_BCST;
            }

            /* configure to quad-port mode to prepare for the broadcast mode */
            SOC_IF_ERROR_RETURN(
                READ_PHY84740_MMF_PMA_PMD_REG(pc->unit, pc,
                            PHY84740_PMAD_CHIP_MODE_REG,&chip_mode));

            /* switch to quad-port mode if not already in */
            if ((chip_mode & PHY84740_PMAD_CHIP_MODE_MASK) == PHY84740_PMAD_MODE_40G) {
                SOC_IF_ERROR_RETURN(
                    _phy_84740_single_to_quad_mode(pc->unit,pc->port,pc));
                SOC_DEBUG_PRINT((DK_PHY,
                    "PHY84740 init pass1 mode S to Q switch: u=%d p=%d\n", unit, port));
            }

            /* indicate second pass of init is needed */
            PHYCTRL_INIT_STATE_SET(pc,PHYCTRL_INIT_STATE_PASS2);
            return SOC_E_NONE;
        }
    }

    if ((PHYCTRL_INIT_STATE(pc) == PHYCTRL_INIT_STATE_PASS2) ||
        (PHYCTRL_INIT_STATE(pc) == PHYCTRL_INIT_STATE_DEFAULT)) {
        SOC_DEBUG_PRINT((DK_PHY,
                  "PHY84740 init pass2: u=%d p=%d\n", unit, port));
        SOC_IF_ERROR_RETURN(
            READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, PHY84740_PMAD_CHIP_MODE_REG,&chip_mode));

        /* check the current chip mode */
        if ((chip_mode & PHY84740_PMAD_CHIP_MODE_MASK) == PHY84740_PMAD_MODE_40G) {
            /* 
             * switch to configured mode(quad mode) if current chip mode is single mode 
             * In quad mode, the supported speed modes are 10G and 1G forced, 1G autoneg.
             * However 1G currently is only working with QSFP fiber cable not Cu cable.
             */
            if (!(PHY84740_SINGLE_PORT_MODE(pc))) {
                SOC_DEBUG_PRINT((DK_WARN,
                  "PHY84740 init pass2 mode switch: u=%d p=%d\n", unit, port));
                SOC_IF_ERROR_RETURN(
                    _phy_84740_single_to_quad_mode(unit,port,NULL));

            } else {  /* configured mode is single mode */
                /* if configured mode(single mode) matches current chip mode. Make sure 
                 * it is SR4/LR4. If a optical module is plugged in when the device is 
                 * configured in CR4 mode, it may damage the optical module because of big
                 * voltage sway in CR4 mode. For now, the device is configured in
                 * LR4/SR4 mode by default. If a QSFP Cu is used, EDC mode should be set to
                 * 0x55.  In case there is a need to go to CR4 mode, user can use interface
                 * set function to change to the CR4 mode. In this mode, both autoneg and 
                 * forced 40G speed is supported.
                 */
                /* config DAC mode and backplane mode for LR4/SR4  */
                mask16 = PHY84740_PMAD_DAC_MODE_MASK | PHY84740_PMAD_BKPLANE_MODE_MASK;
                if (devid == PHY84740_ID_84064) {
                    data16 = PHY84740_PMAD_BKPLANE_MODE_MASK;
                } else {
                    data16 = 0;
                }
    
                SOC_IF_ERROR_RETURN(
                    MODIFY_PHY84740_MMF_PMA_PMD_REG(unit, pc, PHY84740_PMAD_CHIP_MODE_REG,
                          data16, mask16));

                SOC_IF_ERROR_RETURN
                    (MODIFY_PHY84740_PMA_PMD_CTRLr(unit, pc,
                                      MII_CTRL_RESET, MII_CTRL_RESET));
            }
        } else {  /* current chip mode is quad mode */
            /* switch to configured mode(single mode) if current chip mode is quad mode */
            if (PHY84740_SINGLE_PORT_MODE(pc)) {

                SOC_IF_ERROR_RETURN(
                    _phy_84740_quad_to_single_mode(unit,port));
            } else { /* configured mode is quad mode */

                /* chip already in configured mode. config DAC/backplane mode */
                mask16 = PHY84740_PMAD_DAC_MODE_MASK | PHY84740_PMAD_BKPLANE_MODE_MASK;
                if (devid == PHY84740_ID_84064) {
                    data16 = PHY84740_PMAD_BKPLANE_MODE_MASK;
                } else {
                    data16 = 0;
                }
    
                SOC_IF_ERROR_RETURN(
                    MODIFY_PHY84740_MMF_PMA_PMD_REG(unit, pc, PHY84740_PMAD_CHIP_MODE_REG,
                          data16, mask16));
            }
        }

        /* should be done before reset */
        l2p = soc_property_port_get(unit, port,spn_PHY_LANE0_L2P_MAP, 0);
        SOC_IF_ERROR_RETURN(
            MODIFY_PHY84740_MMF_PMA_PMD_REG(unit, pc, PHY84740_PMAD_LANE0_L2P_REG,
                          l2p, PHY84740_L2P_PHYSICAL_LANE_MASK));

        /* do a soft reset */
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY84740_PMA_PMD_CTRLr(unit, pc,
                              MII_CTRL_RESET, MII_CTRL_RESET));

        /* Wait for device to come out of reset */
        soc_timeout_init(&to, 10000, 0);
        while (!soc_timeout_check(&to)) {
            rv = READ_PHY84740_PMA_PMD_CTRLr(unit, pc, &data16);
            if (((data16 & MII_CTRL_RESET) == 0) ||
                SOC_FAILURE(rv)) {
                break;
            }
        }
        if (((data16 & MII_CTRL_RESET) != 0) || SOC_FAILURE(rv)) {
            SOC_DEBUG_PRINT((DK_WARN,
                             "PHY84740 reset failed: u=%d p=%d\n",
                             unit, port));
            return SOC_E_FAIL;
        }

        if (l2p >= NUM_LANES) {
            SOC_DEBUG_PRINT((DK_ERR,
                  "PHY84740 invalid L2P lane configuration: u=%d p=%d, l2p=%d\n",
                   unit, port,l2p));
            return SOC_E_PARAM;
        }

        /* physical to logical lane map, index is physical lane */
        for (ix = 0; ix < NUM_LANES; ix++) {
            P2L_MAP(pc,(l2p + ix) % NUM_LANES) = ix;
        }

        if (pc->flags & PHYCTRL_UCODE_BCST_DONE) {
            /* ucode is already loaded thru bcst */
            pc->flags &= ~PHYCTRL_UCODE_BCST_DONE;
            phy_ext_rom_boot = 0;
        } else {
            phy_ext_rom_boot = soc_property_port_get(unit, port, 
                                            spn_PHY_EXT_ROM_BOOT, 1);

            if (phy_ext_rom_boot) {
                SOC_IF_ERROR_RETURN
                    (_phy84740_rom_firmware_download(unit, port, pc));
            } else { /* MDIO download to RAM */
                /* check if the firmware is delivered by user */
                if (phy_84740_uc_firmware_hook && 
                    phy_84740_uc_firmware_hook(unit,port,devid,&pdata,&data_len) ) {
                    SOC_IF_ERROR_RETURN
                         (_phy84740_mdio_firmware_download(unit,port,pc,pdata,data_len));
                } else {
                    if (devid == PHY84740_ID_84754) {
                        SOC_IF_ERROR_RETURN
                             (_phy84740_mdio_firmware_download(unit,port,pc,phy84754_ucode_bin,
                               phy84754_ucode_bin_len));
                    } else if (devid == PHY84740_ID_84740) {
                        /* do not use 84753/84754 ucode for 84740. There is a voltage differece */
                        SOC_IF_ERROR_RETURN
                             (_phy84740_mdio_firmware_download(unit,port,pc,phy84740_ucode_bin,
                               phy84740_ucode_bin_len));
                    } else if (devid == PHY84740_ID_84064) {
                        SOC_IF_ERROR_RETURN
                             (_phy84740_mdio_firmware_download(unit,port,pc,phy84064_ucode_bin,
                               phy84064_ucode_bin_len));
                    } else if (devid == PHY84740_ID_84753) {
                        SOC_IF_ERROR_RETURN
                             (_phy84740_mdio_firmware_download(unit,port,pc,phy84753_ucode_bin,
                               phy84753_ucode_bin_len));
                    } else {
                        soc_cm_debug(DK_WARN,"84740: p=%d unknown device id: 0x%x\n",
                                     port,devid);
                    }
                }
            }
        }
  
        /* reset will restart the uC. wait a few ms for checksum to be written*/
        sal_usleep(2000);

        /* read the microcode revision */
        SOC_IF_ERROR_RETURN
            (READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0xce00, &ucode_ver));

        /* check the completion of the microcode download. */
        if (PHY84740_SINGLE_PORT_MODE(pc)) {
            uint16 cksum[4];
            int ret = SOC_E_NONE;

            for (ix = 0; ix < NUM_LANES; ix++) {
                SOC_IF_ERROR_RETURN
                    (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc, PHY84740_PMAD_AER_ADDR_REG, ix));

                /* check the completion of the microcode download. */
                rv = READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0xCA1C, &cksum[ix]);
                if (cksum[ix] != 0x600D || SOC_FAILURE(rv)) {
                    ret = SOC_E_FAIL;
                }
            }
            if (ret == SOC_E_FAIL) {
                soc_cm_debug(DK_WARN,"84740: p=%d SPI-ROM load: Bad Checksum lane 0,1,2,3 (0x%x),"
                         "(0x%x), (0x%x), (0x%x)\n",
                         port, cksum[0],cksum[1],cksum[2],cksum[3]);
                return SOC_E_FAIL;
            }
        } else {
            rv = READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0xCA1C, &data16);
            if (data16 != 0x600D || SOC_FAILURE(rv)) {
                soc_cm_debug(DK_WARN,"84740: p=%d SPI-ROM load: Bad Checksum (0x%x)\n",
                         port, data16);
                return SOC_E_FAIL;
            }
        }

        soc_cm_debug(DK_WARN,"84740 init u=%d port=%d rom code ver. 0x%x: %s boot\n",
                     unit, port, ucode_ver,phy_ext_rom_boot? "SPI-ROM":"MDIO_TO_RAM");

        if (!(PHY84740_SINGLE_PORT_MODE(pc))) {
            /* clear 1.0xcd17 to enable the PCS */
            SOC_IF_ERROR_RETURN(
                WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc, 
                            PHY84740_RESET_CONTROL_REGISTER, 0));
        } else {
            /* 40G no autoneg support */
            /* EDC mode for SR4/LR4 0x44, done in ucode, no need to set for CR4 */
            /* disable cl72 */
            SOC_IF_ERROR_RETURN
                (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0x0096, 0x0000));
            /* disable AN */
            SOC_IF_ERROR_RETURN
                (WRITE_PHY84740_MMF_AN_REG(unit, pc, AN_CTRL_REG, 0x0));
        }

        REVERSE_MODE(pc) = soc_property_port_get(unit, port, spn_PORT_PHY_MODE_REVERSE, 0);

        mask16 = PHY84740_RXLOS_OVERRIDE_MASK |
                 PHY84740_MOD_ABS_OVERRIDE_MASK;
        data16 = 0;

        RX_LOS(pc) = soc_property_port_get(unit, port,spn_PHY_RX_LOS, 0);
        MOD_ABS(pc) = soc_property_port_get(unit, port,spn_PHY_MOD_ABS, 0);
        RX_LOS_INVERT(pc) = soc_property_port_get(unit, port,spn_PHY_RX_LOS_INVERT, 0);
        MOD_ABS_INVERT(pc) = soc_property_port_get(unit, port,spn_PHY_MOD_ABS_INVERT, 0);

        /* default: override RX_LOS signalling */
        if (!RX_LOS(pc)) {
            data16 |= PHY84740_RXLOS_OVERRIDE_MASK;
        } 
        /* default: override MOD_ABS signalling */
        if (!MOD_ABS(pc)) {
            data16 |= PHY84740_MOD_ABS_OVERRIDE_MASK;
        } 

        if (PHY84740_SINGLE_PORT_MODE(pc)) {
            for (ix = 0; ix < NUM_LANES; ix++) {
                SOC_IF_ERROR_RETURN
                    (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc,
                                PHY84740_PMAD_AER_ADDR_REG, ix));
                SOC_IF_ERROR_RETURN
                    (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit,pc,
                          PHY84740_PMAD_OPTICAL_CFG_REG, 
                          data16, mask16));
            }
        } else {
            SOC_IF_ERROR_RETURN
               (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit,pc,
                                 PHY84740_PMAD_OPTICAL_CFG_REG, 
                                 data16, mask16));
        }   

        mask16 = PHY84740_RXLOS_LVL_MASK | PHY84740_MOD_ABS_LVL_MASK;
        data16 = 0;
        /* default: RX_LOS level high */
        if (!RX_LOS_INVERT(pc)) {
            data16 |= PHY84740_RXLOS_LVL_MASK;
        }
        /* default: MOD_ABS level high */
        if (!MOD_ABS_INVERT(pc)) {
            data16 |= PHY84740_MOD_ABS_LVL_MASK;
        }

        if (PHY84740_SINGLE_PORT_MODE(pc)) {
            for (ix = 0; ix < NUM_LANES; ix++) {
                SOC_IF_ERROR_RETURN
                    (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc,
                                PHY84740_PMAD_AER_ADDR_REG, ix));
                SOC_IF_ERROR_RETURN
                    (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit,pc,
                          PHY84740_PMAD_OPTICAL_SIG_LVL_REG,
                          data16, mask16));
            }
        } else {
            SOC_IF_ERROR_RETURN
               (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit,pc,
                                 PHY84740_PMAD_OPTICAL_SIG_LVL_REG,
                                 data16, mask16));
        }

        if (soc_property_port_get(unit, port, spn_PHY_PCS_REPEATER, 0)) {
            if (!PHY84740_SINGLE_PORT_MODE(pc)) {
                /* put device in repeat mode. Single port is already in repeat mode */
                SOC_IF_ERROR_RETURN(
                    MODIFY_PHY84740_MMF_PMA_PMD_REG(unit,pc, 0xc806, 0x6,0x6));
                /* in repeater mode, 84740 will use autoneg from internal serdes. But
                 * 84064 will still use cl73/cl72 from ifself.
                 */
                if (devid != PHY84740_ID_84064) {
                    PCS_REPEATER(pc) = TRUE;
                }
            }
        }
  
        /* Polarity configuration */
        SOC_IF_ERROR_RETURN
            (_phy_84740_polarity_flip_set(unit,port));
    }
    return SOC_E_NONE;
}

/*
 * Function:
 *      phy_84740_an_get
 * Purpose:
 *      Get the current auto-negotiation status (enabled/busy)
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      an   - (OUT) if true, auto-negotiation is enabled.
 *      an_done - (OUT) if true, auto-negotiation is complete. This
 *              value is undefined if an == false.
 * Returns:
 *      SOC_E_XXX
 */

STATIC int
phy_84740_an_get(int unit, soc_port_t port, int *an, int *an_done)
{
    phy_ctrl_t *pc;
    uint16 an_status;

    pc = EXT_PHY_SW_STATE(unit, port);

    SOC_IF_ERROR_RETURN(
        READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, PHY84740_PMAD_INTR_VALUE_REG,
                             &an_status));
    *an_done = (an_status & PHY84740_AUTONEG_COMPLETE)? TRUE: FALSE;

    if (PHY84740_SINGLE_PORT_MODE(pc)) {
        SOC_IF_ERROR_RETURN
            (READ_PHY84740_MMF_AN_REG(unit, pc, AN_CTRL_REG, &an_status));
        *an = (an_status & PHY84740_AN_ENABLE)? TRUE: FALSE;
    } else {
        SOC_IF_ERROR_RETURN
            (READ_PHY84740_MMF_AN_REG(unit, pc, 0xFFE0, &an_status));
        *an = (an_status & PHY84740_AN_ENABLE)? TRUE: FALSE;
    }
    return SOC_E_NONE;
}

/*
 * Function:
 *      phy_84740_an_set
 * Purpose:
 *      Enable or disabled auto-negotiation on the specified port.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      an   - Boolean, if true, auto-negotiation is enabled
 *              (and/or restarted). If false, autonegotiation is disabled.
 * Returns:
 *      SOC_E_XXX
 */
STATIC int
phy_84740_an_set(int unit, soc_port_t port, int an)
{
    phy_ctrl_t  *pc;
    phy_ctrl_t  *int_pc;
    uint16 mode;

    pc = EXT_PHY_SW_STATE(unit, port);

    SOC_DEBUG_PRINT((DK_PHY, "phy_84740_an_set: u=%d p=%d an=%d\n",
                     unit, port, an));

    /* make sure internal autoneg is turned off always */
    int_pc = INT_PHY_SW_STATE(unit, port);
    if (NULL != int_pc) {
        if (REVERSE_MODE(pc)) {
            SOC_IF_ERROR_RETURN(
                PHY_AUTO_NEGOTIATE_SET(int_pc->pd, unit, port, an));
        } else {
            /* if in passthru mode, enable autoneg on internal serdes */
            if (PCS_REPEATER(pc)) {
                SOC_IF_ERROR_RETURN(
                    PHY_AUTO_NEGOTIATE_SET(int_pc->pd, unit, port, an));
            } else {
                SOC_IF_ERROR_RETURN(
                    PHY_AUTO_NEGOTIATE_SET(int_pc->pd, unit, port, FALSE));
            }
        }
    }

    /* 0xc700(0xc710 once firmware fixed), PCS link status overide for CR4 autoneg.
     * bit 4 enable override, bit 1 status indication
     * system side link status check in 40G 0xcd04, this is per lane register 
     */
    SOC_IF_ERROR_RETURN(
        READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, PHY84740_PMAD_CHIP_MODE_REG,&mode));

    if (PHY84740_SINGLE_PORT_MODE(pc)) {
        /* make sure internal speed is set to 40G */
        if (an) {
            if (NULL != int_pc) {
                if (!REVERSE_MODE(pc)) {
                    if (soc_property_port_get(unit, port, spn_PHY_LONG_XFI, 0)) {
                        PHY_INTERFACE_SET(int_pc->pd, unit, port, SOC_PORT_IF_CR);
                    }
                    SOC_IF_ERROR_RETURN(
                        PHY_SPEED_SET(int_pc->pd, unit, port, 40000));
                }
            }
        }

        /* autoneg only supported in CR4 mode */
        /* no autoneg if in SR4/LR4 mode */    
        if (!(mode & (PHY84740_PMAD_DAC_MODE_MASK | PHY84740_PMAD_BKPLANE_MODE_MASK))) {
            an = 0; 
        }

        /* enable/disable cl72 */
        SOC_IF_ERROR_RETURN
            (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0x0096, an? 2:0));
        /* enable/disable AN */
        SOC_IF_ERROR_RETURN
            (WRITE_PHY84740_MMF_AN_REG(unit, pc, AN_CTRL_REG, 
               an? AN_ENABLE | AN_RESTART: 0x0));

        return SOC_E_NONE; 
    } else { /* quad mode, cl73 for backplane */
        if (mode & PHY84740_PMAD_BKPLANE_MODE_MASK) {
            /* make sure internal speed is set to 10G */
            if (an) {
                if (NULL != int_pc) {
                    if (soc_property_port_get(unit, port, spn_PHY_LONG_XFI, 0)) {
                        PHY_INTERFACE_SET(int_pc->pd, unit, port, SOC_PORT_IF_CR);
                    }
                    SOC_IF_ERROR_RETURN(
                        PHY_SPEED_SET(int_pc->pd, unit, port, 10000));
                }
            }

            /* enable/disable cl72 */
            SOC_IF_ERROR_RETURN
                (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0x0096, an? 2:0));
            /* enable/disable AN */
            SOC_IF_ERROR_RETURN
                (WRITE_PHY84740_MMF_AN_REG(unit, pc, AN_CTRL_REG, 
                   an? AN_ENABLE | AN_RESTART: 0x0));
        }
    }

    if (an) {
        /* Enable Clause 37 AN */
        SOC_IF_ERROR_RETURN
            (WRITE_PHY84740_MMF_AN_REG(unit, pc, 0xFFE0, MII_CTRL_AE | MII_CTRL_RAN));
        SOC_IF_ERROR_RETURN
             (MODIFY_PHY84740_MMF_AN_REG(unit, pc, 0x8309, 0, (1 << 5)));
    } else {
        /* disable Clause 37 AN */
        SOC_IF_ERROR_RETURN
            (WRITE_PHY84740_MMF_AN_REG(unit, pc, 0xFFE0, 0x0));

        /* Enable encoded forced speed derived from ieeeControl1 and ieeeControl2 */
        SOC_IF_ERROR_RETURN
             (MODIFY_PHY84740_MMF_AN_REG(unit, pc, 0x8309, (1 << 5), (1 << 5)));
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *      phy_84740_ability_advert_get
 * Purpose:
 *      Get the current advertisement for auto-negotiation.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      mode - (OUT) Port mode mask indicating supported options/speeds.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 *      The advertisement is retrieved for the ACTIVE medium.
 *      No synchronization performed at this level.
 */

STATIC int
phy_84740_ability_advert_get(int unit, soc_port_t port,
                           soc_port_ability_t *ability)
{
    uint16           an_adv;
    soc_port_mode_t  mode;
    phy_ctrl_t      *pc;

    if (NULL == ability) {
        return (SOC_E_PARAM);
    }
    pc = EXT_PHY_SW_STATE(unit, port);

    if (PCS_REPEATER(pc)) {
        phy_ctrl_t *int_pc = INT_PHY_SW_STATE(unit, port);
        if (NULL != int_pc) {
            SOC_IF_ERROR_RETURN(
                PHY_ABILITY_ADVERT_GET(int_pc->pd, unit, port, ability));
        }
        return SOC_E_NONE;
    }

    mode = 0;
    ability->pause = 0;
    if (PHY84740_SINGLE_PORT_MODE(pc)) {
        /* advert register 1 */
        SOC_IF_ERROR_RETURN
            (READ_PHY84740_MMF_AN_REG(unit, pc, AN_ADVERT_1_REG, &an_adv));

        mode |= (an_adv & AN_ADVERT_40GCR4) ? SOC_PA_SPEED_40GB : 0;
        mode |= (an_adv & AN_ADVERT_40GKR4) ? SOC_PA_SPEED_40GB : 0;
        mode |= (an_adv & AN_ADVERT_10GKR) ? SOC_PA_SPEED_10GB : 0;
        mode |= (an_adv & AN_ADVERT_1GKX) ? SOC_PA_SPEED_1000MB : 0;
        ability->speed_full_duplex = mode;

        SOC_IF_ERROR_RETURN
            (READ_PHY84740_MMF_AN_REG(unit, pc, AN_ADVERT_0_REG, &an_adv));

        switch (an_adv & (AN_ADVERT_PAUSE | AN_ADVERT_PAUSE_ASYM)) {
            case AN_ADVERT_PAUSE:
                ability->pause = SOC_PA_PAUSE_TX | SOC_PA_PAUSE_RX;
                break;
            case AN_ADVERT_PAUSE_ASYM:
                ability->pause = SOC_PA_PAUSE_TX;
                break;
            case AN_ADVERT_PAUSE | AN_ADVERT_PAUSE_ASYM:
                ability->pause = SOC_PA_PAUSE_RX;
                break;
        }
    } else {
        /* check Full Duplex advertisement on Clause 37 */
        SOC_IF_ERROR_RETURN
            (READ_PHY84740_MMF_AN_REG(unit, pc, 0xFFE4, &an_adv));
        mode |= (an_adv & MII_ANA_C37_FD) ? SOC_PA_SPEED_1000MB : 0;

        ability->speed_full_duplex = mode;

        switch (an_adv & (MII_ANA_C37_PAUSE | MII_ANA_C37_ASYM_PAUSE)) {
            case MII_ANA_C37_PAUSE:
                ability->pause = SOC_PA_PAUSE_TX | SOC_PA_PAUSE_RX;
                break;
            case MII_ANA_C37_ASYM_PAUSE:
                ability->pause = SOC_PA_PAUSE_TX;
                break;
            case MII_ANA_C37_PAUSE | MII_ANA_C37_ASYM_PAUSE:
                ability->pause = SOC_PA_PAUSE_RX;
                break;
        }
    }

    SOC_DEBUG_PRINT((DK_PHY,
        "phy_84740_ability_advert_get: u=%d p=%d speed(FD)=0x%x pause=0x%x\n",
        unit, port, ability->speed_full_duplex, ability->pause));
    return SOC_E_NONE;
}

/*
* Function:
*      phy_84740_ability_remote_get
* Purpose:
*      Get the device's complete abilities.
* Parameters:
*      unit - StrataSwitch unit #.
*      port - StrataSwitch port #.
*      ability - return device's abilities.
* Returns:
*      SOC_E_XXX
*/

STATIC int
phy_84740_ability_remote_get(int unit, soc_port_t port, 
                            soc_port_ability_t *ability)
{
    phy_ctrl_t *pc;
    uint16      lp_abil;

    if (NULL == ability) {
        return SOC_E_PARAM;
    }

    pc = EXT_PHY_SW_STATE(unit, port);
                                           
    if (PCS_REPEATER(pc)) {
        phy_ctrl_t *int_pc = INT_PHY_SW_STATE(unit, port);
        if (NULL != int_pc) {
            SOC_IF_ERROR_RETURN(
                PHY_ABILITY_REMOTE_GET(int_pc->pd, unit, port, ability));
        }
        return SOC_E_NONE;
    }

    ability->speed_half_duplex  = SOC_PA_ABILITY_NONE;

    /* check CL73 first */
    SOC_IF_ERROR_RETURN
        (READ_PHY84740_MMF_AN_REG(unit, pc, AN_LP_ABILITY_1_REG, &lp_abil));
    ability->speed_full_duplex = (lp_abil & AN_ADVERT_40GCR4) ? SOC_PA_SPEED_40GB : 0;
    ability->speed_full_duplex |= (lp_abil & AN_ADVERT_40GKR4) ? SOC_PA_SPEED_40GB : 0;
    ability->speed_full_duplex |= (lp_abil & AN_ADVERT_10GKR) ? SOC_PA_SPEED_10GB : 0;
    ability->speed_full_duplex |= (lp_abil & AN_ADVERT_1GKX) ? SOC_PA_SPEED_1000MB : 0;

    SOC_IF_ERROR_RETURN
        (READ_PHY84740_MMF_AN_REG(unit, pc, AN_LP_ABILITY_0_REG, &lp_abil));

    ability->pause = 0;
    switch (lp_abil & (AN_ADVERT_PAUSE | AN_ADVERT_PAUSE_ASYM)) {
        case AN_ADVERT_PAUSE:
            ability->pause = SOC_PA_PAUSE_TX | SOC_PA_PAUSE_RX;
            break;
        case AN_ADVERT_PAUSE_ASYM:
            ability->pause = SOC_PA_PAUSE_TX;
            break;
        case AN_ADVERT_PAUSE | AN_ADVERT_PAUSE_ASYM:
            ability->pause = SOC_PA_PAUSE_RX;
            break;
    }

    /* check CL37 */
    SOC_IF_ERROR_RETURN
        (READ_PHY84740_MMF_AN_REG(unit, pc, 0xFFE5, &lp_abil));
    ability->speed_full_duplex  |= (lp_abil & MII_ANA_C37_FD) ? 
                                   SOC_PA_SPEED_1000MB : 0;

    switch (lp_abil & (MII_ANP_C37_PAUSE | MII_ANP_C37_ASYM_PAUSE)) {
        case MII_ANP_C37_PAUSE:
            ability->pause |= SOC_PA_PAUSE_TX | SOC_PA_PAUSE_RX;
            break;
        case MII_ANP_C37_ASYM_PAUSE:
            ability->pause |= SOC_PA_PAUSE_TX;
            break;
        case MII_ANP_C37_PAUSE | MII_ANP_C37_ASYM_PAUSE:
            ability->pause |= SOC_PA_PAUSE_RX;
            break;
    }

    SOC_DEBUG_PRINT((DK_PHY,
        "phy_84740_ability_remote_get: u=%d p=%d speed(FD)=0x%x pause=0x%x\n",
        unit, port, ability->speed_full_duplex, ability->pause));

    return (SOC_E_NONE);
}

/*
 * Function:
 *      phy_84740_ability_advert_set
 * Purpose:
 *      Set the current advertisement for auto-negotiation.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      mode - Port mode mask indicating supported options/speeds.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 *      The advertisement is set only for the ACTIVE medium.
 *      No synchronization performed at this level.
 */

STATIC int
phy_84740_ability_advert_set(int unit, soc_port_t port,
                       soc_port_ability_t *ability)
{
    uint16           an_adv,an_adv_cl37,mode;
    phy_ctrl_t      *pc;

    if (NULL == ability) {
        return (SOC_E_PARAM);
    }

    pc = EXT_PHY_SW_STATE(unit, port);

    if (PCS_REPEATER(pc)) {
        phy_ctrl_t *int_pc = INT_PHY_SW_STATE(unit, port);
        if (NULL != int_pc) {
            SOC_IF_ERROR_RETURN(
                PHY_ABILITY_ADVERT_SET(int_pc->pd, unit, port, ability));
        }
        return SOC_E_NONE;
    }

    SOC_IF_ERROR_RETURN(
        READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, PHY84740_PMAD_CHIP_MODE_REG,&mode));

    if (PHY84740_SINGLE_PORT_MODE(pc)) {
        if (mode & PHY84740_PMAD_BKPLANE_MODE_MASK) {
            an_adv = (ability->speed_full_duplex & SOC_PA_SPEED_40GB) ?
                       AN_ADVERT_40GKR4 : 0;
            an_adv |= (ability->speed_full_duplex & SOC_PA_SPEED_10GB) ?
                       AN_ADVERT_10GKR : 0;
        } else {

            /* only advertise 40G CR4 */
            an_adv = (ability->speed_full_duplex & SOC_PA_SPEED_40GB) ?
                   AN_ADVERT_40GCR4 : 0;
        }

        /* CL73 advert register 1 */
        SOC_IF_ERROR_RETURN
            (WRITE_PHY84740_MMF_AN_REG(unit, pc, AN_ADVERT_1_REG, an_adv));

    } else { /* quad-port mode */
        if (mode & PHY84740_PMAD_BKPLANE_MODE_MASK) {
            an_adv = (ability->speed_full_duplex & SOC_PA_SPEED_10GB) ?
                       AN_ADVERT_10GKR : 0;
            an_adv |= (ability->speed_full_duplex & SOC_PA_SPEED_1000MB) ?
                       AN_ADVERT_1GKX : 0;

            /* CL73 advert register 1 */
            SOC_IF_ERROR_RETURN
                (WRITE_PHY84740_MMF_AN_REG(unit, pc, AN_ADVERT_1_REG, an_adv));
        } 

        /* only 1G CL37 autoneg */
        an_adv =  (ability->speed_full_duplex & SOC_PA_SPEED_1000MB) ?
              AN_ADVERT_1G : 0;

        /* write Full Duplex advertisement on Clause 37 */
        SOC_IF_ERROR_RETURN
            (WRITE_PHY84740_MMF_AN_REG(unit, pc, 0xFFE4,an_adv));
    }    

    an_adv = an_adv_cl37 = 0;
    switch (ability->pause & (SOC_PA_PAUSE_TX | SOC_PA_PAUSE_RX)) {
        case SOC_PA_PAUSE_TX:
            an_adv = AN_ADVERT_PAUSE_ASYM;
            an_adv_cl37 = MII_ANA_C37_ASYM_PAUSE;
            break;
        case SOC_PA_PAUSE_RX:
            an_adv = AN_ADVERT_PAUSE_ASYM | AN_ADVERT_PAUSE;
            an_adv_cl37 = MII_ANA_C37_ASYM_PAUSE | MII_ANA_C37_PAUSE;
            break;
        case SOC_PA_PAUSE_TX | SOC_PA_PAUSE_RX:
            an_adv_cl37 = MII_ANA_C37_PAUSE;
            an_adv = AN_ADVERT_PAUSE;
            break;
    }
    if (PHY84740_SINGLE_PORT_MODE(pc)) {
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY84740_MMF_AN_REG(unit, pc, AN_ADVERT_0_REG, an_adv,
                               AN_ADVERT_PAUSE | AN_ADVERT_PAUSE_ASYM));
    } else {
        if (mode & PHY84740_PMAD_BKPLANE_MODE_MASK) {
            SOC_IF_ERROR_RETURN
                (MODIFY_PHY84740_MMF_AN_REG(unit, pc, AN_ADVERT_0_REG, an_adv,
                               AN_ADVERT_PAUSE | AN_ADVERT_PAUSE_ASYM));
        }

        /* write Pause advertisement on Clause 37 */
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY84740_MMF_AN_REG(unit, pc, 0xFFE4, an_adv_cl37,
                                MII_ANA_C37_ASYM_PAUSE | MII_ANA_C37_PAUSE));
    }

    SOC_DEBUG_PRINT((DK_PHY,
        "phy_8072_ability_advert_set: u=%d p=%d pause=0x%08x adv_reg1=0x%04x\n",
        unit, port, ability->pause, an_adv));
        
    return SOC_E_NONE;
}

/*
 * Function:
 *      phy_84740_ability_local_get
 * Purpose:
 *      Get the device's complete abilities.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      ability - return device's abilities.
 * Returns:
 *      SOC_E_XXX
 */

STATIC int
phy_84740_ability_local_get(int unit, soc_port_t port, soc_port_ability_t *ability)
{
    phy_ctrl_t *pc;
    phy_ctrl_t  *int_pc;
    pc = EXT_PHY_SW_STATE(unit, port);
    int_pc = INT_PHY_SW_STATE(unit, port);

    SOC_DEBUG_PRINT((DK_PHY,
                     "phy_84740_ability_local_get: u=%d p=%d\n",
                     unit, port));
                                                                               
    if (NULL == ability) {
        return SOC_E_PARAM;
    }

    if (NULL != int_pc && PCS_REPEATER(pc)) {
        SOC_IF_ERROR_RETURN(
            PHY_ABILITY_LOCAL_GET(int_pc->pd, unit, port, ability));
        return SOC_E_NONE;
    }
    ability->speed_half_duplex  = SOC_PA_ABILITY_NONE;
                                                                               
    switch(pc->speed_max) {
        case 40000:
            ability->speed_full_duplex = SOC_PA_SPEED_40GB;
            break;
        case 10000:
        default:
            ability->speed_full_duplex  = SOC_PA_SPEED_1000MB;
            ability->speed_full_duplex |= SOC_PA_SPEED_10GB;
    }
                                                                               
    ability->pause     = SOC_PA_PAUSE | SOC_PA_PAUSE_ASYMM; 
    ability->interface = SOC_PA_INTF_XGMII;
    ability->medium    = SOC_PA_MEDIUM_FIBER;
    ability->loopback  = SOC_PA_LB_PHY;
                                                                               
    SOC_DEBUG_PRINT((DK_PHY,
        "phy_84740_ability_local_get: u=%d p=%d speed=0x%x\n",
        unit, port, ability->speed_full_duplex));
                                                                               
    return (SOC_E_NONE);
}


/*
 * Function:
 *    phy_84740_link_get
 * Purpose:
 *    Get layer2 connection status.
 * Parameters:
 *    unit - StrataSwitch unit #.
 *    port - StrataSwitch port #. 
 *      link - address of memory to store link up/down state.
 * Returns:    
 *    SOC_E_NONE
 */

STATIC int
phy_84740_link_get(int unit, soc_port_t port, int *link)
{
    uint16      pma_mii_stat, pcs_mii_stat, link_stat;
    phy_ctrl_t *pc;
    phy_ctrl_t  *int_pc;
    int cur_speed = 0;
    int an,an_done;
    uint16 speed_val; 
    int rv;

    if (link == NULL) {
        return SOC_E_NONE;
    }

    if (PHY_FLAGS_TST(unit, port, PHY_FLAGS_DISABLE)) {
        *link = FALSE;
        return SOC_E_NONE;
    }

    pc = EXT_PHY_SW_STATE(unit, port);
    int_pc = INT_PHY_SW_STATE(unit, port);

    if (!REVERSE_MODE(pc)) {
        if (PCS_REPEATER(pc)) {
            rv = PHY_LINK_GET(int_pc->pd, unit, port, link);
            return rv;
        }
    }

    phy_84740_an_get(unit,port,&an,&an_done);

    /* only supports 40G. PHY PCS is not used. Check PMD link only  */
    if (PHY84740_SINGLE_PORT_MODE(pc)) {
        int serdes_link = TRUE; 

        if (REVERSE_MODE(pc)) {
            SOC_IF_ERROR_RETURN
                (PHY84740_XFI(unit,pc));
        }

        SOC_IF_ERROR_RETURN
            (READ_PHY84740_PMA_PMD_STATr(unit, pc, &pma_mii_stat));
        *link = (pma_mii_stat & MII_STAT_LA) ? TRUE : FALSE;
        
        if (REVERSE_MODE(pc)) {
            SOC_IF_ERROR_RETURN
                (PHY84740_MMF(unit,pc));
        }

        if (!REVERSE_MODE(pc)) {
            if ((NULL != int_pc) && an) {
                rv = PHY_LINK_GET(int_pc->pd, unit, port, &serdes_link);
            }
        } 

        *link = (*link) && serdes_link;
        return SOC_E_NONE;
    } 

    /* return link false if in the middle of autoneg */
    if (an == TRUE && an_done == FALSE) {
        *link = FALSE;
        return SOC_E_NONE;
    }

    /* AN only supports 1G */
    if (!an) {
        SOC_IF_ERROR_RETURN
            (READ_PHY84740_MMF_PMA_PMD_REG(unit,pc, PHY84740_PMAD_CTRL2_REG, 
                                          &speed_val));
        if ((speed_val & PHY84740_PMAD_CTRL2r_PMA_TYPE_MASK) !=
                                       PHY84740_PMAD_CTRL2r_PMA_TYPE_1G_KX) {
            cur_speed = 10000;
        }
    } else {
        cur_speed = 1000;
    }

    if (REVERSE_MODE(pc)) {
        SOC_IF_ERROR_RETURN
            (PHY84740_XFI(unit,pc));
    }

    if (cur_speed == 10000) { /* check all 3 device's link status if 10G */

        /* MMF Link status */
        /* Read Status 2 register to get rid of Local faults */
        SOC_IF_ERROR_RETURN
            (READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, PHY84740_PMAD_STAT2_REG,
                                          &pma_mii_stat));
        SOC_IF_ERROR_RETURN
            (READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, PHY84740_PMAD_STAT2_REG,
                                          &pma_mii_stat));
        /* Receive Link status */
        SOC_IF_ERROR_RETURN
            (READ_PHY84740_PCS_STATr(unit, pc, &pcs_mii_stat));

        SOC_IF_ERROR_RETURN
            (READ_PHY84740_PMA_PMD_STATr(unit, pc, &pma_mii_stat));

        /* MMF link status */
        link_stat = pma_mii_stat & pcs_mii_stat;
        *link = (link_stat & MII_STAT_LA) ? TRUE : FALSE;
    } else {
        SOC_IF_ERROR_RETURN
            (READ_PHY84740_MMF_AN_REG(unit, pc, 0xffe1,
                                     &link_stat));
        *link = (link_stat & MII_STAT_LA) ? TRUE : FALSE;
    } 

    if (REVERSE_MODE(pc)) {
        SOC_IF_ERROR_RETURN
            (PHY84740_MMF(unit,pc));
    }

    soc_cm_debug(DK_PHY | DK_VERBOSE,
         "phy_84740_link_get: u=%d port%d: link:%s\n",
         unit, port, *link ? "Up": "Down");

    return SOC_E_NONE;
}

/*
 * Function:
 *    phy_84740_enable_set
 * Purpose:
 *    Enable/Disable phy 
 * Parameters:
 *    unit - StrataSwitch unit #.
 *    port - StrataSwitch port #. 
 *      enable - on/off state to set
 * Returns:    
 *    SOC_E_NONE
 */

STATIC int
phy_84740_enable_set(int unit, soc_port_t port, int enable)
{
    uint16      data;       /* Holder for new value to write to PHY reg */
    uint16      mask;       /* Holder for bit mask to update in PHY reg */
    int ix;
    uint16 tx_ctrl;
    phy_ctrl_t *pc;

    pc = EXT_PHY_SW_STATE(unit, port);

    data = 0;
    mask = 1 << 0; /* Global PMD transmit disable */
    if (enable) {
        PHY_FLAGS_CLR(unit, port, PHY_FLAGS_DISABLE);
    } else {
        data = 1 << 0;  /* Global PMD transmit disable */
        PHY_FLAGS_SET(unit, port, PHY_FLAGS_DISABLE);
    }

    if (REVERSE_MODE(pc)) {
        SOC_IF_ERROR_RETURN
            (PHY84740_XFI(unit,pc));
    }

    /* TX transmitter enable/disable. BCST register in single-port mode */
    SOC_IF_ERROR_RETURN
        (MODIFY_PHY84740_PMA_PMD_TX_DISABLEr(unit, pc, data, mask));

    if (REVERSE_MODE(pc)) {
        SOC_IF_ERROR_RETURN
            (PHY84740_MMF(unit,pc));
    }

    for (ix = 0; ix < NUM_LANES; ix++) {
        if (PHY84740_SINGLE_PORT_MODE(pc)) {
            SOC_IF_ERROR_RETURN
                (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc,
                            PHY84740_PMAD_AER_ADDR_REG, ix));
        }

        /* TxOn control */
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit,pc,
                             PHY84740_PMAD_OPTICAL_CFG_REG, 
                             enable? 0: (1 << 12), 
                             (1 << 12)));
        if (enable) {
            SOC_IF_ERROR_RETURN
                (READ_PHY84740_MMF_PMA_PMD_REG(unit,pc,
                             PHY84740_PMAD_OPTICAL_CFG_REG,&data));

            /* c8e4[4] indicates the TX active state when c8e4[12]=0. 0 means TX 
             * active, 1 means low power mode. This state = TxOnOff pin XOR c800[7]
             * Toggle 0xc800[7] to enable the Tx when c8e4[4]=1.
             * The default value of c800[7] is 1, if user straps the TxOnOff to 0,
             * Then the Tx won't be enabled until this function is called.
             */

            SOC_IF_ERROR_RETURN
                (READ_PHY84740_MMF_PMA_PMD_REG(unit,pc,
                         PHY84740_PMAD_OPTICAL_SIG_LVL_REG, &tx_ctrl));
            tx_ctrl &= (1 << 7);
            if (data & (1 << 4)) {
                SOC_IF_ERROR_RETURN
                    (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit,pc,
                         PHY84740_PMAD_OPTICAL_SIG_LVL_REG,
                         tx_ctrl? 0:(1 << 7),
                         (1 << 7)));
            }
        } 
        /* power on/off the lane */
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit,pc,
                             PHY84740_PMAD_OPTICAL_CFG_REG, 
                             enable? 0: (1 << 4), 
                             (1 << 4)));
        if (!PHY84740_SINGLE_PORT_MODE(pc)) {
            break;
        }
    }
    return (SOC_E_NONE);
}

/*
 * Function:
 *    phy_84740_lb_set
 * Purpose:
 *    Put 84740 in PHY PMA/PMD loopback
 * Parameters:
 *    unit - StrataSwitch unit #.
 *    port - StrataSwitch port #. 
 *      enable - binary value for on/off (1/0)
 * Returns:    
 *    SOC_E_NONE
 */

STATIC int
phy_84740_lb_set(int unit, soc_port_t port, int enable)
{
    phy_ctrl_t *pc; 
    uint16     tmp;
    pc = EXT_PHY_SW_STATE(unit, port);

    tmp = enable ? PHY84740_MII_CTRL_PMA_LOOPBACK : 0;

    if (REVERSE_MODE(pc)) {
        SOC_IF_ERROR_RETURN
            (PHY84740_XFI(unit,pc));
    }

    SOC_IF_ERROR_RETURN
        (MODIFY_PHY84740_PMA_PMD_CTRLr(unit, pc,
                                  tmp, PHY84740_MII_CTRL_PMA_LOOPBACK));

    if (REVERSE_MODE(pc)) {
        SOC_IF_ERROR_RETURN
            (PHY84740_MMF(unit,pc));
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *    phy_84740_lb_get
 * Purpose:
 *    Get 84740 PHY loopback state
 * Parameters:
 *    unit - StrataSwitch unit #.
 *    port - StrataSwitch port #. 
 *      enable - address of location to store binary value for on/off (1/0)
 * Returns:    
 *    SOC_E_NONE
 */

STATIC int
phy_84740_lb_get(int unit, soc_port_t port, int *enable)
{
    uint16      tmp;
    phy_ctrl_t *pc;

    pc = EXT_PHY_SW_STATE(unit, port);

    if (REVERSE_MODE(pc)) {
        SOC_IF_ERROR_RETURN
            (PHY84740_XFI(unit,pc));
    }

    SOC_IF_ERROR_RETURN
        (READ_PHY84740_PMA_PMD_CTRLr(unit, pc, &tmp));
    *enable = (tmp & PHY84740_MII_CTRL_PMA_LOOPBACK) ? TRUE : FALSE;

    if (REVERSE_MODE(pc)) {
        SOC_IF_ERROR_RETURN
            (PHY84740_MMF(unit,pc));
    }

    soc_cm_debug(DK_PHY,
         "phy_84740_lb_get: u=%d port%d: loopback:%s\n",
         unit, port, *enable ? "Enabled": "Disabled");
    
    return SOC_E_NONE;
}

STATIC int
_phy_84740_control_tx_driver_set(int unit, soc_port_t port,
                                soc_phy_control_t type, uint32 value)
{
    uint16       data;  /* Temporary holder of reg value to be written */
    uint16       mask;  /* Bit mask of reg value to be updated */
    int ix;
    int lane;
    int start;
    int end;
    uint16 post_tap;
    uint16 main_tap;
    phy_ctrl_t  *pc;    /* PHY software state */
      
    pc = EXT_PHY_SW_STATE(unit, port);
    switch(type) {
        case SOC_PHY_CONTROL_DRIVER_CURRENT:
        /* fall through */
        case SOC_PHY_CONTROL_DRIVER_CURRENT_LANE0:
        /* fall through */
        case SOC_PHY_CONTROL_DRIVER_CURRENT_LANE1:
        /* fall through */
        case SOC_PHY_CONTROL_DRIVER_CURRENT_LANE2:
        /* fall through */
        case SOC_PHY_CONTROL_DRIVER_CURRENT_LANE3:
            data = (uint16)(value & 0xf);
            data = data << 12;
            mask = 0xf000;
            if (type == SOC_PHY_CONTROL_DRIVER_CURRENT_LANE0) {
                lane = 0;
            } else if (type == SOC_PHY_CONTROL_DRIVER_CURRENT_LANE1) {
                lane = 1;
            } else if (type == SOC_PHY_CONTROL_DRIVER_CURRENT_LANE2) {
                lane = 2;
            } else if (type == SOC_PHY_CONTROL_DRIVER_CURRENT_LANE3) {
                lane = 3;
            } else {  /* SOC_PHY_CONTROL_DRIVER_CURRENT */
                lane = 4; /* all lanes */
            }
            if (PHY84740_SINGLE_PORT_MODE(pc)) {
                start = lane == 4? 0: lane;
                end   = lane == 4? 4: lane + 1;
                for (ix = start; ix < end; ix++) { 
                    SOC_IF_ERROR_RETURN
                        (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc,
                                    PHY84740_PMAD_AER_ADDR_REG, P2L_MAP(pc,ix)));
                    SOC_IF_ERROR_RETURN
                        (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0xca01, data, mask));
                }
            } else {
                SOC_IF_ERROR_RETURN
                    (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0xca01, data, mask));
            }
        break;

        case SOC_PHY_CONTROL_PRE_DRIVER_CURRENT:
        /* fall through */
        case SOC_PHY_CONTROL_PRE_DRIVER_CURRENT_LANE0:
        /* fall through */
        case SOC_PHY_CONTROL_PRE_DRIVER_CURRENT_LANE1:
        /* fall through */
        case SOC_PHY_CONTROL_PRE_DRIVER_CURRENT_LANE2:
        /* fall through */
        case SOC_PHY_CONTROL_PRE_DRIVER_CURRENT_LANE3:
            data = (uint16)(value & 0xf);
            data = data << 8;
            mask = 0x0f00;
            if (type == SOC_PHY_CONTROL_PRE_DRIVER_CURRENT_LANE0) {
                lane = 0;
            } else if (type == SOC_PHY_CONTROL_PRE_DRIVER_CURRENT_LANE1) {
                lane = 1;
            } else if (type == SOC_PHY_CONTROL_PRE_DRIVER_CURRENT_LANE2) {
                lane = 2;
            } else if (type == SOC_PHY_CONTROL_PRE_DRIVER_CURRENT_LANE3) {
                lane = 3;
            } else {  /* SOC_PHY_CONTROL_PRE_DRIVER_CURRENT */
                lane = 4; /* all lanes */
            }
            if (PHY84740_SINGLE_PORT_MODE(pc)) {
                start = lane == 4? 0: lane;
                end   = lane == 4? 4: lane + 1;
                for (ix = start; ix < end; ix++) {
                    SOC_IF_ERROR_RETURN
                        (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc,
                                    PHY84740_PMAD_AER_ADDR_REG, P2L_MAP(pc,ix)));
                    SOC_IF_ERROR_RETURN
                        (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0xca01, data, mask));
                }
            } else {
                SOC_IF_ERROR_RETURN
                    (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0xca01, data, mask));
            }
            break;
        case SOC_PHY_CONTROL_PREEMPHASIS:
            /* fall through */
        case SOC_PHY_CONTROL_PREEMPHASIS_LANE0:
            /* fall through */
        case SOC_PHY_CONTROL_PREEMPHASIS_LANE1:
            /* fall through */
        case SOC_PHY_CONTROL_PREEMPHASIS_LANE2:
            /* fall through */
        case SOC_PHY_CONTROL_PREEMPHASIS_LANE3:
        /* The value format:
         * bit 15:  1 enable forced preemphasis ctrl, 0 auto config
         * bit 14:10:  post_tap value
         * bit 09:04:  main tap value 
         * 
         * set 1.CA05[15] = 0, 1.CA02[15:11] for main_tap and 1.CA05[7:4] for post tap.
         * main tap control is in the register field 0xCA02[15:11]. 
         * If a wrong value is written to the same register, field 0xCA02[9:7],
         *  it may DAMAGE the device. User must avoid writing to 0xCA02[9:7] 
         */ 
        /* Register bit15, 0 force preemphasis, 1 auto config, 1.CA05 */ 
        post_tap = PHY84740_PREEMPH_GET_FORCE(value)? 0:
                   PHY84740_PREEMPH_REG_FORCE_MASK;

        /* add post tap value */
        post_tap |= PHY84740_PREEMPH_GET_POST_TAP(value) << 
                    PHY84740_PREEMPH_REG_POST_TAP_SHFT;

        /* main tap in 1.CA02 */
        main_tap = PHY84740_PREEMPH_GET_MAIN_TAP(value) <<
                   PHY84740_PREEMPH_REG_MAIN_TAP_SHFT;

        if (type == SOC_PHY_CONTROL_PREEMPHASIS_LANE0) {
            lane = 0;
        } else if (type == SOC_PHY_CONTROL_PREEMPHASIS_LANE1) {
            lane = 1;
        } else if (type == SOC_PHY_CONTROL_PREEMPHASIS_LANE2) {
            lane = 2;
        } else if (type == SOC_PHY_CONTROL_PREEMPHASIS_LANE3) {
            lane = 3;
        } else {  /* SOC_PHY_CONTROL_PREEMPHASIS_LANE */
            lane = 4; /* all lanes */
        }
        if (PHY84740_SINGLE_PORT_MODE(pc)) {
            start = lane == 4? 0: lane;
            end   = lane == 4? 4: lane + 1;
        } else {
            /* just make sure execute once */
            start = 0;
            end   = 1;
        }
        for (ix = start; ix < end; ix++) {
            if (PHY84740_SINGLE_PORT_MODE(pc)) {
                SOC_IF_ERROR_RETURN
                    (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc,
                                PHY84740_PMAD_AER_ADDR_REG, P2L_MAP(pc,ix)));
            }
            SOC_IF_ERROR_RETURN
                (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0xca05, 
                          post_tap, PHY84740_PREEMPH_REG_POST_TAP_MASK |
                                    PHY84740_PREEMPH_REG_FORCE_MASK));

            /* always clear tx_pwrdn bit. The read/modify/write may accidently
             * set the bit 0xCA02[10]. Because the CA02[10] read value is: 
             * CA02[10]_read_value = CA02[10] OR Digital_State_Machine 
             * The problem will occur when the Digital_State_Machine output is 1
             * at the time of reading this register.
             */ 
            SOC_IF_ERROR_RETURN
                (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0xca02, 
                          main_tap, 
                    PHY84740_PREEMPH_REG_MAIN_TAP_MASK |
                    PHY84740_PREEMPH_REG_TX_PWRDN_MASK));
        }
        break;
        
        default:
         /* should never get here */
         return SOC_E_PARAM;
    }
    return SOC_E_NONE;
}

STATIC int
_phy_84740_control_tx_driver_get(int unit, soc_port_t port,
                                soc_phy_control_t type, uint32 *value)
{
    uint16         data16;   /* Temporary holder of a reg value */
    uint16         main_tap;
    uint16         post_tap;
    int lane;
    phy_ctrl_t    *pc;       /* PHY software state */

    pc = EXT_PHY_SW_STATE(unit, port);

    data16 = 0;
    switch(type) {
        case SOC_PHY_CONTROL_PREEMPHASIS:
        /* fall through */
        case SOC_PHY_CONTROL_PREEMPHASIS_LANE0:
        /* fall through */
        case SOC_PHY_CONTROL_PREEMPHASIS_LANE1:
        /* fall through */
        case SOC_PHY_CONTROL_PREEMPHASIS_LANE2:
        /* fall through */
        case SOC_PHY_CONTROL_PREEMPHASIS_LANE3:
            if (type == SOC_PHY_CONTROL_PREEMPHASIS_LANE0) {
                lane = 0;
            } else if (type == SOC_PHY_CONTROL_PREEMPHASIS_LANE1) {
                lane = 1;
            } else if (type == SOC_PHY_CONTROL_PREEMPHASIS_LANE2) {
                lane = 2;
            } else if (type == SOC_PHY_CONTROL_PREEMPHASIS_LANE3) {
                lane = 3;
            } else {
                lane = 0;
            } 
            /* 1.CA05[15] = 0, set 1.CA02[15:11] for main_tap and 
             * 1.CA05[7:4] for postcursor 
             */
            if (PHY84740_SINGLE_PORT_MODE(pc)) {
                SOC_IF_ERROR_RETURN
                    (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc,
                                  PHY84740_PMAD_AER_ADDR_REG, P2L_MAP(pc,lane)));
            }
            SOC_IF_ERROR_RETURN
                (READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0xca02, &main_tap));
            SOC_IF_ERROR_RETURN
                (READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0xca05, &post_tap));

            /* bit15 force flag, 14:10 post_tap, 09:04 main tap */
            *value = (post_tap & PHY84740_PREEMPH_REG_FORCE_MASK)? 0:
                     (1 << PHY84740_PREEMPH_CTRL_FORCE_SHFT); 
            *value |= ((post_tap & PHY84740_PREEMPH_REG_POST_TAP_MASK) >>
                      PHY84740_PREEMPH_REG_POST_TAP_SHFT) << 
                      PHY84740_PREEMPH_CTRL_POST_TAP_SHFT; 
            *value |= ((main_tap & PHY84740_PREEMPH_REG_MAIN_TAP_MASK) >>
                      PHY84740_PREEMPH_REG_MAIN_TAP_SHFT) <<
                      PHY84740_PREEMPH_CTRL_MAIN_TAP_SHFT; 

        break;

        case SOC_PHY_CONTROL_DRIVER_CURRENT:
            /* fall through */
        case SOC_PHY_CONTROL_DRIVER_CURRENT_LANE0:
            /* fall through */
        case SOC_PHY_CONTROL_DRIVER_CURRENT_LANE1:
            /* fall through */
        case SOC_PHY_CONTROL_DRIVER_CURRENT_LANE2:
            /* fall through */
        case SOC_PHY_CONTROL_DRIVER_CURRENT_LANE3:
            if (type == SOC_PHY_CONTROL_DRIVER_CURRENT_LANE0) {
                lane = 0;
            } else if (type == SOC_PHY_CONTROL_DRIVER_CURRENT_LANE1) {
                lane = 1;
            } else if (type == SOC_PHY_CONTROL_DRIVER_CURRENT_LANE2) {
                lane = 2;
            } else if (type == SOC_PHY_CONTROL_DRIVER_CURRENT_LANE3) {
                lane = 3;
            } else {
                lane = 0;
            }
            if (PHY84740_SINGLE_PORT_MODE(pc)) {
                SOC_IF_ERROR_RETURN
                    (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc,
                                  PHY84740_PMAD_AER_ADDR_REG, P2L_MAP(pc,lane)));
            }
            SOC_IF_ERROR_RETURN
                (READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0xca01, &data16));
            *value = (data16 & 0xf000) >> 12;
        break;

        case SOC_PHY_CONTROL_PRE_DRIVER_CURRENT:
            /* fall through */
        case SOC_PHY_CONTROL_PRE_DRIVER_CURRENT_LANE0:
            /* fall through */
        case SOC_PHY_CONTROL_PRE_DRIVER_CURRENT_LANE1:
            /* fall through */
        case SOC_PHY_CONTROL_PRE_DRIVER_CURRENT_LANE2:
            /* fall through */
        case SOC_PHY_CONTROL_PRE_DRIVER_CURRENT_LANE3:
            if (type == SOC_PHY_CONTROL_PRE_DRIVER_CURRENT_LANE0) {
                lane = 0;
            } else if (type == SOC_PHY_CONTROL_PRE_DRIVER_CURRENT_LANE1) {
                lane = 1;
            } else if (type == SOC_PHY_CONTROL_PRE_DRIVER_CURRENT_LANE2) {
                lane = 2;
            } else if (type == SOC_PHY_CONTROL_PRE_DRIVER_CURRENT_LANE3) {
                lane = 3;
            } else {
                lane = 0;
            }
            if (PHY84740_SINGLE_PORT_MODE(pc)) {
                SOC_IF_ERROR_RETURN
                    (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc,
                                  PHY84740_PMAD_AER_ADDR_REG, P2L_MAP(pc,lane)));
            }
            SOC_IF_ERROR_RETURN
                (READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0xca01, &data16));
            *value = (data16 & 0x0f00) >> 8;
            break;

       default:
         /* should never get here */
         return SOC_E_PARAM;
    }

    return SOC_E_NONE;
}

STATIC int
_phy_84740_control_edc_mode_set(int unit, soc_port_t port, uint32 value)
{
    uint16         data16;
    uint16         mask16;
    phy_ctrl_t    *pc;       /* PHY software state */
    int ix;

     pc = EXT_PHY_SW_STATE(unit, port);

    /* clear software overrides */
    mask16 = 0;
    if (!RX_LOS(pc)) {
        mask16 |= PHY84740_RXLOS_OVERRIDE_MASK;
    }
    /* default: override MOD_ABS signalling */
    if (!MOD_ABS(pc)) {
        mask16 |= PHY84740_MOD_ABS_OVERRIDE_MASK;
    }

    if (PHY84740_SINGLE_PORT_MODE(pc)) {
        for (ix = 0; ix < NUM_LANES; ix++) {
            SOC_IF_ERROR_RETURN
                (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc,
                                PHY84740_PMAD_AER_ADDR_REG, ix));
            SOC_IF_ERROR_RETURN
                (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit,pc,
                         PHY84740_PMAD_OPTICAL_CFG_REG, 0,
                         mask16));
        }
    } else {
        SOC_IF_ERROR_RETURN
           (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit,pc,
                         PHY84740_PMAD_OPTICAL_CFG_REG, 0,
                         mask16));
    }

    for (ix = 0; ix < NUM_LANES; ix++) {
        if (PHY84740_SINGLE_PORT_MODE(pc)) {
            SOC_IF_ERROR_RETURN
                (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc,
                            PHY84740_PMAD_AER_ADDR_REG, ix));
        }

        /* EDC mode programming sequence*/     
        mask16 = 1 << 9;

        /* induce LOS condition: toggle register bit 0xc800.9 */
        SOC_IF_ERROR_RETURN         
            (READ_PHY84740_MMF_PMA_PMD_REG(unit,pc, 
                 PHY84740_PMAD_OPTICAL_SIG_LVL_REG,&data16));

        /* only change toggled bit 9 */
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit,pc, 
                 PHY84740_PMAD_OPTICAL_SIG_LVL_REG,~data16,mask16));

        /* program EDC mode */
        SOC_IF_ERROR_RETURN
          (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit,pc, 0xCA1A,(uint16)value,
                                  PHY84740_EDC_MODE_MASK));

        /* remove LOS condition: restore back original value of bit 0xc800.9 */
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit,pc, 
                   PHY84740_PMAD_OPTICAL_SIG_LVL_REG,data16,mask16));

        if (!PHY84740_SINGLE_PORT_MODE(pc)) {
            break;
        }
    }

    /* restore software override */
    data16 = 0;
    mask16 = PHY84740_RXLOS_OVERRIDE_MASK | PHY84740_MOD_ABS_OVERRIDE_MASK;
    if (!RX_LOS(pc)) {
        data16 |= PHY84740_RXLOS_OVERRIDE_MASK;
    }
    /* default: override MOD_ABS signalling */
    if (!MOD_ABS(pc)) {
        data16 |= PHY84740_MOD_ABS_OVERRIDE_MASK;
    }

    if (PHY84740_SINGLE_PORT_MODE(pc)) {
        for (ix = 0; ix < NUM_LANES; ix++) {
            SOC_IF_ERROR_RETURN
                (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc,
                                PHY84740_PMAD_AER_ADDR_REG, ix));
            SOC_IF_ERROR_RETURN
                (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit,pc,
                          PHY84740_PMAD_OPTICAL_CFG_REG, 
                          data16,
                          mask16));
        }
    } else {
        /* XXX temp 0xc0c0: RXLOS override: 0x0808 MOD_ABS override */
        SOC_IF_ERROR_RETURN
           (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit,pc,
                         PHY84740_PMAD_OPTICAL_CFG_REG, 
                         data16,
                         mask16));
    }

    return SOC_E_NONE;
}

/*
 * enable/disable syncE recoverd clock for 10G/40G modes.
 */
STATIC int
_phy_84740_control_recovery_clock_set(int unit, soc_port_t port, int enable)
{
    uint16 data16;
    uint16 mask16;
    int lane;
    phy_ctrl_t    *pc;       /* PHY software state */

    pc = EXT_PHY_SW_STATE(unit, port);

   /* 
    *  1.CB27[3]	= 0   to disable FC recovered clock to appear at TESTP/N.
    *  1.CA1D[11]	= 0   to disable PLL clock to appear at TESTP/N.
    *  1.CA1D[1:0]      = 00  to disable bandgap voltage and vco_control voltage appear at TESTP/N.
    *  1.CB27[0] 	= 1   enables rx recovered clock to come out of TESTP/N pads.
    *  1.CB26[14:13]    = 11  for 156.25Mhz syncE divider select
    *  1.CB26[12]	= 1   for syncE clock out.
    */    
  
    for (lane = 0; lane < NUM_LANES; lane++) {
        if (PHY84740_SINGLE_PORT_MODE(pc)) {
            SOC_IF_ERROR_RETURN
                (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc,
                                            PHY84740_PMAD_AER_ADDR_REG, lane));
        }
        /* set to 0 in both enable/disable cases */

        /* disable FC recovered clock to appear at TESTP/N. */
        SOC_IF_ERROR_RETURN
           (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit,pc,PHY84740_PMAD_RECOVERY_CLK_REG, 
                                            0,(1 << 3)));
        /* disable PLL clock to appear at TESTP/N
         * disable bandgap voltage and vco_control voltage appear at TESTP/N
         */
        mask16 = (1 << 11) | 0x3;
        SOC_IF_ERROR_RETURN
           (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit,pc,0xCA1D, 0,mask16));

        /* enables rx recovered clock to come out of TESTP/N pads*/
        mask16 = PHY84740_RECOVERY_CLK_ENABLE_MASK;
        data16 = enable? mask16: 0;
        SOC_IF_ERROR_RETURN
           (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit,pc,PHY84740_PMAD_RECOVERY_CLK_REG, 
                                        data16,mask16));

        /* always use bits 14:13 to select the sync clock frequency */
        mask16 = (0x1 << 12);
        data16 = mask16;
        SOC_IF_ERROR_RETURN
           (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit,pc,PHY84740_PMAD_RECOVERY_CLK_FREQ_REG, 
                                            data16,mask16));
       if (!PHY84740_SINGLE_PORT_MODE(pc)) {
           break;
       }
    }
    return SOC_E_NONE;
}

/*
 * set the frequency of the syncE recoverd clock.
 */
STATIC int
_phy_84740_control_recovery_clock_freq_set(int unit, soc_port_t port, int freq)
{
    phy_ctrl_t    *pc;       /* PHY software state */
    int lane;
    uint16 freq_ctrl;

    pc = EXT_PHY_SW_STATE(unit, port);
    
    /*  
     * 1.CB26[14:13]
     * Select the frequency of SyncE clock to be sent to the test port: 
     * 00 :  Power down syncE clock dividers
     * 01 :  125 MHz 		(divide by 82.5) 
     * 10 :  195.3125 MHz 	(divide by 52.8)
     * 11 :  156.25 MHz 	(divide by 66)
     */

    switch (freq) {
        case 125000:
            freq_ctrl = 0x1;
            break;
        case 195312:
            freq_ctrl = 0x2;
            break;
        case 156250:
            freq_ctrl = 0x3;
            break;
        default:
            freq_ctrl = 0x0;
            break;
    }

    for (lane = 0; lane < NUM_LANES; lane++) {
        if (PHY84740_SINGLE_PORT_MODE(pc)) {
            SOC_IF_ERROR_RETURN
                (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc,
                                            PHY84740_PMAD_AER_ADDR_REG, lane));
        }

        SOC_IF_ERROR_RETURN
            (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit,pc,PHY84740_PMAD_RECOVERY_CLK_FREQ_REG, 
                      PHY84740_RECOVERY_CLK_FREQf(freq_ctrl),
                      PHY84740_RECOVERY_CLK_FREQ_MASK));

        if (!PHY84740_SINGLE_PORT_MODE(pc)) {
           break;
        }
    }
    return SOC_E_NONE;
}

STATIC int
_phy_84740_control_prbs_tx_invert_data_set(int unit, soc_port_t port, int invert)
{
    phy_ctrl_t    *pc;       /* PHY software state */
    uint16 data16;

    pc = EXT_PHY_SW_STATE(unit, port);

    if (PHY84740_SINGLE_PORT_MODE(pc)) {
        return SOC_E_UNAVAIL;
    } else {  /* quad-port mode */
        /* tx inversion */
        data16 = invert? PHY84740_USER_PRBS_TX_INVERT: 0;
        
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit, pc, 
                  PHY84740_USER_PRBS_CONTROL_0_REGISTER, data16,
                           PHY84740_USER_PRBS_TX_INVERT));
    }
    return SOC_E_NONE;
}

STATIC int
_phy_84740_control_prbs_rx_invert_data_set(int unit, soc_port_t port, int invert)
{
    phy_ctrl_t    *pc;       /* PHY software state */
    uint16 data16;
    uint16 mask16;

    pc = EXT_PHY_SW_STATE(unit, port);

    if (PHY84740_SINGLE_PORT_MODE(pc)) {
        return SOC_E_UNAVAIL;
    } else {  /* quad-port mode */
        /* rx inversion */
        data16 = invert? PHY84740_USER_PRBS_RX_INVERT: 0;
        mask16 = PHY84740_USER_PRBS_RX_INVERT;
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit, pc,
                  PHY84740_USER_PRBS_CONTROL_0_REGISTER, data16,
                           mask16));
    }
    return SOC_E_NONE;
}

STATIC int
_phy_84740_control_prbs_polynomial_set(int unit, soc_port_t port,int poly_ctrl,
         int tx)   /* tx or rx */
{
    phy_ctrl_t    *pc;       /* PHY software state */
    uint16 data16;
    uint16 mask16;

        /*
         * poly_ctrl:
         *  0x0 = prbs7
         *  0x1 = prbs15
         *  0x2 = prbs23
         *  0x3 = prbs31
         */
    pc = EXT_PHY_SW_STATE(unit, port);

    if (PHY84740_SINGLE_PORT_MODE(pc)) {
        /* only two types available PRBS9 and PRBS31 */
        if (poly_ctrl == 3) {
            data16 = PHY84740_40G_PRBS31;
        } else {
            data16 = PHY84740_40G_PRBS9;
        }

        SOC_IF_ERROR_RETURN
            (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit, pc, 
                     PHY84740_FORTYG_PRBS_PATTERN_TESTING_CONTROL_STATUS, 
                          data16,
                          PHY84740_40G_PRBS9 | PHY84740_40G_PRBS31));
    } else {  /* quad-port mode */
        /* 001 : prbs7 
           010 : prbs9
           011 : prbs11
           100 : prbs15
           101 : prbs23
           110 : prbs31
         */

        if (poly_ctrl == 0) {
            data16 = 1;
        } else if (poly_ctrl == 1) {
            data16 = 4;
        } else if (poly_ctrl == 2) {
            data16 = 5;
        } else if (poly_ctrl == 3) {
            data16 = 6;
        } else {
            return SOC_E_PARAM;
        }  
        mask16 = PHY84740_USER_PRBS_TYPE_MASK;

        /* default is tx */
        if (!tx) {  /* rx */
            data16 = data16 << 12;
            mask16 = mask16 << 12;
        }
            
        /* both tx/rx types */  
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit, pc, 
                    PHY84740_USER_PRBS_CONTROL_0_REGISTER, 
               data16, mask16));
    }
    return SOC_E_NONE;
}
STATIC int
_phy_84740_control_prbs_enable_set(int unit, soc_port_t port, int enable)
{
    phy_ctrl_t    *pc;       /* PHY software state */
    int lane;
    int repeater_mode;
    uint16 data16;

    pc = EXT_PHY_SW_STATE(unit, port);

    if (PHY84740_SINGLE_PORT_MODE(pc)) {
        for (lane = 0; lane < NUM_LANES; lane++) {
            SOC_IF_ERROR_RETURN
                (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc,
                                            PHY84740_PMAD_AER_ADDR_REG, lane));

            /* GENSIG_8071_REGISTER Clocks enable */
            SOC_IF_ERROR_RETURN
                (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc, 
                     PHY84740_GENSIG_8071_REGISTER, enable? 0x3:0));

            /* RESET_CONTROL_REGISTER */
            SOC_IF_ERROR_RETURN
                (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc, 
                               PHY84740_RESET_CONTROL_REGISTER, 0x0));

            /* DATA_MUX_SEL_CNTRL_REGISTER */
            SOC_IF_ERROR_RETURN
                (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0xcd58, enable?0x2:0));

            /* 40G_PRBS_PATTERN_TESTING_CONTROL_STATUS */
            SOC_IF_ERROR_RETURN
                (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit, pc, 
                     PHY84740_FORTYG_PRBS_PATTERN_TESTING_CONTROL_STATUS, 
                     enable?PHY84740_40G_PRBS_ENABLE:0,
                     0xf));
        }
    } else { /* quad-port mode */
        SOC_IF_ERROR_RETURN
            (READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0xc81d, &data16));

        repeater_mode = ((data16 & 0x6) == 0x6)? TRUE: FALSE;
        if (repeater_mode) {
            /* Enable retimer prbs clocks inside SFI */
            SOC_IF_ERROR_RETURN
                (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit, pc, 
                          PHY84740_GENSIG_8071_REGISTER, 
                                enable?0x3:0,0x3));

            /* Enable the retimer datapath */
            SOC_IF_ERROR_RETURN
                (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0xcd58, 
                            enable?0x2:0,0x3));
        }
        /* clear PRBS control if disabled */
        if (!enable) {
            SOC_IF_ERROR_RETURN
                (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit, pc, 
                           PHY84740_USER_PRBS_CONTROL_0_REGISTER, 0,
                             PHY84740_USER_PRBS_TYPE_MASK |
                             PHY84740_USER_PRBS_TYPE_MASK << 12)); /* rx type*/
        }         

        /*  program the PRBS enable */
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit, pc, 
                     PHY84740_USER_PRBS_CONTROL_0_REGISTER, 
                     enable? PHY84740_USER_PRBS_ENABLE: 0,PHY84740_USER_PRBS_ENABLE));
    }
    return SOC_E_NONE;
}

STATIC int
_phy_84740_control_prbs_enable_get(int unit, soc_port_t port, uint32 *value)
{
    uint16 data16;
    phy_ctrl_t    *pc;       /* PHY software state */

    pc = EXT_PHY_SW_STATE(unit, port);

    if (PHY84740_SINGLE_PORT_MODE(pc)) {
        SOC_IF_ERROR_RETURN
            (READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, 
                   PHY84740_FORTYG_PRBS_PATTERN_TESTING_CONTROL_STATUS, &data16));
        if ((data16 & PHY84740_40G_PRBS_ENABLE) == PHY84740_40G_PRBS_ENABLE) {
            *value = TRUE;
        } else {
            *value = FALSE;
        }
    } else {  /* quad-port mode */
        SOC_IF_ERROR_RETURN
            (READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, 
                   PHY84740_USER_PRBS_CONTROL_0_REGISTER, &data16));
        if (data16 & PHY84740_USER_PRBS_ENABLE) {
            *value = TRUE;
        } else {
            *value = FALSE;
        }
    }

    return SOC_E_NONE;
}
STATIC int
_phy_84740_control_prbs_polynomial_get(int unit, soc_port_t port, uint32 *value)
{
    phy_ctrl_t    *pc;       /* PHY software state */
    uint16 poly_ctrl;

    pc = EXT_PHY_SW_STATE(unit, port);

    if (PHY84740_SINGLE_PORT_MODE(pc)) {
        SOC_IF_ERROR_RETURN
            (READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, 
                   PHY84740_FORTYG_PRBS_PATTERN_TESTING_CONTROL_STATUS, &poly_ctrl));
        if (poly_ctrl & PHY84740_40G_PRBS31) {
            *value = 3;
        } else {
            *value = 0;  /* 0 indicates PRBS9 */
        }
    } else {  /* quad-port mode */
        SOC_IF_ERROR_RETURN
            (READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, 
                       PHY84740_USER_PRBS_CONTROL_0_REGISTER, &poly_ctrl));
        poly_ctrl &= PHY84740_USER_PRBS_TYPE_MASK;

        if (poly_ctrl == 1) {
            *value = 0;
        } else if (poly_ctrl == 4) {
            *value = 1;
        } else if (poly_ctrl == 5) {
            *value = 2;
        } else if (poly_ctrl == 6) {
            *value = 3;
        } else {
            *value = poly_ctrl;
        }
    }

    return SOC_E_NONE;
}

STATIC int
_phy_84740_control_prbs_tx_invert_data_get(int unit, soc_port_t port, uint32 *value)
{
    phy_ctrl_t    *pc;       /* PHY software state */
    uint16 data16;

    pc = EXT_PHY_SW_STATE(unit, port);

    if (PHY84740_SINGLE_PORT_MODE(pc)) {
        return SOC_E_UNAVAIL;
    } else {  /* quad-port mode */
        SOC_IF_ERROR_RETURN
            (READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, 
                      PHY84740_USER_PRBS_CONTROL_0_REGISTER, &data16));
        if (data16 & PHY84740_USER_PRBS_TX_INVERT) {
            *value = TRUE;
        } else {
            *value = FALSE;
        }
    }
    return SOC_E_NONE;
}

STATIC int
_phy_84740_control_prbs_rx_status_get(int unit, soc_port_t port, uint32 *value)
{
    phy_ctrl_t    *pc;       /* PHY software state */
    uint16 data16;
    int lane;

    pc = EXT_PHY_SW_STATE(unit, port);

    *value = 0;
    if (PHY84740_SINGLE_PORT_MODE(pc)) {
        for (lane = 0; lane < 4; lane++) {
            SOC_IF_ERROR_RETURN
                (READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, 
                     PHY84740_FORTYG_PRBS_RECEIVE_ERROR_COUNTER_LANE0 + lane, &data16));
            if (!(data16 & 0xfff)) {
                /* PRBS is in sync */
                continue;
            } else {
                /* Get errors */
                *value += (data16 & 0xfff);
            }
        }
    } else {  /* quad-port mode */
        SOC_IF_ERROR_RETURN
            (READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, 
                          PHY84740_USER_PRBS_STATUS_0_REGISTER, &data16));
        if (data16 == 0x8000) { /* PRBS lock and error free */
            *value = 0;
        } else if (!(data16 & 0x8000)) {
            *value = -1;
        } else {
            *value = data16 & 0x7fff;
        }
    }

    return SOC_E_NONE;
}

STATIC int
_phy_84740_control_edc_mode_get(int unit, soc_port_t port, uint32 *value)
{
    uint16         data16;
    phy_ctrl_t    *pc;       /* PHY software state */

    pc = EXT_PHY_SW_STATE(unit, port);

    /* get EDC mode */     
    SOC_IF_ERROR_RETURN
      (READ_PHY84740_MMF_PMA_PMD_REG(unit,pc, 0xCA1A,&data16));

    *value = data16 & PHY84740_EDC_MODE_MASK;

    return SOC_E_NONE;
}


STATIC int
_phy_84740_remote_loopback_set(int unit, soc_port_t port,int intf,uint32 enable)
{
    phy_ctrl_t    *pc;       /* PHY software state */

    pc = EXT_PHY_SW_STATE(unit, port);

    if (intf == PHY_DIAG_INTF_SYS) {
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit,pc, 0xcd0a,
               enable? 1:0, 1));
    }
    return SOC_E_NONE;
}
/*
 * Function:
 *      phy_84740_control_set
 * Purpose:
 *      Configure PHY device specific control fucntion.
 * Parameters:
 *      unit  - StrataSwitch unit #.
 *      port  - StrataSwitch port #.
 *      type  - Control to update
 *      value - New setting for the control
 * Returns:
 *      SOC_E_NONE
 */
STATIC int
_phy_84740_control_set(int unit, soc_port_t port, int intf,int lane,
                     soc_phy_control_t type, uint32 value)
{
    phy_ctrl_t    *pc;       /* PHY software state */
    int rv;

    pc = EXT_PHY_SW_STATE(unit, port);

    if ((type < 0) || (type >= SOC_PHY_CONTROL_COUNT)) {
        return SOC_E_PARAM;
    }

    rv = SOC_E_UNAVAIL;
    if (intf == PHY_DIAG_INTF_SYS) {
        /* if is targeted to the system side */
        SOC_IF_ERROR_RETURN
            (PHY84740_XFI(unit,pc));
    }
    switch(type) {
    case SOC_PHY_CONTROL_PREEMPHASIS:
        /* fall through */
    case SOC_PHY_CONTROL_PREEMPHASIS_LANE0:
        /* fall through */
    case SOC_PHY_CONTROL_PREEMPHASIS_LANE1:
        /* fall through */
    case SOC_PHY_CONTROL_PREEMPHASIS_LANE2:
        /* fall through */
    case SOC_PHY_CONTROL_PREEMPHASIS_LANE3:
        /* fall through */
    case SOC_PHY_CONTROL_DRIVER_CURRENT:
        /* fall through */
    case SOC_PHY_CONTROL_DRIVER_CURRENT_LANE0:
        /* fall through */
    case SOC_PHY_CONTROL_DRIVER_CURRENT_LANE1:
        /* fall through */
    case SOC_PHY_CONTROL_DRIVER_CURRENT_LANE2:
        /* fall through */
    case SOC_PHY_CONTROL_DRIVER_CURRENT_LANE3:
        /* fall through */
    case SOC_PHY_CONTROL_PRE_DRIVER_CURRENT:
        /* fall through */
    case SOC_PHY_CONTROL_PRE_DRIVER_CURRENT_LANE0:
        /* fall through */
    case SOC_PHY_CONTROL_PRE_DRIVER_CURRENT_LANE1:
        /* fall through */
    case SOC_PHY_CONTROL_PRE_DRIVER_CURRENT_LANE2:
        /* fall through */
    case SOC_PHY_CONTROL_PRE_DRIVER_CURRENT_LANE3:
        rv = _phy_84740_control_tx_driver_set(unit, port, type, value);
        break;
    case SOC_PHY_CONTROL_EDC_MODE:
        rv = _phy_84740_control_edc_mode_set(unit,port,value);
        break;
    case SOC_PHY_CONTROL_CLOCK_ENABLE:
        rv = _phy_84740_control_recovery_clock_set(unit,port,value);
        break;
    case SOC_PHY_CONTROL_CLOCK_FREQUENCY:
        rv = _phy_84740_control_recovery_clock_freq_set(unit,port,value);
        break;
    case SOC_PHY_CONTROL_PRBS_POLYNOMIAL:
        /* on line side, rx poly type is auto-detected. On system side,
         * need to enable the rx type as well.
         */
        rv = _phy_84740_control_prbs_polynomial_set(unit, port, value, TRUE);
        if (intf == PHY_DIAG_INTF_SYS) {
            rv = _phy_84740_control_prbs_polynomial_set(unit, port,value,FALSE);
        }
        break;
    case SOC_PHY_CONTROL_PRBS_TX_INVERT_DATA:
        /* on line side, rx invertion is auto-detected. On system side,
         * need to enable the rx as well.
         */
        rv = _phy_84740_control_prbs_tx_invert_data_set(unit, port, value);
        if (intf == PHY_DIAG_INTF_SYS) {
            rv = _phy_84740_control_prbs_rx_invert_data_set(unit, port, value);
        }
        break;
    case SOC_PHY_CONTROL_PRBS_TX_ENABLE:
        /* fall through */
    case SOC_PHY_CONTROL_PRBS_RX_ENABLE:
        /* tx/rx is enabled at the same time. no seperate control */
        rv = _phy_84740_control_prbs_enable_set(unit, port, value);
        break;
    case SOC_PHY_CONTROL_LOOPBACK_REMOTE:
        rv = _phy_84740_remote_loopback_set(unit, port,PHY_DIAG_INTF_SYS,value);
        break;
    default:
        rv = SOC_E_UNAVAIL;
        break;
    }
    if (intf == PHY_DIAG_INTF_SYS) {
        /* if it is targeted to the system side, switch back */
        SOC_IF_ERROR_RETURN
            (PHY84740_MMF(unit,pc));
    }
    return rv;
}
STATIC int
phy_84740_control_set(int unit, soc_port_t port,
                     soc_phy_control_t type, uint32 value)
{
    phy_ctrl_t    *pc;       /* PHY software state */
    int intf;

    pc = EXT_PHY_SW_STATE(unit, port);

    intf = REVERSE_MODE(pc)? PHY_DIAG_INTF_SYS: PHY_DIAG_INTF_LINE;
    SOC_IF_ERROR_RETURN
        (_phy_84740_control_set(unit, port, intf,
            PHY_DIAG_LN_DFLT, type, value));
    return SOC_E_NONE;
}
/*
 * Function:
 *      phy_84740_control_get
 * Purpose:
 *      Get current control settign of the PHY.
 * Parameters:
 *      unit  - StrataSwitch unit #.
 *      port  - StrataSwitch port #.
 *      type  - Control to update
 *      value - (OUT)Current setting for the control
 * Returns:
 *      SOC_E_NONE
 */
STATIC int
_phy_84740_control_get(int unit, soc_port_t port, int intf,int lane,
                     soc_phy_control_t type, uint32 *value)
{
    phy_ctrl_t    *pc;       /* PHY software state */
    int rv;
    uint16 data16;

    pc = EXT_PHY_SW_STATE(unit, port);

    if ((type < 0) || (type >= SOC_PHY_CONTROL_COUNT)) {
        return SOC_E_PARAM;
    }

    if (intf == PHY_DIAG_INTF_SYS) {
        /* targeted to the system side */
        SOC_IF_ERROR_RETURN
            (PHY84740_XFI(unit,pc));
    }
    rv = SOC_E_UNAVAIL;
    switch(type) {
    case SOC_PHY_CONTROL_PREEMPHASIS:
        /* fall through */
    case SOC_PHY_CONTROL_PREEMPHASIS_LANE0:
        /* fall through */
    case SOC_PHY_CONTROL_PREEMPHASIS_LANE1:
        /* fall through */
    case SOC_PHY_CONTROL_PREEMPHASIS_LANE2:
        /* fall through */
    case SOC_PHY_CONTROL_PREEMPHASIS_LANE3:
        /* fall through */
    case SOC_PHY_CONTROL_DRIVER_CURRENT:
        /* fall through */
    case SOC_PHY_CONTROL_DRIVER_CURRENT_LANE0:
        /* fall through */
    case SOC_PHY_CONTROL_DRIVER_CURRENT_LANE1:
        /* fall through */
    case SOC_PHY_CONTROL_DRIVER_CURRENT_LANE2:
        /* fall through */
    case SOC_PHY_CONTROL_DRIVER_CURRENT_LANE3:
        /* fall through */
    case SOC_PHY_CONTROL_PRE_DRIVER_CURRENT:
        /* fall through */
    case SOC_PHY_CONTROL_PRE_DRIVER_CURRENT_LANE0:
        /* fall through */
    case SOC_PHY_CONTROL_PRE_DRIVER_CURRENT_LANE1:
        /* fall through */
    case SOC_PHY_CONTROL_PRE_DRIVER_CURRENT_LANE2:
        /* fall through */
    case SOC_PHY_CONTROL_PRE_DRIVER_CURRENT_LANE3:
        rv = _phy_84740_control_tx_driver_get(unit, port, type, value);
        break;
    case SOC_PHY_CONTROL_EDC_MODE:
        rv = _phy_84740_control_edc_mode_get(unit,port,value);
        break;
    case SOC_PHY_CONTROL_CLOCK_ENABLE:
        SOC_IF_ERROR_RETURN
           (READ_PHY84740_MMF_PMA_PMD_REG(unit,pc,PHY84740_PMAD_RECOVERY_CLK_REG, 
                                          &data16));
        *value = (data16 & PHY84740_RECOVERY_CLK_ENABLE_MASK)? TRUE: FALSE; 
        rv = SOC_E_NONE;
        break;
    case SOC_PHY_CONTROL_PRBS_POLYNOMIAL:
        rv = _phy_84740_control_prbs_polynomial_get(unit, port, value);
        break;
    case SOC_PHY_CONTROL_PRBS_TX_INVERT_DATA:
        rv = _phy_84740_control_prbs_tx_invert_data_get(unit, port, value);
        break;
    case SOC_PHY_CONTROL_PRBS_TX_ENABLE:
        /* fall through */
    case SOC_PHY_CONTROL_PRBS_RX_ENABLE:
        rv = _phy_84740_control_prbs_enable_get(unit, port, value);
        break;
    case SOC_PHY_CONTROL_PRBS_RX_STATUS:
        rv = _phy_84740_control_prbs_rx_status_get(unit, port, value);
        break;
    case SOC_PHY_CONTROL_CLOCK_FREQUENCY:
        SOC_IF_ERROR_RETURN
            (READ_PHY84740_MMF_PMA_PMD_REG(unit,pc,PHY84740_PMAD_RECOVERY_CLK_FREQ_REG, 
                   &data16));
        switch ((data16 >> PHY84740_RECOVERY_CLK_FREQ_SHIFT) & 0x3) {
            case 0x1:
                *value = 125000;
                break;
            case 0x2:
                *value = 195312;
                break;
            case 0x3:
                *value = 156250;
                break;
            default:
                *value = 0x0;
                break;
        }
        rv = SOC_E_NONE;
        break;

    default:
        rv = SOC_E_UNAVAIL;
        break;
    }

    if (intf == PHY_DIAG_INTF_SYS) {
        /* if it is targeted to the system side, switch back */
        SOC_IF_ERROR_RETURN
            (PHY84740_MMF(unit,pc));
    }
    return rv;
}
STATIC int
phy_84740_control_get(int unit, soc_port_t port,
                     soc_phy_control_t type, uint32 *value)
{
    int intf;
    phy_ctrl_t    *pc;       /* PHY software state */

    pc = EXT_PHY_SW_STATE(unit, port);

    intf = REVERSE_MODE(pc)? PHY_DIAG_INTF_SYS: PHY_DIAG_INTF_LINE;

    SOC_IF_ERROR_RETURN
        (_phy_84740_control_get(unit, port, intf,
            PHY_DIAG_LN_DFLT, type, value));
    return SOC_E_NONE;
}

STATIC int
phy_84740_diag_ctrl(
   int unit, /* unit */
   soc_port_t port, /* port */
   uint32 inst, /* the specific device block the control action directs to */
   int op_type,  /* operation types: read,write or command sequence */
   int op_cmd,   /* command code */
   void *arg)     /* command argument  */
{
    phy_ctrl_t       *pc;
    int lane;
    int intf;

    pc = EXT_PHY_SW_STATE(unit, port);

    soc_cm_debug(DK_PHY,"phy_84740_diag_ctrl: u=%d p=%d ctrl=0x%x\n", 
                 unit, port,op_cmd);
   
    lane = PHY_DIAG_INST_LN(inst);
    intf = PHY_DIAG_INST_INTF(inst);
    if (intf == PHY_DIAG_INTF_DFLT) {
        intf = PHY_DIAG_INTF_LINE;
    }

    if (op_type == PHY_DIAG_CTRL_GET) { 
        SOC_IF_ERROR_RETURN
            (_phy_84740_control_get(unit, port, intf, lane,
                  op_cmd,(uint32 *)arg));
    } else if (op_type == PHY_DIAG_CTRL_SET) {
        SOC_IF_ERROR_RETURN
            (_phy_84740_control_set(unit, port, intf, lane,
                  op_cmd,PTR_TO_INT(arg)));
    }
    return SOC_E_NONE;
}

/*
 * Function:
 *      phy_84740_speed_set
 * Purpose:
 *      Set PHY speed
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      speed - link speed in Mbps
 * Returns:
 *      SOC_E_NONE
 */

STATIC int
phy_84740_speed_set(int unit, soc_port_t port, int speed)
{
    phy_ctrl_t  *pc;
    phy_ctrl_t  *int_pc;
    int rv = SOC_E_NONE;

    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(port); 

    int_pc = INT_PHY_SW_STATE(unit, port);
    pc = EXT_PHY_SW_STATE(unit, port);

    soc_cm_debug(DK_PHY,"phy_84740_speed_set: u=%d p=%d speed=%d\n", 
                 unit, port,speed);

    if (PHY84740_SINGLE_PORT_MODE(pc)) {
        if (speed != 40000) {
            return SOC_E_PARAM;
        }

        /* single port mode: doesn't use PCS. should be already in repeater mode */
        /* it is always in 40G speed, make sure internal device is in 40G */
        if (NULL != int_pc) {
            if (REVERSE_MODE(pc)) {
                SOC_IF_ERROR_RETURN
                    (PHY_INTERFACE_SET(int_pc->pd, unit, port, SOC_PORT_IF_SR));
            } else {
                if (soc_property_port_get(unit, port, spn_PHY_LONG_XFI, 0)) {
                        rv = PHY_INTERFACE_SET(int_pc->pd, unit, port, SOC_PORT_IF_CR);
                }
            }

            rv = PHY_SPEED_SET(int_pc->pd, unit, port, speed);
        }
        return rv;
    }

    /* quad lane mode */
    if ((!(speed == 10000 || speed == 1000)) &&
        (!PCS_REPEATER(pc))) {
        return SOC_E_PARAM;
    }

    if (speed == 10000) {
        SOC_IF_ERROR_RETURN
            (WRITE_PHY84740_MMF_PMA_PMD_REG(unit,pc, PHY84740_PMAD_CTRL_REG,
                                          MII_CTRL_SS_MSB |  MII_CTRL_SS_LSB));
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit,pc, PHY84740_PMAD_CTRL2_REG,
                                        PHY84740_PMAD_CTRL2r_PMA_TYPE_10G_LRM, 
                                        PHY84740_PMAD_CTRL2r_PMA_TYPE_MASK));

    } else {
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit,pc, PHY84740_PMAD_CTRL2_REG,
                                        PHY84740_PMAD_CTRL2r_PMA_TYPE_1G_KX,
                                        PHY84740_PMAD_CTRL2r_PMA_TYPE_MASK));
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit,pc, MII_CTRL_REG,
                                        0, MII_CTRL_SS_LSB));
    }
    /* need to set the internal phy's speed accordingly */

    if (NULL != int_pc) {
        if (REVERSE_MODE(pc)) {
            SOC_IF_ERROR_RETURN
                (PHY_INTERFACE_SET(int_pc->pd, unit, port, SOC_PORT_IF_SR));
        } else {
            /* For long XFI channel, it is best to use SFP+ DAC mode 
             * in the serdes side to provide adequate Rx equalization
             */
            if (soc_property_port_get(unit, port, spn_PHY_LONG_XFI, 0)) {
                if (!PHY84740_SINGLE_PORT_MODE(pc)) {
                    rv = PHY_INTERFACE_SET(int_pc->pd, unit, port, SOC_PORT_IF_CR);
                }
            }
        }
        rv = PHY_SPEED_SET(int_pc->pd, unit, port, speed);
    }
    return rv;
}

/*
 * Function:
 *      phy_84740_speed_get
 * Purpose:
 *      Get PHY speed
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      speed - current link speed in Mbps
 * Returns:
 *      SOC_E_NONE
 */

STATIC int
phy_84740_speed_get(int unit, soc_port_t port, int *speed)
{
    phy_ctrl_t  *pc;
    uint16       speed_val;

    pc = EXT_PHY_SW_STATE(unit, port);

    if (PCS_REPEATER(pc)) {
        phy_ctrl_t    *int_pc;

        int_pc = INT_PHY_SW_STATE(unit, port);
        SOC_IF_ERROR_RETURN
            (PHY_SPEED_GET(int_pc->pd, unit, port, speed));
        return SOC_E_NONE;
    }

    /* this register contains the speed info for both single/quad modes */
    SOC_IF_ERROR_RETURN
        (READ_PHY84740_MMF_PMA_PMD_REG(unit,pc, 0xc81f, &speed_val));

    if (speed_val & (1 << 5)) {
        *speed = 40000;
    } else if (speed_val & (1 << 4)) {
        *speed = 10000;
    } else if (speed_val & (1 << 2)) {
        *speed = 1000;
    } else { /* default */
        if (PHY84740_SINGLE_PORT_MODE(pc)) {
            *speed = 40000;
        } else {
            *speed = 10000;
        }
    }
    return SOC_E_NONE;
}

/*
 * Function:
 *      phy_84740_interface_set
 * Purpose:
 *      Set either CR4 interface or SR4/LR4. The valid interface are SOC_PORT_IF_CR4
 *      and SOC_PORT_IF_XLAUI which put device in default mode: SR4/LR4 mode
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      pif - one of SOC_PORT_IF_*
 * Returns:
 *      SOC_E_NONE - success
 *      SOC_E_UNAVAIL - unsupported interface
 */

STATIC int
phy_84740_interface_set(int unit, soc_port_t port, soc_port_if_t pif)
{
    phy_ctrl_t     *pc;
    phy_ctrl_t  *int_pc;
    uint16 sig_lvl_reg[4];
    uint16 rxlos_reg[4];
    uint16 pol_reg[4];
    int lane;
    soc_port_ability_t ability;

    int_pc = INT_PHY_SW_STATE(unit, port);
    pc = EXT_PHY_SW_STATE(unit, port);

    if (int_pc != NULL) {
        if (REVERSE_MODE(pc)) {
            SOC_IF_ERROR_RETURN
                (PHY_INTERFACE_SET(int_pc->pd, unit, port, pif));
            SOC_IF_ERROR_RETURN(
                PHY_ABILITY_ADVERT_GET(int_pc->pd, unit, port, &ability));
            SOC_IF_ERROR_RETURN(
                PHY_ABILITY_ADVERT_SET(int_pc->pd, unit, port, &ability));
        }
    }

    if (!(PHY84740_SINGLE_PORT_MODE(pc))) {
        return phy_null_interface_set(unit,port,pif);
    }

    switch (pif) {
    case SOC_PORT_IF_CR4:
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit, pc, PHY84740_PMAD_CHIP_MODE_REG,
                   PHY84740_PMAD_DAC_MODE_MASK, PHY84740_PMAD_DAC_MODE_MASK));
        break;
 
    case SOC_PORT_IF_XLAUI:
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit, pc, PHY84740_PMAD_CHIP_MODE_REG,
                   0, PHY84740_PMAD_DAC_MODE_MASK));
        break;
    default:
        break;
    }
    /* After switching mode, user need to redo the configuration */
    if (pif == SOC_PORT_IF_CR4 || pif == SOC_PORT_IF_XLAUI) {

        /* save the register settings */
        for (lane = 0; lane < NUM_LANES; lane++) {
            /* point to the each lane starting at lane0 */
            SOC_IF_ERROR_RETURN
                    (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc,
                                PHY84740_PMAD_AER_ADDR_REG, lane));
            SOC_IF_ERROR_RETURN(
                READ_PHY84740_MMF_PMA_PMD_REG(unit,pc,
                                         PHY84740_DIG_CTRL_REG,
                                         &pol_reg[lane]));

            SOC_IF_ERROR_RETURN
                (READ_PHY84740_MMF_PMA_PMD_REG(unit,pc,
                             PHY84740_PMAD_OPTICAL_CFG_REG,
                             &rxlos_reg[lane]));
            SOC_IF_ERROR_RETURN
                (READ_PHY84740_MMF_PMA_PMD_REG(unit,pc,
                             PHY84740_PMAD_OPTICAL_SIG_LVL_REG,
                             &sig_lvl_reg[lane]));
        }

        /* perform IEEE reset */
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY84740_PMA_PMD_CTRLr(pc->unit, pc,
                              MII_CTRL_RESET, MII_CTRL_RESET));

        sal_udelay(100); /* Wait for 100us */

        /* restore back saved register values */
        for (lane = 0; lane < NUM_LANES; lane++) {
            /* point to the each lane starting at lane0 */
            SOC_IF_ERROR_RETURN
                    (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc,
                                PHY84740_PMAD_AER_ADDR_REG, lane));
            SOC_IF_ERROR_RETURN(
                WRITE_PHY84740_MMF_PMA_PMD_REG(unit,pc,
                                         PHY84740_DIG_CTRL_REG,
                                         pol_reg[lane]));

            SOC_IF_ERROR_RETURN
                (WRITE_PHY84740_MMF_PMA_PMD_REG(unit,pc,
                             PHY84740_PMAD_OPTICAL_CFG_REG,
                             rxlos_reg[lane]));
            SOC_IF_ERROR_RETURN
                (WRITE_PHY84740_MMF_PMA_PMD_REG(unit,pc,
                             PHY84740_PMAD_OPTICAL_SIG_LVL_REG,
                             sig_lvl_reg[lane]));
        }

        /* redo Polarity configuration */
        SOC_IF_ERROR_RETURN
            (_phy_84740_polarity_flip_set(unit,port));
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *      phy_84740_interface_get
 * Purpose:
 *      Get the current operating mode of the PHY.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      pif - (OUT) one of SOC_PORT_IF_*
 * Returns:
 *      SOC_E_NONE
 */

STATIC int
phy_84740_interface_get(int unit, soc_port_t port, soc_port_if_t *pif)
{
    phy_ctrl_t    *pc;
    uint16 mode;

    pc = EXT_PHY_SW_STATE(unit, port);

    if (!(PHY84740_SINGLE_PORT_MODE(pc))) {
        *pif = SOC_PORT_IF_XAUI;
        return SOC_E_NONE;
    }

    SOC_IF_ERROR_RETURN
        (READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, PHY84740_PMAD_CHIP_MODE_REG,&mode));
    if (mode & PHY84740_PMAD_DAC_MODE_MASK) {
        *pif = SOC_PORT_IF_CR4;
    } else {
        *pif = SOC_PORT_IF_XLAUI;
    }

    return SOC_E_NONE;
}

/*
 *  Function:  *      phy_84740_rom_wait
 *
 *  Purpose:
 *      Wait for data to be written to the SPI-ROM.
 *  Input:
 *      unit, port
 */
STATIC int
phy_84740_rom_wait(int unit, int port)
{
    uint16     rd_data;     
    uint16     wr_data;
    uint16     data16;
    int        count;     
    phy_ctrl_t *pc;
    soc_timeout_t to;     
    int        rv;
    int        SPI_READY; 

    rv = SOC_E_NONE;     
    pc = EXT_PHY_SW_STATE(unit, port);

    rd_data = 0;
    soc_timeout_init(&to, WR_TIMEOUT, 0);
    do {
        /* Write SPI Control Register Read Command. */
        count = 1;
        wr_data = ((RD_CPU_CTRL_REGS * 0x0100) | count);
        SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));
        wr_data = SPI_CTRL_1_L;
        SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));

        /* extra reads to clear the status */
        SOC_IF_ERROR_RETURN
            (READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0xCA13, &data16));
        SOC_IF_ERROR_RETURN
            (READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0x9003, &data16));

        if (rd_data & 0x0100) {
            break;
        }
    } while (!soc_timeout_check(&to));
    if (!(rd_data & 0x0100)) {
        soc_cm_debug(DK_ERR,"phy_84740_rom_program: u = %d p = %d "
                            "timeout 1\n", unit, port);
        return SOC_E_TIMEOUT;
    }

    SPI_READY = 1;
    while (SPI_READY == 1) {
        /* Set-up SPI Controller To Receive SPI EEPROM Status. */
        count = 1;
        wr_data = ((WR_CPU_CTRL_REGS * 0x0100) | count);
        SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));
        wr_data = SPI_CTRL_2_H;
        SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));
        wr_data = 0x0100;
        SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));
        /* Fill-up SPI Transmit Fifo To check SPI Status. */
        count = 2;
        wr_data = ((WR_CPU_CTRL_FIFO * 0x0100) | count);
        SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));
        /* Write Tx Fifo Register Address. */
        wr_data = SPI_TXFIFO;
        SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));

        /* Write SPI Tx Fifo Control Word-1. */
        wr_data = ((1 * 0x0100) | MSGTYPE_HRD);
        SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));
        /* Write SPI Tx Fifo Control Word-2. */
        wr_data = RDSR_OPCODE;
        SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));
        /* Write SPI Control Register Write Command. */
        count = 2;
        wr_data = ((WR_CPU_CTRL_FIFO * 0x0100) | count);
        SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));
        /* Write SPI Control -1 Register Address. */
        wr_data = SPI_CTRL_1_L;
        SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));
        /* Write SPI Control -1 Register Word-1. */
        wr_data = 0x0101;
        SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));
        /* Write SPI Control -1 Register Word-2. */
        wr_data = 0x0100;
        SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));
        /* Write SPI Control Register Write Command. */
        count = 1;
        wr_data = ((WR_CPU_CTRL_REGS * 0x0100) | count);
        SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));
        /* Write SPI Control -1 Register Address. */
        wr_data = SPI_CTRL_1_H;
        SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));
        /* Write SPI Control -1 Register Word-2. */
        wr_data = 0x0103;
        SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));
        /* Wait For 64 bytes To be written.   */
        rd_data = 0x0000;
        soc_timeout_init(&to, WR_TIMEOUT, 0);
        do {
            count = 1;
            wr_data = ((RD_CPU_CTRL_REGS * 0x0100) | count);
            SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));
            wr_data = SPI_CTRL_1_L;
            SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));

            /* extra reads to clear the status */
            SOC_IF_ERROR_RETURN
                (READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0xCA13, &data16));
            SOC_IF_ERROR_RETURN
                (READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0x9003, &data16));

            if (rd_data & 0x0100) {
                break;
            }
        } while (!soc_timeout_check(&to));
        if (!(rd_data & 0x0100)) {
            soc_cm_debug(DK_ERR,"phy_84740_rom_program: u = %d p = %d "
                                "timeout 2\n", unit, port);
            return SOC_E_TIMEOUT;
        }
        /* Write SPI Control Register Read Command. */
        count = 1;
        wr_data = ((RD_CPU_CTRL_REGS * 0x0100) | count);
        SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));
        /* Write SPI Control -1 Register Address. */
        wr_data = SPI_RXFIFO;

        SOC_IF_ERROR_RETURN         
            (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0xCA12, wr_data));
        SOC_IF_ERROR_RETURN
            (READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0xCA13, &rd_data));

        /* Clear LASI Message Out Status. */
        SOC_IF_ERROR_RETURN
            (READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0x9003, &wr_data));

        if ((rd_data & 0x1) == 0) {
            SPI_READY = 0;
        }
        /* extra reads to clear the status */
        SOC_IF_ERROR_RETURN
            (READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0xCA13, &data16));
        SOC_IF_ERROR_RETURN
            (READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0x9003, &data16));
    } /* SPI_READY  */
    return SOC_E_NONE;
}


/*
 * Function:
 *      phy_84740_rom_write_enable_set
 *
 * Purpose:
 *      Enable disable protection on SPI_EEPROM
 *
 * Input:
 *      unit
 *      port
 *      enable
 * Output:
 *      SOC_E_xxx
 *
 * Notes:
 *          25AA256 256Kbit Serial EEPROM
 *          STATUS Register
 *          +------------------------------------------+
 *          | WPEN | x | x | x | BP1 | BP0 | WEL | WIP |
 *          +------------------------------------------+
 *      BP1 BP0  :   Protected Blocks
 *       0   0   :  Protect None
 *       1   1   :  Protect All
 *
 *      WEL : Write Latch Enable
 *       0  : Do not allow writes
 *       1  : Allow writes
 */

STATIC int
phy_84740_rom_write_enable_set(int unit, int port, int enable)
{
    uint16     rd_data;
    uint16     wr_data;
    uint8      wrsr_data;
    int        count;
    phy_ctrl_t *pc;
    int        rv;

    rv = SOC_E_NONE;
    pc = EXT_PHY_SW_STATE(unit, port);

    /*
     * Write SPI Control Register Write Command.
     */
    count = 2;
    wr_data = ((WR_CPU_CTRL_FIFO * 0x0100) | count);
    SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));

    /*
     * Write SPI Control -2 Register Address.
     */
    wr_data = SPI_CTRL_2_L;
    SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));

    /*
     * Write SPI Control -2 Register Word-1.
     */
    wr_data = 0x8200;
    SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));

    /*
     * Write SPI Control -2 Register Word-2.
     */
    wr_data = 0x0100;
    SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));

    /*
     * Fill-up SPI Transmit Fifo With SPI EEPROM Messages.
     * Write SPI Control Register Write Command.
     */
    count = 4;
    wr_data = ((WR_CPU_CTRL_FIFO * 0x0100) | count);
    SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));

    /*
     * Write Tx Fifo Register Address.
     */
    wr_data = SPI_TXFIFO;
    SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));

    /*
     * Write SPI Tx Fifo Control Word-1.
     */
    wr_data = ((1 * 0x0100) | MSGTYPE_HWR);
    SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));

    /*
     * Write SPI Tx Fifo Control Word-2.
     */
    wr_data = ((MSGTYPE_HWR * 0x0100) | WREN_OPCODE);
    SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));

    /*
     * Write SPI Tx Fifo Control Word-3.
     */
    wr_data = ((WRSR_OPCODE * 0x100) | (0x2));
    SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));

    /*
     * Write SPI Tx Fifo Control Word-4.
     */
    wrsr_data = enable ? 0x2 : 0xc;
    wr_data = ((wrsr_data * 0x0100) | wrsr_data);
    SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));

    /*
     * Write SPI Control Register Write Command.
     */
    count = 2;
    wr_data = ((WR_CPU_CTRL_FIFO * 0x0100) | count);
    SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));

    /*
     * Write SPI Control -1 Register Address.
     */
    wr_data = SPI_CTRL_1_L;
    SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));

    /*
     * Write SPI Control -1 Register Word-1.
     */
    wr_data = 0x0101;
    SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));

    /*
     * Write SPI Control -1 Register Word-2.
     */
    wr_data = 0x0003;
    SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));

    /*
     * Wait For WRSR Command To be written.
     */
    SOC_IF_ERROR_RETURN(phy_84740_rom_wait(unit, port));

    return SOC_E_NONE;
}

STATIC int
_phy84740_init_ucode_bcst(int unit, int port, int cmd)
{
    uint16 data16;
    uint16 mask16;
    uint16 phy_id;
    int j;
    uint16 num_words;
    int i;
    uint16 chip_mode;
    int num_lanes;
    uint8 *fw_ptr;
    int fw_length;
    phy_ctrl_t *pc;

    pc = EXT_PHY_SW_STATE(unit, port);

    if (cmd == PHYCTRL_UCODE_BCST_SETUP) {
        SOC_IF_ERROR_RETURN(
            READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, PHY84740_PMAD_CHIP_MODE_REG,&chip_mode));

        SOC_DEBUG_PRINT((DK_PHY,
                  "PHY84740 BCST start: u=%d p=%d\n", unit, port));
        /* check the current chip mode, switch to quad if in single port mode */
        if ((chip_mode & PHY84740_PMAD_CHIP_MODE_MASK) == PHY84740_PMAD_MODE_40G) {
            SOC_DEBUG_PRINT((DK_PHY,
                  "PHY84740 BCST mode switch: u=%d p=%d\n", unit, port));
            SOC_IF_ERROR_RETURN(
                _phy_84740_single_to_quad_mode(unit,port,NULL));
        }       

        /* program the bcst register. do 4 lanes for single port configuration */
        phy_id = pc->phy_id;
        /* configure all four ports  */
        for (i = 0; i < 4; i++) {
            pc->phy_id = (phy_id & (~0x3)) + i; /* Port 3, 2, 1 and 0 */

            SOC_IF_ERROR_RETURN
                (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0xc8fe, 0xffff));

        }
        /* restore the original phy_id */
        pc->phy_id = phy_id;
        return SOC_E_NONE;
    } else if (cmd == PHYCTRL_UCODE_BCST_uC_SETUP) {
        SOC_DEBUG_PRINT((DK_PHY,
                  "PHY84740 BCST1: u=%d p=%d\n", unit, port));
        /* clear SPA ctrl reg bit 15 and bit 13.
         * bit 15, 0-use MDIO download to SRAM, 1 SPI-ROM download to SRAM
         * bit 13, 0 clear download done status, 1 skip download
         */
        mask16 = (1 << 13) | (1 << 15);
        data16 = 0;
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit, pc, PHY84740_PMAD_SPA_CTRL_REG,
                                        data16,mask16));

        /* set SPA ctrl reg bit 14, 1 RAM boot, 0 internal ROM boot */
        mask16 = 1 << 14;
        data16 = mask16;
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit, pc, PHY84740_PMAD_SPA_CTRL_REG,
                                        data16,mask16));

        /* misc_ctrl1 reg bit 3 to 1 for 32k downloading size */
        mask16 = 1 << 3;
        data16 = mask16;
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit, pc, PHY84740_PMAD_MISC_CTRL1_REG,
                                           data16,mask16));

        /* apply bcst Reset to start download code from MDIO */
        SOC_IF_ERROR_RETURN
            (WRITE_PHY84740_PMA_PMD_CTRLr(unit, pc, 0x8000));
        return SOC_E_NONE;
    } else if (cmd == PHYCTRL_UCODE_BCST_ENABLE) {
        SOC_DEBUG_PRINT((DK_PHY,
                  "PHY84740 BCST2: u=%d p=%d\n", unit, port));

        /* reset clears bcst register */
        /* restore bcst register after reset */
        phy_id = pc->phy_id;
        /* configure all four ports  */
        for (i = 0; i < 4; i++) {
            pc->phy_id = (phy_id & (~0x3)) + i; /* Port 3, 2, 1 and 0 */

            SOC_IF_ERROR_RETURN
                (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0xc8fe, 0xffff));

        }
        /* restore the original phy_id */
        pc->phy_id = phy_id;
        return SOC_E_NONE;
    } else if (cmd == PHYCTRL_UCODE_BCST_LOAD) {
        SOC_DEBUG_PRINT((DK_PHY, "firmware_bcst,device name %s: u=%d p=%d\n",
                         pc->dev_name? pc->dev_name: "NULL", unit, port));
        /* find the right firmware */
        if (pc->dev_name == dev_name_84740) {
            fw_ptr = phy84740_ucode_bin;
            fw_length = phy84740_ucode_bin_len;
        } else if (pc->dev_name == dev_name_84753) {
            fw_ptr = phy84753_ucode_bin;
            fw_length = phy84753_ucode_bin_len;
        } else if (pc->dev_name == dev_name_84754) {
            fw_ptr = phy84754_ucode_bin;
            fw_length = phy84754_ucode_bin_len;
        } else if (pc->dev_name == dev_name_84064) {
            fw_ptr = phy84064_ucode_bin;
            fw_length = phy84064_ucode_bin_len;
        } else {
            /* invalid device name */
            SOC_DEBUG_PRINT((DK_WARN, "firmware_bcst,invalid device name %s: u=%d p=%d\n",
                         pc->dev_name? pc->dev_name: "NULL", unit, port));
            return SOC_E_NONE;
        }

        /* wait for 2ms for M8051 start and 5ms to initialize the RAM */
        sal_usleep(10000); /* Wait for 10ms */

        /* Write Starting Address, where the Code will reside in SRAM */
        data16 = 0x8000;
        SOC_IF_ERROR_RETURN
            (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc, PHY84740_PMAD_M8051_MSGIN_REG, data16));

        /* make sure address word is read by the micro */
        sal_udelay(10); /* Wait for 10us */

        /* Write SPI SRAM Count Size */
        data16 = (fw_length)/2;
        SOC_IF_ERROR_RETURN
            (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc, PHY84740_PMAD_M8051_MSGIN_REG, data16));

        /* make sure read by the micro */
        sal_udelay(10); /* Wait for 10us */

        /* Fill in the SRAM */
        num_words = (fw_length - 1);
        for (j = 0; j < num_words; j+=2) {
            /* Make sure the word is read by the Micro */
            sal_udelay(10);

            data16 = (fw_ptr[j] << 8) | fw_ptr[j+1];

            SOC_IF_ERROR_RETURN
                (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc, PHY84740_PMAD_M8051_MSGIN_REG,
                     data16));
        }
        return SOC_E_NONE;
    } else if (cmd == PHYCTRL_UCODE_BCST_END) {
        SOC_DEBUG_PRINT((DK_PHY,
                  "PHY84740 BCST end: u=%d p=%d\n", unit, port));

        /* make sure last code word is read by the micro */
        sal_udelay(20);

        if (PHY84740_SINGLE_PORT_MODE(pc)) {
            num_lanes = 4;
        } else {
            num_lanes = 1;
        }

        /* first disable bcst mode */
        phy_id = pc->phy_id;
        for (i = 0; i < num_lanes; i++) {
            if (num_lanes > 1) {
                pc->phy_id = (phy_id & (~0x3)) + i; /* Port 3, 2, 1 and 0 */
            }
            SOC_IF_ERROR_RETURN
                (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0xc8fe, 0x0));
        }
        /* restore the original phy_id */
        pc->phy_id = phy_id;

        phy_id = pc->phy_id;
        for (i = 0; i < num_lanes; i++) {
            if (num_lanes > 1) {
                pc->phy_id = (phy_id & (~0x3)) + i; /* Port 3, 2, 1 and 0 */
            }

            /* Read Hand-Shake message (Done) from Micro */
            SOC_IF_ERROR_RETURN
                (READ_PHY84740_MMF_PMA_PMD_REG(unit, pc,PHY84740_PMAD_M8051_MSGOUT_REG, 
                        &data16));

            /* Download done message */
            SOC_DEBUG_PRINT((DK_PHY, "u=%d p=%d MDIO firmware download done message: 0x%x\n",
                         unit, port,data16));

            /* Clear LASI status */
            SOC_IF_ERROR_RETURN
                (READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0x9003, &data16));
        }
        /* restore the original phy_id */
        pc->phy_id = phy_id;


        /* Wait for LASI to be asserted when M8051 writes checksum to MSG_OUTr */
        sal_udelay(100); /* Wait for 100 usecs */

        phy_id = pc->phy_id;
        for (i = 0; i < num_lanes; i++) {
            if (num_lanes > 1) {
                pc->phy_id = (phy_id & (~0x3)) + i; /* Port 3, 2, 1 and 0 */
            }
            SOC_IF_ERROR_RETURN
                (READ_PHY84740_MMF_PMA_PMD_REG(unit,pc,PHY84740_PMAD_M8051_MSGOUT_REG,&data16));

            /* Need to check if checksum is correct */
            SOC_IF_ERROR_RETURN
                (READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0xCA1C, &data16));

            if (data16 != 0x600D) {
                /* Bad CHECKSUM */
                soc_cm_print("firmware_bcst downlad failure: port %d "
                         "Incorrect Checksum %x\n", port,data16);
            return SOC_E_FAIL;
            }

            /* read Rev-ID */
            SOC_IF_ERROR_RETURN
                (READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0xCA1A, &data16));
            SOC_DEBUG_PRINT((DK_PHY, "u=%d p=%d MDIO Firmware download revID: 0x%x\n",
                         unit, port,data16));
        }
        /* restore the original phy_id */
        pc->phy_id = phy_id;

        return SOC_E_NONE;
    } else {
        SOC_DEBUG_PRINT((DK_WARN, "u=%d p=%d firmware_bcst: invalid cmd 0x%x\n",
                         unit, port,cmd));
    }

    return SOC_E_FAIL;
}

/*
 * Function:
 *      phy_84740_firmware_set
 * Purpose:
 *      program the given firmware into the SPI-ROM 
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      offset - offset to the data stream
 *      array  - the given data
 *      datalen- the data length  
 * Returns:
 *      SOC_E_NONE
 */

STATIC int
phy_84740_firmware_set(int unit, int port, int offset, uint8 *array,int datalen)
{
    uint16     saved_phy_addr;
    int        ix;
    phy_ctrl_t *pc;
    uint32 devid;
    uint32 uCode_devid;

    /* overload this function a littl bit if array == NULL 
     * special handling for init. uCode broadcast. Internal use only 
     */
    if (array == NULL) {
        return _phy84740_init_ucode_bcst(unit,port,offset);
    }

    pc = EXT_PHY_SW_STATE(unit, port);

    SOC_IF_ERROR_RETURN(
        _phy_84740_config_devid(pc->unit,pc->port, &devid));

    /* first validate the firmware is for the configured device
     * 84740 has 1.5V from TXDRV pin, 84754 is internally wired at 1.0V.
     * If the 84754 firmware which programs chip for 1.0V is loaded
     * to the 84740, then it may damage the device and module.
     * 
     *	Chip ID is at addresses 0x7FF9, 0x7FFA, 0x7FFB.e.g. 
     *  for 84740, [0x7FF9] = 0x08, [0x7FFA] = 0x47, [0x7FFB] = 0x40. 
     *	Version number is at 0x7FFC-0x7FFD
     */

    /* length should be at least 0xFFFE */
    if (datalen < PHY84740_UCODE_DEVID_ADDR+3) {
        SOC_DEBUG_PRINT((DK_WARN, "firmware_set, wrong firmware len=0x%x : u=%d p=%d\n",
                         datalen, unit, port));
        return SOC_E_PARAM;
    }

    /* check the firmware devid matches the configured devid */
    ix = PHY84740_UCODE_DEVID_ADDR;
    uCode_devid = (array[ix] << 16) | (array[ix+1] << 8) |
                  array[ix+2];

    if (uCode_devid != devid) {
        SOC_DEBUG_PRINT((DK_WARN, "firmware_set,u=%d p=%d:  Firmware device "
           "type(0x%x) does not match configured device type(0x%x)\n",
                         unit, port,uCode_devid,devid));
        return SOC_E_PARAM;
    }

    if (PHY84740_SINGLE_PORT_MODE(pc)) {
        /* switch to quad-mode */
        SOC_IF_ERROR_RETURN(
            _phy_84740_single_to_quad_mode(unit,port,NULL));
    }
    saved_phy_addr = pc->phy_id;

    SOC_DEBUG_PRINT((DK_PHY, "firmware_set init,quad port : u=%d p=%d\n",
                         unit, port));
    /* configure all four lanes to bcst mode */
    for (ix = 0; ix < 4; ix++) {
        pc->phy_id = (saved_phy_addr & (~0x3)) + ix;
        SOC_IF_ERROR_RETURN
            (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc,PHY84740_PMAD_BCST_REG,0xffff));
    }
    pc->phy_id = saved_phy_addr;

    /* place all uC in reset */
    SOC_IF_ERROR_RETURN
        (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0xCA10, 0x018f));

    /* disable all port's SPI */
    SOC_IF_ERROR_RETURN
        (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0xC843, 0x000f));
  
    SOC_IF_ERROR_RETURN
        (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0xC840, 0x000c));

    /* Set bit 0 (spa enable) in C848 for master channel ? */
    SOC_IF_ERROR_RETURN
        (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0xC848, 0x2000,0x2000));

    /* disable the bcst mode for all channels */
    SOC_IF_ERROR_RETURN
        (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc,PHY84740_PMAD_BCST_REG,0xfffe));

    SOC_IF_ERROR_RETURN
        (_phy_84740_firmware_to_rom(unit,port,offset,array,datalen));

    /* switch back to single mode */
    if (PHY84740_SINGLE_PORT_MODE(pc)) {
        /* switch to quad-mode */
        SOC_IF_ERROR_RETURN(
            _phy_84740_quad_to_single_mode(unit,port));
    }
    return SOC_E_NONE;
}
STATIC int
_phy_84740_firmware_to_rom(int unit, int port, int offset, uint8 *array,int datalen)
{
    uint16     rd_data;
    uint16     wr_data;
    int        j;
    int        i = 0;
    int        count;
    uint8      spi_values[WR_BLOCK_SIZE];
    uint16     data16;
    soc_timeout_t to;
    phy_ctrl_t *pc;

    pc = EXT_PHY_SW_STATE(unit, port);

    /*
     * Enable SPI for Port0
     */    
    SOC_IF_ERROR_RETURN
        (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0xC843, 0x0000));
    SOC_IF_ERROR_RETURN
        (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0xC840, 0x0000));
    /*
     * Remove M8051 resets for port 0.
     * and wait for Micro to complete Init.
     */
    /* clear bits 3:0 */
    SOC_IF_ERROR_RETURN
        (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0xCA10, 0,0xf));
    sal_usleep(10000);

    /* set bit(request) in 0xc848 for the required channel, remove done bit */
    SOC_IF_ERROR_RETURN
        (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0xC848, 0,0x2000));

    /* enable SPI port enable */
    SOC_IF_ERROR_RETURN
            (MODIFY_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0xC848, 0xc004,0xc004));

    /*
     * Read LASI Status registers To clear initial Failure status.
     */
    SOC_IF_ERROR_RETURN
        (READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0x9003, &rd_data));
    SOC_IF_ERROR_RETURN
        (READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0x9004, &rd_data));
    SOC_IF_ERROR_RETURN
        (READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0x9005, &rd_data));

    /*
     * Enable the LASI For Message out. 
     */
    SOC_IF_ERROR_RETURN
        (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0x9000, 0x0004));
    /*
     * 0x9002, bit 2 Is Rx Alarm enabled For LASI. 
     */
    SOC_IF_ERROR_RETURN
        (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0x9002, 0x0004));
    /*
     * Read Any Residual Message out register.
     */
    SOC_IF_ERROR_RETURN
        (READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0xCA13, &rd_data));
    /*
     * Clear LASI Message Out Status. 
     */
    SOC_IF_ERROR_RETURN
        (READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0x9003, &rd_data));

    /* set SPI-ROM write enable */
    SOC_IF_ERROR_RETURN(phy_84740_rom_write_enable_set(unit, port, 1));

    SOC_DEBUG_PRINT((DK_PHY, "init0: u=%d p=%d\n",
                         unit, port));

    for (j = 0; j < datalen; j += WR_BLOCK_SIZE) {
        /*
         * Setup SPI Controller. 
         */
        count = 2;
        wr_data = ((WR_CPU_CTRL_FIFO * 0x0100) | count);
        SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));

        /* Write SPI Control -2 Register Address.*/
        wr_data = SPI_CTRL_2_L;
        SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));

        /* Write SPI Control -2 Register Word-1. */
        wr_data = 0x8200;
        SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));

        /* Write SPI Control -2 Register Word-2. */
        wr_data = 0x0100;
        SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));

        /* Fill-up SPI Transmit Fifo.
         * Write SPI Control Register Write Command.
         */
        count = 4 + (WR_BLOCK_SIZE / 2);
        wr_data = ((WR_CPU_CTRL_FIFO * 0x0100) | count);
        SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));

        /* Write Tx Fifo Register Address. */
        wr_data = SPI_TXFIFO;
        SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));

        /* Write SPI Tx Fifo Control Word-1. */
        wr_data = ((1 * 0x0100) | MSGTYPE_HWR);
        SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));

        /* Write SPI Tx Fifo Control Word-2. */
        wr_data = ((MSGTYPE_HWR * 0x0100) | WREN_OPCODE);
        SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));

        /* Write SPI Tx Fifo Control Word-3. */
        wr_data = ((WR_OPCODE * 0x0100) | (0x3 + WR_BLOCK_SIZE));
        SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));

        /* Write SPI Tx Fifo Control Word-4. */
        wr_data = (((j & 0x00FF) * 0x0100) | ((j & 0xFF00) / 0x0100));
        SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));

        SOC_DEBUG_PRINT((DK_PHY, "loop: u=%d p=%d,inxj: %d,inxi:%d\n",
                         unit, port,j,i));

        if (datalen < (j + WR_BLOCK_SIZE)) {   /* last block */ 
            sal_memset(spi_values,0,WR_BLOCK_SIZE);
            sal_memcpy(spi_values,&array[j],datalen - j);

            for (i = 0; i < WR_BLOCK_SIZE; i += 2) {
                /* Write SPI Tx Fifo Data Word-4. */
                wr_data = ((spi_values[i+1] * 0x0100) | spi_values[i]);
                SOC_IF_ERROR_RETURN(write_message(unit, pc,wr_data, &rd_data));
            }           
        } else {        
            for (i = 0; i < WR_BLOCK_SIZE; i += 2) {
                /* Write SPI Tx Fifo Data Word-4. */
                wr_data = ((array[j+i+1] * 0x0100) | array[j+i]);
                SOC_IF_ERROR_RETURN(write_message(unit, pc,wr_data, &rd_data));
            }
        }

        /* 
         * Set-up SPI Controller To Transmit.
         * Write SPI Control Register Write Command.
         */
        count = 2;
        wr_data = ((WR_CPU_CTRL_FIFO * 0x0100) | count);
        SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));

        wr_data = SPI_CTRL_1_L;
        SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));

        /* Write SPI Control -1 Register Word-1. */
        wr_data = 0x0501;
        SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));

        /* Write SPI Control -1 Register Word-2. */
        wr_data = 0x0003;
        SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));

        /* Wait For 64 bytes To be written.   */
        SOC_IF_ERROR_RETURN(phy_84740_rom_wait(unit,port));

    } /* SPI_WRITE */

    /* clear SPI-ROM write enable */
    SOC_IF_ERROR_RETURN(phy_84740_rom_write_enable_set(unit, port, 0));

    /* Disable SPI EEPROM. */
    count = 2;
    wr_data = ((WR_CPU_CTRL_FIFO * 0x0100) | count);
    SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));

    /* Write SPI Control -2 Register Address. */
    wr_data = SPI_CTRL_2_L;
    SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));

    /* Write SPI Control -2 Register Word-1. */
    wr_data = 0x8200;
    SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));

    /* Write SPI Control -2 Register Word-2. */
    wr_data = 0x0100;
    SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));


    /* Fill-up SPI Transmit Fifo With SPI EEPROM Messages. */
    count = 2;
    wr_data = ((WR_CPU_CTRL_FIFO * 0x0100) | count);
    SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));

    /* Write Tx Fifo Register Address. */
    wr_data = SPI_TXFIFO;
    SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));

    /* Write SPI Tx Fifo Control Word-1. */
    wr_data = ((0x1*0x0100) | MSGTYPE_HWR);
    SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));

    /* Write SPI Tx Fifo Control Word-2. */
    wr_data = WRDI_OPCODE;
    SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));

    /* Write SPI Control Register Write Command. */
    count = 2;
    wr_data = ((WR_CPU_CTRL_FIFO * 0x0100) | count);
    SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));

    /* Write SPI Control -1 Register Address. */
    wr_data = SPI_CTRL_1_L;
    SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));

    /* Write SPI Control -1 Register Word-1. */
    wr_data = 0x0101;
    SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));

    /* Write SPI Control -1 Register Word-2. */
    wr_data = 0x0003;
    SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));

    /* Wait For WRDI Command To be written */
    rd_data = 0;
    soc_timeout_init(&to, WR_TIMEOUT, 0);
    do {
        /* Write SPI Control Register Read Command. */
        count = 1;
        wr_data = ((RD_CPU_CTRL_REGS * 0x0100) | count);
        SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));
        wr_data = SPI_CTRL_1_L;
        SOC_IF_ERROR_RETURN(write_message(unit, pc, wr_data, &rd_data));

        /* extra reads to clear the status */
        SOC_IF_ERROR_RETURN
            (READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0xCA13, &data16));
        SOC_IF_ERROR_RETURN
            (READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0x9003, &data16));

        if (rd_data & 0x0100) {
            break;
        }
    } while (!soc_timeout_check(&to));

    if (!(rd_data & 0x0100)) {
        soc_cm_debug(DK_ERR,"phy_84740_rom_program: u = %d p = %d "
                            "WRDI command timeout\n", unit, port);
        return SOC_E_TIMEOUT;
    }

    soc_cm_debug(DK_PHY, "phy_84740_rom_program: u=%d p=%d done\n", unit, port);

    return SOC_E_NONE;
}

static int
write_message(int unit, phy_ctrl_t *pc, uint16 wrdata, uint16 *rddata)
{

    uint16     tmp_data = 0;
    soc_timeout_t to;

    SOC_IF_ERROR_RETURN
        (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0xCA12, wrdata));

    soc_timeout_init(&to, WR_TIMEOUT, 0);
    do {
        SOC_IF_ERROR_RETURN
            (READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0x9005, &tmp_data));
        if (tmp_data & 0x4)
            break;
    } while (!soc_timeout_check(&to));
    if (!(tmp_data & 0x4)) {
        soc_cm_debug(DK_ERR, "write_message failed: wrdata %04x\n", wrdata);
        return SOC_E_FAIL;
    }

    SOC_IF_ERROR_RETURN
        (READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0xCA13, &tmp_data));
    *rddata = tmp_data;
    SOC_IF_ERROR_RETURN
        (READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0x9003, &tmp_data));

    return SOC_E_NONE;
}

/*
 * Function:
 *      phy_84740_probe
 * Purpose:
 *      Complement the generic phy probe routine to identify this phy when its
 *      phy id0 and id1 is same as some other phy's.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      pc   - phy ctrl descriptor.
 * Returns:
 *      SOC_E_NONE,SOC_E_NOT_FOUND and SOC_E_<error>
 */
STATIC int
phy_84740_probe(int unit, phy_ctrl_t *pc)
{
    uint32 devid;

    SOC_IF_ERROR_RETURN(
        _phy_84740_config_devid(pc->unit,pc->port, &devid));

    if (devid == PHY84740_ID_84740) {
        pc->dev_name = dev_name_84740;
    } else if (devid == PHY84740_ID_84753) {
        pc->dev_name = dev_name_84753;
    } else if (devid == PHY84740_ID_84754) {
        pc->dev_name = dev_name_84754;
    } else if (devid == PHY84740_ID_84064) {
        pc->dev_name = dev_name_84064;
    } else {  /* not found */
        SOC_DEBUG_PRINT((DK_WARN, "port %d: BCM84xxx type PHY device detected, please use "
                         "phy_84<xxx> config variable to select the specific type\n",
                             pc->port));
        return SOC_E_NOT_FOUND;
    }
    pc->size = sizeof(PHY84740_DEV_DESC_t);
    return SOC_E_NONE;
}

STATIC int
_phy_84740_bsc_rw(int unit, soc_port_t port, int dev_addr, int opr,
                    int addr, int count, void *data_array,uint32 ram_start)
{
    phy_ctrl_t  *pc;
    int rv = SOC_E_NONE;
    soc_timeout_t  to;
    uint16 data16;
    int i;
    sal_usecs_t start;
    sal_usecs_t end;
    int access_type;
    int data_type;

    soc_cm_debug(DK_PHY,"phy_84740_bsc_read: u=%d p=%d addr=%04x\n",
                 unit, port, addr);

    if (!data_array) {
        return SOC_E_PARAM;
    }

    if (count > PHY84740_BSC_XFER_MAX) {
        return SOC_E_PARAM;
    }

    pc = EXT_PHY_SW_STATE(unit, port);
    data_type = PHY84740_I2C_DATA_TYPE(opr);
    access_type = PHY84740_I2C_ACCESS_TYPE(opr);

    if (access_type == PHY84740_I2CDEV_WRITE) {
        for (i = 0; i < count; i++) {
            if (data_type == PHY84740_I2C_8BIT) {
                SOC_IF_ERROR_RETURN
                    (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc, ram_start + i,
                          ((uint8 *)data_array)[i]));
            } else {  /* 16 bit */
                SOC_IF_ERROR_RETURN
                    (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc, ram_start + i,
                          ((uint16 *)data_array)[i]));
            }
        }
    }

    data16 = ram_start;
    SOC_IF_ERROR_RETURN
        (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0x8004, data16));
    SOC_IF_ERROR_RETURN
        (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0x8003, addr));
    SOC_IF_ERROR_RETURN
        (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0x8002, count));

    data16 = 1;
    data16 |= (dev_addr<<9);
    if (access_type == PHY84740_I2CDEV_WRITE) {
        data16 |= PHY84740_WR_FREQ_400KHZ;
    }

    SOC_IF_ERROR_RETURN
        (WRITE_PHY84740_MMF_PMA_PMD_REG(unit,pc,0x8005,data16));

    if (access_type == PHY84740_I2CDEV_WRITE) {
        data16 =  0x8000 | PHY84740_BSC_WRITE_OP;
    } else {
        data16 =  0x8000 | PHY84740_BSC_READ_OP;
    }

    if (data_type == PHY84740_I2C_16BIT) {
        data16 |= (1 << 12);
    }

    /* for single port mode, there should be only one I2C interface active
     * from lane0. The 0x800x register block is bcst type registers. If writing
     * to 0x8000 directly, it will enable all four I2C masters. Use indirect access
     * to enable only the lane 0.
     */

    if (PHY84740_SINGLE_PORT_MODE(pc)) {

        /* point to lane 0 */
        SOC_IF_ERROR_RETURN
            (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc, PHY84740_PMAD_AER_ADDR_REG, 0));

        /* indirect access: 0x8000=data16 */ 
        SOC_IF_ERROR_RETURN
            (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0xcd0c, 0x8000));
        SOC_IF_ERROR_RETURN
            (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0xcd0d, data16));

        /* issue the indirect access command */  
        SOC_IF_ERROR_RETURN
            (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0xcd13, 0x3));

        soc_timeout_init(&to, 1000000, 0);
        while (!soc_timeout_check(&to)) {
            rv = READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0xcd13, &data16);
            if (!data16) {
                break;
            }
        }
        if (data16) {
            SOC_DEBUG_PRINT((DK_WARN, "port %d: Fail register 0xcd13 status 0x%x\n",
                             port,data16));
            return SOC_E_TIMEOUT; 
        }
    } else { 
        SOC_IF_ERROR_RETURN
            (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0x8000, data16));
    }

    start = sal_time_usecs();
    soc_timeout_init(&to, 1000000, 0);
    while (!soc_timeout_check(&to)) {
        rv = READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, 0x8000, &data16);
        if (((data16 & PHY84740_2W_STAT) == PHY84740_2W_STAT_COMPLETE)) {
            break;
        }
    }
    end = sal_time_usecs();

    /* need some delays */
    sal_usleep(10000);

    SOC_DEBUG_PRINT((DK_PHY, "BSC command status %d time=%d\n",
            (data16 & PHY84740_2W_STAT), SAL_USECS_SUB(end, start)));

    if (access_type == PHY84740_I2CDEV_WRITE) {
        return SOC_E_NONE;
    }

    if ((data16 & PHY84740_2W_STAT) == PHY84740_2W_STAT_COMPLETE) {
        for (i = 0; i < count; i++) {
            SOC_IF_ERROR_RETURN
                (READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, (ram_start+i), &data16));
            if (data_type == PHY84740_I2C_16BIT) {
                ((uint16 *)data_array)[i] = data16;
                SOC_DEBUG_PRINT((DK_PHY, "%04x ", data16));
            } else {
                ((uint8 *)data_array)[i] = (uint8)data16;
                SOC_DEBUG_PRINT((DK_PHY, "%02x ", data16));
            }
        }
    }
    return SOC_E_NONE;
}

/*
 * Read a slave device such as NVRAM/EEPROM connected to the 84740's I2C
 * interface. This function will be mainly used for diagnostic or workaround
 * purpose.
 * Note:
 * The size of read_array buffer must be greater than or equal to the
 * parameter nbytes.
 * usage example:
 *   Retrieve the first 100 byte data of the non-volatile storage device with
 *   I2C bus device address 0x50(default SPF eeprom I2C address) on unit 0,
 *   port 2.
 *   uint8 data8[100];
 *   phy_84740_i2cdev_read(0,2,0x50,0,100,data8);
 */
int
phy_84740_i2cdev_read(int unit,
                     soc_port_t port,
                     int dev_addr,  /* 7 bit I2C bus device address */
                     int offset,    /* starting data address to read */
                     int nbytes,    /* number of bytes to read */
                     uint8 *read_array)   /* buffer to hold retrieved data */
{
    return _phy_84740_bsc_rw(unit, port, dev_addr,PHY84740_I2CDEV_READ,
             offset, nbytes, (void *)read_array,PHY84740_READ_START_ADDR);

}

/*
 * Write to a slave device such as NVRAM/EEPROM connected to the 84740's I2C
 * interface. This function will be mainly used for diagnostic or workaround
 * purpose.
 * Note:
 * The size of write_array buffer should be equal to the parameter nbytes.
 * The EEPROM may limit the maximun write size to 16 bytes
 * usage example:
 *   Write to first 100 byte space of the non-volatile storage device with
 *   I2C bus device address 0x50(default SPF eeprom I2C address) on unit 0,
 *   port 2, with written data specified in array data8.
 *   uint8 data8[100];
 *   *** initialize the data8 array with written data ***
 *
 *   phy_84740_i2cdev_write(0,2,0x50,0,100,data8);
 */

int
phy_84740_i2cdev_write(int unit,
                     soc_port_t port,
                     int dev_addr,  /* I2C bus device address */
                     int offset,    /* starting data address to write to */
                     int nbytes,    /* number of bytes to write */
                     uint8 *write_array)   /* buffer to hold written data */
{
    int j;
    int rv = SOC_E_NONE;
    phy_ctrl_t  *pc;

    pc = EXT_PHY_SW_STATE(unit, port);

    for (j = 0; j < (nbytes/PHY84740_BSC_WR_MAX); j++) {
        rv = _phy_84740_bsc_rw(unit, port, dev_addr,PHY84740_I2CDEV_WRITE,
                    offset + j * PHY84740_BSC_WR_MAX, PHY84740_BSC_WR_MAX,
                    (void *)(write_array + j * PHY84740_BSC_WR_MAX),
                    PHY84740_WRITE_START_ADDR);
        if (rv != SOC_E_NONE) {
            return rv;
        }
        sal_usleep(20000);
    }
    if (nbytes%PHY84740_BSC_WR_MAX) {
        rv = _phy_84740_bsc_rw(unit, port, dev_addr,PHY84740_I2CDEV_WRITE,
                offset + j * PHY84740_BSC_WR_MAX, nbytes%PHY84740_BSC_WR_MAX,
                (void *)(write_array + j * PHY84740_BSC_WR_MAX),
                PHY84740_WRITE_START_ADDR);
    }
    return rv;
}

/*
 * Function:
 *      phy_84740_reg_read
 * Purpose:
 *      Routine to read PHY register
 * Parameters:
 *      uint         - BCM unit number
 *      port         - port number
 *      flags        - Flags which specify the register type
 *      phy_reg_addr - Encoded register address
 *      phy_data     - (OUT) Value read from PHY register
 * Note:
 *      This register read function is not thread safe. Higher level
 * function that calls this function must obtain a per port lock
 * to avoid overriding register page mapping between threads.
 */
STATIC int
phy_84740_reg_read(int unit, soc_port_t port, uint32 flags,
                  uint32 phy_reg_addr, uint32 *phy_data)
{
    uint16               data16;
    uint16               regdata;
    phy_ctrl_t          *pc;      /* PHY software state */
    int rv = SOC_E_NONE;
    int rd_cnt;

    pc = EXT_PHY_SW_STATE(unit, port);

    rd_cnt = 1;

    if (flags & SOC_PHY_I2C_DATA8) {

        SOC_IF_ERROR_RETURN
            (phy_84740_i2cdev_read(unit, port,
                        SOC_PHY_I2C_DEVAD(phy_reg_addr),
                        SOC_PHY_I2C_REGAD(phy_reg_addr),
                        rd_cnt,
                        (uint8 *)&data16));
        *phy_data = *((uint8 *)&data16);

    } else if (flags & SOC_PHY_I2C_DATA16) {
        /* This operation is generally targeted to access 16-bit device,
         * such as PHY IC inside the SFP.  However there is no 16-bit
         * scratch register space on the device.  Use 1.800e
         * for this operation.
         */
        SOC_IF_ERROR_RETURN
            (READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, PHY84740_I2C_TEMP_RAM,
                   &regdata));

        rv = _phy_84740_bsc_rw(unit, port,
               SOC_PHY_I2C_DEVAD(phy_reg_addr),
               PHY84740_I2C_OP_TYPE(PHY84740_I2CDEV_READ,PHY84740_I2C_16BIT),
               SOC_PHY_I2C_REGAD(phy_reg_addr),1,
               (void *)&data16,PHY84740_I2C_TEMP_RAM);

        *phy_data = data16;

        /* restore the ram register value */
        SOC_IF_ERROR_RETURN
            (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc, PHY84740_I2C_TEMP_RAM,
                 regdata));
    } else {
        SOC_IF_ERROR_RETURN
            (READ_PHY_REG(unit, pc, phy_reg_addr, &data16));
        *phy_data = data16;
    }

    return rv;
}

/*
 * Function:
 *      phy_84740_reg_write
 * Purpose:
 *      Routine to write PHY register
 * Parameters:
 *      uint         - BCM unit number
 *      port         - port number
 *      flags        - Flags which specify the register type
 *      phy_reg_addr - Encoded register address
 *      phy_data     - Value write to PHY register
 * Note:
 *      This register read function is not thread safe. Higher level
 * function that calls this function must obtain a per port lock
 * to avoid overriding register page mapping between threads.
 */
STATIC int
phy_84740_reg_write(int unit, soc_port_t port, uint32 flags,
                   uint32 phy_reg_addr, uint32 phy_data)
{
    uint8  data8[4];
    uint16 data16[2];
    uint16 regdata[2];
    phy_ctrl_t          *pc;      /* PHY software state */
    int rv;
    int wr_cnt;

    pc = EXT_PHY_SW_STATE(unit, port);

    wr_cnt = 1;

    if (flags & SOC_PHY_I2C_DATA8) {
        data8[0] = (uint8)phy_data;
        SOC_IF_ERROR_RETURN
            (phy_84740_i2cdev_write(unit, port,
                        SOC_PHY_I2C_DEVAD(phy_reg_addr),
                        SOC_PHY_I2C_REGAD(phy_reg_addr),
                        wr_cnt,
                        data8));
    } else if (flags & SOC_PHY_I2C_DATA16) {
        /* This operation is generally targeted to access 16-bit device,
         * such as PHY IC inside the SFP.  However there is no 16-bit
         * scratch register space on the device.  Use 1.800e
         * for this operation.
         */
        /* save the temp ram register */
        SOC_IF_ERROR_RETURN
            (READ_PHY84740_MMF_PMA_PMD_REG(unit, pc, PHY84740_I2C_TEMP_RAM,
                 &regdata[0]));
        data16[0] = phy_data;
        rv = _phy_84740_bsc_rw(unit, port,
              SOC_PHY_I2C_DEVAD(phy_reg_addr),
              PHY84740_I2C_OP_TYPE(PHY84740_I2CDEV_WRITE,PHY84740_I2C_16BIT),
              SOC_PHY_I2C_REGAD(phy_reg_addr),wr_cnt,
              (void *)data16,PHY84740_I2C_TEMP_RAM);

        /* restore the ram register value */
        SOC_IF_ERROR_RETURN
            (WRITE_PHY84740_MMF_PMA_PMD_REG(unit, pc, PHY84740_I2C_TEMP_RAM,
                  regdata[0]));
    } else {
        SOC_IF_ERROR_RETURN
            (WRITE_PHY_REG(unit, pc, phy_reg_addr, (uint16)phy_data));
    }

    return SOC_E_NONE;
}

/*
 * Variable:
 *    phy_84740_drv
 * Purpose:
 *    Phy Driver for 40G PHY.
 */

phy_driver_t phy_84740drv_xe = {
    "84740 10-Gigabit PHY Driver",
    phy_84740_init,        /* Init */
    phy_null_reset,       /* Reset (dummy) */
    phy_84740_link_get,    /* Link get   */
    phy_84740_enable_set,  /* Enable set */
    phy_null_enable_get,  /* Enable get */
    phy_null_set,         /* Duplex set */
    phy_null_one_get,     /* Duplex get */
    phy_84740_speed_set,   /* Speed set  */
    phy_84740_speed_get,   /* Speed get  */
    phy_null_set,         /* Master set */
    phy_null_zero_get,    /* Master get */
    phy_84740_an_set,      /* ANA set */
    phy_84740_an_get,      /* ANA get */
    NULL,                 /* Local Advert set */
    NULL,                 /* Local Advert get */
    phy_null_mode_get,    /* Remote Advert get */
    phy_84740_lb_set,      /* PHY loopback set */
    phy_84740_lb_get,      /* PHY loopback set */
    phy_84740_interface_set, /* IO Interface set */
    phy_84740_interface_get, /* IO Interface get */
    NULL,   /* PHY abilities mask */
    NULL,
    NULL,
    phy_null_mdix_set,
    phy_null_mdix_get,
    phy_null_mdix_status_get,
    NULL,
    NULL,
    phy_null_medium_get,
    NULL,                    /* phy_cable_diag  */
    NULL,                    /* phy_link_change */
    phy_84740_control_set,    /* phy_control_set */
    phy_84740_control_get,    /* phy_control_get */
    phy_84740_reg_read,       /* phy_reg_read */
    phy_84740_reg_write,      /* phy_reg_write */
    NULL,                    /* phy_reg_modify */
    NULL,                    /* phy_notify */
    phy_84740_probe,         /* pd_probe  */
    phy_84740_ability_advert_set,  /* pd_ability_advert_set */
    phy_84740_ability_advert_get,  /* pd_ability_advert_get */
    phy_84740_ability_remote_get,  /* pd_ability_remote_get */
    phy_84740_ability_local_get,   /* pd_ability_local_get  */
    phy_84740_firmware_set,        /* pd_firmware_set */
    NULL,                          /* pd_timesync_config_set */
    NULL,                          /* pd_timesync_config_get */
    NULL,                          /* pd_timesync_control_set */
    NULL,                          /* pd_timesync_control_get */
    phy_84740_diag_ctrl             /* .pd_diag_ctrl */
};

#else /* INCLUDE_PHY_84740 */
int _phy_84740_not_empty;
#endif /* INCLUDE_PHY_84740 */
