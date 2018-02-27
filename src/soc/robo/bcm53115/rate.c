/*
 * $Id: rate.c 1.15.56.2 Broadcom SDK $
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

static int robo53115_ingress_rate_init_flag = 0;

#define BCM53115_IRC_PKT_MASK    0x7f
#define BCM53101_IRC_PKT_MASK    0x3f
#define BCM53115_ENABLE_DUAL_IMP_PORT    0x3

/* Rate for IMP0 and IMP1 in terms of Packets Per Second(PPS) */
static int rate_index_value[64] = {
384, 512, 639, 768, 1024, 1280, 1536, 1791,
2048, 2303, 2559, 2815, 3328, 3840, 4352, 4863,
5376, 5887, 6400, 6911, 7936, 8960, 9984, 11008,
12030, 13054, 14076, 15105, 17146, 19201, 21240, 23299,
25354, 27382, 29446, 31486, 35561, 39682, 42589, 56818,
71023, 85324, 99602, 113636, 127551, 142045, 213675, 284091, 
357143, 423729, 500000, 568182, 641026, 714286, 781250, 862069, 
925926, 1000000, 1086957, 1136364, 1190476, 1250000, 1315789, 1388889
};

/*
 *  Function : _drv_bcm53115_port_irc_set
 *
 *  Purpose :
 *  Set the burst size and rate limit value of the selected port ingress rate.
 *
 *  Parameters :
 *      unit        :   unit id
 *      port   :   port id.
 *      limit  :   rate limit value. (Kbits)
 *      burst_size  :   max burst size. (Kbits)
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      Set the limit and burst size to bucket 0(storm control use bucket1). 
 *
 */
int
_drv_bcm53115_port_irc_set(int unit, uint32 port, 
                              uint32 limit, uint32 burst_size)
{
    uint32  reg_value, temp = 0;
    int     rv= SOC_E_NONE;
    soc_pbmp_t pbmp;
    uint32  burst_kbyte = 0;

    if ((rv = REG_READ_BC_SUP_RATECTRL_Pr(unit, port, &reg_value)) < 0) {
        return rv;
    }
    if (limit == 0) { /* Disable ingress rate control */
        
        /* disable ingress rate control bucket 0*/
        temp = 0;
        soc_BC_SUP_RATECTRL_Pr_field_set(unit, &reg_value, 
            EN_BUCKET0f, &temp);
          
    } else {    /* Enable ingress rate control */
        /* check if global ingress rate config is set */ 
        if (!robo53115_ingress_rate_init_flag) {
            pbmp = PBMP_ALL(unit);
            if (SOC_IS_ROBO53101(unit)) {
                soc_BC_SUP_RATECTRL_Pr_field_get(unit, &reg_value, 
                    EN_VLAN_POLICINGf, &temp);
                if (!temp) {
                    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->rate_config_set)(
                        unit, pbmp, DRV_RATE_CONFIG_PKT_MASK, 
                        BCM53101_IRC_PKT_MASK));
                }
            } else {
                SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->rate_config_set)(
                    unit, pbmp, DRV_RATE_CONFIG_PKT_MASK, 
                    BCM53115_IRC_PKT_MASK));
            }
            robo53115_ingress_rate_init_flag = 1;
        }

        if (burst_size > (500 * 8)) { /* 500 KB */
            return SOC_E_PARAM;
        }
        burst_kbyte = burst_size / 8;
        if (SOC_IS_ROBO53101(unit) || SOC_IS_ROBO53125(unit) ||
            SOC_IS_ROBO53128(unit)) {
            if (burst_kbyte <= 16) { /* 16KB */
                temp = 0;
            } else if (burst_kbyte <= 20) { /* 20KB */
                temp = 1;
            } else if (burst_kbyte <= 28) { /* 28KB */
                temp = 2;
            } else if (burst_kbyte <= 44) { /* 44KB */
                temp = 3;
            } else if (burst_kbyte <= 76) { /* 76KB */
                temp = 4;
            } else if (burst_kbyte <= 500){ /* 500KB */
                temp = 7;
            } 
        } else {
            if (burst_kbyte <= 16) { /* 16KB */
                temp = 0;
            } else if (burst_kbyte <= 20) { /* 20KB */
                temp = 1;
            } else if (burst_kbyte <= 28) { /* 28KB */
                temp = 2;
            } else if (burst_kbyte <= 40) { /* 40KB */
                temp = 3;
            } else if (burst_kbyte <= 76) { /* 76KB */
                temp = 4;
            } else if (burst_kbyte <= 140){ /* 140KB */
                temp = 5;
            } else if (burst_kbyte <= 268){ /* 268KB */
                temp = 6;
            } else if (burst_kbyte <= 500){ /* 500KB */
                temp = 7;
            } 
        }

        soc_BC_SUP_RATECTRL_Pr_field_set(unit, &reg_value, 
            BUCKET0_SIZEf, &temp);

        /* refresh count  (fixed type)*/
        if (limit <= 1792) { /* 64KB ~ 1.792MB */
            temp = ((limit-1) / 64) +1;
        } else if (limit <= 100000){ /* 2MB ~ 100MB */
            temp = (limit /1000 ) + 27;
        } else if (limit <= 1000000){ /* 104MB ~ 1000MB */
            temp = (limit /8000) + 115;
        } else {
            rv = SOC_E_PARAM;
            return rv;
        }

        soc_BC_SUP_RATECTRL_Pr_field_set(unit, &reg_value, 
            BUCKET0_REF_CNTf, &temp);

        /* enable ingress rate control */
        temp = 1;
        soc_BC_SUP_RATECTRL_Pr_field_set(unit, &reg_value, 
            EN_BUCKET0f, &temp);
    }
    /* write register */
    SOC_IF_ERROR_RETURN(
        REG_WRITE_BC_SUP_RATECTRL_Pr(unit, port, &reg_value));
    return rv;
}

