/*
 * $Id: miim.c 1.9 Broadcom SDK $
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
 * MII Management (MDIO) bus access routines for reading and writing PHY
 * registers.
 */

#include <soc/debug.h>
#include <soc/drv.h>
#include <soc/error.h>
#include <soc/phyreg.h>
#include <soc/linkctrl.h>

#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_ca_auto.h>
#include <soc/sbx/hal_c2_auto.h>
#include <soc/sbx/hal_ca_c2.h>
#include <soc/sbx/bm9600_soc_init.h>

/*
 * PHY ID Encoding
 *
 * Defined in common phy driver as follows:
 *
 * bits [4:0] - MDIO address
 * bit  [6]   - Indicates MDIO bus
 *                  1: M1  -> used to access XGXS in XE ports
 *                  0: M0  -> used to access SERDES in GE ports
 * bit  [7]   - Indicates internal or external PHY
 *                  1: Internal
 *                  0: External
 */
#define PHY_ID_MDIO1_BUS          0x40
#define PHY_ID_INTERNAL           0x80
#define PHY_ID_MDIO_ADDRESS_MASK  0x1f


/*
 * MIIM CLAUSE
 *
 * The MIIM_ADDRESS register is defined as follows:
 *
 * Clause 22 - bits  [4:0] supply the register address REGAD
 *             bits [15:0] are ignored
 *
 * Clause 45 - bits [20:16] supply the device address DEVAD
 *             bits [15:0]  supply the register address REGAD
 */
#define CLAUSE_22        1
#define CLAUSE_45        2

#define MIIM_ADDRESS_CLAUSE_22(regad)    ((regad) & 0x1f)
#define MIIM_ADDRESS_CLAUSE_45(devad, regad)    \
    ((((devad) & 0x1f) << 16) | ((regad) & 0xffff))

/*
 * Defines for MIIM operation complete polling cycle
 * TODO:  Check mdio speed to determine if polling times are enough
 */
#define MIIM_POLL_TIMES_MAX      500  /* Max poll times */
#define MIIM_POLL_INTERVAL       250  /* Poll time interval in nanosecs */


/*
 * Function:
 *     _soc_sbx_phy_id_decode
 * Purpose:
 *     Decodes value in 'phy_id' as described in "PHY ID Encoding"
 *     comment section.
 * Parameters:
 *     phy_id    - Phy ID to decode
 *     internal  - (OUT) Indicates if PHY is internal (1) or external (0)
 *     mdio_bus  - (OUT) MDIO bus 0 or 1
 *     mdio_addr - (OUT) MDIO address
 * Returns:
 *     None
 */
STATIC void
_soc_sbx_phy_id_decode(uint32 phy_id, uint8 *internal, uint8 *mdio_bus,
                       uint16 *mdio_addr)
{
    if (phy_id & PHY_ID_INTERNAL) {
        *internal = 0x1;  /* Internal */
    } else {
        *internal = 0x0;  /* External */
    }

    if (phy_id & PHY_ID_MDIO1_BUS) {
        *mdio_bus = 0x1;  /* MDIO bus 1 */
    } else {
        *mdio_bus = 0x0;  /* MDIO bus 0 */
    }

    *mdio_addr = phy_id & PHY_ID_MDIO_ADDRESS_MASK;

    return;
}

#if defined(BCM_FE2000_SUPPORT)
/*
 * Function:
 *     _soc_fe2000_miim_write
 * Purpose:
 *     Write a value to a MIIM register using clause 22 or clause 45
 *     transaction.
 * Parameters:
 *     unit         - Device number
 *     clause       - MIIM transaction clause, 22 or 45
 *     phy_id       - Phy ID encoding mdio address, mdio bus, internal phy
 *     phy_reg_addr - PHY register to write
 *     phy_wr_data  - Data to write
 * Returns:
 *     SOC_E_XXX
 */
