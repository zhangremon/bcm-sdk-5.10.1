/*
 * $Id: miim.c 1.37.2.1 Broadcom SDK $
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
 *
 * BCM5690 has an internal MDIO bus in addition to the external one.
 * The internal bus is accessed by setting bit 7 in the PHY ID.
 */

#include <sal/core/libc.h>
#include <sal/core/boot.h>

#include <soc/mem.h>
#include <soc/debug.h>
#include <soc/cm.h>

#include <soc/mcm/robo/driver.h>
#include <soc/error.h>
#include <soc/cmic.h>
#include <soc/drv.h>
#include <soc/spi.h>
#include <soc/phyctrl.h>

#define PHY_MII_BASEPAGE_UNKNOW         0xFF    /* unknow chip */
#define PHY_MII_BASEPAGE_INT            0x10    /* internal phy: bcm5324 bcm5498 */
#define PHY_MII_BASEPAGE_EXT            0x80    /* external phy: bcm5396 bcm5489 */

#define PHY_5348_MII_BASEPAGE_FE_0_TO_23  0xc0 /* BCM5348 port 0~23. External PHY */
#define PHY_5348_MII_BASEPAGE_FE_24_TO_47 0xa0 /* BCM5348 port 24~47. Internal PHY */
#define PHY_5348_MII_BASEPAGE_IMP_AND_GE  0xd8 /* BCM5348 port 48~52. External PHY */
#define PHY_5348_MII_BASEPAGE_INTER_SERDES 0xb8 /* BCM5348 port 49 ~52 Sedes MII*/
/* Defined here for further implementation of BCM5347 SDK */
#define PHY_5347_MII_BASEPAGE_FE_0_TO_23  0xa0 /* BCM5347 port 0~23. Internal PHY */
#define PHY_5347_MII_BASEPAGE_IMP_AND_GE  0xd8 /* BCM5347 port 24~28. External PHY */

#define PHY_53242_MII_BASEPAGE_FE_0_TO_23  0xa0 /* BCM53242 port 0~23. Internal PHY */
#define PHY_53242_MII_BASEPAGE_IMP  0xd8 /* BCM53242 port IMP. External PHY */
#define PHY_53242_MII_BASEPAGE_GE  0xd9 /* BCM53242 port 24~28. External PHY */

#define PHY_53115_MII_BASEPAGE_EXT_WAN  0x85 /* BCM53115 port5 External PHY */

#define PHY_TB_MII_BASEPAGE_FE  0xA0 /* Thunderbolt Internal PHY(FE) */
#define PHY_TB_MII_BASEPAGE_INT_SERDES  0xB9 /* Thunderbolt Internal SerDes */
#define PHY_TB_MII_BASEPAGE_EXT_GE  0xD8 /* Thunderbolt External PHY(GE) */

#define PHY_VO_MII_BASEPAGE_EXT_FE  0xC0 /* Voyager External S3MII PHY(FE) */
#define PHY_VO_MII_BASEPAGE_INT_SERDES  0xBB /* Voyager Internal SerDes (ge2(port27)/ge3(port28))*/


#define PHY_MIIREG_LENGTH_ROBO      2           /* 2 bytes */
                                   
#define UNKNOW_ROBO_PHY_MIIADDR(addr)      \
        (((addr) >> 8) == PHY_MII_BASEPAGE_UNKNOW)
                            
