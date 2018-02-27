/*
 * $Id: cmicsim.c 1.132.4.4 Broadcom SDK $
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
 * The part of PCID that simulates the cmic
 *
 * Requires:
 *
 * Provides:
 *     soc_internal_pcic_init
 *     soc_internal_read
 *     soc_internal_write
 *     soc_internal_reset
 *     soc_internal_vlan_tab_init
 *     soc_internal_memory_fetch
 *     soc_internal_memory_store
 *     soc_internal_manual_hash
 *     soc_internal_vlan_tab_init
 *     soc_internal_manual_hash
 *     soc_internal_send_int
 *     soc_internal_table_dma
 *     soc_internal_schan_ctrl_write
 */

#include <unistd.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/time.h>
#include <soc/mem.h>
#include <soc/hash.h>

#include <soc/cmic.h>
#ifdef BCM_CMICM_SUPPORT
#include <soc/cmicm.h>
#endif
#include <soc/drv.h>
#ifdef BCM_SBX_SUPPORT
#include <soc/sbx/sbx_drv.h>
#endif
#include <soc/debug.h>
#include <sal/appl/io.h>
#include <bde/pli/verinet.h>

#include "pcid.h"
#include "mem.h"
#include "cmicsim.h"
#include "dma.h"
#ifdef BCM_ISM_SUPPORT
#include "ism.h"
#endif

/*
 * PCI Config Space Simulation
 */

void
soc_internal_pcic_init(pcid_info_t *pcid_info, uint16 pci_dev_id,
                       uint16 pci_ven_id, uint8 pci_rev_id, uint32 pci_mbar0)
{
    /* PCI ID's translated at higher level */

    /* Device ID (16b), Vendor ID (16b) */
    PCIC(pcid_info, 0x00) = (pci_dev_id << 16) | pci_ven_id;

    /* Status (16b), Command (16b) */
    PCIC(pcid_info, 0x04) = 0x04800046;

    /* Class Code (24b), Rev ID (8b) */
    PCIC(pcid_info, 0x08) = 0x02800000 | pci_rev_id;

    /* BIST, Hdr Type, Lat Timer, Cache Line Size */
    PCIC(pcid_info, 0x0c) = 0x00002008;

    /* BAR0 */
    PCIC(pcid_info, 0x10) = pci_mbar0;

    /* BAR1 */
    PCIC(pcid_info, 0x14) = 0x00000000;

    /* BAR2 */
    PCIC(pcid_info, 0x18) = 0x00000000;

    /* BAR3 */
    PCIC(pcid_info, 0x1c) = 0x00000000;

    /* BAR4 */
    PCIC(pcid_info, 0x20) = 0x00000000;

    /* BAR5 */
    PCIC(pcid_info, 0x24) = 0x00000000;

    /* Cardbus CIS Pointer */
    PCIC(pcid_info, 0x28) = 0x00000000;

    /* Subsystem ID (16b), Subsystem Vendor ID (16b) */
    PCIC(pcid_info, 0x2c) = 0x00000000;

    /* Expansion ROM base addr */
    PCIC(pcid_info, 0x30) = 0x00000000;

    /* Reserved (24b), Capabilities Ptr (8b) */
    PCIC(pcid_info, 0x34) = 0x00000000;

    /* Reserved */
    PCIC(pcid_info, 0x38) = 0x00000000;

    /* Max_Lat, Min_Gnt, Int Pin, Int Line */
    PCIC(pcid_info, 0x3c) = 0x0000010b;

    /* Retry_Timeout, TRDY_Timeout */
    PCIC(pcid_info, 0x40) = 0x00008080;

    /* Retry_Timeout, TRDY_Timeout */
    PCIC(pcid_info, 0x44) = 0x00000000;
}

static soc_mem_t
addr_to_mem(int unit, uint32 address)
{
    soc_mem_t       mem;
    uint32          offset, min_addr, max_addr, block;

    offset = address & ~0xC0f00000; /* Strip block ID */
    for (mem = 0; mem < NUM_SOC_MEM; mem++) {
        if (soc_mem_is_valid(unit, mem) &&
            ((SOC_MEM_INFO(unit, mem).blocks | SOC_MEM_INFO(unit, mem).blocks_hi) != 0) ) {
            min_addr = max_addr = SOC_MEM_INFO(unit, mem).base;
            min_addr += SOC_MEM_INFO(unit, mem).index_min;
            max_addr += SOC_MEM_INFO(unit, mem).index_max;
            if (offset >= min_addr && offset <= max_addr) {
		if (SOC_IS_XGS3_SWITCH(unit) || SOC_IS_SIRIUS(unit)) {
                    /* Match block */
                    block = ((address >> SOC_BLOCK_BP) & 0xf) | 
                            (((address >> SOC_BLOCK_MSB_BP) & 0x3) << 4);
                    if (block != SOC_BLOCK2OFFSET(unit, SOC_MEM_BLOCK_ANY(unit, mem))) {
                        continue;
                    }
                }
                return mem;
            }
        }
    }

    return INVALIDm;
}

static soc_mem_t
addr_to_mem_extended(int unit, uint32 block, uint32 address)
{
    soc_mem_t       mem;
    uint32          offset, min_addr, max_addr;

    offset = address;
    for (mem = 0; mem < NUM_SOC_MEM; mem++) {
        if (soc_mem_is_valid(unit, mem) &&
            ((SOC_MEM_INFO(unit, mem).blocks | SOC_MEM_INFO(unit, mem).blocks_hi) != 0) ) {
            min_addr = max_addr = SOC_MEM_INFO(unit, mem).base;
            min_addr += SOC_MEM_INFO(unit, mem).index_min;
            max_addr += SOC_MEM_INFO(unit, mem).index_max;
            if (offset >= min_addr && offset <= max_addr) {
                if (SOC_IS_XGS3_SWITCH(unit) || SOC_IS_SIRIUS(unit)) {
                    /* Match block */
                    if (block != SOC_BLOCK2OFFSET(unit, SOC_MEM_BLOCK_ANY(unit, mem))) {
                        continue;
                    }
                }
                return mem;
            }
        }
    }

    return INVALIDm;
}

int
soc_internal_write_reg(pcid_info_t *pcid_info, uint32 address, uint32 *data)
{
    soc_regaddrinfo_t ainfo;
    int rv;

    soc_regaddrinfo_get(pcid_info->unit, &ainfo, address);

    if (!ainfo.valid || ainfo.reg < 0) {
        /* One hidden register recognized for Draco */
        if (!SOC_IS_DRACO(pcid_info->unit) || address != 0xe00010) {
            debugk(DK_WARN,
                   "Attempt to write to unknown S-channel address 0x%x\n",
                   address);
        }
    } else if (SOC_REG_IS_VALID(pcid_info->unit, ainfo.reg) &&
        SOC_REG_INFO(pcid_info->unit, ainfo.reg).flags &
               SOC_REG_FLAG_RO) {
        if (ainfo.reg != HASH_OUTPUTr) {
            debugk(DK_WARN,
                   "Attempt to write to read-only S-channel address 0x%x\n",
                   address);
        }
    }

#if defined(BCM_FIREBOLT_SUPPORT)
    /*
    ** Map EPIPE_HI/IPIPE_HI port registers to EPIPE/IPIPE block
    */
    if (SOC_IS_FB_FX_HX(pcid_info->unit)) {
        int blk;
        int unit = pcid_info->unit;
        blk = SOC_BLOCK2OFFSET(unit, IPIPE_HI_BLOCK(unit)) << 20;
        if (((address & 0x00f80000) == blk)
            && ((address & 0x0001f000) != (CMIC_PORT(unit) << 12)))    {
            if ((ainfo.reg == ICOS_SELr) ||
                (ainfo.reg == IUNKNOWN_UCAST_BLOCK_MASKr) ||
                (ainfo.reg == IUNKNOWN_MCAST_BLOCK_MASKr) ||
                (ainfo.reg == IBCAST_BLOCK_MASKr) ||
                (ainfo.reg == IEMIRROR_CONTROLr) ||
                (ainfo.reg == IMIRROR_CONTROLr) ||
#if defined(BCM_RAPTOR_SUPPORT) 
                (ainfo.reg == IUNKNOWN_UCAST_BLOCK_MASK_HIr) ||
                (ainfo.reg == IUNKNOWN_MCAST_BLOCK_MASK_HIr) ||
                (ainfo.reg == IBCAST_BLOCK_MASK_HIr) ||
                (ainfo.reg == IEMIRROR_CONTROL_HIr) ||
#endif /* BCM_RAPTOR_SUPPORT */
                0) {
                blk = SOC_BLOCK2OFFSET(unit, IPIPE_BLOCK(unit)) << 20;
                address = (address & ~(0x00f00000)) | blk;
            }
        }
        blk = SOC_BLOCK2OFFSET(unit, EPIPE_HI_BLOCK(unit)) << 20;
        if (((address & 0x00f80000) == blk)
            && ((address & 0x0001f000) != (CMIC_PORT(unit) << 12)))    {
            blk = SOC_BLOCK2OFFSET(unit, EPIPE_BLOCK(unit)) << 20;
            address = (address & ~(0x00f00000)) | blk;
        }
    }
#endif	/* BCM_FIREBOLT_SUPPORT */

#if defined(BCM_RAVEN_SUPPORT)
    if (SOC_IS_RAVEN(pcid_info->unit)) {
        if (ainfo.reg == COMMAND_CONFIGr) {
            uint32 old_data[2];
            if (soc_internal_read_reg(pcid_info, address, old_data) >= 0) {
                if (soc_reg_field_get(pcid_info->unit, COMMAND_CONFIGr,
                                 data[0], SW_RESETf) == 0) {
                    /*
                     * Warn if any field is modified without UNIMAC in reset.
                     */
                    uint32  cc_rst;
                    cc_rst = data[0] ^ old_data[0];
                    soc_reg_field_set(pcid_info->unit, COMMAND_CONFIGr,
                                         &cc_rst, SW_RESETf, 0);
                    if (cc_rst) {
                        debugk(DK_WARN,
                           "UNIMAC: Attempt to Modify COMMAND_CONFIG without MAC in reset old = 0x%08X new = 0x%08X\n", old_data[0], data[0]);
                    }
                }
            }
        }
    }
#endif	/* BCM_RAVEN_SUPPORT */

#if defined(BCM_SIRIUS_SUPPORT)
    if (SOC_IS_SIRIUS(pcid_info->unit)) {
	/* 
	 * if the register has a init bit set, modify the init_done bit to be 1 if exist
	 * similar concept for other bits require hardware response
	 */
	switch (ainfo.reg) {
	    /* various init/init_done pair */
	    case TS_CONFIG0r:
	    case QMA_CONFIG0r:
	    case QMB_CONFIG0r:
	    case QMC_CONFIG0r:
	    case QSA_INITr:
	    case QSB_INITr:
	    case TX_CONFIG0r:
	    case EP_CONFIGr:
	    case FR_CONFIG0r:
	    case EB_CONFIGr:
	    case FD_CONFIGr:
	    case FF_CONFIGr:
	    case ES_CONFIGr:
		if (soc_reg_field_get(pcid_info->unit, ainfo.reg, data[0], INITf) == 1) {
		    soc_reg_field_set(pcid_info->unit, ainfo.reg, data, INIT_DONEf, 1);
		}
		break;
	    case CI_DDR_PHY_REG_CTRLr:
		if (soc_reg_field_get(pcid_info->unit, CI_DDR_PHY_REG_CTRLr, data[0], PHY_REG_REQf) == 1) {
		    soc_reg_field_set(pcid_info->unit, CI_DDR_PHY_REG_CTRLr, data, PHY_REG_ACKf, 1);
		}
		break;
	    case CI_MEM_ACC_CTRLr:
		if (soc_reg_field_get(pcid_info->unit, CI_MEM_ACC_CTRLr, data[0], MEM_ACC_REQf) == 1) {
		    soc_reg_field_set(pcid_info->unit, CI_MEM_ACC_CTRLr, data, MEM_ACC_ACKf, 1);
		}
		break;
	    case CI_DDR_AUTOINITr:
		if (soc_reg_field_get(pcid_info->unit, CI_DDR_AUTOINITr, data[0], STARTf) == 1) {
		    soc_reg_field_set(pcid_info->unit, CI_DDR_AUTOINITr, data, DONEf, 1);
		}
		break;		
	    case QMB_QUEUE_CONFIG_CTRLr:
		if (soc_reg_field_get(pcid_info->unit, QMB_QUEUE_CONFIG_CTRLr, data[0], CQ_REQf) == 1) {
		    soc_reg_field_set(pcid_info->unit, QMB_QUEUE_CONFIG_CTRLr, data, CQ_ACKf, 1);
		}		
		break;
	    case QMB_FL_DEBUG0r:
		if (soc_reg_field_get(pcid_info->unit, QMB_FL_DEBUG0r, data[0], CQ_REQf) == 1) {
		    soc_reg_field_set(pcid_info->unit, QMB_FL_DEBUG0r, data, CQ_ACKf, 1);
		}		
		break;
		/*
		 * read only bits, keep it here in case it's changed
	    case QPP_BP_MONITOR_DEBUGr:
		if ((soc_reg_field_get(pcid_info->unit, QPP_BP_MONITOR_DEBUGr, data[0], DEQ_REQf) == 1) ||
		    (soc_reg_field_get(pcid_info->unit, QPP_BP_MONITOR_DEBUGr, data[0], ENQ_REQf) == 1) ) {
		    soc_reg_field_set(pcid_info->unit, QPP_BP_MONITOR_DEBUGr, data, ENQDEQ_ACKf, 1);
		}
		if (soc_reg_field_get(pcid_info->unit, QPP_BP_MONITOR_DEBUGr, data[0], CUPD_REQf) == 1) {
		    soc_reg_field_set(pcid_info->unit, QPP_BP_MONITOR_DEBUGr, data, CUPD_ACKf, 1);
		}		
		if (soc_reg_field_get(pcid_info->unit, QPP_BP_MONITOR_DEBUGr, data[0], CTX_REQf) == 1) {
		    soc_reg_field_set(pcid_info->unit, QPP_BP_MONITOR_DEBUGr, data, CTX_ACKf, 1);
		}		
		if (soc_reg_field_get(pcid_info->unit, QPP_BP_MONITOR_DEBUGr, data[0], CAL_REQf) == 1) {
		    soc_reg_field_set(pcid_info->unit, QPP_BP_MONITOR_DEBUGr, data, CAL_ACKf, 1);
		}		
		if (soc_reg_field_get(pcid_info->unit, QPP_BP_MONITOR_DEBUGr, data[0], AGER_REQf) == 1) {
		    soc_reg_field_set(pcid_info->unit, QPP_BP_MONITOR_DEBUGr, data, AGER_ACKf, 1);
		}
		if (soc_reg_field_get(pcid_info->unit, QPP_BP_MONITOR_DEBUGr, data[0], SHAPER_REQf) == 1) {
		    soc_reg_field_set(pcid_info->unit, QPP_BP_MONITOR_DEBUGr, data, SHAPER_ACKf, 1);
		}		
		if (soc_reg_field_get(pcid_info->unit, QPP_BP_MONITOR_DEBUGr, data[0], BAA_REQf) == 1) {
		    soc_reg_field_set(pcid_info->unit, QPP_BP_MONITOR_DEBUGr, data, BAA_ACKf, 1);
		}		
		break;
		*/
	    case OTPC_CNTRLr:
		if (soc_reg_field_get(pcid_info->unit, OTPC_CNTRLr, data[0], STARTf) == 1) {
		    /* mark the command_done bit */
		    uint32 old_data[2];
		    uint32 status_address = soc_reg_addr(pcid_info->unit, OTPC_CPU_STATUSr,REG_PORT_ANY, 0);
		    if (soc_internal_read_reg(pcid_info, status_address, old_data) >= 0) {
			soc_reg_field_set(pcid_info->unit, OTPC_CPU_STATUSr, old_data, COMMAND_DONEf, 1);
			soc_internal_write_reg(pcid_info, status_address, old_data);
		    }
		}		
		break;

	    case QMB_CONFIG3r:
		if (soc_reg_field_get(pcid_info->unit, QMB_CONFIG3r, data[0], FREE_LIST_ENABLEf) == 1) {
		    /* mark the command_done bit */
		    uint32 old_data[2];
		    uint32 status_address = soc_reg_addr(pcid_info->unit, QMB_FL_DEBUG0r,REG_PORT_ANY, 0);
		    if (soc_internal_read_reg(pcid_info, status_address, old_data) >= 0) {
			soc_reg_field_set(pcid_info->unit, QMB_FL_DEBUG0r, old_data, HEAD_SHOT_FIFO_LEVELf, 16);
			soc_internal_write_reg(pcid_info, status_address, old_data);
		    }
		}		
		break;

            case CS_MANUAL_EJECT_CTRLr:
                /* Clear EJECT_DONE upon writing 1 to it */
	        if (soc_reg_field_get(pcid_info->unit, CS_MANUAL_EJECT_CTRLr, data[0], EJECT_DONEf) == 1) {
		    soc_reg_field_set(pcid_info->unit, CS_MANUAL_EJECT_CTRLr, data, EJECT_DONEf, 0);
	        }
                /* set EJECT_DONE on GO */
	        if (soc_reg_field_get(pcid_info->unit, CS_MANUAL_EJECT_CTRLr, data[0], GOf) == 1) {
		    soc_reg_field_set(pcid_info->unit, CS_MANUAL_EJECT_CTRLr, data, EJECT_DONEf, 1);
	        }
		break;
	    default:
		break;
	}
    }
#endif  /* BCM_SIRIUS_SUPPORT */

#ifdef BCM_TRIDENT_SUPPORT
    if (soc_feature(pcid_info->unit, soc_feature_logical_port_num)) {
        soc_info_t *si;
        int phy_port;
        int blk;

        si = &SOC_INFO(pcid_info->unit);
        switch (ainfo.reg) {
        case IFP_GM_LOGICAL_TO_PHYSICAL_MAPPINGr:
            phy_port = soc_reg_field_get(pcid_info->unit, ainfo.reg, data[0],
                                         PHYSICAL_PORT_NUMf);
            if (phy_port != 0x7f) {
                si->port_p2l_mapping[phy_port] = ainfo.port;
                si->port_l2p_mapping[ainfo.port] = phy_port;
                blk = SOC_DRIVER(pcid_info->unit)->port_info[phy_port].blk;
                si->port_type[ainfo.port] =
                    SOC_BLOCK_INFO(pcid_info->unit, blk).type;
                if (ainfo.port > si->port.max) {
                    si->port.max = ainfo.port;
                }
            }
            break;
        case MMU_TO_PHY_PORT_MAPPINGr:
            phy_port = soc_reg_field_get(pcid_info->unit, ainfo.reg, data[0],
                                         PHY_PORTf);
            if (phy_port != 0x7f) {
                si->port_p2m_mapping[phy_port] = ainfo.idx & 0x7f;
                si->port_m2p_mapping[ainfo.idx & 0x7f] = phy_port;
            }
            break;
        default:
            break;
        }
    }
#endif /* BCM_TRIDENT_SUPPORT */

    rv = soc_datum_reg_write(pcid_info, ainfo, 0, address, data);

    return rv;
}

