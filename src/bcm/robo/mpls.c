/*
 * $Id: mpls.c 1.13 Broadcom SDK $
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
 * File:    mpls.c
 * Purpose: Manages MPLS functions
 */

#ifdef INCLUDE_L3


#include <bcm/error.h>
#include <bcm/types.h>
#include <bcm/l3.h>
#include <bcm/mpls.h>

/*
 * Function:
 *      bcm_mpls_init
 * Purpose:
 *      Initialize the MPLS software module, clear all HW MPLS states
 * Parameters:
 *      unit     - Device Number
 * Returns:
 *      BCM_E_XXXX
 */

int
bcm_robo_mpls_init(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_mpls_cleanup(int unit)
{
    return BCM_E_UNAVAIL;
}
/*
 * Function:
 *      bcm_mpls_info
 * Purpose:
 *      Get the HW MPLS table information such as table size etc
 * Parameters:
 *      unit - Device Number
 *      info - (OUT) table size and used info
 * Returns:
 *      BCM_E_XXXX
 */

int
bcm_robo_mpls_info(int unit, bcm_mpls_info_t *info)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_station_add
 * Purpose:
 *      Add a (MAC, VLAN) for tunnel/MPLS processing, frames
 *      destined to (MAC, VLAN) is subjected to TUNNEL/MPLS processing.
 * Parameters:
 *      unit     - Device Number
 *      mac_addr - MAC address of tunnel/MPLS header
 *      vlan     - VLAN ID of tunnel/MPLS header
 * Returns:
 *      BCM_E_XXXX
 */

int
bcm_robo_station_add (int unit, bcm_mac_t mac_addr, bcm_vlan_t vlan)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_station_delete
 * Purpose:
 *      Delete a (MAC, VLAN) to stop tunnel/MPLS processing
 * Parameters:
 *      unit     - Device Number
 *      mac_addr - MAC address of tunnel/MPLS header
 *      vlan     - VLAN ID of tunnel/MPLS header
 * Returns:
 *      BCM_E_XXXX
 */

int
bcm_robo_station_delete (int unit, bcm_mac_t mac_addr, bcm_vlan_t vlan)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_station_delete_all
 * Purpose:
 *      Delete all tunnel/MPLS (MAC, VLAN)s
 * Parameters:
 *      unit     - Device Number
 * Returns:
 *      BCM_E_XXXX
 */

int
bcm_robo_station_delete_all (int unit)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_mpls_vpn_create
 * Purpose:
 *      Create a VPN instance (no hardware write)
 * Parameters:
 *      unit   - Device Number
 *      vpn - VPN instance
 *      flags  - VPN property (BCM_MPLS_L2_VPN or BCM_MPLS_L3_VPN)
 * Returns:
 *      BCM_E_XXXX
 * Note:
 *      No hardware action
 */

int
bcm_robo_mpls_vpn_create (int unit, bcm_vpn_t vpn, uint32 flags)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_mpls_vpn_destroy
 * Purpose:
 *      Delete a VPN instance
 * Parameters:
 *      unit - Device Number
 *      vpn - VPN instance ID
 * Returns:
 *      BCM_E_XXXX
 * Note:
 *      This aumatically unbinds all interfaces to this VPN
 */

int
bcm_robo_mpls_vpn_destroy (int unit, bcm_vpn_t vpn)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_mpls_vpn_add (bind)
 * Purpose:
 *      Bind interface/port/VLAN to VPN instance
 * Parameters:
 *      unit - Device Number
 *      vpn - VPN instance ID
 *      vpn_info - interface/port/VLAN
 * Returns:
 *      BCM_E_XXXX
 * Note:
 *      This port/VID binding of customer ports to VPN
 *      instance is normally done as a result of admin configuration.
 */

int
bcm_robo_mpls_vpn_add (int unit, bcm_vpn_t vpn, bcm_mpls_vpn_t *vpn_info)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_mpls_vpn_delete (unbind)
 * Purpose:
 *      Unbind interface/port/VLAN to VPN instance
 * Parameters:
 *      unit - Device Number
 *      vpn - VPN instance ID
 *      key - interface/port/VLAN
 * Returns:
 *      BCM_E_XXXX
 */

int
bcm_robo_mpls_vpn_delete (int unit, bcm_vpn_t vpn, bcm_mpls_vpn_t *key)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_mpls_vpn_delete_all
 * Purpose:
 *      Unbind all interface/port/VLAN to the VPN instance
 * Parameters:
 *      unit - Device Number
 *      vpn - VPN instance ID
 * Returns:
 *      BCM_E_XXXX
 */

int
bcm_robo_mpls_vpn_delete_all (int unit, bcm_vpn_t vpn)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_mpls_vpn_get
 * Purpose:
 *      Get the binding of interface/port/VLAN to VPN instance
 * Parameters:
 *      unit       - Device Number
 *      vpn     - VPN instance ID
 *      max_out    - max elements of the array
 *      vpn_info   - (OUT) array of interface/port/VLAN struct
 *      real_count - (OUT) actual counts
 * Returns:
 *      BCM_E_XXXX
 * Note:
 *      This port/VID binding of customer ports to VPN
 *      instance is normally done as a result of admin configuration.
 */

int
bcm_robo_mpls_vpn_get (int unit, bcm_vpn_t vpn, int max_out,
                  bcm_mpls_vpn_t *vpn_info, int *real_count)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_mpls_vpn_circuit_add
 * Purpose:
 *      Add (Bind) a VC to existing VPN
 * Parameters:
 *      unit     - Device Number
 *      vpn      - The VPN to bind VC to
 *      vc_info  - Virtual circuit info
 * Returns:
 *      BCM_E_XXXX
 * Note:
 *      Assign L2 MPLS label and L2 outer header info to L2 MPLS pseudo-wire
 *      For Point-to-Multipoint connection, this function is called
 *      multiple times.
 */

int
bcm_robo_mpls_vpn_circuit_add (int unit, bcm_vpn_t vpn,
                          bcm_mpls_circuit_t *vc_info)
{
    return BCM_E_UNAVAIL;
}

/*      
 * Function:
 *      bcm_mpls_vpn_circuit_delete
 * Purpose:
 *      Delete (unbind) a VC from existing VPN
 * Parameters:
 *      unit     - Device Number
 *      vpn      - The VPN to bind VC to
 *      vc_info  - Virtual circuit info
 * Returns:
 *      BCM_E_XXXX
 * Note:
 *      Delete L2 MPLS label and L2 outer header info to L2 MPLS pseudo-wire
 *      For Point-to-Multipoint connection, this function is called
 *      multiple times.
 */

int
bcm_robo_mpls_vpn_circuit_delete (int unit, bcm_vpn_t vpn,
                             bcm_mpls_circuit_t *vc_info)
{
    return BCM_E_UNAVAIL;
}

/*     
 * Function:
 *      bcm_mpls_vpn_circuit_delete_all
 * Purpose:
 *      Delete (unbind) all VCs from existing VPN
 * Parameters:
 *      unit     - Device Number
 *      vpn      - The VPN to bind VC to
 * Returns:
 *      BCM_E_XXXX
 */

int
bcm_robo_mpls_vpn_circuit_delete_all (int unit, bcm_vpn_t vpn)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_mpls_vpn_circuit_get
 * Purpose:
 *      Get the VC label and header info for the VPN
 * Parameters:   
 *      unit       - Device Number
 *      vpn     - The VPN to bind VC to
 *      vc_array    - max elements of the array
 *      vc_info    - (OUT) Virtual circuit info
 *      vc_count - (OUT) actual counts
 * Returns:
 *      BCM_E_XXXX
 * Note:
 *      The current ER implementation only supports Point-to-Point
 *      pseudo-wire, so one VC per VPN.
 */

int
bcm_robo_mpls_vpn_circuit_get (int unit, bcm_vpn_t vpn, int vc_array,
                          bcm_mpls_circuit_t *vc_info, int *vc_count)
{
    return BCM_E_UNAVAIL;
}

/*  
 * Function:
 *      bcm_mpls_l3_initiator_set
 * Purpose:
 *      Set the FEC to MPLS label mapping
 * Parameters:
 *      unit - Device Number
 *      intf - The egress L3 interface
 *      mpls_label - MPLS label and header information
 * Returns:
 *      BCM_E_XXXX
 */

int
bcm_robo_mpls_l3_initiator_set (int unit, bcm_l3_intf_t *intf,
                           bcm_mpls_l3_initiator_t *mpls_label)
{
    return BCM_E_UNAVAIL;
}

/*  
 * Function:
 *      bcm_mpls_l3_initiator_clear
 * Purpose:
 *      Clear the FEC to MPLS label mapping
 * Parameters:
 *      unit - Device Number
 *      intf - The egress L3 interface
 * Returns: 
 *      BCM_E_XXXX
 */

int
bcm_robo_mpls_l3_initiator_clear (int unit, bcm_l3_intf_t *intf)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_mpls_l3_initiator_get
 * Purpose:
 *      Get the FEC to MPLS label mapping info
 * Parameters:
 *      unit - Device Number
 *      intf - The egress L3 interface
 *      mpls_label - MPLS header information
 * Returns:
 *      BCM_E_XXXX
 */

int
bcm_robo_mpls_l3_initiator_get (int unit, bcm_l3_intf_t *intf,
                           bcm_mpls_l3_initiator_t *mpls_label)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_mpls_l3_initiator_clear_all
 * Purpose:
 *      Clear all the FEC to MPLS label mapping
 * Parameters:
 *      unit - Device Number
 * Returns:
 *      BCM_E_XXXX
 */

int
bcm_robo_mpls_l3_initiator_clear_all (int unit)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_mpls_switch_add
 * Purpose:
 *      Add MPLS label swapping, label popping, and packet forwarding
 *      information.  This will set up the LPM table (MPLS view) to:
 *   1) Switch MPLS labeled packets at LSR (action codes 0, 1, 2, 3, 4, 8)
 *   2) L2 MPLS payload forwarding at the VC termination point
 *      to properly forward the "original" L2 payload when VC
 *      terminates (action code 5 or 6)
 *   3) Add VRF information for L3 MPLS payload (IP packet)
 *      forwarding at the LSP termination point.  This is used
 *      to properly forward the "original" IP payload at the
 *      LSP edge LSR based on the original payload IP header
 *      plus the VRF info. (action code 7)
 * Parameters:
 *      unit - Device Number
 *      switch_info - (IN) L3 MPLS label switching and packet forwarding info
 *                      src_label1     \
 *                      src_label2      \   Key
 *                      src_port        /
 *                      src_mod        /
 *                      src_trunk     /
 *                      action
 *                      src_l3_intf    \
 *                      next_label      \
 *                      next_vlan        \  LSR case
 *                      next_mac         /
 *                      next_modid      /
 *                      next_port      /
 *                      next_trunk    /
 *                      vpn           -     L2/L3 payload forwarding
 *                      dst_port      \
 *                      dst_modid      \    L2 payload forwarding
 *                      dst_vid        /
 *                      dst_trunk     /
 * Returns:
 *      BCM_E_XXXX
 */

