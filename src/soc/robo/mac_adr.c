/*
 * $Id: mac_adr.c 1.4.248.1 Broadcom SDK $
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
 */
#include <soc/drv.h>
#include <soc/debug.h>

int
_drv_sec_mac_set(int unit, uint32 bmp, uint32 *mac)
{
    int rv = SOC_E_NONE;
    stsec_mac0_entry_t secmac_entry;
    soc_pbmp_t      pbmp;
    int     i, index;
    uint32      port, temp, secmac_num;

    if ((rv = (DRV_SERVICES(unit)->dev_prop_get)
        (unit, DRV_DEV_PROP_SEC_MAC_NUM_PER_PORT, &secmac_num)) < 0) {
        return rv;
    }
    SOC_PBMP_CLEAR(pbmp);
    SOC_PBMP_WORD_SET(pbmp, 0, bmp);
    PBMP_ITER(pbmp, port) {
        for (i = 0; i < secmac_num; i++) {
            sal_memset(&secmac_entry, 0, sizeof(stsec_mac0_entry_t));
            index = (port * secmac_num) + i;
            if ((rv = (DRV_SERVICES(unit)->mem_read)(unit, DRV_MEM_SECMAC, 
                index, 1, (uint32 *)&secmac_entry)) < 0) {
                return rv;
            }
            if ((rv = (DRV_SERVICES(unit)->mem_field_get)
                (unit, DRV_MEM_SECMAC, DRV_MEM_FIELD_VALID, 
                (uint32 *)&secmac_entry, &temp)) < 0) {
                return rv;
            }
            if (!temp) {
                if ((rv = (DRV_SERVICES(unit)->mem_field_set)
                    (unit, DRV_MEM_SECMAC, DRV_MEM_FIELD_MAC, 
                    (uint32 *)&secmac_entry, mac)) < 0) {
                    return rv;
                }
                temp = 1;
                if ((rv = (DRV_SERVICES(unit)->mem_field_set)
                    (unit, DRV_MEM_SECMAC, DRV_MEM_FIELD_VALID, 
                    (uint32 *)&secmac_entry, &temp)) < 0) {
                    return rv;
                }
                if ((rv = (DRV_SERVICES(unit)->mem_write)(unit, 
                    DRV_MEM_SECMAC, index, 1, (uint32 *)&secmac_entry)) < 0) {
                    return rv;
                }
                break;
            }
        }
        if (i == secmac_num) {
            return SOC_E_FULL;
        }
    }
    return rv;
}

int
_drv_sec_mac_remove(int unit, uint32 bmp, uint32 *mac)
{
    int rv = SOC_E_NONE;
    stsec_mac0_entry_t secmac_entry;
    soc_pbmp_t      pbmp;
    int     i, index;
    uint32      port, temp, secmac_num;
    uint32      temp_mac[2];

    if ((rv = (DRV_SERVICES(unit)->dev_prop_get)
        (unit, DRV_DEV_PROP_SEC_MAC_NUM_PER_PORT, &secmac_num)) < 0) {
        return rv;
    }
    SOC_PBMP_CLEAR(pbmp);
    SOC_PBMP_WORD_SET(pbmp, 0, bmp);
    PBMP_ITER(pbmp, port) {
        for (i = 0; i < secmac_num; i++) {
            sal_memset(&secmac_entry, 0, sizeof(stsec_mac0_entry_t));
            index = (port * secmac_num) + i;
            if ((rv = (DRV_SERVICES(unit)->mem_read)(unit, DRV_MEM_SECMAC, 
                index, 1, (uint32 *)&secmac_entry)) < 0) {
                return rv;
            }
            if ((rv = (DRV_SERVICES(unit)->mem_field_get)
                (unit, DRV_MEM_SECMAC, DRV_MEM_FIELD_VALID, 
                (uint32 *)&secmac_entry, &temp)) < 0) {
                return rv;
            }
            if (temp) {
                if ((rv = (DRV_SERVICES(unit)->mem_field_get)
                    (unit, DRV_MEM_SECMAC, DRV_MEM_FIELD_MAC, 
                    (uint32 *)&secmac_entry, temp_mac)) < 0) {
                    return rv;
                }
                if (!sal_memcmp(temp_mac, mac, 6)) {
                    temp = 0;
                    if ((rv = (DRV_SERVICES(unit)->mem_field_set)
                        (unit, DRV_MEM_SECMAC, DRV_MEM_FIELD_VALID, 
                        (uint32 *)&secmac_entry, &temp)) < 0) {
                        return rv;
                    }
                    if ((rv = (DRV_SERVICES(unit)->mem_write)
                        (unit, DRV_MEM_SECMAC, index, 1, 
                        (uint32 *)&secmac_entry)) < 0) {
                        return rv;
                    }
                    return SOC_E_NONE;
                }
            }
        }
        if (i == secmac_num) {
            return SOC_E_NOT_FOUND;
        }
    }
    return SOC_E_NONE;
}

