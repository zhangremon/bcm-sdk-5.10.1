/*
 * $Id: spi.c 1.15 Broadcom SDK $
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
 * Routines for accessing BCM53xx SPI memory mapped registers
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

#include <shared/et/osl.h>
#include <shared/et/bcmendian.h>
#include <shared/et/bcmutils.h>
#include <shared/et/proto/ethernet.h>
#include <shared/et/bcmenetmib.h>
#include <shared/et/bcmenetrxh.h>
#include <shared/et/et_dbg.h>
#include <shared/et/et_export.h>
#include <soc/etc.h>

#include <ibde.h>

#ifdef MDC_MDIO_SUPPORT
#include <soc/phy.h>

extern et_soc_info_t *et_soc;
extern int access_page[SOC_MAX_NUM_SWITCH_DEVICES];

#define PSEUDO_PHY_ADDR 0x1e
#define PHYRD_TIMEOUT 500    

int 
_mdc_mdio_phy_read(int unit, uint32 addr, uint8 *buf, int len)
{
    uint8 phyaddr, regoffset, tmp;
    uint16 *rd_val;

        tmp = phyaddr = (addr >> 8) & 0xFF;

    if (SOC_IS_TBX(unit)){
#ifdef BCM_TB_SUPPORT
        /* internal serdes existed check is done in _mii_reg_offset_check()*/
       if (tmp >= 0xd8) {
            phyaddr -= 0xc0;
        } else {
            phyaddr -= 0xa0;
        }
#endif
    } else if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)||
        SOC_IS_ROBO5348(unit)||SOC_IS_ROBO5347(unit)) {
        if (tmp >= 0xd8) {
            phyaddr -= 0xc0;
        } else {
            phyaddr -= 0xa0;
        }
    } else if (SOC_IS_ROBO5389(unit)||SOC_IS_ROBO5396(unit)) {
        if (tmp >= 0x80) {
            phyaddr -= 0x70; /* External phy id start from 0x10 */
        } else {
            phyaddr -= 0x10; /* Internal phy id start from 0x0 */
        }
    } else if (SOC_IS_ROBO5395(unit)) {
        if ((tmp >= 0x10) && (tmp <= 0x14)) {
            phyaddr -= 0x10;
        }
    } else if (SOC_IS_ROBO53115(unit) || SOC_IS_ROBO53101(unit) ||
        SOC_IS_ROBO53125(unit)) {
        if (tmp == 0x85) {
            phyaddr = 0x15;
        } else if (tmp == 0x88) {
            phyaddr = 0x18;
        } else {
            phyaddr -= 0x10;
        }
    } else if (SOC_IS_ROBO53128V(unit)) {
        if (tmp == 0x87) {
            phyaddr = 0x17;
        }
    } else { /* 5324/5398/5397/53118 */
        phyaddr -= 0x10;
    }

    regoffset = addr & 0xFF;
    regoffset /= 2;

    rd_val = (uint16 *)buf;

    *rd_val = et_soc_phyrd(et_soc, phyaddr, regoffset);
#ifdef BE_HOST
    *rd_val = (*rd_val >> 8) | (*rd_val << 8);
#endif
    return SOC_E_NONE;
}

int 
_mdc_mdio_phy_write(int unit, uint32 addr, uint8 *buf, int len)
{
    uint8 phyaddr, regoffset, tmp;
    uint16 *wr_buf;

        tmp = phyaddr = (addr >> 8) & 0xFF;
    if (SOC_IS_TBX(unit)){
#ifdef BCM_TB_SUPPORT
        /* internal serdes existed check is done in _mii_reg_offset_check()*/
       if (tmp >= 0xd8) {
            phyaddr -= 0xc0;
        } else {
            phyaddr -= 0xa0;
        }
#endif
    } else if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)||
        SOC_IS_ROBO5348(unit)||SOC_IS_ROBO5347(unit)) {
        if (tmp >= 0xd8) {
            phyaddr -= 0xc0;
        } else {
            phyaddr -= 0xa0;
        }
    } else if (SOC_IS_ROBO5389(unit)||SOC_IS_ROBO5396(unit)) {
        if (tmp >= 0x80) {
            phyaddr -= 0x70; /* External phy id start from 0x10 */
        } else {
            phyaddr -= 0x10; /* Internal phy id start from 0x0 */
        }
    } else if (SOC_IS_ROBO5395(unit)) {
        if ((tmp >= 0x10) && (tmp <= 0x14)) {
            phyaddr -= 0x10;
        }
    } else if (SOC_IS_ROBO53115(unit) || SOC_IS_ROBO53101(unit) ||
        SOC_IS_ROBO53125(unit)) {
        if (tmp == 0x85) {
            phyaddr = 0x15;
        } else if (tmp == 0x88) {
            phyaddr = 0x18;
        } else {
            phyaddr -= 0x10;
        }
    } else if (SOC_IS_ROBO53128V(unit)) {
        if (tmp == 0x87) {
            phyaddr = 0x17;
        }
    } else { /* 5324/5398/5397/53118 */
        phyaddr -= 0x10;
    }

    regoffset = addr & 0xFF;
    regoffset /= 2;

    wr_buf = (uint16 *)buf;
