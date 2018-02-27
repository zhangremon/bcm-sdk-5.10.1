/*
 * $Id: management.c 1.1.2.15 Broadcom SDK $
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
#if defined(INCLUDE_PTP)

#ifdef BCM_HIDE_DISPATCHABLE
#undef BCM_HIDE_DISPATCHABLE
#endif

#include <soc/defs.h>
#include <soc/drv.h>

#include <bcm/pkt.h>
#include <bcm/tx.h>
#include <bcm/rx.h>
#include <bcm/error.h>
#include <bcm/ptp.h>
#include <bcm_int/esw/ptp.h>

#include <bcm_int/esw/rx.h>
#include <bcm_int/esw/tx.h>
#include <bcm_int/esw_dispatch.h>

#if defined(BCM_KATANA_SUPPORT)
#include <bcm_int/esw/katana.h>
#include <soc/uc_msg.h>
#endif

/* Parameters and constants. */
#define PTP_MGMT_RESPONSE_TIMEOUT_US         (10000000)  /* 1 sec */

#define PTP_MGMT_RESP_ACTION_OFFSET          (46)
#define PTP_MGMT_RESP_TLV_TYPE_OFFSET        (48)
#define PTP_MGMT_RESP_TLV_LEN_OFFSET         (50)
#define PTP_MGMT_RESP_TLV_VAL_OFFSET         (52)
#define PTP_MGMT_RESP_PAYLOAD_OFFSET         (54)

static const uint8 l2hdr_default[] = 
{
    /* Destination MAC address (zeros placeholder). */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* Source MAC address (zeros placeholder). */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
};

static const uint8 vlanhdr_default[] = 
{
    /* Tag protocol identifier (TPID). */ 
    0x81, 0x00,
    /*
     * Priority code point (PCP), 
     * canonical form indicator (CFI), 
     * VLAN identifier (VID) .
     */
    0xe0, 0x00, 
};

static const uint8 ethhdr_default[] =
{
    /* Ethertype (0x0800 = IPv4, 0x86dd = IPv6). */
    0x08, 0x00,                          
};

static const uint8 ipv4hdr_default[] = 
{
    /* Version (IPv4), header length. */
    0x45,                               
    /*
     * Differential service code point (DSCP), 
     * Explicit congestion notification (ECN). 
     */
    0x00,                              
    /* 
     * Total length (octets). 
     * NOTICE: Placeholder value. 
     */
    0x00, 0x52,
    /* Identification. */                                 
    0x00, 0x00,
    /* Flags (Do not fragment), Fragment offset. */
    0x40, 0x00,
    /* Time to live (TTL). */
    0x01,
    /* Protocol (UDP). */
    0x11,
    /* Header checksum (zeros placeholder). */
    0x00, 0x00,                         
    /* Source IPv4 address (zeros placeholder). */
    0x00, 0x00, 0x00, 0x00,
    /* Destination IPv4 address (zeros placeholder). */
    0x00, 0x00, 0x00, 0x00,             
};

static const uint8 udphdr_default[] =
{
    /* Source port. */
    0x01, 0x40,
    /* Destination port. */
    0x01, 0x40,                        
    /* 
     * Length (octets). 
     * NOTICE: Placeholder value. 
     */
    0x00, 0x00,                         
    /* 
     * Checksum (optional). 
     * NOTICE: 0x0000 indicates checksum skipped.
     */                           
    0x00, 0x00,                         
};

static const uint8 udphdr_tunnel[] =
{
    /* Source port. */
    0x01, 0x41,                         
    /* Destination port. */
    0x01, 0x41,                       
    /* Length (zeros placeholder). */
    0x00, 0x00,
    /* Checksum (optional). */
    0x00, 0x00,
};

static const uint8 ptphdr_mgmt[] = 
{
    /* 
     * Message ID (management message).
     * Ref. IEEE Std. 1588-2008, Sect. 13.3.2.2. 
     */
    _bcmPTPmessageType_management,                  
    /* PTP version. */
    0x02,             
    /* 
     * PTP message length, TLV "V" part only. 
     * NOTICE: Placeholder value. 
     */
    0x00, 0x00,
    /* PTP domain (zero default). */
    0x00, 0x00,
    /* PTP flags  (Unicast= true). */
    0x04, 0x00,
    /* 
     * Correction field. 
     * Ref. IEEE Std. 1588-2008, Chapter 13.3.2.7.
     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    /* Reserved. */
    0x00, 0x00, 0x00, 0x00,                         
    /* Source port ID. */
    0x00, 0x10, 0x94, 0xff, 0xfe, 0x00, 0x00, 0x0d, 
    0x00, 0x01,
    /* Sequence ID. */
    0x00, 0x01,
    /* 
     * Control field (management).
     * Ref. IEEE Std. 1588-2008, Sect. 13.3.2.10. 
     */
    0x04,
    /* 
     * Log inter-message interval.
     * Ref. IEEE Std. 1588-2008, Sect. 13.3.2.11. 
     */
    0x02,
};

static const uint8 mgmthdr_mgmt[]= 
{
    /* Target clock ID. */
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    /* Target port ID. */
    0xff, 0xff,                                     
    /* Starting boundary hops. */
    0x01,
    /* Boundary hops. */
    0x01,
    /* Action field. */
    0x00,
    /* Reserved. */
    0x00,                                           
};

static const uint8 tlvhdr_mgmt[]=
{
    /* TLV type (0x0001 = Management message). */
    0x00, 0x01,                      
    /* 
     * TLV length (octets).
     * NOTICE: (0x0002 = TLV prefix, no payload placeholder). 
     */
    0x00, 0x02,
     /* TLV value prefix (management ID, i.e. command definition). */
    0x00, 0x00,                                    
};

static const uint8 ptphdr_ucsig[] = 
{
    /* 
     * Message ID (signaling message).
     * Ref. IEEE Std. 1588-2008, Sect. 13.3.2.2. 
     */
    _bcmPTPmessageType_signaling,                  
    /* PTP version. */
    0x02,             
    /* 
     * PTP message length, TLV "V" part only. 
     * NOTICE: Placeholder based on REQUEST_UNICAST_TRANSMISSION TLV. 
     */
    0x00, (PTP_UCSIG_REQUEST_TOTAL_SIZE_OCTETS - PTP_SIGHDR_START_IDX),
    /* PTP domain (zero default). */
    0x00, 0x00,
    /* PTP flags  (Unicast= true). */
    0x04, 0x00,
    /* 
     * Correction field. 
     * Ref. IEEE Std. 1588-2008, Chapter 13.3.2.7.
     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    /* Reserved. */
    0x00, 0x00, 0x00, 0x00,                         
    /* Source port ID. */
    0x00, 0x10, 0x94, 0xff, 0xfe, 0x00, 0x00, 0x0d, 
    0x00, 0x01,
    /* Sequence ID. */
    0x00, 0x02,
    /* 
     * Control field (all others).
     * Ref. IEEE Std. 1588-2008, Sect. 13.3.2.10. 
     */
    0x05,
    /* 
     * Log inter-message interval.
     * Ref. IEEE Std. 1588-2008, Sect. 13.3.2.11. 
     */
    0x7f,
};

static const uint8 tgtport_ucsig[] =
{
    /* Signaling message target clock identity. */
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    /* Signaling message target port. */      
    0xff, 0xff,
};

