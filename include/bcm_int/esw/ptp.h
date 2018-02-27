/*
 * $Id: ptp.h 1.1.2.12 Broadcom SDK $
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

#ifndef __BCM_INT_PTP_H__
#define __BCM_INT_PTP_H__

#include <soc/defs.h>
#include <soc/drv.h>

#include <bcm/ptp.h>

#ifndef BCM_PTP_MAX_BUFFERS
/* BCM_PTP_MAX_BUFFERS: number of message buffers usable by CMICm */
#define BCM_PTP_MAX_BUFFERS (16)
#endif

#ifndef BCM_PTP_MAX_LOG
/* BCM_PTP_MAX_LOG: size of debug log from CMICm */
#define BCM_PTP_MAX_LOG (1024)
#endif

extern int bcm_esw_ptp_lock(int unit);
extern int bcm_esw_ptp_unlock(int unit);
#ifdef BCM_WARM_BOOT_SUPPORT
extern int _bcm_esw_ptp_sync(int unit);
#endif
#ifdef BCM_WARM_BOOT_SUPPORT_SW_DUMP
extern void _bcm_ptp_sw_dump(int unit);
#endif /* BCM_WARM_BOOT_SUPPORT_SW_DUMP */

#define PTP_ERROR_FUNC(func)   soc_cm_print("%s() failed " \
                               func " returned %d : %s\n", \
                               __func__, rv, bcm_errmsg(rv))
                             
#define CONFIG_RCPU_OFFSET (0)
#define CONFIG_RCPU_SIZE (4 + 4 + 4 + 4 + 4 + 4 + 6 + 2)
#define CONFIG_VLAN_OFFSET (CONFIG_RCPU_OFFSET + CONFIG_RCPU_SIZE)
#define CONFIG_VLAN_SIZE (2 + 2)
#define CONFIG_MPLS_OFFSET (CONFIG_VLAN_OFFSET + CONFIG_VLAN_SIZE)
#define CONFIG_MPLS_SIZE (2 + 2)
#define CONFIG_HOST_OFFSET (CONFIG_MPLS_OFFSET + CONFIG_MPLS_SIZE)
#define CONFIG_HOST_SIZE (6 + 2 + 6 + 2 + 4 + 4 + 2 + 2)
#define CONFIG_DEBUG_OFFSET (CONFIG_HOST_OFFSET + CONFIG_HOST_SIZE)
#define CONFIG_DEBUG_SIZE (4)
#define CONFIG_PPSOUT_OFFSET (CONFIG_DEBUG_OFFSET + CONFIG_DEBUG_SIZE)
#define CONFIG_PPSOUT_SIZE (4 + 4 + 4 + 4 + 4 + 4)
#define CONFIG_TIMESOURCE_OFFSET (CONFIG_PPSOUT_OFFSET + CONFIG_PPSOUT_SIZE)
#define CONFIG_TIMESOURCE_SIZE (4)
#define CONFIG_EXTRA_SIZE (256)

#define CONFIG_TOTAL_SIZE (CONFIG_RCPU_SIZE + CONFIG_VLAN_SIZE + CONFIG_MPLS_SIZE + \
                           CONFIG_HOST_SIZE + CONFIG_DEBUG_SIZE + CONFIG_PPSOUT_SIZE + \
                           CONFIG_TIMESOURCE_SIZE + CONFIG_EXTRA_SIZE)

#define INFO_BASE_ADDR    (0x19000bac)
#define CONFIG_BASE       (0x19000c2c)
                           
#define BOOT_STATUS_ADDR  (0x19000500)
#define FAULT_STATUS_ADDR (0x19000504)
#define MAX_BOOT_ITER (1000)    /* cycles of 1ms to wait for the ToP boot to complete */

/* 
 * All ports identifier.
 * Ref. IEEE Std. 1588-2008, Chapters 13.12.1 and 15.3.1. 
 */
#define PTP_IEEE1588_ALL_PORTS (0xffff) 

/* 
 * PTP management message actions.
 * Ref. IEEE Std. 1588-2008, Chapter 15.4.1.6, Table 38. 
 */
#define PTP_MGMTMSG_GET           (0)
#define PTP_MGMTMSG_SET           (1)
#define PTP_MGMTMSG_RESP          (2)
#define PTP_MGMTMSG_CMD           (3)
#define PTP_MGMTMSG_ACK           (4)
    
/* 
 * PTP management message ID (i.e. managementId) values.
 * Ref. IEEE Std. 1588-2008, Chapter 15.5.2.3, Table 40. 
 */
#define PTP_MGMTMSG_ID_USER_DESCRIPTION                            (0x0002)
                           
#define PTP_MGMTMSG_ID_DEFAULT_DATASET                             (0x2000)
#define PTP_MGMTMSG_ID_CURRENT_DATASET                             (0x2001)
#define PTP_MGMTMSG_ID_PARENT_DATASET                              (0x2002)
#define PTP_MGMTMSG_ID_TIME_PROPERTIES_DATASET                     (0x2003)
#define PTP_MGMTMSG_ID_PORT_DATASET                                (0x2004)
                           
#define PTP_MGMTMSG_ID_PRIORITY1                                   (0x2005)
#define PTP_MGMTMSG_ID_PRIORITY2                                   (0x2006)
#define PTP_MGMTMSG_ID_DOMAIN                                      (0x2007)                           
#define PTP_MGMTMSG_ID_SLAVE_ONLY                                  (0x2008)
#define PTP_MGMTMSG_ID_LOG_ANNOUNCE_INTERVAL                       (0x2009)
#define PTP_MGMTMSG_ID_ANNOUNCE_RECEIPT_TIMEOUT                    (0x200a)
#define PTP_MGMTMSG_ID_LOG_SYNC_INTERVAL                           (0x200b)
#define PTP_MGMTMSG_ID_VERSION_NUMBER                              (0x200c)

#define PTP_MGMTMSG_ID_ENABLE_PORT                                 (0x200d)
#define PTP_MGMTMSG_ID_DISABLE_PORT                                (0x200e)
                           
#define PTP_MGMTMSG_ID_TIME                                        (0x200f)                   
#define PTP_MGMTMSG_ID_CLOCK_ACCURACY                              (0x2010)
#define PTP_MGMTMSG_ID_UTC_PROPERTIES                              (0x2011)                           
#define PTP_MGMTMSG_ID_TRACEABILITY_PROPERTIES                     (0x2012)
#define PTP_MGMTMSG_ID_TIMESCALE_PROPERTIES                        (0x2013)
 