uint32
_soc_robo_serdes_phy_miiaddr_get(int unit, soc_port_t port, 
                                  uint16 phy_reg_addr)
{
    uint32 page = 0, offset= 0;

    offset = (uint32)(phy_reg_addr * PHY_MIIREG_LENGTH_ROBO);

    if (IS_FE_PORT(unit, port)) {
        /* BCM53101 build-in serdes on port5 only */
        if (SOC_IS_ROBO53101(unit) && (port == 5)) {
            page = (port + PHY_MII_BASEPAGE_INT) << 8;
            return (page | offset);
        }
    }

    if (!IS_GE_PORT(unit, port)) {
        return -1;
    }
    if (SOC_IS_ROBO5348(unit)||SOC_IS_ROBO5347(unit)
        ||SOC_IS_ROBO53262(unit)) {
        page = (port - NUM_FE_PORT(unit) ) + 
            PHY_5348_MII_BASEPAGE_INTER_SERDES;
        page <<= 8;
    } else if (SOC_IS_ROBODINO(unit)) {
    	page = (port + PHY_MII_BASEPAGE_INT) << 8;
    } else if (SOC_IS_ROBO53115(unit) || SOC_IS_ROBO53125(unit)){
        /* bcm53115 build-in serdes on port5 only. 
         *  - reture unknow page address for the internal SerDes miim reg 
         *      read/write access. (the external phy miim read/write will be 
         *      executed as a proper PHY miim read/write those known SerDes 
         *      ports)
         */
        if (port == 5){
    	    page = (port + PHY_MII_BASEPAGE_INT) << 8;
        } else {
    	    page = PHY_MII_BASEPAGE_UNKNOW << 8;
        }
    } else if (SOC_IS_TBX(unit)){
        if (IS_CPU_PORT(unit, port) || IS_GMII_PORT(unit, port)){
    	    page = PHY_MII_BASEPAGE_UNKNOW << 8;
        } else {
            if (SOC_IS_TB(unit)){            
                page = ((port - 24 - 1) + 
    	            PHY_TB_MII_BASEPAGE_INT_SERDES) << 8;
            } else if (SOC_IS_VO(unit)){            
                /* Voyager port 27/28 with SGMII interface*/
                page = ((port - 27) + 
    	            PHY_VO_MII_BASEPAGE_INT_SERDES) << 8;
            } else {
                page = PHY_MII_BASEPAGE_UNKNOW << 8;            
            }
        }
    } 
     return (page | offset);
}    
uint32
_soc_robo_phy_miiaddr_get(int unit, soc_port_t port, 
                                  uint16 phy_reg_addr)
{
    uint32 page = 0, offset= 0;
#ifdef BCM_TB_SUPPORT
    char *s;
#endif

    offset = (uint32)(phy_reg_addr * PHY_MIIREG_LENGTH_ROBO);
    
    if (SOC_IS_ROBO5324(unit)||SOC_IS_ROBO5398(unit)||SOC_IS_ROBO5397(unit)) {
    	page = (port + PHY_MII_BASEPAGE_INT) << 8;
    } else if (SOC_IS_ROBODINO(unit)) {
    	page = (port + PHY_MII_BASEPAGE_EXT) << 8;
    } else if (SOC_IS_ROBO5348(unit)) {
        if (port < 24) { /* Port 0~23, external phy */
            page = (port + PHY_5348_MII_BASEPAGE_FE_0_TO_23) << 8;
        } else if (port < 48) { /* Port 24~47, internal phy */
            page = ((port - 24) + PHY_5348_MII_BASEPAGE_FE_24_TO_47) << 8;
        } else if (port < 53) { /* Port 48 and 49~52, external ge phy */
            page = ((port - 48) + PHY_5348_MII_BASEPAGE_IMP_AND_GE) << 8;
        } else { /* Invalid port */
            page = PHY_MII_BASEPAGE_UNKNOW << 8;
        }
    } else if (SOC_IS_ROBO5347(unit)) {
        if (port < 24) { /* Port 24~47, internal phy */
            page = (port + PHY_5348_MII_BASEPAGE_FE_24_TO_47) << 8;
        } else if (port < 29) { /* Port 48 and 49~52, external ge phy */
            page = ((port - 24) + PHY_5348_MII_BASEPAGE_IMP_AND_GE) << 8;
        } else { /* Invalid port */
            page = PHY_MII_BASEPAGE_UNKNOW << 8;
        }
    } else if (SOC_IS_ROBO5395(unit) || SOC_IS_ROBO_ARCH_VULCAN(unit)) {
        if (port == CMIC_PORT(unit)) {
            page = (port + PHY_MII_BASEPAGE_EXT) << 8;
        } else {
            /* bcm53115 port5 within built-in SerDes and allowed the external 
             *  phy to connect through SGMII with this SerDes.
             */
            if ((SOC_IS_ROBO53115(unit) || SOC_IS_ROBO53101(unit) ||
                SOC_IS_ROBO53125(unit)) && 
                (port == 5)){
                page = (PHY_53115_MII_BASEPAGE_EXT_WAN) << 8;
            } else if (SOC_IS_ROBO53128V(unit) && (port == 7)) {
                page = (port + PHY_MII_BASEPAGE_EXT) << 8;
            } else {
                page = (port + PHY_MII_BASEPAGE_INT) << 8;
            }
        }
    } else if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
        if (port < 24) { /* Port 0~23, internal phy */
            page = (port + PHY_53242_MII_BASEPAGE_FE_0_TO_23) << 8;
        } else if (port == 24) { /* IMP Port, external phy */
            page = ((port - 24) + PHY_53242_MII_BASEPAGE_IMP) << 8;
        } else if (port < 29) { /* Port 25~28, external ge phy */
            page = ((port - 25) + PHY_53242_MII_BASEPAGE_GE) << 8;
        } else { /* Invalid port */
            page = PHY_MII_BASEPAGE_UNKNOW << 8;
        }
#ifdef BCM_TB_SUPPORT
    } else if (SOC_IS_TBX(unit)){
        s = soc_property_get_str(unit, "board_name");
        if( (s != NULL) && (sal_strcmp(s, "bcm53280_fpga") == 0)) {
            if (port < 4) {
                page = (PHY_TB_MII_BASEPAGE_FE + port) << 8;
            } else if (port < 29){
                page = (PHY_TB_MII_BASEPAGE_EXT_GE + 
                        (port - 24 )) << 8;
            }
        } else {
            if (port < 24) {
                page = (PHY_TB_MII_BASEPAGE_FE + port) << 8;
                if (SOC_IS_VO(unit) && 
                    (SOC_ROBO_CONTROL(unit)->chip_bonding == 
                    SOC_ROBO_BOND_PHY_S3MII)) {
                    /* bcm53606 Voyager with S3MII bonding */
                    page = (PHY_VO_MII_BASEPAGE_EXT_FE + port) << 8;
                }
            } else if (port < 29){
                if (IS_CPU_PORT(unit, port)){
                    page = PHY_TB_MII_BASEPAGE_EXT_GE << 8;
                } else if (IS_GE_PORT(unit, port)){
                    page = (PHY_TB_MII_BASEPAGE_EXT_GE + 
                            (port - 24 )) << 8;
                } else {
                    soc_cm_debug(DK_WARN,"%s,%d,Unexpected port property!",
                            FUNCTION_NAME(), __LINE__);
                    page = (PHY_TB_MII_BASEPAGE_INT_SERDES + 
                            (port - 24 )) << 8;
                }
            }
        }
#endif
    } else {
        page = PHY_MII_BASEPAGE_UNKNOW << 8;
    }
                                   
    return (page | offset);
}