/*
 *  Function : _drv_bcm53115_port_irc_get
 *
 *  Purpose :
 *   Get the burst size and rate limit value of the selected port ingress rate.
 *
 *  Parameters :
 *      unit        :   unit id
 *      port   :   port id.
 *      limit  :   rate limit value. (Kbits)
 *      burst_size  :   max burst size. (Kbits)
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      Set the limit and burst size to bucket 0(storm control use bucket1). 
 *
 */
int
_drv_bcm53115_port_irc_get(uint32 unit, uint32 port, 
                                 uint32 *limit, uint32 *burst_size)
{
    uint32  reg_value, temp;
    int     rv = SOC_E_NONE;

    if ((rv = REG_READ_BC_SUP_RATECTRL_Pr(unit, port, &reg_value)) < 0) {
        return rv;
    }

    /* check global ingress rate control setting */
    if (robo53115_ingress_rate_init_flag) {
        temp = 0;
        if (SOC_IS_ROBO53101(unit)) {
            soc_BC_SUP_RATECTRL_Pr_field_get(unit, &reg_value, 
                EN_VLAN_POLICINGf, &temp);
            if (temp) {
                *limit = 0;
                *burst_size = 0;
                return rv; 
            }
        }
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->rate_config_get)(
                    unit, SOC_PBMP_WORD_GET(PBMP_ALL(unit), 0), 
                    DRV_RATE_CONFIG_PKT_MASK, &temp));
                            
        /* if robo_ingress_rate_init_flag=0, only next ingress rate setting 
         * may set the properly PKT_MASK0 again currenly.
         */ 
        robo53115_ingress_rate_init_flag = (temp == 0) ? 0 : 1;
    }

    temp = 0;
    soc_BC_SUP_RATECTRL_Pr_field_get(unit, &reg_value, 
        EN_BUCKET0f, &temp);

    if (temp == 0) {
        *limit = 0;
        *burst_size = 0;
    } else {
        soc_BC_SUP_RATECTRL_Pr_field_get(unit, &reg_value, 
            BUCKET0_SIZEf, &temp);

        if (SOC_IS_ROBO53101(unit) || SOC_IS_ROBO53125(unit) || 
            SOC_IS_ROBO53128(unit)) {
            switch (temp) {
            case 0:
                *burst_size = 16 * 8; /* 16KB */
                break;
            case 1:
                *burst_size = 20 * 8; /* 20KB */
                break;
            case 2:
                *burst_size = 28 * 8; /* 28KB */
                break;
            case 3:
                *burst_size = 44 * 8; /* 44KB */
                break;
            case 4:
                *burst_size = 76 * 8; /* 76KB */
                break;
            case 5:
            case 6:
            case 7:
                *burst_size = 500 * 8; /* 500KB */
                break;

            default:
                return SOC_E_INTERNAL;
            }
        } else {
            switch (temp) {
            case 0:
                *burst_size = 16 * 8; /* 16KB */
                break;
            case 1:
                *burst_size = 20 * 8; /* 20KB */
                break;
            case 2:
                *burst_size = 28 * 8; /* 28KB */
                break;
            case 3:
                *burst_size = 40 * 8; /* 40KB */
                break;
            case 4:
                *burst_size = 76 * 8; /* 76KB */
                break;
            case 5:
                *burst_size = 140 * 8; /* 140KB */
                break;
            case 6:
                *burst_size = 268 * 8; /* 268KB */
                break;
            case 7:
                *burst_size = 500 * 8; /* 500KB */
                break;

            default:
                return SOC_E_INTERNAL;
            }
        }

        soc_BC_SUP_RATECTRL_Pr_field_get(unit, &reg_value, 
            BUCKET0_REF_CNTf, &temp);
        if (temp <= 28) {
            *limit = temp * 64;
        } else if (temp <= 127) {
            *limit = (temp -27) * 1000;
        } else if (temp <=240) {
            *limit = (temp -115) * 1000 * 8;
        } else {
            return SOC_E_INTERNAL;
        }
    }

    return rv;
}


