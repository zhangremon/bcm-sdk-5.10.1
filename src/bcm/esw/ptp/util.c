/*
 * $Id: util.c 1.1.2.5 Broadcom SDK $
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

#include <soc/defs.h>
#include <soc/drv.h>

#include <bcm/ptp.h>
#include <bcm_int/esw/ptp.h>
#include <bcm/error.h>

#include <bcm_int/control.h>

#if defined(BCM_KATANA_SUPPORT)
#include <bcm_int/esw/katana.h>
#endif

/*
 * Function:
 *      _bcm_ptp_uint16_read
 * Purpose:
 *      Read a 16-bit unsigned integer from a buffer in network byte order.
 * Parameters:
 *      buffer - (IN) Data buffer.
 * Returns:
 *      Result.
 * Notes:
 */
uint16 
_bcm_ptp_uint16_read(
        uint8* buffer)
{   
    return ((((uint16)(buffer[0])) << 8) + (((uint16)(buffer[1]))));
}

/*
 * Function:
 *      _bcm_ptp_uint32_read
 * Purpose:
 *      Read a 32-bit unsigned integer from a buffer in network byte order.
 * Parameters:
 *      buffer - (IN) Data buffer.
 * Returns:
 *      Result.
 * Notes:
 */
uint32 
_bcm_ptp_uint32_read(
    uint8* buffer)
{
    return ((((uint32)(buffer[0])) << 24) + 
            (((uint32)(buffer[1])) << 16) +
            (((uint32)(buffer[2])) << 8)  +
            (((uint32)(buffer[3]))));
}

/*
 * Function:
 *      _bcm_ptp_uint64_read
 * Purpose:
 *      Read a 64-bit unsigned integer from a buffer in network byte order.
 * Parameters:
 *      buffer - (IN) Data buffer.
 * Returns:
 *      Result.
 * Notes:
 */
uint64 
_bcm_ptp_uint64_read(uint8* buffer)
{
    return ((((uint64)(buffer[0])) << 56) + 
            (((uint64)(buffer[1])) << 48) +
            (((uint64)(buffer[2])) << 40) +
            (((uint64)(buffer[3])) << 32) +
            (((uint64)(buffer[4])) << 24) +
            (((uint64)(buffer[5])) << 16) +
            (((uint64)(buffer[6])) << 8)  +           
            (((uint64)(buffer[7]))));
}

/*
 * Function:
 *      _bcm_ptp_uint16_write
 * Purpose:
 *      Write a 16-bit unsigned integer to a buffer in network byte order.
 * Parameters:
 *      value  - (IN)  Data.
 *      buffer - (OUT) Data buffer.
 * Returns:
 *      None.
 * Notes:
 */
void 
_bcm_ptp_uint16_write(
    uint8* buffer, 
    const uint16 value)
{
    buffer[0] = ((value >> 8) & 0xff);
    buffer[1] = ((value) & 0xff);
}

/*
 * Function:
 *      _bcm_ptp_uint32_write
 * Purpose:
 *      Write a 32-bit unsigned integer to a buffer in network byte order.
 * Parameters:
 *      value  - (IN)  Data.
 *      buffer - (OUT) Data buffer.
 * Returns:
 *      None.
 * Notes:
 */
void
_bcm_ptp_uint32_write(
    uint8* buffer, 
    const uint32 value)
{
    buffer[0] = ((value >> 24) & 0xff);
    buffer[1] = ((value >> 16) & 0xff);
    buffer[2] = ((value >> 8) & 0xff);
    buffer[3] = ((value) & 0xff);
}

/*
 * Function:
 *      _bcm_ptp_uint64_write
 * Purpose:
 *      Write a 64-bit unsigned integer to a buffer in network byte order.
 * Parameters:
 *      value  - (IN)  Data.
 *      buffer - (OUT) Data buffer.
 * Returns:
 *      None.
 * Notes:
 */
void 
_bcm_ptp_uint64_write(
    uint8* buffer, 
    const uint64 value)
{
    buffer[0] = ((value >> 56) & 0xff);
    buffer[1] = ((value >> 48) & 0xff);
    buffer[2] = ((value >> 40) & 0xff);
    buffer[3] = ((value >> 32) & 0xff);
    buffer[4] = ((value >> 24) & 0xff);
    buffer[5] = ((value >> 16) & 0xff);
    buffer[6] = ((value >> 8) & 0xff);
    buffer[7] = ((value) & 0xff);
}