/*
 * Function:
 *	soc_miim_write
 * Purpose:
 *	Write a value to a MIIM register.
 * Parameters:
 *	unit - StrataSwitch Unit #.
 *	phy_id - Phy ID to write (MIIM address)
 *	phy_reg_addr - PHY register to write
 *	phy_wr_data - Data to write.
 * Returns:
 *	SOC_E_XXX
 * Notes:
 * 	Temporarily disables auto link scan if it was enabled.  The MIIM
 * 	registers are locked during the operation to prevent multiple
 * 	tasks from trying to access PHY registers simultaneously.
 */

int 
drv_miim_write(int unit, uint8 phy_id,
	       uint8 phy_reg_addr, uint16 phy_wr_data)
{
    int	rv = SOC_E_NONE;
    int	len = 2; /* 16-bit register */
    uint32 addr;
    soc_port_t port;

    assert(!sal_int_context());

    soc_cm_debug(DK_MIIM,
		 "drv_miim_write: id=0x%02x addr=0x%02x data=0x%04x\n",
		 phy_id, phy_reg_addr, phy_wr_data);

#if defined(BCM_53101)
    if (SOC_IS_ROBO53101(unit) && 
        SOC_MAC_LOW_POWER_ENABLED(unit)) {
        soc_cm_debug(DK_ERR, "Don't allow to write the PHY addr 0x%02x.\n",
            phy_reg_addr);
        return SOC_E_UNAVAIL;
    }
#endif /* BCM_53101 */

    /* PHY_ADDR_ROBO_INT_SERDES is a special flag for indicating the GE port 
     *  is a ROBO internal SerDes (check phyident.c)
     */
    if (PHY_ADDR_ROBO_INT_FLAG_CHK(phy_id)) {
        PHY_ADDR_ROBO_INT_FLAG_CLR(phy_id);
        
        rv = soc_robo_miim_int_write(unit, phy_id, 
                phy_reg_addr, phy_wr_data);
        return rv;
    }

    port = soc_phy_addr_to_port(unit, phy_id);

    addr = _soc_robo_phy_miiaddr_get(unit, port, phy_reg_addr);

    if (UNKNOW_ROBO_PHY_MIIADDR(addr)){
        /* PHY_MII_BASEPAGE not defined for current robo_chip */
        return SOC_E_RESOURCE;
    }

#ifdef BE_HOST
    phy_wr_data = (phy_wr_data>>8) | (phy_wr_data<<8);
#endif

    SPI_LOCK;

    rv = soc_spi_write(unit, addr, (uint8*)&phy_wr_data, len);

    SPI_UNLOCK;

    return rv;
}