static const uint8 unicast_request_tlv_ucsig[] = 
{
    /* 
     * TLV type. (0x0004 = REQUEST_UNICAST_TRANSMISSION placeholder). 
     * Ref. IEEE Std. 1588-2008, Sect. 14.1.1.
     */
    0x00, 0x04,                                     
    /* 
     * TLV length (octets). 
     * NOTICE: Placeholder based on REQUEST_UNICAST_TRANSMISSION TLV.
     */
    0x00, (PTP_UCSIG_REQUEST_TLV_SIZE_OCTETS - 4),
    /* 
     * TLV value (data). Message type (0x0b = Announce message, placeholder). 
     * Ref. IEEE Std. 1588-2008, Sect. 13.3.2.2.
     */
    _bcmPTPmessageType_announce,
    /* Interval. */
    0x00,
    /* Duration. */
    0x00, 0x00, 0x00, 0x14,
    /* Address type. */
    bcmPTPUDPIPv4,
    /* Peer address. */
    0x01, 0x02, 0x03, 0x04,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    /* L2 header length (zeros placeholder). */
    0x00,
    /* L2 header (zeros placeholder). */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

/* Clock PTP management data. */
typedef struct _bcm_ptp_clock_mgmt_data_s {
    uint8 domain; /* PTP domain. */
} _bcm_ptp_clock_mgmt_data_t;

/* Stack PTP management data array. */
typedef struct _bcm_ptp_stack_mgmt_array_s {
    _bcm_ptp_memstate_t memstate;
    
    bcm_mac_t src_mac;
    bcm_mac_t dest_mac;
    
    bcm_ip_t src_ip;
    bcm_ip_t dest_ip;
    
    uint16 vlan;
    uint8 prio;

    sal_sem_t mgmt_available;
    
    bcm_pkt_t *mgmt_packet;
    bcm_pkt_t *tunnel_packet;
    bcm_pkt_t *ucsig_packet;
    
    _bcm_ptp_clock_mgmt_data_t *clock_data;
} _bcm_ptp_stack_mgmt_array_t;

/* Unit PTP management data array(s). */
typedef struct _bcm_ptp_unit_mgmt_array_s {
    _bcm_ptp_memstate_t memstate;
    _bcm_ptp_stack_mgmt_array_t *stack_array;
} _bcm_ptp_unit_mgmt_array_t;

static const _bcm_ptp_clock_mgmt_data_t mgmt_default = { 0 };

static _bcm_ptp_unit_mgmt_array_t unit_mgmt_array[BCM_MAX_NUM_UNITS];

static int _bcm_ptp_management_message_create(
    uint8 *message);

/* static int _bcm_ptp_management_message_make( */
/*     uint8 *message,  */
/*     uint8 action,  */
/*     uint16 cmd,  */
/*     uint8 *payload,  */
/*     int payload_len); */

/* static int _bcm_ptp_management_message_header_metadata_set( */
/*     uint8 *message); */

static int _bcm_ptp_management_message_macaddr_set(
    uint8 *message, 
    bcm_mac_t *src_mac, 
    bcm_mac_t *dest_mac);

static int _bcm_ptp_management_message_vlantag_set(
    uint8 *message, 
    uint16 vlan_tag, 
    uint8 prio);

static int _bcm_ptp_management_message_ipaddr_set(
    uint8 *message, 
    bcm_ip_t src_ip, 
    bcm_ip_t dest_ip);

static int _bcm_ptp_management_message_ipv4hdr_checksum_set(
    uint8 *message);

static int _bcm_ptp_management_message_port_set(
    uint8 *message,
    uint16 src_port,
    uint16 dest_port);

/* static int _bcm_ptp_management_message_target_set( */
/*     uint8 *message,  */
/*     const bcm_ptp_clock_identity_t clock_identity,  */
/*     uint16 port_num); */

/* static int _bcm_ptp_management_message_length_get( */
/*     uint8 *message); */

#if 0 /* Unused. */
static int _bcm_ptp_management_message_domain_get(
    uint8 *message,
    uint8 *domain);
#endif /* Unused. */

static int _bcm_ptp_management_message_domain_set(
    uint8 *message,
    uint8 domain);

static uint16 _bcm_ptp_management_rfc791_checksum(
    uint8* packet, 
    int packet_len);

/*
 * Function:
 *      _bcm_ptp_management_init
 * Purpose:
 *      Initialize the PTP management framework of a unit.
 * Parameters:
 *      unit - (IN) Unit number.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
_bcm_ptp_management_init(
    int unit)
{
    int rv = BCM_E_UNAVAIL;
     
    _bcm_ptp_stack_mgmt_array_t *stack_p;
    
    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, PTP_STACK_ID_DEFAULT, 
            PTP_CLOCK_NUMBER_DEFAULT, PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        return rv;   
    }
            
    /* TODO: Fully implement logic to handle repeated initialization. */
    stack_p = sal_alloc(PTP_MAX_STACKS_PER_UNIT*
                        sizeof(_bcm_ptp_stack_mgmt_array_t),"Unit MGMT arrays");

    if (!stack_p) {
        unit_mgmt_array[unit].memstate = PTP_MEMSTATE_FAILURE;
        return BCM_E_MEMORY;
    }

    unit_mgmt_array[unit].stack_array = stack_p;
    unit_mgmt_array[unit].memstate = PTP_MEMSTATE_INITIALIZED;
    
    return rv;
}

/*
 * Function:
 *      _bcm_ptp_management_stack_create
 * Purpose:
 *      Create the PTP management data array of a PTP stack
 * Parameters:
 *      unit       - (IN) Unit number.
 *      ptp_id     - (IN) PTP stack ID.
 *      src_mac    - (IN) Source MAC address.
 *      dest_mac   - (IN) Destination (PTP stack) MAC address.
 *      src_ip     - (IN) Source IPv4 address.
 *      dest_ip    - (IN) Destination (PTP stack) IPv4 address.
 *      vlan       - (IN) VLAN.
 *      prio       - (IN) VLAN priority.
 *      top_bitmap - (IN) ToP bitmap.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
_bcm_ptp_management_stack_create(
    int unit, 
    bcm_ptp_stack_id_t ptp_id,
    bcm_mac_t *src_mac,
    bcm_mac_t *dest_mac,
    bcm_ip_t src_ip,
    bcm_ip_t dest_ip,
    uint16 vlan,
    uint8 prio,
    bcm_pbmp_t top_bitmap)
{
    int rv = BCM_E_UNAVAIL;
    _bcm_ptp_info_t *ptp_info_p;
    _bcm_ptp_stack_info_t *stack_p;
     
    _bcm_ptp_clock_mgmt_data_t *data_p;
    
    SET_PTP_INFO;

    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, ptp_id, 
            PTP_CLOCK_NUMBER_DEFAULT, PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        return rv;   
    }
    
    if (unit_mgmt_array[unit].memstate != PTP_MEMSTATE_INITIALIZED) {
        return BCM_E_UNAVAIL;
    }

    stack_p = &ptp_info_p->stack_info[ptp_id];
        
    /* TODO: Fully implement logic to handle repeated creation. */
    data_p = sal_alloc(PTP_MAX_CLOCK_INSTANCES*
                       sizeof(_bcm_ptp_clock_mgmt_data_t),
                       "PTP stack MGMT array");

    if (!data_p) {
        unit_mgmt_array[unit].stack_array[ptp_id].memstate = 
            PTP_MEMSTATE_FAILURE;
        return BCM_E_MEMORY;
    }

    unit_mgmt_array[unit].stack_array[ptp_id].clock_data = data_p;
    unit_mgmt_array[unit].stack_array[ptp_id].memstate = 
        PTP_MEMSTATE_INITIALIZED;

    unit_mgmt_array[unit].stack_array[ptp_id].mgmt_available = 
        sal_sem_create("BCM PTP mgmt", sal_sem_BINARY, 0);
    sal_sem_give(unit_mgmt_array[unit].stack_array[ptp_id].mgmt_available);

    rv = stack_p->transport_init(unit, ptp_id, src_mac, dest_mac, src_ip, dest_ip, vlan, prio, top_bitmap);

    return rv;
}



/*
 * Function:
 *      _bcm_ptp_external_transport_init
 * Purpose:
 *      Initialize message transport to an external ToP
 * Parameters:
 *      unit       - (IN) Unit number.
 *      ptp_id     - (IN) PTP stack ID.
 *      src_mac    - (IN) Source MAC address.
 *      dest_mac   - (IN) Destination (PTP stack) MAC address.
 *      src_ip     - (IN) Source IPv4 address.
 *      dest_ip    - (IN) Destination (PTP stack) IPv4 address.
 *      vlan       - (IN) VLAN.
 *      prio       - (IN) VLAN priority.
 *      top_bitmap - (IN) ToP bitmap.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int _bcm_ptp_external_transport_init(
    int unit, 
    bcm_ptp_stack_id_t ptp_id,
    bcm_mac_t *src_mac,
    bcm_mac_t *dest_mac,
    bcm_ip_t src_ip,
    bcm_ip_t dest_ip,
    uint16 vlan,
    uint8 prio,
    bcm_pbmp_t top_bitmap)
{
    bcm_pkt_t *mgmt_packet;
    bcm_pkt_t *tunnel_packet;
    bcm_pkt_t *ucsig_packet;
    int i = 0;
    int rv = BCM_E_UNAVAIL;

    sal_memcpy(unit_mgmt_array[unit].stack_array[ptp_id].src_mac, src_mac, 
               sizeof(bcm_mac_t));

    sal_memcpy(unit_mgmt_array[unit].stack_array[ptp_id].dest_mac, dest_mac, 
               sizeof(bcm_mac_t));

    unit_mgmt_array[unit].stack_array[ptp_id].src_ip = src_ip;
    unit_mgmt_array[unit].stack_array[ptp_id].dest_ip = dest_ip;

    unit_mgmt_array[unit].stack_array[ptp_id].vlan = vlan;
    unit_mgmt_array[unit].stack_array[ptp_id].prio = prio;

    if (BCM_FAILURE(rv = bcm_pkt_alloc(unit, 1600, BCM_TX_CRC_REGEN, 
            &unit_mgmt_array[unit].stack_array[ptp_id].mgmt_packet))) {
        return rv;
    }

    if (BCM_FAILURE(rv = bcm_pkt_alloc(unit, 1600, BCM_TX_CRC_REGEN, 
            &unit_mgmt_array[unit].stack_array[ptp_id].tunnel_packet))) {
        bcm_pkt_free(unit, 
            unit_mgmt_array[unit].stack_array[ptp_id].mgmt_packet);
        return rv;
    }

    if (BCM_FAILURE(rv = bcm_pkt_alloc(unit, 1600, BCM_TX_CRC_REGEN, 
            &unit_mgmt_array[unit].stack_array[ptp_id].ucsig_packet))) {
        bcm_pkt_free(unit, 
            unit_mgmt_array[unit].stack_array[ptp_id].mgmt_packet);
        bcm_pkt_free(unit, 
            unit_mgmt_array[unit].stack_array[ptp_id].tunnel_packet);
        return rv;
    }

    mgmt_packet = unit_mgmt_array[unit].stack_array[ptp_id].mgmt_packet;
    tunnel_packet = unit_mgmt_array[unit].stack_array[ptp_id].tunnel_packet;
    ucsig_packet = unit_mgmt_array[unit].stack_array[ptp_id].ucsig_packet;

    BCM_PBMP_ASSIGN(mgmt_packet->tx_pbmp, top_bitmap);
    BCM_PBMP_ASSIGN(tunnel_packet->tx_pbmp, top_bitmap);
    BCM_PBMP_ASSIGN(ucsig_packet->tx_pbmp, top_bitmap);        

    _bcm_ptp_management_message_create(BCM_PKT_IEEE(mgmt_packet));
    _bcm_ptp_management_message_macaddr_set(BCM_PKT_IEEE(mgmt_packet), 
        src_mac, dest_mac);
    _bcm_ptp_management_message_ipaddr_set(BCM_PKT_IEEE(mgmt_packet), 
        src_ip, dest_ip);
    _bcm_ptp_management_message_vlantag_set(BCM_PKT_IEEE(mgmt_packet), 
        vlan, prio);

    /* Make template (default) tunnel message packet. */
    i = 0;
    sal_memcpy(BCM_PKT_IEEE(tunnel_packet), l2hdr_default, 
               sizeof(l2hdr_default));
    i += sizeof(l2hdr_default);
    
    sal_memcpy(BCM_PKT_IEEE(tunnel_packet) + i, vlanhdr_default, 
               sizeof(vlanhdr_default));
    i += sizeof(vlanhdr_default);
    
    sal_memcpy(BCM_PKT_IEEE(tunnel_packet) + i, ethhdr_default, 
               sizeof(ethhdr_default));
    i += sizeof(ethhdr_default);
    
    sal_memcpy(BCM_PKT_IEEE(tunnel_packet) + i, ipv4hdr_default, 
               sizeof(ipv4hdr_default));
    i += sizeof(ipv4hdr_default);
    
    sal_memcpy(BCM_PKT_IEEE(tunnel_packet) + i, udphdr_tunnel,
               sizeof(udphdr_tunnel));

    _bcm_ptp_management_message_macaddr_set(BCM_PKT_IEEE(tunnel_packet), 
        src_mac, dest_mac);
    _bcm_ptp_management_message_ipaddr_set(BCM_PKT_IEEE(tunnel_packet), 
        src_ip, dest_ip);
    _bcm_ptp_management_message_vlantag_set(BCM_PKT_IEEE(tunnel_packet), 
        vlan, prio);
    
    /* Make template (default) unicast transmission signaling packet. */
    i = 0;
    sal_memcpy(BCM_PKT_IEEE(ucsig_packet), l2hdr_default, 
               sizeof(l2hdr_default));
    i += sizeof(l2hdr_default);
    
    sal_memcpy(BCM_PKT_IEEE(ucsig_packet) + i, vlanhdr_default, 
               sizeof(vlanhdr_default));
    i += PTP_VLANHDR_SIZE_OCTETS;
    
    sal_memcpy(BCM_PKT_IEEE(ucsig_packet) + i, ethhdr_default, 
               sizeof(ethhdr_default));
    i += sizeof(ethhdr_default);
    
    sal_memcpy(BCM_PKT_IEEE(ucsig_packet) + i, ipv4hdr_default, 
               sizeof(ipv4hdr_default));
    i += sizeof(ipv4hdr_default);
    
    sal_memcpy(BCM_PKT_IEEE(ucsig_packet) + i, udphdr_default, 
               sizeof(udphdr_default));
    i += sizeof(udphdr_default);
    
    sal_memcpy(BCM_PKT_IEEE(ucsig_packet) + i, ptphdr_ucsig, 
               sizeof(ptphdr_ucsig));
    i += sizeof(ptphdr_ucsig);
    
    sal_memcpy(BCM_PKT_IEEE(ucsig_packet) + i, tgtport_ucsig, 
               sizeof(tgtport_ucsig));
    i += sizeof(tgtport_ucsig);
    
    sal_memcpy(BCM_PKT_IEEE(ucsig_packet) + i, unicast_request_tlv_ucsig, 
               sizeof(unicast_request_tlv_ucsig));    
    
    _bcm_ptp_management_message_macaddr_set(BCM_PKT_IEEE(ucsig_packet), 
        src_mac, dest_mac);
    _bcm_ptp_management_message_ipaddr_set(BCM_PKT_IEEE(ucsig_packet), 
        src_ip, dest_ip);
    _bcm_ptp_management_message_vlantag_set(BCM_PKT_IEEE(ucsig_packet), 
        vlan, prio);
    
    return rv;
}