int
_drv_sec_mac_clear(int unit, uint32 bmp)
{
    int rv = SOC_E_NONE;
    stsec_mac0_entry_t secmac_entry;
    soc_pbmp_t      pbmp;
    int     i, index;
    uint32      port, secmac_num;

    if ((rv = (DRV_SERVICES(unit)->dev_prop_get)
        (unit, DRV_DEV_PROP_SEC_MAC_NUM_PER_PORT, &secmac_num)) < 0) {
        return rv;
    }
    SOC_PBMP_CLEAR(pbmp);
    SOC_PBMP_WORD_SET(pbmp, 0, bmp);
    PBMP_ITER(pbmp, port) {
        for (i = 0; i < secmac_num; i++) {
            sal_memset(&secmac_entry, 0, sizeof(stsec_mac0_entry_t));
            index = (port * secmac_num) + i;
            if ((rv = (DRV_SERVICES(unit)->mem_write)
                (unit, DRV_MEM_SECMAC, index, 1, 
                (uint32 *)&secmac_entry)) < 0) {
                return rv;
            }
        }
    }
    return rv;
}


/*
 *  Function : drv_mac_set
 *
 *  Purpose :
 *      Set the MAC address base on its type.
 *
 *  Parameters :
 *      unit        :   unit id
 *      bmp     :   port bitmap.
 *      mac_type   :   mac address type.
 *      mac     :   mac address.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */
