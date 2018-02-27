/*
 * $Id: port.c 1.20 Broadcom SDK $
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
 * File:        port.c
 * Purpose:     Easyrider port function implementations
 */

#include <soc/defs.h>

#if defined (BCM_EASYRIDER_SUPPORT)

#include <sal/core/libc.h>

#include <soc/mem.h>
#include <soc/debug.h>
#include <soc/cm.h>
#include <soc/drv.h>
#include <soc/register.h>
#include <soc/memory.h>
#include <soc/ll.h>
#include <soc/ptable.h>
#include <soc/l2x.h>

#include <bcm/port.h>
#include <bcm/vlan.h>
#include <bcm/stack.h>
#include <bcm/filter.h>
#include <bcm/error.h>
#include <bcm/link.h>
#include <bcm/mirror.h>

#include <bcm_int/common/lock.h>
#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw/firebolt.h>
#include <bcm_int/esw/mirror.h>

#include <bcm_int/esw/easyrider.h>

/*
 * Function:
 *      _bcm_er_port_tpid_set
 * Description:
 *      Set the default Tag Protocol ID for a port.
 * Parameters:
 *      unit - (IN) Device number
 *      port - (IN) Port number
 *      tpid - (IN) Tag Protocol ID
 * Return Value:
 *      BCM_E_XXX
 */