/*
 *  Function : _drv_bcm53115_port_erc_set
 *
 *  Purpose :
 *     Set the burst size and rate limit value of the selected port egress rate.
 *
 *  Parameters :
 *      unit        :   unit id
 *      port   :   port id.
 *      limit  :   rate limit value.
 *      burst_size  :   max burst size.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int
_drv_bcm53115_port_erc_set(uint32 unit, uint32 port, 
                                 uint32 limit, uint32 burst_size)
{
    uint32  reg_value, temp = 0;
    int i, rv = SOC_E_NONE;
    uint32  burst_kbyte = 0;

    soc_cm_debug(DK_PORT, 
    "_drv_bcm53115_port_erc_set: unit = %d, port = %d, limit = %d, burst_size = %d\n",
        unit, port, limit, burst_size);

    if (IS_CPU_PORT(unit, port)) {
        if ((rv = REG_READ_IMP_EGRESS_RATE_CTRL_CFG_REGr(
            unit, &reg_value)) < 0) {
            return rv;
        }

        if (limit > 1388889) {
            return SOC_E_PARAM;
        }

        /* Rate for IMP0 in terms of Packets Per Second(PPS) */
        for(i = 0 ; i < 64 ; i++) {
            if (limit <= rate_index_value[i]) {
                temp = i;
                break;
            }
        }
        soc_IMP_EGRESS_RATE_CTRL_CFG_REGr_field_set(unit, &reg_value,
            RATE_INDEXf, &temp);

        /* write register */
        SOC_IF_ERROR_RETURN(
            REG_WRITE_IMP_EGRESS_RATE_CTRL_CFG_REGr(unit, &reg_value));

    } else {
        if ((rv = REG_READ_GMNGCFGr(unit, &reg_value)) < 0) {
            return rv;
        }

        soc_GMNGCFGr_field_get(unit, &reg_value,
            FRM_MNGPf, &temp);
 
        /* if enable Dual-IMP port (IMP0 and IMP1) */
        /* Support Dual-IMP : BCM53115 */
        if ((SOC_IS_ROBO53115(unit) || SOC_IS_ROBO53125(unit)) && 
            (temp == BCM53115_ENABLE_DUAL_IMP_PORT) && (port == 5)) {
            if ((rv = REG_READ_IMP1_EGRESS_RATE_CTRL_CFG_REGr(
                unit, &reg_value)) < 0) {
                return rv;
            }

            if (limit > 1388889) {
                return SOC_E_PARAM;
            }

            /* Rate for IMP1 in terms of Packets Per Second(PPS) */
            for(i = 0 ; i < 64 ; i++) {
                if (limit <= rate_index_value[i]) {
                    temp = i;
                    break;
                }
            }
            soc_IMP1_EGRESS_RATE_CTRL_CFG_REGr_field_set(unit, &reg_value,
                RATE_INDEXf, &temp);

            /* write register */
            SOC_IF_ERROR_RETURN(
                REG_WRITE_IMP1_EGRESS_RATE_CTRL_CFG_REGr(unit, &reg_value));
        } else {
            if ((rv = REG_READ_PORT_ERC_CONr(unit, port, &reg_value)) < 0) {
                return rv;
            }
            if (limit == 0) { /* Disable ingress rate control */
                temp = 0;
                soc_PORT_ERC_CONr_field_set(unit, &reg_value, 
                    EGRESS_RC_ENf, &temp);
            } else {    /* Enable ingress rate control */
                if (burst_size > (500 * 8)) { /* 500 KB */
                    return SOC_E_PARAM;
                }
                /* burst size */
                burst_kbyte = burst_size / 8;
                if (SOC_IS_ROBO53101(unit) || SOC_IS_ROBO53125(unit) ||
                    SOC_IS_ROBO53128(unit)) {
                    if (burst_kbyte <= 16) { /* 16KB */
                        temp = 0;
                    } else if (burst_kbyte <= 20) { /* 20KB */
                        temp = 1;
                    } else if (burst_kbyte <= 28) { /* 28KB */
                        temp = 2;
                    } else if (burst_kbyte <= 44) { /* 44KB */
                        temp = 3;
                    } else if (burst_kbyte <= 76) { /* 76KB */
                        temp = 4;
                    } else if (burst_kbyte <= 500) { /* 500KB */
                        temp = 7;
                    } 
                } else {
                    if (burst_kbyte <= 16) { /* 16KB */
                        temp = 0;
                    } else if (burst_kbyte <= 20) { /* 20KB */
                        temp = 1;
                    } else if (burst_kbyte <= 28) { /* 28KB */
                        temp = 2;
                    } else if (burst_kbyte <= 40) { /* 40KB */
                        temp = 3;
                    } else if (burst_kbyte <= 76) { /* 76KB */
                        temp = 4;
                    } else if (burst_kbyte <= 140) { /* 140KB */
                        temp = 5;
                    } else if (burst_kbyte <= 268) { /* 268KB */
                        temp = 6;
                    } else if (burst_kbyte <= 500) { /* 500KB */
                        temp = 7;
                    } 
                }

                soc_PORT_ERC_CONr_field_set(unit, &reg_value, 
                    BUCKET_SIZEf, &temp);

                /* refresh count  (fixed type)*/
                if (limit <= 1792) { /* 64KB ~ 1.792MB */
                    temp = ((limit-1) / 64) +1;
                } else if (limit <= 102400){ /* 2MB ~ 100MB */
                    temp = (limit /1000 ) + 27;
                } else if (limit <= 1000000) { /* 104MB ~ 1000MB */
                    temp = (limit /8000) + 115;
                } else {
                    rv = SOC_E_PARAM;
                    return rv;
                }
                if (SOC_IS_ROBO53101(unit) || SOC_IS_ROBO53125(unit) ||
                    SOC_IS_ROBO53128(unit)) {
                    soc_PORT_ERC_CONr_field_set(unit, &reg_value, 
                        REF_CNTf, &temp);
                } else {
                    soc_PORT_ERC_CONr_field_set(unit, &reg_value, 
                        REF_CNT_Rf, &temp);
                }

                /* enable ingress rate control */
                temp = 1;
                soc_PORT_ERC_CONr_field_set(unit, &reg_value, 
                        EGRESS_RC_ENf, &temp);
            }
            /* write register */
            SOC_IF_ERROR_RETURN(
                REG_WRITE_PORT_ERC_CONr(unit, port, &reg_value));
        }
    }
    return rv;
}