#define PTP_MGMTMSG_ID_UNICAST_MASTER_TABLE                        (0x2018)
#define PTP_MGMTMSG_ID_UNICAST_MASTER_MAX_TABLE_SIZE               (0x2019)

#define PTP_MGMTMSG_ID_ACCEPTABLE_MASTER_TABLE                     (0x201a)
#define PTP_MGMTMSG_ID_ACCEPTABLE_MASTER_TABLE_ENABLED             (0x201b)
#define PTP_MGMTMSG_ID_ACCEPTABLE_MASTER_MAX_TABLE_SIZE            (0x201c)
   
#define PTP_MGMTMSG_ID_TRANSPARENT_CLOCK_DEFAULT_DATASET           (0x4000)
#define PTP_MGMTMSG_ID_TRANSPARENT_CLOCK_PORT_DATASET              (0x4001)
#define PTP_MGMTMSG_ID_PRIMARY_DOMAIN                              (0x4002)
                           
#define PTP_MGMTMSG_ID_DELAY_MECHANISM                             (0x6000)
#define PTP_MGMTMSG_ID_LOG_MIN_PDELAY_REQ_INTERVAL                 (0x6001)
                           
/* 
 * Implementation-specific PTP management message IDs.
 * Hits the standard management message get/set code. 
 */
#define PTP_MGMTMSG_ID_OUTPUT_SIGNALS                              (0xdffa)
#define PTP_MGMTMSG_ID_CHANNELS                                    (0xdffb)
#define PTP_MGMTMSG_ID_PEER_DATASET_FIRST                          (0xdffc)
#define PTP_MGMTMSG_ID_PEER_DATASET_NEXT                           (0xdffd)
#define PTP_MGMTMSG_ID_FOREIGN_MASTER_DATASET                      (0xdffe)
#define PTP_MGMTMSG_ID_LOG_MIN_DELAY_REQ_INTERVAL                  (0xdfff)  

/* 
 * Implementation-specific PTP management message IDs.
 * Following hit the custom handler that does not depend on having an
 * instantiated clock or correctly configured ports.
 */
#define PTP_MGMTMSG_ID_CREATE_CLOCK_INSTANCE                       (0xd000)  
#define PTP_MGMTMSG_ID_CONFIGURE_CLOCK_PORT                        (0xd002)  
#define PTP_MGMTMSG_ID_CONFIGURE_TOD_OUT                           (0xd004)  
#define PTP_MGMTMSG_ID_CONFIGURE_TOD_IN                            (0xd005)
#define PTP_MGMTMSG_ID_SYNC_PHY                                    (0xd006)
#define PTP_MGMTMSG_ID_PORT_LATENCY                                (0xd017)
#define PTP_MGMTMSG_ID_LOG_LEVEL                                   (0xd018)

/* Management message maximum response size (octets). */
#define PTP_MGMTMSG_RESP_MAX_SIZE_OCTETS                             (1024)

/* Management message empty TLV payload size (octets). */
#define PTP_MGMTMSG_PAYLOAD_NULL_SIZE_OCTETS                            (0) 
/* Management message minimum, non-empty TLV payload size (octets). */
#define PTP_MGMTMSG_PAYLOAD_MIN_SIZE_OCTETS                             (2) 
/* Management message maximum USER_DESCIPTION TLV payload size (octets). */
#define PTP_MGMTMSG_PAYLOAD_MAX_USER_DESCRIPTION_SIZE_OCTETS           (32)
/* Management message DEFAULT_DATA_SET TLV payload size (octets). */
#define PTP_MGMTMSG_PAYLOAD_DEFAULT_DATASET_SIZE_OCTETS                (20) 
/* Management message CURRENT_DATA_SET TLV payload size (octets). */
#define PTP_MGMTMSG_PAYLOAD_CURRENT_DATASET_SIZE_OCTETS                (18)
/* Management message PARENT_DATA_SET TLV payload size (octets). */
#define PTP_MGMTMSG_PAYLOAD_PARENT_DATASET_SIZE_OCTETS                 (32) 
/* Management message TIME TLV payload size (octets). */
#define PTP_MGMTMSG_PAYLOAD_TIME_SIZE_OCTETS                           (10) 
/* Management message UTC_PROPERTIES TLV payload size (octets). */                           
#define PTP_MGMTMSG_PAYLOAD_UTC_PROPERTIES_SIZE_OCTETS                  (4) 
/* Management message UNICAST_MASTER_TABLE TLV payload size (octets). */
#define PTP_MGMTMSG_PAYLOAD_MAX_UNICAST_MASTER_TABLE_SIZE_OCTETS     (1404) 
/* Management message ACCEPTABLE_MASTER_TABLE TLV payload size (octets). */
#define PTP_MGMTMSG_PAYLOAD_MAX_ACCEPTABLE_MASTER_TABLE_SIZE_OCTETS  (1404) 
/* Management message UNICAST_SLAVE_TABLE TLV payload size (octets). */
#define PTP_MGMTMSG_PAYLOAD_MAX_UNICAST_SLAVE_TABLE_SIZE_OCTETS      (1404) 
/* Management message clock instance creation TLV payload size (octets). */
#define PTP_MGMTMSG_PAYLOAD_CLOCK_INSTANCE_SIZE_OCTETS                 (41) 
/* Management message configure clock port TLV payload size (octets). */
#define PTP_MGMTMSG_PAYLOAD_CONFIGURE_CLOCK_PORT_SIZE_OCTETS          (177) 
/* Management message PORT_LATENCY TLV payload size (octets). */
#define PTP_MGMTMSG_PAYLOAD_PORT_LATENCY_SIZE_OCTETS                   (22) 
/* Management message LOG_LEVEL TLV payload size (octets). */
#define PTP_MGMTMSG_PAYLOAD_LOG_LEVEL_SIZE_OCTETS                      (10) 
/* Management message CHANNELS TLV payload size (octets). */
#define PTP_MGMTMSG_PAYLOAD_CHANNELS_SIZE_OCTETS                       (46) 
/* Management message OUTPUT_SIGNALS TLV payload size (octets). */
#define PTP_MGMTMSG_PAYLOAD_OUTPUT_SIGNALS_SIZE_OCTETS                (102) 