int
soc_internal_read_reg(pcid_info_t *pcid_info, uint32 address, uint32 *data)
{
    soc_regaddrinfo_t ainfo;
    int rv;

    soc_regaddrinfo_get(pcid_info->unit, &ainfo, address);

    if (!ainfo.valid || ainfo.reg < 0) {
        debugk(DK_ERR,
               "Attempt to read from unknown S-channel address 0x%x\n",
               address);
    } else if (SOC_REG_IS_VALID(pcid_info->unit, ainfo.reg) &&
        SOC_REG_INFO(pcid_info->unit, ainfo.reg).flags & SOC_REG_FLAG_WO) {
        debugk(DK_WARN,
               "Attempt to read from write-only S-channel address 0x%x\n",
               address);
    }

#ifdef BCM_EASYRIDER_SUPPORT
    /*
    ** 
    */
    if (SOC_IS_EASYRIDER(pcid_info->unit)) {
        int blk;
        int unit = pcid_info->unit;
        uint32 mdata[SOC_MAX_MEM_WORDS];
        blk = SOC_BLOCK2OFFSET(unit, HSE_BLOCK(unit)) << 20;
        if ((address & 0x00f00000) == blk) {
            if (ainfo.reg == CMDWORD_SHADOW_HSEr) {
                soc_internal_read_mem(pcid_info,
                    soc_mem_addr(pcid_info->unit, COMMAND_MEMORY_HSEm,
                        SOC_MEM_BLOCK_ANY(pcid_info->unit,
                                          COMMAND_MEMORY_HSEm), 0),
                                      mdata);

                data[0] = soc_mem_field32_get(pcid_info->unit,
                                              COMMAND_MEMORY_HSEm, mdata,
                                              COMMAND_WORDf);
                return 0;
            }
        }
    }
#endif	/* BCM_EASYRIDER_SUPPORT */
#if defined(BCM_FIREBOLT_SUPPORT)
    /*
    ** Map EPIPE_HI/IPIPE_HI port registers to EPIPE/IPIPE block
    ** Well not really. Some of the registers are Higig only.
    */
    if (SOC_IS_FB_FX_HX(pcid_info->unit)) {
        int blk;
        int unit = pcid_info->unit;
        blk = SOC_BLOCK2OFFSET(unit, IPIPE_HI_BLOCK(unit)) << 20;
        if (((address & 0x00f80000) == blk)
            && ((address & 0x0001f000) != (CMIC_PORT(unit) << 12)))    {
            if ((ainfo.reg == ICOS_SELr) ||
                (ainfo.reg == IUNKNOWN_UCAST_BLOCK_MASKr) ||
                (ainfo.reg == IUNKNOWN_MCAST_BLOCK_MASKr) ||
                (ainfo.reg == IBCAST_BLOCK_MASKr) ||
                (ainfo.reg == IEMIRROR_CONTROLr) ||
                (ainfo.reg == IMIRROR_CONTROLr) ||
#if defined(BCM_RAPTOR_SUPPORT)
                (ainfo.reg == IUNKNOWN_UCAST_BLOCK_MASK_HIr) ||
                (ainfo.reg == IUNKNOWN_MCAST_BLOCK_MASK_HIr) ||
                (ainfo.reg == IBCAST_BLOCK_MASK_HIr) ||
                (ainfo.reg == IEMIRROR_CONTROL_HIr) ||
#endif /* BCM_RAPTOR_SUPPORT */
                0) {
                blk = SOC_BLOCK2OFFSET(unit, IPIPE_BLOCK(unit)) << 20;
                address = (address & ~(0x00f00000)) | blk;
                soc_regaddrinfo_get(pcid_info->unit, &ainfo, address);
            }
        }
        blk = SOC_BLOCK2OFFSET(unit, EPIPE_HI_BLOCK(unit)) << 20;
        if (((address & 0x00f80000) == blk)
            && ((address & 0x0001f000) != (CMIC_PORT(unit) << 12)))    {
            blk = SOC_BLOCK2OFFSET(unit, EPIPE_BLOCK(unit)) << 20;
            address = (address & ~(0x00f00000)) | blk;
            soc_regaddrinfo_get(pcid_info->unit, &ainfo, address);
        }
    }
#endif	/* BCM_FIREBOLT_SUPPORT */
#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_TRX_SUPPORT) \
|| defined(BCM_RAPTOR_SUPPORT)
    /*
    ** Patch over array registers with different reset values
    */
    if (SOC_IS_FIREBOLT2(pcid_info->unit) || SOC_IS_TRX(pcid_info->unit) ||
        SOC_IS_RAVEN(pcid_info->unit) || SOC_IS_HAWKEYE(pcid_info->unit)) {
        if (ainfo.reg == EGR_OUTER_TPIDr) {
            switch (address & 0x3) {
            case 0:
                ainfo.reg = EGR_OUTER_TPID_0r;
                break;
            case 1:
                ainfo.reg = EGR_OUTER_TPID_1r;
                break;
            case 2:
                ainfo.reg = EGR_OUTER_TPID_2r;
                break;
            case 3:
                ainfo.reg = EGR_OUTER_TPID_3r;
                break;
            default:
                break;
            }
            ainfo.idx = -1;
        }
        if (ainfo.reg == ING_OUTER_TPIDr) {
            switch (address & 0x3) {
            case 0:
                ainfo.reg = ING_OUTER_TPID_0r;
                break;
            case 1:
                ainfo.reg = ING_OUTER_TPID_1r;
                break;
            case 2:
                ainfo.reg = ING_OUTER_TPID_2r;
                break;
            case 3:
                ainfo.reg = ING_OUTER_TPID_3r;
                break;
            default:
                break;
            }
            ainfo.idx = -1;
        }
    }
#if defined(BCM_TRIUMPH_SUPPORT)
    if (SOC_IS_TR_VL(pcid_info->unit)) {
        if (ainfo.reg == ING_MPLS_TPIDr) {
            switch (address & 0x3) {
            case 0:
                ainfo.reg = ING_MPLS_TPID_0r;
                break;
            case 1:
                ainfo.reg = ING_MPLS_TPID_1r;
                break;
            case 2:
                ainfo.reg = ING_MPLS_TPID_2r;
                break;
            case 3:
                ainfo.reg = ING_MPLS_TPID_3r;
                break;
            default:
                break;
            }
            ainfo.idx = -1;
        }
    }
#endif /* BCM_TRIUMPH_SUPPORT */
#endif	/* BCM_FIREBOLT2_SUPPORT || BCM_TRX_SUPPORT
           || BCM_RAPTOR_SUPPORT */
    rv = soc_datum_reg_read(pcid_info, ainfo, 0, address, data);

    if (SOC_E_NONE == rv) {
        /* Chip specific register value patches */
#if defined(BCM_FIREBOLT_SUPPORT)
        if (SOC_IS_TRX(pcid_info->unit)) {
            /*
             * Reading ING_HW_RESET_CONTROL_2/EGR_HW_RESET_CONTROL_1
             * returns DONE=1
             */
            if (SOC_IS_TD_TT(pcid_info->unit)) {
                if (address == 0x180003) {	/* ING_HW_RESET_CONTROL_2 */
                    data[0] |= (1 << 18);
                }
                if (address == 0x280001) {	/* EGR_HW_RESET_CONTROL_1 */
                    data[0] |= (1 << 18);
                }
            } else if (SOC_IS_TRIUMPH2(pcid_info->unit) ||
                       SOC_IS_APOLLO(pcid_info->unit)) {
                if (address == 0x080003) {	/* ING_HW_RESET_CONTROL_2 */
                    data[0] |= (1 << 18);
                }
                if (address == 0x180001) {	/* EGR_HW_RESET_CONTROL_1 */
                    data[0] |= (1 << 18);
                }
            } else {
                if (address == 0xa80003) {	/* ING_HW_RESET_CONTROL_2 */
                    data[0] |= (1 << 18);
                }
                if (address == 0xb80001) {	/* EGR_HW_RESET_CONTROL_1 */
                    data[0] |= (1 << 18);
                }
            }
        }
        else if (SOC_IS_FB_FX_HX(pcid_info->unit)) {
            /*
             * Reading ING_HW_RESET_CONTROL_2/EGR_HW_RESET_CONTROL_1
             * returns DONE=1
             */
            if (address == 0x780002) {      /* ING_HW_RESET_CONTROL_2 */
                    data[0] |= (1 << 18);
            }
            if (address == 0x980001) {      /* EGR_HW_RESET_CONTROL_1 */
                    data[0] |= (1 << 18);
            }
        }
        else if (SOC_IS_HBX(pcid_info->unit)) {
            /*
             * Reading ING_HW_RESET_CONTROL_2/EGR_HW_RESET_CONTROL_1
             * returns DONE=1
             */
            if (address == 0x180203) {      /* ING_HW_RESET_CONTROL_2_X */
                    data[0] |= (1 << 18);
            }
            if (address == 0x180403) {      /* ING_HW_RESET_CONTROL_2_Y */
                    data[0] |= (1 << 18);
            }
            if (address == 0x180603) {      /* ING_HW_RESET_CONTROL_2 */
                    data[0] |= (1 << 18);
            }
            if (address == 0x480601) {      /* EGR_HW_RESET_CONTROL_1 */
                    data[0] |= (1 << 18);
            }
        }
#endif  /* BCM_FIREBOLT_SUPPORT */
        return 0;
    } else if (SOC_E_NOT_FOUND == rv) {

    /*
         * The register is not in the hash table, so it has not been written
         * before.  Provide a default value.
         */
        if (SOC_REG_IS_VALID(pcid_info->unit, ainfo.reg) &&
            (ainfo.valid && ainfo.reg >= 0)) {
            data[0] = SOC_REG_INFO(pcid_info->unit, ainfo.reg).rst_val_lo;
            data[1] = SOC_REG_INFO(pcid_info->unit, ainfo.reg).rst_val_hi;
        } else {
            data[0] = 0xffffffff;
            data[1] = 0xffffffff;
        }

        rv = SOC_E_NONE;

#if defined(BCM_HELIX15_SUPPORT) || defined(BCM_FELIX15_SUPPORT)
        if (SOC_IS_FX_HX(pcid_info->unit)) {
            if (address == 0xb80210) {  /* BSAFE_GLB_DEV_STATUS */
                data[0] |= (1 << 7);    /* PROD_CFG_VLD */
            }
            if (address == 0xb8020c) {  /* BSAFE_GLB_PROD_CFG */
                data[0] |= (pcid_info->opt_gbp_wid << 8); /* LMD_ENABLE */
            }
        }
#endif /* BCM_HELIX15_SUPPORT || BCM_FELIX15_SUPPORT */
#ifdef  BCM_XGS_FABRIC_SUPPORT
        if (SOC_IS_HERCULES1(pcid_info->unit) &&
            (address & 0xff0fffff) == 0x00000002) {
            /* Reading MAC_XGXS_STAT returns PLL_LOCK=1 */
            data[0] |= 0x00000008;
        }
        if (SOC_IS_HERCULES15(pcid_info->unit) &&
            (address & 0xff0fffff) == 0x00000202) {
            /* Reading MAC_XGXS_STAT returns PLL_LOCK=1 */
            data[0] |= 0x00000008;
        }
#endif  /* BCM_XGS_FABRIC_SUPPORT */

#ifdef  BCM_DRACO_SUPPORT
        if ((SOC_IS_DRACO(pcid_info->unit) || SOC_IS_TUCANA(pcid_info->unit)) &&
            (address == 0xc00022)) {
            /* Reading MAC_XGXS_STAT returns PLL_LOCK=1 */
            data[0] |= 0x00000008;
        }
#endif  /* BCM_DRACO_SUPPORT */

#ifdef  BCM_LYNX_SUPPORT
        if (SOC_IS_LYNX(pcid_info->unit) &&
            (address == 0x00000202 ||   /* MAC_XGXS_STAT.XE0 */
             address == 0x00100202)) {  /* MAX_XGXS_STAT.HG0 */
            /* Reading MAC_XGXS_STAT returns PLL_LOCK=1 */
            data[0] |= 0x00000008;
        }
#endif  /* BCM_LYNX_SUPPORT */

#if defined(BCM_EASYRIDER_SUPPORT)
        if (SOC_IS_EASYRIDER(pcid_info->unit) &&
            (address == 0x00100002 ||   /* MAC_XGXS_STAT.XE0 */
             address == 0x00200002)) {  /* MAX_XGXS_STAT_XE3 */
            /* Reading MAC_XGXS_STAT returns PLL_LOCK=1 */
            data[0] |= 0x00000008;
        }
#endif  /* BCM_EASYRIDER_SUPPORT */
#if defined(BCM_FIREBOLT_SUPPORT)
        if (SOC_IS_FB(pcid_info->unit) ||
            SOC_IS_HELIX(pcid_info->unit) ||
            SOC_IS_FELIX(pcid_info->unit)) {
            if (address == 0x00200002 ||        /* MAC_XGXS_STAT.XE0 */
                address == 0x00300002 ||        /* MAC_XGXS_STAT.XE1 */
                address == 0x00400002 ||        /* MAC_XGXS_STAT.XE2 */
                address == 0x00500002) {        /* MAX_XGXS_STAT_XE3 */
                /* Reading MAC_XGXS_STAT returns PLL_LOCK=1 */
                data[0] |= 0x00000008;
            }
        }
#endif  /* BCM_FIREBOLT_SUPPORT */
#if defined(BCM_BRADLEY_SUPPORT)
        if (SOC_IS_HBX(pcid_info->unit)) {
            if ((address & 0xfff00fff) == 0x000002) { /* MAX_XGXS_STAT */
                /* Reading MAC_XGXS_STAT returns PLL_LOCK=1 */
                data[0] |= 0x00000020;
            }
        }
#endif  /* BCM_BRADLEY_SUPPORT */
#if defined(BCM_TRIUMPH_SUPPORT)
        if (SOC_IS_TR_VL(pcid_info->unit)) {
            if (address == 0x00200002 || /* MAC_XGXS_STAT.XE0 */
                address == 0x00300002 || /* MAC_XGXS_STAT.XE1 */
                address == 0x00400002 || /* MAC_XGXS_STAT.XE2 */
                address == 0x00500002 || /* MAC_XGXS_STAT.XE3 */
                address == 0x00600002 || /* MAC_XGXS_STAT.XE4 */
                address == 0x00700002 || /* MAC_XGXS_STAT.XE5 */
                address == 0x00800002 || /* MAC_XGXS_STAT.XE6 */
                address == 0x00900002) { /* MAX_XGXS_STAT.XE7 */
                /* Reading MAC_XGXS_STAT returns PLL_LOCK=1 */
                data[0] |= 0x00000020;
            }
        }
#endif  /* BCM_TRIUMPH_SUPPORT */
#if defined(BCM_TRIUMPH2_SUPPORT)
        if (SOC_IS_TRIUMPH2(pcid_info->unit) || SOC_IS_APOLLO(pcid_info->unit) ||
            SOC_IS_VALKYRIE2(pcid_info->unit)) {
            if (address == 0x800002 || /* MAC_XGXS_STAT.XP0 */
                address == 0x900002 || /* MAC_XGXS_STAT.XP1 */
                address == 0xa00002 || /* MAC_XGXS_STAT.XP2 */
                address == 0xb00002 || /* MAC_XGXS_STAT.XP3 */
                address == 0x40000002 || /* MAC_XGXS_STAT.XQ0 */
                address == 0x40100002 || /* MAC_XGXS_STAT.XQ1 */
                address == 0x40200002 || /* MAC_XGXS_STAT.XQ2 */
                address == 0x40300002 || /* MAC_XGXS_STAT.XQ3 */
                address == 0x40400002 || /* MAC_XGXS_STAT.XQ4 */
                address == 0x40500002) { /* MAX_XGXS_STAT.XQ5 */
                /* Reading MAC_XGXS_STAT returns PLL_LOCK=1 */
                data[0] |= 0x00000020;
            }
        }
#endif  /* BCM_TRIUMPH2_SUPPORT */
    } else {
        debugk(DK_ERR, "RPC error: soc_internalreg_read failed %d. \n", rv);
    }

    return rv;
}

int
soc_internal_write_bytes(pcid_info_t *pcid_info, uint32 address, uint8 *data, 
                         int len)
{
    uint32  temp[SOC_MAX_MEM_WORDS];

    if ((len < 0) || (len > (SOC_MAX_MEM_WORDS * (sizeof(uint32))))) {
        return 1;
    }

    /* writing bytes is supported only for bcmsim */
    if ((pcid_info->regmem_cb) && 
        (pcid_info->regmem_cb(pcid_info, BCMSIM_SCHAN_MEM_WRITE, address, 
                              (uint32 *)data, (len * 8)) == 0)) {
        return 0;
    }

    /* if no registered call-back, default to write_mem */
    sal_memset(temp, 0, (SOC_MAX_MEM_WORDS * sizeof(uint32)));
    sal_memcpy(temp, data, len);
    return soc_internal_write_mem(pcid_info, address, temp);

}