/*
 *  Function : _drv_bcm53115_port_erc_get
 *
 *  Purpose :
 *     Get the burst size and rate limit value of the selected port.
 *
 *  Parameters :
 *      unit        :   unit id
 *      port   :   port id.
 *      limit  :   rate limit value.
 *      burst_size  :   max burst size.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int
_drv_bcm53115_port_erc_get(uint32 unit, uint32 port, 
                                 uint32 *limit, uint32 *burst_size)
{
    uint32  reg_addr, reg_value, reg_len, temp = 0;
    int     rv= SOC_E_NONE;

    if (IS_CPU_PORT(unit, port)) {
        if ((rv = REG_READ_IMP_EGRESS_RATE_CTRL_CFG_REGr(
            unit, &reg_value)) < 0) {
            return rv;
        }
        soc_IMP_EGRESS_RATE_CTRL_CFG_REGr_field_get(unit, &reg_value,
            RATE_INDEXf, &temp);

        /* Rate for IMP0 in terms of Packets Per Second(PPS) */
        *limit = rate_index_value[temp];
        *burst_size = 0;
    } else {
        
        if ((rv = REG_READ_GMNGCFGr(unit, &reg_value)) < 0) {
            return rv;
        }        

        soc_GMNGCFGr_field_get(unit, &reg_value,
            FRM_MNGPf, &temp);

        /* if enable Dual-IMP port (IMP0 and IMP1) */
        /* Support Dual-IMP : BCM53115 */
        if ((SOC_IS_ROBO53115(unit)) &&
            (temp == BCM53115_ENABLE_DUAL_IMP_PORT) && (port == 5)) {
            if ((rv = REG_READ_IMP1_EGRESS_RATE_CTRL_CFG_REGr(
                unit, &reg_value)) < 0) {
                return rv;
            }
            soc_IMP1_EGRESS_RATE_CTRL_CFG_REGr_field_get(unit, &reg_value,
                RATE_INDEXf, &temp);

            /* Rate for IMP1 in terms of Packets Per Second(PPS) */
            *limit = rate_index_value[temp];
            *burst_size = 0;
        } else if ((SOC_IS_ROBO53101(unit) ||SOC_IS_ROBO53125(unit)) &&
            (port == 5) && (temp == BCM53115_ENABLE_DUAL_IMP_PORT)) {
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, INDEX(IMP_EGRESS_RATE_CTRL_CFG_REGr), 0, 1);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, INDEX(IMP_EGRESS_RATE_CTRL_CFG_REGr));
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
                return rv;
            }
            soc_IMP_EGRESS_RATE_CTRL_CFG_REGr_field_get(unit, &reg_value,
                RATE_INDEXf, &temp);
            
            /* Rate for IMP0 in terms of Packets Per Second(PPS) */
            *limit = rate_index_value[temp];
            *burst_size = 0;
        } else {
            if ((rv = REG_READ_PORT_ERC_CONr(unit, port, &reg_value)) < 0) {
                return rv;
            }
            soc_PORT_ERC_CONr_field_get(unit, &reg_value, 
                    EGRESS_RC_ENf, &temp);
            if (temp ==0) {
                *limit = 0;
                *burst_size = 0;
            } else {
                soc_PORT_ERC_CONr_field_get(unit, &reg_value, 
                    BUCKET_SIZEf, &temp);

                if (SOC_IS_ROBO53101(unit) || SOC_IS_ROBO53125(unit) ||
                    SOC_IS_ROBO53128(unit)) {
                    switch (temp) {
                        case 0:
                            *burst_size = 16 * 8; /* 16KB */
                            break;
                        case 1:
                            *burst_size = 20 * 8; /* 20KB */
                            break;
                        case 2:
                            *burst_size = 28 * 8; /* 28KB */
                            break;
                        case 3:
                            *burst_size = 44 * 8; /* 44KB */
                            break;
                        case 4:
                            *burst_size = 76 * 8; /* 76KB */
                            break;
                        case 5:
                        case 6:
                        case 7:
                            *burst_size = 500 * 8; /* 500KB */
                            break;
                        default:
                            return SOC_E_INTERNAL;
                    }
                } else {
                    switch (temp) {
                        case 0:
                            *burst_size = 16 * 8; /* 16KB */
                            break;
                        case 1:
                            *burst_size = 20 * 8; /* 20KB */
                            break;
                        case 2:
                            *burst_size = 28 * 8; /* 28KB */
                            break;
                        case 3:
                            *burst_size = 40 * 8; /* 40KB */
                            break;
                        case 4:
                            *burst_size = 76 * 8; /* 76KB */
                            break;
                        case 5:
                            *burst_size = 140 * 8; /* 140KB */
                            break;
                        case 6:
                            *burst_size = 268 * 8; /* 268KB */
                            break;
                        case 7:
                            *burst_size = 500 * 8; /* 500KB */
                            break;
                        default:
                            return SOC_E_INTERNAL;
                    }
                }

                if (SOC_IS_ROBO53101(unit) || SOC_IS_ROBO53125(unit) ||
                    SOC_IS_ROBO53128(unit)) {
                    soc_PORT_ERC_CONr_field_get(unit, &reg_value, 
                        REF_CNTf, &temp);
                } else {
                    soc_PORT_ERC_CONr_field_get(unit, &reg_value, 
                        REF_CNT_Rf, &temp);
                }
                if (temp <= 28) {
                    *limit = temp * 64;
                } else if (temp <= 127) {
                    *limit = (temp -27) * 1000;
                } else if (temp <=240) {
                    *limit = (temp -115) * 1000 * 8;
                } else {
                    return SOC_E_INTERNAL;
                }
            }
        }
    }

    soc_cm_debug(DK_PORT, 
    "_drv_bcm53115_port_erc_get: unit = %d, port = %d, limit = %dK,\
      burst size = %dKB\n", unit, port, *limit, *burst_size);
      
    return rv;
}