/*
 * Function:
 *      soc_robo_miim_write
 * Purpose:
 *      New interface to write a value to a MIIM register.
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      phy_id - Phy ID to write (MIIM address)
 *      phy_reg_addr - PHY register to write
 *      phy_wr_data - Data to write.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 * Added to have the same interface for PHY register access among
 * ESW, ROBO, SBX
 */
int
soc_robo_miim_write(int unit, uint32 phy_id,
                   uint32 phy_reg_addr, uint16 phy_wr_data)
{
    return drv_miim_write(unit, (uint8)phy_id,
                          (uint16)phy_reg_addr, phy_wr_data);
}

/*
 * Function:
 *	soc_miim_read
 * Purpose:
 *	Read a value from an MII register.
 * Parameters:
 *	unit - StrataSwitch Unit #.
 *	phy_id - Phy ID to write (MIIM address)
 *	phy_reg_addr - PHY register to write
 *	phy_rd_data - 16bit data to write into
 * Returns:
 *	SOC_E_XXX
 * Notes:
 * 	Temporarily disables auto link scan if it was enabled.  The MIIM
 * 	registers are locked during the operation to prevent multiple
 * 	tasks from trying to access PHY registers simultaneously.
 */

int
drv_miim_read(int unit, uint8 phy_id,
	      uint8 phy_reg_addr, uint16 *phy_rd_data)
{
    int	rv = SOC_E_NONE;
    int	len = 2; /* 16-bit register */
    uint32 addr;
    soc_port_t port;

    assert(!sal_int_context());
    assert(phy_rd_data);

    soc_cm_debug(DK_MIIM,
		 "drv_miim_read: id=0x%02x addr=0x%02x\n",
		 phy_id, phy_reg_addr);

#if defined(BCM_53101)
    if (SOC_IS_ROBO53101(unit) && 
        SOC_MAC_LOW_POWER_ENABLED(unit)) {
        soc_cm_debug(DK_ERR, "Don't allow to read the PHY addr 0x%02x.\n",
            phy_reg_addr);
        return SOC_E_UNAVAIL;
    }
#endif /* BCM_53101 */    

    if (PHY_ADDR_ROBO_INT_FLAG_CHK(phy_id)) {
        PHY_ADDR_ROBO_INT_FLAG_CLR(phy_id);
        
        rv = soc_robo_miim_int_read(unit, phy_id, 
                phy_reg_addr, phy_rd_data);
        return rv;
    }
    
    port = soc_phy_addr_to_port(unit, phy_id);

    addr = _soc_robo_phy_miiaddr_get(unit, port, phy_reg_addr);
    
    if (UNKNOW_ROBO_PHY_MIIADDR(addr)){
        /* PHY_MII_BASEPAGE not defined for current robo_chip */
        return SOC_E_RESOURCE;
    }

    SPI_LOCK;

    rv = soc_spi_read(unit, addr, (uint8*)phy_rd_data, len);

    SPI_UNLOCK;
#ifdef BE_HOST
    *phy_rd_data = (*phy_rd_data>>8) | (*phy_rd_data<<8);
#endif

    soc_cm_debug(DK_MIIM, "drv_miim_read: spi_addr=0x%04x,read data=0x%04x\n", 
            addr, *phy_rd_data);

    return rv;
}

/*
 * Function:
 *      soc_robo_miim_read
 * Purpose:
 *      New interface to read a value from a MIIM register.
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      phy_id - Phy ID to write (MIIM address)
 *      phy_reg_addr - PHY register to write
 *      phy_rd_data - Data read.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 * Added to have the same interface for PHY register access among
 * ESW, ROBO, SBX.
 */
int
soc_robo_miim_read(int unit, uint32 phy_id,
                  uint32 phy_reg_addr, uint16 *phy_rd_data)
{
    return drv_miim_read(unit, (uint8)phy_id,
                         (uint16)phy_reg_addr, phy_rd_data);
}