STATIC int
_soc_fe2000_miim_write(int unit, int clause,
                       uint32 phy_id, uint32 phy_reg_addr, uint16 phy_wr_data)
{
    sbhandle  sb_handle;
    uint32    data;
    uint32    miim_addr;
    uint8     c45_sel;       /* 1 is clause 45, 0 is clause 22 */
    uint8     internal_phy;  /* 1 is internal, 0 is external */
    uint8     mdio_bus;      /* 1 is MDIO1, 0 is MDIO0 */
    uint16    mdio_addr;
    uint32    ack;
    uint32    timeout;

    sb_handle = SOC_SBX_SBHANDLE(unit);

    /* Set phy register address */
    data = 0;

    if (clause == CLAUSE_22) {
        miim_addr = MIIM_ADDRESS_CLAUSE_22(phy_reg_addr);
        c45_sel   = 0x0;
    } else if (clause == CLAUSE_45) {
        uint8   dev_addr;
        uint16  reg_addr;
        dev_addr  = SOC_PHY_CLAUSE45_DEVAD(phy_reg_addr);
        reg_addr  = SOC_PHY_CLAUSE45_REGAD(phy_reg_addr);
        miim_addr = MIIM_ADDRESS_CLAUSE_45(dev_addr, reg_addr);
        c45_sel   = 0x1;
    } else {
        return SOC_E_PARAM;
    }

    data = SAND_HAL_FE2000_MOD_FIELD(sb_handle, PC_MIIM_ADDRESS, MIIM_ADDRESS,
                              data, miim_addr);
    SAND_HAL_FE2000_WRITE(sb_handle, PC_MIIM_ADDRESS, data);

    /*
     * Set control registers
     * - Internal or external phy access
     * - MDIO bus
     * - MDIO address
     */
    _soc_sbx_phy_id_decode(phy_id, &internal_phy, &mdio_bus, &mdio_addr);
    data = SAND_HAL_FE2000_READ(sb_handle, PC_MIIM_CONTROL1);
    data = SAND_HAL_FE2000_MOD_FIELD(sb_handle, PC_MIIM_CONTROL1, MIIM_INT_PHY_SEL,
                              data, internal_phy);
    data = SAND_HAL_FE2000_MOD_FIELD(sb_handle, PC_MIIM_CONTROL1, MIIM_HG_PHY_SEL,
                              data, mdio_bus);
    data = SAND_HAL_FE2000_MOD_FIELD(sb_handle, PC_MIIM_CONTROL1, MIIM_C45_PHY_SEL,
                              data, c45_sel);
    data = SAND_HAL_FE2000_MOD_FIELD(sb_handle, PC_MIIM_CONTROL1, MIIM_PORT_NUM,
                              data, mdio_addr);
    data = SAND_HAL_FE2000_MOD_FIELD(sb_handle, PC_MIIM_CONTROL1, MIIM_PHY_WR_DATA,
                              data, phy_wr_data);
    SAND_HAL_FE2000_WRITE(sb_handle, PC_MIIM_CONTROL1, data);

    /* Start MIIM operation */
    data = SAND_HAL_FE2000_READ(sb_handle, PC_MIIM_CONTROL0);
    data = SAND_HAL_FE2000_MOD_FIELD(sb_handle, PC_MIIM_CONTROL0, OVER_RIDE_EXT_MDIO,
                              data, 0x1);
    data = SAND_HAL_FE2000_MOD_FIELD(sb_handle, PC_MIIM_CONTROL0, MIIM_WR_CMD_EN,
                              data, 0x1);
    SAND_HAL_FE2000_WRITE(sb_handle, PC_MIIM_CONTROL0, data);

    /* Wait for MIIM operation to complete */
    ack = 0;
    timeout = 0;
    while (!ack && (MIIM_POLL_TIMES_MAX > timeout)) {
        ack = SAND_HAL_FE2000_GET_FIELD(sb_handle, PC_INTERRUPT, MIIM_OP_COMPLETE,
                                 SAND_HAL_FE2000_READ(sb_handle, PC_INTERRUPT));
        thin_delay(MIIM_POLL_INTERVAL);
        timeout++;
    }
    if (timeout == MIIM_POLL_TIMES_MAX) {
        return SOC_E_INTERNAL;
    }

    /* Clear interrupt status (W1TC) */
    SAND_HAL_FE2000_WRITE(sb_handle, PC_INTERRUPT,
                   (1<<SAND_HAL_FE2000_FIELD_SHIFT(sb_handle, PC_INTERRUPT, MIIM_OP_COMPLETE)));

    return SOC_E_NONE;
}

/*
 * Function:
 *     _soc_fe2000_miim_read
 * Purpose:
 *     Read a value from a MIIM register.
 * Parameters:
 *     unit         - Device number
 *     clause       - MIIM transaction clause, 22 or 45
 *     phy_id       - Phy ID encoding mdio address, mdio bus, internal phy
 *     phy_reg_addr - PHY register to read
 *     phy_rd_data  - (OUT) Returns data read
 * Returns:
 *     SOC_E_XXX
 */