#ifdef BE_HOST
    *wr_buf = (*wr_buf >> 8) | (*wr_buf << 8);
#endif
    et_soc_phywr(et_soc, phyaddr, regoffset, *wr_buf);

    return SOC_E_NONE;
}

#define LINK_STATUS_SUMMARY   0x10
#define LINK_STATUS_CHANGE    0x14
#define PORT_SPEED_SUMMARY    0x18
#define DUPLEX_STATUS_SUMMARY 0x1c
#define PAUSE_STATUS_SUMMARY  0x20

#define AN_COMPLETE_TIMEOUT 5000
#define TX_PAUSE_STATE 0
#define RX_PAUSE_STATE 1

static uint16 last_link_status_ge0 = 0;
static uint16 last_link_status_ge1 = 0;

static bool
_mii_reg_offset_check(int unit, uint8 page)
{
    bool val = FALSE;
    uint16      dev_id;
    uint8       rev_id;
    bool serdes = FALSE;
    
    soc_cm_get_id(unit, &dev_id, &rev_id);

    if (SOC_IS_ROBO5324(unit)) {
        if ((page >= 0x10) && (page <= 0x2a)) {
            val = TRUE;
        }
    } else if (SOC_IS_ROBO5347(unit)||SOC_IS_ROBO5348(unit)) {
        if (((page >= 0xa0) && (page <= 0xb7)) || \
            ((page >= 0xc0) && (page <= 0xdc))) {
            val = TRUE;
        }
    } else if (SOC_IS_ROBO5389(unit)) {
        if (((page >= 0x10) && (page <= 0x17)) || \
            ((page >= 0x80) && (page <= 0x87))) {
            val = TRUE;
        }
    } else if (SOC_IS_ROBO5395(unit)) {
        if ((page >= 0x10) && (page <= 0x14)) {
            val = TRUE;
        }
    } else if (SOC_IS_ROBO5396(unit)) {
        if (((page >= 0x10) && (page <= 0x1f)) || \
            ((page >= 0x80) && (page <= 0x8f))) {
            val = TRUE;
        }
    } else if (SOC_IS_ROBO5397(unit)||SOC_IS_ROBO5398(unit)) {
        if ((page >= 0x10) && (page <= 0x17)) {
            val = TRUE;
        }
    } else if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
        if (((page >= 0xa0) && (page <= 0xb7)) || \
            ((page >= 0xd8) && (page <= 0xdc))) {
            val = TRUE;
        }
    } else if (SOC_IS_ROBO53118(unit) || SOC_IS_ROBO53128(unit)) {
        if ((page >= 0x10) && (page <= 0x17)) {
            val = TRUE;
        }
        if (SOC_IS_ROBO53128V(unit) && (page == 0x87)) {
            val = TRUE;
        }
    } else if (SOC_IS_ROBO53115(unit) || SOC_IS_ROBO53101(unit) ||
        SOC_IS_ROBO53125(unit)) {
        if (((page >= 0x10) && (page <= 0x17)) || \
            (page == 0x85) || \
            (page == 0x88)) {
            /* 
             * 0x10~0x17: front ports' phy 
             * 0x85: wan port's phy
             * 0x88: imp port external phy
             */
            val = TRUE;
        }
    } else if (SOC_IS_TB(unit)) {
#ifdef BCM_TB_SUPPORT
          switch (dev_id) {
                case BCM53284_DEVICE_ID:
                case BCM53283_DEVICE_ID:
                case BCM53282_DEVICE_ID:                    
                    serdes = FALSE;
                    break;
                case BCM53286_DEVICE_ID:
                case BCM53288_DEVICE_ID:
                    serdes = TRUE;
                    break;
        }
        if (((page >= 0xa0) && (page <= 0xb7)) ||
            ((page >= 0xd8) && (page <= 0xdc)) ||
            ((page >= 0xb9) && (page <= 0xbc))
            ){
            if ((page >= 0xa0) && (page <= 0xb7)) {
                val = TRUE;
            }
            if ((page >= 0xb9) && (page <= 0xbc)) {
                if(serdes) {
                    val = TRUE;
                } else {
                    SOC_DEBUG_PRINT((DK_WARN,
                        "TB: register (page :0x%x) is not exist in \
                            bcm53284/53283/53282",
                        page));
                    val = FALSE;
                }
            } 
            if ((page >= 0xd9) && (page <= 0xdc)) {
                if(serdes) {
                    SOC_DEBUG_PRINT((DK_WARN,
                        "TB: register (page :0x%x) can not be access via mdc/mdio \
                        due to the shortage of phy id supported in cpu",
                        page));
                    val = FALSE;
                } else {
                    val = TRUE;
                }
            } 
        } else {
            val = FALSE;
        }
#endif /* BCM_TB_SUPPORT */
    /* Add new chips' page check of mii registers here. */
    } else {
        val = FALSE;
    }

    return val;
}