/*
 *  Function : drv_bcm53115_rate_config_set
 *
 *  Purpose :
 *      Set the rate control type value to the selected ports.
 *
 *  Parameters :
 *      unit        :   unit id
 *      pbmp   :   port bitmap.
 *      config_type  :   rate control type.
 *      value  :   value of rate control type.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int 
drv_bcm53115_rate_config_set(int unit, soc_pbmp_t pbmp, uint32 config_type, 
    uint32 value)
{
    uint32  reg_value, temp;
    int     rv = SOC_E_NONE;

    soc_cm_debug(DK_PORT, 
    "drv_bcm53115_rate_config_set: unit = %d, bmp = %x, type = %d, value = %d\n",
        unit, SOC_PBMP_WORD_GET(pbmp, 0), config_type, value);
    /* set bucket 0*/
    switch (config_type) {
        case DRV_RATE_CONFIG_RATE_TYPE: 
            /* per chip */
            if (SOC_PBMP_EQ(pbmp, PBMP_ALL(unit))) {
                if ((rv = REG_READ_COMM_IRC_CONr(unit, &reg_value)) < 0) {
                    return rv;
                }
                temp = value;  
                soc_COMM_IRC_CONr_field_set(unit, &reg_value, 
                    RATE_TYPE0f, &temp);
                SOC_IF_ERROR_RETURN(
                    REG_WRITE_COMM_IRC_CONr(unit, &reg_value));
            }
            break;
        case DRV_RATE_CONFIG_DROP_ENABLE:
            /* per chip */
            if (SOC_PBMP_EQ(pbmp, PBMP_ALL(unit))) {
                if ((rv = REG_READ_COMM_IRC_CONr(unit, &reg_value)) < 0) {
                    return rv;
                }
                temp = value;
                soc_COMM_IRC_CONr_field_set(unit, &reg_value, 
                    DROP_EN0f, &temp);
                SOC_IF_ERROR_RETURN(
                    REG_WRITE_COMM_IRC_CONr(unit, &reg_value));
            }
            break;
        case DRV_RATE_CONFIG_PKT_MASK: 
            /* per chip */
            if (SOC_PBMP_EQ(pbmp, PBMP_ALL(unit))) {
                if ((rv = REG_READ_COMM_IRC_CONr(unit, &reg_value)) < 0) {
                    return rv;
                }
                if (SOC_IS_ROBO53101(unit) || SOC_IS_ROBO53125(unit) ||
                    SOC_IS_ROBO53128(unit)) {
                    temp = value & BCM53101_IRC_PKT_MASK;
                } else {
                    temp = value & BCM53115_IRC_PKT_MASK;
                }
                soc_COMM_IRC_CONr_field_set(unit, &reg_value, 
                    PKT_MSK0f, &temp);
                    
                SOC_IF_ERROR_RETURN(
                    REG_WRITE_COMM_IRC_CONr(unit, &reg_value));

            }
            break;
        default:
            return SOC_E_PARAM;
    }
    return rv;
}