STATIC int
_soc_fe2000_miim_read(int unit, int clause,
                      uint32 phy_id, uint32 phy_reg_addr, uint16 *phy_rd_data)
{
    sbhandle  sb_handle;
    uint32    data;
    uint32    miim_addr;
    uint8     c45_sel;       /* 1 is clause 45, 0 is clause 22 */
    uint8     internal_phy;  /* 1 is internal, 0 is external */
    uint8     mdio_bus;      /* 1 is MDIO1, 0 is MDIO0 */
    uint16    mdio_addr;
    uint32    ack;
    uint32    timeout;

    sb_handle = SOC_SBX_SBHANDLE(unit);

    /* Set phy register address */
    data = 0;

    if (clause == CLAUSE_22) {
        miim_addr = MIIM_ADDRESS_CLAUSE_22(phy_reg_addr);
        c45_sel   = 0x0;
    } else if (clause == CLAUSE_45) {
        uint8   dev_addr;
        uint16  reg_addr;
        dev_addr  = SOC_PHY_CLAUSE45_DEVAD(phy_reg_addr);
        reg_addr  = SOC_PHY_CLAUSE45_REGAD(phy_reg_addr);
        miim_addr = MIIM_ADDRESS_CLAUSE_45(dev_addr, reg_addr);
        c45_sel   = 0x1;
    } else {
        return SOC_E_PARAM;
    }

    data = SAND_HAL_FE2000_MOD_FIELD(sb_handle, PC_MIIM_ADDRESS, MIIM_ADDRESS,
                              data, miim_addr);
    SAND_HAL_FE2000_WRITE(sb_handle, PC_MIIM_ADDRESS, data);

    /*
     * Set control registers
     * - Internal or external phy access
     * - MDIO bus
     * - MDIO address
     */
    _soc_sbx_phy_id_decode(phy_id, &internal_phy, &mdio_bus, &mdio_addr);
    data = SAND_HAL_FE2000_READ(sb_handle, PC_MIIM_CONTROL1);
    data = SAND_HAL_FE2000_MOD_FIELD(sb_handle, PC_MIIM_CONTROL1, MIIM_INT_PHY_SEL,
                              data, internal_phy);
    data = SAND_HAL_FE2000_MOD_FIELD(sb_handle, PC_MIIM_CONTROL1, MIIM_HG_PHY_SEL,
                              data, mdio_bus);
    data = SAND_HAL_FE2000_MOD_FIELD(sb_handle, PC_MIIM_CONTROL1, MIIM_C45_PHY_SEL,
                              data, c45_sel);
    data = SAND_HAL_FE2000_MOD_FIELD(sb_handle, PC_MIIM_CONTROL1, MIIM_PORT_NUM,
                              data, mdio_addr);
    SAND_HAL_FE2000_WRITE(sb_handle, PC_MIIM_CONTROL1, data);

    /* Start MIIM operation */
    data = SAND_HAL_FE2000_READ(sb_handle, PC_MIIM_CONTROL0);
    data = SAND_HAL_FE2000_MOD_FIELD(sb_handle, PC_MIIM_CONTROL0, OVER_RIDE_EXT_MDIO,
                              data, 0x1);
    data = SAND_HAL_FE2000_MOD_FIELD(sb_handle, PC_MIIM_CONTROL0, MIIM_RD_CMD_EN,
                              data, 0x1);
    SAND_HAL_FE2000_WRITE(sb_handle, PC_MIIM_CONTROL0, data);

    /* Wait for MIIM operation to complete */
    ack = 0;
    timeout = 0;
    while (!ack && (MIIM_POLL_TIMES_MAX > timeout)) {
        ack = SAND_HAL_FE2000_GET_FIELD(sb_handle, PC_INTERRUPT, MIIM_OP_COMPLETE,
                                 SAND_HAL_FE2000_READ(sb_handle, PC_INTERRUPT));
        thin_delay(MIIM_POLL_INTERVAL);
        timeout++;
    }
    if (timeout == MIIM_POLL_TIMES_MAX) {
        return SOC_E_INTERNAL;
    }

    /* Clear interrupt status (W1TC) */
    SAND_HAL_FE2000_WRITE(sb_handle, PC_INTERRUPT,
                   (1<<SAND_HAL_FE2000_FIELD_SHIFT(sb_handle, PC_INTERRUPT, MIIM_OP_COMPLETE)));

    *phy_rd_data = (uint16) SAND_HAL_FE2000_READ(sb_handle, PC_MIIM_READ_DATA);

    return SOC_E_NONE;
}
#endif /* defined(BCM_FE2000_SUPPORT) */

#ifdef BCM_SIRIUS_SUPPORT
/*
 * Function:
 *     _soc_sirius_miim_write
 * Purpose:
 *     Write a value to a MIIM register using clause 22 or clause 45
 *     transaction.
 * Parameters:
 *     unit         - Device number
 *     clause       - MIIM transaction clause, 22 or 45
 *     phy_id       - Phy ID encoding mdio address, mdio bus, internal phy
 *     phy_reg_addr - PHY register to write
 *     phy_wr_data  - Data to write
 * Returns:
 *     SOC_E_XXX
 */