static bool
_mii_status_reg_check(int unit, uint32 addr)
{
    bool val = FALSE;
    if (SOC_IS_ROBO5324(unit) ||
        SOC_IS_ROBO5347(unit) || SOC_IS_ROBO5348(unit) ||
        SOC_IS_ROBO5389(unit) || SOC_IS_ROBO5395(unit) ||
        SOC_IS_ROBO5397(unit) || SOC_IS_ROBO5398(unit) ||
        SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit) ||
        SOC_IS_ROBO_ARCH_VULCAN(unit)) {
        if ((addr == SOC_REG_INFO(unit, LNKSTSr).offset) || \
            (addr == SOC_REG_INFO(unit, LNKSTSCHGr).offset) || \
            (addr == SOC_REG_INFO(unit, SPDSTSr).offset) || \
            (addr == SOC_REG_INFO(unit, DUPSTSr).offset) || \
            (addr == SOC_REG_INFO(unit, PAUSESTSr).offset)) {
            val = TRUE;
        }
    } else if (SOC_IS_ROBO5396(unit)) {
        if ((addr == SOC_REG_INFO(unit, LNKSTSr).offset) || \
            (addr == SOC_REG_INFO(unit, LNKSTSCHGr).offset) || \
            (addr == SOC_REG_INFO(unit, SPDSTSr).offset) || \
            (addr == SOC_REG_INFO(unit, DUPSTSr).offset) || \
            (addr == SOC_REG_INFO(unit, RXPAUSESTSr).offset) || \
            (addr == SOC_REG_INFO(unit, TXPAUSESTSr).offset)) {
            val = TRUE;
        }
    /* Add new chips' address check of phy status registers here. */
    } else {
        val = FALSE;
    }

    return val;
}

uint16
_get_MII_ASSR_REG_value(soc_port_t port)
{
    uint8 process = 1;
    uint16 wait_counter = 0;
    uint16 rd_buf = 0;

    while (process) {
        rd_buf = et_soc_phyrd(et_soc, port, 0x19);
        if (rd_buf & MII_ASSR_ANC) {
            process = 0;
        }
        wait_counter ++;

        if (wait_counter > AN_COMPLETE_TIMEOUT) {
            soc_cm_debug(DK_REG, "Get port %d status timeout\n", port);
            break;
        }
    }
    
    return rd_buf;
}

uint16
_get_ge_port_link_status(int unit, soc_port_t port)
{
    uint16 value = 0;

    value = _get_MII_ASSR_REG_value(port);

    value &= MII_ASSR_LS;
    value >>= 2;

    return (value);
}

