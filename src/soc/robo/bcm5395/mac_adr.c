/*
 * $Id: mac_adr.c 1.12.122.1 Broadcom SDK $
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
#include <soc/mcm/robo/memregs.h>

int
_drv_bcm5395_sec_mac_set(int unit, uint32 bmp, uint32 *mac, int mcast)
{
    int rv = SOC_E_NONE;
    soc_pbmp_t      pbmp;
    uint32      port, temp, vid, prio;
    l2_arl_sw_entry_t    arl_entry;
    
    SOC_PBMP_CLEAR(pbmp);
    SOC_PBMP_WORD_SET(pbmp, 0, bmp);
    PBMP_ITER(pbmp, port) {
        sal_memset(&arl_entry, 0, sizeof (arl_entry));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->port_vlan_pvid_get)
            (unit, port, &vid, &prio));

        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_set)
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
             (uint32 *)&arl_entry, mac)); 

        temp = vid;
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_set)
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, 
             (uint32 *)&arl_entry, &temp)); 
        temp = prio;
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_set)
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_PRIORITY, 
             (uint32 *)&arl_entry, &temp));
        temp = 1;
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_set)
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VALID, 
             (uint32 *)&arl_entry, &temp));    
    
        if(mcast) {
            temp = port;
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_MARL, DRV_MEM_FIELD_DEST_BITMAP, 
                (uint32 *)&arl_entry, &temp));

        } else {
            temp = port;
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, 
                 (uint32 *)&arl_entry, &temp));                
            temp = 1;
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_set)
              (unit, DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, 
                 (uint32 *)&arl_entry, &temp));

        }

        /* Insert this address into arl table. */
        rv = (DRV_SERVICES(unit)->mem_insert)
        (unit, DRV_MEM_ARL, (uint32 *)&arl_entry,
         (DRV_MEM_OP_BY_HASH_BY_MAC | DRV_MEM_OP_BY_HASH_BY_VLANID));

    }

    return rv;
}

int
_drv_bcm5395_sec_mac_remove(int unit, uint32 bmp, uint32 *mac, int mcast)
{
    int rv = SOC_E_NONE;
    soc_pbmp_t      pbmp;
    uint32      port, temp, vid, prio;
    l2_arl_sw_entry_t    arl_entry;
    
    SOC_PBMP_CLEAR(pbmp);
    SOC_PBMP_WORD_SET(pbmp, 0, bmp);
    PBMP_ITER(pbmp, port) {
        sal_memset(&arl_entry, 0, sizeof (arl_entry));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->port_vlan_pvid_get)
            (unit, port, &vid, &prio));

        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_set)
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
             (uint32 *)&arl_entry, mac)); 

        temp = vid;
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_set)
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, 
             (uint32 *)&arl_entry, &temp)); 
        temp = prio;
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_set)
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_PRIORITY, 
             (uint32 *)&arl_entry, &temp));
        temp = 1;
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_set)
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VALID, 
             (uint32 *)&arl_entry, &temp));    
    
        if(mcast) {
            temp = port;
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_MARL, DRV_MEM_FIELD_DEST_BITMAP, 
                (uint32 *)&arl_entry, &temp));

        } else {
            temp = port;
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, 
                 (uint32 *)&arl_entry, &temp));                
            temp = 1;
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_set)
              (unit, DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, 
                 (uint32 *)&arl_entry, &temp));

        }

        /* Insert this address into arl table. */
       rv = (DRV_SERVICES(unit)->mem_delete)
        (unit, DRV_MEM_ARL, (uint32 *)&arl_entry,
         (DRV_MEM_OP_BY_HASH_BY_MAC | DRV_MEM_OP_BY_HASH_BY_VLANID
           | DRV_MEM_OP_DELETE_BY_STATIC));

    }
    return rv;
}