int 
drv_mac_set(int unit, soc_pbmp_t pbmp, uint32 mac_type, uint8* mac, uint32 bpdu_idx)
{
    int     rv = SOC_E_NONE;
    uint32  reg_addr, port, mirror_in;
    uint64 reg_v64, mac_field;
    int     reg_len;
    uint32      reg_index = 0, fld_index = 0;
    uint32  bmp_index = 0, bmp_fld = 0, reg_v32;
    uint32  val32;

    if (mac_type != DRV_MAC_SECURITY_CLEAR) {
        soc_cm_debug(DK_VERBOSE, 
            "drv_mac_set: unit %d, bmp = %x, type = %d,  \
            mac =%02x-%02x-%02x-%02x-%02x-%02x\n",
            unit, SOC_PBMP_WORD_GET(pbmp, 0), mac_type, *mac, *(mac+1), *(mac+2),
            *(mac+3), *(mac+4), *(mac+5));
        SAL_MAC_ADDR_TO_UINT64(mac, mac_field);
    } else {
        soc_cm_debug(DK_VERBOSE, 
            "drv_mac_set: unit %d, val = %x, type = %d",
            unit, SOC_PBMP_WORD_GET(pbmp, 0), mac_type);
    }
    
    switch (mac_type) {
        case DRV_MAC_CUSTOM_BPDU:
        	if (bpdu_idx == 0) {
                reg_index = BPDU_MCADDRr;
                fld_index = BPDU_MC_ADDRf;
        	} else if (SOC_IS_ROBO5324_A1(unit)){
                /* enable MULTIPORT ADDR1 and ADDR2 
                 * to store the 2nd and 3rd BPDU addresses.
                 */
                reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, GARLCFGr, 0, 0);
                if ((rv = (DRV_SERVICES(unit)->reg_read)
                    (unit, reg_addr, (uint32 *)&reg_v64, 1)) < 0) {
                   return rv;
                }
                reg_v32 = 0;
                SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                    (unit, GARLCFGr, (uint32 *)&reg_v64, 
                        MPADDR_ENf, (uint32 *)&reg_v32));
                if ((rv = (DRV_SERVICES(unit)->reg_write)
                    (unit, reg_addr, (uint32 *)&reg_v64, 1)) < 0) {
                   return rv;
                }
                
                if (bpdu_idx == 1) {
                    reg_index = GRPADDR1r;
                    fld_index = GRP_ADDRf;
                } else { /* bpdu_idx = 2 */
                    reg_index = GRPADDR2r;
                    fld_index = GRP_ADDRf;
            	} 
        	}        	
            break;
        case DRV_MAC_MULTIPORT_0:
            reg_index = GRPADDR1r;
            fld_index = GRP_ADDRf;
            bmp_index = PORTVEC1r;
            bmp_fld = PORT_VCTRf;
            break;
        case DRV_MAC_MULTIPORT_1:
            reg_index = GRPADDR2r;
            fld_index = GRP_ADDRf;
            bmp_index = PORTVEC2r;
            bmp_fld = PORT_VCTRf;
            break;
        case DRV_MAC_CUSTOM_EAP:
            reg_index = PORT_EAP_CONr;
            fld_index = EAP_UNI_DAf;
            break;
        case DRV_MAC_MIRROR_IN:
            reg_index = IGMIRMACr;
            fld_index = IN_MIR_MACf;
            /* only mask bit 47-24 */
            COMPILER_64_SHR(mac_field, 24);
            COMPILER_64_TO_32_LO(mirror_in, mac_field);
            break;
        case DRV_MAC_MIRROR_OUT:
            reg_index = EGMIRMACr;
            fld_index = OUT_MIR_MACf;
            break;
        case DRV_MAC_IGMP_REPLACE:
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_MAC_SECURITY_ADD:
            rv = _drv_sec_mac_set(unit, SOC_PBMP_WORD_GET(pbmp, 0), (uint32 *)&mac_field);
            return rv;
        case DRV_MAC_SECURITY_REMOVE:
            rv = _drv_sec_mac_remove(unit, SOC_PBMP_WORD_GET(pbmp, 0), (uint32 *)&mac_field);
            return rv;
        case DRV_MAC_SECURITY_CLEAR:
            rv = _drv_sec_mac_clear(unit, SOC_PBMP_WORD_GET(pbmp, 0));
            return rv;
        default :
            rv = SOC_E_PARAM;
            return rv;
    }
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, reg_index);
    if (mac_type == DRV_MAC_CUSTOM_EAP) {
        PBMP_ITER(pbmp, port) {
            reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, reg_index, port, 0);
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                (unit, reg_index, (uint32 *)&reg_v64, 
                    fld_index, (uint32 *)&mac_field));
            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, (uint32 *)&reg_v64, reg_len)) < 0) {
               return rv;
            }
        }
    } else {
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, reg_index, 0, 0);
        if (mac_type == DRV_MAC_MIRROR_IN) {
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                (unit, reg_index, (uint32 *)&reg_v64, fld_index, &mirror_in));
        } else {
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                (unit, reg_index, (uint32 *)&reg_v64, 
                    fld_index, (uint32 *)&mac_field));
        }
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, reg_index, (uint32 *)&reg_v64, 
                fld_index, (uint32 *)&mac_field));
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, (uint32 *)&reg_v64, reg_len)) < 0) {
           return rv;
        }
        if((mac_type == DRV_MAC_MULTIPORT_0) || 
    	    (mac_type == DRV_MAC_MULTIPORT_1))  {
            /* enable MULTIPORT Address 1 and 2 register
              * and their associated MULTIPORT Vector1 and 2 register
              */
            reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, GARLCFGr, 0, 0);
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, (uint32 *)&reg_v64, 1)) < 0) {
                return rv;
            }
            reg_v32 = 1;
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                (unit, GARLCFGr, (uint32 *)&reg_v64, 
                    MPADDR_ENf, (uint32 *)&reg_v32));
            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, (uint32 *)&reg_v64, 1)) < 0) {
                return rv;
            }
            
            reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, bmp_index);
            reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, bmp_index, 0, 0);

            val32 = SOC_PBMP_WORD_GET(pbmp, 0);
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                (unit, bmp_index, &reg_v32, 
                    bmp_fld, &val32));
            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_v32, reg_len)) < 0) {
               return rv;
            }
        } 

    }
    return rv;
}

/*
 *  Function : drv_mac_get
 *
 *  Purpose :
 *      Get the MAC address base on its type.
 *
 *  Parameters :
 *      unit        :   unit id
 *      port        :   port number.
 *      mac_type   :   mac address type.
 *      mac     :   mac address.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      It didn't support to get the Secure MAC address for each port. 
 *      Using the mem_read to achieve this.
 *
 */
