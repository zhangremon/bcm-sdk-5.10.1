/*
 * $Id: eav.c 1.2 Broadcom SDK $
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
 * Ethernet AV BCM APIs
 */

#include <bcm/error.h>
#include <bcm/eav.h>
#include <soc/drv.h>
#include <soc/debug.h>


/*
 * Function:
 *      bcm_sbx_eav_init
 * Description:
 *      Initialize the Residential Ethernet module and enable
 *      the Ethernet AV (EAV) support.
 * Parameters:
 *      unit - device unit number.
 * Returns:
 *      BCM_E_XXX
 *      
 * Notes:
 *      1. This function will enable the global EAV functionality
 *      2. Decide the way to report egress timestamp info to CPU
 *         Either loopback reporting packets
 *         or CPU directly read register later.
 */
int 
bcm_sbx_eav_init(int unit)
{
    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }
    
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_sbx_eav_port_enable_get
 * Description:
 *      Get enable status of per port Ethernet AV functionality
 * Parameters:
 *      unit - device unit number.
 *      port - port number
 *      enable - (OUT) TRUE, port is enabled for Ethernet AV
 *                     FALSE, port is disabled for Ethernet AV
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_eav_port_enable_get(int unit,
                            bcm_port_t port,
                            int *enable)
{
    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }
    if (!SOC_PORT_VALID(unit, port)) { 
        return BCM_E_PORT; 
    }
    
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_sbx_eav_port_enable_set
 * Description:
 *      Enable or disable per port Ethernet AV functionality
 * Parameters:
 *      unit - device unit number.
 *      port - port number
 *      enable - TRUE, port is enabled for Ethernet AV
 *               FALSE, port is disabled for Ethernet AV
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 *
 * Notes:
 *      Need to disable the per port flow control
 */
int 
bcm_sbx_eav_port_enable_set(int unit,
                            bcm_port_t port,
                            int enable)
{
    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }
    if (!SOC_PORT_VALID(unit, port)) { 
        return BCM_E_PORT; 
    }
    
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_sbx_eav_link_status_get
 * Description:
 *      Get link status of per port Ethernet AV functionality
 * Parameters:
 *      unit - device unit number.
 *      port - port number
 *      link - (OUT) TRUE, Ethernet AV led is light on
 *                     FALSE, Ethernet AV led is light off
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_eav_link_status_get(int unit,
                            bcm_port_t port,
                            int *link)
{
    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }
    if (!SOC_PORT_VALID(unit, port)) { 
        return BCM_E_PORT; 
    }
    
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_sbx_eav_link_status_set
 * Description:
 *      Set the EAV link status of  per port Ethernet AV functionality
 * Parameters:
 *      unit - device unit number.
 *      port - port number
 *      link - TRUE, Ethernet AV led is light on
 *               FALSE, Ethernet AV led is light off
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 *
 * Notes:
 */
int 
bcm_sbx_eav_link_status_set(int unit,
                            bcm_port_t port,
                            int link)
{
    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }
    if (!SOC_PORT_VALID(unit, port)) { 
        return BCM_E_PORT; 
    }
    
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_sbx_eav_control_get
 * Description:
 *      Get the configuration for specific type
 * Parameters:
 *      unit - device unit number.
 *      type - configuration type
 *      arg1 - (OUT) the pointer buffer to store the returned configuration
 *      arg2 - (OUT) the pointer buffer to store the returned configuration
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_eav_control_get(int unit,
                        bcm_eav_control_t type, 
                        uint32 *arg1,
                        uint32 *arg2)
{
    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_sbx_eav_control_set
 * Description:
 *      Set the configuration for specific type
 * Parameters:
 *      unit - device unit number.
 *      type - configuration type
 *      arg1 - the configuration data to set 
 *      arg2 - the configuration data to set 
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_eav_control_set(int unit,
                        bcm_eav_control_t type,
                        uint32 arg1,
                        uint32 arg2)
{
    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }
    
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_sbx_eav_egress_timestamp_get
 * Description:
 *      Get the per port egress timestamp value
 * Parameters:
 *      unit - device unit number
 *      port - port number
 *      timestamp - (OUT) the pointer buffer to store the returned timestamp  
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_eav_timestamp_get(int unit,
                          bcm_port_t port,
                          uint32 *timestamp)
{
    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }
    if (!SOC_PORT_VALID(unit, port)) { 
        return BCM_E_PORT; 
    }

    return BCM_E_UNAVAIL;
}