/* PTP Header parameters and constants.*/
/* L2 header size (octets). */
#define PTP_L2HDR_SIZE_OCTETS                                          (12) 
/* IEEE 802.1Q, VLAN tagging header size (octets). */
#define PTP_VLANHDR_SIZE_OCTETS                                         (4) 
/*
 * VLAN identifier (VID) size (octets). 
 * NOTICE: VID is 12-bit value. 
 */
#define PTP_VLANHDR_VID_SIZE_OCTETS                                     (2) 
/* VLAN identifier (VID) offset (octets). */
#define PTP_VLANHDR_VID_OFFSET_OCTETS                                   (2) 

/* Ethertype size (octets). */
#define PTP_ETHHDR_SIZE_OCTETS                                          (2) 

/* 
 * IPv4 header start index.
 * NOTICE: Assumes single VLAN tag.
 */
#define PTP_IPV4HDR_START_IDX                  (PTP_L2HDR_SIZE_OCTETS + \
                                                PTP_VLANHDR_SIZE_OCTETS + \
                                                  PTP_ETHHDR_SIZE_OCTETS)
                                                
/* IPv4 header offset of total message length (octets). */
#define PTP_IPV4HDR_MSGLEN_OFFSET_OCTETS                                (2) 
/* IPv4 header offset of source IP address (octets). */
#define PTP_IPV4HDR_SRC_IPADDR_OFFSET_OCTETS                           (12) 
/* IPv4 header offset of header checksum (octets). */
#define PTP_IPV4HDR_HDR_CHKSUM_OFFSET_OCTETS                           (10) 
/* 
 * IPv4 header size (octets). 
 * ASSUMPTION: Field 13 (Options) omitted. 
 */
#define PTP_IPV4HDR_SIZE_OCTETS                                        (20) 

#define PTP_UDPHDR_START_IDX                     (PTP_IPV4HDR_START_IDX + \
                                                  PTP_IPV4HDR_SIZE_OCTETS)
                                                  
/* UDP header size (octets). */
#define PTP_UDPHDR_SIZE_OCTETS                                          (8) 
/* UDP header offset of destination port (octets). */
#define PTP_UDPHDR_DESTPORT_OFFSET_OCTETS                               (2) 
/* UDP header offset of message length (octets). */
#define PTP_UDPHDR_MSGLEN_OFFSET_OCTETS                                 (4) 
/* UDP header offset of checksum (octets). */
#define PTP_UDPHDR_CHKSUM_OFFSET_OCTETS                                 (6) 

#define PTP_PTPHDR_START_IDX                      (PTP_UDPHDR_START_IDX + \
                                                   PTP_UDPHDR_SIZE_OCTETS)
/* PTP header size (octets). */
#define PTP_PTPHDR_SIZE_OCTETS                                         (34) 
/* PTP header offset of PTP message length (octets). */
#define PTP_PTPHDR_MSGLEN_OFFSET_OCTETS                                 (2) 
/* PTP header offset of PTP domain (octets). */
#define PTP_PTPHDR_DOMAIN_OFFSET_OCTETS                                 (4) 
/* PTP header offset of source port identity (octets). */
#define PTP_PTPHDR_SRCPORT_OFFSET_OCTETS                               (20) 
/* PTP header offset of PTP sequence ID (octets). */
#define PTP_PTPHDR_SEQID_OFFSET_OCTETS                                 (30) 

#define PTP_MGMTHDR_START_IDX                     (PTP_PTPHDR_START_IDX + \
                                                   PTP_PTPHDR_SIZE_OCTETS)
                                                   
/* Management header size (octets). */
#define PTP_MGMTHDR_SIZE_OCTETS                                        (14) 
/* Management header offset of management message actions (octets). */
#define PTP_MGMTHDR_ACTION_OFFSET_OCTETS                               (12) 

/* 
 * Message TLV header size (octets).  
 * NOTICE: Excludes message payload. 
 */
#define PTP_TLVHDR_SIZE_OCTETS                                          (6) 
/* Message TLV header offset of message TLV type (octets). */
#define PTP_TLVHDR_TYP_OFFSET_OCTETS                                    (0) 
/* Message TLV header offset of message TLV length (octets). */
#define PTP_TLVHDR_LEN_OFFSET_OCTETS                                    (2) 
/* 
 * Message TLV header offset of message TLV value prefix, 
 * i.e. management ID (octets). 
 */
#define PTP_TLVHDR_VAL_PREFIX_OFFSET_OCTETS                             (4) 
/* Message TLV header TLV value prefix size (octets). */
#define PTP_TLVHDR_VAL_PREFIX_SIZE_OCTETS                               (2) 

/* Management message maximum size (octets). */
#define PTP_MGMTMSG_TOTAL_SIZE_OCTETS                                (1500) 

#define PTP_MGMTMSG_TOTAL_HEADER_SIZE          (PTP_MGMTHDR_START_IDX + \
                                                PTP_MGMTHDR_SIZE_OCTETS + \
                                                PTP_TLVHDR_SIZE_OCTETS)

/* Payload maximum size (octets). */
#define PTP_MGMTMSG_PAYLOAD_MAX_SIZE_OCTETS  (PTP_MGMTMSG_TOTAL_SIZE_OCTETS - ( \
                                              PTP_L2HDR_SIZE_OCTETS + \
                                              PTP_VLANHDR_SIZE_OCTETS + \
                                              PTP_ETHHDR_SIZE_OCTETS + \
                                              PTP_IPV4HDR_SIZE_OCTETS + \
                                              PTP_UDPHDR_SIZE_OCTETS + \
                                              PTP_PTPHDR_SIZE_OCTETS + \
                                              PTP_MGMTHDR_SIZE_OCTETS + \
                                              PTP_TLVHDR_SIZE_OCTETS))
                                                 

#define PTP_SIGHDR_START_IDX                 (PTP_PTPHDR_START_IDX)
#define PTP_SIGHDR_TGTCLOCK_OFFSET_OCTETS    (PTP_PTPHDR_SIZE_OCTETS)
#define PTP_SIGHDR_TGTPORT_OFFSET_OCTETS     (PTP_PTPHDR_SIZE_OCTETS + \
                                              BCM_PTP_CLOCK_EUID_IEEE1588_SIZE)
/* 
 * PTP signaling message header size (octets).
 * NOTICE: Includes PTP header and target port identity. 
 * Ref. IEEE Std. 1588-2008, Sect. 13.12.2.
 */