STATIC int
_soc_sirius_miim_write(int unit, int clause,
                       uint32 phy_id, uint32 phy_reg_addr, uint16 phy_wr_data)
{
    int                 rv = SOC_E_NONE;
    int                 clause45;
    uint32              phy_param;
    uint32              phy_miim_addr;
    uint32              phy_devad;
    int                 bus_sel;
    
    assert(!sal_int_context());

    clause45 = soc_feature(unit, soc_feature_phy_cl45);

    if (clause == CLAUSE_22) {

	if (SOC_WARM_BOOT(unit) || SOC_IS_RELOADING(unit)) {
	    return SOC_E_NONE;
	}

	
	bus_sel = (phy_id & 0x60) >> 5;
	    
	phy_param = (uint32)phy_wr_data |
	    (((uint32)phy_id & 0x1f) << MIIM_PARAM_ID_OFFSET);

        if (phy_id & 0x80) { /* Internal/External select */
            phy_param |= (1 << (MIIM_PARAM_ID_OFFSET + 7));
        }
        if (bus_sel == 1) {
            phy_param |= (1 << (MIIM_PARAM_ID_OFFSET + 6));
        } else if (bus_sel == 2) {
            phy_param |= (1 << (MIIM_PARAM_ID_OFFSET + 8));
        }

	/* Write parameter register and tell CMIC to start */

	/* Clause 45 support changes Clause 22 access method */
	if (clause45) {
	    WRITE_CMIC_MIIM_ADDRESSr(unit, phy_reg_addr);
	} else {
	    phy_param |= (uint32)phy_reg_addr << MIIM_PARAM_REG_ADDR_OFFSET;
	}
	
	soc_pci_write(unit, CMIC_MIIM_PARAM, phy_param);
    
	soc_pci_write(unit, CMIC_SCHAN_CTRL, SC_MIIM_WR_START_SET);

	/* Wait for completion using either the interrupt or polling method */
	
	if (SOC_CONTROL(unit)->miimIntrEnb) {
	    soc_intr_enable(unit, IRQ_MIIM_OP_DONE);
	    
	    if (sal_sem_take(SOC_CONTROL(unit)->miimIntr,
			     SOC_CONTROL(unit)->miimTimeout) != 0) {
		rv = SOC_E_TIMEOUT;
	    } else {
		soc_cm_debug(DK_MIIM | DK_VERBOSE,
			     "  Interrupt received\n");
	    }
	    
	    soc_intr_disable(unit, IRQ_MIIM_OP_DONE);
	} else {
	    soc_timeout_t to;
	    
	    soc_timeout_init(&to, SOC_CONTROL(unit)->miimTimeout, 250);
	    
	    while ((soc_pci_read(unit, CMIC_SCHAN_CTRL) &
		    SC_MIIM_OP_DONE_TST) == 0) {
		if (soc_timeout_check(&to)) {
		    rv = SOC_E_TIMEOUT;
		    break;
		}
	    }
	    
	    if (rv == SOC_E_NONE) {
		soc_cm_debug(DK_MIIM | DK_VERBOSE,
			     "  Done in %d polls\n", to.polls);
	    }
	    
	    soc_pci_write(unit, CMIC_SCHAN_CTRL, SC_MIIM_OP_DONE_CLR);
	}
	
	if (rv == SOC_E_TIMEOUT) {
	    soc_cm_debug(DK_ERR,
			 "soc_miim_write: "
			 "timeout (id=0x%02x addr=0x%02x data=0x%04x)\n",
			 phy_id, phy_reg_addr, phy_wr_data);
	    SOC_CONTROL(unit)->stat.err_mii_tmo++;
	}
	
    } else if (clause == CLAUSE_45) {

	if (SOC_WARM_BOOT(unit)) {
	    return SOC_E_NONE;
	}

	phy_devad = SOC_PHY_CLAUSE45_DEVAD(phy_reg_addr);
	phy_reg_addr = SOC_PHY_CLAUSE45_DEVAD(phy_reg_addr);

	soc_cm_debug(DK_MIIM,
		     "soc_sbx_miimc45_write: id=0x%02x phy_devad=0x%02x "
		     "addr=0x%02x data=0x%04x\n",
		     phy_id, phy_devad, phy_reg_addr, phy_wr_data);

	if (!clause45) {
	    return SOC_E_UNAVAIL;
	}

	if (SOC_IS_RELOADING(unit)) {
	    return SOC_E_NONE;
	}

	/* Write parameter registers and tell CMIC to start */
        phy_param = 0;
        soc_reg_field_set(unit, CMIC_MIIM_PARAMr, &phy_param,
                          MIIM_CYCLEf, MIIM_CYCLE_AUTO);
        if (phy_id & 0x80) {
            phy_id &= (~0x80);
            soc_reg_field_set(unit, CMIC_MIIM_PARAMr,
                              &phy_param, INTERNAL_SELf, 1);
        }

        if (soc_feature(unit, soc_feature_mdio_enhanced)) {
            soc_reg_field_set(unit, CMIC_MIIM_PARAMr,
                              &phy_param, C45_SELf, 1);
	    soc_reg_field_set(unit, CMIC_MIIM_PARAMr, &phy_param, 
			      BUS0_1_SELf, ((phy_id & 0x20) ? 1 : 0));
	    phy_id &= (~0x20);
	    
	    soc_reg_field_set(unit, CMIC_MIIM_PARAMr, &phy_param, 
			      BUS2_SELf, ((phy_id & 0x40) ? 1 : 0));
	    phy_id &= (~0x40);
        }

        soc_reg_field_set(unit, CMIC_MIIM_PARAMr, &phy_param, PHY_IDf, phy_id);
        soc_reg_field_set(unit, CMIC_MIIM_PARAMr, &phy_param,
                          PHY_DATAf, phy_wr_data);
        phy_miim_addr = 0;
        soc_reg_field_set(unit, CMIC_MIIM_ADDRESSr, &phy_miim_addr,
                          CLAUSE_45_DTYPEf, phy_devad);
        soc_reg_field_set(unit, CMIC_MIIM_ADDRESSr, &phy_miim_addr,
                          CLAUSE_45_REGADRf, phy_reg_addr);

        WRITE_CMIC_MIIM_ADDRESSr(unit, phy_miim_addr);

        soc_pci_write(unit, CMIC_MIIM_PARAM, phy_param);

        soc_pci_write(unit, CMIC_SCHAN_CTRL, SC_MIIM_WR_START_SET);

        /* Wait for completion using either the interrupt or polling method */

        if (SOC_CONTROL(unit)->miimIntrEnb) {
            soc_intr_enable(unit, IRQ_MIIM_OP_DONE);
	    
            if (sal_sem_take(SOC_CONTROL(unit)->miimIntr,
                             SOC_CONTROL(unit)->miimTimeout) != 0) {
                rv = SOC_E_TIMEOUT;
            } else {
                soc_cm_debug(DK_MIIM | DK_VERBOSE,
                             "  Interrupt received\n");
            }

            soc_intr_disable(unit, IRQ_MIIM_OP_DONE);
        } else {
            soc_timeout_t to;

            soc_timeout_init(&to, SOC_CONTROL(unit)->miimTimeout, 250);

            while ((soc_pci_read(unit, CMIC_SCHAN_CTRL) &
                    SC_MIIM_OP_DONE_TST) == 0) {
                if (soc_timeout_check(&to)) {
                    rv = SOC_E_TIMEOUT;
                    break;
                }
            }

            if (rv == SOC_E_NONE) {
                soc_cm_debug(DK_MIIM | DK_VERBOSE,
                             "  Done in %d polls\n", to.polls);
            }

            soc_pci_write(unit, CMIC_SCHAN_CTRL, SC_MIIM_OP_DONE_CLR);
        }

        if (rv == SOC_E_TIMEOUT) {
            soc_cm_debug(DK_ERR,
                         "soc_miimc45_write: "
                         "timeout (id=0x%02x addr=0x%02x data=0x%04x)\n",
                         phy_id, phy_reg_addr, phy_wr_data);
            SOC_CONTROL(unit)->stat.err_mii_tmo++;
        }

    } else {
	soc_cm_debug(DK_ERR,
		     "soc_miim_write: "
		     "only clause 22 and 45 are supported\n");
	rv = SOC_E_UNAVAIL;
    }

    return rv;
}