/*
 * Function:
 *      _bcm_ptp_function_precheck
 * Purpose:
 *      Perform basic argument and PTP module prechecks on a function.
 * Parameters:
 *      unit       - (IN) Unit number.
 *      ptp_id     - (IN) PTP stack ID.
 *      clock_num  - (IN) PTP clock number.
 *      clock_port - (IN) PTP clock port number.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
_bcm_ptp_function_precheck(
    int unit, 
    bcm_ptp_stack_id_t ptp_id, 
    int clock_num, 
    uint32 clock_port)
{
    int rv = BCM_E_UNAVAIL;
   
    _bcm_ptp_info_t *ptp_info_p;
    bcm_ptp_clock_info_t ci;    
    
    if (!BCM_UNIT_VALID(unit)) {
        soc_cm_print("bad unit: %d\n", unit);
        return BCM_E_UNIT;
    }
    
    if (soc_feature(unit, soc_feature_ptp)) {                
        SET_PTP_INFO;
     
        if (!ptp_info_p->initialized) {
            return BCM_E_UNAVAIL;
        }
        
        /* Argument checking and error handling. */
        if ((unit < PTP_UNIT_NUMBER_DEFAULT) ||
                (unit >= BCM_MAX_NUM_UNITS) ||
                (ptp_id < PTP_STACK_ID_DEFAULT) ||
                (ptp_id >= PTP_MAX_STACKS_PER_UNIT) ||
                (clock_num < PTP_CLOCK_NUMBER_DEFAULT) ||
                (clock_num >= PTP_MAX_CLOCK_INSTANCES)) {
            return BCM_E_PARAM;
        }
        
        if ((clock_port == PTP_IEEE1588_ALL_PORTS) ||
                (clock_port == PTP_CLOCK_PORT_NUMBER_DEFAULT)) {
            return BCM_E_NONE;
        } else {
            if (BCM_FAILURE(rv = _bcm_ptp_clock_cache_info_get(unit, ptp_id, 
                    clock_num, &ci))) {
                return rv;
            }

            if ((clock_port < PTP_CLOCK_PORT_NUMBER_DEFAULT) || 
                    (clock_port > ci.num_ports)) {
                return BCM_E_PORT; 
            }

            switch (ci.type) {
            case bcmPTPClockTypeOrdinary:
            case bcmPTPClockTypeBoundary:
            case bcmPTPClockTypeTransparent:
                break;
            default:
                return BCM_E_PARAM;
            }
        }
        
    }
    
    return rv;
}

/*
 * Function:
 *      _bcm_ptp_clock_lookup
 * Purpose:
 *      Lookup the unit number, PTP stack ID, and PTP clock number of a PTP
 *      clock based on caller-provided clock identity. 
 * Parameters:
 *      clock_identity - (IN)  PTP clock identity to lookup.
 *      unit           - (OUT) Unit number.
 *      ptp_id         - (OUT) PTP stack ID.
 *      clock_num      - (OUT) PTP clock number.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int _bcm_ptp_clock_lookup(
    const bcm_ptp_clock_identity_t clock_identity,
    int *unit,
    bcm_ptp_stack_id_t *ptp_id,
    int *clock_num)
{
    int rv = BCM_E_NOT_FOUND;
    
    bcm_ptp_clock_info_t ci;
    int i, j, k;
    
    for (i = PTP_UNIT_NUMBER_DEFAULT; i < BCM_MAX_NUM_UNITS; ++i) {
        if (!BCM_UNIT_VALID(i)) {
            continue;
        }
        for (j = PTP_STACK_ID_DEFAULT; j < PTP_MAX_STACKS_PER_UNIT; ++j) {
            for (k = PTP_CLOCK_NUMBER_DEFAULT; k < PTP_MAX_CLOCK_INSTANCES; ++k) {
                if (BCM_FAILURE(_bcm_ptp_clock_cache_info_get(i, 
                        (bcm_ptp_stack_id_t)j, k, &ci))) {
                    continue;
                }
                
                if (!memcmp(ci.clock_identity, 
                            clock_identity, sizeof(bcm_ptp_clock_identity_t))) {
                    *unit = i;
                    *ptp_id = (bcm_ptp_stack_id_t)j;
                    *clock_num = k;

                    return BCM_E_NONE;
                }
            }
        }
    }
    
    return rv;
}

/*
 * Function:
 *      _bcm_ptp_peer_address_compare
 * Purpose:
 *      Compare addresses.
 * Parameters:
 *      a - (IN) Address "A".
 *      b - (IN) Address "B".
 * Returns:
 *      0: Addresses do not match.
 *      1: Addresses match.
 * Notes:
 */
int 
_bcm_ptp_peer_address_compare(
    const bcm_ptp_clock_peer_address_t *a, 
    const bcm_ptp_clock_peer_address_t *b) 
{
    if (a->addr_type != b->addr_type) 
    {
        return 0;
    }

    switch (a->addr_type) {
    case bcmPTPUDPIPv4:
        return (a->ipv4_addr == b->ipv4_addr);

    case bcmPTPUDPIPv6:
        return !memcmp(a->ipv6_addr, b->ipv6_addr, PTP_IPV6_ADDR_SIZE_BYTES);

    default:
        return 0;

    }
}

/*
 * Function:
 *      _bcm_ptp_dump_hex
 * Purpose:
 *      Print hexadecimal buffer.
 * Parameters:
 *      buf - (IN) buffer to be printed
 *      len - (IN) message length
 * Returns:
 *      none
 * Notes:
 */
void _bcm_ptp_dump_hex(uint8 *buf, int len)
{
    char line[100];
    int i;
    int linepos = 0;
    for (i = 0; i < len; ++i) {
        sal_sprintf(line + linepos, "%02x", *buf++);
        linepos += 2;
        sal_sprintf(line + linepos, "  ");
        ++linepos;

        if ((i & 0x1f) == 0x1f) {
            soc_cm_print("%s\n", line);
            line[0] = 0;
            linepos = 0;
        }
    }

    /* output last line if it wasn't complete */
    if (len & 0x1f) {
        soc_cm_print("%s\n", line);
    }
}
#endif /* defined(INCLUDE_PTP)*/