#define PTP_SIGHDR_SIZE_OCTETS       (PTP_PTPHDR_SIZE_OCTETS + \
                                      BCM_PTP_CLOCK_EUID_IEEE1588_SIZE + 2)
                                                    

#define PTP_UCSIG_TLV_START_IDX                (PTP_L2HDR_SIZE_OCTETS + \
                                                PTP_VLANHDR_SIZE_OCTETS + \
                                                PTP_ETHHDR_SIZE_OCTETS + \
                                                PTP_IPV4HDR_SIZE_OCTETS + \
                                                PTP_UDPHDR_SIZE_OCTETS + \
                                                PTP_SIGHDR_SIZE_OCTETS)
/* 
 * PTP unicast transmission signaling message 
 * REQUEST_UNICAST_TRANSMISSION TLV size (octets). 
 */
#define PTP_UCSIG_REQUEST_TLV_SIZE_OCTETS   (PTP_MAX_NETW_ADDR_SIZE + \
                                             PTP_MAX_L2_HEADER_LENGTH + 12)
/* 
 * PTP unicast transmission signaling message 
 * CANCEL_UNICAST_TRANSMISSION TLV size (octets).
 */
#define PTP_UCSIG_CANCEL_TLV_SIZE_OCTETS       (PTP_MAX_NETW_ADDR_SIZE + 7)    

/* 
 * PTP unicast transmission signaling message TLV 
 * offset of TLV length (octets). 
 */
#define PTP_UCSIG_TLVLEN_OFFSET_OCTETS                                  (2)
/* 
 * PTP unicast transmission signaling message TLV 
 * offset of message type (octets). 
 */
#define PTP_UCSIG_MSGTYPE_OFFSET_OCTETS                                 (4)
/* 
 * PTP unicast transmission signaling message TLV 
 * offset of interval parameter (octets). 
 */
#define PTP_UCSIG_INTERVAL_OFFSET_OCTETS                                (5)
/* 
 * PTP unicast transmission signaling message TLV
 * offset of duration parameter (octets). 
 */
#define PTP_UCSIG_DURATION_OFFSET_OCTETS                                (6)
/* 
 * PTP unicast transmission signaling message TLV 
 * offset of address type (octets). 
 */
#define PTP_UCSIG_ADDRTYPE_OFFSET_OCTETS                               (10)
/* 
 * PTP unicast transmission signaling message TLV 
 * offset of peer address (octets). 
 */
#define PTP_UCSIG_PEERADDR_OFFSET_OCTETS                               (11)
/* 
 * PTP unicast transmission signaling message TLV 
 * offset of L2 header length value (octets). 
 */
#define PTP_UCSIG_L2HDRLEN_OFFSET_OCTETS                               (27)
/* 
 * PTP unicast transmission signaling message TLV 
 * offset of L2 header (octets).
 */
#define PTP_UCSIG_L2HDR_OFFSET_OCTETS                                  (28)   

/* 
 * PTP cancel unicast transmission signaling message TLV 
 * offset of reserved field (octets). 
 */
#define PTP_UCSIG_CANCEL_RESERVED_OFFSET_OCTETS                         (5)
/* 
 * PTP cancel unicast transmission signaling message TLV 
 * offset of address type (octets). 
 * NOTICE: Address type and peer address data are custom 
 *         additions to TLV data.
 */
#define PTP_UCSIG_CANCEL_ADDRTYPE_OFFSET_OCTETS                         (6)
/* 
 * PTP cancel unicast transmission signaling message TLV 
 * offset of peer address (octets). 
 * NOTICE: Address type and peer address data are custom 
 *         additions to TLV data.
 */
#define PTP_UCSIG_CANCEL_PEERADDR_OFFSET_OCTETS                         (7)

/* 
 * PTP unicast transmission signaling message 
 * REQUEST_UNICAST_TRANSMISSION TLV total size (octets).
 */
#define PTP_UCSIG_REQUEST_TOTAL_SIZE_OCTETS  (PTP_UCSIG_TLV_START_IDX + \
                                              PTP_UCSIG_REQUEST_TLV_SIZE_OCTETS)
                
/* 
 * PTP unicast transmission signaling message 
 * CANCEL_UNICAST_TRANSMISSION TLV total size (octets).
 */
#define PTP_UCSIG_CANCEL_TOTAL_SIZE_OCTETS   (PTP_UCSIG_TLV_START_IDX + \
                                              PTP_UCSIG_CANCEL_TLV_SIZE_OCTETS)
   
/* PTP clock presets. */
#define PTP_CLOCK_PRESETS_LOG_MIN_DELAY_REQ_INTERVAL_MINIMUM           (-7)
#define PTP_CLOCK_PRESETS_LOG_MIN_DELAY_REQ_INTERVAL_DEFAULT           (-5)
#define PTP_CLOCK_PRESETS_LOG_MIN_DELAY_REQ_INTERVAL_MAXIMUM            (5)
   
#define PTP_CLOCK_PRESETS_ANNOUNCE_RECEIPT_TIMEOUT_MINIMUM              (2)
#define PTP_CLOCK_PRESETS_ANNOUNCE_RECEIPT_TIMEOUT_DEFAULT              (3)
#define PTP_CLOCK_PRESETS_ANNOUNCE_RECEIPT_TIMEOUT_MAXIMUM             (10)

#define PTP_CLOCK_PRESETS_LOG_ANNOUNCE_INTERVAL_MINIMUM                 (0)
#define PTP_CLOCK_PRESETS_LOG_ANNOUNCE_INTERVAL_DEFAULT                 (1)
#define PTP_CLOCK_PRESETS_LOG_ANNOUNCE_INTERVAL_MAXIMUM                 (4)
   
#define PTP_CLOCK_PRESETS_LOG_SYNC_INTERVAL_MINIMUM                    (-7)
#define PTP_CLOCK_PRESETS_LOG_SYNC_INTERVAL_DEFAULT                    (-5)
#define PTP_CLOCK_PRESETS_LOG_SYNC_INTERVAL_MAXIMUM                     (1)
                     