int
_drv_bcm5395_sec_mac_clear(int unit, uint32 bmp)
{
    int rv = SOC_E_NONE;
    uint32  reg_len, reg_addr, reg_value, temp, port;
    int count;
    soc_pbmp_t  pbmp;

    SOC_PBMP_CLEAR(pbmp);
    SOC_PBMP_WORD_SET(pbmp, 0, bmp);
    PBMP_ITER(pbmp, port) {
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, FAST_AGING_PORTr);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, FAST_AGING_PORTr, 0, 0);
        reg_value = 0;
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            goto sec_mac_exit;
        } 
        temp = port;
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)
            (unit, FAST_AGING_PORTr, &reg_value, AGE_SRC_PORTf, &temp));
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            goto sec_mac_exit;
        }
        /* start fast aging process */
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, FAST_AGE_CTLr);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, FAST_AGE_CTLr, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            goto sec_mac_exit;
        }

        temp = 1;    
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, 
            FAST_AGE_CTLr, &reg_value, 
            EN_FAST_AGE_STATICf, &temp));

        temp = 1;
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)(unit, 
            FAST_AGE_CTLr, &reg_value, 
            FAST_AGE_START_DONEf, &temp));
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            goto sec_mac_exit;
        }
        /* wait for complete */
        for (count = 0; count < SOC_TIMEOUT_VAL; count++) {
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
                goto sec_mac_exit;
            }
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                (unit, FAST_AGE_CTLr, &reg_value, 
                FAST_AGE_START_DONEf, &temp));
            if (!temp) {
               break;
            }
        }
        if (count >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto sec_mac_exit;
        }
    }
sec_mac_exit:
    return rv;
}