/*
 *  Function : drv_bcm_53115_rate_config_get
 *
 *  Purpose :
 *      Get the rate control type value to the selected ports.
 *
 *  Parameters :
 *      unit        :   unit id
 *      pbmp   :   port bitmap.
 *      config_type  :   rate control type.
 *      value  :   value of rate control type.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int 
drv_bcm53115_rate_config_get(int unit, uint32 port, uint32 config_type, 
     uint32 *value)
{
    uint32  reg_value, temp = 0;
    int     rv = SOC_E_NONE;

    /* get bucket 0*/
    switch (config_type) {
        case DRV_RATE_CONFIG_RATE_TYPE: /* per chip */
            if ((rv = REG_READ_COMM_IRC_CONr(unit, &reg_value)) < 0) {
                return rv;
            }
            soc_COMM_IRC_CONr_field_get(unit, &reg_value, 
                    RATE_TYPE0f, &temp);
            *value = temp;
            break;

        case DRV_RATE_CONFIG_DROP_ENABLE: /* per chip */
            if ((rv = REG_READ_COMM_IRC_CONr(unit, &reg_value)) < 0) {
                return rv;
            }
            soc_COMM_IRC_CONr_field_get(unit, &reg_value, 
                    DROP_EN0f, &temp);
            *value = temp;
            break;
        case DRV_RATE_CONFIG_PKT_MASK: /* per chip */
            if ((rv = REG_READ_COMM_IRC_CONr(unit, &reg_value)) < 0) {
                return rv;
            }
            soc_COMM_IRC_CONr_field_get(unit, &reg_value, 
                    PKT_MSK0f, &temp);
            *value = temp;
            break;
        default:
            return SOC_E_PARAM;
    }
    soc_cm_debug(DK_PORT, 
   "drv_bcm53115_rate_config_get: unit = %d, port = %d, type = %d, value = %d\n",
        unit, port, config_type, *value);
    return rv;
}