/* PTP clock MAC address size (bytes). */                           
#define PTP_MAC_ADDR_SIZE_BYTES   (6) 
/* PTP clock IPv4 address size. */
#define PTP_IPV4_ADDR_SIZE_BYTES  (4) 
/* PTP clock IPv6 address size. */                           
#define PTP_IPV6_ADDR_SIZE_BYTES (16) 
/* Maximum size of address (octets). */                           
#define PTP_MAX_NETW_ADDR_SIZE   (16) 
/* Maximum length of packet header before IPv4/IPv6 header. */                           
#define PTP_MAX_L2_HEADER_LENGTH (64)  
               
/* 
 * Maximum number of entries in a unicast master table of a PTP clock.
 * NOTICE: Corresponds to k_MAX_UC_MST_TBL_SZE in PTP stack firmware.
 */
#define PTP_MAX_UNICAST_MASTER_TABLE_ENTRIES                           (10)
/* Maximum number of entries in an acceptable master table of a PTP clock. */
#define PTP_MAX_ACCEPTABLE_MASTER_TABLE_ENTRIES                        (10)
/* Maximum number of entries in a unicast slave table of a PTP clock port. */
#define PTP_MAX_UNICAST_SLAVE_TABLE_ENTRIES                            (40)               
    
/* Infinite unicast message transmission duration. */
#define PTP_UNICAST_NEVER_EXPIRE (0xffffffff) 
                           
typedef enum _bcm_ptp_memstate_e 
{
    PTP_MEMSTATE_FAILURE     = -1,
    PTP_MEMSTATE_STARTUP     =  0,
    PTP_MEMSTATE_INITIALIZED =  1,
} _bcm_ptp_memstate_t;

/* dispatchable: initialize the transport mechanism to the ToP */
typedef int _bcm_ptp_transport_init_t(
    int unit, 
    bcm_ptp_stack_id_t ptp_id,
    bcm_mac_t *src_mac,
    bcm_mac_t *dest_mac,
    bcm_ip_t src_ip,
    bcm_ip_t dest_ip,
    uint16 vlan,
    uint8 prio,
    bcm_pbmp_t top_bitmap);

/* dispatchable: transmit a command to the ToP */
typedef int _bcm_ptp_transport_tx_t(
    int unit,
    bcm_ptp_stack_id_t ptp_id,
    uint8 *header,
    int header_len,
    uint8 *data,
    int data_len);

/* dispatchable: free a buffer containing a received response */
typedef int _bcm_ptp_transport_rx_free_t(
    int unit,
    bcm_ptp_stack_id_t ptp_id,
    uint8 *data);

/* dispatchable: tear down the transport mechanism to the ToP */
typedef int _bcm_ptp_transport_terminate_t(
    int unit,
    bcm_ptp_stack_id_t ptp_id);


/****************************************************/
/* constants shared with firmware */

#define MBOX_STATUS_EMPTY                    (0)
#define MBOX_STATUS_ATTN_TOP_CMD             (1)
#define MBOX_STATUS_ATTN_TOP_TUNNEL_TO_TOP   (2)
#define MBOX_STATUS_ATTN_TOP_TUNNEL_EXTERNAL (3)
#define MBOX_STATUS_ATTN_HOST_RESP           (4)
#define MBOX_STATUS_ATTN_HOST_EVENT          (5)
#define MBOX_STATUS_ATTN_HOST_TUNNEL         (6)
#define MBOX_STATUS_PENDING_TOP              (7)  // being processed on TOP


/****************************************************/
/* structures shared with firmware */

/*   mbox 0 is for cmd/resp.  Others are for async event & msg to host */
typedef struct _bcm_ptp_internal_stack_mbox_s {
    volatile uint32 num_buffers;                     /* == BCM_PTP_MAX_BUFFERS */
    volatile uint32 status[BCM_PTP_MAX_BUFFERS];     /* MBOX_STATUS_x value */
    volatile uint32 clock_num[BCM_PTP_MAX_BUFFERS];  /* clock number that the message is addressed to/from */
    volatile uint32 data_len[BCM_PTP_MAX_BUFFERS];
    volatile uint8 data[BCM_PTP_MAX_BUFFERS][1536];
} _bcm_ptp_internal_stack_mbox_t;

typedef struct _bcm_ptp_internal_stack_log_s {
    volatile uint32 size;                         /* allocated size of buf (below)              */
    volatile uint32 head;                         /* host-read, updated by CMICm                */
    uint32 tail;                                  /* host-read/write.  not updated by CMICm     */
    volatile uint8 buf[4];                        /* '4' is placeholder.  Actual size is 'size' */
} _bcm_ptp_internal_stack_log_t;

typedef struct _bcm_ptp_internal_stack_state_s {
    /* network-byte-ordered pointers to the shared structures */
    volatile uint32 mbox_ptr;
    volatile uint32 log_ptr;

    _bcm_ptp_internal_stack_mbox_t *  mbox;
    _bcm_ptp_internal_stack_log_t * log;

    /* Elements after this point are not accessed by firmware */
    int core_num;
} _bcm_ptp_internal_stack_state_t;

/****************************************************/


/* PTP stack information. */ 
typedef struct _bcm_ptp_stack_info_s
{
    int in_use;
    void *cookie;
    _bcm_ptp_memstate_t initialized;
    bcm_ptp_external_stack_info_t ext_info;
    _bcm_ptp_internal_stack_state_t int_state;

    int unit;
    bcm_ptp_stack_id_t stack_id;
    uint32 system_debug_flags;
    uint8 persistent_config[CONFIG_TOTAL_SIZE];


    /* dispatch table */
    _bcm_ptp_transport_init_t *transport_init;
    _bcm_ptp_transport_tx_t *tx;
    _bcm_ptp_transport_rx_free_t *rx_free;
    _bcm_ptp_transport_terminate_t *transport_terminate;

} _bcm_ptp_stack_info_t;

#define PTP_MAX_STACKS_PER_UNIT       (1)
#define PTP_MAX_CLOCKS_PER_STACK      (1)
#define PTP_MAX_CLOCK_INSTANCES       (1)
#define PTP_MAX_CLOCK_INSTANCE_PORTS  (32)
#define PTP_MAX_ORDINARY_CLOCK_PORTS  (1)
#define PTP_MAX_OUTPUT_SIGNALS        (6)

#define PTP_UNIT_NUMBER_DEFAULT       (0)
#define PTP_STACK_ID_DEFAULT          (0)
#define PTP_CLOCK_NUMBER_DEFAULT      (0)
#define PTP_CLOCK_PORT_NUMBER_DEFAULT (1)