/*
 * Function:
 *      _bcm_ptp_internal_transport_init
 * Purpose:
 *      Initialize message transport to an internal ToP core
 * Parameters:
 *      unit       - (IN) Unit number.
 *      ptp_id     - (IN) PTP stack ID.
 *      src_mac    - (IN) Source MAC address.
 *      dest_mac   - (IN) Destination (PTP stack) MAC address.
 *      src_ip     - (IN) Source IPv4 address.
 *      dest_ip    - (IN) Destination (PTP stack) IPv4 address.
 *      vlan       - (IN) VLAN.
 *      prio       - (IN) VLAN priority.
 *      top_bitmap - (IN) ToP bitmap.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int _bcm_ptp_internal_transport_init(
    int unit, 
    bcm_ptp_stack_id_t ptp_id,
    bcm_mac_t *src_mac,
    bcm_mac_t *dest_mac,
    bcm_ip_t src_ip,
    bcm_ip_t dest_ip,
    uint16 vlan,
    uint8 prio,
    bcm_pbmp_t top_bitmap)
{
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_ptp_management_clock_create
 * Purpose:
 *      Create the PTP management data of a PTP clock.
 * Parameters:
 *      unit      - (IN) Unit number.
 *      ptp_id    - (IN) PTP stack ID.
 *      clock_num - (IN) PTP clock number.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
_bcm_ptp_management_clock_create(
    int unit, 
    bcm_ptp_stack_id_t ptp_id,
    int clock_num)
{
    int rv = BCM_E_UNAVAIL;
             
    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, ptp_id, clock_num,
            PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        return rv;   
    }
    
    if ((unit_mgmt_array[unit].memstate != PTP_MEMSTATE_INITIALIZED) ||
            (unit_mgmt_array[unit].stack_array[ptp_id].memstate != 
            PTP_MEMSTATE_INITIALIZED)) {
        return BCM_E_UNAVAIL;
    }
                
    unit_mgmt_array[unit].stack_array[ptp_id]
                         .clock_data[clock_num] = mgmt_default;
    
    return rv;
}

/*
 * Function:
 *      _bcm_ptp_management_message_send
 * Purpose:
 *      Send a PTP management message.
 * Parameters:
 *      unit            - (IN) Unit number.
 *      ptp_id          - (IN) PTP stack ID.
 *      clock_num       - (IN) PTP clock number.
 *      dest_clock_port - (IN) Destination PTP clock port.
 *      action          - (IN) PTP management action (actionField).
 *      cmd             - (IN) PTP command (managementId).
 *      payload         - (IN) PTP management message payload.
 *      payload_len     - (IN) PTP management message payload size (octets).
 *      resp            - (IN/OUT) PTP management message response.
 *      resp_len        - (OUT) PTP management message response size (octets).
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
_bcm_ptp_management_message_send(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num, 
    const bcm_ptp_port_identity_t* dest_clock_port, 
    uint8 action,
    uint16 cmd, 
    uint8* payload, 
    int payload_len,
    uint8* resp, 
    int* resp_len)
{
    int rv = BCM_E_UNAVAIL;

    uint8 *response_data;
    int response_len;
    
    uint8 resp_action;
    
    uint16 tlvtype;
    uint16 tlvlen; 
    
    uint16 err_code;
    uint16 mgmt_id;
    
    int response_payload_len;
    int max_resp_len = (resp_len) ? *resp_len : 0;
    int tlv_len;
    int ptp_len;
    uint8 domain;
    uint8 *clock_id;
    uint16 port_id;
    uint8 ptp_mgmt_header[56];
    uint8 *cursor;

    _bcm_ptp_info_t *ptp_info_p;
    _bcm_ptp_stack_info_t *stack_p;

    SET_PTP_INFO;

    stack_p = &ptp_info_p->stack_info[ptp_id];
    
    if (resp_len) {
        *resp_len = 0;
    }
    
    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, ptp_id, clock_num,
            PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        PTP_ERROR_FUNC("_bcm_ptp_function_precheck()");
        return rv;   
    }
    
    if ((unit_mgmt_array[unit].memstate != PTP_MEMSTATE_INITIALIZED) ||
            (unit_mgmt_array[unit].stack_array[ptp_id].memstate != 
            PTP_MEMSTATE_INITIALIZED)) {
        SOC_DEBUG_PRINT((DK_ERR | DK_VERBOSE,
            "%s() failed %s\n", __func__, 
            "_bcm_ptp_management_message_send(): memory state"));
        return BCM_E_UNAVAIL;
    }  
            
    if (BCM_FAILURE(rv = sal_sem_take(unit_mgmt_array[unit]
            .stack_array[ptp_id].mgmt_available, 
            PTP_MGMT_RESPONSE_TIMEOUT_US))) {
        PTP_ERROR_FUNC("sal_sem_take()");
        return rv;
    }

    tlv_len = 2 + payload_len;
    ptp_len = 48 + 6 + payload_len;

    domain = unit_mgmt_array[unit].stack_array[ptp_id].clock_data[clock_num].domain;

    clock_id = (uint8*)dest_clock_port->clock_identity;
    port_id = dest_clock_port->port_number;

    sal_memset ((void *)ptp_mgmt_header, 0, sizeof (ptp_mgmt_header));    
    /* format PTP header */
    cursor = &ptp_mgmt_header[0]; 
    *cursor++ = _bcmPTPmessageType_management; /* Message ID */
    *cursor++ = 0x02;                     /* PTP version. */
    *cursor++ = ((ptp_len >> 8) & 0xff);  /* PTP message length */
    *cursor++ = (ptp_len & 0xff);
    *cursor++ = domain;
    *cursor++ = 0x00;                     /* reserved */
        /* PTP flags  (Unicast = true). */
    *cursor++ = 0x04;   
    *cursor++ = 0x00;
        /* Source clock ID. */
    cursor = &ptp_mgmt_header[20]; 
    *cursor++ = 0x00; 
    *cursor++ = 0x10; 
    *cursor++ = 0x94; 
    *cursor++ = 0xff; 
    *cursor++ = 0xfe;
    *cursor++ = 0x00;
    *cursor++ = 0x00;
    *cursor++ = 0x04;
        /* Source port      */        
    *cursor++ = 0x00;
    *cursor++ = 0x01;   
        /* Sequence ID. */
    *cursor++ = 0x00; 
    *cursor++ = 0x01; 

    *cursor++ = 0x04;          /* Control field (management) */
    *cursor++ = 0x7f;          /* log meessage interval */

        /* Management message header */
        /* Target clock ID */
    *cursor++ = clock_id[0];
    *cursor++ = clock_id[1];
    *cursor++ = clock_id[2];
    *cursor++ = clock_id[3];
    *cursor++ = clock_id[4];
    *cursor++ = clock_id[5];
    *cursor++ = clock_id[6];
    *cursor++ = clock_id[7];
        /* Target port num */
    *cursor++= ((port_id >> 8) & 0xff);
    *cursor++ = (port_id & 0xff);
        /* Starting boundary hops */
    *cursor++ = 0x01;
        /* Remaining boundary hops */
    *cursor++ = 0x01;
        /* Action */
    *cursor++ = action;
    *cursor++ = 0x00;                     /* reserved */
        /* TLV Type: Management */
    *cursor++ = 0x00;
    *cursor++ = 0x01; 

    *cursor++ = (tlv_len >> 8) & 0xff;
    *cursor++ = (tlv_len) & 0xff;
    *cursor++ = (cmd >> 8) & 0xff;       /* management ID */
    *cursor++ = (cmd) & 0xff;

    /* TODO: flush response here, if necessary */
    /* _bcm_ptp_rx_response_flush(unit, ptp_id, clock_num); */
    rv = stack_p->tx(unit, ptp_id, ptp_mgmt_header, 48 + 6, payload, payload_len);

    if (rv != BCM_E_NONE) {
        SOC_DEBUG_PRINT((DK_ERR | DK_VERBOSE,
            "%s() failed %s\n", __func__, "Tx failed"));
        goto release_mgmt_lock;
    }

    /* get rx buffer, either from rx callback or from cmicm wait task */

    /* 
     * NOTICE: This call will return an rx response buffer that we will need to
     *         release by notifying the Rx section
     */
    if (BCM_FAILURE(rv = _bcm_ptp_rx_response_get(unit, ptp_id, clock_num, 
                             PTP_MGMT_RESPONSE_TIMEOUT_US, &response_data, &response_len))) {
        SOC_DEBUG_PRINT((DK_ERR | DK_VERBOSE,
            "%s() failed %s\n", __func__, "No Response"));
        goto release_mgmt_lock;
    } 

    rv = BCM_E_FAIL;
    

    resp_action = (response_data[PTP_MGMT_RESP_ACTION_OFFSET] & 0x0f);
    
    if (((action == PTP_MGMTMSG_SET) || 
         (action == PTP_MGMTMSG_GET)) &&
        (resp_action != PTP_MGMTMSG_RESP) ) {

        /* soc_cm_print("(%p) bad resp action: %d (from %02x) vs  %d\n", response_data, action, response_data[PTP_MGMT_RESP_ACTION_OFFSET], PTP_MGMTMSG_RESP); */
        /* int i; */
        /* for (i = 0; i < response_len; ++i) { */
        /*     soc_cm_print("%02x ", response_data[i]); */
        /*     if ((i & 0x1f) == 0x1f) { */
        /*         soc_cm_print("\n"); */
        /*     } */
        /* } */
        SOC_DEBUG_PRINT((DK_ERR | DK_VERBOSE,
            "%s() failed %s\n", __func__, "Bad response action"));

        goto dispose_of_resp;
    }
    
    if ((action == PTP_MGMTMSG_CMD) && 
        (resp_action != PTP_MGMTMSG_ACK) ) {
        SOC_DEBUG_PRINT((DK_ERR | DK_VERBOSE,
            "%s() failed %s\n", __func__, "Bad cmd response action"));
        goto dispose_of_resp;
    }
    
    tlvtype = _bcm_ptp_uint16_read(response_data + 
                                   PTP_MGMT_RESP_TLV_TYPE_OFFSET);
    
    if (tlvtype == _bcmPTPtlvType_management_error_status) {
        err_code = _bcm_ptp_uint16_read(response_data + 
                                        PTP_MGMT_RESP_TLV_VAL_OFFSET);
        
        SOC_DEBUG_PRINT((DK_ERR | DK_VERBOSE,
            "Error response: 0x%04x\n", err_code));
        goto dispose_of_resp;
    }
    
    tlvlen = _bcm_ptp_uint16_read(response_data + 
                                  PTP_MGMT_RESP_TLV_LEN_OFFSET);
    
    if (tlvlen + PTP_MGMT_RESP_TLV_VAL_OFFSET > response_len) {
        /* soc_cm_print("bad tlvlen: %d + %d > %d\n", tlvlen, PTP_MGMT_RESP_TLV_VAL_OFFSET, response_len); */

        /* int i; */
        /* for (i = 0; i < response_len; ++i) { */
        /*     soc_cm_print("%02x ", response_data[i]); */
        /*     if ((i & 0x1f) == 0x1f) { */
        /*         soc_cm_print("\n"); */
        /*     } */
        /* } */
        SOC_DEBUG_PRINT((DK_ERR | DK_VERBOSE,
            "%s() failed %s\n", __func__, "Bad TLV len"));
        goto dispose_of_resp;
    }
    
    mgmt_id = _bcm_ptp_uint16_read(response_data + 
                                   PTP_MGMT_RESP_TLV_VAL_OFFSET);
    
    if (mgmt_id != cmd) {
        SOC_DEBUG_PRINT((DK_ERR | DK_VERBOSE,
            "Bad mgmt ID: %04x vs %04x\n", mgmt_id, cmd));
        goto dispose_of_resp;
    }
    
    if (resp && resp_len) {
        response_payload_len = tlvlen - 2;
        if (response_payload_len > max_resp_len) {
            response_payload_len = max_resp_len;
        }
        *resp_len = response_payload_len;
        sal_memcpy(resp, response_data + 
                   PTP_MGMT_RESP_PAYLOAD_OFFSET,
                   response_payload_len);
    }
    
    rv = BCM_E_NONE;
    
 dispose_of_resp:
    _bcm_ptp_external_rx_response_free(unit, ptp_id, response_data);
    
 release_mgmt_lock:
    sal_sem_give(unit_mgmt_array[unit].stack_array[ptp_id].mgmt_available);
    
    return rv;
}