uint16
_get_ge_port_speed_duplex(int unit, soc_port_t port, uint8 mode)
{
    uint16 value = 0;

    value = _get_MII_ASSR_REG_value(port);
    value &= MII_ASSR_HCD;
    value >>= 8;

    switch(mode) {
        case PORT_SPEED_SUMMARY:
            if ((value == 0x7) || (value == 0x6)) {
                value = 0x2; /* 1000 Mb/s */
            } else if ((value == 0x5) || (value == 0x3)) {
                value = 0x1; /* 100 Mb/s */
            } else if ((value == 0x2) || (value == 0x1)) {
                value = 0x0; /* 10 Mb/s */
            }           
            break;
        case DUPLEX_STATUS_SUMMARY:
            if ((value == 0x7) || (value == 0x5) || (value == 0x2)) {
                value = 0x1; /* Full Duplex */
            } else if ((value == 0x6) || (value == 0x3) || (value == 0x1)) {
                value = 0x0; /* Half Duplex */
            }           
            break;
    }
    
    return (value);
}

uint16
_get_ge_port_pause_status(int unit, soc_port_t port, uint8 mode)
{
    uint16 value = 0;

    value = _get_MII_ASSR_REG_value(port);

    switch (mode) {
        case TX_PAUSE_STATE:
            value &= MII_ASSR_PRTD;
            break;
        case RX_PAUSE_STATE:
            value &= MII_ASSR_PRRD;
            value >>= 1;
            break;
    }
    
    return (value);
}

int 
_mdc_mdio_get_ge_port_status(int unit, uint32 addr, uint8 *buf, int len)
{
    uint8 offset;
    uint16 ge0_phy_status = 0, ge1_phy_status = 0;
    uint8 *status;
    offset = addr & 0xFF;

    status = (uint8 *)(buf+3);


    /* clear GE ports status */
    *status &= 0x1f;

    switch(offset) {
        case LINK_STATUS_SUMMARY:
            ge0_phy_status = _get_ge_port_link_status(unit, 0x19);
            ge1_phy_status = _get_ge_port_link_status(unit, 0x1a);
            
            *status |= ((ge0_phy_status << 1) | (ge1_phy_status << 2));
            break;
        case LINK_STATUS_CHANGE:
            ge0_phy_status = _get_ge_port_link_status(unit, 0x19);
            if (ge0_phy_status != last_link_status_ge0) {
                last_link_status_ge0 = ge0_phy_status;
                ge0_phy_status = 1;
            } else {
                ge0_phy_status = 0;
            }
            ge1_phy_status = _get_ge_port_link_status(unit, 0x1a);
            if (ge1_phy_status != last_link_status_ge1) {
                last_link_status_ge1 = ge1_phy_status;
                ge1_phy_status = 1;
            } else {
                ge1_phy_status = 0;
            }

            *status |= ((ge0_phy_status << 1) | (ge1_phy_status << 2));
            break;
        case PORT_SPEED_SUMMARY:
            ge0_phy_status = \
                _get_ge_port_speed_duplex(unit, 0x19, PORT_SPEED_SUMMARY);
            ge1_phy_status = \
                _get_ge_port_speed_duplex(unit, 0x1a, PORT_SPEED_SUMMARY);
            
            *status |= ((ge0_phy_status << 1) | (ge1_phy_status << 3));
            break;
        case DUPLEX_STATUS_SUMMARY:
            ge0_phy_status = \
                _get_ge_port_speed_duplex(unit, 0x19, DUPLEX_STATUS_SUMMARY);
            ge1_phy_status = \
                _get_ge_port_speed_duplex(unit, 0x1a, DUPLEX_STATUS_SUMMARY);

            *status |= ((ge0_phy_status << 1) | (ge1_phy_status << 2));
            break;
        case PAUSE_STATUS_SUMMARY:
            ge0_phy_status = \
                _get_ge_port_pause_status(unit, 0x19, TX_PAUSE_STATE);
            ge1_phy_status = \
                _get_ge_port_pause_status(unit, 0x1a, TX_PAUSE_STATE);

            *status |= ((ge0_phy_status << 1) | (ge1_phy_status << 3));

            ge0_phy_status = \
                _get_ge_port_pause_status(unit, 0x19, RX_PAUSE_STATE);
            ge1_phy_status = \
                _get_ge_port_pause_status(unit, 0x1a, RX_PAUSE_STATE);

            *status |= ((ge0_phy_status << 2) | (ge1_phy_status << 4));
            break;
        default:
            return SOC_E_PARAM;
            break;
    }

    return SOC_E_NONE;
}