/*
 *  Function : drv_bcm53115_rate_set
 *
 *  Purpose :
 *      Set the ingress/egress rate control to the selected ports.
 *
 *  Parameters :
 *      unit        :   unit id
 *      bmp   :   port bitmap.
 *      direction   :   direction of rate control (ingress/egress). 
 *      kbits_sec_min  :  minimum bandwidth, kbits/sec.
 *      kbits_sec_max  :  maximum bandwidth, kbits/sec.
 *      burst_size  :   max burst size.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int 
drv_bcm53115_rate_set(int unit, soc_pbmp_t bmp, uint8 queue_n, int direction, 
    uint32 kbits_sec_min, uint32 kbits_sec_max, uint32 burst_size)
{
    uint32      port;
    int         rv = SOC_E_NONE;

    soc_cm_debug(DK_PORT, 
        "drv_bcm53115_rate_set: unit = %d, bmp = %x, %s, \
        kbits_sec_min = %dK, kbits_sec_max = %dK, burst size = %dKB\n", 
        unit, SOC_PBMP_WORD_GET(bmp, 0), (direction - 1) ? "EGRESS" : "INGRESS", 
        kbits_sec_min, kbits_sec_max, burst_size);
    switch (direction) {
        case DRV_RATE_CONTROL_DIRECTION_INGRESSS:
            PBMP_ITER(bmp, port) {
                SOC_IF_ERROR_RETURN(
                    _drv_bcm53115_port_irc_set(unit, port, 
                                            kbits_sec_max, burst_size));
            }
            break;
        case DRV_RATE_CONTROL_DIRECTION_EGRESSS:
            PBMP_ITER(bmp, port) {
                SOC_IF_ERROR_RETURN(
                    _drv_bcm53115_port_erc_set(unit, port, 
                                            kbits_sec_max, burst_size));
            }    
            break;
        case DRV_RATE_CONTROL_DIRECTION_EGRESSS_PER_QUEUE:
            return SOC_E_UNAVAIL;
        default:
            return SOC_E_PARAM;
    }
    return rv;
}

/*
 *  Function : drv_bcm53115_rate_get
 *
 *  Purpose :
 *      Get the ingress/egress rate control to the selected ports.
 *
 *  Parameters :
 *      unit        :   unit id
 *      bmp   :   port bitmap.
 *      direction   :   direction of rate control (ingress/egress). 
 *      kbits_sec_min  :  (OUT) minimum bandwidth, kbits/sec.
 *      kbits_sec_max  :  (OUT) maximum bandwidth, kbits/sec.
 *      burst_size  :   max burst size.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int 
drv_bcm53115_rate_get(int unit, uint32 port, uint8 queue_n, int direction, 
    uint32 *kbits_sec_min, uint32 *kbits_sec_max, uint32 *burst_size)
{
    int         rv = SOC_E_NONE;
    uint32 min_rate = 0; /* Dummy variable */

    switch (direction) {
        case DRV_RATE_CONTROL_DIRECTION_INGRESSS:
            SOC_IF_ERROR_RETURN(
                    _drv_bcm53115_port_irc_get(unit, port, 
                                        kbits_sec_max, burst_size));
            break;
        case DRV_RATE_CONTROL_DIRECTION_EGRESSS:
            SOC_IF_ERROR_RETURN(
                    _drv_bcm53115_port_erc_get(unit, port, 
                                        kbits_sec_max, burst_size));
            break;
        case DRV_RATE_CONTROL_DIRECTION_EGRESSS_PER_QUEUE:
            return SOC_E_UNAVAIL;
        default:
            return SOC_E_PARAM;
    }
    soc_cm_debug(DK_PORT, 
        "drv_bcm53115_rate_get: unit = %d, port = %d, %s, \
        kbits_sec_min = %dK, kbits_sec_max = %dK, burst size = %dKB\n",
        unit, port, (direction - 1) ? "EGRESS" : "INGRESS", 
        min_rate, *kbits_sec_max, *burst_size);

    return rv;
}