/*
 * Function:
 *     _soc_sirius_miim_read
 * Purpose:
 *     Read a value from a MIIM register.
 * Parameters:
 *     unit         - Device number
 *     clause       - MIIM transaction clause, 22 or 45
 *     phy_id       - Phy ID encoding mdio address, mdio bus, internal phy
 *     phy_reg_addr - PHY register to read
 *     phy_rd_data  - (OUT) Returns data read
 * Returns:
 *     SOC_E_XXX
 */
STATIC int
_soc_sirius_miim_read(int unit, int clause,
                      uint32 phy_id, uint32 phy_reg_addr, uint16 *phy_rd_data)
{
    int                 rv = SOC_E_NONE;
    int                 clause45;
    uint32              phy_param;
    uint32              phy_miim_addr;
    uint32              phy_devad;
    int                 bus_sel;

    clause45 = soc_feature(unit, soc_feature_phy_cl45);

    if (clause == CLAUSE_22) {

	
	bus_sel = (phy_id & 0x60) >> 5;
	
	phy_param = (((uint32)phy_id & 0x1f) << MIIM_PARAM_ID_OFFSET);
	
	if (phy_id & 0x80) { /* Internal/External select */
	    phy_param |= (1 << (MIIM_PARAM_ID_OFFSET + 7));
	}
	if (bus_sel == 1) {
	    phy_param |= (1 << (MIIM_PARAM_ID_OFFSET + 6));
	} else if (bus_sel == 2) {
	    phy_param |= (1 << (MIIM_PARAM_ID_OFFSET + 8));
	}
	
	
	/* Write parameter register and tell CMIC to start */
	
	/* Clause 45 support changes Clause 22 access method */
	if (clause45) {
	    WRITE_CMIC_MIIM_ADDRESSr(unit, phy_reg_addr);
	} else {
	    phy_param |= (uint32)phy_reg_addr << MIIM_PARAM_REG_ADDR_OFFSET;
	}
	
	soc_pci_write(unit, CMIC_MIIM_PARAM, phy_param);
	
	soc_pci_write(unit, CMIC_SCHAN_CTRL, SC_MIIM_RD_START_SET);
	
	/* Wait for completion using either the interrupt or polling method */
	
	if (SOC_CONTROL(unit)->miimIntrEnb) {
	    soc_intr_enable(unit, IRQ_MIIM_OP_DONE);
	    
	    if (sal_sem_take(SOC_CONTROL(unit)->miimIntr,
			     SOC_CONTROL(unit)->miimTimeout) != 0) {
		rv = SOC_E_TIMEOUT;
	    } else {
		soc_cm_debug(DK_MIIM | DK_VERBOSE,
			     "  Interrupt received\n");
	    }
	    
	    soc_intr_disable(unit, IRQ_MIIM_OP_DONE);
	} else {
	    soc_timeout_t to;
	    
	    soc_timeout_init(&to, SOC_CONTROL(unit)->miimTimeout, 250);
	    
	    while ((soc_pci_read(unit, CMIC_SCHAN_CTRL) &
		    SC_MIIM_OP_DONE_TST) == 0) {
		if (soc_timeout_check(&to)) {
		    rv = SOC_E_TIMEOUT;
		    break;
		}
	    }
	    
	    if (rv == SOC_E_NONE) {
		soc_cm_debug(DK_MIIM | DK_VERBOSE,
			     "  Done in %d polls\n", to.polls);
	    }
	    
	    soc_pci_write(unit, CMIC_SCHAN_CTRL, SC_MIIM_OP_DONE_CLR);
	}
	
	if (rv == SOC_E_TIMEOUT) {
	    soc_cm_debug(DK_ERR,
			 "soc_miim_read: "
			 "timeout (id=0x%02x addr=0x%02x)\n",
			 phy_id, phy_reg_addr);
	    SOC_CONTROL(unit)->stat.err_mii_tmo++;
	} else {
	    *phy_rd_data = (uint16)soc_pci_read(unit, CMIC_MIIM_READ_DATA);
	    
	    soc_cm_debug(DK_MIIM,
			 "soc_sbx_miim_read: read data=0x%04x\n", *phy_rd_data);
	}
    } else if (clause == CLAUSE_45) {
	if (!clause45) {
	    return SOC_E_UNAVAIL;
	}
	    
	phy_devad = SOC_PHY_CLAUSE45_DEVAD(phy_reg_addr);
	phy_reg_addr = SOC_PHY_CLAUSE45_DEVAD(phy_reg_addr);

	/* Write parameter registers and tell CMIC to start */
	phy_param = 0;
	soc_reg_field_set(unit, CMIC_MIIM_PARAMr, &phy_param,
			  MIIM_CYCLEf, MIIM_CYCLE_AUTO);
	if (phy_id & 0x80) {
	    phy_id &= (~0x80);
	    soc_reg_field_set(unit, CMIC_MIIM_PARAMr,
				  &phy_param, INTERNAL_SELf, 1);
	}
	if (soc_feature(unit, soc_feature_mdio_enhanced)) {
	    soc_reg_field_set(unit, CMIC_MIIM_PARAMr,
			      &phy_param, C45_SELf, 1);
	    soc_reg_field_set(unit, CMIC_MIIM_PARAMr, &phy_param,
			      BUS0_1_SELf, ((phy_id & 0x20) ? 1 : 0));
	    phy_id &= (~0x20);
	    
	    soc_reg_field_set(unit, CMIC_MIIM_PARAMr, &phy_param,
			      BUS2_SELf, ((phy_id & 0x40) ? 1 : 0));
	    phy_id &= (~0x40);

	}

	soc_reg_field_set(unit, CMIC_MIIM_PARAMr, &phy_param, PHY_IDf, phy_id);
	phy_miim_addr = 0;
	soc_reg_field_set(unit, CMIC_MIIM_ADDRESSr, &phy_miim_addr,
			  CLAUSE_45_DTYPEf, phy_devad);
	soc_reg_field_set(unit, CMIC_MIIM_ADDRESSr, &phy_miim_addr,
			  CLAUSE_45_REGADRf, phy_reg_addr);
	
	WRITE_CMIC_MIIM_ADDRESSr(unit, phy_miim_addr);
	    
	soc_pci_write(unit, CMIC_MIIM_PARAM, phy_param);
	    
	soc_pci_write(unit, CMIC_SCHAN_CTRL, SC_MIIM_RD_START_SET);
	    
	/* Wait for completion using either the interrupt or polling method */
	if (SOC_CONTROL(unit)->miimIntrEnb) {
	    soc_intr_enable(unit, IRQ_MIIM_OP_DONE);
		
	    if (sal_sem_take(SOC_CONTROL(unit)->miimIntr,
			     SOC_CONTROL(unit)->miimTimeout) != 0) {
		rv = SOC_E_TIMEOUT;
	    } else {
		soc_cm_debug(DK_MIIM | DK_VERBOSE,
			     "  Interrupt received\n");
	    }
	    
	    soc_intr_disable(unit, IRQ_MIIM_OP_DONE);
	} else {
	    soc_timeout_t to;
	    
	    soc_timeout_init(&to, SOC_CONTROL(unit)->miimTimeout, 250);
	    
	    while ((soc_pci_read(unit, CMIC_SCHAN_CTRL) &
		    SC_MIIM_OP_DONE_TST) == 0) {
		if (soc_timeout_check(&to)) {
		    rv = SOC_E_TIMEOUT;
		    break;
		}
	    }
	    
	    if (rv == SOC_E_NONE) {
		soc_cm_debug(DK_MIIM | DK_VERBOSE,
			     "  Done in %d polls\n", to.polls);
	    }
	    
	    soc_pci_write(unit, CMIC_SCHAN_CTRL, SC_MIIM_OP_DONE_CLR);
	}
	
	if (rv == SOC_E_TIMEOUT) {
	    soc_cm_debug(DK_ERR,
			 "soc_miimc45_read: "
			 "timeout (id=0x%02x addr=0x%02x)\n",
			 phy_id, phy_reg_addr);
	    SOC_CONTROL(unit)->stat.err_mii_tmo++;
	} else {
	    *phy_rd_data = (uint16)soc_pci_read(unit, CMIC_MIIM_READ_DATA);
	    
	    soc_cm_debug(DK_MIIM,
			 "soc_sbx_miimc45_read: read data=0x%04x\n", *phy_rd_data);
	}
	
    } else {
	soc_cm_debug(DK_ERR,
		     "soc_miim_read: "
		     "only clause 22 and 45 are supported\n");
	rv = SOC_E_UNAVAIL;
    }
    
    return rv;
}