/*
 *  Function : drv_bcm5395_mac_set
 *
 *  Purpose :
 *      Set the MAC address base on its type.
 *
 *  Parameters :
 *      unit        :   unit id
 *      val     :   port bitmap.
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
drv_bcm5395_mac_set(int unit, soc_pbmp_t pbmp, uint32 mac_type, uint8* mac, uint32 bpdu_idx)
{
    int     rv = SOC_E_NONE, mcast = 0;
    uint32  reg_addr, port;
    uint64 reg_v64, mac_field;
    int     reg_len;
    uint32      reg_index = 0, fld_index = 0;
    uint32  bmp_index = 0, bmp_fld = 0;
    uint32 mport_ctrl_fld = 0;
    uint32  reg_v32;
    uint32  val32;
    int     customeap_en = 0;

    COMPILER_64_ZERO(reg_v64);
    COMPILER_64_ZERO(mac_field);
    if (mac_type != DRV_MAC_SECURITY_CLEAR) {
    soc_cm_debug(DK_VERBOSE, 
            "drv_mac_set: unit %d, bmp = %x, type = %d,  \
            mac =%02x-%02x-%02x-%02x-%02x-%02x\n",
            unit, SOC_PBMP_WORD_GET(pbmp, 0), mac_type, 
            *mac, *(mac+1), *(mac+2),
            *(mac+3), *(mac+4), *(mac+5));
    SAL_MAC_ADDR_TO_UINT64(mac, mac_field);
        if (mac[0] & 0x01) {
            mcast = 1;
        } else {
            mcast = 0;
        }

    } else {
        soc_cm_debug(DK_VERBOSE, 
            "drv_mac_set: unit %d, bmp = %x, type = %d",
            unit, SOC_PBMP_WORD_GET(pbmp, 0), mac_type);
    }       
    
    switch (mac_type) {
        case DRV_MAC_CUSTOM_BPDU:
            reg_index = BPDU_MCADDRr;
            fld_index = BPDU_MC_ADDRf;
            break;
        case DRV_MAC_MULTIPORT_0:
            reg_index = MULTIPORT_ADDR0r;
            fld_index = MPORT_ADDRf;
            bmp_index = MPORTVEC0r;
            bmp_fld = PORT_VCTRf;
            mport_ctrl_fld = MPORT_CTRL0f;
            break;
        case DRV_MAC_MULTIPORT_1:
            reg_index = MULTIPORT_ADDR1r;
            fld_index = MPORT_ADDRf;
            bmp_index = MPORTVEC1r;
            bmp_fld = PORT_VCTRf;
            mport_ctrl_fld = MPORT_CTRL1f;
            break;
        case DRV_MAC_MULTIPORT_2:
            reg_index = MULTIPORT_ADDR2r;
            fld_index = MPORT_ADDRf;
            bmp_index = MPORTVEC2r;
            bmp_fld = PORT_VCTRf;
            mport_ctrl_fld = MPORT_CTRL2f;
            break;
        case DRV_MAC_MULTIPORT_3:
            reg_index = MULTIPORT_ADDR3r;
            fld_index = MPORT_ADDRf;
            bmp_index = MPORTVEC3r;
            bmp_fld = PORT_VCTRf;
            mport_ctrl_fld = MPORT_CTRL3f;
            break;
        case DRV_MAC_MULTIPORT_4:
            reg_index = MULTIPORT_ADDR4r;
            fld_index = MPORT_ADDRf;
            bmp_index = MPORTVEC4r;
            bmp_fld = PORT_VCTRf;
            mport_ctrl_fld = MPORT_CTRL4f;
            break;
        case DRV_MAC_MULTIPORT_5:
            reg_index = MULTIPORT_ADDR5r;
            fld_index = MPORT_ADDRf;
            bmp_index = MPORTVEC5r;
            bmp_fld = PORT_VCTRf;
            mport_ctrl_fld = MPORT_CTRL5f;
            break;
        case DRV_MAC_CUSTOM_EAP:      
            customeap_en = (COMPILER_64_IS_ZERO(mac_field)) ? 0 : 1;
            PBMP_ITER(pbmp, port) {
                SOC_IF_ERROR_RETURN(REG_READ_PORT_EAP_CONr(
                        unit, port, (uint32 *)&reg_v64 ));
                /* set EAP_DA */
                SOC_IF_ERROR_RETURN(soc_PORT_EAP_CONr_field_set(
                        unit, (uint32 *)&reg_v64, EAP_UNI_DAf, 
                        (uint32 *)&mac_field));
                /* set enabling status :
                 *  - set EAP_DA to zero MAC will disable this feature. 
                 */
                SOC_IF_ERROR_RETURN(soc_PORT_EAP_CONr_field_set(
                        unit, (uint32 *)&reg_v64, EAP_EN_UNI_DAf, 
                        (uint32 *)&customeap_en));
                SOC_IF_ERROR_RETURN(REG_WRITE_PORT_EAP_CONr(
                        unit, port, (uint32 *)&reg_v64 ));
            }
            return SOC_E_NONE;
            break;
        case DRV_MAC_MIRROR_IN:
            reg_index = IGMIRMACr;
            fld_index = IN_MIR_MACf;
            break;
        case DRV_MAC_MIRROR_OUT:
            reg_index = EGMIRMACr;
            fld_index = OUT_MIR_MACf;
            break;
        case DRV_MAC_SECURITY_ADD:                                  
            rv = _drv_bcm5395_sec_mac_set
                (unit, SOC_PBMP_WORD_GET(pbmp, 0), (uint32 *)&mac_field, mcast);
            return rv;
        case DRV_MAC_SECURITY_REMOVE:            
            rv = _drv_bcm5395_sec_mac_remove
                (unit, SOC_PBMP_WORD_GET(pbmp, 0), (uint32 *)&mac_field, mcast);
            return rv;
        case DRV_MAC_SECURITY_CLEAR:            
            rv = _drv_bcm5395_sec_mac_clear
                (unit, SOC_PBMP_WORD_GET(pbmp, 0));
            return rv;
        case DRV_MAC_IGMP_REPLACE:            
            rv = SOC_E_UNAVAIL;
            return rv;
        default :
            rv = SOC_E_PARAM;
            return rv;
    }
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, reg_index);
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, reg_index, 0, 0);
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, reg_index, (uint32 *)&reg_v64, 
            fld_index, (uint32 *)&mac_field));
    if ((rv = (DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, (uint32 *)&reg_v64, reg_len)) < 0) {
       return rv;
    }
    if((mac_type == DRV_MAC_MULTIPORT_0) || 
        (mac_type == DRV_MAC_MULTIPORT_1) ||
        (mac_type == DRV_MAC_MULTIPORT_2) ||
        (mac_type == DRV_MAC_MULTIPORT_3) ||
        (mac_type == DRV_MAC_MULTIPORT_4) ||
        (mac_type == DRV_MAC_MULTIPORT_5)) {
        /* Enable MPORT_CTRL(N) for Multiport Address(N) and Vector(N), N = 0 ~ 5
          * N = 0 : DRV_MAC_MULTIPORT_0,
          * N = 1 : DRV_MAC_MULTIPORT_1,
          * N = 2 : DRV_MAC_MULTIPORT_2,
          * N = 3 : DRV_MAC_MULTIPORT_3,
          * N = 4 : DRV_MAC_MULTIPORT_4,
          * N = 5 : DRV_MAC_MULTIPORT_5,
          */           
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, MULTI_PORT_CTLr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, MULTI_PORT_CTLr);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_v32, reg_len)) < 0) {
            return rv;
        }

        /* Disable MPORT_CTRL(N) if Multiport Address and Vector are all zero */
        if ((ENET_CMP_MACADDR(mac, _soc_mac_all_zeroes) == 0) && 
            (SOC_PBMP_IS_NULL(pbmp))) {
            val32 = DRV_MULTIPORT_CTRL_DISABLE;
        } else {
            val32 = DRV_MULTIPORT_CTRL_MATCH_ADDR;
        }
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                (unit, MULTI_PORT_CTLr, &reg_v32, 
                mport_ctrl_fld, &val32));

        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_v32, reg_len)) < 0) {
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

    return rv;
}