int
soc_internal_write_mem(pcid_info_t *pcid_info, uint32 address, uint32 *data)
{
    soc_mem_t       mem;
    int rv;

    if ((pcid_info->regmem_cb) 
        && (pcid_info->regmem_cb(pcid_info, BCMSIM_SCHAN_MEM_WRITE, address, data, 
                                 ((SOC_MAX_MEM_WORDS * 4) * 8)) == 0)) {
        return 0;
    }

    if ((mem = addr_to_mem(pcid_info->unit, address)) != INVALIDm) {
#ifdef	BCM_FIREBOLT_SUPPORT
        if (SOC_IS_FBX(pcid_info->unit)) {
            if (SOC_IS_FB_FX_HX(pcid_info->unit)) {
                /*
                 * Map EPIPE_HI/IPIPE_HI port registers to EPIPE/IPIPE block
                 */
                int blk;
                int unit = pcid_info->unit;
                int cpu_port = CMIC_PORT(unit);
                blk = SOC_BLOCK2OFFSET(unit, IPIPE_HI_BLOCK(unit)) << 20;
                if (((address & 0x00f00000) == blk)
                    && ((address & 0x0000001f) != cpu_port )
                    && (mem != IFP_PORT_FIELD_SELm))    {
                    blk = SOC_BLOCK2OFFSET(unit, IPIPE_BLOCK(unit)) << 20;
                    address = (address & ~(0x00f00000)) | blk;
                }
                blk = SOC_BLOCK2OFFSET(unit, EPIPE_HI_BLOCK(unit)) << 20;
                if (((address & 0x00f00000) == blk)
                    && ((address & 0x0000001f) != cpu_port))    {
                    blk = SOC_BLOCK2OFFSET(unit, EPIPE_BLOCK(unit)) << 20;
                    address = (address & ~(0x00f00000)) | blk;
                }
            }

            switch(mem) {
                case L3_ENTRY_IPV4_UNICASTm:
                case L3_ENTRY_IPV4_MULTICASTm:
                case L3_ENTRY_IPV6_UNICASTm:
                case L3_ENTRY_IPV6_MULTICASTm:
                    if(SOC_IS_HAWKEYE(pcid_info->unit)){
                        break;
                    }
                    return (soc_internal_l3x2_write(pcid_info, mem,
                                                    address, data));
                default: break;
            }
        }
#endif /* BCM_FIREBOLT_SUPPORT */
#ifdef	BCM_EASYRIDER_SUPPORT
        if (SOC_IS_EASYRIDER(pcid_info->unit)) {
            switch(mem) {
            case COMMAND_MEMORY_BSEm:
            case COMMAND_MEMORY_CSEm:
            case COMMAND_MEMORY_HSEm:
                return (soc_internal_mcmd_write(pcid_info, mem, data, TRUE));
                default: break;
            }
        }
#endif /* BCM_EASYRIDER_SUPPORT */
#if defined(BCM_TRX_SUPPORT)
        if (SOC_IS_TRX(pcid_info->unit)) {
            if (mem == VLAN_MACm) {
                /* In real HW, VLAN_MAC table is overlaid on VLAN_XLATE table */
                return (soc_internal_vlan_xlate_entry_write(pcid_info, address,
                                       (vlan_xlate_entry_t *) data));
            }
        }
#endif

        if  (soc_mem_is_aggr(pcid_info->unit, mem)) {
            return soc_internal_write_aggrmem(pcid_info, address, data);
        }
    }

#ifdef BCM_XGS_SWITCH_SUPPORT
    if (SOC_IS_XGS_SWITCH(pcid_info->unit) &&
        !SOC_IS_XGS3_SWITCH(pcid_info->unit)) {
        uint32	arl_block, l3xaddr;

        arl_block = 0x00e00000;
        l3xaddr = 0x05090000;
        if (SOC_IS_LYNX(pcid_info->unit)) {
            arl_block = 0x00300000;
        }
        if (SOC_IS_DRACO15(pcid_info->unit)) {
            l3xaddr = 0x05080000;
        }
        if ((address & 0xffff0000) == (0x01050000 | arl_block)) {
            debugk(DK_WARN, "Attempt to write R/O memory L2X\n");
            return -1;
        }
        if ((address & 0xffff0000) == (l3xaddr | arl_block)) {
            debugk(DK_WARN, "Attempt to write R/O memory L3X\n");
            return -1;
        }
        if (SOC_IS_DRACO15(pcid_info->unit)) {
            if ((address & 0xffff0000) == (0x05090000 | arl_block)) {
                debugk(DK_WARN, "Attempt to write R/O memory DEFIP_ALL\n");
                return -1;
            }
        } else {
            if ((address & 0xffff0000) == (0x050a0000 | arl_block)) {
                debugk(DK_WARN, "Attempt to write R/O memory DEFIP_HI_ALL\n");
                return -1;
            }
            if ((address & 0xffff0000) == (0x050b0000 | arl_block)) {
                debugk(DK_WARN, "Attempt to write R/O memory DEFIP_LO_ALL\n");
                return -1;
            }
        }
        if (address == (0x06000000 | arl_block)) {
            /* This is the hash input memory */
            soc_internal_manual_hash(pcid_info, data);
        }
    }
#endif

    rv = soc_datum_mem_write(pcid_info, 0, address, data);
    return rv;
}

int
soc_internal_read_bytes(pcid_info_t *pcid_info, uint32 address, uint8 *data,
                        int len)
{
    uint32  temp[SOC_MAX_MEM_WORDS];
    int     rv;

    if ((len < 0) || (len > (SOC_MAX_MEM_WORDS * (sizeof(uint32))))) {
        return 1;
    }

    /* reading bytes is supported only for bcmsim */
    if ((pcid_info->regmem_cb) && 
        (pcid_info->regmem_cb(pcid_info, BCMSIM_SCHAN_MEM_READ, address, 
                              (uint32 *) data, (len * 8)) == 0)) {
        return 0;
    }

    /* if no registered call-back, default to read_mem */
    sal_memset(temp, 0, (SOC_MAX_MEM_WORDS * sizeof(uint32)));
    rv = soc_internal_read_mem(pcid_info, address, temp);
    if (!rv) {
        sal_memcpy(data, temp, len);
    }
    return rv;
}
int
soc_internal_read_mem(pcid_info_t *pcid_info, uint32 address, uint32 *data)
{
    int             i, rv;
    soc_mem_t       mem;

    if ((pcid_info->regmem_cb) 
        && (pcid_info->regmem_cb(pcid_info, BCMSIM_SCHAN_MEM_READ, address, data, 
                                 ((SOC_MAX_MEM_WORDS * 4) * 8)) == 0)) {
        return 0;
    }

    if ((mem = addr_to_mem(pcid_info->unit, address)) != INVALIDm) {
#ifdef	BCM_FIREBOLT_SUPPORT
        if (SOC_IS_FBX(pcid_info->unit)) {
            if (SOC_IS_FB_FX_HX(pcid_info->unit)) {
                /*
                 * Map EPIPE_HI/IPIPE_HI port registers to EPIPE/IPIPE block
                 */
                int blk;
                int unit = pcid_info->unit;
                int cpu_port = CMIC_PORT(unit);
                blk = SOC_BLOCK2OFFSET(unit, IPIPE_HI_BLOCK(unit)) << 20;
                if (((address & 0x00f00000) == blk)
                    && ((address & 0x0000001f) != cpu_port )
                    && (mem != IFP_PORT_FIELD_SELm))    {
                    blk = SOC_BLOCK2OFFSET(unit, IPIPE_BLOCK(unit)) << 20;
                    address = (address & ~(0x00f00000)) | blk;
                }
                blk = SOC_BLOCK2OFFSET(unit, EPIPE_HI_BLOCK(unit)) << 20;
                if (((address & 0x00f00000) == blk)
                    && ((address & 0x0000001f) != cpu_port))    {
                    blk = SOC_BLOCK2OFFSET(unit, EPIPE_BLOCK(unit)) << 20;
                    address = (address & ~(0x00f00000)) | blk;
                }
            }

            switch(mem) {
                case L3_ENTRY_IPV4_UNICASTm:
                case L3_ENTRY_IPV4_MULTICASTm:
                case L3_ENTRY_IPV6_UNICASTm:
                case L3_ENTRY_IPV6_MULTICASTm:
                    if(SOC_IS_HAWKEYE(pcid_info->unit)){
                        break;
                    }
                    return (soc_internal_l3x2_read(pcid_info, mem,
                                                   address, data));
                default: break;
            }
        }
#endif /* BCM_FIREBOLT_SUPPORT */
#if defined(BCM_TRX_SUPPORT)
        if (SOC_IS_TRX(pcid_info->unit)) {
            if (mem == VLAN_MACm) {
                /* In real HW, VLAN_MAC table is overlaid on VLAN_XLATE table */
                return (soc_internal_vlan_xlate_entry_read(pcid_info, address,
                                       (vlan_xlate_entry_t *) data));
            }
        }
#endif
        if  (soc_mem_is_aggr(pcid_info->unit, mem)) {
            return soc_internal_read_aggrmem(pcid_info, address, data);
        }
    }

#ifdef	BCM_XGS_SWITCH_SUPPORT
    if (SOC_IS_XGS_SWITCH(pcid_info->unit) &&
        !SOC_IS_XGS3_SWITCH(pcid_info->unit)) {
        uint32	arl_block, l3xaddr;

        arl_block = 0x00e00000;
        l3xaddr = 0x05090000;
        if (SOC_IS_LYNX(pcid_info->unit)) {
            arl_block = 0x00300000;
        }
        if (SOC_IS_DRACO15(pcid_info->unit)) {
            l3xaddr = 0x05080000;
        }
        if ((address & 0xffff0000) == (0x01050000 | arl_block)) {
            return soc_internal_l2x_read(pcid_info, address,
                                         (l2x_entry_t *)data);
        }
        if ((address & 0xffff0000) == (l3xaddr | arl_block)) {
            return soc_internal_l3x_read(pcid_info, address,
                                         (l3x_entry_t *)data);
        }
    }
#endif	/* BCM_XGS_SWITCH_SUPPORT */

    rv = soc_datum_mem_read(pcid_info, 0, address, data);

#ifdef	BCM_TUCANA_SUPPORT
    if (SOC_SUCCESS(rv) && /* i.e., memory is not default null entry */
        SOC_IS_TUCANA(pcid_info->unit) &&
        ((mem = addr_to_mem(pcid_info->unit, address)) != INVALIDm)) {
	/* Simulate Tucana missing the FSEL field */
	if ((address & 0xff0ff000) == 0x07020000) {
	    /*
	     * IRULE.x or GIRULE.x (the FSEL field happens to be in the
	     * same bit positions for each).
	     */
	    soc_mem_field32_set(pcid_info->unit, mem, data, FSELf, 0);
	}
    }
#endif /* BCM_TUCANA_SUPPORT */

    if (SOC_E_NOT_FOUND == rv) {
        /* Return null entry value if recognize, otherwise 0xf's */
        if ((mem = addr_to_mem(pcid_info->unit, address)) != INVALIDm) {
            memcpy(data,		       /* Memory */
                   soc_mem_entry_null(pcid_info->unit, mem),
                   4 * soc_mem_entry_words(pcid_info->unit, mem));
        } else {			       /* Other */
            for (i = 0; i < SOC_MAX_MEM_WORDS; i++) {
                data[i] = 0xffffffff;
            }
        }
    }

    return SOC_E_NONE;
}

/*
 * Re-sort fields of aggregate mems based on bit position
 */
void
_soc_internal_fields_sort(soc_field_info_t *src, uint32 count, soc_field_info_t *dst)
{
    uint32 i, j;
    soc_field_info_t tmp;
    
    sal_memcpy(dst, src, count * sizeof(soc_field_info_t));
    for (i = 0; i < count; i++) {
        for (j = 0; j < count - i - 1; j++) {
            if (dst[j].bp < dst[j+1].bp) {
                sal_memcpy(&tmp, &dst[j], 
                           sizeof(soc_field_info_t));
                sal_memcpy(&dst[j], &dst[j+1],
                           sizeof(soc_field_info_t));
                sal_memcpy(&dst[j+1], &tmp,
                           sizeof(soc_field_info_t));
            }
        }
    }    
}

/*
** This is a generic function to handle aggregate memories. Memories
** made up of unequal depth and field mismatch between unified view
** and individual component memories require special handling.
** For example L2 table has HIT bits which are packet into 1/8 size table
*/
int
soc_internal_read_aggrmem(pcid_info_t *pcid_info, uint32 addr, uint32 *data)
{
    int         unit = pcid_info->unit;
    uint32      ent[SOC_MAX_MEM_WORDS];
    uint32      fv[SOC_MAX_MEM_WORDS];
    soc_mem_t   *m = NULL;
    int         index, i;
    int         offset;
    int         adj_offset = 0;
    int         f;
    int         nfield;
    int         pck_factor;
    soc_mem_t   mem;
    soc_mem_info_t *amemp;
    soc_mem_info_t *memp;
    soc_field_info_t *fieldp;
    soc_field_info_t *asflds; /* array for aggregate mem sorted fields */
    soc_field_info_t *sflds; /* array for partial mem sorted fields */

    mem = addr_to_mem(pcid_info->unit, addr);

    if (mem == INVALIDm)
    {
        return SOC_E_PARAM;
    }

    offset = (addr ^ SOC_MEM_INFO(unit, mem).base) & 0xfffff;
    index = 0;
    memset(data, 0, 4 * soc_mem_entry_words(unit, mem));
    do {
        m = SOC_MEM_AGGR(unit, index);
        if (m[0] == mem) {
            /* Fetch the individual components */
            nfield = 1;
            for(i = 1; m[i] != INVALIDm; i++) {
                /*
                ** Multiple entries compacted into one entry like HIT
                ** requires special handling.
                */
                amemp = &SOC_MEM_INFO(unit, mem);
                if(!soc_mem_index_max(unit, m[i]) || !soc_mem_index_max(unit, mem)) {
                    continue;
                }
                pck_factor = (soc_mem_index_max(unit, mem) /
                              soc_mem_index_max(unit, m[i]));
                assert(pck_factor != 0);
                adj_offset = offset / pck_factor;
                soc_internal_read_mem(pcid_info,
                        soc_mem_addr(unit, m[i],
                                SOC_MEM_BLOCK_ANY(unit, m[i]),
                                adj_offset), ent);

                /* Iterate thru' the fields in this View */
                memp = &SOC_MEM_INFO(unit, m[i]);
                if (pck_factor == 1) {
                    for (f = memp->nFields - 1; f >= 0; f--) {
                        fieldp = &memp->fields[f];
                        soc_mem_field_get(unit, m[i], ent, fieldp->field, fv);
                        if (soc_mem_field_valid(unit, mem, fieldp->field) &&
                            soc_mem_field_valid(unit, m[i], fieldp->field)) {
                            soc_mem_field_set(unit, mem, data, fieldp->field,
                                              fv);
                        }
                        nfield++;
                    }
                } else if (memp->nFields == pck_factor) {
                        asflds = sal_alloc(amemp->nFields * sizeof(soc_field_info_t), 
                                           "aggregate fields");
                        sflds = sal_alloc(memp->nFields * sizeof(soc_field_info_t), 
                                           "mem fields");
                        if ((asflds == NULL) || (sflds == NULL)) {
                            return SOC_E_INTERNAL;
                        }
                        _soc_internal_fields_sort(amemp->fields, amemp->nFields,
                                                  asflds);
                        _soc_internal_fields_sort(memp->fields, memp->nFields,
                                                  sflds);
                        fieldp = &sflds[memp->nFields -
                                    (offset % pck_factor) - 1];
                        soc_mem_field_get(unit, m[i], ent, fieldp->field, fv);
                        fieldp = &asflds[(amemp->nFields - nfield)];
                        soc_mem_field_set(unit, mem, data, fieldp->field, fv);
                        nfield++;
                        sal_free(asflds); sal_free(sflds);
                } else if (memp->nFields == (pck_factor * 2)) {
                        asflds = sal_alloc(amemp->nFields * sizeof(soc_field_info_t), 
                                           "aggregate fields");
                        sflds = sal_alloc(memp->nFields * sizeof(soc_field_info_t), 
                                           "mem fields");
                        if ((asflds == NULL) || (sflds == NULL)) {
                            return SOC_E_INTERNAL;
                        }
                        _soc_internal_fields_sort(amemp->fields, amemp->nFields,
                                                  asflds);
                        _soc_internal_fields_sort(memp->fields, memp->nFields,
                                                  sflds);
                        fieldp = &sflds[memp->nFields -
                                    (offset % pck_factor) - 1];
                        soc_mem_field_get(unit, m[i], ent, fieldp->field, fv);
                        fieldp = &asflds[(amemp->nFields - nfield)];
                        soc_mem_field_set(unit, mem, data, fieldp->field, fv);
                        nfield++;

                        fieldp = &sflds[memp->nFields - pck_factor - 
                                    (offset % pck_factor) - 1];
                        soc_mem_field_get(unit, m[i], ent, fieldp->field, fv);
                        fieldp = &asflds[(amemp->nFields - nfield)];
                        soc_mem_field_set(unit, mem, data, fieldp->field, fv);
                        nfield++;
                        sal_free(asflds); sal_free(sflds);
                } else {
                    debugk(DK_ERR,
                           "soc_internal_read_aggrmem:mem=%d"
                           "Unable to handle Aggregate Mem component %d\n",
                           mem, m[i]);
                }
            }
            return(0);
        }
        index++;
    } while (m[0] != INVALIDm);

    /* Not found in the table of aggregate memories */
    debugk(DK_ERR,
           "soc_internal_read_aggrmem:mem=%d missing in Aggregate Mem List\n",
           mem);
    memcpy(data,		       /* Memory */
           soc_mem_entry_null(unit, mem),
           4 * soc_mem_entry_words(unit, mem));
    return(0);
}