#endif /* BCM_SIRIUS_SUPPORT */

#ifdef BCM_BM9600_SUPPORT
STATIC int
_soc_bm9600_miim_write(int unit, uint32 phy_id, uint32 phy_reg_addr,
                       uint16 phy_wr_data)
{
    int rv = SOC_E_NONE;
    int port = phy_id;

#ifdef BCM_EASY_RELOAD_SUPPORT
    /* Indirect register write */
    if (SOC_IS_RELOADING(unit) && (phy_reg_addr == 0x1f)) {
        rv = soc_bm9600_mdio_hc_cl22_write_easy_reload(unit, port, phy_reg_addr, phy_wr_data);
    } else {
        rv = soc_bm9600_mdio_hc_cl22_write(unit, port, phy_reg_addr, phy_wr_data);
    }
#else
     rv = soc_bm9600_mdio_hc_cl22_write(unit, port, phy_reg_addr, phy_wr_data);
#endif

    if (rv != SOC_E_NONE) {
        soc_cm_debug(DK_ERR, "bm9600 miim write failed %d\n", rv);
    }
    return rv;
}
#endif /* BCM_BM9600_SUPPORT */


/*
 * Function:
 *     soc_sbx_miim_write
 * Purpose:
 *     Write a value to a MIIM register.
 * Parameters:
 *     unit         - Device number
 *     phy_id       - Phy ID encoding mdio address, mdio bus, internal phy
 *     phy_reg_addr - PHY register to write
 *     phy_wr_data  - Data to write
 * Returns:
 *     SOC_E_XXX
 */