int _bcm_ptp_external_tx(
    int unit,
    bcm_ptp_stack_id_t ptp_id,
    uint8 *header,
    int header_len,
    uint8 *data,
    int data_len)
{
    int rv = BCM_E_NONE;

    /* WFT: probably should just allocate packet here rather than having pre-allocated... */
    _bcm_ptp_stack_mgmt_array_t *mgmt = &unit_mgmt_array[unit].stack_array[ptp_id];
    bcm_pkt_t *mgmt_packet = mgmt->mgmt_packet;
    uint8 *cursor = BCM_PKT_IEEE(mgmt_packet);
    int prio = 0;

    /* L2 Header MACs*/
    sal_memcpy(cursor, mgmt->dest_mac, sizeof(bcm_mac_t));  cursor += sizeof(bcm_mac_t);
    sal_memcpy(cursor, mgmt->src_mac, sizeof(bcm_mac_t));  cursor += sizeof(bcm_mac_t);

    /*   VLAN */
    *cursor++ = 0x81; *cursor++ = 0x00;
    *cursor++ = (prio << 5) | (0x0f & (mgmt->vlan >> 8));
    *cursor++ = (mgmt->vlan);

    /*   Ethertype: IPv4 */
    *cursor++ = 0x08;
    *cursor++ = 0x00;

    /* IPv4 Header */
    *cursor++ = 0x45;  /*type & header length */
    *cursor++ = 0x00;

    /*   IPv4 total length */
    _bcm_ptp_uint16_write(cursor, 20 + 8 + header_len + data_len); cursor += 2;

    *cursor++ = 0;
    *cursor++ = 0;

    *cursor++ = 0x40;  /* flags */
    *cursor++ = 0;

    *cursor++ = 1;     /* TTL */
    *cursor++ = 0x11;  /* protocol = UDP */

    *cursor++ = 0;     /* header checksum (zeros for computation) */
    *cursor++ = 0;

    _bcm_ptp_uint32_write(cursor, mgmt->src_ip);  cursor += 4;
    _bcm_ptp_uint32_write(cursor, mgmt->dest_ip);  cursor += 4;

    /* UDP Header */
    _bcm_ptp_uint16_write(cursor, 0x140);  cursor += 2;  /* source port */
    _bcm_ptp_uint16_write(cursor, 0x140);  cursor += 2;  /* dest port */

    _bcm_ptp_uint16_write(cursor, 8 + header_len + data_len);  cursor += 2;  /* UDP length */

    _bcm_ptp_uint16_write(cursor, 0);  cursor += 2;  /* UDP Checksum (optional) */

    /* Update IPv4 header checksum. */
    rv = _bcm_ptp_management_message_ipv4hdr_checksum_set(BCM_PKT_IEEE(mgmt_packet));

    sal_memcpy(cursor, header, header_len);  cursor += header_len;
    sal_memcpy(cursor, data, data_len);  cursor += data_len;



    /* 
     * Set packet length.
     * NOTICE: Includes 4 for the CRC appended to the packet 
     */
    mgmt_packet->pkt_data[0].len = (cursor - BCM_PKT_IEEE(mgmt_packet)) + 4;

    rv = bcm_tx(unit, mgmt_packet, NULL);

    return rv;
}


#if defined(BCM_KATANA_SUPPORT)
int _bcm_ptp_internal_tx(
    int unit,
    bcm_ptp_stack_id_t ptp_id,
    uint8 *header,
    int header_len,
    uint8 *data,
    int data_len)
{
    int rv = BCM_E_NONE;
    _bcm_ptp_info_t *ptp_info_p;
    _bcm_ptp_stack_info_t *stack_p;
     
    mos_msg_data_t uc_msg;
    uint8 *msg_data;

    SET_PTP_INFO;
    stack_p = &ptp_info_p->stack_info[ptp_id];

    /* mbox 0 reserved for cmd/resp */
    msg_data = (uint8 *)stack_p->int_state.mbox->data[0];

    /* to-TOP buffer should not be in use */
    if (stack_p->int_state.mbox->status[0] != MBOX_STATUS_EMPTY) {
        soc_cm_print("TOP message buffer in use on Tx\n");
    }

    sal_memcpy(msg_data, header, header_len);
    sal_memcpy(msg_data + header_len, data, data_len);

    stack_p->int_state.mbox->data_len[0] = header_len + data_len;
    stack_p->int_state.mbox->status[0] = MBOX_STATUS_ATTN_TOP_CMD;

    sal_memset(&uc_msg, 0, sizeof(uc_msg));
    uc_msg.s.mclass = MOS_MSG_CLASS_1588;
    uc_msg.s.subclass = MOS_MSG_SUBCLASS_1588_CMDRESP;
    uc_msg.s.len = header_len + data_len;
    uc_msg.s.data = 0;

    sal_usleep(250000);

    rv = soc_cmic_uc_msg_send(unit, stack_p->int_state.core_num, &uc_msg, 1000000);

    return rv;
}
#endif