int
soc_internal_write_aggrmem(pcid_info_t *pcid_info, uint32 addr, uint32 *data)
{
    int         unit = pcid_info->unit;
    uint32      ent[SOC_MAX_MEM_WORDS];
    uint32      fv[SOC_MAX_MEM_WORDS];
    soc_mem_t   *m = NULL;
    int         index, i;
    int         offset;
    int         adj_offset = 0;
    int         f;
    int         nfield;
    int         pck_factor;
    soc_mem_t   mem;
    soc_mem_info_t *amemp;
    soc_mem_info_t *memp;
    soc_field_info_t *fieldp;
    soc_field_info_t *asflds; /* array for aggregate mem sorted fields */
    soc_field_info_t *sflds; /* array for partial mem sorted fields */

    mem = addr_to_mem(pcid_info->unit, addr);

    if (mem == INVALIDm)
    {
        return SOC_E_PARAM;
    }

    memset(ent, 0, 4 * soc_mem_entry_words(unit, mem));
    offset = (addr ^ SOC_MEM_INFO(unit, mem).base) & 0xfffff;
    index = 0;
    do {
        m = SOC_MEM_AGGR(unit, index);
        if (m[0] == mem) {
            /* Fetch the individual components */
            nfield = 1;
            for(i = 1; m[i] != INVALIDm; i++) {
                /*
                ** Multiple entries compacted into one entry like HIT
                ** requires special handling.
                */
                amemp = &SOC_MEM_INFO(unit, mem);
                if(!soc_mem_index_max(unit, m[i]) || !soc_mem_index_max(unit, mem)) {
                    continue;
                }
                pck_factor = (soc_mem_index_max(unit, mem) /
                              soc_mem_index_max(unit, m[i]));
                assert(pck_factor != 0);
                adj_offset = offset / pck_factor;
                memp = &SOC_MEM_INFO(unit, m[i]);
                
                /* Iterate thru' the fields in this View */
                if (pck_factor == 1) {
                    for (f = memp->nFields - 1; f >= 0; f--) {
                        fieldp = &memp->fields[f];
                        if (soc_mem_field_valid(unit, mem, fieldp->field) &&
                            soc_mem_field_valid(unit, m[i], fieldp->field)) {
                            soc_mem_field_get(unit, mem, data, fieldp->field,
                                              fv);
                            soc_mem_field_set(unit, m[i], ent, fieldp->field,
                                              fv);
                        }
                        nfield++;
                    }
                } else if (memp->nFields == pck_factor) {
                        asflds = sal_alloc(amemp->nFields * sizeof(soc_field_info_t), 
                                           "aggregate fields");
                        sflds = sal_alloc(memp->nFields * sizeof(soc_field_info_t), 
                                           "mem fields");
                        if ((asflds == NULL) || (sflds == NULL)) {
                            return SOC_E_INTERNAL;
                        }
                        _soc_internal_fields_sort(amemp->fields, amemp->nFields,
                                                  asflds);
                        _soc_internal_fields_sort(memp->fields, memp->nFields,
                                                  sflds);
                        fieldp = &asflds[(amemp->nFields - nfield)];
                        soc_mem_field_get(unit, mem, data, fieldp->field, fv);
                        fieldp = &sflds[memp->nFields -
                                    (offset % pck_factor) - 1];
                        soc_internal_read_mem(pcid_info,
                                soc_mem_addr(unit, m[i],
                                        SOC_MEM_BLOCK_ANY(unit, m[i]),
                                        adj_offset), ent);
                        soc_mem_field_set(unit, m[i], ent, fieldp->field, fv);
                        nfield++;
                        sal_free(asflds); sal_free(sflds);
                } else if (memp->nFields == (pck_factor * 2)) {
                        asflds = sal_alloc(amemp->nFields * sizeof(soc_field_info_t), 
                                           "aggregate fields");
                        sflds = sal_alloc(memp->nFields * sizeof(soc_field_info_t), 
                                           "mem fields");
                        if ((asflds == NULL) || (sflds == NULL)) {
                            return SOC_E_INTERNAL;
                        }
                        _soc_internal_fields_sort(amemp->fields, amemp->nFields,
                                                  asflds);
                        _soc_internal_fields_sort(memp->fields, memp->nFields,
                                                  sflds);
                        fieldp = &asflds[(amemp->nFields - nfield)];
                        soc_mem_field_get(unit, mem, data, fieldp->field, fv);
                        fieldp = &sflds[memp->nFields -
                                    (offset % pck_factor) - 1];
                        soc_internal_read_mem(pcid_info,
                                soc_mem_addr(unit, m[i],
                                        SOC_MEM_BLOCK_ANY(unit, m[i]),
                                        adj_offset), ent);
                        soc_mem_field_set(unit, m[i], ent, fieldp->field, fv);
                        nfield++;

                        fieldp = &asflds[(amemp->nFields - nfield)];
                        soc_mem_field_get(unit, mem, data, fieldp->field, fv);
                        fieldp = &sflds[memp->nFields - pck_factor -
                                    (offset % pck_factor) - 1];
                        soc_mem_field_set(unit, m[i], ent, fieldp->field, fv);
                        nfield++;
                        sal_free(asflds); sal_free(sflds);
                } else {
                    debugk(DK_ERR,
                           "soc_internal_write_aggrmem:mem=%d"
                           "Unable to handle Aggregate Mem component %d\n",
                           mem, m[i]);
                }
                soc_internal_write_mem(pcid_info,
                                soc_mem_addr(unit, m[i],
                                    SOC_MEM_BLOCK_ANY(unit, m[i]),
                                    adj_offset), ent);
            }
            return(0);
        }
        index++;
    } while (m[0] != INVALIDm);

    /* Not found in the table of aggregate memories */
    debugk(DK_ERR,
           "soc_internal_write_aggrmem:mem=%d missing in Aggregate Mem List\n",
           mem);
    return(0);
}

/*
 * Draco VLAN hardware table initialization (clear all valid bits)
 */

int
soc_internal_vlan_tab_init(pcid_info_t *pcid_info)
{
    int index, index_min, index_max;

    debugk(DK_VERBOSE, "VLAN_TAB Init\n");

    index_min = soc_mem_index_min(pcid_info->unit, VLAN_TABm);
    index_max = soc_mem_index_max(pcid_info->unit, VLAN_TABm);

    for (index = index_min; index <= index_max; index++) {
        soc_internal_write_mem(pcid_info,
                               soc_mem_addr(pcid_info->unit, VLAN_TABm, 0,
					    index),
                               soc_mem_entry_null(pcid_info->unit, VLAN_TABm));
    }

    return 0;
}

#ifdef	BCM_FIREBOLT_SUPPORT
void
_soc_l2x2_nak_check(int unit, schan_msg_t    *msg)
{
    int             op_fail_pos;

    /*
     * ==================================================
     * | MFIFO_FULL | OP_FAIL | Index  | L2x entry data |
     * ==================================================
     */
    op_fail_pos = (_shr_popcount(soc_mem_index_max(unit, L2Xm)) +
                   soc_mem_entry_bits(unit, L2Xm)) % 32;

    if (msg->readresp.data[3] & (0x3 << op_fail_pos)) {
        msg->readresp.header.cpu = 1;
    }
}
#endif	/* BCM_FIREBOLT_SUPPORT */

#ifdef BCM_TRX_SUPPORT
static int ing_ipfix_fifo_count[SOC_MAX_NUM_DEVICES] = { 0 };
static int egr_ipfix_fifo_count[SOC_MAX_NUM_DEVICES] = { 0 };
static int ext_l2mod_fifo_count[SOC_MAX_NUM_DEVICES] = { 0 };
static int l2mod_fifo_count[SOC_MAX_NUM_DEVICES]     = { 0 };

/*
 * soc_internal_ing_ipfix_fifo_pop
 */
static int
soc_internal_fifo_pop(pcid_info_t * pcid_info, soc_mem_t mem, uint32 *result)
{
    int      *count, i;
    uint32   tmp[SOC_MAX_MEM_WORDS];
    int      unit = pcid_info->unit;

    debugk(DK_VERBOSE, "Ingress IPFIX fifo pop\n");

    *result = 0;
    switch (mem) {
        case ING_IPFIX_EXPORT_FIFOm:
            count = &ing_ipfix_fifo_count[unit];
            break;
        case EGR_IPFIX_EXPORT_FIFOm:
            count = &egr_ipfix_fifo_count[unit];
            break;
        case EXT_L2_MOD_FIFOm:
            count = &ext_l2mod_fifo_count[unit];
            break;
        case L2_MOD_FIFOm:
            count = &l2mod_fifo_count[unit];
            break;
        default:
            debugk(DK_VERBOSE, "Pop not supported on this memory\n");
/*             PCIM(pcid_info, CMIC_SCHAN_CTRL) |= SC_MSG_NAK_TST; */
            pcid_reg_or_write(pcid_info, CMIC_SCHAN_CTRL, SC_MSG_NAK_TST);
            return 0;
    }

    if (*count > 0) {
        soc_internal_read_mem(pcid_info, soc_mem_addr(unit, mem,
                                         SOC_MEM_BLOCK_ANY(unit, mem),
                                         0), result);

        /* Shift up the remaining entries */
        for (i = 0; i < *count; i++) {
            soc_internal_read_mem(pcid_info, soc_mem_addr(unit, mem,
                                             SOC_MEM_BLOCK_ANY(unit, mem),
                                             i + 1), tmp);
            soc_internal_write_mem(pcid_info, soc_mem_addr(unit, mem,
                                              SOC_MEM_BLOCK_ANY(unit, mem),
                                              i), tmp);
        }
        *count -= 1;
        debugk(DK_VERBOSE, "Fifo entry popped.\n");
    } else {
        debugk(DK_VERBOSE, "Fifo Empty\n");
        return 1;
    }
    return 0;
}

/*
 * soc_internal_ing_ipfix_fifo_push
 */
static int
soc_internal_fifo_push(pcid_info_t * pcid_info, soc_mem_t mem, uint32 *entry)
{
    int       *count, i;
    uint32    tmp[SOC_MAX_MEM_WORDS];
    int       unit = pcid_info->unit;

    debugk(DK_VERBOSE, "Ingress IPFIX fifo push\n");

    switch (mem) {
        case ING_IPFIX_EXPORT_FIFOm:
            count = &ing_ipfix_fifo_count[unit];
            break;
        case EGR_IPFIX_EXPORT_FIFOm:
            count = &egr_ipfix_fifo_count[unit];
            break;
        case EXT_L2_MOD_FIFOm:
            count = &ext_l2mod_fifo_count[unit];
            break;
        case L2_MOD_FIFOm:
            count = &l2mod_fifo_count[unit];
            break;
        default:
            debugk(DK_VERBOSE, "Pop not supported on this memory\n");
/*            PCIM(pcid_info, CMIC_SCHAN_CTRL) |= SC_MSG_NAK_TST; */
            pcid_reg_or_write(pcid_info, CMIC_SCHAN_CTRL, SC_MSG_NAK_TST);
            return 0;
    }

    if (*count == soc_mem_index_count(unit, mem)) {
        debugk(DK_VERBOSE, "Fifo Full\n");
        return 1;
    } else {
        /* Shift down the existing entries */
        for (i = *count; i > 0; i--) {
            soc_internal_read_mem(pcid_info, soc_mem_addr(unit, mem,
                                             SOC_MEM_BLOCK_ANY(unit, mem),
                                             i - 1), tmp);
            soc_internal_write_mem(pcid_info, soc_mem_addr(unit, mem,
                                              SOC_MEM_BLOCK_ANY(unit, mem),
                                              i), tmp);
        }
        soc_internal_write_mem(pcid_info, soc_mem_addr(unit, mem,
                                          SOC_MEM_BLOCK_ANY(unit, mem),
                                          0), entry);
        *count += 1;
        debugk(DK_VERBOSE, "Fifo entry pushed.\n");
    }
    return 0;
}

#endif /* BCM_TRX_SUPPORT */

/*
 * schan_op
 *
 * Look at schan header, perform action(s), and make response.
 */