int
soc_sbx_miim_write(int unit, uint32 phy_id, uint32 phy_reg_addr,
                   uint16 phy_wr_data)
{
    int  rv = SOC_E_UNAVAIL;

    soc_cm_debug(DK_MIIM,
                 "soc_sbx_miim_write: id=0x%02x addr=0x%02x data=0x%04x\n",
                 phy_id, phy_reg_addr, phy_wr_data);

    /* Pause link scanning and hold MIIM lock */
    soc_linkctrl_linkscan_pause(unit);
    MIIM_LOCK(unit);

#if defined(BCM_FE2000_SUPPORT)
    if (SOC_IS_SBX_FE2000(unit)) {
        rv = _soc_fe2000_miim_write(unit, CLAUSE_22,
                                    phy_id, phy_reg_addr, phy_wr_data);
    }
#endif

#if defined(BCM_SIRIUS_SUPPORT)
    if (SOC_IS_SIRIUS(unit)) {
        rv = _soc_sirius_miim_write(unit, CLAUSE_22,
                                    phy_id, phy_reg_addr, phy_wr_data);
    }
#endif
#if defined(BCM_BM9600_SUPPORT)
    if (SOC_IS_SBX_BM9600(unit)) {
        rv = _soc_bm9600_miim_write(unit, phy_id, phy_reg_addr, phy_wr_data);
    }
#endif
    /* Release linkscan pause and MIIM lock */
    MIIM_UNLOCK(unit);
    soc_linkctrl_linkscan_continue(unit);

    return rv;
}

#ifdef BCM_BM9600_SUPPORT
STATIC int
_soc_bm9600_miim_read(int unit, uint32 phy_id, uint32 phy_reg_addr,
                      uint16 *phy_rd_data)
{
    int rv;
    int port = phy_id;
    uint32 data;

    rv = soc_bm9600_mdio_hc_cl22_read(unit, port, phy_reg_addr, &data);
    if (rv != SOC_E_NONE) {
        soc_cm_debug(DK_ERR, "bm9600 miim read failed %d\n", rv);
    } else {
        *phy_rd_data = (uint16) data;
    }
    soc_cm_debug(DK_MIIM,
                 "soc_sbx_miim_read: read data=0x%04x\n", *phy_rd_data);
    
    return rv;
}
#endif /* BCM_BM9600_SUPPORT */

/*
 * Function:
 *     soc_sbx_miim_read
 * Purpose:
 *     Read a value from a MIIM register.
 * Parameters:
 *     unit         - Device number
 *     phy_id       - Phy ID encoding mdio address, mdio bus, internal phy
 *     phy_reg_addr - PHY register to read
 *     phy_rd_data  - (OUT) Returns data read
 * Returns:
 *     SOC_E_XXX
 */
