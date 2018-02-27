/*
 * $Id: ptp.c 1.4.2.8 Broadcom SDK $
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
 * File: 	ptp.c
 * Purpose: IEEE1588 (PTP) Support
 */

#include <assert.h>
#include <sal/core/libc.h>
#include <bcm/error.h>
#include <soc/types.h>
#include <appl/diag/system.h>
#include <sal/core/dpc.h>

#if defined(INCLUDE_PTP)

#include <bcm/ptp.h>
#include <bcm_int/esw/ptp.h>


#define LOCAL_DEBUGBUFSIZE (1024)

static char local_debugbuf[LOCAL_DEBUGBUFSIZE];
static int local_head = 0;

static char output_debugbuf[LOCAL_DEBUGBUFSIZE * 2];
static int local_tail = 0;

int ptp_printf(const char *fmt, ...) {
    int ret;
    va_list args;
    char buf[256];
    int i;

    va_start(args, fmt);
    ret = vsprintf(buf, fmt, args);

    for (i = 0; i < ret; ++i) {
        local_debugbuf[local_head++] = buf[i];
        if (local_head >= LOCAL_DEBUGBUFSIZE) local_head = 0;
    }
    return ret;
}


/* Dump anything new in the debug buffers.
 *  owner:         The owner if this is being called as a DPC.
 *  time_as_ptr:   Recurrence time if a follow-up DPC to this func should be scheduled.
 *  unit_as_ptr:   unit
 *  ptp_id_as_ptr: ptp_id
 */
static void output_current_debug(void* owner, void* time_as_ptr, void *unit_as_ptr, void *ptp_id_as_ptr, void* unused_3)
{
    int callback_time = (int)(size_t)time_as_ptr;
    int unit = (int)(size_t)unit_as_ptr;
    bcm_ptp_stack_id_t ptp_id = (int)(size_t)ptp_id_as_ptr;

    uint32 head, size;
    int out_idx = 0;

    /* output the local debug first */
    while (local_tail != local_head) {
        char c = local_debugbuf[local_tail++];
        
        if (c) {
            output_debugbuf[out_idx++] = c;
        }
        
        if (local_tail == LOCAL_DEBUGBUFSIZE) {
            local_tail = 0;
        }
    }

    /* Ensure that PTP is up & initialized before proceeding */
    if (_bcm_esw_ptp_unit_array[unit].memstate == PTP_MEMSTATE_INITIALIZED &&
        _bcm_esw_ptp_unit_array[unit].stack_array[ptp_id].memstate == PTP_MEMSTATE_INITIALIZED) {
        
        _bcm_ptp_info_t *ptp_info_p = &_bcm_esw_ptp_info[unit];
        _bcm_ptp_stack_info_t *stack_p = &ptp_info_p->stack_info[ptp_id];

        /* Katana shared-mem output */
        if (SOC_IS_KATANA(unit)) {
            /* head is written in externally, so will be in network byte order
             * tail is local, so we'll keep it in local endianness
             * size is read externally, so is also network byte order
             */
            head = soc_htonl(stack_p->int_state.log->head);
            size = soc_htonl(stack_p->int_state.log->size);
        
            while (stack_p->int_state.log->tail != head) {
                char c = stack_p->int_state.log->buf[stack_p->int_state.log->tail++];
                if (c) {
                    output_debugbuf[out_idx++] = c;
                }
                if (stack_p->int_state.log->tail == size) {
                    stack_p->int_state.log->tail = 0;
                }
            }
        } else {
#if defined(PTP_KEYSTONE_STACK)
            #define DEBUG_WINDOW_SIZE (1024)
            #define DEBUG_OFFSET_MASK (DEBUG_WINDOW_SIZE - 1)
            static uint32 tail = 0;
            const uint32 debug_head_idx_addr = 0x190005a8;
            const uint32 debug_window_addr = 0x190005ac;
            void *cookie = stack_p->ext_info.cookie;
            uint32 head;

            /* get the current head of the debug buffer (written by ToP) */
            stack_p->ext_info.read_fn(cookie, debug_head_idx_addr, &head);
            head &= DEBUG_OFFSET_MASK;
            
            /* walk our tail up to the current head */
            while (tail != head) {
                uint32 wordval;
                uint32 addr = debug_window_addr + tail;
                /* read a byte at "addr", by reading the word that the byte is in, then shifting */
                stack_p->ext_info.read_fn(cookie, addr & 0xfffffffc, &wordval);
                output_debugbuf[out_idx++] = wordval >> (24 - (addr & 3) * 8);
                tail = ((tail + 1) & DEBUG_OFFSET_MASK);
            }
#endif
        }
    }

    if (out_idx) {
        output_debugbuf[out_idx] = 0;
        sal_printf("%s", output_debugbuf);
    }
        

    if (callback_time) {
        sal_dpc_time(callback_time, &output_current_debug,
                     &output_current_debug, time_as_ptr, unit_as_ptr, ptp_id_as_ptr, 0);
    }

}

