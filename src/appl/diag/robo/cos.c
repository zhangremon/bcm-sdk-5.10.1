/*
 * $Id: cos.c 1.12 Broadcom SDK $
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

#include <appl/diag/system.h>
#include <appl/diag/parse.h>

#include <bcm/error.h>
#include <bcm/cosq.h>
#include <bcm/debug.h>

#include <bcm_int/robo/port.h>

static int robo_cos_first_setting_flag = 0;

/*
 * Manage classes of service
 */

cmd_result_t
cmd_robo_cos(int unit, args_t *a)
{
    int r;
    char *subcmd, *c;
    int fabric;
    bcm_cos_queue_t cosq;
    static int weights[BCM_COS_COUNT];
    static int tmp_weights[BCM_COS_COUNT];
    int delay = 0;
    int numq, mode = BCM_COSQ_STRICT;
    bcm_cos_t prio;
    static bcm_cos_t lastPrio = 0;
    parse_table_t pt;
    cmd_result_t ret_code;
    pbmp_t              pbmp;
    soc_port_t          p;

    if (robo_cos_first_setting_flag == 0) {
        sal_memset(weights, 0, BCM_COS_COUNT * sizeof(int));
        sal_memset(tmp_weights, 0, BCM_COS_COUNT * sizeof(int));
    }

    if (!sh_check_attached(ARG_CMD(a), unit)) {
        return CMD_FAIL;
    }

    fabric = SOC_IS_XGS(unit) && !SOC_IS_XGS_SWITCH(unit);

    if ((subcmd = ARG_GET(a)) == NULL) {
        return CMD_USAGE;
    }

    c = ARG_CUR(a);

    if ((c) == NULL) {
        if (sal_strcasecmp(subcmd, "clear") == 0) {
            if ((r = bcm_cosq_init(unit)) < 0) {
                goto bcm_err;
            }

            return CMD_OK;
        }

        if (sal_strcasecmp(subcmd, "strict") == 0) {
            if ((r = bcm_cosq_sched_set(unit, BCM_COSQ_STRICT,
                        weights, 0)) < 0) {
                goto bcm_err;
            }

            return CMD_OK;
        }

    }

    if ((r = bcm_cosq_config_get(unit, &numq)) < 0) {
        goto bcm_err;
    }

    if (!fabric &&
        (r = bcm_cosq_sched_get(unit, &mode, weights, &delay)) < 0) {
        goto bcm_err;
    }

    if (sal_strcasecmp(subcmd, "port") == 0) {
        if ((subcmd = ARG_GET(a)) == NULL) {
            return CMD_USAGE;
        }

        if (sal_strcasecmp(subcmd, "show") == 0) { 
            char *mode_name;
            int weight_max;

            BCM_PBMP_CLEAR(pbmp);
            parse_table_init(unit, &pt);
            parse_table_add(&pt, "PortBitMap", PQ_DFL|PQ_PBMP,
                            (void *)(0), &pbmp, NULL);

            if (!parseEndOk( a, &pt, &ret_code)) {
                return ret_code;
            }
            if (BCM_PBMP_IS_NULL(pbmp)) {
                printk("%s: ERROR: must specify valid port bitmap.\n",
                       ARG_CMD(a));
                return CMD_FAIL;
            } else {
                PBMP_ITER(pbmp, p) {
                    if (IS_CPU_PORT(unit, p)) {
                        printk("%s: ERROR: No support for CPU port.\n",
                               ARG_CMD(a));
                        return CMD_FAIL;
                    }

                    switch (mode) {
                        case BCM_COSQ_STRICT:
                            mode_name = "strict";
                            break;
                        case BCM_COSQ_WEIGHTED_ROUND_ROBIN:
                            mode_name = "weighted round-robin";
                            break;
                        case BCM_COSQ_DEFICIT_ROUND_ROBIN:
                            mode_name = "deficit round-robin";
                            break;
                        default:
                            mode_name = "?";
                            break;
                    }

                    if ((r = bcm_cosq_sched_weight_max_get(unit,
                                                           mode, &weight_max)) < 0) {
                        goto bcm_err;
                    }
                            
                    printk("\n  Port %s COS configuration:\n", SOC_PORT_NAME(unit, p));
                    printk(" ------------------------------\n");
                    printk("  Config (max queues): %d\n", numq);
                    printk("  Schedule mode: %s\n", mode_name);

                    if (mode != BCM_COSQ_STRICT) {
                        printk("  Weighting (in packets, max_weight = %d):\n", weight_max);

                        for (cosq = 0; cosq < numq; cosq++) {
                            if (!robo_cos_first_setting_flag) {
                                /* show init cos queues weights status */
                                printk("    COSQ %d = %d packets\n", cosq, weights[cosq]);
                            } else {
                                /* show "Strict Priority" if setting cos queue weight = 0 */
                                if (tmp_weights[cosq] == BCM_COSQ_WEIGHT_STRICT) {
                                    printk("    COSQ %d = Strict Priority\n", cosq);
                                } else {
                                    printk("    COSQ %d = %d %s\n", cosq, weights[cosq],
                                        (mode ==BCM_COSQ_DEFICIT_ROUND_ROBIN) ? "64-bytes" : "packets");
                                }
                            }
                        }
                    }

                    printk("  Priority to queue mappings:\n");

                    for (prio = 0; prio < 8; prio++) {
                        if ((r = bcm_cosq_port_mapping_get(unit, p, prio, &cosq)) < 0) {
                            goto bcm_err;
                        }
                        printk("    PRIO %d ==> COSQ %d\n", prio, cosq);
                    } 

                    /* Only BCM53280 on ROBO chips support more than 8 priorities */
                    for (prio = 8; prio < 16; prio++) {
                        if ((r = bcm_cosq_port_mapping_get(unit, p, prio, &cosq)) < 0) {
                            break;
                        }
                        printk("    PRIO %d ==> COSQ %d\n", prio, cosq);
                    }
                }
            }
            return CMD_OK;
        }

        if (sal_strcasecmp(subcmd, "map") == 0) {
            BCM_PBMP_CLEAR(pbmp);
            parse_table_init(unit, &pt);
            parse_table_add(&pt, "PortBitMap", PQ_DFL|PQ_PBMP,
                            (void *)(0), &pbmp, NULL);
            parse_table_add(&pt, "Pri",     PQ_DFL|PQ_INT,
                            (void *)(0), &lastPrio, NULL);
            parse_table_add(&pt, "Queue",   PQ_DFL|PQ_INT,
                            (void *)(0), &cosq, NULL);

            if ((c != NULL) && !parseEndOk( a, &pt, &ret_code)) {
                return ret_code;
            }
            if (BCM_PBMP_IS_NULL(pbmp)) {
                printk("%s: ERROR: must specify valid port bitmap.\n",
                       ARG_CMD(a));
                return CMD_FAIL;
            } else {
                PBMP_ITER(pbmp, p) {
                    r = bcm_cosq_port_mapping_set(unit, p, lastPrio, cosq);
                    if (r < 0) {
                        printk("%s: ERROR: port %s: %s\n",
                               ARG_CMD(a),
                               SOC_PORT_NAME(unit, p),
                               bcm_errmsg(r));
                        return CMD_FAIL;
                    }
                }
            }
            return CMD_OK;
        }

        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "show") == 0) {
        char *mode_name;
        int weight_max;

        if (c != NULL) {
            return CMD_USAGE;
        }

        if (fabric) {
            printk("COS configuration:\n");
            printk("  Config (max queues): %d\n", numq);

            return CMD_OK;
        }

        switch (mode) {
            case BCM_COSQ_STRICT:
                mode_name = "strict";
                break;
            case BCM_COSQ_WEIGHTED_ROUND_ROBIN:
                mode_name = "weighted round-robin";
                break;
            case BCM_COSQ_DEFICIT_ROUND_ROBIN:
                mode_name = "deficit round-robin";
                break;
            default:
                mode_name = "?";
                break;
        }

        if ((r = bcm_cosq_sched_weight_max_get(unit,
                                               mode, &weight_max)) < 0) {
            goto bcm_err;
        }
                
        printk("COS configuration:\n");
        printk("  Config (max queues): %d\n", numq);
        printk("  Schedule mode: %s\n", mode_name);

        if (mode != BCM_COSQ_STRICT) {
            printk("  Weighting (in packets, max_weight = %d):\n", weight_max);

            for (cosq = 0; cosq < numq; cosq++) {
                if (!robo_cos_first_setting_flag) {
                    /* show init cos queues weights status */
                    printk("    COSQ %d = %d packets\n", cosq, weights[cosq]);
                } else {
                    /* show "Strict Priority" if setting cos queue weight = 0 */
                    if (tmp_weights[cosq] == BCM_COSQ_WEIGHT_STRICT) {
                        printk("    COSQ %d = Strict Priority\n", cosq);
                    } else {
                        printk("    COSQ %d = %d %s\n", cosq, weights[cosq],
                            (mode ==BCM_COSQ_DEFICIT_ROUND_ROBIN) ? "64-bytes" : "packets");
                    }
                }
            }
        }

        printk("  Priority to queue mappings:\n");

        for (prio = 0; prio < 8; prio++) {
            if ((r = bcm_cosq_mapping_get(unit, prio, &cosq)) < 0) {
                goto bcm_err;
            }
            printk("    PRIO %d ==> COSQ %d\n", prio, cosq);
        }

        /* Only BCM53280 on ROBO chips support more than 8 priorities */
        for (prio = 8; prio < 16; prio++) {
            if ((r = bcm_cosq_mapping_get(unit, prio, &cosq)) < 0) {
                break;
            }
            printk("    PRIO %d ==> COSQ %d\n", prio, cosq);
        }

        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "config") == 0) {
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "Numcos", PQ_DFL|PQ_INT,
            (void *)( 0), &numq, NULL);

        if (!parseEndOk( a, &pt, &ret_code)) {
            return ret_code;
        }

        if ((r = bcm_cosq_config_set(unit, numq)) < 0) {
            goto bcm_err;
        }

        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "map") == 0) {
        if ((r = bcm_cosq_mapping_get(unit, lastPrio, &cosq)) < 0) {
            goto bcm_err;
        }

        parse_table_init(unit, &pt);
        parse_table_add(&pt, "Pri", PQ_DFL|PQ_INT,
            (void *)(0), &lastPrio, NULL);
        parse_table_add(&pt, "Queue", PQ_DFL|PQ_INT,
            (void *)(0), &cosq, NULL);

        if (!parseEndOk( a, &pt, &ret_code)) {
            return ret_code;
        }

        if ((r = bcm_cosq_mapping_set(unit, lastPrio, cosq)) < 0) {
            goto bcm_err;
        }

        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "weight") == 0 ||
        sal_strcasecmp(subcmd, "drr") == 0) {
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "W0",  PQ_DFL|PQ_INT,
            (void *)( 0), &weights[0], NULL);
        parse_table_add(&pt, "W1",  PQ_DFL|PQ_INT,
            (void *)( 0), &weights[1], NULL);
        parse_table_add(&pt, "W2",  PQ_DFL|PQ_INT,
            (void *)( 0), &weights[2], NULL);
        parse_table_add(&pt, "W3",  PQ_DFL|PQ_INT,
            (void *)( 0), &weights[3], NULL);
        parse_table_add(&pt, "W4",  PQ_DFL|PQ_INT,
            (void *)( 0), &weights[4], NULL);
        parse_table_add(&pt, "W5",  PQ_DFL|PQ_INT,
            (void *)( 0), &weights[5], NULL);
        parse_table_add(&pt, "W6",  PQ_DFL|PQ_INT,
            (void *)( 0), &weights[6], NULL);
        parse_table_add(&pt, "W7",  PQ_DFL|PQ_INT,
            (void *)( 0), &weights[7], NULL);

        if (!parseEndOk( a, &pt, &ret_code)) {
            return ret_code;
        }

        if (sal_strcasecmp(subcmd, "drr") == 0) {
	     mode = BCM_COSQ_DEFICIT_ROUND_ROBIN;
	 } else {
            mode = BCM_COSQ_WEIGHTED_ROUND_ROBIN;
	 }

        if ((r = bcm_cosq_sched_set(unit, mode, weights, 0)) < 0) {
            goto bcm_err;
        }

        /* check first time to set cos queues weights status */
        if (!robo_cos_first_setting_flag) {
            robo_cos_first_setting_flag = 1;
        }
        /* record each time to set cos queues weights value */
        for (cosq = 0 ; cosq < numq ; cosq++) {
            tmp_weights[cosq] = weights[cosq];
        }
        
        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "bandwidth") == 0) {
        uint32 kbits_sec_min, kbits_max_burst, bw_flags;

        BCM_PBMP_CLEAR(pbmp);
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "PortBitMap", PQ_DFL|PQ_PBMP,
                        (void *)(0), &pbmp, NULL);
	parse_table_add(&pt, "Queue", PQ_INT,
			(void *)( 0), &cosq, NULL);
	parse_table_add(&pt, "KbpsMIn", PQ_INT,
			(void *)( 0), &kbits_sec_min, NULL);
	parse_table_add(&pt, "KbitsMaxBurst", PQ_INT,
			(void *)( 0), &kbits_max_burst, NULL);
	parse_table_add(&pt, "Flags", PQ_INT,
			(void *)( 0), &bw_flags, NULL);

        if (!parseEndOk( a, &pt, &ret_code)) {
            return ret_code;
        }

        if (BCM_PBMP_IS_NULL(pbmp)) {
            printk("%s ERROR: empty port bitmap\n", ARG_CMD(a));
            return CMD_FAIL;
        }

        PBMP_ITER(pbmp, p) {
	    if ((r = bcm_cosq_port_bandwidth_set(unit, p, cosq, 
                                                 kbits_sec_min,
                                                 kbits_max_burst,
                                                 bw_flags)) < 0) {
	        goto bcm_err;
	    }
        }

	return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "bandwidth_show") == 0) {
        uint32 kbits_sec_min, kbits_max_burst, bw_flags;

        if (c != NULL) {
	    return CMD_USAGE;
	}

        BCM_PBMP_ASSIGN(pbmp, PBMP_PORT_ALL(unit));
        printk("  COSQ bandwith configuration:\n");

        printk("    port | q | KbpsMin  | KbitsMaxBurst  | Flags\n");

        PBMP_ITER(pbmp, p) {
            printk("    -----+---+----------+----------------+-------\n");
            /* Queue = 0 ~ numq for Non EAV queue  */
            for (cosq = 0; cosq < numq; cosq++) {
                /* There is no bw_flags for Queue 0 ~ numq */
                bw_flags = -1;
                if ((r = bcm_cosq_port_bandwidth_get(unit, p, cosq, 
                      &kbits_sec_min, &kbits_max_burst, 0)) == 0) {
                    printk("    %4s | %d | %8d | %14d | %6d\n",
                           SOC_PORT_NAME(unit, p), cosq, kbits_sec_min,
                           kbits_max_burst, bw_flags);
                }
            }
            if (soc_feature(unit, soc_feature_eav_support)) {
                /* Queue = 4 ~ 5 for EAV queue*/
                for (cosq = 4; cosq < 6; cosq++) {
                    if ((r = bcm_cosq_port_bandwidth_get(unit, p, cosq, 
                               &kbits_sec_min, &kbits_max_burst, &bw_flags)) == 0) {
                        printk("    %4s | %d | %8d | %14d | %6d\n",
                               SOC_PORT_NAME(unit, p), cosq, kbits_sec_min,
                               kbits_max_burst, bw_flags);
                    }
                }
            }
        }
	return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "dpcontrol") == 0) {
        int  dlf_dp_value, dlf_dp_en, xoff_dp_en;

        if ((r = bcm_cosq_control_get
                (unit, -1, -1, bcmCosqControlDpValueDlf, &dlf_dp_value)) < 0) {
            goto bcm_err;
        }  

        if ((r = bcm_cosq_control_get
                (unit, -1, -1, bcmCosqControlDpChangeDlf, &dlf_dp_en)) < 0) {
            goto bcm_err;
        }  

        if ((r = bcm_cosq_control_get
                (unit, -1, -1, bcmCosqControlDpChangeXoff, &xoff_dp_en)) < 0) {
            goto bcm_err;
        }  

        parse_table_init(unit, &pt);
        parse_table_add(&pt, "DlfDpValue",  PQ_DFL|PQ_INT,
            (void *)( 0), &dlf_dp_value, NULL);
        parse_table_add(&pt, "DlfDpEn",  PQ_DFL|PQ_INT,
            (void *)( 0), &dlf_dp_en, NULL);
        parse_table_add(&pt, "XoffDpEn",  PQ_DFL|PQ_INT,
            (void *)( 0), &xoff_dp_en, NULL);

        if (!parseEndOk( a, &pt, &ret_code)) {
            return ret_code;
        }

        if ((r = bcm_cosq_control_set
                (unit, -1, -1, bcmCosqControlDpValueDlf, 
                _BCM_COLOR_DECODING(unit, dlf_dp_value))) < 0) {
            goto bcm_err;
        }  

        if ((r = bcm_cosq_control_set
                (unit, -1, -1, bcmCosqControlDpChangeDlf, dlf_dp_en)) < 0) {
            goto bcm_err;
        }  

        if ((r = bcm_cosq_control_set
                (unit, -1, -1, bcmCosqControlDpChangeXoff, xoff_dp_en)) < 0) {
            goto bcm_err;
        }  

	return CMD_OK;
    }

    return CMD_USAGE;

bcm_err:
    printk("%s: ERROR: %s\n", ARG_CMD(a), bcm_errmsg(r));
    return CMD_FAIL;
}