int
schan_op(pcid_info_t *pcid_info, int unit, schan_msg_t* data)
{
    schan_msg_t    *msg;
    int             rv = -1;
#ifdef	BCM_FIREBOLT_SUPPORT
    uint8           banks = 0;
#endif
#if defined(BCM_TRIUMPH_SUPPORT) || defined(BCM_SCORPION_SUPPORT)
    soc_mem_t mem;
#endif

#ifdef BCM_CMICM_SUPPORT
    if (pcid_info->cmicm >= CMC0) {
        msg = (schan_msg_t *)&PCIM(pcid_info, CMIC_CMCx_SCHAN_MESSAGEn(CMC0, 0));
    } else 
#endif
    {
        msg = (schan_msg_t *)&PCIM(pcid_info, CMIC_SCHAN_MESSAGE(unit, 0));
    }

    if(data) {
        *msg = *data; 
    }

    /* Clear nak bit in schan control */
#ifdef BCM_CMICM_SUPPORT
    if (pcid_info->cmicm >= CMC0) {
/*         PCIM(pcid_info, CMIC_CMC0_SCHAN_CTRL_OFFSET) &= ~SC_CMCx_MSG_NAK; */
       pcid_reg_and_write(pcid_info, CMIC_CMC0_SCHAN_CTRL_OFFSET, SC_CMCx_MSG_NAK);
       
    } else 
#endif
    {
/*        PCIM(pcid_info, CMIC_SCHAN_CTRL) &= ~SC_MSG_NAK_TST; */
        pcid_reg_and_write(pcid_info, CMIC_SCHAN_CTRL, SC_MSG_NAK_TST);
    }

    switch (msg->header.opcode) {
    case WRITE_REGISTER_CMD_MSG:
       if (pcid_info->opt_pli_verbose)
          debugk(DK_VERBOSE,
                 "S-Channel operation: WRITE_REGISTER_CMD 0x%x\n",
                 msg->writecmd.address);
        msg->readresp.header.opcode = WRITE_REGISTER_ACK_MSG;
        rv = soc_internal_extended_write_reg(pcid_info, msg->header.dstblk, 0,
                                             msg->writecmd.address,
                                             msg->writecmd.data); 
        break;
    case READ_REGISTER_CMD_MSG:
       if (pcid_info->opt_pli_verbose)
          debugk(DK_VERBOSE,
                 "S-Channel operation: READ_REGISTER_CMD 0x%x\n",
                 msg->readcmd.address);
        msg->readresp.header.opcode = READ_REGISTER_ACK_MSG;
        rv = soc_internal_extended_read_reg(pcid_info, msg->header.dstblk, 0,
                                            msg->readcmd.address,
                                            msg->readresp.data);
        break;
    case WRITE_MEMORY_CMD_MSG:
       if (pcid_info->opt_pli_verbose)
          debugk(DK_VERBOSE,
                 "S-Channel operation: WRITE_MEMORY_CMD 0x%x\n",
                 msg->writecmd.address);
        /* coverity[overrun-buffer-val] */
        msg->readresp.header.opcode = WRITE_MEMORY_ACK_MSG;
        rv = soc_internal_extended_write_mem(pcid_info, msg->header.dstblk, 0,
                                             msg->writecmd.address,
                                             msg->writecmd.data); 
        break;
    case READ_MEMORY_CMD_MSG:
       if (pcid_info->opt_pli_verbose)
          debugk(DK_VERBOSE,
                 "S-Channel operation: READ_MEMORY_CMD 0x%x\n",
                 msg->readcmd.address);
        /* coverity[overrun-buffer-val] */
        msg->readresp.header.opcode = READ_MEMORY_ACK_MSG;
        rv = soc_internal_extended_read_mem(pcid_info, msg->header.dstblk, 0,
                                            msg->readcmd.address,
                                            msg->readresp.data);
        break;
    case ARL_INSERT_CMD_MSG:
        debugk(DK_VERBOSE, "S-Channel operation: ARL_INSERT_CMD_MSG\n");
#ifdef	BCM_FIREBOLT_SUPPORT
        if (SOC_IS_FBX(pcid_info->unit)) {
            msg->readresp.header.opcode = ARL_INSERT_DONE_MSG;
            banks =  msg->l2x2.header.cos & 0x3;
            /* coverity[overrun-buffer-val] */
            rv = soc_internal_l2x2_entry_ins(pcid_info, banks,
                                      (l2x_entry_t *)msg->l2x2.data,
                                      msg->readresp.data);
            _soc_l2x2_nak_check(pcid_info->unit, msg);
            break;
        }
#endif	/* BCM_FIREBOLT_SUPPORT */
#ifdef	BCM_XGS_SWITCH_SUPPORT
        if (SOC_IS_XGS_SWITCH(pcid_info->unit)) {
            rv = soc_internal_l2x_ins(pcid_info,
                                      (l2x_entry_t *)msg->arlins.data);
            break;
        }
#endif	/* BCM_XGS_SWITCH_SUPPORT */
        if (SOC_IS_XGS_FABRIC(pcid_info->unit)) {
            debugk(DK_WARN, "Bad call for Hercules: ARL_INSERT_CMD_MSG\n");
            break;
        }
        break;
    case ARL_DELETE_CMD_MSG:
        debugk(DK_VERBOSE, "S-Channel operation: ARL_DELETE_CMD_MSG\n");
#ifdef	BCM_FIREBOLT_SUPPORT
        if (SOC_IS_FBX(pcid_info->unit)) {
            msg->readresp.header.opcode = ARL_DELETE_DONE_MSG;
            banks = msg->l2x2.header.cos & 0x3;
            rv = soc_internal_l2x2_entry_del(pcid_info, banks,
                                      (l2x_entry_t *)msg->l2x2.data,
                                      msg->readresp.data);
            _soc_l2x2_nak_check(pcid_info->unit, msg);
            break;
        }
#endif	/* BCM_FIREBOLT_SUPPORT */
#ifdef	BCM_XGS_SWITCH_SUPPORT
        if (SOC_IS_XGS_SWITCH(pcid_info->unit)) {
            rv = soc_internal_l2x_del(pcid_info,
                                      (l2x_entry_t *)msg->arlins.data);
            break;
        }
#endif	/* BCM_XGS_SWITCH_SUPPORT */
        if (SOC_IS_XGS_FABRIC(pcid_info->unit)) {
            debugk(DK_WARN, "Bad call for Hercules: ARL_DELETE_CMD_MSG\n");
            break;
        }
        break;
    case L2_LOOKUP_CMD_MSG:
#ifdef	BCM_FIREBOLT_SUPPORT
        if (SOC_IS_FBX(pcid_info->unit)) {
            msg->readresp.header.opcode = L2_LOOKUP_ACK_MSG;
            banks = msg->l2x2.header.cos & 0x3;
            rv = soc_internal_l2x2_entry_lkup(pcid_info, banks,
                                      (l2x_entry_t *)msg->l2x2.data,
                                      msg->readresp.data);
            _soc_l2x2_nak_check(pcid_info->unit, msg);
            break;
        }
        break;
#endif	/* BCM_FIREBOLT_SUPPORT */
    case ARL_LOOKUP_CMD_MSG:
#ifdef	BCM_XGS_SWITCH_SUPPORT
        if (SOC_IS_XGS_SWITCH(pcid_info->unit) &&
            !SOC_IS_XGS3_SWITCH(pcid_info->unit)) {
            rv = soc_internal_l2x_lkup(pcid_info,
                                       (l2x_entry_t *)msg->arllkup.data,
                                       msg->readresp.data);
            msg->readresp.header.opcode = READ_MEMORY_ACK_MSG;
            break;
        }
#endif	/* BCM_XGS_SWITCH_SUPPORT */
        if (SOC_IS_XGS_FABRIC(pcid_info->unit)) {
            debugk(DK_WARN, "Bad call for Hercules: ARL_LOOKUP_CMD_MSG\n");
            break;
        }
        break;
    case L3_INSERT_CMD_MSG:
        debugk(DK_VERBOSE, "S-Channel operation: L3_INSERT_CMD_MSG\n");
#ifdef	BCM_FIREBOLT_SUPPORT
        if (SOC_IS_FBX(pcid_info->unit)) {
            msg->readresp.header.opcode = L3_INSERT_DONE_MSG;
            banks = msg->l3x2.header.cos & 0x3;
            rv = soc_internal_l3x2_entry_ins(pcid_info, banks,
                          (l3_entry_ipv6_multicast_entry_t *)msg->l3x2.data,
                          msg->readresp.data);
            break;
        }
#endif	/* BCM_FIREBOLT_SUPPORT */
#ifdef	BCM_XGS_SWITCH_SUPPORT
        if (SOC_IS_XGS_SWITCH(pcid_info->unit)) {
            rv = soc_internal_l3x_ins(pcid_info,
                                      (l3x_entry_t *)msg->l3ins.data);
            break;
        }
#endif	/* BCM_XGS_SWITCH_SUPPORT */
        debugk(DK_WARN, "Bad call for Non-Draco: L3_INSERT_CMD_MSG\n");
        rv = -1;
        break;
    case L3_DELETE_CMD_MSG:
        debugk(DK_VERBOSE, "S-Channel operation: L3_DELETE_CMD_MSG\n");
#ifdef	BCM_FIREBOLT_SUPPORT
        if (SOC_IS_FBX(pcid_info->unit)) {
            msg->readresp.header.opcode = L3_DELETE_DONE_MSG;
            banks = msg->l3x2.header.cos & 0x3;
            rv = soc_internal_l3x2_entry_del(pcid_info, banks,
                          (l3_entry_ipv6_multicast_entry_t *)msg->l3x2.data,
                          msg->readresp.data);
            break;
        }
#endif	/* BCM_FIREBOLT_SUPPORT */
#ifdef	BCM_XGS_SWITCH_SUPPORT
        if (SOC_IS_XGS_SWITCH(pcid_info->unit)) {
            rv = soc_internal_l3x_del(pcid_info,
                                      (l3x_entry_t *)msg->l3ins.data);
            break;
        }
#endif	/* BCM_XGS_SWITCH_SUPPORT */
        debugk(DK_WARN, "Bad call for Non-Draco: L3_DELETE_CMD_MSG\n");
        rv = -1;
        break;
    case L3X2_LOOKUP_CMD_MSG:
#ifdef	BCM_FIREBOLT_SUPPORT
        if (SOC_IS_FBX(pcid_info->unit)) {
            msg->readresp.header.opcode = L3X2_LOOKUP_ACK_MSG;
            banks = msg->l3x2.header.cos & 0x3;
            rv = soc_internal_l3x2_entry_lkup(pcid_info, banks,
                          (l3_entry_ipv6_multicast_entry_t *)msg->l3x2.data,
                          msg->readresp.data);
            if (msg->readresp.data[0] == -1) {
                msg->readresp.header.cpu = 1;
            }
            break;
        }
        break;
#endif	/* BCM_FIREBOLT_SUPPORT */

#if defined(BCM_TRX_SUPPORT)
    case TABLE_INSERT_CMD_MSG:
        if (soc_feature(unit, soc_feature_generic_table_ops)) {
            debugk(DK_VERBOSE, "S-Channel operation: TABLE_INSERT_CMD_MSG\n");

            msg->gencmd.header.opcode = TABLE_INSERT_DONE_MSG;
            banks = msg->gencmd.header.cos & 0x3;
            if (SBUS_ADDR_VERSION_2 == pcid_info->sbus_addr_version) {
                mem = addr_to_mem_extended(pcid_info->unit, msg->header.dstblk,
                                           msg->gencmd.address);
            } else {
                mem = addr_to_mem(pcid_info->unit, msg->gencmd.address);
            } 
            if (mem == L2Xm) {
                /* coverity[overrun-buffer-val] */
                rv = soc_internal_l2_tr_entry_ins(pcid_info, banks,
                                          (l2x_entry_t *)msg->gencmd.data,
                                          (uint32 *)&msg->genresp.response);
            } else if ((mem == VLAN_MACm) || (mem == VLAN_XLATEm)) {
                rv = soc_internal_vlan_xlate_entry_ins(pcid_info, banks,
                                          (vlan_xlate_entry_t *)msg->gencmd.data,
                                          (uint32 *)&msg->genresp.response);
            } else if (mem == EGR_VLAN_XLATEm) {
                rv = soc_internal_egr_vlan_xlate_entry_ins(pcid_info, banks,
                                          (egr_vlan_xlate_entry_t *)msg->gencmd.data,
                                          (uint32 *)&msg->genresp.response);
#ifdef BCM_TRIUMPH_SUPPORT
            } else if (mem == MPLS_ENTRYm) {
                rv = soc_internal_mpls_entry_ins(pcid_info, banks,
                                          (mpls_entry_entry_t *)msg->gencmd.data,
                                          (uint32 *)&msg->genresp.response);
#endif
            }
#ifdef BCM_ISM_SUPPORT
              else if (soc_feature(unit, soc_feature_ism_memory)) {
                
                rv = soc_internal_generic_hash_insert(pcid_info, mem, banks,
                                                      msg->gencmd.data,
                                                      (uint32 *)&msg->genresp.response);
            } 
#endif
              else {
                /* coverity[overrun-buffer-val] */
                rv = soc_internal_l3x2_entry_ins(pcid_info, banks,
                              (l3_entry_ipv6_multicast_entry_t *)msg->gencmd.data,
                              (uint32 *)&msg->genresp.response);
            }
#ifdef BCM_ISM_SUPPORT
            if (soc_feature(unit, soc_feature_ism_memory)) {
                if ((msg->genresp_ism.response.type != SCHAN_GEN_RESP_TYPE_INSERTED) &&
                    (msg->genresp_ism.response.type != SCHAN_GEN_RESP_TYPE_REPLACED)) {
                    msg->genresp.header.cpu = 1;
                }
            } else
#endif /* BCM_ISM_SUPPORT */
            {
                if ((msg->genresp.response.type != SCHAN_GEN_RESP_TYPE_INSERTED) &&
                    (msg->genresp.response.type != SCHAN_GEN_RESP_TYPE_REPLACED)) {
                    msg->genresp.header.cpu = 1;
                }
            }
            break;
        }
    case TABLE_DELETE_CMD_MSG:
        if (soc_feature(unit, soc_feature_generic_table_ops)) {
            debugk(DK_VERBOSE, "S-Channel operation: TABLE_DELETE_CMD_MSG\n");

            msg->gencmd.header.opcode = TABLE_DELETE_DONE_MSG;
            banks = msg->gencmd.header.cos & 0x3;
            if (SBUS_ADDR_VERSION_2 == pcid_info->sbus_addr_version) {
                mem = addr_to_mem_extended(pcid_info->unit, msg->header.dstblk,
                                           msg->gencmd.address);
            } else {
                mem = addr_to_mem(pcid_info->unit, msg->gencmd.address);
            } 
            if (mem == L2Xm) {
                rv = soc_internal_l2_tr_entry_del(pcid_info, banks,
                                          (l2x_entry_t *)msg->gencmd.data,
                                          (uint32 *)&msg->genresp.response);
            } else if ((mem == VLAN_MACm) || (mem == VLAN_XLATEm)) {
                rv = soc_internal_vlan_xlate_entry_del(pcid_info, banks,
                                          (vlan_xlate_entry_t *)msg->gencmd.data,
                                          (uint32 *)&msg->genresp.response);
            } else if (mem == EGR_VLAN_XLATEm) {
                rv = soc_internal_egr_vlan_xlate_entry_del(pcid_info, banks,
                                          (egr_vlan_xlate_entry_t *)msg->gencmd.data,
                                          (uint32 *)&msg->genresp.response);
#ifdef BCM_TRIUMPH_SUPPORT
            } else if (mem == MPLS_ENTRYm) {
                rv = soc_internal_mpls_entry_del(pcid_info, banks,
                                          (mpls_entry_entry_t *)msg->gencmd.data,
                                          (uint32 *)&msg->genresp.response);
#endif
            }
#ifdef BCM_ISM_SUPPORT
              else if (soc_feature(unit, soc_feature_ism_memory)) {
                
                rv = soc_internal_generic_hash_delete(pcid_info, mem, banks,
                                                      msg->gencmd.data,
                                                      (uint32 *)&msg->genresp.response);
            } 
#endif 
             else {
                rv = soc_internal_l3x2_entry_del(pcid_info, banks,
                              (l3_entry_ipv6_multicast_entry_t *)msg->gencmd.data,
                              (uint32 *)&msg->genresp.response);
            }
#ifdef BCM_ISM_SUPPORT
            if (soc_feature(unit, soc_feature_ism_memory)) {
                if (msg->genresp_ism.response.type != SCHAN_GEN_RESP_TYPE_DELETED) {
                    msg->genresp_ism.header.cpu = 1;
                }
            } else
#endif /* BCM_ISM_SUPPORT */
            {
                if (msg->genresp.response.type != SCHAN_GEN_RESP_TYPE_DELETED) {
                    msg->genresp.header.cpu = 1;
                }
            }
            break;
        }
    case TABLE_LOOKUP_CMD_MSG:
        if (soc_feature(unit, soc_feature_generic_table_ops)) {
            debugk(DK_VERBOSE, "S-Channel operation: TABLE_LOOKUP_CMD_MSG\n");

            msg->gencmd.header.opcode = TABLE_LOOKUP_DONE_MSG;
            banks = msg->gencmd.header.cos & 0x3;
            if (SBUS_ADDR_VERSION_2 == pcid_info->sbus_addr_version) {
                mem = addr_to_mem_extended(pcid_info->unit, msg->header.dstblk,
                                           msg->gencmd.address);
            } else {
                mem = addr_to_mem(pcid_info->unit, msg->gencmd.address);
            } 
            assert(mem != INVALIDm);

            if (mem == L2Xm) {
                rv = soc_internal_l2_tr_entry_lkup(pcid_info, banks,
                                          (l2x_entry_t *)msg->gencmd.data,
                                          (uint32 *)&msg->genresp.response);
            } else if ((mem == VLAN_MACm) || (mem == VLAN_XLATEm)) {
                rv = soc_internal_vlan_xlate_entry_lkup(pcid_info, banks,
                                          (vlan_xlate_entry_t *)msg->gencmd.data,
                                          (uint32 *)&msg->genresp.response);
            } else if (mem == EGR_VLAN_XLATEm) {
                rv = soc_internal_egr_vlan_xlate_entry_lkup(pcid_info, banks,
                                          (egr_vlan_xlate_entry_t *)msg->gencmd.data,
                                          (uint32 *)&msg->genresp.response);
#ifdef BCM_TRIUMPH_SUPPORT
            } else if (mem == MPLS_ENTRYm) {
                rv = soc_internal_mpls_entry_lkup(pcid_info, banks,
                                          (mpls_entry_entry_t *)msg->gencmd.data,
                                          (uint32 *)&msg->genresp.response);
#endif
            } 
#ifdef BCM_ISM_SUPPORT
              else if (soc_feature(unit, soc_feature_ism_memory)) {
                
                rv = soc_internal_generic_hash_lookup(pcid_info, mem, banks,
                                                      msg->gencmd.data,
                                                      (uint32 *)&msg->genresp.response);
            } 
#endif
              else {
                rv = soc_internal_l3x2_entry_lkup(pcid_info, banks,
                              (l3_entry_ipv6_multicast_entry_t *)msg->gencmd.data,
                              (uint32 *)&msg->genresp.response);
            }
#ifdef BCM_ISM_SUPPORT
            if (soc_feature(unit, soc_feature_ism_memory)) {
                if (msg->genresp_ism.response.type != SCHAN_GEN_RESP_TYPE_FOUND) {
                    msg->genresp_ism.header.cpu = 1;
                }
            } else
#endif /* BCM_ISM_SUPPORT */
            {
                if (msg->genresp.response.type != SCHAN_GEN_RESP_TYPE_FOUND) {
                    msg->genresp.header.cpu = 1;
                }
            }
            break;
        }
    case FIFO_POP_CMD_MSG:
        if (soc_feature(unit, soc_feature_mem_push_pop)) {
            debugk(DK_VERBOSE, "S-Channel operation: FIFO_POP_CMD_MSG\n");

            mem = addr_to_mem(pcid_info->unit, msg->popcmd.address);
            /* coverity[overrun-buffer-val] */
            rv = soc_internal_fifo_pop(pcid_info, mem, msg->popresp.data);
            if (rv == 1) {
                /* Fifo empty */
                msg->popresp.header.cpu = 1;
                rv = 0;
            }
            msg->popresp.header.opcode = FIFO_POP_DONE_MSG;
        }
        break;
    case FIFO_PUSH_CMD_MSG:
        if (soc_feature(unit, soc_feature_mem_push_pop)) {
            debugk(DK_VERBOSE, "S-Channel operation: FIFO_PUSH_CMD_MSG\n");
            
            mem = addr_to_mem(pcid_info->unit, msg->pushcmd.address);
            /* coverity[overrun-buffer-val] */
            rv = soc_internal_fifo_push(pcid_info, mem, msg->pushcmd.data);
            if (rv == 1) {
                /* Fifo full */
                msg->popresp.header.cpu = 1;
                rv = 0;
            }
            msg->pushresp.header.opcode = FIFO_PUSH_DONE_MSG;
        }
        break;
#endif /* BCM_TRX_SUPPORT */
    default:
        debugk(DK_VERBOSE,
               "S-Channel operation: message not handled: %s (%d)\n",
               soc_schan_op_name(msg->header.opcode), msg->header.opcode);
        break;
    }

    if (rv == 0) {
#ifdef BCM_CMICM_SUPPORT
        if (pcid_info->cmicm >= CMC0) {
/*            PCIM(pcid_info, CMIC_CMC0_SCHAN_CTRL_OFFSET) |= SC_CMCx_MSG_DONE;  */
            pcid_reg_or_write(pcid_info, CMIC_CMC0_SCHAN_CTRL_OFFSET, SC_CMCx_MSG_DONE);
        } else 
#endif
        {
/*            PCIM(pcid_info, CMIC_SCHAN_CTRL) |= SC_MSG_DONE_TST; */
            pcid_reg_or_write(pcid_info, CMIC_SCHAN_CTRL, SC_MSG_DONE_TST);
        }
    }

    if (soc_feature(unit, soc_feature_schmsg_alias)) {
        /* Synchronize message buffer (at 0) with aliased memory */
        memcpy(&(PCIM(pcid_info, 0)),
               &(PCIM(pcid_info, PCIM_ALIAS_OFFSET)), PCIM_ALIASED_BYTES);
    }

    if(data) {
        *data = *msg; 
    }

    return rv;
}

/*
 * soc_internal_table_dma
 *
 * Do a table DMA transfer
 */

void
soc_internal_table_dma(pcid_info_t *pcid_info)
{
    uint32		entry[SOC_MAX_MEM_WORDS];
    uint32		table_addr;
    uint32		dma_addr;
    uint32		dma_count;
    uint32		dma_beats;

    table_addr = PCIM(pcid_info, CMIC_TABLE_DMA_START);
    dma_addr = PCIM(pcid_info, CMIC_ARL_DMA_ADDR);
    dma_count = PCIM(pcid_info, CMIC_ARL_DMA_CNT) & 0x1fffffff;
    dma_beats = PCIM(pcid_info, CMIC_ARL_DMA_CNT) >> 29;

    debugk(DK_VERBOSE,
           "Table DMA: schan_addr=0x%x count=0x%x beats=%d dest=0x%x\n",
           table_addr, dma_count, dma_beats, dma_addr);

    while (dma_count-- > 0) {
        soc_internal_read_mem(pcid_info, table_addr, entry);
        table_addr++;

        soc_internal_memory_store(pcid_info, dma_addr,
                                  (uint8 *)entry, dma_beats * 4,
                                  MF_ES_DMA_OTHER);
        dma_addr += dma_beats * 4;
    }

    

    PCIM(pcid_info, CMIC_SCHAN_CTRL) |= SC_ARL_DMA_DONE_TST;
    PCIM(pcid_info, CMIC_IRQ_STAT) |= IRQ_ARL_DMA_CNT0;
    soc_internal_send_int(pcid_info);
}

/*
 * soc_internal_xgs3_table_dma
 *
 * Do a table DMA transfer
 */

#define DMA_BURST_SIZE  256 /* Max number of WORDS per DMA transfer */

#if DMA_BURST_SIZE < SOC_MAX_MEM_WORDS
#error Burst size is too small
#endif

void
soc_internal_xgs3_table_dma(pcid_info_t *pcid_info)
{
    uint32 entry[DMA_BURST_SIZE];
    uint32 table_addr;
    uint32 dma_addr;
    uint32 dma_count;
    uint32 dma_incr;
    uint32 dma_beats;
    int    i, burst_count;
    schan_msg_t    *msg;

    msg = (schan_msg_t *)&PCIM(pcid_info,
                               CMIC_SCHAN_MESSAGE(pcid_info->unit, 0));

    if (!(PCIM(pcid_info, CMIC_TABLE_DMA_CFG) & CMIC_TDMA_CFG_EN)) {
        return;
    }

    table_addr = PCIM(pcid_info, CMIC_TABLE_DMA_SBUS_START_ADDR);
    dma_addr = PCIM(pcid_info, CMIC_TABLE_DMA_PCIMEM_START_ADDR);
    dma_count = PCIM(pcid_info, CMIC_TABLE_DMA_ENTRY_COUNT) & 0xffffff;
    dma_incr = (PCIM(pcid_info, CMIC_TABLE_DMA_ENTRY_COUNT) >> 24) & 0x1f;
    dma_beats = (PCIM(pcid_info, CMIC_TABLE_DMA_CFG) >> 16) & (0x1f);

    debugk(DK_VERBOSE,
           "Table DMA: schan_addr=0x%x count=0x%x inc=%d beats=%d dest=0x%x\n",
           table_addr, dma_count, dma_incr, dma_beats, dma_addr);

    burst_count = DMA_BURST_SIZE / dma_beats;

    while (dma_count > 0) {
        /* Adjust the burst size at the end of the table */
        if (dma_count < burst_count) burst_count = dma_count;
       
        /* Read as many entries as can fit in the burst buffer */
        for (i = 0; i < burst_count; i++) {
            soc_internal_extended_read_bytes
                (pcid_info, msg->header.dstblk, 0, table_addr,
                 (uint8 *) (entry + i * dma_beats),
                 dma_beats * (sizeof(uint32)));
            table_addr += 1 << dma_incr;
        }

        /* convert to CPU endian-ness */
        for (i=0; i < (burst_count * dma_beats); i++) {
            *(entry + i) = soc_internal_endian_swap(pcid_info, *(entry + i), 
                                                    MF_ES_DMA_OTHER);
        }
        
        /* Send all these entries in one shot */
        soc_internal_bytes_store(pcid_info, dma_addr, (uint8 *)entry,
                                 (dma_beats * sizeof(uint32) * burst_count));
        dma_addr += dma_beats * sizeof(uint32) * burst_count;
        dma_count -= burst_count;
    }

    PCIM(pcid_info, CMIC_TABLE_DMA_CFG) |= CMIC_TDMA_CFG_DONE;
    PCIM(pcid_info, CMIC_IRQ_STAT) |= IRQ_TDMA_DONE;
    soc_internal_send_int(pcid_info);
}