/*
 * Function:
 *      soc_robo_miim_int_write
 * Purpose:
 *      New interface to write a value to a MIIM register.
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      phy_id - Phy ID to write (MIIM address)
 *      phy_reg_addr - PHY register to write
 *      phy_wr_data - Data to write.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 * Added to have the same interface for PHY register access among
 * ESW, ROBO, SBX
 */
int
soc_robo_miim_int_write(int unit, uint32 phy_id,
                   uint32 phy_reg_addr, uint16 phy_wr_data)
{
    int	rv = SOC_E_NONE;
    int	len = 2; /* 16-bit register */
    uint32 addr;
    soc_port_t port;

    assert(!sal_int_context());

    soc_cm_debug(DK_MIIM,
		 "soc_robo_miim_int_write: id=0x%02x addr=0x%02x data=0x%04x\n",
		 phy_id, phy_reg_addr, phy_wr_data);

    port = soc_phy_addr_to_port(unit, phy_id);
    
    /* soc_feature_dodeca_serdes is a system-wide SerDes feature 
     *  definition on all GE port in this chip, but is not suitable for 
     *  some ROBO device. Like bcm53115, 6 GE ports included, but the last 
     *  GE port is a built-in SerDes.
     */
    if (!IS_ROBO_SPECIFIC_INT_SERDES(unit, port)){
        if (!soc_feature(unit, soc_feature_dodeca_serdes)) {
            return rv;
        }
    }

    if (!IS_GE_PORT(unit, port)){
        return rv;
    }

    addr = _soc_robo_serdes_phy_miiaddr_get(unit, port, phy_reg_addr);
    if (UNKNOW_ROBO_PHY_MIIADDR(addr)){
        /* PHY_MII_BASEPAGE not defined for current robo_chip */
        return SOC_E_RESOURCE;
    }

#ifdef BE_HOST
    phy_wr_data = (phy_wr_data>>8) | (phy_wr_data<<8);
#endif

    SPI_LOCK;

    rv = soc_spi_write(unit, addr, (uint8*)&phy_wr_data, len);

    SPI_UNLOCK;

    return rv;

}
/*
 * Function:
 *      soc_robo_miim_int_read
 * Purpose:
 *      New interface to read a value from a MIIM register.
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      phy_id - Phy ID to write (MIIM address)
 *      phy_reg_addr - PHY register to write
 *      phy_rd_data - Data read.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 * Added to have the same interface for PHY register access among
 * ESW, ROBO, SBX.
 */
int
soc_robo_miim_int_read(int unit, uint32 phy_id,
                  uint32 phy_reg_addr, uint16 *phy_rd_data)
{
    int	rv = SOC_E_NONE;
    int	len = 2; /* 16-bit register */
    uint32 addr;
    soc_port_t port;

    assert(!sal_int_context());
    assert(phy_rd_data);

    soc_cm_debug(DK_MIIM,
		 "soc_robo_miim_int_read: id=0x%02x addr=0x%02x\n",
		 phy_id, phy_reg_addr);

    port = soc_phy_addr_to_port(unit, phy_id);
    
    /* soc_feature_dodeca_serdes is a system-wide SerDes feature 
     *  definition on all GE port in this chip, but is not suitable for 
     *  some ROBO device. Like bcm53115, 6 GE ports included, but the last 
     *  GE port is a built-in SerDes.
     */
    if (!IS_ROBO_SPECIFIC_INT_SERDES(unit, port)){
        if (!soc_feature(unit, soc_feature_dodeca_serdes)) {
            *phy_rd_data = 0;
            return rv;
        }
    }

    if (!IS_GE_PORT(unit, port)){
        *phy_rd_data = 0;
        return rv;
    }

    addr = _soc_robo_serdes_phy_miiaddr_get(unit, port, phy_reg_addr);

    if (UNKNOW_ROBO_PHY_MIIADDR(addr)){
        /* PHY_MII_BASEPAGE not defined for current robo_chip */
        *phy_rd_data = 0;
        return SOC_E_RESOURCE;
    }

    SPI_LOCK;

    rv = soc_spi_read(unit, addr, (uint8*)phy_rd_data, len);

    SPI_UNLOCK;
#ifdef BE_HOST
    *phy_rd_data = (*phy_rd_data>>8) | (*phy_rd_data<<8);
#endif

    soc_cm_debug(DK_MIIM, "soc_robo_miim_int_read: read data=0x%04x\n", *phy_rd_data);

    return rv;
}