int 
drv_mac_get(int unit, uint32 val, uint32 mac_type, soc_pbmp_t *bmp, uint8* mac)
{
    int     rv = SOC_E_NONE;
    uint32  reg_addr;
    uint64  reg_v64, mac_field;
    int     reg_len;
    uint32  bmp_index = 0, bmp_fld = 0, reg_v32, fld_v32;
    uint32      reg_index = 0, fld_index = 0;
    
    switch (mac_type) {
        case DRV_MAC_CUSTOM_BPDU:
        	if (val == 0) {
            reg_index = BPDU_MCADDRr;
            fld_index = BPDU_MC_ADDRf;
        	} else if (SOC_IS_ROBO5324_A1(unit)){
                /* enable MULTIPORT ADDR1 and ADDR2 
                 * to store the 2nd and 3rd BPDU addresses.
                 */
                reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, GARLCFGr, 0, 0);
                if ((rv = (DRV_SERVICES(unit)->reg_read)
                    (unit, reg_addr, (uint32 *)&reg_v64, 1)) < 0) {
                   return rv;
                }
                reg_v32 = 0;
                SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                    (unit, GARLCFGr, (uint32 *)&reg_v64, 
                        MPADDR_ENf, (uint32 *)&reg_v32));
                if ((rv = (DRV_SERVICES(unit)->reg_write)
                    (unit, reg_addr, (uint32 *)&reg_v64, 1)) < 0) {
                   return rv;
                }
                
                if (val == 1) {
                    reg_index = GRPADDR1r;
                    fld_index = GRP_ADDRf;
                } else { /* val = 2 */
                    reg_index = GRPADDR2r;
                    fld_index = GRP_ADDRf;
            	} 
        	}        	
            break;
        case DRV_MAC_MULTIPORT_0:
            reg_index = GRPADDR1r;
            fld_index = GRP_ADDRf;
            bmp_index = PORTVEC1r;
            bmp_fld = PORT_VCTRf;
            break;
        case DRV_MAC_MULTIPORT_1:
            reg_index = GRPADDR2r;
            fld_index = GRP_ADDRf;
            bmp_index = PORTVEC2r;
            bmp_fld = PORT_VCTRf;
            break;
        case DRV_MAC_CUSTOM_EAP:
            reg_index = PORT_EAP_CONr;
            fld_index = EAP_UNI_DAf;
            break;
        case DRV_MAC_MIRROR_IN:
            reg_index = IGMIRMACr;
            fld_index = IN_MIR_MACf;
            /* only mask bit 47-24 */
            break;
        case DRV_MAC_MIRROR_OUT:
            reg_index = EGMIRMACr;
            fld_index = OUT_MIR_MACf;
            break;
        case DRV_MAC_IGMP_REPLACE:
        case DRV_MAC_SECURITY_ADD:
        case DRV_MAC_SECURITY_REMOVE:
        case DRV_MAC_SECURITY_CLEAR:
            rv = SOC_E_UNAVAIL;
            return rv;
        default :
            rv = SOC_E_PARAM;
            return rv;
    }
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, reg_index);
    if (mac_type == DRV_MAC_CUSTOM_BPDU) {
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, reg_index, 0, 0);
    } else {
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, reg_index, val, 0);
    }
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, (uint32 *)&reg_v64, reg_len)) < 0) {
       return rv;
    }
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, reg_index, (uint32 *)&reg_v64, 
                fld_index, (uint32 *)&mac_field));
    if (mac_type == DRV_MAC_MIRROR_IN) {
        COMPILER_64_SHL(mac_field, 24);
    }

    SAL_MAC_ADDR_FROM_UINT64(mac, mac_field);

    if((mac_type == DRV_MAC_MULTIPORT_0) || 
    	(mac_type == DRV_MAC_MULTIPORT_1)) {
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, bmp_index);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, bmp_index, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_v32, reg_len)) < 0) {
           return rv;
        }
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                (unit, bmp_index, &reg_v32, 
                    bmp_fld, &fld_v32));  
        SOC_PBMP_WORD_SET(*bmp, 0, fld_v32);
    }
    soc_cm_debug(DK_VERBOSE, 
        "drv_mac_get: unit %d, port = %d, type = %d,  \
        mac =%02x-%02x-%02x-%02x-%02x-%02x\n",
        unit, val, mac_type, *mac, *(mac+1), *(mac+2),
        *(mac+3), *(mac+4), *(mac+5));
    return rv;
}