int
soc_sbx_miim_read(int unit, uint32 phy_id, uint32 phy_reg_addr,
                  uint16 *phy_rd_data)
{
    int  rv = SOC_E_UNAVAIL;

    soc_cm_debug(DK_MIIM,
                 "soc_sbx_miim_read: id=0x%02x addr=0x%02x\n",
                 phy_id, phy_reg_addr);

    /* Pause link scanning and hold MIIM lock */
    soc_linkctrl_linkscan_pause(unit);
    MIIM_LOCK(unit);

#if defined(BCM_FE2000_SUPPORT)
    if (SOC_IS_SBX_FE2000(unit)) {
        rv = _soc_fe2000_miim_read(unit, CLAUSE_22,
                                   phy_id, phy_reg_addr, phy_rd_data);
    }
#endif

#if defined(BCM_SIRIUS_SUPPORT)
    if (SOC_IS_SIRIUS(unit)) {
        rv = _soc_sirius_miim_read(unit, CLAUSE_22,
                                   phy_id, phy_reg_addr, phy_rd_data);
    }
#endif
#if defined (BCM_BM9600_SUPPORT)
    if (SOC_IS_SBX_BM9600(unit)) {
        rv = _soc_bm9600_miim_read(unit, phy_id, phy_reg_addr, phy_rd_data);

    }
#endif
  
    /* Release linkscan pause and MIIM lock */
    MIIM_UNLOCK(unit);
    soc_linkctrl_linkscan_continue(unit);

    return rv;
}

/*
 * Function:
 *      soc_sbx_miimc45_write
 * Purpose:
 *     Write a value to a MIIM register.
 * Parameters:
 *     unit         - Device number
 *     phy_id       - Phy ID encoding mdio address, mdio bus, internal phy
 *     phy_reg_addr - PHY register to write (Encoded with PMA/PMD, PCS, PHY XS)
 *     phy_wr_data  - Data to write
 * Returns:
 *     SOC_E_XXX
 */
int
soc_sbx_miimc45_write(int unit, uint32 phy_id, uint32 phy_reg_addr,
                      uint16 phy_wr_data)
{
    int     rv = SOC_E_UNAVAIL;

    soc_cm_debug(DK_MIIM,
                 "soc_sbx_miimc45_write: id=0x%02x "
                 "addr=0x%02x data=0x%04x\n",
                 phy_id, phy_reg_addr, phy_wr_data);


    /* Pause link scanning and hold MIIM lock */
    soc_linkctrl_linkscan_pause(unit);
    MIIM_LOCK(unit);

#if defined(BCM_FE2000_SUPPORT)
    if (SOC_IS_SBX_FE2000(unit)) {
        rv = _soc_fe2000_miim_write(unit, CLAUSE_45,
                                    phy_id, phy_reg_addr, phy_wr_data);
    }
#endif

#if defined(BCM_SIRIUS_SUPPORT)
    if (SOC_IS_SIRIUS(unit)) {
        rv = _soc_sirius_miim_write(unit, CLAUSE_45,
                                   phy_id, phy_reg_addr, phy_wr_data);
    }
#endif

    /* Release linkscan pause and MIIM lock */
    MIIM_UNLOCK(unit);
    soc_linkctrl_linkscan_continue(unit);

    return rv;
}

/*
 * Function:
 *      soc_sbx_miimc45_write
 * Purpose:
 *     Read a value from a MIIM register.
 * Parameters:
 *     unit         - Device number
 *     phy_id       - Phy ID encoding mdio address, mdio bus, internal phy
 *     phy_reg_addr - PHY register to read (Encoded with PMA/PMD, PCS, PHY XS)
 *     phy_rd_data  - (OUT) Returns data read
 * Returns:
 *     SOC_E_XXX
 */
int
soc_sbx_miimc45_read(int unit, uint32 phy_id, uint32 phy_reg_addr,
                     uint16 *phy_rd_data)
{
    int     rv = SOC_E_UNAVAIL;

    soc_cm_debug(DK_MIIM,
                 "soc_sbx_miimc45_read: id=0x%02x "
                 "addr=0x%02x\n",
                 phy_id, phy_reg_addr);
 
    /* Pause link scanning and hold MIIM lock */
    soc_linkctrl_linkscan_pause(unit);
    MIIM_LOCK(unit);

#if defined(BCM_FE2000_SUPPORT)
    if (SOC_IS_SBX_FE2000(unit)) {
        rv = _soc_fe2000_miim_read(unit, CLAUSE_45,
                                   phy_id, phy_reg_addr, phy_rd_data);
    }
#endif

#if defined(BCM_SIRIUS_SUPPORT)
    if (SOC_IS_SIRIUS(unit)) {
        rv = _soc_sirius_miim_read(unit, CLAUSE_45,
                                   phy_id, phy_reg_addr, phy_rd_data);
    }
#endif

    /* Release linkscan pause and MIIM lock */
    MIIM_UNLOCK(unit);
    soc_linkctrl_linkscan_continue(unit);

    return rv;
}