int
_bcm_er_port_tpid_set(int unit, bcm_port_t port, uint16 tpid)
{
    uint32  regval, oldregval;

    if (IS_HG_PORT(unit, port)) {
        SOC_IF_ERROR_RETURN(READ_IGR_IPORTr(unit, port, &regval));
        oldregval = regval;
        soc_reg_field_set(unit, IGR_IPORTr, &regval,
                          OUTER_TPIDf, tpid);
        if (regval != oldregval) {
            SOC_IF_ERROR_RETURN(WRITE_IGR_IPORTr(unit, port, regval));
        }
        SOC_IF_ERROR_RETURN(READ_IEGR_PORTr(unit, port, &regval));
        oldregval = regval;
        soc_reg_field_set(unit, IEGR_PORTr, &regval,
                          OUTER_TPIDf, tpid);
        if (regval != oldregval) {
            SOC_IF_ERROR_RETURN(WRITE_IEGR_PORTr(unit, port, regval));
        }
    } else {
        SOC_IF_ERROR_RETURN(READ_IGR_PORTr(unit, port, &regval));
        oldregval = regval;
        soc_reg_field_set(unit, IGR_PORTr, &regval,
                          OUTER_TPIDf, tpid);
        if (regval != oldregval) {
            SOC_IF_ERROR_RETURN(WRITE_IGR_PORTr(unit, port, regval));
        }
        SOC_IF_ERROR_RETURN(READ_EGR_PORTr(unit, port, &regval));
        oldregval = regval;
        soc_reg_field_set(unit, EGR_PORTr, &regval,
                          OUTER_TPIDf, tpid);
        if (regval != oldregval) {
            SOC_IF_ERROR_RETURN(WRITE_EGR_PORTr(unit, port, regval));
        }
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_er_port_tpid_get
 * Description:
 *      Retrieve the default Tag Protocol ID for a port.
 * Parameters:
 *      unit - (IN) Device number
 *      port - (IN) Port number
 *      tpid - (OUT) Tag Protocol ID
 * Return Value:
 *      BCM_E_XXX
 */
int
_bcm_er_port_tpid_get(int unit, bcm_port_t port, uint16 *tpid)
{
    uint32  regval;
    /* Input parameters check. */
    if (NULL == tpid) {
        return (BCM_E_PARAM);
    }

    if (IS_HG_PORT(unit, port)) {
        SOC_IF_ERROR_RETURN(READ_IGR_IPORTr(unit, port, &regval));
        *tpid = soc_reg_field_get(unit, IGR_IPORTr, regval,
                                  OUTER_TPIDf);
    } else {
        SOC_IF_ERROR_RETURN(READ_IGR_PORTr(unit, port, &regval));
        *tpid = soc_reg_field_get(unit, IGR_PORTr, regval,
                                  OUTER_TPIDf);
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_er_port_inner_tpid_set
 * Purpose:
 *      Set the expected TPID for the inner tag in double-tagging mode.
 * Parameters:
 *      unit - (IN) BCM device number
 *      port - (IN) Port number
 *      tpid - (IN) Tag Protocol ID
 * Returns:
 *      BCM_E_NONE - Success.
 *      BCM_E_XXX
 */
int
_bcm_er_port_inner_tpid_set(int unit, bcm_port_t port, uint16 tpid)
{
    uint32 rval=0;
    soc_reg_field_set(unit, IGR_VLAN_CONTROLr, &rval, INNER_TPIDf, tpid);
    BCM_IF_ERROR_RETURN(WRITE_IGR_VLAN_CONTROLr(unit, rval));

    rval=0;
    soc_reg_field_set(unit, EGR_VLAN_CONTROLr, &rval, 
                      INNER_TAG_TYPE_FIELDf, tpid);
    BCM_IF_ERROR_RETURN(WRITE_EGR_VLAN_CONTROLr(unit, rval));
    return BCM_E_NONE;
}


/*
 * Function:
 *      _bcm_er_port_inner_tpid_get
 * Purpose:
 *      Get the expected TPID for the inner tag in double-tagging mode.
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      tpid - (OUT) Tag Protocol ID
 * Returns:
 *      BCM_E_NONE - Success.
 *      BCM_E_XXX
 */
int
_bcm_er_port_inner_tpid_get(int unit, bcm_port_t port, uint16 *tpid)
{
    uint32 rval;

    /* Input parameters check. */
    if (NULL == tpid) {
        return (BCM_E_PARAM);
    }

    BCM_IF_ERROR_RETURN(READ_IGR_VLAN_CONTROLr(unit, &rval));
    *tpid = soc_reg_field_get(unit, IGR_VLAN_CONTROLr, rval, INNER_TPIDf);

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_er_port_protocol_vlan_add
 * Purpose:
 *      Adds a protocol based vlan to a port.  The protocol
 *      is matched by frame type and ether type.  
 * Parameters:
 *      unit  - (IN) BCM device number
 *      port  - (IN) Port number
 *      frame - (IN) one of BCM_PORT_FRAMETYPE_{ETHER2,8023,LLC}
 *      ether - (IN) 16 bit Ethernet type field
 *      vid   - (IN)VLAN ID
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_er_port_protocol_vlan_add(int unit,
                           bcm_port_t port,
                           bcm_port_frametype_t frame,
                           bcm_port_ethertype_t ether,
                           bcm_vlan_t vid)
{
    int                         i;
    int                         empty;
    bcm_port_frametype_t        ft;
    bcm_port_ethertype_t        et;
    _bcm_port_info_t            *pinfo;
    uint32                      reg32;
    /*
     * Search for an empty protocol mapping entry.
     * Must not be an existing entry that matches requested protocol.
     */
    empty = -1;
    for (i = 0; i < SOC_REG_NUMELS(unit, VLAN_PROTOCOLr); i++) {
        SOC_IF_ERROR_RETURN
            (READ_VLAN_PROTOCOLr(unit, port, i, &reg32));
        ft = 0;
        if (soc_reg_field_get(unit, VLAN_PROTOCOLr, reg32, ETHERIIf)) {
            ft |= BCM_PORT_FRAMETYPE_ETHER2;
        }
        if (soc_reg_field_get(unit, VLAN_PROTOCOLr, reg32, SNAPf)) {
            ft |= BCM_PORT_FRAMETYPE_8023;
        }
        if (soc_reg_field_get(unit, VLAN_PROTOCOLr, reg32, LLCf)) {
            ft |= BCM_PORT_FRAMETYPE_LLC;
        }
        et = soc_reg_field_get(unit, VLAN_PROTOCOLr, reg32, ETHERTYPEf);
        if (empty < 0 && ft == 0) {
            empty = i;
        }
        if (ft == frame && et == ether) {
            /* Mapping already exists; must remove it first. */
            return BCM_E_EXISTS;
        }
    }

    if (empty < 0) {
        return BCM_E_FULL;
    }

    /*
     * Set VLAN ID for target port.
     */
    reg32 = 0;
    BCM_IF_ERROR_RETURN(_bcm_port_info_get(unit, port, &pinfo));
    soc_reg_field_set(unit, VLAN_PROTOCOL_DATAr, &reg32,
                      VIDf, vid);
    soc_reg_field_set(unit, VLAN_PROTOCOL_DATAr, &reg32,
                      PRIf, pinfo->p_ut_prio);
    SOC_IF_ERROR_RETURN
        (WRITE_VLAN_PROTOCOL_DATAr(unit, port, empty, reg32));

    /*
     * Enable protocol mapping for target port.
     */
    reg32 = 0;
    soc_reg_field_set(unit, VLAN_PROTOCOLr, &reg32,
                      ETHERTYPEf, ether);
    if (frame & BCM_PORT_FRAMETYPE_ETHER2) {
        soc_reg_field_set(unit, VLAN_PROTOCOLr, &reg32, ETHERIIf, 1);
    }
    if (frame & BCM_PORT_FRAMETYPE_8023) {
        soc_reg_field_set(unit, VLAN_PROTOCOLr, &reg32, SNAPf, 1);
    }
    if (frame & BCM_PORT_FRAMETYPE_LLC) {
        soc_reg_field_set(unit, VLAN_PROTOCOLr, &reg32, LLCf, 1);
    }
    soc_reg_field_set(unit, VLAN_PROTOCOLr, &reg32, MATCHUPPERf, 1);
    soc_reg_field_set(unit, VLAN_PROTOCOLr, &reg32, MATCHLOWERf, 1);
    SOC_IF_ERROR_RETURN
        (WRITE_VLAN_PROTOCOLr(unit, port, empty, reg32));

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_er_port_protocol_vlan_delete
 * Purpose:
 *      Remove an already created proto protocol based vlan
 *      on a port.
 * Parameters:
 *      unit - (IN) BCM device number
 *      port - (IN) Port number
 *      frame - (IN) one of BCM_PORT_FRAMETYPE_{ETHER2,8023,LLC}
 *      ether - (IN) 16 bit Ethernet type field
 * Returns:
 *      BCM_E_XXX
 */

int
_bcm_er_port_protocol_vlan_delete(int unit,
                              bcm_port_t port,
                              bcm_port_frametype_t frame,
                              bcm_port_ethertype_t ether)
{
    int                          i;
    uint32                       reg32;
    bcm_port_frametype_t         ft;
    bcm_port_ethertype_t         et;
    int                          vp_index;
    /*
     * Search for the indicated protocol mapping entry.
     * Must not be an existing entry that matches requested protocol.
     */
    vp_index = -1;
    for (i = 0; i < SOC_REG_NUMELS(unit, VLAN_PROTOCOLr); i++) {
        SOC_IF_ERROR_RETURN
            (READ_VLAN_PROTOCOLr(unit, port, i, &reg32));
        ft = 0;
        if (soc_reg_field_get(unit, VLAN_PROTOCOLr, reg32, ETHERIIf)) {
            ft |= BCM_PORT_FRAMETYPE_ETHER2;
        }
        if (soc_reg_field_get(unit, VLAN_PROTOCOLr, reg32, SNAPf)) {
            ft |= BCM_PORT_FRAMETYPE_8023;
        }
        if (soc_reg_field_get(unit, VLAN_PROTOCOLr, reg32, LLCf)) {
            ft |= BCM_PORT_FRAMETYPE_LLC;
        }
        et = soc_reg_field_get(unit, VLAN_PROTOCOLr, reg32, ETHERTYPEf);
        if (ft == frame && et == ether) {
            vp_index = i;
            break;
        }
    }

    if (vp_index < 0) {
        return BCM_E_NOT_FOUND;
    }

    /*
     * Disable protocol mapping & clear its map data for target port.
     */
    SOC_IF_ERROR_RETURN
        (WRITE_VLAN_PROTOCOLr(unit, port, vp_index, 0));
    SOC_IF_ERROR_RETURN
        (WRITE_VLAN_PROTOCOL_DATAr(unit, port, vp_index, 0));

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_er_port_dtag_mode_set
 * Description:
 *      Set the double-tagging mode of a port.
 * Parameters:
 *      unit - (IN) BCM device number
 *      port - (IN) Port number
 *      mode - (IN) Double-tagging mode, one of:
 *              BCM_PORT_DTAG_MODE_NONE            No double tagging
 *              BCM_PORT_DTAG_MODE_INTERNAL        Service Provider port
 *              BCM_PORT_DTAG_MODE_EXTERNAL        Customer port
 *              BCM_PORT_DTAG_REMOVE_EXTERNAL_TAG  Remove customer tag
 *              BCM_PORT_DTAG_ADD_EXTERNAL_TAG     Add customer tag
 *      dt_mode - (IN) True/False double tagged port.
 *      ignore_tag - (IN) Ignore outer tag. 
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *      BCM_PORT_DTAG_MODE_INTERNAL is for service provider ports.
 *              A tag will be added if the packet does not already
 *              have the internal TPID (see bcm_port_tpid_set below).
 *              Internally this sets DT_MODE and clears IGNORE_TAG.
 *      BCM_PORT_DTAG_MODE_EXTERNAL is for customer ports.
 *              The service provider TPID will always be added
 *              (see bcm_port_tpid_set below).
 *              Internally this sets DT_MODE and sets IGNORE_TAG.
 */
int
_bcm_er_port_dtag_mode_set(int unit, bcm_port_t port, int mode, 
                           int dt_mode, int ignore_tag)
{
    uint32  regval, oldregval;

    SOC_IF_ERROR_RETURN(READ_IGR_CONFIGr(unit, &regval));
    oldregval = regval;
    soc_reg_field_set(unit, IGR_CONFIGr, &regval,
                      DT_MODEf, dt_mode);
    if (regval != oldregval) {
        SOC_IF_ERROR_RETURN(WRITE_IGR_CONFIGr(unit, regval));
    }

    SOC_IF_ERROR_RETURN(READ_SEER_CONFIGr(unit, &regval));
    oldregval = regval;
    soc_reg_field_set(unit, SEER_CONFIGr, &regval,
                      DT_MODEf, dt_mode);
    if (regval != oldregval) {
        SOC_IF_ERROR_RETURN(WRITE_SEER_CONFIGr(unit, regval));
    }

    SOC_IF_ERROR_RETURN(READ_IGR_PORTr(unit, port, &regval));
    oldregval = regval;
    soc_reg_field_set(unit, IGR_PORTr, &regval,
                      NNI_PORTf, !ignore_tag);
    if (regval != oldregval) {
        SOC_IF_ERROR_RETURN(WRITE_IGR_PORTr(unit, port, regval));
    }

    SOC_IF_ERROR_RETURN(READ_EGR_CONFIGr(unit, &regval));
    oldregval = regval;
    soc_reg_field_set(unit, EGR_CONFIGr, &regval, DT_MODEf, dt_mode);
    /* Record double tag mode for use in DCBs */
    SOC_DT_MODE(unit) = (dt_mode != 0);

    if (SOC_REG_FIELD_VALID(unit, EGR_CONFIGr, 
                            QINQ_PROXY_UNTAG_AS_TWO_TAGf) &&
        IS_HG_PORT(unit, port)) {
        /* For Q-in-Q proxy, untagged packets from UNI ports
         * can either be sent out NNI ports with outer + inner tags
         * or just outer tag.
         */
        soc_reg_field_set(unit, EGR_CONFIGr, &regval, 
                          QINQ_PROXY_UNTAG_AS_TWO_TAGf, 
                          (mode & BCM_PORT_DTAG_ADD_EXTERNAL_TAG) ? 1 : 0);
    }
    if (regval != oldregval) {
        SOC_IF_ERROR_RETURN(WRITE_EGR_CONFIGr(unit, regval));
    }

    SOC_IF_ERROR_RETURN(READ_EGR_PORTr(unit, port, &regval));
    oldregval = regval;
    soc_reg_field_set(unit, EGR_PORTr, &regval, NNIf, !ignore_tag); 
    if (regval != oldregval) {
        SOC_IF_ERROR_RETURN(WRITE_EGR_PORTr(unit, port, regval));
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_er_port_dtag_mode_get
 * Description:
 *      Return the current double-tagging mode of a port.
 * Parameters:
 *      unit - (IN) BCM device number
 *      port - (IN) Port number
 *      mode - (OUT) Double-tagging mode
 * Return Value:
 *      BCM_E_XXX
 */

int
_bcm_er_port_dtag_mode_get(int unit, bcm_port_t port, int *mode)
{
    uint32  config;
    uint32      igr_port;

    /* Input parameters check. */
    if (NULL == mode) {
        return (BCM_E_PARAM);
    }

    SOC_IF_ERROR_RETURN(READ_IGR_CONFIGr(unit, &config));
    if (soc_reg_field_get(unit, IGR_CONFIGr, config, DT_MODEf) == 0) {
        *mode = BCM_PORT_DTAG_MODE_NONE;
    } else {
        SOC_IF_ERROR_RETURN(READ_IGR_PORTr(unit, port, &igr_port));
        if (soc_reg_field_get(unit, IGR_PORTr, igr_port, NNI_PORTf)) {
            *mode = BCM_PORT_DTAG_MODE_INTERNAL;
        } else {
            *mode = BCM_PORT_DTAG_MODE_EXTERNAL;
        }
    }
    return BCM_E_NONE;
}
#else /*  BCM_EASYRIDER_SUPPORT */
int bcm_esw_easyrider_port_not_empty;
#endif  /* BCM_EASYRIDER_SUPPORT */