/*
 * Function:
 *      _bcm_ptp_tunnel_message_to_top
 * Purpose:
 *      Tunnel a message to ToP processor.
 * Parameters:
 *      unit        - (IN) Unit number.
 *      ptp_id      - (IN) PTP stack ID.
 *      payload_len - (IN) Tunnel message payload size (octets).
 *      payload     - (IN) Tunnel message payload.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
_bcm_ptp_tunnel_message_to_top(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    unsigned payload_len,
    uint8 *payload)
{
    int rv = BCM_E_UNAVAIL;

    bcm_pkt_t *tunnel_packet;
    
    uint16 udp_length = payload_len + PTP_UDPHDR_SIZE_OCTETS;
    uint16 ip_length  = udp_length + PTP_IPV4HDR_SIZE_OCTETS;
    uint16 packet_length = ip_length + PTP_IPV4HDR_START_IDX;
    int i = 0;

    /* TODO: use the dispatched stack_p->transport_tx functions */
    
    tunnel_packet = unit_mgmt_array[unit].stack_array[ptp_id].tunnel_packet;

    i = PTP_PTPHDR_START_IDX;
    sal_memcpy(BCM_PKT_IEEE(tunnel_packet) + i, payload, payload_len);
    
    i = PTP_UDPHDR_START_IDX + PTP_UDPHDR_MSGLEN_OFFSET_OCTETS;
    _bcm_ptp_uint16_write(BCM_PKT_IEEE(tunnel_packet) + i, udp_length);
    
    i = PTP_IPV4HDR_START_IDX + PTP_IPV4HDR_MSGLEN_OFFSET_OCTETS;
    _bcm_ptp_uint16_write(BCM_PKT_IEEE(tunnel_packet) + i, ip_length);

    _bcm_ptp_management_message_port_set(BCM_PKT_IEEE(tunnel_packet), 0x0141, 0x0141);
     
    /* Set IPv4 header checksum. */
    if (BCM_FAILURE(rv = _bcm_ptp_management_message_ipv4hdr_checksum_set(
            BCM_PKT_IEEE(tunnel_packet)))) {
        return rv;
    }
    
    /* Set packet size. */
    tunnel_packet->pkt_data[0].len = packet_length + 4;

    return bcm_tx(unit, tunnel_packet, NULL);    
}

/*
 * Function:
 *      _bcm_ptp_tunnel_message_to_world
 * Purpose:
 *      Tunnel a message to "world".
 * Parameters:
 *      unit        - (IN) Unit number.
 *      ptp_id      - (IN) PTP stack ID.
 *      payload_len - (IN) Tunnel message payload size (octets).
 *      payload     - (IN) Tunnel message payload.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
_bcm_ptp_tunnel_message_to_world(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    unsigned payload_len,
    uint8 *payload)
{
    /* TODO: use the dispatched stack_p->transport_tx functions */

    int rv = BCM_E_UNAVAIL;

    bcm_pkt_t *tunnel_packet;
    
    uint16 udp_length = payload_len + PTP_UDPHDR_SIZE_OCTETS;
    uint16 ip_length  = udp_length + PTP_IPV4HDR_SIZE_OCTETS;
    uint16 packet_length = ip_length + PTP_IPV4HDR_START_IDX;
    int i = 0;
    
    tunnel_packet = unit_mgmt_array[unit].stack_array[ptp_id].tunnel_packet;

    i = PTP_PTPHDR_START_IDX;
    sal_memcpy(BCM_PKT_IEEE(tunnel_packet) + i, payload, payload_len);
    
    i = PTP_UDPHDR_START_IDX + PTP_UDPHDR_MSGLEN_OFFSET_OCTETS;
    _bcm_ptp_uint16_write(BCM_PKT_IEEE(tunnel_packet) + i, udp_length);
    
    i = PTP_IPV4HDR_START_IDX + PTP_IPV4HDR_MSGLEN_OFFSET_OCTETS;
    _bcm_ptp_uint16_write(BCM_PKT_IEEE(tunnel_packet) + i, ip_length);

    _bcm_ptp_management_message_port_set(BCM_PKT_IEEE(tunnel_packet), 0x0143, 0x0143);
     
    /* Set IPv4 header checksum. */
    if (BCM_FAILURE(rv = _bcm_ptp_management_message_ipv4hdr_checksum_set(
            BCM_PKT_IEEE(tunnel_packet)))) {
        return rv;
    }
    
    /* Set packet size. */
    tunnel_packet->pkt_data[0].len = packet_length + 4;

    return bcm_tx(unit, tunnel_packet, NULL);    
}

/*
 * Function:
 *      _bcm_ptp_unicast_slave_subscribe
 * Purpose:
 *      Manage external slave subscriptions of a PTP clock port.
 * Parameters:
 *      unit       - (IN) Unit number.
 *      ptp_id     - (IN) PTP stack ID.
 *      clock_num  - (IN) PTP clock number.
 *      clock_port - (IN) PTP clock port number.
 *      slave_info - (IN) Unicast slave information.
 *      msg_type   - (IN) PTP message type to request/cancel.
 *      tlv_type   - (IN) TLV type (request/cancel unicast transmission).
 *      interval   - (IN) Log inter-message interval (logInterMessagePeriod).
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      PTP unicast negotiation is used to manage external slave services
 *      through REQUEST_UNICAST_TRANSMISSION and CANCEL_UNICAST_TRANSMISSION
 *      TLVs.
 *      Ref. IEEE Std. 1588-2008, Chapters 14.1 and 16.1.
 */