typedef struct _bcm_ptp_info_s
{
    _bcm_ptp_memstate_t initialized;
    sal_mutex_t mutex;
    _bcm_ptp_stack_info_t stack_info[PTP_MAX_STACKS_PER_UNIT];
} _bcm_ptp_info_t;

extern _bcm_ptp_info_t _bcm_esw_ptp_info[BCM_MAX_NUM_UNITS];
#define SET_PTP_INFO ptp_info_p = &_bcm_esw_ptp_info[unit]

/* PTP clock and clock port information cache. */
typedef struct _bcm_ptp_clock_cache_s
{
    bcm_ptp_clock_info_t  clock_info;
    bcm_ptp_signal_output_t signal_output[PTP_MAX_OUTPUT_SIGNALS];
    bcm_ptp_clock_port_info_t port_info[PTP_MAX_CLOCK_INSTANCE_PORTS];
} _bcm_ptp_clock_cache_t;

/* PTP stack clock and port information caches. */ 
typedef struct _bcm_ptp_stack_cache_s
{
    _bcm_ptp_memstate_t memstate;
    _bcm_ptp_clock_cache_t *clock_array;
} _bcm_ptp_stack_cache_t;

/* Unit clock and port information caches. */
typedef struct _bcm_ptp_unit_cache_s
{
    _bcm_ptp_memstate_t memstate;
    _bcm_ptp_stack_cache_t *stack_array;
} _bcm_ptp_unit_cache_t;

/* 
 * PTP state enumeration.
 * Ref. IEEE Std. 1588-2008, Chapter 8.2.5.3.1.
 */
typedef enum _bcm_ptp_port_state_e {
    _bcm_ptp_state_initializing = 1, 
    _bcm_ptp_state_faulty       = 2,
    _bcm_ptp_state_disabled     = 3,
    _bcm_ptp_state_listening    = 4,
    _bcm_ptp_state_pre_master   = 5,
    _bcm_ptp_state_master       = 6,
    _bcm_ptp_state_passive      = 7,
    _bcm_ptp_state_uncalibrated = 8,
    _bcm_ptp_state_slave        = 9
} _bcm_ptp_port_state_t;

/*
 * PTP messageType enumeration.
 * Ref. IEEE Std. 1588-2008, Chapter 13.3.2.2.
 */
typedef enum _bcmPTPmessageType_e {
    /* Sync message */
    _bcmPTPmessageType_sync                  = 0x00, 
    /* Delay request message */
    _bcmPTPmessageType_delay_req             = 0x01, 
    /* Peer delay request message */
    _bcmPTPmessageType_pdelay_req            = 0x02, 
    /* Peer delay response message */
    _bcmPTPmessageType_pdelay_resp           = 0x03, 
    /* Follow-up message */
    _bcmPTPmessageType_follow_up             = 0x08, 
    /* Delay response message */
    _bcmPTPmessageType_delay_resp            = 0x09, 
    /* Peer delay response follow-up message */
    _bcmPTPmessageType_pdelay_resp_follow_up = 0x0a,
    /* Announce message */
    _bcmPTPmessageType_announce              = 0x0b, 
    /* Signaling message */
    _bcmPTPmessageType_signaling             = 0x0c, 
    /* Management message */
    _bcmPTPmessageType_management            = 0x0d  
} _bcmPTPmessageType_t;

/* 
 * PTP tlvType enumeration.
 * Ref. IEEE Std. 1588-2008, Chapter 14.1.1.
 */
typedef enum _bcmPTPtlvType_e {
    /* Management TLV type */
    _bcmPTPtlvType_management                      = 0x0001, 
    /* Management error status TLV type */
    _bcmPTPtlvType_management_error_status         = 0x0002, 
    /* Organization extension TLV type */
    _bcmPTPtlvType_organization_extension          = 0x0003, 
    /* Request unicast transmission TLV type */
    _bcmPTPtlvType_request_unicast_transmission    = 0x0004, 
    /* Grant unicast transmission TLV type */
    _bcmPTPtlvType_grant_unicast_transmission      = 0x0005, 
    /* Cancel unicast transmission TLV type */
    _bcmPTPtlvType_cancel_unicast_transmission     = 0x0006, 
    /* Acknowledge cancel unicast transmission TLV type */
    _bcmPTPtlvType_ack_cancel_unicast_transmission = 0x0007  
} _bcmPTPtlvType_t;

/* 
 * PTP Boot type enumeration.
 */
typedef enum _bcmPTPBootType_e {
    BCM_PTP_COLDSTART,
    BCM_PTP_WARMSTART
} _bcmPTPBootType_t;

typedef enum bcm_ptp_callback_e {
   PTP_CALLBACK_REJECT = 0,
   PTP_CALLBACK_ACCEPT = 1
} _bcm_ptp_callback_t;

/* Enumeration of events which can generate a callback */
typedef enum _bcm_ptp_event_e {
    _bcm_ptp_state_change_event    = 0x01,
    _bcm_ptp_master_change_event   = 0x02,
    _bcm_ptp_master_avail_event    = 0x03,
    _bcm_ptp_master_unavail_event  = 0x04,
    _bcm_ptp_slave_avail_event     = 0x05,
    _bcm_ptp_slave_unavail_event   = 0x06,
    _bcm_ptp_top_oom_event         = 0x07,
    _bcm_ptp_top_watchdog_event    = 0x08,
    _bcm_ptp_top_ready_event       = 0x09,
    _bcm_ptp_top_misc_event        = 0x0a,
    _bcm_ptp_top_tod_avail_event   = 0x0b,
    _bcm_ptp_top_tod_unavail_event = 0x0c,
    _bcm_ptp_ieee1588_warn_event   = 0x0d, 
    _bcm_ptp_servo_state_event     = 0x0e,
    _bcm_ptp_pps_in_state_event    = 0x0f,
    _bcm_ptp_last_event
} _bcm_ptp_event_t;
   
/* 
 * Event support (STATE CHANGE). 
 * Reason for a change in PTP port state.
 */