char cmd_ptp_usage[] =
    "\tptp init\n"
    "\tptp stack create\n"
    "\tptp clock create\n"
    "\tptp port configure\n"
     "\tptp debug [start|stop|<level>]\n";

    
cmd_result_t cmd_ptp(int unit, args_t *a)
{
    char *arg;
    static int clock_num = 0;  /* for now: per-clock operations happen on the last created clock */
    int rv;
    uint8 ipv4_multicast_mac[6] = {0x01, 0x00, 0x5e, 0x00, 0x01, 0x81};
    uint8 ipv4_multicast_mac_pdelay[6] = {0x01, 0x00, 0x5e, 0x00, 0x00, 0x6b};

    arg = ARG_GET(a);
    if (!arg) {
        return CMD_USAGE;
    }

    if (strcmp(arg, "init") == 0) {
        rv = bcm_ptp_init(unit);
        return rv ? CMD_FAIL : CMD_OK;
    } else if (strcmp(arg, "stack") == 0) {
        arg = ARG_GET(a);
        if (!arg) {
            return CMD_USAGE;
        }
        if (strcmp(arg, "create") == 0) {
            bcm_ptp_external_stack_info_t external = {0};
            bcm_ptp_stack_info_t stack_info = {0, 0, &external};

            rv = bcm_ptp_stack_create(unit, &stack_info);
            return rv ? CMD_FAIL : CMD_OK;
        } else {
            return CMD_USAGE;
        }
    } else if (strcmp(arg, "clock") == 0) {
        arg = ARG_GET(a);
        if (!arg) {
            return CMD_USAGE;
        }
        if (strcmp(arg, "create") == 0) {
            bcm_ptp_clock_info_t clock_info = {0, 0, {1,2,3,4,5,6,7,8}, bcmPTPClockTypeBoundary, 2, 248, 0, 1, 128, 64, 0, 0, 0, 1,
                                               2, 3, 10,
                                               0, 1, 2,
                                               -5, -5, 0,
                                               -5, -5, 0,
                                               0, 0, 255,
                                               0, 128, 255,
                                               0, 64, 255, 0};
            rv = bcm_ptp_clock_create(0, 0, &clock_info);    
            if (rv == BCM_E_NONE) {
                clock_num = clock_info.clock_num;
            }
            return rv ? CMD_FAIL : CMD_OK;
        } else {
            return CMD_USAGE;
        }
    } else if (strcmp(arg, "port") == 0) {
        arg = ARG_GET(a);
        if (!arg) {
            return CMD_USAGE;
        }
        if (strcmp(arg, "configure") == 0) {
            bcm_ptp_clock_port_info_t port_info = {{bcmPTPUDPIPv4, {192,168,0,55}},
                                                   {0, 0, 1, 5, 4, 3},
                                                   0, {}, 0, {},
                                                   1, bcmPTPPortTypeStandard,
                                                   3, 1, 0, 0, bcmPTPDelayMechanismEnd2End, bcmPTPMac32CorrectionTimestamps,
                                                   1, 0, 0xffffffff, 1
            };
            

            memcpy(port_info.multicast_l2, ipv4_multicast_mac, 6);
            memcpy(port_info.multicast_l2 + 6, port_info.mac, 6);
            soc_htons_store(port_info.multicast_l2 + 12, 0x8100);
            soc_htons_store(port_info.multicast_l2 + 14, port_info.rx_packets_vlan);
            soc_htons_store(port_info.multicast_l2 + 16, 0x0800);
            port_info.multicast_l2_size = 18;
            
            memcpy(port_info.multicast_pdelay_l2, ipv4_multicast_mac_pdelay, 6);
            memcpy(port_info.multicast_pdelay_l2 + 6, port_info.mac, 6);
            soc_htons_store(port_info.multicast_pdelay_l2 + 12, 0x8100);
            soc_htons_store(port_info.multicast_pdelay_l2 + 14, port_info.rx_packets_vlan);
            soc_htons_store(port_info.multicast_pdelay_l2 + 16, 0x0800);
            port_info.multicast_pdelay_l2_size = 18;

            rv = bcm_ptp_clock_port_configure(0, 0, clock_num, 1, &port_info);
            return rv ? CMD_FAIL : CMD_OK;
        } else {
            printk("  failed on '%s'\n", arg);
            return CMD_USAGE;
        }
    } else if (strcmp(arg, "time") == 0) {
        arg = ARG_GET(a);
        if (!arg) {
            return CMD_USAGE;
        }
        if (strcmp(arg, "get") == 0) {
            bcm_ptp_timestamp_t time;
            rv = bcm_ptp_clock_time_get(0, 0, clock_num, &time);
            printk("Time: %llu.%9u\n", time.seconds, (unsigned)time.nanoseconds);
            return rv ? CMD_FAIL : CMD_OK;
        } else if (strcmp(arg, "set") == 0) {
            /* ... */
            return CMD_FAIL;
        }
    } else if (strcmp(arg, "channels") == 0) {
        static bcm_ptp_channel_t channels[2];
        static int num_channels;
        arg = ARG_GET(a);
        if (!arg) {
            return CMD_USAGE;
        }
        if (strcmp(arg, "get") == 0) {
            /* here for now, since it is not part of external API */
            extern int _bcm_esw_ptp_input_channels_get(int unit, bcm_ptp_stack_id_t ptp_id, int clock_num,
                                                   int *num_channels, bcm_ptp_channel_t *channels);

            num_channels = 2;
            rv = _bcm_esw_ptp_input_channels_get(0, 0, clock_num, &num_channels, channels);
            printk("Channels: %d  channel[0]: type=%d, source=%d, freq=%d\n", num_channels,
                   channels[0].type, channels[0].source, channels[0].frequency);                    
            return rv ? CMD_FAIL : CMD_OK;
        } else if (strcmp(arg, "set") == 0) {
            parse_table_t pt;
            parse_table_init(unit, &pt);
            parse_table_add(&pt, "Num", PQ_INT, (void*)1, &num_channels, NULL);
            parse_table_add(&pt, "PrimaryType", PQ_INT, 0, &channels[0].type, NULL);
            parse_table_add(&pt, "PrimarySource", PQ_INT, 0, &channels[0].source, NULL);
            parse_table_add(&pt, "PrimaryFrequency", PQ_INT, (void*)1000, &channels[0].frequency, NULL);
            parse_table_add(&pt, "BackupType", PQ_INT, 0, &channels[0].type, NULL);
            parse_table_add(&pt, "BackupSource", PQ_INT, 0, &channels[0].source, NULL);
            parse_table_add(&pt, "BackupFrequency", PQ_INT, (void*)1000, &channels[0].frequency, NULL);
            parse_arg_eq(a, &pt);
            parse_arg_eq_done(&pt);

            rv = bcm_ptp_input_channels_set(unit, 0, clock_num, num_channels, channels);
            return rv ? CMD_FAIL : CMD_OK;
        } 

    } else if (strcmp(arg, "signal") == 0) {
        static bcm_ptp_signal_output_t signal;
        int signal_id;
        arg = ARG_GET(a);
        if (!arg) {
            return CMD_USAGE;
        }
        if (strcmp(arg, "get") == 0) {
            /* XXX */
            return CMD_FAIL;
        } else if (strcmp(arg, "set") == 0) {
            parse_table_t pt;
            parse_table_init(unit, &pt);
            parse_table_add(&pt, "Pin", PQ_INT, (void*)1, &signal.pin, NULL);
            parse_table_add(&pt, "Frequency", PQ_INT, (void*)1, &signal.frequency, NULL);
            parse_table_add(&pt, "PhaseLock", PQ_BOOL, (void*)1, &signal.phase_lock, NULL);
            parse_table_add(&pt, "Width", PQ_INT, (void*)1000, &signal.pulse_width_ns, NULL);
            parse_table_add(&pt, "Offset", PQ_INT, 0, &signal.pulse_offset_ns, NULL);
            parse_arg_eq(a, &pt);
            parse_arg_eq_done(&pt);

            rv = bcm_ptp_signal_output_set(unit, 0, clock_num, &signal_id, &signal);
            return rv ? CMD_FAIL : CMD_OK;
        } 

    } else if (strcmp(arg, "debug") == 0) {
        int unit = 0;    /* for now, just assume unit/ptp_id = 0 */
        int ptp_id = 0;
        int clock_idx = 0;

        arg = ARG_GET(a);
        if (!arg) {
            /* just plain "debug", so dump the current buffer.  Calling with callback time of 0 means it won't self-schedule  */
            output_current_debug(0, 0, (void*)unit, (void*)ptp_id, 0);
            return CMD_OK;
        } else if (strcmp(arg, "start") == 0) {
            output_current_debug(&output_current_debug, (void*)1000, (void*)unit, (void*)ptp_id, 0);
            return CMD_OK;
        } else if (strcmp(arg, "stop") == 0) {
            sal_dpc_cancel(&output_current_debug);
            return CMD_OK;
        } else {            
#if defined(PTP_KEYSTONE_STACK)
            if (esw_ptp_unit_array[unit].memstate == PTP_MEMSTATE_INITIALIZED &&
                esw_ptp_unit_array[unit].stack_array[ptp_id].memstate == PTP_MEMSTATE_INITIALIZED) {
                
                _bcm_ptp_info_t *ptp_info_p = &esw_ptp_info[unit];
                _bcm_ptp_stack_info_t *stack_p = &ptp_info_p->stack_info[ptp_id];
                void *cookie = stack_p->ext_info.cookie;
                stack_p->ext_info.write_fn(cookie, 0x19000c74, strtoul(arg, 0, 16));
            }
            clock_idx = clock_idx;  /* prevent unused warning */
            return CMD_OK;
#else
            rv = _bcm_esw_ptp_log_level_set(unit, ptp_id, clock_idx, strtoul(arg, 0, 16));
            return rv ? CMD_FAIL : CMD_OK;
#endif /* PTP_KEYSTONE_STACK */
        }
    } else {
        printk("  failed on '%s'\n", arg);
    }

    return CMD_USAGE;
}

#endif  /* defined(INCLUDE_PTP) */