int
bcm_robo_mpls_switch_add (int unit, bcm_mpls_switch_t *switch_info)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_mpls_switch_delete
 * Purpose:
 *      Delete MPLS label swapping and packet forwarding information
 *   1) label swapping, popping at LSR
 *   2) Ethernet MPLS payload forwarding at the VC termination point
 *   3) L3 MPLS payload (IP packet) forwarding at the LSP termination point
 * Parameters:
 *      unit - Device Number
 *      switch_info - (IN) L3 MPLS label switching and packet forwarding info
 * Returns:
 *      BCM_E_XXXX
 */

int
bcm_robo_mpls_switch_delete (int unit, bcm_mpls_switch_t *switch_info)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_mpls_switch_get
 * Purpose:
 *      Get MPLS label swapping and packet forwarding information
 *   1) label swapping, popping at LSR
 *   2) Ethernet MPLS payload forwarding at the VC termination point
 *   3) L3 MPLS payload (IP packet) forwarding at the LSP termination point
 * Parameters:
 *      unit - Device Number
 *      switch_info - (IN) L3 MPLS label switching and packet forwarding info
 * Returns:
 *      BCM_E_XXXX
 */

int
bcm_robo_mpls_switch_get (int unit, bcm_mpls_switch_t *switch_info)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_mpls_switch_delete_all
 * Purpose:
 *      Delete all MPLS label swapping and packet forwarding information
 * Parameters: 
 *      unit - Device Number 
 *      flag - BCM_MPLS_LSR       delete LSR action entries
 *             BCM_MPLS_L3_VPN    delete L2 VC termination action entries
 *             BCM_MPLS_L2_VPN    delete L3 LSP termination action entries
 * Returns:
 *      BCM_E_XXXX
 */

int
bcm_robo_mpls_switch_delete_all (int unit, int flag)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_mpls_switch_traverse(int unit, uint32 flags, 
          uint32 start, uint32 end,
          bcm_mpls_switch_traverse_cb trav_fn, void *user_data)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_mpls_exp_map_create(int unit, int *exp_map_id)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_mpls_exp_map_destroy(int unit, int exp_map_id)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_mpls_exp_map_set(int unit, int exp_map_id,
                         bcm_mpls_exp_map_t *exp_map)
{
    return BCM_E_UNAVAIL;
}

int     
bcm_robo_mpls_exp_map_get(int unit, int exp_map_id,
                         bcm_mpls_exp_map_t *exp_map)
{
    return BCM_E_UNAVAIL;
}

#endif  /* INCLUDE_L3 */