typedef enum _bcm_ptp_state_change_reason_e {
   /* Startup, instance creation. */
   _bcm_ptp_state_change_reason_startup         = 0, 
   /* Port initialization. */
   _bcm_ptp_state_change_reason_port_init       = 1, 
   /* Fault detected. */
   _bcm_ptp_state_change_reason_fault           = 2, 
   /* BMCA state transition. */
   _bcm_ptp_state_change_reason_bmca            = 3, 
   /* Enable port management message. */
   _bcm_ptp_state_change_reason_mnt_enable      = 4, 
   /* Disable port management message. */
   _bcm_ptp_state_change_reason_mnt_disable     = 5, 
   /* Network interface re-initialization. */
   _bcm_ptp_state_change_reason_netw_reinit     = 6, 
   /* Timestamp difference, master-to-slave. */
   _bcm_ptp_state_change_reason_dt_master_slave = 7, 
   _bcm_ptp_state_change_reason_last
} _bcm_ptp_state_change_reason_t;

/* 
 * Event support (IEEE Std. 1588-2008 WARNING).
 * Reason for a warning related to IEEE Std. 1588-2008.
 */
typedef enum _bcm_ptp_ieee1588_warn_reason_e {
   /* Non-uniform logAnnounceInterval in a PTP domain. */
   _bcm_ptp_ieee1588_warn_reason_logAnnounceInterval = 0, 
   _bcm_ptp_ieee1588_warn_reason_last
} _bcm_ptp_ieee1588_warn_reason_t;

/* Event support (SERVO STATE). */
typedef enum _bcm_ptp_fll_state_e {
    /* FLL Acquiring lock */
    _bcm_ptp_fll_state_acquiring = 0,   
    /* FLL warmup state   */
    _bcm_ptp_fll_state_warmup    = 1,
    /* FLL fast loop      */
    _bcm_ptp_fll_state_fast      = 2,    
    /* FLL normal loop    */
    _bcm_ptp_fll_state_normal    = 3,
    /* FLL bridge state   */
    _bcm_ptp_fll_state_bridge    = 4,
    /* FLL Holdover state */
    _bcm_ptp_fll_state_holdover  = 5     
} _bcm_ptp_fll_state_t;

/* Event support (PPS-IN STATE). */
typedef enum _bcm_ptp_pps_in_state_e {
   _bcm_ptp_pps_in_state_missing            = 0,
   _bcm_ptp_pps_in_state_active_missing_tod = 1,
   _bcm_ptp_pps_in_state_valid              = 2
} _bcm_ptp_pps_in_state_t;

/* PTP PCI setconfig */
typedef int (*bcm_ptp_pci_setconfig_t) (uint32 pciconfig_register, uint32 value);

extern int _bcm_ptp_external_stack_create(
    int unit, 
    bcm_ptp_stack_info_t *info,
    bcm_ptp_stack_id_t ptp_id);

extern int _bcm_ptp_internal_stack_create(
    int unit, 
    bcm_ptp_stack_info_t *info,
    bcm_ptp_stack_id_t ptp_id);

extern int
_bcm_ptp_clock_cache_init(
    int unit);

extern int
_bcm_ptp_clock_cache_stack_create(
    int unit,
    bcm_ptp_stack_id_t ptp_id);

extern int
_bcm_ptp_clock_cache_info_create(
    int unit,
    bcm_ptp_stack_id_t ptp_id,
    int clock_num);

extern int _bcm_ptp_clock_cache_info_get(
    int unit, 
    bcm_ptp_stack_id_t ptp_id,
    int clock_num,
    bcm_ptp_clock_info_t *clock_info);

extern int _bcm_ptp_clock_cache_signal_get(
    int unit, 
    bcm_ptp_stack_id_t ptp_id,
    int clock_num,
    int signal_num,
    bcm_ptp_signal_output_t *signal);

extern int _bcm_ptp_clock_cache_signal_set(
    int unit, 
    bcm_ptp_stack_id_t ptp_id,
    int clock_num,
    int signal_num,
    const bcm_ptp_signal_output_t *signal);

extern int _bcm_ptp_clock_cache_info_set(
    int unit, 
    bcm_ptp_stack_id_t ptp_id,
    int clock_num,
    const bcm_ptp_clock_info_t clock_info);

extern int _bcm_ptp_clock_port_cache_info_get(
    int unit, 
    bcm_ptp_stack_id_t ptp_id,
    int clock_num,
    uint32 clock_port,
    bcm_ptp_clock_port_info_t *port_info);

extern int _bcm_ptp_clock_port_cache_info_set(
    int unit, 
    bcm_ptp_stack_id_t ptp_id,
    int clock_num,
    uint32 clock_port,
    const bcm_ptp_clock_port_info_t port_info);

extern int _bcm_ptp_rx_init(
    int unit);

extern int _bcm_ptp_rx_stack_create(
    int unit, 
    bcm_ptp_stack_id_t ptp_id,
    bcm_mac_t *host_mac,
    bcm_mac_t *top_mac,
    int tpid,
    int vlan);

extern int _bcm_ptp_rx_clock_create(
    int unit, 
    bcm_ptp_stack_id_t ptp_id,
    int clock_num);

extern int _bcm_ptp_rx_response_get(
    int unit,
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num, 
    int usec, 
    uint8 **data, 
    int *data_len);

extern int _bcm_ptp_external_rx_response_free(
    int unit,
    int ptp_id,
    uint8 *resp_data);

extern int _bcm_ptp_internal_rx_response_free(
    int unit,
    int ptp_id,
    uint8 *resp_data);

extern int _bcm_ptp_rx_response_flush(
    int unit,
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num);

extern int _bcm_ptp_management_init(
    int unit);

extern int _bcm_ptp_management_stack_create(
    int unit, 
    bcm_ptp_stack_id_t ptp_id,
    bcm_mac_t *src_mac,
    bcm_mac_t *dest_mac,
    bcm_ip_t src_ip,
    bcm_ip_t dest_ip,
    uint16 vlan,
    uint8 prio,
    bcm_pbmp_t top_bitmap);

extern int _bcm_ptp_internal_management_stack_create(
    int unit, 
    bcm_ptp_stack_id_t ptp_id);

extern int _bcm_ptp_management_clock_create(
    int unit, 
    bcm_ptp_stack_id_t ptp_id,
    int clock_num);

extern int _bcm_ptp_management_message_send(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num, 
    const bcm_ptp_port_identity_t* dest_clock_port, 
    uint8 action,
    uint16 cmd, 
    uint8* payload, 
    int payload_len,
    uint8* resp, 
    int* resp_len);