/*
 * Function:
 *      bcm_sbx_eav_timesync_mac_get
 * Description:
 *      Get the Mac address of Time Sync protocol
 * Parameters:
 *      unit - device unit number
 *      eav_mac - the pointer buffer to restorm the mac addrss  
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_eav_timesync_mac_get(int unit,
                             bcm_mac_t eav_mac)
{
    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_sbx_eav_timesync_mac_set
 * Description:
 *      Set the Mac address of Time Sync protocol
 * Parameters:
 *      unit - device unit number
 *      eav_mac - the pointer buffer to restorm the mac addrss  
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_eav_timesync_mac_set(int unit,
                             bcm_mac_t eav_mac)
{
    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_eav_srp_mac_ethertype_set
 * Description:
 *      Get the Mac address and Ethertype used to trap SRP protocol packets
 * Parameters:
 *      unit - device unit number
 *      mac  - the mac addrss   
 *      ethertype - the EtherType
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_sbx_eav_srp_mac_ethertype_set(int unit,
                                  bcm_mac_t mac,
                                  uint16 ethertype)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_eav_srp_mac_ethertype_get
 * Description:
 *      Get the Mac address and Ethertype used to trap SRP protocol packets
 * Parameters:
 *      unit - device unit number
 *      mac  - (OUT)the mac addrss   
 *      ethertype - (OUT)the EtherType
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 * Notes:
 */                               
int 
bcm_sbx_eav_srp_mac_ethertype_get(int unit,
                                  bcm_mac_t mac,
                                  uint16 *ethertype)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_eav_pcp_mapping_set
 * Description:
 *      Set the PCP(priority) value mapping for each EAV class packets
 * Parameters:
 *      unit - device unit number
 *      type - Class A or Class B stream
 *      pcp  - Priority for the Class   
 *      rempapped_pcp - For NonEAV traffic with PCP=ClassA_PCP or ClassB_PCP
 *      exiting through an egress port configured in EAV mode must be remapped
 *      to another pcp.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_sbx_eav_pcp_mapping_set(int unit, 
                            bcm_eav_stream_class_t type, 
                            int pcp, 
                            int remapped_pcp)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_sbx_eav_pcp_mapping_get
 * Description:
 *      Set the PCP(priority) value mapping for each EAV class packets
 * Parameters:
 *      unit - device unit number
 *      type - Class A or Class B stream
 *      pcp  - Priority for the Class   
 *      rempapped_pcp - For NonEAV traffic with PCP=ClassA_PCP or ClassB_PCP
 *      exiting through an egress port configured in EAV mode must be remapped
 *      to another pcp.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_sbx_eav_pcp_mapping_get(int unit, 
                            bcm_eav_stream_class_t type, 
                            int *pcp, 
                            int *remapped_pcp)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_sbx_eav_bandwidth_set
 * Description:
 *      Set the reserved bandwidth for Class A or B stream traffic
 * Parameters:
 *      unit - device unit number
 *      port - port number   
 *      type - Class A or Class B stream
 *      bytes_sec - maximum bytes per second.
 *      bytes_burst - maximum burst size in bytes. 
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_sbx_eav_bandwidth_set(int unit, 
                          bcm_port_t port, 
                          bcm_eav_stream_class_t type, 
                          uint32 bytes_sec,
                          uint32 bytes_burst)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_sbx_eav_bandwidth_get
 * Description:
 *      Get the reserved bandwidth for Class A or B stream traffic
 * Parameters:
 *      unit - device unit number
 *      port - port number   
 *      type - Class A or Class B stream
 *      bytes_sec - bytes per second.
 *      bytes_burst - max burst bytes.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_sbx_eav_bandwidth_get(int unit, 
                          bcm_port_t port, 
                          bcm_eav_stream_class_t type, 
                          uint32 *bytes_sec,
                          uint32 *bytes_burst)
{
    return BCM_E_UNAVAIL;
}