int
_bcm_ptp_unicast_slave_subscribe(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num, 
    uint32 clock_port, 
    bcm_ptp_clock_peer_t *slave_info,
    _bcmPTPmessageType_t msg_type,
    _bcmPTPtlvType_t tlv_type,
    int interval)
{
    int rv = BCM_E_UNAVAIL;
    
    bcm_ptp_port_identity_t portid;
    bcm_pkt_t *ucsig_packet;
    
    uint16 ucsig_octets = 0;     
    uint16 tlv_octets = 0;       
    uint16 sequence_id = 0x0002; 
    
    uint8 *response_data;
    uint16 response_tlv_type;
    int response_len;
    int i = 0;

    _bcm_ptp_info_t *ptp_info_p;
    _bcm_ptp_stack_info_t *stack_p;

    SET_PTP_INFO;
    stack_p = &ptp_info_p->stack_info[ptp_id];
  
    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, ptp_id, clock_num,
            PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        return rv;   
    }
    
    if ((unit_mgmt_array[unit].memstate != PTP_MEMSTATE_INITIALIZED) ||
            (unit_mgmt_array[unit].stack_array[ptp_id].memstate != 
            PTP_MEMSTATE_INITIALIZED)) {
        return BCM_E_UNAVAIL;
    }  
  
    if (BCM_FAILURE(rv = sal_sem_take(unit_mgmt_array[unit]
            .stack_array[ptp_id].mgmt_available, 
            PTP_MGMT_RESPONSE_TIMEOUT_US))) {
        PTP_ERROR_FUNC("sal_sem_take()");
        return rv;
    }
    
    /* 
     * TLV-type dependent message constructor parameters. 
     * Set unicast signaling message size information based on TLV type.
     * Set sequence ID numbering scheme offset. 
     */
    switch (tlv_type) {
    case _bcmPTPtlvType_request_unicast_transmission:
        ucsig_octets = PTP_UCSIG_REQUEST_TOTAL_SIZE_OCTETS;
        tlv_octets = PTP_UCSIG_REQUEST_TLV_SIZE_OCTETS;
        sequence_id += 0;
        break;

    case _bcmPTPtlvType_cancel_unicast_transmission:
        ucsig_octets = PTP_UCSIG_CANCEL_TOTAL_SIZE_OCTETS; 
        tlv_octets = PTP_UCSIG_CANCEL_TLV_SIZE_OCTETS;
        sequence_id += 4;
        break;

    default:
        return BCM_E_PARAM;
    }

    /* Make unicast transmission message. */
    ucsig_packet = unit_mgmt_array[unit].stack_array[ptp_id].ucsig_packet;

    /* Set PTP domain. */
    _bcm_ptp_management_message_domain_set(BCM_PKT_IEEE(ucsig_packet), 
        unit_mgmt_array[unit].stack_array[ptp_id]
                             .clock_data[clock_num].domain);
    /* 
     * Insert sequence ID.
     * NOTICE: Logic uses a unique sequence IDs for distinct classes only of 
     *         REQUEST_UNICAST_TRANSMISSION and CANCEL_UNICAST_TRANSMISSION 
     *         signaling messages.
     */
    switch (msg_type) {
    case _bcmPTPmessageType_announce:
        break;

    case _bcmPTPmessageType_sync:
       sequence_id += 1;
       break;

    case _bcmPTPmessageType_delay_resp:
       sequence_id += 2;
       break;

    case _bcmPTPmessageType_pdelay_resp:
        sequence_id += 3;
        break;

    default:
        SOC_DEBUG_PRINT((DK_ERR | DK_VERBOSE,
            "%s() failed %s\n", __func__, "Unknown message type"));
        rv = BCM_E_PARAM;
        goto release_return;
    }

    i = PTP_PTPHDR_START_IDX + PTP_PTPHDR_SEQID_OFFSET_OCTETS;
    _bcm_ptp_uint16_write(BCM_PKT_IEEE(ucsig_packet) + i, sequence_id);

    /* 
     * Insert source port identity.
     * NOTICE: In this context, the source port is the remote port, i.e.,
     *         requester/grantee of signaling messages. 
     */
    i = PTP_PTPHDR_START_IDX + PTP_PTPHDR_SRCPORT_OFFSET_OCTETS;
    sal_memcpy(BCM_PKT_IEEE(ucsig_packet) + i, 
               slave_info->clock_identity, 
               sizeof(bcm_ptp_clock_identity_t));
    i += sizeof(bcm_ptp_clock_identity_t);
    
    _bcm_ptp_uint16_write(BCM_PKT_IEEE(ucsig_packet) + i, 
        slave_info->remote_port_number);

    /* Insert target port identity. */
    if (BCM_FAILURE(rv = bcm_esw_ptp_clock_port_identity_get(unit, ptp_id, 
            clock_num, slave_info->local_port_number, &portid))) {
        goto release_return;
    }

    i = PTP_SIGHDR_START_IDX + PTP_SIGHDR_TGTCLOCK_OFFSET_OCTETS;
    sal_memcpy(BCM_PKT_IEEE(ucsig_packet) + i, portid.clock_identity, 
               sizeof(bcm_ptp_clock_identity_t));

    i = PTP_SIGHDR_START_IDX + PTP_SIGHDR_TGTPORT_OFFSET_OCTETS;
    _bcm_ptp_uint16_write(BCM_PKT_IEEE(ucsig_packet) + i, 
        slave_info->local_port_number);

    /* Insert TLV type. */
    i = PTP_UCSIG_TLV_START_IDX;
    _bcm_ptp_uint16_write(BCM_PKT_IEEE(ucsig_packet) + i, tlv_type);

    /* 
     * Insert TLV length. 
     * NOTICE: PTP message length, TLV "V" part only.
     */
    i = PTP_UCSIG_TLV_START_IDX + PTP_UCSIG_TLVLEN_OFFSET_OCTETS;
    _bcm_ptp_uint16_write(BCM_PKT_IEEE(ucsig_packet) + i, (tlv_octets-4));

    /*
     * Insert message type.
     * NOTICE: IEEE Std. 1588-2008, Tables 73-75 specify following format for 
     *         4-bit message type (messageType).
     *         |B7|B6|B5|B4|B3|B2|B1|B0| = 
     *         |messageType|X |X |X |X |. X = Reserved.
     */
    i = PTP_UCSIG_TLV_START_IDX + PTP_UCSIG_MSGTYPE_OFFSET_OCTETS;
    BCM_PKT_IEEE(ucsig_packet)[i] = ((msg_type << 4) & 0xf0);

    /* Insert IP header length. */
    i = PTP_IPV4HDR_START_IDX + PTP_IPV4HDR_MSGLEN_OFFSET_OCTETS;
    _bcm_ptp_uint16_write(BCM_PKT_IEEE(ucsig_packet) + i, 
        (ucsig_octets - PTP_IPV4HDR_START_IDX));

    /* Insert UDP header length. */
    i = PTP_UDPHDR_START_IDX + PTP_UDPHDR_MSGLEN_OFFSET_OCTETS;
    _bcm_ptp_uint16_write(BCM_PKT_IEEE(ucsig_packet) + i, 
        (ucsig_octets - PTP_UDPHDR_START_IDX));    

    /* Insert PTP header length. */
    i = PTP_PTPHDR_START_IDX + PTP_PTPHDR_MSGLEN_OFFSET_OCTETS;
    _bcm_ptp_uint16_write(BCM_PKT_IEEE(ucsig_packet) + i, 
        (ucsig_octets - PTP_PTPHDR_START_IDX));    

    switch (tlv_type) {
    case _bcmPTPtlvType_request_unicast_transmission:
        /* REQUEST_UNICAST_TRANSMISSION TLV. */
        
        /* Set log message interval. */
        i = PTP_UCSIG_TLV_START_IDX + PTP_UCSIG_INTERVAL_OFFSET_OCTETS;
        BCM_PKT_IEEE(ucsig_packet)[i++] = (uint8)interval;

        /* Set message duration. */
        _bcm_ptp_uint32_write(BCM_PKT_IEEE(ucsig_packet) + i, 
            PTP_UNICAST_NEVER_EXPIRE);

        /*
         * Insert signaling message TLV data.
         *    Peer address.
         */
        i = PTP_UCSIG_TLV_START_IDX + PTP_UCSIG_ADDRTYPE_OFFSET_OCTETS;
        switch (slave_info->peer_address.addr_type) {
        case bcmPTPUDPIPv4:
            /* Ethernet/UDP/IPv4 address type. */
            BCM_PKT_IEEE(ucsig_packet)[i++] = bcmPTPUDPIPv4;
            _bcm_ptp_uint32_write(BCM_PKT_IEEE(ucsig_packet) + i, 
                slave_info->peer_address.ipv4_addr);

            break;

        case bcmPTPUDPIPv6:
            /* Ethernet/UDP/IPv6 address type. */
            BCM_PKT_IEEE(ucsig_packet)[i++] = bcmPTPUDPIPv6;
            sal_memcpy(BCM_PKT_IEEE(ucsig_packet)  + i, 
                       slave_info->peer_address.ipv6_addr, sizeof(bcm_ip6_t));

            break;

        default:
            /* Unknown or unsupported address type. */
            SOC_DEBUG_PRINT((DK_ERR | DK_VERBOSE,
                "%s() failed %s\n", __func__, "Unknown or unsupported address type"));
            rv = BCM_E_PARAM;
            goto release_return;
        }

        /* Signaling message L2 header length and L2 header. */
        i += PTP_MAX_NETW_ADDR_SIZE;

        BCM_PKT_IEEE(ucsig_packet)[i++] =
            (slave_info->peer_address.raw_l2_header_length > PTP_MAX_L2_HEADER_LENGTH) ? 
            (PTP_MAX_L2_HEADER_LENGTH) : (slave_info->peer_address.raw_l2_header_length);

        sal_memset(BCM_PKT_IEEE(ucsig_packet) + i, 0, PTP_MAX_L2_HEADER_LENGTH);
        sal_memcpy(BCM_PKT_IEEE(ucsig_packet) + i,
                   slave_info->peer_address.raw_l2_header,
                   slave_info->peer_address.raw_l2_header_length);

        break;

    case _bcmPTPtlvType_cancel_unicast_transmission:
        /* CANCEL_UNICAST_TRANSMISSION TLV. */

        /* Reserved TLV element. */
        i = PTP_UCSIG_TLV_START_IDX + PTP_UCSIG_CANCEL_RESERVED_OFFSET_OCTETS;
        BCM_PKT_IEEE(ucsig_packet)[i] = 0x00;
        
        /*
         * Insert signaling message TLV data.
         *    Peer address.
         */
        i = PTP_UCSIG_TLV_START_IDX + PTP_UCSIG_CANCEL_ADDRTYPE_OFFSET_OCTETS;
        switch (slave_info->peer_address.addr_type) {
        case bcmPTPUDPIPv4:
            /* Ethernet/UDP/IPv4 address type. */
            BCM_PKT_IEEE(ucsig_packet)[i++] = bcmPTPUDPIPv4;
            sal_memset(BCM_PKT_IEEE(ucsig_packet) + i, 0, PTP_MAX_NETW_ADDR_SIZE);
            _bcm_ptp_uint32_write(BCM_PKT_IEEE(ucsig_packet) + i, 
                slave_info->peer_address.ipv4_addr);

            break;

        case bcmPTPUDPIPv6:
            /* Ethernet/UDP/IPv6 address type. */
            BCM_PKT_IEEE(ucsig_packet)[i++] = bcmPTPUDPIPv6;
            sal_memset(BCM_PKT_IEEE(ucsig_packet) + i, 0, PTP_MAX_NETW_ADDR_SIZE);
            sal_memcpy(BCM_PKT_IEEE(ucsig_packet) + i, 
                       slave_info->peer_address.ipv6_addr, sizeof(bcm_ip6_t));

            break;

        default:
            /* Unknown or unsupported address type. */
            SOC_DEBUG_PRINT((DK_ERR | DK_VERBOSE,
                "%s() failed %s\n", __func__, "Unknown or unsupported address type"));
            rv = BCM_E_PARAM;
            goto release_return;
        }
        
        break;

    default:
        /* Unknown or unsupported TLV. */
        SOC_DEBUG_PRINT((DK_ERR | DK_VERBOSE,
            "%s() failed %s\n", __func__, "Unknown or unsupported TLV"));
        rv = BCM_E_PARAM;
        goto release_return;
    }

    /* Message consistency. */

    /* Set IPv4 header checksum. */
    if (BCM_FAILURE(rv = _bcm_ptp_management_message_ipv4hdr_checksum_set(
            BCM_PKT_IEEE(ucsig_packet)))) {
        return rv;
    }

    /* Set packet size. */
    ucsig_packet->pkt_data[0].len = ucsig_octets + 4;

    _bcm_ptp_rx_response_flush(unit, ptp_id, clock_num);

    /* Transmit message. */
    if (BCM_FAILURE(rv = bcm_tx(unit, ucsig_packet, NULL))) {
        PTP_ERROR_FUNC("bcm_tx()");
        goto release_return;
    }

    /* 
     * NOTICE: This call will return an rx response buffer that we 
     *         will own and need to free. 
     */
    if (BCM_FAILURE(rv = _bcm_ptp_rx_response_get(unit, ptp_id, clock_num,          
            PTP_MGMT_RESPONSE_TIMEOUT_US, &response_data, &response_len))) {
        PTP_ERROR_FUNC("bcmptp_rx_get_response()");
        goto release_return;
    }

    rv = BCM_E_FAIL;

    /* Parse response. */
    response_tlv_type = _bcm_ptp_uint16_read(response_data + 
        PTP_UCSIG_TLV_START_IDX);
    if (tlv_type == _bcmPTPtlvType_request_unicast_transmission &&
            response_tlv_type != _bcmPTPtlvType_grant_unicast_transmission) {
        SOC_DEBUG_PRINT((DK_ERR | DK_VERBOSE,
            "%s() failed %s\n", __func__, "Unicast request failed"));
        goto dispose_of_resp;
    }

    if (tlv_type == _bcmPTPtlvType_cancel_unicast_transmission &&
            response_tlv_type != _bcmPTPtlvType_ack_cancel_unicast_transmission) {
        SOC_DEBUG_PRINT((DK_ERR | DK_VERBOSE,
            "%s() failed %s\n", __func__, "Unicast cancel failed"));
        goto dispose_of_resp;
    }

    rv = BCM_E_NONE;

