/*
 * $Id: counter.c 1.4.2.1 Broadcom SDK $
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
 * Packet Statistics Counter Management
 *
 */
#include <soc/drv.h>
#include <soc/debug.h>
#include <soc/counter.h>
#include <soc/robo_stat.h>
/*
 *  Function : drv_counter_set
 *
 *  Purpose :
 *      Set the snmp counter value.
 *
 *  Parameters :
 *      uint    :   uint number.
 *      port        :   port bitmap.
 *      counter_type   :   counter_type.
 *                  If want clear all counters associated to this port, 
 *                  counter_type = DRV_SNMP_COUNTER_COUNT
 *      val  :   counter val.
 *              Now only support to set zero.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */
int
drv_bcm5396_counter_set(int unit, soc_pbmp_t bmp, uint32 counter_type, uint64 val)
{
    int     rv = SOC_E_NONE;
    uint32 port;

    if (soc_feature(unit, soc_feature_no_stat_mib)) {
        return SOC_E_UNAVAIL;
    }
    if (!COMPILER_64_IS_ZERO(val)){
        return SOC_E_UNAVAIL;
    }

    /* set by port */
    if (counter_type == DRV_SNMP_VAL_COUNT) {
        rv = soc_robo_counter_set_by_port(unit, bmp, val);
        return rv;
    }

     PBMP_ITER(bmp, port) {
        switch (counter_type) {
            /* *** RFC 1213 *** */
            case DRV_SNMP_IF_IN_OCTETS:       	
                return SOC_E_UNAVAIL;
            break;

            case DRV_SNMP_IF_IN_UCAST_PKTS:     
                return SOC_E_UNAVAIL;
            break;

            case DRV_SNMP_IF_IN_N_UCAST_PKTS:    
                return SOC_E_UNAVAIL;
            break;

            case DRV_SNMP_IF_IN_DISCARDS:
                return SOC_E_UNAVAIL;
            break;

            case DRV_SNMP_IF_IN_ERRORS:	
                return SOC_E_UNAVAIL;
            break;

            case DRV_SNMP_IF_IN_UNKNOWN_PROTOS:
                return SOC_E_UNAVAIL;
            break;

            case DRV_SNMP_IF_OUT_OCTETS:
                return SOC_E_UNAVAIL;
            break;

            case DRV_SNMP_IF_OUT_UCAST_PKTS:	/* ALL - mcast - bcast */
                return SOC_E_UNAVAIL;
            break;

            case DRV_SNMP_IF_OUT_N_UCAST_PKTS:	
                return SOC_E_UNAVAIL;
            break;

            case DRV_SNMP_IF_OUT_DISCARDS:
                return SOC_E_UNAVAIL;
            break;

            case DRV_SNMP_IF_OUT_ERRORS:
                return SOC_E_UNAVAIL;                
            break;

            case DRV_SNMP_IF_OUT_Q_LEN:  /* robo not suppport */
                return SOC_E_UNAVAIL;
            break;

            case DRV_SNMP_IP_IN_RECEIVES:  /* robo not suppport */
                return SOC_E_UNAVAIL;
            break;

            case DRV_SNMP_IP_IN_HDR_ERRORS:  /* robo not suppport */
                return SOC_E_UNAVAIL;
            break;

            case DRV_SNMP_IP_FORW_DATAGRAMS:  /* robo not suppport */
                return SOC_E_UNAVAIL;
            break;

            case DRV_SNMP_IP_IN_DISCARDS:  /* robo not suppport */
                return SOC_E_UNAVAIL;
            break;


            /* *** RFC 1493 *** */   

            case DRV_SNMP_DOT1D_BASE_PORT_DELAY_EXCEEDED_DISCARDS:  
                /* robo not suppport */
                return SOC_E_UNAVAIL;
            break;

            case DRV_SNMP_DOT1D_BASE_PORT_MTU_EXCEEDED_DISCARDS:  
                /* robo not suppport */
                return SOC_E_UNAVAIL;
            break;


            case DRV_SNMP_DOT1D_TP_PORT_IN_FRAMES:
                return SOC_E_UNAVAIL;
            break;

            case DRV_SNMP_DOT1D_TP_PORT_OUT_FRAMES:
                return SOC_E_UNAVAIL;
            break;

            case DRV_SNMP_DOT1D_PORT_IN_DISCARDS:
                return SOC_E_UNAVAIL;
            break;

            /* *** RFC 1757 *** */
            case DRV_SNMP_ETHER_STATS_DROP_EVENTS:
                return SOC_E_UNAVAIL;
            break;

            case DRV_SNMP_ETHER_STATS_MULTICAST_PKTS:
                return SOC_E_UNAVAIL;
            break;

            case DRV_SNMP_ETHER_STATS_BROADCAST_PKTS:
                return SOC_E_UNAVAIL;
            break;

            case DRV_SNMP_ETHER_STATS_UNDERSIZE_PKTS:	/* Undersize frames */
                return SOC_E_UNAVAIL;
            break;

            case DRV_SNMP_ETHER_STATS_FRAGMENTS:
                return SOC_E_UNAVAIL;
            break;

            case DRV_SNMP_ETHER_STATS_PKTS64_OCTETS:
                return SOC_E_UNAVAIL;
            break;

            case DRV_SNMP_ETHER_STATS_PKTS65TO127_OCTETS:
                return SOC_E_UNAVAIL;
            break;

            case DRV_SNMP_ETHER_STATS_PKTS128TO255_OCTETS:
                return SOC_E_UNAVAIL;
            break;

            case DRV_SNMP_ETHER_STATS_PKTS256TO511_OCTETS:
                return SOC_E_UNAVAIL;
            break;

            case DRV_SNMP_ETHER_STATS_PKTS512TO1023_OCTETS:
                return SOC_E_UNAVAIL;
            break;

            case DRV_SNMP_ETHER_STATS_PKTS1024TO1518_OCTETS:
                return SOC_E_UNAVAIL;
            break;

            case DRV_SNMP_ETHER_STATS_OVERSIZE_PKTS:
            case DRV_SNMP_ETHER_RX_OVERSIZE_PKTS:
            case DRV_SNMP_ETHER_TX_OVERSIZE_PKTS:
                return SOC_E_UNAVAIL;
            break;

            case DRV_SNMP_ETHER_STATS_JABBERS:
                return SOC_E_UNAVAIL;
            break;

            case DRV_SNMP_ETHER_STATS_OCTETS:
                return SOC_E_UNAVAIL;
            break;

            case DRV_SNMP_ETHER_STATS_PKTS:
                return SOC_E_UNAVAIL;
            break;

            case DRV_SNMP_ETHER_STATS_COLLISIONS:
                return SOC_E_UNAVAIL;
            break;

            case DRV_SNMP_ETHER_STATS_CRC_ALIGN_ERRORS:	
                /* CRC errors + alignment errors */
                return SOC_E_UNAVAIL;
            break;


            case DRV_SNMP_ETHER_STATS_TX_NO_ERRORS:  
                /*  TPKT - (TNCL + TOVR + TFRG + TUND) */
                return SOC_E_UNAVAIL;
            break;    

            case DRV_SNMP_ETHER_STATS_RX_NO_ERRORS:  
                /* RPKT - ( RFCS + RXUO + RFLR) */
                return SOC_E_UNAVAIL;
            break;        

            /* RFC 1643 */        
            case DRV_SNMP_DOT3_STATS_INTERNAL_MAC_RECEIVE_ERRORS:	    
                return SOC_E_UNAVAIL;
            break;        

            case DRV_SNMP_DOT3_STATS_FRAME_TOO_LONGS:
                return SOC_E_UNAVAIL;
            break;        

            case DRV_SNMP_DOT3_STATS_ALIGNMENT_ERRORS:	/* *** RFC 2665 *** */
                return SOC_E_UNAVAIL;
            break;      

            case DRV_SNMP_DOT3_STATS_FCS_ERRORS:	/* *** RFC 2665 *** */
                return SOC_E_UNAVAIL;
            break;  

            case DRV_SNMP_DOT3_STATS_INTERNAL_MAC_TRANSMIT_ERRORS:	
                return SOC_E_UNAVAIL;
            break;  	

            case DRV_SNMP_DOT3_STATS_SINGLE_COLLISION_FRAMES:	
                /* *** RFC 2665 *** */
                return SOC_E_UNAVAIL;
            break;  	

            case DRV_SNMP_DOT3_STATS_MULTIPLE_COLLISION_FRAMES:	
                /* *** RFC 2665 *** */
                return SOC_E_UNAVAIL;
            break;  

            case DRV_SNMP_DOT3_STATS_DEFERRED_TRANSMISSIONS:	
                return SOC_E_UNAVAIL;
            break;  

            case DRV_SNMP_DOT3_STATS_LATE_COLLISIONS:	
                return SOC_E_UNAVAIL;
            break;  	

            case DRV_SNMP_DOT3_STATS_EXCESSIVE_COLLISIONS:	
                return SOC_E_UNAVAIL;
            break;  	

            case DRV_SNMP_DOT3_STATS_CARRIER_SENSE_ERRORS:  
                return SOC_E_UNAVAIL;
            break;  

            case DRV_SNMP_DOT3_STATS_SQET_TEST_ERRORS:	
                return SOC_E_UNAVAIL;
            break;  								    	  		

                
            /* *** RFC 2665 *** some object same as RFC 1643 */

            case DRV_SNMP_DOT3_STATS_SYMBOL_ERRORS:  
                return SOC_E_UNAVAIL;
            break;  

            case DRV_SNMP_DOT3_CONTROL_IN_UNKNOWN_OPCODES:  
                return SOC_E_UNAVAIL;
            break;  

            case DRV_SNMP_DOT3_IN_PAUSE_FRAMES:  
                return SOC_E_UNAVAIL;
            break;  

            case DRV_SNMP_DOT3_OUT_PAUSE_FRAMES:  
                return SOC_E_UNAVAIL;
            break;  

            /*** RFC 2233 ***/
            case DRV_SNMP_IF_HC_IN_OCTETS:	
                return SOC_E_UNAVAIL;
            break;  	

            case DRV_SNMP_IF_HC_IN_UCAST_PKTS:	
                return SOC_E_UNAVAIL;
            break;  	

            case DRV_SNMP_IF_HC_IN_MULTICAST_PKTS:	
                return SOC_E_UNAVAIL;
            break;  	

            case DRV_SNMP_IF_HC_IN_BROADCAST_PKTS:	
                return SOC_E_UNAVAIL;
            break;  	

            case DRV_SNMP_IF_HC_OUT_OCTETS:	
                return SOC_E_UNAVAIL;
            break;  	

            case DRV_SNMP_IF_HC_OUT_UCAST_PKTS:	
                return SOC_E_UNAVAIL;
            break;  	

            case DRV_SNMP_IF_HC_OUT_MULTICAST_PKTS:	
                return SOC_E_UNAVAIL;
            break;  	

            case DRV_SNMP_IF_HC_OUT_BROADCAST_PCKTS:	
                return SOC_E_UNAVAIL;
            break;  	    

            /*** RFC 2465 ***/
            case DRV_SNMP_IPV6_IF_STATS_IN_RECEIVES:
            case DRV_SNMP_IPV6_IF_STATS_IN_HDR_ERRORS:
            case DRV_SNMP_IPV6_IF_STATS_IN_ADDR_ERRORS:
            case DRV_SNMP_IPV6_IF_STATS_IN_DISCARDS:
            case DRV_SNMP_IPV6_IF_STATS_OUT_FORW_DATAGRAMS:
            case DRV_SNMP_IPV6_IF_STATS_OUT_DISCARDS:
            case DRV_SNMP_IPV6_IF_STATS_IN_MCAST_PKTS:
            case DRV_SNMP_IPV6_IF_STATS_OUT_MCAST_PKTS:
                /* not support for robo */
                return SOC_E_UNAVAIL;
            break;

            case DRV_SNMP_BCM_IPMC_BRIDGED_PCKTS:
            case DRV_SNMP_BCM_IPMC_ROUTED_PCKTS:    
            case DRV_SNMP_BCM_IPMC_IN_DROPPED_PCKTS:
            case DRV_SNMP_BCM_IPMC_OUT_DROPPED_PCKTS:
                /* not support for robo */
                return SOC_E_UNAVAIL;

            case DRV_SNMP_BCM_ETHER_STATS_PKTS1519TO1522_OCTETS:
            case DRV_SNMP_BCM_ETHER_STATS_PKTS1522TO2047_OCTETS:
            case DRV_SNMP_BCM_ETHER_STATS_PKTS2048TO4095_OCTETS:
            case DRV_SNMP_BCM_ETHER_STATS_PKTS4095TO9216_OCTETS:
                /* not support for robo */
                return SOC_E_UNAVAIL;

            case DRV_SNMP_BCM_CUSTOM_RECEIVE0:
            case DRV_SNMP_BCM_CUSTOM_RECEIVE1:
            case DRV_SNMP_BCM_CUSTOM_RECEIVE2:
            case DRV_SNMP_BCM_CUSTOM_RECEIVE3:
            case DRV_SNMP_BCM_CUSTOM_RECEIVE4:
            case DRV_SNMP_BCM_CUSTOM_RECEIVE5:
            case DRV_SNMP_BCM_CUSTOM_RECEIVE6:
            case DRV_SNMP_BCM_CUSTOM_RECEIVE7:
            case DRV_SNMP_BCM_CUSTOM_RECEIVE8:
            case DRV_SNMP_BCM_CUSTOM_TRANSMIT0:
            case DRV_SNMP_BCM_CUSTOM_TRANSMIT1:
            case DRV_SNMP_BCM_CUSTOM_TRANSMIT2:
            case DRV_SNMP_BCM_CUSTOM_TRANSMIT3:
            case DRV_SNMP_BCM_CUSTOM_TRANSMIT4:
            case DRV_SNMP_BCM_CUSTOM_TRANSMIT5:
            case DRV_SNMP_BCM_CUSTOM_TRANSMIT6:
            case DRV_SNMP_BCM_CUSTOM_TRANSMIT7:
            case DRV_SNMP_BCM_CUSTOM_TRANSMIT8:
            case DRV_SNMP_BCM_CUSTOM_TRANSMIT9:
            case DRV_SNMP_BCM_CUSTOM_TRANSMIT10:
            case DRV_SNMP_BCM_CUSTOM_TRANSMIT11:
            case DRV_SNMP_BCM_CUSTOM_TRANSMIT12:
            case DRV_SNMP_BCM_CUSTOM_TRANSMIT13:
            case DRV_SNMP_BCM_CUSTOM_TRANSMIT14:
                /* not support for robo */
                return SOC_E_UNAVAIL;

            default:
            soc_cm_debug(DK_WARN+DK_VERBOSE,
            "drv_snmp_counter_set: Statistic not supported: %d\n", 
            counter_type);
            return SOC_E_PARAM;
        }
    }
    return rv;
}

 /*
 *  Function : drv_counter_get
 *
 *  Purpose :
 *      Get the snmp counter value.
 *
 *  Parameters :
 *      uint    :   uint number.
 *      port        :   port number.
 *      counter_type   :   counter_type.
 *      val  :   counter val.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */
int
drv_bcm5396_counter_get(int unit, uint32 port, uint32 counter_type, uint64 *val)
{
    uint64	count;

    COMPILER_64_ZERO(count);

    if (soc_feature(unit, soc_feature_no_stat_mib)) {
        return SOC_E_UNAVAIL;
    }

    switch (counter_type) 
    {
        /* *** RFC 1213 *** */

        case DRV_SNMP_IF_IN_OCTETS:       	
            return SOC_E_UNAVAIL;
        break;

        case DRV_SNMP_IF_IN_UCAST_PKTS:     
            return SOC_E_UNAVAIL;
        break;

        case DRV_SNMP_IF_IN_N_UCAST_PKTS:    
            return SOC_E_UNAVAIL;
        break;

        case DRV_SNMP_IF_IN_DISCARDS:
            return SOC_E_UNAVAIL;
        break;

        case DRV_SNMP_IF_IN_ERRORS:	
            return SOC_E_UNAVAIL;
        break;

        case DRV_SNMP_IF_IN_UNKNOWN_PROTOS:
            return SOC_E_UNAVAIL;
        break;

        case DRV_SNMP_IF_OUT_OCTETS:
            return SOC_E_UNAVAIL;
        break;

        case DRV_SNMP_IF_OUT_UCAST_PKTS:	/* ALL - mcast - bcast */
            return SOC_E_UNAVAIL;
        break;

        case DRV_SNMP_IF_OUT_N_UCAST_PKTS:	
            return SOC_E_UNAVAIL;
        break;

        case DRV_SNMP_IF_OUT_DISCARDS:
            return SOC_E_UNAVAIL;
        break;

        case DRV_SNMP_IF_OUT_ERRORS:
            return SOC_E_UNAVAIL;
        break;

        case DRV_SNMP_IF_OUT_Q_LEN:  /* robo not suppport */
            return SOC_E_UNAVAIL;
        break;

        case DRV_SNMP_IP_IN_RECEIVES:  /* robo not suppport */
            return SOC_E_UNAVAIL;
        break;

        case DRV_SNMP_IP_IN_HDR_ERRORS:  /* robo not suppport */
            return SOC_E_UNAVAIL;
        break;

        case DRV_SNMP_IP_FORW_DATAGRAMS:  /* robo not suppport */
            return SOC_E_UNAVAIL;
        break;

        case DRV_SNMP_IP_IN_DISCARDS:  /* robo not suppport */
            return SOC_E_UNAVAIL;
        break;


        /* *** RFC 1493 *** */   

        case DRV_SNMP_DOT1D_BASE_PORT_DELAY_EXCEEDED_DISCARDS:  
            /* robo not suppport */
            return SOC_E_UNAVAIL;
        break;

        case DRV_SNMP_DOT1D_BASE_PORT_MTU_EXCEEDED_DISCARDS:  
            /* robo not suppport */
            return SOC_E_UNAVAIL;
        break;


        case DRV_SNMP_DOT1D_TP_PORT_IN_FRAMES:
            return SOC_E_UNAVAIL;
        break;

        case DRV_SNMP_DOT1D_TP_PORT_OUT_FRAMES:
            return SOC_E_UNAVAIL;
        break;

        case DRV_SNMP_DOT1D_PORT_IN_DISCARDS:
            return SOC_E_UNAVAIL;
        break;

        /* *** RFC 1757 *** */
        case DRV_SNMP_ETHER_STATS_DROP_EVENTS:
            return SOC_E_UNAVAIL;
        break;

        case DRV_SNMP_ETHER_STATS_MULTICAST_PKTS:
            return SOC_E_UNAVAIL;
        break;

        case DRV_SNMP_ETHER_STATS_BROADCAST_PKTS:
            return SOC_E_UNAVAIL;
        break;

        case DRV_SNMP_ETHER_STATS_UNDERSIZE_PKTS:	/* Undersize frames */
            return SOC_E_UNAVAIL;
        break;

        case DRV_SNMP_ETHER_STATS_FRAGMENTS:
            return SOC_E_UNAVAIL;
        break;

        case DRV_SNMP_ETHER_STATS_PKTS64_OCTETS:
            return SOC_E_UNAVAIL;
        break;

        case DRV_SNMP_ETHER_STATS_PKTS65TO127_OCTETS:
            return SOC_E_UNAVAIL;
        break;

        case DRV_SNMP_ETHER_STATS_PKTS128TO255_OCTETS:
            return SOC_E_UNAVAIL;
        break;

        case DRV_SNMP_ETHER_STATS_PKTS256TO511_OCTETS:
            return SOC_E_UNAVAIL;
        break;

        case DRV_SNMP_ETHER_STATS_PKTS512TO1023_OCTETS:
            return SOC_E_UNAVAIL;
        break;

        case DRV_SNMP_ETHER_STATS_PKTS1024TO1518_OCTETS:
            return SOC_E_UNAVAIL;
        break;

        case DRV_SNMP_ETHER_STATS_OVERSIZE_PKTS:
        case DRV_SNMP_ETHER_RX_OVERSIZE_PKTS:
        case DRV_SNMP_ETHER_TX_OVERSIZE_PKTS:
            return SOC_E_UNAVAIL;
        break;

        case DRV_SNMP_ETHER_STATS_JABBERS:
            return SOC_E_UNAVAIL;
        break;

        case DRV_SNMP_ETHER_STATS_OCTETS:
            return SOC_E_UNAVAIL;
        break;

        case DRV_SNMP_ETHER_STATS_PKTS:
            return SOC_E_UNAVAIL;
        break;

        case DRV_SNMP_ETHER_STATS_COLLISIONS:
            return SOC_E_UNAVAIL;
        break;

        case DRV_SNMP_ETHER_STATS_CRC_ALIGN_ERRORS:	
            return SOC_E_UNAVAIL;
        break;    

        case DRV_SNMP_ETHER_STATS_TX_NO_ERRORS:  
            /*  TPKT - (TNCL + TOVR + TFRG + TUND) */
            return SOC_E_UNAVAIL;
        break;    

        case DRV_SNMP_ETHER_STATS_RX_NO_ERRORS:  
            /* RPKT - ( RFCS + RXUO + RFLR) */
            return SOC_E_UNAVAIL;
        break;        

        /* RFC 1643 */        
        case DRV_SNMP_DOT3_STATS_INTERNAL_MAC_RECEIVE_ERRORS:	    
            return SOC_E_UNAVAIL;
        break;        

        case DRV_SNMP_DOT3_STATS_FRAME_TOO_LONGS:	
            return SOC_E_UNAVAIL;
        break;    

        case DRV_SNMP_DOT3_STATS_ALIGNMENT_ERRORS:	/* *** RFC 2665 *** */
            return SOC_E_UNAVAIL;
        break;      

        case DRV_SNMP_DOT3_STATS_FCS_ERRORS:	/* *** RFC 2665 *** */
            return SOC_E_UNAVAIL;
        break;  

        case DRV_SNMP_DOT3_STATS_INTERNAL_MAC_TRANSMIT_ERRORS:	
            return SOC_E_UNAVAIL;
        break;  	

        case DRV_SNMP_DOT3_STATS_SINGLE_COLLISION_FRAMES:	
            /* *** RFC 2665 *** */
            return SOC_E_UNAVAIL;
        break;  	

        case DRV_SNMP_DOT3_STATS_MULTIPLE_COLLISION_FRAMES:	
            /* *** RFC 2665 *** */
            return SOC_E_UNAVAIL;
        break;  

        case DRV_SNMP_DOT3_STATS_DEFERRED_TRANSMISSIONS:	
            return SOC_E_UNAVAIL;
        break;  

        case DRV_SNMP_DOT3_STATS_LATE_COLLISIONS:	
            return SOC_E_UNAVAIL;
        break;  	

        case DRV_SNMP_DOT3_STATS_EXCESSIVE_COLLISIONS:	
            return SOC_E_UNAVAIL;
        break;  	

        case DRV_SNMP_DOT3_STATS_CARRIER_SENSE_ERRORS:  
            return SOC_E_UNAVAIL;
        break;  

        case DRV_SNMP_DOT3_STATS_SQET_TEST_ERRORS:	
            return SOC_E_UNAVAIL;
        break;  								    	  		

            
        /* *** RFC 2665 *** some object same as RFC 1643 */

        case DRV_SNMP_DOT3_STATS_SYMBOL_ERRORS:  
            return SOC_E_UNAVAIL;
        break;  

        case DRV_SNMP_DOT3_CONTROL_IN_UNKNOWN_OPCODES:  
            return SOC_E_UNAVAIL;
        break;  

        case DRV_SNMP_DOT3_IN_PAUSE_FRAMES:  
            return SOC_E_UNAVAIL;
        break;  

        case DRV_SNMP_DOT3_OUT_PAUSE_FRAMES:  
            return SOC_E_UNAVAIL;
        break;  

        /*** RFC 2233 ***/
        case DRV_SNMP_IF_HC_IN_OCTETS:	
            return SOC_E_UNAVAIL;
        break;  	

        case DRV_SNMP_IF_HC_IN_UCAST_PKTS:	
            return SOC_E_UNAVAIL;
        break;  	

        case DRV_SNMP_IF_HC_IN_MULTICAST_PKTS:	
            return SOC_E_UNAVAIL;
        break;  	

        case DRV_SNMP_IF_HC_IN_BROADCAST_PKTS:	
            return SOC_E_UNAVAIL;
        break;  	

        case DRV_SNMP_IF_HC_OUT_OCTETS:	
            return SOC_E_UNAVAIL;
        break;  	

        case DRV_SNMP_IF_HC_OUT_UCAST_PKTS:	
            return SOC_E_UNAVAIL;
        break;  	

        case DRV_SNMP_IF_HC_OUT_MULTICAST_PKTS:	
            return SOC_E_UNAVAIL;
        break;  	

        case DRV_SNMP_IF_HC_OUT_BROADCAST_PCKTS:	
            return SOC_E_UNAVAIL;
        break;  	    

        /*** RFC 2465 ***/
        case DRV_SNMP_IPV6_IF_STATS_IN_RECEIVES:
        case DRV_SNMP_IPV6_IF_STATS_IN_HDR_ERRORS:
        case DRV_SNMP_IPV6_IF_STATS_IN_ADDR_ERRORS:
        case DRV_SNMP_IPV6_IF_STATS_IN_DISCARDS:
        case DRV_SNMP_IPV6_IF_STATS_OUT_FORW_DATAGRAMS:
        case DRV_SNMP_IPV6_IF_STATS_OUT_DISCARDS:
        case DRV_SNMP_IPV6_IF_STATS_IN_MCAST_PKTS:
        case DRV_SNMP_IPV6_IF_STATS_OUT_MCAST_PKTS:
            /* not support for robo */
            COMPILER_64_ZERO(*val);
            return SOC_E_UNAVAIL;
        break;

        case DRV_SNMP_BCM_IPMC_BRIDGED_PCKTS:
        case DRV_SNMP_BCM_IPMC_ROUTED_PCKTS:    
        case DRV_SNMP_BCM_IPMC_IN_DROPPED_PCKTS:
        case DRV_SNMP_BCM_IPMC_OUT_DROPPED_PCKTS:
            /* not support for robo */
            COMPILER_64_ZERO(*val);
            return SOC_E_UNAVAIL;

        case DRV_SNMP_BCM_ETHER_STATS_PKTS1519TO1522_OCTETS:
        case DRV_SNMP_BCM_ETHER_STATS_PKTS1522TO2047_OCTETS:
        case DRV_SNMP_BCM_ETHER_STATS_PKTS2048TO4095_OCTETS:
        case DRV_SNMP_BCM_ETHER_STATS_PKTS4095TO9216_OCTETS:
            /* not support for robo */
            COMPILER_64_ZERO(*val);
            return SOC_E_UNAVAIL;

case DRV_SNMP_BCM_CUSTOM_RECEIVE0:
            case DRV_SNMP_BCM_CUSTOM_RECEIVE1:
            case DRV_SNMP_BCM_CUSTOM_RECEIVE2:
            case DRV_SNMP_BCM_CUSTOM_RECEIVE3:
            case DRV_SNMP_BCM_CUSTOM_RECEIVE4:
            case DRV_SNMP_BCM_CUSTOM_RECEIVE5:
            case DRV_SNMP_BCM_CUSTOM_RECEIVE6:
            case DRV_SNMP_BCM_CUSTOM_RECEIVE7:
            case DRV_SNMP_BCM_CUSTOM_RECEIVE8:
            case DRV_SNMP_BCM_CUSTOM_TRANSMIT0:
            case DRV_SNMP_BCM_CUSTOM_TRANSMIT1:
            case DRV_SNMP_BCM_CUSTOM_TRANSMIT2:
            case DRV_SNMP_BCM_CUSTOM_TRANSMIT3:
            case DRV_SNMP_BCM_CUSTOM_TRANSMIT4:
            case DRV_SNMP_BCM_CUSTOM_TRANSMIT5:
            case DRV_SNMP_BCM_CUSTOM_TRANSMIT6:
            case DRV_SNMP_BCM_CUSTOM_TRANSMIT7:
            case DRV_SNMP_BCM_CUSTOM_TRANSMIT8:
            case DRV_SNMP_BCM_CUSTOM_TRANSMIT9:
            case DRV_SNMP_BCM_CUSTOM_TRANSMIT10:
            case DRV_SNMP_BCM_CUSTOM_TRANSMIT11:
            case DRV_SNMP_BCM_CUSTOM_TRANSMIT12:
            case DRV_SNMP_BCM_CUSTOM_TRANSMIT13:
            case DRV_SNMP_BCM_CUSTOM_TRANSMIT14:            /* not support for robo */
            COMPILER_64_ZERO(*val);
            return SOC_E_UNAVAIL;

        default:
        soc_cm_debug(DK_WARN+DK_VERBOSE,
        "drv_snmp_counter_get: Statistic not supported: %d\n", counter_type);
        return SOC_E_PARAM;
    }

    *val = count;

    return SOC_E_NONE;
}