#ifdef BCM_CMICM_SUPPORT
void
soc_internal_xgs3_cmicm_table_dma(pcid_info_t *pcid_info)
{
    uint32      entry[DMA_BURST_SIZE];
    uint32      table_addr;
    uint32      dma_addr;
    uint32      dma_count;
    uint32      dma_incr;
    uint32      dma_beats;
    int         i, burst_count;
    soc_block_t block;

    if (!(PCIM(pcid_info, CMIC_CMC0_TABLE_DMA_CFG_OFFSET) & CMIC_TDMA_CFG_EN)) {
        return;
    }

    table_addr = PCIM(pcid_info, CMIC_CMC0_TABLE_DMA_SBUS_START_ADDR_OFFSET);
    dma_addr = PCIM(pcid_info, CMIC_CMC0_TABLE_DMA_PCIMEM_START_ADDR_OFFSET);
    dma_count = PCIM(pcid_info, CMIC_CMC0_TABLE_DMA_ENTRY_COUNT_OFFSET) & 0xffffff;
    dma_incr = (PCIM(pcid_info, CMIC_CMC0_TABLE_DMA_ENTRY_COUNT_OFFSET) >> 24) & 0x1f;
    dma_beats = (PCIM(pcid_info, CMIC_CMC0_TABLE_DMA_CFG_OFFSET) >> 11) & (0x1f);
    
    block = (PCIM(pcid_info, CMIC_CMC0_TABLE_DMA_SBUS_CMD_CONFIG_OFFSET) >> 3) & 0x3f;

    debugk(DK_VERBOSE,
           "Table DMA: schan_addr=0x%x block: %d count=0x%x inc=%d beats=%d dest=0x%x\n",
           table_addr, block, dma_count, dma_incr, dma_beats, dma_addr);

    burst_count = DMA_BURST_SIZE / dma_beats;

    while (dma_count > 0) {
        /* Adjust the burst size at the end of the table */
        if (dma_count < burst_count) burst_count = dma_count;
       
        /* Read as many entries as can fit in the burst buffer */
        for (i = 0; i < burst_count; i++) {
            soc_internal_extended_read_bytes(pcid_info, block, 0, table_addr,
                                             (uint8 *) (entry + i * dma_beats),
                                             dma_beats * (sizeof(uint32)));
            table_addr += 1 << dma_incr;
        }

        /* convert to CPU endian-ness */
        for (i=0; i < (burst_count * dma_beats); i++) {
            *(entry + i) = soc_internal_endian_swap(pcid_info, *(entry + i), 
                                                    MF_ES_DMA_OTHER);
        }
        
        /* Send all these entries in one shot */
        soc_internal_bytes_store(pcid_info, dma_addr, (uint8 *)entry,
                                 (dma_beats * sizeof(uint32) * burst_count));
        dma_addr += dma_beats * sizeof(uint32) * burst_count;
        dma_count -= burst_count;
    }

    PCIM(pcid_info, CMIC_CMC0_TABLE_DMA_STAT_OFFSET) |= TDMA_DONE;
    PCIM(pcid_info, CMIC_CMC0_IRQ_STAT0_OFFSET) |= IRQ_CMCx_TDMA_DONE;
    soc_internal_cmicm_send_int(pcid_info, CMIC_CMC0_PCIE_IRQ_MASK0_OFFSET);
}
#endif

/*
 * soc_internal_xgs3_tslam_dma
 *
 * Do a table SLAM DMA transfer
 */

void
soc_internal_xgs3_tslam_dma(pcid_info_t *pcid_info)
{
    uint32		entry[DMA_BURST_SIZE];
    uint32		table_addr;
    uint32		dma_addr;
    uint32		dma_count;
    uint32      dma_incr;
    uint32		dma_beats;
    int         direction;
    int         i, burst_count, entry_idx;
    schan_msg_t    *msg;

    msg = (schan_msg_t *)&PCIM(pcid_info,
                               CMIC_SCHAN_MESSAGE(pcid_info->unit, 0));

    if (!(PCIM(pcid_info, CMIC_SLAM_DMA_CFG) & CMIC_SLAM_CFG_EN)) {
        return;
    }
    
    table_addr = PCIM(pcid_info, CMIC_SLAM_DMA_SBUS_START_ADDR);
    dma_addr = PCIM(pcid_info, CMIC_SLAM_DMA_PCIMEM_START_ADDR);
    dma_count = PCIM(pcid_info, CMIC_SLAM_DMA_ENTRY_COUNT) & 0xffffff;
    dma_incr = (PCIM(pcid_info, CMIC_SLAM_DMA_ENTRY_COUNT) >> 24) & 0x1f;
    dma_beats = (PCIM(pcid_info, CMIC_SLAM_DMA_CFG) >> 16) & (0x1f);
    direction = (PCIM(pcid_info, CMIC_SLAM_DMA_CFG) & CMIC_SLAM_CFG_DIR);

    if (direction) {
        /* Last entry */
        dma_addr = dma_addr + dma_count * 4 * dma_beats;
        table_addr = table_addr + (dma_count - 1) * (1 << dma_incr);
    }

    debugk(DK_VERBOSE,
           "Table DMA: schan_addr=0x%x count=0x%x inc=%d beats=%d dest=0x%x\n",
           table_addr, dma_count, dma_incr, dma_beats, dma_addr);

    burst_count = DMA_BURST_SIZE / dma_beats;

    while (dma_count > 0) {
        /* Adjust the burst size at the end of the table */
        if (dma_count < burst_count) burst_count = dma_count;

        /* Get as many entries as can fit in the burst buffer */
        soc_internal_bytes_fetch(pcid_info, 
                                 (direction ? 
                                   (dma_addr - burst_count * dma_beats * 4) : 
                                   dma_addr),
                                 (uint8 *)entry,
                                 dma_beats * 4 * burst_count);
        
        /* convert to chip (same as host for sim) endian-ness */
        for (i=0; i < (burst_count * dma_beats); i++) {
            *(entry + i) = soc_internal_endian_swap(pcid_info, *(entry + i),
                                                    MF_ES_DMA_OTHER);
        }

        /* Write all the entries we've got */
        for (i = 0; i < burst_count; i++) {
            entry_idx = direction ? burst_count - i - 1 : i;
            soc_internal_extended_write_bytes
                (pcid_info, msg->header.dstblk, 0, table_addr,
                 (uint8 *)(entry + entry_idx * dma_beats), dma_beats * 4);
            table_addr += direction ? -(1 << dma_incr) : 1 << dma_incr;
        }

        dma_addr += (direction ? -(dma_beats * 4 * burst_count) : 
                                 (dma_beats * 4 * burst_count));
        dma_count -= burst_count;
    }

    PCIM(pcid_info, CMIC_SLAM_DMA_CFG) |= CMIC_SLAM_CFG_DONE;
    PCIM(pcid_info, CMIC_IRQ_STAT) |= IRQ_TSLAM_DONE;
    soc_internal_send_int(pcid_info);
}

#ifdef BCM_CMICM_SUPPORT
void
soc_internal_xgs3_cmicm_tslam_dma(pcid_info_t *pcid_info)
{
    uint32      entry[DMA_BURST_SIZE];
    uint32      table_addr;
    uint32      dma_addr;
    uint32      dma_count;
    uint32      dma_incr;
    uint32      dma_beats;
    int         direction;
    int         i, burst_count, entry_idx;
    soc_block_t block;

    if (!(PCIM(pcid_info, CMIC_CMC0_SLAM_DMA_CFG_OFFSET) & CMIC_SLAM_CFG_EN)) {
        return;
    }
    
    table_addr = PCIM(pcid_info, CMIC_CMC0_SLAM_DMA_SBUS_START_ADDR_OFFSET);
    dma_addr = PCIM(pcid_info, CMIC_CMC0_SLAM_DMA_PCIMEM_START_ADDR_OFFSET);
    dma_count = PCIM(pcid_info, CMIC_CMC0_SLAM_DMA_ENTRY_COUNT_OFFSET) & 0xffffff;
    dma_incr = (PCIM(pcid_info, CMIC_CMC0_SLAM_DMA_ENTRY_COUNT_OFFSET) >> 24) & 0x1f;
    dma_beats = (PCIM(pcid_info, CMIC_CMC0_SLAM_DMA_CFG_OFFSET) >> 11) & (0x1f);
    direction = (PCIM(pcid_info, CMIC_CMC0_SLAM_DMA_CFG_OFFSET) & SLDMA_CFG_DIR);

    block = (PCIM(pcid_info, CMIC_CMC0_SLAM_DMA_SBUS_CMD_CONFIG_OFFSET) >> 3) & 0x3f;

    if (direction) {
        /* Last entry */
        dma_addr = dma_addr + dma_count * 4 * dma_beats;
        table_addr = table_addr + (dma_count - 1) * (1 << dma_incr);
    }

    debugk(DK_VERBOSE,
           "Table DMA: schan_addr=0x%x block=%d count=0x%x inc=%d beats=%d dest=0x%x\n",
           table_addr, block, dma_count, dma_incr, dma_beats, dma_addr);

    burst_count = DMA_BURST_SIZE / dma_beats;

    while (dma_count > 0) {
        /* Adjust the burst size at the end of the table */
        if (dma_count < burst_count) burst_count = dma_count;

        /* Get as many entries as can fit in the burst buffer */
        soc_internal_bytes_fetch(pcid_info, 
                                 (direction ? 
                                   (dma_addr - burst_count * dma_beats * 4) : 
                                   dma_addr),
                                 (uint8 *)entry,
                                 dma_beats * 4 * burst_count);
        
        /* convert to chip (same as host for sim) endian-ness */
        for (i=0; i < (burst_count * dma_beats); i++) {
            *(entry + i) = soc_internal_endian_swap(pcid_info, *(entry + i),
                                                    MF_ES_DMA_OTHER);
        }

        /* Write all the entries we've got */
        for (i = 0; i < burst_count; i++) {
            entry_idx = direction ? burst_count - i - 1 : i;
            soc_internal_extended_write_bytes(pcid_info, block, 0, table_addr, 
                                     (uint8 *)(entry + entry_idx * dma_beats),
                                     dma_beats * 4);
            table_addr += direction ? -(1 << dma_incr) : 1 << dma_incr;
        }

        dma_addr += (direction ? -(dma_beats * 4 * burst_count) : 
                                 (dma_beats * 4 * burst_count));
        dma_count -= burst_count;
    }

    PCIM(pcid_info, CMIC_CMC0_SLAM_DMA_STAT_OFFSET) |= SLDMA_DONE;
    PCIM(pcid_info, CMIC_CMC0_IRQ_STAT0_OFFSET) |= IRQ_CMCx_TSLAM_DONE;
    soc_internal_cmicm_send_int(pcid_info, CMIC_CMC0_PCIE_IRQ_MASK0_OFFSET);
}
#endif

void
soc_internal_schan_ctrl_write(pcid_info_t *pcid_info, uint32 value)
{
    if (value & 0x80) {
        debugk(DK_VERBOSE, "Set SCHAN_CTRL bit %d\n", value & 0x1f);
        switch (value & 0xff) {
        case SC_MSG_START_SET:
            if ((pcid_info->schan_cb) && 
                (pcid_info->schan_cb(pcid_info, pcid_info->unit, NULL) == 0)) {
                PCIM(pcid_info, CMIC_IRQ_STAT) |= IRQ_SCH_MSG_DONE;
                soc_internal_send_int(pcid_info);
                break;
            } else {
                if (schan_op(pcid_info, pcid_info->unit, NULL) == 0) {
                    PCIM(pcid_info, CMIC_IRQ_STAT) |= IRQ_SCH_MSG_DONE;
                    soc_internal_send_int(pcid_info);
                }
            }
            break;
        case SC_MIIM_RD_START_SET:
            soc_internal_miim_op(pcid_info, 1);
            PCIM(pcid_info, CMIC_IRQ_STAT) |= IRQ_MIIM_OP_DONE;
            soc_internal_send_int(pcid_info);
            break;
        case SC_MIIM_WR_START_SET:
            soc_internal_miim_op(pcid_info, 0);
            PCIM(pcid_info, CMIC_IRQ_STAT) |= IRQ_MIIM_OP_DONE;
            soc_internal_send_int(pcid_info);
            break;
        case SC_ARL_DMA_EN_SET:
            if (SOC_IS_XGS_SWITCH(pcid_info->unit)) {
                soc_internal_table_dma(pcid_info);
                PCIM(pcid_info, CMIC_IRQ_STAT) |= IRQ_ARL_DMA_CNT0;
                soc_internal_send_int(pcid_info);
            } else {
                PCIM(pcid_info, CMIC_SCHAN_CTRL) |= 1 << (value & 0x1f);
            }
            break;
        default:
            PCIM(pcid_info, CMIC_SCHAN_CTRL) |= 1 << (value & 0x1f);
            break;
        }
    } else {
        debugk(DK_VERBOSE, "Clear SCHAN_CTRL bit %d\n", value & 0x1f);
        PCIM(pcid_info, CMIC_SCHAN_CTRL) &= ~(1 << (value & 0x1f));
        switch (value & 0xff) {
        case SC_MSG_DONE_CLR:
            PCIM(pcid_info, CMIC_IRQ_STAT) &= ~IRQ_SCH_MSG_DONE;
            break;
        case SC_MIIM_OP_DONE_CLR:
            PCIM(pcid_info, CMIC_IRQ_STAT) &= ~IRQ_MIIM_OP_DONE;
            break;
        case SC_ARL_DMA_EN_CLR:
            PCIM(pcid_info, CMIC_IRQ_STAT) &= ~IRQ_ARL_DMA_CNT0;
            break;
        default:
            break;
        }
    }
}

#ifdef BCM_CMICM_SUPPORT
void
soc_internal_cmicm_schan_ctrl_write(pcid_info_t *pcid_info, uint32 reg, uint32 value)
{
    if (value & SC_CMCx_MSG_START) {
        debugk(DK_VERBOSE, "SCHAN_CTRL msg start.\n");
        if ((pcid_info->schan_cb) && 
            (pcid_info->schan_cb(pcid_info, pcid_info->unit, NULL) == 0)) {
            PCIM(pcid_info, CMIC_CMC0_IRQ_STAT0_OFFSET) |= IRQ_CMCx_SCH_OP_DONE;
            soc_internal_cmicm_send_int(pcid_info, reg);
        } else {
            if (schan_op(pcid_info, pcid_info->unit, NULL) == 0) {
                PCIM(pcid_info, CMIC_CMC0_SCHAN_CTRL_OFFSET) &= 
                    ~(SC_CMCx_MSG_SER_CHECK_FAIL | SC_CMCx_MSG_NAK | SC_CMCx_MSG_TIMEOUT_TST);
                PCIM(pcid_info, CMIC_CMC0_IRQ_STAT0_OFFSET) |= IRQ_CMCx_SCH_OP_DONE;
                soc_internal_cmicm_send_int(pcid_info, reg);
            }
        }
    } else {
        debugk(DK_VERBOSE, "SCHAN_CTRL msg done.\n");
        PCIM(pcid_info, CMIC_CMC0_SCHAN_CTRL_OFFSET) &= 
                ~(SC_CMCx_MSG_DONE | SC_CMCx_MSG_SER_CHECK_FAIL | 
                SC_CMCx_MSG_NAK | SC_CMCx_MSG_TIMEOUT_TST);
    }
}
#endif

void
soc_internal_reset(pcid_info_t *pcid_info)
{
    int index;
    soc_datum_t *d;

    memset(&PCIM(pcid_info, 0), 0, PCIM_SIZE(0));

#ifdef BCM_CMICM_SUPPORT
    if (soc_feature(pcid_info->unit, soc_feature_cmicm)) {
        pcid_info->cmicm = CMC0;
        /* ToDo: Get CMC Number from config var spn_PCI_CMC */
    } else
#endif
    {
        pcid_info->cmicm = -1;
        PCIM(pcid_info, CMIC_SCHAN_CTRL) = 0x48002;
    }
    /*
     * Delete all register values from hash table, which effectively
     * returns them to their reset default values.
     */
    for (index = 0; index < SOC_DATUM_HASH_SIZE; index++) {
        while ((d = pcid_info->reg_ht[index]) != NULL) {
            pcid_info->reg_ht[index] = d->next;
            sal_free(d);
        }
    }

    /*
     * Delete all memory entries from hash table, which effectively
     * returns them to their null entry values.
     */
    for (index = 0; index < SOC_DATUM_HASH_SIZE; index++) {
        while ((d = pcid_info->mem_ht[index]) != NULL) {
            pcid_info->mem_ht[index] = d->next;
            sal_free(d);
        }
    }

#ifdef	BCM_XGS_SWITCH_SUPPORT
    if (SOC_IS_DRACO(pcid_info->unit) || SOC_IS_LYNX(pcid_info->unit)) {
        soc_internal_l2x_init(pcid_info);
        soc_internal_l3x_init(pcid_info);
        soc_internal_vlan_tab_init(pcid_info);
    }
    if (SOC_IS_TUCANA(pcid_info->unit)) {
        soc_internal_l2x_init(pcid_info);
        soc_internal_l3x_init(pcid_info);
    }
#endif	/* BCM_XGS_SWITCH_SUPPORT */
#ifdef	BCM_XGS_FABRIC_SUPPORT
    if (SOC_IS_XGS_FABRIC(pcid_info->unit)) {
    }
#endif	/* BCM_XGS_FABRIC_SUPPORT */

    /*
     * Invoke reset callback if installed.
     */
    if (pcid_info->reset_cb) {
        pcid_info->reset_cb(pcid_info, pcid_info->unit);
    }
}