/*
 *  Function : drv_bcm5395_mac_get
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
drv_bcm5395_mac_get(int unit, uint32 port, uint32 mac_type, 
                                                soc_pbmp_t *bmp, uint8* mac)
{
    int     rv = SOC_E_NONE;
    uint32  reg_addr;
    uint64  reg_v64, mac_field;
    int     reg_len;
    uint32      reg_index = 0, fld_index = 0;
    uint32  bmp_index = 0, bmp_fld = 0, reg_v32, fld_v32;
    
    switch (mac_type) {
        case DRV_MAC_CUSTOM_BPDU:
            reg_index = BPDU_MCADDRr;
            fld_index = BPDU_MC_ADDRf;
            break;
        case DRV_MAC_MULTIPORT_0:
            reg_index = MULTIPORT_ADDR0r;
            fld_index = MPORT_ADDRf;
            bmp_index = MPORTVEC0r;
            bmp_fld = PORT_VCTRf;
            break;
        case DRV_MAC_MULTIPORT_1:
            reg_index = MULTIPORT_ADDR1r;
            fld_index = MPORT_ADDRf;
            bmp_index = MPORTVEC1r;
            bmp_fld = PORT_VCTRf;
            break;
        case DRV_MAC_MULTIPORT_2:
            reg_index = MULTIPORT_ADDR2r;
            fld_index = MPORT_ADDRf;
            bmp_index = MPORTVEC2r;
            bmp_fld = PORT_VCTRf;
            break;
        case DRV_MAC_MULTIPORT_3:
            reg_index = MULTIPORT_ADDR3r;
            fld_index = MPORT_ADDRf;
            bmp_index = MPORTVEC3r;
            bmp_fld = PORT_VCTRf;
            break;
        case DRV_MAC_MULTIPORT_4:
            reg_index = MULTIPORT_ADDR4r;
            fld_index = MPORT_ADDRf;
            bmp_index = MPORTVEC4r;
            bmp_fld = PORT_VCTRf;
            break;
        case DRV_MAC_MULTIPORT_5:
            reg_index = MULTIPORT_ADDR5r;
            fld_index = MPORT_ADDRf;
            bmp_index = MPORTVEC5r;
            bmp_fld = PORT_VCTRf;
            break;
        case DRV_MAC_CUSTOM_EAP:
            reg_index = PORT_EAP_CONr;
            fld_index = EAP_UNI_DAf;
            break;
        case DRV_MAC_MIRROR_IN:
            reg_index = IGMIRMACr;
            fld_index = IN_MIR_MACf;
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
    if (mac_type == DRV_MAC_CUSTOM_EAP) {
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, reg_index, port, 0);
    } else {
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, reg_index, 0, 0);
    }
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, (uint32 *)&reg_v64, reg_len)) < 0) {
       return rv;
    }
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, reg_index, (uint32 *)&reg_v64, 
                fld_index, (uint32 *)&mac_field));
  
    SAL_MAC_ADDR_FROM_UINT64(mac, mac_field);

    if((mac_type == DRV_MAC_MULTIPORT_0) || 
        (mac_type == DRV_MAC_MULTIPORT_1) ||
        (mac_type == DRV_MAC_MULTIPORT_2) ||
        (mac_type == DRV_MAC_MULTIPORT_3) ||
        (mac_type == DRV_MAC_MULTIPORT_4) ||
        (mac_type == DRV_MAC_MULTIPORT_5)) {
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
        unit, port, mac_type, *mac, *(mac+1), *(mac+2),
        *(mac+3), *(mac+4), *(mac+5));
    return rv;
}