dispose_of_resp:
    stack_p->rx_free(unit, ptp_id, response_data);

release_return:
    sal_sem_give(unit_mgmt_array[unit].stack_array[ptp_id].mgmt_available);
    
    return rv;
}

/*
 * Function:
 *      _bcm_ptp_management_domain_get
 * Purpose:
 *      Get the current domain used for PTP management messages for a PTP clock.
 * Parameters:
 *      unit      - (IN)  Unit number.
 *      ptp_id    - (IN)  PTP stack ID.
 *      clock_num - (IN)  PTP clock number.
 *      domain    - (OUT) PTP domain.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
_bcm_ptp_management_domain_get(
    int unit, 
    bcm_ptp_stack_id_t ptp_id,
    int clock_num,
    uint8 *domain)
{
    int rv = BCM_E_UNAVAIL;
         
    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, ptp_id, clock_num,
            PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        return rv;   
    }
    
    if ((unit_mgmt_array[unit].memstate != PTP_MEMSTATE_INITIALIZED) ||
            (unit_mgmt_array[unit].stack_array[ptp_id].memstate != 
            PTP_MEMSTATE_INITIALIZED)) {
        return BCM_E_UNAVAIL;
    }
    
    *domain = unit_mgmt_array[unit].stack_array[ptp_id]
                                   .clock_data[clock_num].domain;
    
    return rv;
}

/*
 * Function:
 *      _bcm_ptp_management_domain_set
 * Purpose:
 *      Set the domain to use in PTP management messages for a PTP clock.
 * Parameters:
 *      unit      - (IN) Unit number.
 *      ptp_id    - (IN) PTP stack ID.
 *      clock_num - (IN) PTP clock number.
 *      domain    - (IN) PTP domain.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
_bcm_ptp_management_domain_set(
    int unit, 
    bcm_ptp_stack_id_t ptp_id,
    int clock_num,
    uint8 domain)
{
    int rv = BCM_E_UNAVAIL;
         
    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, ptp_id, clock_num,
            PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        return rv;   
    }
    
    if ((unit_mgmt_array[unit].memstate != PTP_MEMSTATE_INITIALIZED) ||
            (unit_mgmt_array[unit].stack_array[ptp_id].memstate != 
            PTP_MEMSTATE_INITIALIZED)) {
        return BCM_E_UNAVAIL;
    }
        
    unit_mgmt_array[unit].stack_array[ptp_id]
                         .clock_data[clock_num].domain = domain;
    
    return rv;
}

/*
 * Function:
 *      _bcm_ptp_management_message_create
 * Purpose:
 *      Create a PTP management message from template.
 * Parameters:
 *      message - (IN/OUT) PTP management message.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
static int
_bcm_ptp_management_message_create(
    uint8 *message)
{
    int rv = BCM_E_UNAVAIL;
    int i = 0;
    
    /* 
     * Management message header 
     * NOTICE: Includes L2, VLAN, IPv4, UDP, PTP, and management headers. 
     */
    sal_memcpy(message, l2hdr_default, sizeof(l2hdr_default));
    i += sizeof(l2hdr_default);

    sal_memcpy(message + i, vlanhdr_default, sizeof(vlanhdr_default));
    i += sizeof(vlanhdr_default);

    sal_memcpy(message + i, ethhdr_default, sizeof(ethhdr_default));
    i += sizeof(ethhdr_default);

    sal_memcpy(message + i, ipv4hdr_default, sizeof(ipv4hdr_default));
    i += sizeof(ipv4hdr_default);

    sal_memcpy(message + i, udphdr_default, sizeof(udphdr_default));
    i += sizeof(udphdr_default);

    sal_memcpy(message + i, ptphdr_mgmt, sizeof(ptphdr_mgmt));
    i += sizeof(ptphdr_mgmt);

    sal_memcpy(message + i, mgmthdr_mgmt, sizeof(mgmthdr_mgmt));
    i += sizeof(mgmthdr_mgmt);

    sal_memcpy(message + i, tlvhdr_mgmt, sizeof(tlvhdr_mgmt));
    i += sizeof(tlvhdr_mgmt);

    /* Set L2 header (Ethertype= IPv4). */
    i = PTP_L2HDR_SIZE_OCTETS + PTP_VLANHDR_SIZE_OCTETS;
    message[i++] = 0x08;
    message[i] = 0x00;

    /* Update IPv4 header checksum. */
    rv = _bcm_ptp_management_message_ipv4hdr_checksum_set(message);
    
    return rv;
}

/* /\* */
/*  * Function: */
/*  *      _bcm_ptp_management_message_make */
/*  * Purpose: */
/*  *      Make a PTP management message. */
/*  * Parameters: */
/*  *      message     - (IN/OUT) PTP management message. */
/*  *      action      - (IN) PTP management action (actionField). */
/*  *      cmd         - (IN) PTP command (managementId). */
/*  *      payload     - (IN) PTP management message payload. */
/*  *      payload_len - (IN) PTP management message payload size (octets). */
/*  * Returns: */
/*  *      BCM_E_XXX */
/*  * Notes: */
/*  *\/ */
/* static int  */
/* _bcm_ptp_management_message_make( */
/*     uint8 *message,  */
/*     uint8 action,  */
/*     uint16 cmd,  */
/*     uint8 *payload,  */
/*     int payload_len) */
/* { */
/*     int rv = BCM_E_UNAVAIL; */
/*     int i = 0; */
    
/*     /\* Set management message action. *\/ */
/*     i = PTP_MGMTHDR_START_IDX + PTP_MGMTHDR_ACTION_OFFSET_OCTETS; */
/*     message[i] = action; */

/*     /\*  */
/*      * Set TLV data. */
/*      * TLV length.   */
/*      *\/ */
/*     i = PTP_MGMTHDR_START_IDX + PTP_MGMTHDR_SIZE_OCTETS +  */
/*         PTP_TLVHDR_LEN_OFFSET_OCTETS; */
    
/*     message[i++] = (uint8)((payload_len + PTP_TLVHDR_VAL_PREFIX_SIZE_OCTETS) >> 8); */
/*     message[i] = (uint8)((payload_len + PTP_TLVHDR_VAL_PREFIX_SIZE_OCTETS) & 0x00ff); */

/*     /\*  */
/*      * Set TLV data.  */
/*      * TLV value prefix (management ID/command definition). */
/*      *\/ */
/*     i = PTP_MGMTHDR_START_IDX + PTP_MGMTHDR_SIZE_OCTETS +  */
/*         PTP_TLVHDR_VAL_PREFIX_OFFSET_OCTETS; */
    
/*     message[i++]= (uint8)((cmd) >> 8); */
/*     message[i]= (uint8)((cmd) & 0x00ff); */

/*     /\* */
/*      * Insert payload. */
/*      * TLV value body. */
/*      *\/ */
/*     sal_memcpy(message + PTP_MGMTMSG_TOTAL_HEADER_SIZE, payload, payload_len); */

/*     /\* Update management message header metadata. *\/ */
/*     rv = _bcm_ptp_management_message_header_metadata_set(message); */

/*     return rv; */
/* } */

/*
 * Function:
 *      _bcm_ptp_management_message_header_metadata_set
 * Purpose:
 *      Set header lengths and IPv4 checksum based on PTP message payload.
 * Parameters:
 *      message     - (IN/OUT) PTP management message.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
/* static int  */
/* _bcm_ptp_management_message_header_metadata_set( */
/*     uint8 *message) */
/* { */
/*     int rv = BCM_E_UNAVAIL; */
    
/*     int tlv_length_pos; */
/*     uint16 tlv_length; */
/*     uint16 ptp_length; */
/*     uint16 udp_length; */
/*     uint16 ip_length; */
    
/*     tlv_length_pos = PTP_MGMTHDR_START_IDX + PTP_MGMTHDR_SIZE_OCTETS +  */
/*                      PTP_TLVHDR_LEN_OFFSET_OCTETS; */
    
/*     tlv_length = _bcm_ptp_uint16_read(message + tlv_length_pos); */
/*     ptp_length = tlv_length + PTP_PTPHDR_SIZE_OCTETS + PTP_MGMTHDR_SIZE_OCTETS +  */
/*                  PTP_TLVHDR_VAL_PREFIX_OFFSET_OCTETS; */
/*     udp_length = ptp_length + PTP_UDPHDR_SIZE_OCTETS; */
/*     ip_length = udp_length + PTP_IPV4HDR_SIZE_OCTETS; */

/*     _bcm_ptp_uint16_write(message + PTP_PTPHDR_START_IDX +  */
/*                           PTP_PTPHDR_MSGLEN_OFFSET_OCTETS, ptp_length); */
/*     _bcm_ptp_uint16_write(message + PTP_UDPHDR_START_IDX +  */
/*                           PTP_UDPHDR_MSGLEN_OFFSET_OCTETS, udp_length); */
/*     _bcm_ptp_uint16_write(message + PTP_IPV4HDR_START_IDX +  */
/*                           PTP_IPV4HDR_MSGLEN_OFFSET_OCTETS, ip_length); */