uint32
soc_internal_endian_swap(pcid_info_t *pcid_info, uint32 data, int mode)
{
    if (((mode == MF_ES_DMA_PACKET) &&
         (PCIM(pcid_info, CMIC_ENDIAN_SELECT) & ES_BIG_ENDIAN_DMA_PACKET))
        ||
        ((mode == MF_ES_DMA_OTHER) &&
         (PCIM(pcid_info, CMIC_ENDIAN_SELECT) & ES_BIG_ENDIAN_DMA_OTHER))
        ||
        ((mode == MF_ES_PIO) &&
         (PCIM(pcid_info, CMIC_ENDIAN_SELECT) & ES_BIG_ENDIAN_PIO))) {
        data = (data << 16) | (data >> 16);
        return (data & 0xff00ffff) >> 8 | (data & 0xffff00ff) << 8;
    } else {
        return(data);
    }
}

void
soc_internal_memory_fetch(pcid_info_t *pcid_info, uint32 addr, uint8 *b,
                          int length, int mode)
{
    uint32	    offset;

    if (pcid_info->opt_rpc_error) {
        return;
    }

    debugk(DK_VERBOSE, "soc_internal_memory_fetch: addr=0x%08x length=%d\n",
           addr, length);

    /*
     * For memory fetch, any alignment is allowed, but 4-byte accesses
     * are always made. This is slow but ....
     */
    offset = addr & 3;

    addr &= ~3;			/* Only read 4-bytes */

    while (length) {
        uint32	data;
        int	tl;
        int rv;

        rv = dma_readmem(pcid_info->client->dmasock, addr, &data);
        if (rv != RPC_OK) {
            debugk(DK_ERR, "RPC error: soc_internal_memory_fetch failed. \n");
            pcid_info->opt_rpc_error = 1;
            break;
        }

        data =
            soc_internal_endian_swap(pcid_info,
                                     data,
                                     mode);
        tl = (sizeof(uint32) - offset);
        if (length < tl) {
            tl = length;
        }

        sal_memcpy(b, ((uint8 *)&data) + offset, tl);
        length -= tl;
        addr += 4;
        b += 4 - offset;
        offset = 0;
    }
}

/* Get a string of bytes from memory addr given length; store in buf. */
void
soc_internal_bytes_fetch(pcid_info_t *pcid_info, uint32 addr, uint8 *buf,
                          int length)
{
    int read_len;

    if (pcid_info->opt_rpc_error) {
        return;
    }

    debugk(DK_VERBOSE, "soc_internal_bytes_fetch: addr=0x%08x length=%d\n",
           addr, length);

    read_len = dma_read_bytes(pcid_info->client->dmasock, addr, buf, length);

    if (read_len != length) {
        debugk(DK_ERR,
               "soc_internal_bytes_fetch: length mismatch.  want %d. got %d\n",
               length, read_len);
    }

    if (read_len < 0) {
        debugk(DK_ERR, "RPC error: soc_internal_bytes_fetch failed. \n");
        pcid_info->opt_rpc_error = 1;
    }
}

/* Get a string of bytes from memory addr given length; store in buf. */
void
soc_internal_bytes_store(pcid_info_t *pcid_info, uint32 addr, uint8 *buf,
                         int length)
{
    int rv;

    if (pcid_info->opt_rpc_error) {
        return;
    }

    debugk(DK_VERBOSE, "soc_internal_bytes_store: addr=0x%08x length=%d\n",
           addr, length);

    rv = dma_write_bytes(pcid_info->client->dmasock, addr, buf, length);
    if (rv < 0) {
        debugk(DK_ERR, "RPC error: soc_internal_bytes_store failed. \n");
        pcid_info->opt_rpc_error = 1;
    }
}

void
soc_internal_memory_store(pcid_info_t *pcid_info, uint32 addr, uint8 *b,
                          int length, int mode)
{
    uint32          data;
    int             rv;

    if (pcid_info->opt_rpc_error) {
        return;
    }

    debugk(DK_VERBOSE, "soc_internal_memory_store: addr=0x%08x length=%d\n",
           addr, length);

    /*
     * Strata (and friends) only support 4-byte aligned writes
     */

   addr &= ~3;

    while (length > 0) {
        sal_memcpy(&data, b, length < sizeof(data) ? length : sizeof(data));
        rv = dma_writemem(pcid_info->client->dmasock,
                     addr, soc_internal_endian_swap(pcid_info, data, mode));
        if (rv != RPC_OK) {
            debugk(DK_ERR, "RPC error: soc_internal_memory_store failed. \n");
            pcid_info->opt_rpc_error = 1;
            break;
        }
        b += sizeof(uint32);
        addr += sizeof(uint32);
        length -= sizeof(uint32);
    }
}

#ifdef BCM_XGS_SWITCH_SUPPORT
int
soc_internal_manual_hash(pcid_info_t *pcid_info, uint32 *data)
{
    /* First try at endianness */
    uint32          tmp_hs[SOC_MAX_MEM_WORDS];
    int             hash_type;
    int             hashed_result = 0;
    int             hash_sel;
    uint8           key[XGS_HASH_KEY_SIZE];

    soc_internal_read_reg(pcid_info,
                          soc_reg_addr(pcid_info->unit, HASH_CONTROLr,
				       REG_PORT_ANY, 0),
                          tmp_hs);
    hash_sel = soc_reg_field_get(pcid_info->unit, HASH_CONTROLr,
                                 tmp_hs[0], HASH_SELECTf);

    hash_type = soc_mem_field32_get(pcid_info->unit, HASHINPUTm,
                                  data, KEY_TYPEf);

    if (hash_sel >= XGS_HASH_COUNT) {
        debugk(DK_VERBOSE, "invalid hash selection %d", hash_sel);
        return -1;
    }

    switch (hash_type) {
    case XGS_HASH_KEY_TYPE_L2:		       /* L2 table */
    {
        /* I can't imagine the loop arithmetic for this would suck worse */
        key[0] = (data[0] << 4) & 0xf0;
        key[1] = (data[0] >> 4) & 0xff;
        key[2] = (data[0] >> 12) & 0xff;
        key[3] = (data[0] >> 20) & 0xff;
        key[4] = ((data[1] << 4) & 0xf0) | ((data[0] >> 28) & 0x0f);
        key[5] = (data[1] >> 4) & 0xff;
        key[6] = (data[1] >> 12) & 0xff;
        key[7] = (data[1] >> 20) & 0xff;

        hashed_result = soc_draco_l2_hash(pcid_info->unit, hash_sel, key);
        break;
    }

    case XGS_HASH_KEY_TYPE_L3UC:	       /* L3 table, 32-bit */
    {
        int             ix;
        for (ix = 0; ix < 4; ix++) {
            key[ix] = (data[0] >> (8 * ix)) & 0xff;
        }

        hashed_result = soc_draco_l3_hash(pcid_info->unit, hash_sel, 0, key);
        break;
    }

    case XGS_HASH_KEY_TYPE_L3MC:	       /* L3 table, 64-bit */
    {
        int             ix, key_src_ip;

        if (SOC_IS_DRACO1(pcid_info->unit)) {
            soc_internal_read_reg(pcid_info,
                                  soc_reg_addr(pcid_info->unit, CONFIGr,
					       REG_PORT_ANY, 0),
                                  tmp_hs);
            key_src_ip = soc_reg_field_get(pcid_info->unit, CONFIGr, tmp_hs[0],
                                           SRC_IP_CFGf);
        } else if (SOC_IS_LYNX(pcid_info->unit)) {
            soc_internal_read_reg(pcid_info,
                                  soc_reg_addr(pcid_info->unit, ARL_CONTROLr,
					       REG_PORT_ANY, 0),
                                  tmp_hs);
            key_src_ip = soc_reg_field_get(pcid_info->unit, ARL_CONTROLr,
					   tmp_hs[0], SRC_IP_CFGf);
        } else { /* Draco1.5 and Tucana */
            key_src_ip = FALSE;
        }

        for (ix = 0; ix < 4; ix++) {
            key[ix] = (data[0] >> (8 * ix)) & 0xff;
            key[ix + 4] = key_src_ip ? 0 : ((data[1] >> (8 * ix)) & 0xff);
        }

        if (soc_feature(pcid_info->unit, soc_feature_l3_sgv)) {
            key[8] = data[2] & 0xff;
            key[9] = (data[2] >> 8) & 0xf;
        }

        hashed_result = soc_draco_l3_hash(pcid_info->unit, hash_sel, 1, key);
        break;
    }

    default:
        debugk(DK_VERBOSE, "invalid hash type selection");
        return -1;
    }

    tmp_hs[0] = hashed_result;
    soc_internal_write_reg(pcid_info,
                           soc_reg_addr(pcid_info->unit, HASH_OUTPUTr,
					REG_PORT_ANY, 0),
                           tmp_hs);
    return 0;
}
#endif	/* BCM_XGS_SWITCH_SUPPORT */

/*
 * soc_internal_send_int
 *
 *    Checks if any interrupt is pending, and if so, sends an interrupt
 *    over the PLI socket.
 */

void
soc_internal_send_int(pcid_info_t *pcid_info)
{
    int rv;

    if (pcid_info->opt_rpc_error) {
        return;
    }
    if (PCIM(pcid_info, CMIC_IRQ_MASK) & PCIM(pcid_info, CMIC_IRQ_STAT)) {
        debugk(DK_VERBOSE, "Send interrupt: msk 0x%x stat 0x%x\n",
            PCIM(pcid_info, CMIC_IRQ_MASK), PCIM(pcid_info, CMIC_IRQ_STAT));
        rv = send_interrupt(pcid_info->client->intsock, 0);
        if (rv < 0) {
            debugk(DK_ERR, "RPC error: soc_internal_send_int failed. \n");
            pcid_info->opt_rpc_error = 1;
        }
    } else {
        debugk(DK_VERBOSE, "Interrupt masked: msk 0x%x stat 0x%x\n",
            PCIM(pcid_info, CMIC_IRQ_MASK), PCIM(pcid_info, CMIC_IRQ_STAT));
    }
}

#ifdef BCM_CMICM_SUPPORT
void
soc_internal_cmicm_send_int(pcid_info_t *pcid_info, uint32 reg)
{
    int rv;

    if (pcid_info->opt_rpc_error) {
        return;
    }
    switch (reg) {
    case CMIC_CMC0_PCIE_IRQ_MASK0_OFFSET:
        if (pcid_reg_read(pcid_info, CMIC_CMC0_PCIE_IRQ_MASK0_OFFSET) & 
            pcid_reg_read(pcid_info, CMIC_CMC0_IRQ_STAT0_OFFSET)) {
            debugk(DK_VERBOSE, "Send interrupt: msk0 0x%x stat0 0x%x\n",
                   pcid_reg_read(pcid_info, CMIC_CMC0_PCIE_IRQ_MASK0_OFFSET), 
                   pcid_reg_read(pcid_info, CMIC_CMC0_IRQ_STAT0_OFFSET));
            rv = send_interrupt(pcid_info->client->intsock, 0);
            if (rv < 0) {
                debugk(DK_ERR, "RPC error: soc_internal_send_int failed. \n");
                pcid_info->opt_rpc_error = 1;
            }
        } else {
            debugk(DK_VERBOSE, "Interrupt masked: msk0 0x%x stat0 0x%x\n",
                   pcid_reg_read(pcid_info, CMIC_CMC0_PCIE_IRQ_MASK0_OFFSET), 
                   pcid_reg_read(pcid_info, CMIC_CMC0_IRQ_STAT0_OFFSET));
        }
        break;
    case CMIC_CMC0_PCIE_IRQ_MASK1_OFFSET:
        if (pcid_reg_read(pcid_info, CMIC_CMC0_PCIE_IRQ_MASK1_OFFSET) & 
            pcid_reg_read(pcid_info, CMIC_CMC0_IRQ_STAT1_OFFSET)) {
            debugk(DK_VERBOSE, "Send interrupt: msk1 0x%x stat1 0x%x\n",
                   pcid_reg_read(pcid_info, CMIC_CMC0_PCIE_IRQ_MASK1_OFFSET), 
                   pcid_reg_read(pcid_info, CMIC_CMC0_IRQ_STAT1_OFFSET));
            rv = send_interrupt(pcid_info->client->intsock, 0);
            if (rv < 0) {
                debugk(DK_ERR, "RPC error: soc_internal_send_int failed. \n");
                pcid_info->opt_rpc_error = 1;
            }
        } else {
            debugk(DK_VERBOSE, "Interrupt masked: msk1 0x%x stat1 0x%x\n",
                   pcid_reg_read(pcid_info, CMIC_CMC0_PCIE_IRQ_MASK1_OFFSET), 
                   pcid_reg_read(pcid_info, CMIC_CMC0_IRQ_STAT1_OFFSET));
        }
        break;
    case CMIC_CMC0_PCIE_IRQ_MASK2_OFFSET:
        if (pcid_reg_read(pcid_info, CMIC_CMC0_PCIE_IRQ_MASK2_OFFSET) & 
            pcid_reg_read(pcid_info, CMIC_CMC0_IRQ_STAT2_OFFSET)) {
            debugk(DK_VERBOSE, "Send interrupt: msk2 0x%x stat2 0x%x\n",
                   pcid_reg_read(pcid_info, CMIC_CMC0_PCIE_IRQ_MASK2_OFFSET), 
                   pcid_reg_read(pcid_info, CMIC_CMC0_IRQ_STAT2_OFFSET));
            rv = send_interrupt(pcid_info->client->intsock, 0);
            if (rv < 0) {
                debugk(DK_ERR, "RPC error: soc_internal_send_int failed. \n");
                pcid_info->opt_rpc_error = 1;
            }
        } else {
            debugk(DK_VERBOSE, "Interrupt masked: msk2 0x%x stat2 0x%x\n",
                   pcid_reg_read(pcid_info, CMIC_CMC0_PCIE_IRQ_MASK2_OFFSET), 
                   pcid_reg_read(pcid_info, CMIC_CMC0_IRQ_STAT2_OFFSET));
        }
    }
}
#endif

int
soc_internal_extended_write_reg(pcid_info_t *pcid_info,
                                soc_block_t block, int acc_type,
                                uint32 address, uint32 *data)
{
    soc_regaddrinfo_t ainfo;
    int rv;

    if (SBUS_ADDR_VERSION_2 == pcid_info->sbus_addr_version) {
        /* To fill in */

        soc_regaddrinfo_extended_get(pcid_info->unit, &ainfo, block, address);
    
        if (!ainfo.valid || ainfo.reg < 0) {
            /* One hidden register recognized for Draco */
            if (!SOC_IS_DRACO(pcid_info->unit) || address != 0xe00010) {
                debugk(DK_WARN,
                       "Attempt to write to unknown S-channel address 0x%x\n",
                       address);
            }
        } else if (SOC_REG_IS_VALID(pcid_info->unit, ainfo.reg) &&
            SOC_REG_INFO(pcid_info->unit, ainfo.reg).flags &
                   SOC_REG_FLAG_RO) {
            if (ainfo.reg != HASH_OUTPUTr) {
                debugk(DK_WARN,
                       "Attempt to write to read-only S-channel address 0x%x\n",
                       address);
            }
        }
#ifdef BCM_TRIDENT_SUPPORT
        if (soc_feature(pcid_info->unit, soc_feature_logical_port_num)) {
            soc_info_t *si;
            int phy_port;
            int blk;

            si = &SOC_INFO(pcid_info->unit);
            switch (ainfo.reg) {
            case IFP_GM_LOGICAL_TO_PHYSICAL_MAPPINGr:
                phy_port = soc_reg_field_get(pcid_info->unit, ainfo.reg,
                                             data[0], PHYSICAL_PORT_NUMf);
                if (phy_port != 0x7f) {
                    si->port_p2l_mapping[phy_port] = ainfo.port;
                    si->port_l2p_mapping[ainfo.port] = phy_port;
                    blk = SOC_DRIVER(pcid_info->unit)->port_info[phy_port].blk;
                    si->port_type[ainfo.port] =
                        SOC_BLOCK_INFO(pcid_info->unit, blk).type;
                    if (ainfo.port > si->port.max) {
                        si->port.max = ainfo.port;
                    }
                }
                break;
            case MMU_TO_PHY_PORT_MAPPINGr:
                phy_port = soc_reg_field_get(pcid_info->unit, ainfo.reg,
                                             data[0], PHY_PORTf);
                if (phy_port != 0x7f) {
                    si->port_p2m_mapping[phy_port] = ainfo.idx & 0x7f;
                    si->port_m2p_mapping[ainfo.idx & 0x7f] = phy_port;
                }
                break;
            default:
                break;
            }
        }
#endif /* BCM_TRIDENT_SUPPORT */
        rv = soc_datum_reg_write(pcid_info, ainfo, block, address, data);

        return rv;
    } else {
        return soc_internal_write_reg(pcid_info, address, data);
    }
}

int
soc_internal_extended_read_reg(pcid_info_t *pcid_info,
                               soc_block_t block, int acc_type,
                               uint32 address, uint32 *data)
{
    soc_regaddrinfo_t ainfo;
    int rv;
        
    if (SBUS_ADDR_VERSION_2 == pcid_info->sbus_addr_version) {
        /* To fill in */

        soc_regaddrinfo_extended_get(pcid_info->unit, &ainfo, block, address);

        if (!ainfo.valid || ainfo.reg < 0) {
            debugk(DK_ERR, 
                   "Attempt to read from unknown S-channel address 0x%x\n",
                   address);
        } else if (SOC_REG_IS_VALID(pcid_info->unit, ainfo.reg) &&
            SOC_REG_INFO(pcid_info->unit, ainfo.reg).flags & SOC_REG_FLAG_WO) {
            debugk(DK_WARN,
                   "Attempt to read from write-only S-channel address 0x%x\n",
                   address);
        }
        rv = soc_datum_reg_read(pcid_info, ainfo, block, address, data);
        if (SOC_E_NOT_FOUND == rv) {
            /*
             * The register is not in the hash table, so it has not been
             * written before.  Provide a default value.
             */
            if (SOC_REG_IS_VALID(pcid_info->unit, ainfo.reg) &&
                (ainfo.valid && ainfo.reg >= 0)) {
                data[0] = SOC_REG_INFO(pcid_info->unit,
                                       ainfo.reg).rst_val_lo;
                data[1] = SOC_REG_INFO(pcid_info->unit,
                                       ainfo.reg).rst_val_hi;
            } else {
                data[0] = 0xffffffff;
                data[1] = 0xffffffff;
            }
            rv = SOC_E_NONE;
        }
        return rv;
    } else {
        return soc_internal_read_reg(pcid_info, address, data);
    }
}