int
_mdc_mdio_reg_read(int unit, uint32 addr, uint8 *buf, int len)
{
    uint8 page, offset;
    uint16 phywr_val = 0, phyrd_val = 0;
    uint16 process,phyrd_timeout_count;
    uint16 *rd_buf = NULL;

    if (et_soc) {
        page = (addr >> SOC_ROBO_PAGE_BP) & 0xFF;
        offset = addr & 0xFF;

        /* If phy pages, read phy registers directly instead of via mac */
        if (_mii_reg_offset_check(unit, page)) {
            return (_mdc_mdio_phy_read(unit, addr, buf, len));
        }
            
        /* If accessing register is in another page*/
        if (page != access_page[unit]) {
            phywr_val = (page << 8) | (unit & 0x3) << 1 | 0x1;
            et_soc_phywr(et_soc, PSEUDO_PHY_ADDR, 16, phywr_val);
            access_page[unit] = page;
        }
            
        phywr_val = (offset << 8) | 0x2; /*OP code read.*/
        et_soc_phywr(et_soc, PSEUDO_PHY_ADDR, 17, phywr_val);
        
        process = 1;
        phyrd_timeout_count = 0;

        while (process) {
            phyrd_val = et_soc_phyrd(et_soc, PSEUDO_PHY_ADDR, 17);
            if (!(phyrd_val & 0x03)) {
                process = 0;
            }
            phyrd_timeout_count ++;
        
            if (phyrd_timeout_count > PHYRD_TIMEOUT) {
                return SOC_E_TIMEOUT;
            }
        }
        
        rd_buf = (uint16*)buf;
        *rd_buf = et_soc_phyrd(et_soc, PSEUDO_PHY_ADDR, 24);
        *(rd_buf + 1) = et_soc_phyrd(et_soc, PSEUDO_PHY_ADDR, 25);
        *(rd_buf + 2) = et_soc_phyrd(et_soc, PSEUDO_PHY_ADDR, 26);
        *(rd_buf + 3) = et_soc_phyrd(et_soc, PSEUDO_PHY_ADDR, 27);
#ifdef BE_HOST
        *rd_buf = (*rd_buf >> 8) | (*rd_buf << 8);
        *(rd_buf + 1) = (*(rd_buf + 1) >> 8) | (*(rd_buf + 1) << 8);
        *(rd_buf + 2) = (*(rd_buf + 2) >> 8) | (*(rd_buf + 2) << 8);
        *(rd_buf + 3) = (*(rd_buf + 3) >> 8) | (*(rd_buf + 3) << 8);
#endif
        /* 
         * Status Register: 
         * Because phy polling disabled, the GE ports phy status
         * have to be read from phy registers directly.
         *
         * Currently, only 5324 need this operation.
         */
        if (SOC_IS_ROBO5324(unit)) {
            if (_mii_status_reg_check(unit, addr)) {
                return (_mdc_mdio_get_ge_port_status(unit, addr, buf, len));
            }
        }
    }else {
        soc_cm_debug(DK_REG, "soc_spi_read: MDC/MDIO addr %d failed\n", addr);
    }
    
    return SOC_E_NONE;
}   
int
_mdc_mdio_reg_write(int unit, uint32 addr, uint8 *buf, int len)
{
    uint8 page = 0, offset = 0;
    uint16 phywr_val = 0, phyrd_val = 0;
    uint16 process,phyrd_timeout_count;
    uint16 *wr_buf;
    int hw_reset = 0;

    if (et_soc) {
        page = (addr >> SOC_ROBO_PAGE_BP) & 0xFF;
        offset = addr & 0xFF;

        /* If phy pages, read phy registers directly instead of via mac */
        if (_mii_reg_offset_check(unit, page)) {
            return (_mdc_mdio_phy_write(unit, addr, buf, len));
        }

        /* If accessing register is in another page*/
        if (page != access_page[unit]) {
            phywr_val = (page << 8) | (unit & 0x3) << 1 | 0x1;
            et_soc_phywr(et_soc, PSEUDO_PHY_ADDR, 16, phywr_val);
            access_page[unit] = page;
        }

        wr_buf = (uint16 *)buf;
#ifdef BE_HOST
        *wr_buf = (*wr_buf >> 8) | (*wr_buf << 8);
        *(wr_buf + 1) = (*(wr_buf + 1) >> 8) | (*(wr_buf + 1) << 8);
        *(wr_buf + 2) = (*(wr_buf + 2) >> 8) | (*(wr_buf + 2) << 8);
        *(wr_buf + 3) = (*(wr_buf + 3) >> 8) | (*(wr_buf + 3) << 8);
#endif
        et_soc_phywr(et_soc, PSEUDO_PHY_ADDR, 24, *wr_buf);
        et_soc_phywr(et_soc, PSEUDO_PHY_ADDR, 25, *(wr_buf + 1));
        et_soc_phywr(et_soc, PSEUDO_PHY_ADDR, 26, *(wr_buf + 2));
        et_soc_phywr(et_soc, PSEUDO_PHY_ADDR, 27, *(wr_buf + 3));

        phywr_val = (offset << 8) | 0x1; /*OP code wrte.*/
        et_soc_phywr(et_soc, PSEUDO_PHY_ADDR, 17, phywr_val);

        if (SOC_IS_ROBO5398(unit) || SOC_IS_ROBO5397(unit) ||
            SOC_IS_ROBO5395(unit) || SOC_IS_ROBO_ARCH_VULCAN(unit)) {
            if ((page == 0x0) && (offset == 0x79) && ((*wr_buf & 0x90) == 0x90)) {
                hw_reset = 1;
            }
        }

        if (SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit) ||
            SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit) ||
            SOC_IS_TBX(unit)) {
            if ((page == 0x03) && (offset == 0x7c) && ((*wr_buf & 0x01) == 0x01)) {
                hw_reset = 1;
            }
        }

        process = 1;
        phyrd_timeout_count = 0;
        while (process) {
            if (hw_reset) {
                /* 
                 * For HW reset operation, 
                 * waveform need to be re-enterered after HW reset configured 
                 * in oreder to complete the entire register write cycle.
                 */
                phywr_val = (page << 8) | (unit & 0x3) << 1 | 0x1;
                et_soc_phywr(et_soc, PSEUDO_PHY_ADDR, 16, phywr_val);
                et_soc_phywr(et_soc, PSEUDO_PHY_ADDR, 24, *wr_buf);
                et_soc_phywr(et_soc, PSEUDO_PHY_ADDR, 25, *(wr_buf + 1));
                et_soc_phywr(et_soc, PSEUDO_PHY_ADDR, 26, *(wr_buf + 2));
                et_soc_phywr(et_soc, PSEUDO_PHY_ADDR, 27, *(wr_buf + 3));
            }
            phyrd_val = et_soc_phyrd(et_soc, PSEUDO_PHY_ADDR, 17);
            if (!(phyrd_val & 0x03)) {
                process = 0;
            }
            phyrd_timeout_count ++;
        
            if (phyrd_timeout_count > PHYRD_TIMEOUT) {
                return SOC_E_TIMEOUT;
            }
        }
    } else {
        soc_cm_debug(DK_REG, "soc_spi_write: MDC/MDIO addr %d failed\n", addr);
    }

    return SOC_E_NONE;
}   
#endif

int
soc_spi_read(int unit, uint32 addr, uint8 *buf, int len)
{
#ifdef MDC_MDIO_SUPPORT
    return (_mdc_mdio_reg_read(unit, addr, buf, len));
#else
    CMVEC(unit).spi_read(&CMDEV(unit).dev, addr, buf, len);
    return SOC_E_NONE;
#endif
}

int
soc_spi_write(int unit, uint32 addr, uint8 *buf, int len)
{
#ifdef MDC_MDIO_SUPPORT
    return (_mdc_mdio_reg_write(unit, addr, buf, len));
#else
    CMVEC(unit).spi_write(&CMDEV(unit).dev, addr, buf, len);
    return SOC_E_NONE;
#endif
}