extern int _bcm_ptp_unicast_slave_subscribe(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num, 
    uint32 clock_port, 
    bcm_ptp_clock_peer_t *slave_info,
    _bcmPTPmessageType_t msg_type,
    _bcmPTPtlvType_t tlv_type,
    int interval);

extern int _bcm_ptp_management_domain_get(
    int unit, 
    bcm_ptp_stack_id_t ptp_id,
    int clock_num,
    uint8 *domain);

extern int _bcm_ptp_management_domain_set(
    int unit, 
    bcm_ptp_stack_id_t ptp_id,
    int clock_num,
    uint8 domain);

extern int _bcm_ptp_tunnel_message_to_top(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    unsigned payload_len,
    uint8 *payload);

extern int _bcm_ptp_unicast_master_table_init(
    int unit);

extern int _bcm_ptp_unicast_master_table_stack_create(
    int unit, 
    bcm_ptp_stack_id_t ptp_id);

extern int _bcm_ptp_unicast_master_table_clock_create(
    int unit, 
    bcm_ptp_stack_id_t ptp_id,
    int clock_num);

extern int _bcm_ptp_unicast_slave_table_init(
    int unit);

extern int _bcm_ptp_unicast_slave_table_stack_create(
    int unit, 
    bcm_ptp_stack_id_t ptp_id);

extern int _bcm_ptp_unicast_slave_table_clock_create(
    int unit, 
    bcm_ptp_stack_id_t ptp_id,
    int clock_num);

extern int _bcm_ptp_acceptable_master_table_init(
    int unit);

extern int _bcm_ptp_acceptable_master_table_stack_create(
    int unit, 
    bcm_ptp_stack_id_t ptp_id);

extern int _bcm_ptp_acceptable_master_table_clock_create(
    int unit, 
    bcm_ptp_stack_id_t ptp_id,
    int clock_num);

extern uint16 _bcm_ptp_uint16_read(
    uint8* buffer);

extern uint32 _bcm_ptp_uint32_read(
    uint8* buffer);

extern uint64 _bcm_ptp_uint64_read(
    uint8* buffer);

extern void _bcm_ptp_uint16_write(
    uint8* buffer, 
    const uint16 value);

extern void _bcm_ptp_uint32_write(
    uint8* buffer, 
    const uint32 value);

extern void _bcm_ptp_uint64_write(
    uint8* buffer, 
    const uint64 value);

extern int _bcm_ptp_function_precheck(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num, 
    uint32 clock_port);

extern int _bcm_ptp_clock_lookup(
    const bcm_ptp_clock_identity_t clock_identity,
    int *unit,
    bcm_ptp_stack_id_t *ptp_id,
    int *clock_num);

extern int _bcm_ptp_peer_address_compare(
    const bcm_ptp_clock_peer_address_t *a, 
    const bcm_ptp_clock_peer_address_t *b);

extern void _bcm_ptp_dump_hex(
    uint8 *buf, 
    int len);

/* Prototypes for esw functions */
extern void esw_set_ext_stack_config_uint32(
    _bcm_ptp_stack_info_t *stack_p, 
    uint32 offset, uint32 value);

extern void esw_set_ext_stack_config_array(
    _bcm_ptp_stack_info_t* stack_p, 
    uint32 offset, 
    const uint8 * array, 
    int len);

extern void _bcm_ptp_read_pcishared_uint8_aligned_array(
    bcm_ptp_external_stack_info_t *stack_p, 
    uint32 addr, 
    uint8 * array, 
    int array_len);

extern void _bcm_ptp_read_pcishared_uint8_aligned_array(
    bcm_ptp_external_stack_info_t *stack_p, 
    uint32 addr, 
    uint8 * array, 
    int array_len);

extern void _bcm_ptp_write_pcishared_uint8_aligned_array(
    bcm_ptp_external_stack_info_t *stack_p, 
    uint32 addr, 
    uint8 * array, 
    int array_len);

extern int _bcm_ptp_write_pcishared_uint32(
    void *cookie, 
    uint32 addr, 
    uint32 val);

extern int _bcm_ptp_read_pcishared_uint32(
    void *cookie, 
    uint32 addr, 
    uint32 *val);

extern int bcm_esw_ptp_clock_port_identity_get(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num, 
    uint32 clock_port, 
    bcm_ptp_port_identity_t *identity);

extern int bcm_esw_ptp_clock_time_get(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num,
    bcm_ptp_timestamp_t *time);

extern int _bcm_ptp_register_management_callback(
    int unit, 
    bcm_ptp_cb cb, 
    void *user_data);

extern int _bcm_ptp_register_event_callback(
    int unit, 
    bcm_ptp_cb cb, 
    void *user_data);

extern int _bcm_ptp_register_fault_callback(
    int unit, 
    bcm_ptp_cb cb, 
    void *user_data);

extern int _bcm_ptp_register_signal_callback(
    int unit, 
    bcm_ptp_cb cb, 
    void *user_data);

extern int _bcm_ptp_unregister_management_callback(
    int unit);

extern int _bcm_ptp_unregister_event_callback(
    int unit);

extern int _bcm_ptp_unregister_fault_callback(
    int unit);

extern int _bcm_ptp_unregister_signal_callback(
    int unit);

/* external-specific transport functions */
extern _bcm_ptp_transport_init_t _bcm_ptp_external_transport_init;
extern _bcm_ptp_transport_tx_t _bcm_ptp_external_tx;
extern _bcm_ptp_transport_terminate_t _bcm_ptp_external_transport_terminate;

/* internal-specific transport functions */
extern _bcm_ptp_transport_init_t _bcm_ptp_internal_transport_init;
extern _bcm_ptp_transport_tx_t _bcm_ptp_internal_tx;
extern _bcm_ptp_transport_terminate_t _bcm_ptp_internal_transport_terminate;

/* For use by diag shell */
extern _bcm_ptp_unit_cache_t _bcm_esw_ptp_unit_array[BCM_MAX_NUM_UNITS];

/* For use by diag shell */
extern int _bcm_esw_ptp_log_level_set(int unit, bcm_ptp_stack_id_t ptp_id, 
                                      int clock_num, uint32 level);

/* For use by diag shell */
extern int _bcm_esw_ptp_input_channels_get(int unit, bcm_ptp_stack_id_t ptp_id,
                                           int clock_num, int *num_channels,
                                           bcm_ptp_channel_t *channels);

#endif /* !__BCM_INT_PTP_H__ */