int
soc_internal_extended_write_mem(pcid_info_t *pcid_info,
                                soc_block_t block, int acc_type,
                                uint32 address, uint32 *data)
{
    int rv;
    soc_mem_t       mem;

    if (SBUS_ADDR_VERSION_2 == pcid_info->sbus_addr_version) {
        /* To fill in */

        if ((pcid_info->regmem_cb) 
            && (pcid_info->regmem_cb(pcid_info, BCMSIM_SCHAN_MEM_WRITE, address, data, 
                                     ((SOC_MAX_MEM_WORDS * 4) * 8)) == 0)) {
            return 0;
        }
    
        if ((mem = addr_to_mem_extended(pcid_info->unit, block, address)) != INVALIDm) {
#ifdef	BCM_FIREBOLT_SUPPORT
            if (SOC_IS_FBX(pcid_info->unit)) {
                if (SOC_IS_FB_FX_HX(pcid_info->unit)) {
                    /*
                     * Map EPIPE_HI/IPIPE_HI port registers to EPIPE/IPIPE block
                     */
                    int blk;
                    int unit = pcid_info->unit;
                    int cpu_port = CMIC_PORT(unit);
                    blk = SOC_BLOCK2OFFSET(unit, IPIPE_HI_BLOCK(unit)) << 20;
                    if (((address & 0x00f00000) == blk)
                        && ((address & 0x0000001f) != cpu_port )
                        && (mem != IFP_PORT_FIELD_SELm))    {
                        blk = SOC_BLOCK2OFFSET(unit, IPIPE_BLOCK(unit)) << 20;
                        address = (address & ~(0x00f00000)) | blk;
                    }
                    blk = SOC_BLOCK2OFFSET(unit, EPIPE_HI_BLOCK(unit)) << 20;
                    if (((address & 0x00f00000) == blk)
                        && ((address & 0x0000001f) != cpu_port))    {
                        blk = SOC_BLOCK2OFFSET(unit, EPIPE_BLOCK(unit)) << 20;
                        address = (address & ~(0x00f00000)) | blk;
                    }
                }
    
                switch(mem) {
                    case L3_ENTRY_IPV4_UNICASTm:
                    case L3_ENTRY_IPV4_MULTICASTm:
                    case L3_ENTRY_IPV6_UNICASTm:
                    case L3_ENTRY_IPV6_MULTICASTm:
                        if(SOC_IS_HAWKEYE(pcid_info->unit)){
                            break;
                        }
                        return (soc_internal_l3x2_write(pcid_info, mem,
                                                        address, data));
                    default: break;
                }
            }
#endif /* BCM_FIREBOLT_SUPPORT */

#if defined(BCM_TRX_SUPPORT)
            if (SOC_IS_TRX(pcid_info->unit)) {
                if (mem == VLAN_MACm) {
                    /* In real HW, VLAN_MAC table is overlaid on VLAN_XLATE table */
                    return (soc_internal_vlan_xlate_entry_write(pcid_info, 
                                address, (vlan_xlate_entry_t *) data));
                }
            }
#endif

            if  (soc_mem_is_aggr(pcid_info->unit, mem)) {
                return soc_internal_extended_write_aggrmem(pcid_info, block, acc_type, 
                                                           address, data);
            }
        }

        rv = soc_datum_mem_write(pcid_info, block, address, data);
        return rv;
    } else {
        return soc_internal_write_mem(pcid_info, address, data);
    }
}

int
soc_internal_extended_read_mem(pcid_info_t *pcid_info,
                               soc_block_t block, int acc_type,
                               uint32 address, uint32 *data)
{
    int             i, rv;
    soc_mem_t       mem;

    if (SBUS_ADDR_VERSION_2 == pcid_info->sbus_addr_version) {
        /* To fill in */
        if ((pcid_info->regmem_cb) 
            && (pcid_info->regmem_cb(pcid_info, BCMSIM_SCHAN_MEM_READ, address, data, 
                                     ((SOC_MAX_MEM_WORDS * 4) * 8)) == 0)) {
            return 0;
        }

        if ((mem = addr_to_mem_extended(pcid_info->unit, block, address)) != INVALIDm) {
#ifdef	BCM_FIREBOLT_SUPPORT
            if (SOC_IS_FBX(pcid_info->unit)) {
                switch(mem) {
                    case L3_ENTRY_IPV4_UNICASTm:
                    case L3_ENTRY_IPV4_MULTICASTm:
                    case L3_ENTRY_IPV6_UNICASTm:
                    case L3_ENTRY_IPV6_MULTICASTm:
                        if(SOC_IS_HAWKEYE(pcid_info->unit)){
                            break;
                        }
                        return (soc_internal_l3x2_read(pcid_info, mem,
                                                       address, data));
                    default: break;
                }
            }
#endif /* BCM_FIREBOLT_SUPPORT */
#if defined(BCM_TRX_SUPPORT)
            if (SOC_IS_TRX(pcid_info->unit)) {
                if (mem == VLAN_MACm) {
                    /* In real HW, VLAN_MAC table is overlaid on VLAN_XLATE table */
                    return (soc_internal_vlan_xlate_entry_read(pcid_info, 
                                address, (vlan_xlate_entry_t *) data));
                }
            }
#endif
            if  (soc_mem_is_aggr(pcid_info->unit, mem)) {
                return soc_internal_extended_read_aggrmem(pcid_info, block, 
                                                          acc_type, address, data);
            }
        }

        rv = soc_datum_mem_read(pcid_info, block, address, data);
        if (SOC_E_NOT_FOUND == rv) {
            /* Return null entry value if recognized, otherwise 0xf's */
            if (mem != INVALIDm) {
                memcpy(data,		       /* Memory */
                       soc_mem_entry_null(pcid_info->unit, mem),
                       4 * soc_mem_entry_words(pcid_info->unit, mem));
            } else {			       /* Other */
                for (i = 0; i < SOC_MAX_MEM_WORDS; i++) {
                    data[i] = 0xffffffff;
                }
            }
            rv = SOC_E_NONE;
        }

        return rv;
    } else {
        return soc_internal_read_mem(pcid_info, address, data);
    }
}

int
soc_internal_extended_read_aggrmem(pcid_info_t *pcid_info,
                                   soc_block_t block, int acc_type,
                                   uint32 addr, uint32 *data)
{
    uint32      ent[SOC_MAX_MEM_WORDS];
    uint32      fv[SOC_MAX_MEM_WORDS];
    soc_mem_t   *m = NULL;
    int         index, i;
    int         offset;
    int         adj_offset = 0;
    int         f;
    int         nfield;
    int         pck_factor;
    soc_mem_t   mem;
    soc_mem_info_t *amemp;
    soc_mem_info_t *memp;
    soc_field_info_t *fieldp;
    uint8       at;
    int         unit = pcid_info->unit;

    if (SBUS_ADDR_VERSION_2 == pcid_info->sbus_addr_version) {
        /* To fill in */
        mem = addr_to_mem_extended(pcid_info->unit, block, addr);
    
        if (mem == INVALIDm) {
            return SOC_E_PARAM;
        }
    
        offset = (addr ^ SOC_MEM_INFO(unit, mem).base) & 0xfffff;
        index = 0;
        memset(data, 0, 4 * soc_mem_entry_words(unit, mem));
        do {
            m = SOC_MEM_AGGR(unit, index);
            if (m[0] == mem) {
                /* Fetch the individual components */
                nfield = 1;
                for(i = 1; m[i] != INVALIDm; i++) {
                    /*
                    ** Multiple entries compacted into one entry like HIT
                    ** requires special handling.
                    */
                    amemp = &SOC_MEM_INFO(unit, mem);
                    if(!soc_mem_index_max(unit, m[i]) || !soc_mem_index_max(unit, mem)) {
                        continue;
                    }
                    pck_factor = (soc_mem_index_max(unit, mem) /
                                  soc_mem_index_max(unit, m[i]));
                    assert(pck_factor != 0);
                    adj_offset = offset / pck_factor;
                    soc_internal_extended_read_mem(pcid_info, block, acc_type, 
                                                   soc_mem_addr_get(unit, m[i],
                                                   SOC_MEM_BLOCK_ANY(unit, m[i]),
                                                   adj_offset, &at), ent);
    
                    /* Iterate thru' the fields in this View */
                    memp = &SOC_MEM_INFO(unit, m[i]);
                    if (pck_factor == 1) {
                        for (f = memp->nFields - 1; f >= 0; f--) {
                            fieldp = &memp->fields[f];
                            soc_mem_field_get(unit, m[i], ent, fieldp->field, fv);
                            if (soc_mem_field_valid(unit, mem, fieldp->field) &&
                                soc_mem_field_valid(unit, m[i], fieldp->field)) {
                                soc_mem_field_set(unit, mem, data, fieldp->field,
                                                  fv);
                            }
                            nfield++;
                        }
                    } else if (memp->nFields == pck_factor) {
                            fieldp = &memp->fields[memp->nFields -
                                        (offset % pck_factor) - 1];
                            soc_mem_field_get(unit, m[i], ent, fieldp->field, fv);
                            fieldp = &amemp->fields[(amemp->nFields - nfield)];
                            soc_mem_field_set(unit, mem, data, fieldp->field, fv);
                            nfield++;
                    } else if (memp->nFields == (pck_factor * 2)) {
                            fieldp = &memp->fields[memp->nFields -
                                        (offset % pck_factor) - 1];
                            soc_mem_field_get(unit, m[i], ent, fieldp->field, fv);
                            fieldp = &amemp->fields[(amemp->nFields - nfield)];
                            soc_mem_field_set(unit, mem, data, fieldp->field, fv);
                            nfield++;
    
                            fieldp = &memp->fields[memp->nFields - pck_factor - 
                                        (offset % pck_factor) - 1];
                            soc_mem_field_get(unit, m[i], ent, fieldp->field, fv);
                            fieldp = &amemp->fields[(amemp->nFields - nfield)];
                            soc_mem_field_set(unit, mem, data, fieldp->field, fv);
                            nfield++;
                    } else {
                        debugk(DK_ERR,
                               "soc_internal_extended_read_aggrmem:mem=%d"
                               "Unable to handle Aggregate Mem component %d\n",
                               mem, m[i]);
                    }
                }
                return(0);
            }
            index++;
        } while (m[0] != INVALIDm);
    
        /* Not found in the table of aggregate memories */
        debugk(DK_ERR,
               "soc_internal_extended_read_aggrmem:mem=%d missing in Aggregate Mem List\n",
               mem);
        memcpy(data,		       /* Memory */
               soc_mem_entry_null(unit, mem),
               4 * soc_mem_entry_words(unit, mem));        
        return 0;
    } else {
        return soc_internal_read_aggrmem(pcid_info, addr, data);
    }
}

int
soc_internal_extended_write_aggrmem(pcid_info_t *pcid_info,
                                    soc_block_t block, int acc_type,
                                    uint32 addr, uint32 *data)
{
    uint32      ent[SOC_MAX_MEM_WORDS];
    uint32      fv[SOC_MAX_MEM_WORDS];
    soc_mem_t   *m = NULL;
    int         index, i;
    int         offset;
    int         adj_offset = 0;
    int         f;
    int         nfield;
    int         pck_factor;
    soc_mem_t   mem;
    soc_mem_info_t *amemp;
    soc_mem_info_t *memp;
    soc_field_info_t *fieldp;
    uint8       at;
    int         unit = pcid_info->unit;
    	
    if (SBUS_ADDR_VERSION_2 == pcid_info->sbus_addr_version) {
        /* To fill in */

        mem = addr_to_mem_extended(pcid_info->unit, block, addr);
    
        if (mem == INVALIDm)
        {
            return SOC_E_PARAM;
        }
    
        memset(ent, 0, 4 * soc_mem_entry_words(unit, mem));
        offset = (addr ^ SOC_MEM_INFO(unit, mem).base) & 0xfffff;
        index = 0;
        do {
            m = SOC_MEM_AGGR(unit, index);
            if (m[0] == mem) {
                /* Fetch the individual components */
                nfield = 1;
                for(i = 1; m[i] != INVALIDm; i++) {
                    /*
                    ** Multiple entries compacted into one entry like HIT
                    ** requires special handling.
                    */
                    amemp = &SOC_MEM_INFO(unit, mem);
                    if(!soc_mem_index_max(unit, m[i]) || !soc_mem_index_max(unit, mem)) {
                        continue;
                    }
                    pck_factor = (soc_mem_index_max(unit, mem) /
                                  soc_mem_index_max(unit, m[i]));
                    assert(pck_factor != 0);
                    adj_offset = offset / pck_factor;
                    memp = &SOC_MEM_INFO(unit, m[i]);
    
                    /* Iterate thru' the fields in this View */
                    if (pck_factor == 1) {
                        for (f = memp->nFields - 1; f >= 0; f--) {
                            fieldp = &memp->fields[f];
                            if (soc_mem_field_valid(unit, mem, fieldp->field) &&
                                soc_mem_field_valid(unit, m[i], fieldp->field)) {
                                soc_mem_field_get(unit, mem, data, fieldp->field,
                                                  fv);
                                soc_mem_field_set(unit, m[i], ent, fieldp->field,
                                                  fv);
                            }
                            nfield++;
                        }
                    } else if (memp->nFields == pck_factor) {
                            fieldp = &amemp->fields[(amemp->nFields - nfield)];
                            soc_mem_field_get(unit, mem, data, fieldp->field, fv);
                            fieldp = &memp->fields[memp->nFields -
                                        (offset % pck_factor) - 1];
                            soc_internal_extended_read_mem(pcid_info, block, acc_type,
                                                           soc_mem_addr_get(unit, m[i],
                                                           SOC_MEM_BLOCK_ANY(unit, m[i]),
                                                           adj_offset, &at), ent);
                            soc_mem_field_set(unit, m[i], ent, fieldp->field, fv);
                            nfield++;
                    } else if (memp->nFields == (pck_factor * 2)) {
                            fieldp = &amemp->fields[(amemp->nFields - nfield)];
                            soc_mem_field_get(unit, mem, data, fieldp->field, fv);
                            fieldp = &memp->fields[memp->nFields -
                                        (offset % pck_factor) - 1];
                            soc_internal_extended_read_mem(pcid_info, block, 0,
                                                           soc_mem_addr_get(unit, m[i],
                                                           SOC_MEM_BLOCK_ANY(unit, m[i]),
                                                           adj_offset, &at), ent);
                            soc_mem_field_set(unit, m[i], ent, fieldp->field, fv);
                            nfield++;
    
                            fieldp = &amemp->fields[(amemp->nFields - nfield)];
                            soc_mem_field_get(unit, mem, data, fieldp->field, fv);
                            fieldp = &memp->fields[memp->nFields - pck_factor -
                                        (offset % pck_factor) - 1];
                            soc_mem_field_set(unit, m[i], ent, fieldp->field, fv);
                            nfield++;
                    } else {
                        debugk(DK_ERR,
                               "soc_internal_extended_write_mem:mem=%d"
                               "Unable to handle Aggregate Mem component %d\n",
                               mem, m[i]);
                    }
                    soc_internal_extended_write_mem(pcid_info, block, 0,
                                                    soc_mem_addr_get(unit, m[i],
                                                    SOC_MEM_BLOCK_ANY(unit, m[i]),
                                                    adj_offset, &at), ent);
                }
                return(0);
            }
            index++;
        } while (m[0] != INVALIDm);
    
        /* Not found in the table of aggregate memories */
        debugk(DK_ERR,
               "soc_internal_write_aggrmem:mem=%d missing in Aggregate Mem List\n",
               mem);        
        return 0;
    } else {
        return soc_internal_write_aggrmem(pcid_info, addr, data);
    }
}

int
soc_internal_extended_read_bytes(pcid_info_t *pcid_info,
                                 soc_block_t block, int acc_type,
                                 uint32 address, uint8 *data, int len)
{
    uint32  temp[SOC_MAX_MEM_WORDS];
    int     rv;

    if (SBUS_ADDR_VERSION_2 == pcid_info->sbus_addr_version) {
        /* To fill in */
    
        if ((len < 0) || (len > (SOC_MAX_MEM_WORDS * (sizeof(uint32))))) {
            return 1;
        }
    
        /* reading bytes is supported only for bcmsim */
        if ((pcid_info->regmem_cb) && 
            (pcid_info->regmem_cb(pcid_info, BCMSIM_SCHAN_MEM_READ, address, 
                                  (uint32 *) data, (len * 8)) == 0)) {
            return 0;
        }
    
        /* if no registered call-back, default to read_mem */
        sal_memset(temp, 0, (SOC_MAX_MEM_WORDS * sizeof(uint32)));
        rv = soc_internal_extended_read_mem(pcid_info, block, acc_type, address, 
                                            temp);
        if (!rv) {
            sal_memcpy(data, temp, len);
        }
        return rv;
    } else {
        return soc_internal_read_bytes(pcid_info, address, data, len);
    }
}

int
soc_internal_extended_write_bytes(pcid_info_t *pcid_info,
                                  soc_block_t block, int acc_type,
                                  uint32 address, uint8 *data, int len)
{
    uint32  temp[SOC_MAX_MEM_WORDS];
    if (SBUS_ADDR_VERSION_2 == pcid_info->sbus_addr_version) {
        /* To fill in */

        if ((len < 0) || (len > (SOC_MAX_MEM_WORDS * (sizeof(uint32))))) {
            return 1;
        }
        
        /* writing bytes is supported only for bcmsim */
        if ((pcid_info->regmem_cb) && 
            (pcid_info->regmem_cb(pcid_info, BCMSIM_SCHAN_MEM_WRITE, address, 
                                  (uint32 *)data, (len * 8)) == 0)) {
            return 0;
        }
        
        /* if no registered call-back, default to write_mem */
        sal_memset(temp, 0, (SOC_MAX_MEM_WORDS * sizeof(uint32)));
        sal_memcpy(temp, data, len);
        return soc_internal_extended_write_mem(pcid_info, block, acc_type, 
                                               address, temp);
    } else {
        return soc_internal_write_bytes(pcid_info, address, data, len);
    }
}