/*     /\* Update IPv4 header checksum. *\/ */
/*     rv = _bcm_ptp_management_message_ipv4hdr_checksum_set(message); */

/*     return rv; */
/* } */

/*
 * Function:
 *      _bcm_ptp_management_macaddr_set
 * Purpose:
 *      Set MAC addresses in PTP management message.
 * Parameters:
 *      message  - (IN/OUT) PTP management message.
 *      src_mac  - (IN) Source MAC address.
 *      dest_mac - (IN) Destination MAC address.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
static int 
_bcm_ptp_management_message_macaddr_set(
    uint8 *message, 
    bcm_mac_t *src_mac, 
    bcm_mac_t *dest_mac)
{
    int i = sizeof(bcm_mac_t); 

    sal_memcpy(message, dest_mac, i);
    sal_memcpy(message + i, src_mac, i);

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_ptp_management_message_vlantag_set
 * Purpose:
 *      Set VLAN tag in PTP management message.
 * Parameters:
 *      message  - (IN/OUT) PTP management message.
 *      vlan_tag - (IN) VLAN identifier (VID).
 *      prio     - (IN) Priority code point (PCP).
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
static int 
_bcm_ptp_management_message_vlantag_set(
    uint8 *message, 
    uint16 vlan_tag, 
    uint8 prio)
{
    int i = PTP_L2HDR_SIZE_OCTETS + PTP_VLANHDR_VID_OFFSET_OCTETS;

    /*
     * Set VLAN tag.
     * Logically combine VLAN identifier (VID) with priority code point (PCP).
     *
     *    IEEE 802.1Q VLAN tagging 32-bit frame.
     *    |16 Bits|      16 Bits       |
     *    |       |3 Bits|1 Bit|12 Bits|
     *    |TPID   |PCP   |CFI  |VID    |
     */
    message[i++] = (prio << 5) | (0x0f & (vlan_tag >> 8));
    message[i] = (vlan_tag & 0x00ff);

    return BCM_E_NONE;   
}

/*
 * Function:
 *      _bcm_ptp_management_message_ipaddr_set
 * Purpose:
 *      Set IP addresses in PTP management message.
 * Parameters:
 *      message - (IN/OUT) PTP management message.
 *      src_ip  - (IN) Source IPv4 address.
 *      dest_ip - (IN) Destination IPv4 address.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
static int 
_bcm_ptp_management_message_ipaddr_set(
    uint8 *message, 
    bcm_ip_t src_ip, 
    bcm_ip_t dest_ip)
{
    int rv = BCM_E_UNAVAIL;
    int i = PTP_L2HDR_SIZE_OCTETS + PTP_ETHHDR_SIZE_OCTETS +
            PTP_VLANHDR_SIZE_OCTETS + PTP_IPV4HDR_SRC_IPADDR_OFFSET_OCTETS;

    /* Update IPv4 header checksum. */
    if (BCM_FAILURE(rv = 
            _bcm_ptp_management_message_ipv4hdr_checksum_set(message))) {
        return rv;
    }

    /* Set management message header IP addresses. */
    _bcm_ptp_uint32_write(message + i, src_ip);
    i += sizeof(bcm_ip_t);
    _bcm_ptp_uint32_write(message + i, dest_ip);

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_ptp_management_message_ipv4hdr_checksum_set
 * Purpose:
 *      Set IPv4 header checksum.
 * Parameters:
 *      message - (IN/OUT) PTP management message.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
static int
_bcm_ptp_management_message_ipv4hdr_checksum_set(
    uint8 *message)
{
    uint16 chksum;
    int i = PTP_L2HDR_SIZE_OCTETS + PTP_VLANHDR_SIZE_OCTETS +
            PTP_ETHHDR_SIZE_OCTETS; 
   
    /*
     * Zero header checksum fields in IPv4 header.
     * Ref. RFC 791, pp. 14.
     */
    message[i+PTP_IPV4HDR_HDR_CHKSUM_OFFSET_OCTETS] = 0x00;
    message[i+PTP_IPV4HDR_HDR_CHKSUM_OFFSET_OCTETS+1] = 0x00;

    /* Calculate checksum. */
    chksum = _bcm_ptp_management_rfc791_checksum(message + i, 
                                                 PTP_IPV4HDR_SIZE_OCTETS);
   
    /* Insert checksum answer in IPv4 header. */
    message[i+PTP_IPV4HDR_HDR_CHKSUM_OFFSET_OCTETS] = (uint8)((chksum) >> 8);
    message[i+PTP_IPV4HDR_HDR_CHKSUM_OFFSET_OCTETS+1] = (uint8)((chksum) & 0x00ff);
   
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_ptp_management_message_port_set
 * Purpose:
 *      Set source and destination port numbers in UDP header.
 * Parameters:
 *      message   - (IN/OUT) PTP management message.
 *      src_port  - (IN) Source port number.
 *      dest_port - (IN) Destination port number.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
static int 
_bcm_ptp_management_message_port_set(
    uint8 *message, 
    uint16 src_port, 
    uint16 dest_port)
{
    int i = PTP_UDPHDR_START_IDX;
    
    _bcm_ptp_uint16_write(message + i, src_port);
    i += sizeof(uint16);
    _bcm_ptp_uint16_write(message + i, dest_port);

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_ptp_management_message_target_set
 * Purpose:
 *      Set target PTP clock port identity in PTP management message.
 * Parameters:
 *      message        - (IN/OUT) PTP management message.
 *      clock_identity - (IN) Target PTP clock identity.
 *      port_num       - (IN) Target PTP clock port number.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
/* static int  */
/* _bcm_ptp_management_message_target_set( */
/*     uint8 *message,  */
/*     const bcm_ptp_clock_identity_t clock_identity,  */
/*     uint16 port_num) */
/* { */
/*     int i = PTP_MGMTHDR_START_IDX;    */

/*     sal_memcpy(message + i, clock_identity, sizeof(bcm_ptp_clock_identity_t)); */
/*     i += sizeof(bcm_ptp_clock_identity_t); */
/*     _bcm_ptp_uint16_write(message + i, port_num); */

/*     return BCM_E_NONE; */
/* } */

/*
 * Function:
 *      _bcm_ptp_management_message_length_get
 * Purpose:
 *      Get the length of PTP management message.
 * Parameters:
 *      message - (IN)  PTP management message.
 * Returns:
 *      Length.
 * Notes:
 */
/* static int  */
/* _bcm_ptp_management_message_length_get( */
/*     uint8 *message) */
/* { */
/*     int len = PTP_MGMTMSG_TOTAL_HEADER_SIZE - PTP_TLVHDR_VAL_PREFIX_SIZE_OCTETS; */
    
/*     len += _bcm_ptp_uint16_read(message + PTP_MGMTHDR_START_IDX +  */
/*                                 PTP_MGMTHDR_SIZE_OCTETS +  */
/*                                 PTP_TLVHDR_LEN_OFFSET_OCTETS); */

/*     return len;    */
/* } */

#if 0 /* Unused. */
/*
 * Function:
 *      _bcm_ptp_management_message_domain_get
 * Purpose:
 *      Get the PTP domain in PTP management message.
 * Parameters:
 *      message - (IN)  PTP management message.
 *      domain  - (OUT) PTP domain.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
static int 
_bcm_ptp_management_message_domain_get(
    uint8 *message,
    uint8 *domain)
{
    *domain = message[PTP_PTPHDR_START_IDX + PTP_PTPHDR_DOMAIN_OFFSET_OCTETS];
    return BCM_E_NONE;
}
#endif /* Unused. */

/*
 * Function:
 *      _bcm_ptp_management_message_domain_set
 * Purpose:
 *      Set the PTP domain in PTP management message.
 * Parameters:
 *      message - (IN/OUT) PTP management message.
 *      domain  - (IN) PTP domain.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
static int 
_bcm_ptp_management_message_domain_set(
    uint8 *message, 
    uint8 domain)
{
    message[PTP_PTPHDR_START_IDX + PTP_PTPHDR_DOMAIN_OFFSET_OCTETS] = domain;
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_ptp_management_rfc791_checksum
 * Purpose:
 *      Calculate checksum of a packet per RFC 791.
 * Parameters:
 *      packet     - (IN) Packet.
 *      packet_len - (IN) Packet size (octets).
 * Returns:
 *      Checksum.
 * Notes:
 *      Caller is responsible for zeroing header checksum fields if IPv4
 *      packets are passed to the algorithm.
 */
static uint16 
_bcm_ptp_management_rfc791_checksum(
    uint8 *packet, 
    int packet_len)
{
    uint32 chksum = 0x00000000;
           
    /* 
     * CHECKSUM ALGORITHM: RFC 791, pp 14.
     *
     * "The checksum algorithm is:
     *    The checksum field is the 16 bit one's complement of the one's 
     *    complement sum of all 16 bit words in the header. For purposes of 
     *    computing the checksum, the value of the checksum field is zero."
     *
     * NOTICE: An equivalent, efficient implementation algorithm enlists 
     *         32-bit addition and handles 16-bit overflows (carries) as 
     *         a penultimate step. 
     */
   
    while (packet_len > 1)
    {
        /* Assemble words from consecutive octets and add to sum. */
        chksum += (uint32)((((uint16)packet[0]) << 8) | (packet[1]));

        /* 
         * Increment pointer in packet.
         * Decrement remaining size.
         */
        packet += 2;
        packet_len -= 2;
    }
   
    /* Process odd octet. */
    if (packet_len == 1) {
        chksum += (uint32)(((uint16)packet[0]) << 8);
    }
   
    /* Carry(ies). */
    chksum = (chksum >> 16) + (chksum & 0x0000ffff);
   
    /* 
     * Result.
     * NOTICE: 16-bit ones complement.
     */
    return (~((uint16)chksum));
}
#endif /* defined(INCLUDE_PTP)*/
