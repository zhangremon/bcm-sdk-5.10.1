/*
 * $Id: storm.c 1.14 Broadcom SDK $
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

#define SUPPRESSION_DLF_MASK    0x20
#define SUPPRESSION_BPDU_MASK   0x10
#define SUPPRESSION_BROADCAST_MASK  0x0c
#define SUPPRESSION_MULTICAST_MASK  0x02
#define SUPPRESSION_UNICAST_MASK    0x01

#define BCM5348_SUPPRESSION_UNKNOW_SA_MASK    0x40
#define BCM5348_SUPPRESSION_DLF_MASK          0x20
#define BCM5348_SUPPRESSION_MLF_MASK          0x10
#define BCM5348_SUPPRESSION_BROADCAST_MASK    0x08
#define BCM5348_SUPPRESSION_BPDU_MASK         0x04
#define BCM5348_SUPPRESSION_MULTICAST_MASK    0x02
#define BCM5348_SUPPRESSION_UNICAST_MASK      0x01

/*
 *  Function : _drv_storm_control_type_enable_set
 *
 *  Purpose :
 *      Set the status and types of storm control.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      port   :   port number.
 *      type  :   storm control types.
 *      enable   :   status of the storm control types.
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      
 *
 */
int 
_drv_storm_control_type_enable_set(int unit, uint32 port, 
    uint32 type, uint32 enable)
{
    uint32  reg_addr, reg_len, temp;
    uint64  reg_value64;
    uint32  reg_value;
    uint32  reg_index; 
    uint32 bcast_fld_index= 0, mcast_fld_index = 0, dlf_fld_index = 0;
    int     rv= SOC_E_NONE;

    if(SOC_IS_ROBODINO(unit)){
        /* Enable drop_enable bit of bucket 1 */
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, COMM_IRC_CONr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, COMM_IRC_CONr);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            return rv;
        }
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, COMM_IRC_CONr, &reg_value, PKT_MSK1f, &temp));

        if (type & DRV_STORM_CONTROL_BCAST) {
            if (enable) {
                temp |= SUPPRESSION_BROADCAST_MASK;
            } else {
                temp &= ~SUPPRESSION_BROADCAST_MASK;
            }
        }
        if (type & DRV_STORM_CONTROL_MCAST) {
            if (enable) {
                temp |= SUPPRESSION_MULTICAST_MASK;
            } else {
                temp &= ~SUPPRESSION_MULTICAST_MASK;
            }
        } 
        if (type & DRV_STORM_CONTROL_DLF) {
            if (enable) {
                temp |= SUPPRESSION_DLF_MASK;
            } else {
                temp &= ~SUPPRESSION_DLF_MASK;
            }
        }
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, COMM_IRC_CONr, &reg_value, PKT_MSK1f, &temp));

        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value,reg_len));

        return rv;
    }
    if(SOC_IS_ROBO5324(unit)){
        if (port <16) {
            reg_index = MCAST_SUP0r;
        } else {
            reg_index = MCAST_SUP1r;
        }
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, reg_index, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, reg_index);

        switch (port) {
            case 0:
                if (type & DRV_STORM_CONTROL_BCAST) {
                    bcast_fld_index = EN_P0_BCASTf;
                }
                if (type & DRV_STORM_CONTROL_MCAST) {
                    mcast_fld_index = EN_P0_MCASTf;
                } 
                if (type & DRV_STORM_CONTROL_DLF) {
                    dlf_fld_index = EN_P0_DLFf;
                }
                break;
            case 1:
                if (type & DRV_STORM_CONTROL_BCAST) {
                   bcast_fld_index = EN_P1_BCASTf;
                }
                if (type & DRV_STORM_CONTROL_MCAST) {
                    mcast_fld_index = EN_P1_MCASTf;
                } 
                if (type & DRV_STORM_CONTROL_DLF) {
                    dlf_fld_index = EN_P1_DLFf;
                }
                break;
            case 2:
                if (type & DRV_STORM_CONTROL_BCAST) {
                    bcast_fld_index = EN_P2_BCASTf;
                }
                if (type & DRV_STORM_CONTROL_MCAST) {
                    mcast_fld_index = EN_P2_MCASTf;
                } 
                if (type & DRV_STORM_CONTROL_DLF) {
                    dlf_fld_index = EN_P2_DLFf;
                }
                break;
            case 3:
                if (type & DRV_STORM_CONTROL_BCAST) {
                    bcast_fld_index = EN_P3_BCASTf;
                }
                if (type & DRV_STORM_CONTROL_MCAST) {
                    mcast_fld_index = EN_P3_MCASTf;
                } 
                if (type & DRV_STORM_CONTROL_DLF) {
                    dlf_fld_index = EN_P3_DLFf;
                }
               break;
            case 4:
                if (type & DRV_STORM_CONTROL_BCAST) {
                    bcast_fld_index = EN_P4_BCASTf;
                }
                if (type & DRV_STORM_CONTROL_MCAST) {
                    mcast_fld_index = EN_P4_MCASTf;
                } 
                if (type & DRV_STORM_CONTROL_DLF) {
                    dlf_fld_index = EN_P4_DLFf;
                }
                break;
            case 5:
                if (type & DRV_STORM_CONTROL_BCAST) {
                    bcast_fld_index = EN_P5_BCASTf;
                }
                if (type & DRV_STORM_CONTROL_MCAST) {
                    mcast_fld_index = EN_P5_MCASTf;
                } 
                if (type & DRV_STORM_CONTROL_DLF) {
                    dlf_fld_index = EN_P5_DLFf;
                }
                break;
            case 6:
                if (type & DRV_STORM_CONTROL_BCAST) {
                    bcast_fld_index = EN_P6_BCASTf;
                }
                if (type & DRV_STORM_CONTROL_MCAST) {
                    mcast_fld_index = EN_P6_MCASTf;
                } 
                if (type & DRV_STORM_CONTROL_DLF) {
                    dlf_fld_index = EN_P6_DLFf;
                }
                break;
            case 7:
                if (type & DRV_STORM_CONTROL_BCAST) {
                    bcast_fld_index = EN_P7_BCASTf;
                }
                if (type & DRV_STORM_CONTROL_MCAST) {
                    mcast_fld_index = EN_P7_MCASTf;
                } 
                if (type & DRV_STORM_CONTROL_DLF) {
                    dlf_fld_index = EN_P7_DLFf;
                }
                break;
            case 8:
                if (type & DRV_STORM_CONTROL_BCAST) {
                    bcast_fld_index = EN_P8_BCASTf;
                }
                if (type & DRV_STORM_CONTROL_MCAST) {
                    mcast_fld_index = EN_P8_MCASTf;
                } 
                if (type & DRV_STORM_CONTROL_DLF) {
                    dlf_fld_index = EN_P8_DLFf;
                }
                break;
            case 9:
                if (type & DRV_STORM_CONTROL_BCAST) {
                    bcast_fld_index = EN_P9_BCASTf;
                }
                if (type & DRV_STORM_CONTROL_MCAST) {
                    mcast_fld_index = EN_P9_MCASTf;
                } 
                if (type & DRV_STORM_CONTROL_DLF) {
                    dlf_fld_index = EN_P9_DLFf;
                }
                break;
            case 10:
                if (type & DRV_STORM_CONTROL_BCAST) {
                    bcast_fld_index = EN_P10_BCASTf;
                }
                if (type & DRV_STORM_CONTROL_MCAST) {
                    mcast_fld_index = EN_P10_MCASTf;
                } 
                if (type & DRV_STORM_CONTROL_DLF) {
                    dlf_fld_index = EN_P10_DLFf;
                }
                break;
            case 11:
                if (type & DRV_STORM_CONTROL_BCAST) {
                    bcast_fld_index = EN_P11_BCASTf;
                }   
                if (type & DRV_STORM_CONTROL_MCAST) {
                    mcast_fld_index = EN_P11_MCASTf;
                } 
                if (type & DRV_STORM_CONTROL_DLF) {
                    dlf_fld_index = EN_P11_DLFf;
                }
                break;
            case 12:
                if (type & DRV_STORM_CONTROL_BCAST) {
                    bcast_fld_index = EN_P12_BCASTf;
                }
                if (type & DRV_STORM_CONTROL_MCAST) {
                    mcast_fld_index = EN_P12_MCASTf;
                } 
                if (type & DRV_STORM_CONTROL_DLF) {
                    dlf_fld_index = EN_P12_DLFf;
                }
                break;
            case 13:
                if (type & DRV_STORM_CONTROL_BCAST) {
                    bcast_fld_index = EN_P13_BCASTf;
                }
                if (type & DRV_STORM_CONTROL_MCAST) {
                    mcast_fld_index = EN_P13_MCASTf;
                } 
                if (type & DRV_STORM_CONTROL_DLF) {
                    dlf_fld_index = EN_P13_DLFf;
                }
                break;
            case 14:
                if (type & DRV_STORM_CONTROL_BCAST) {
                    bcast_fld_index = EN_P14_BCASTf;
                }
                if (type & DRV_STORM_CONTROL_MCAST) {
                    mcast_fld_index = EN_P14_MCASTf;
                } 
                if (type & DRV_STORM_CONTROL_DLF) {
                    dlf_fld_index = EN_P14_DLFf;
                }
                break;
            case 15:
                if (type & DRV_STORM_CONTROL_BCAST) {
                    bcast_fld_index = EN_P15_BCASTf;
                }
                if (type & DRV_STORM_CONTROL_MCAST) {
                    mcast_fld_index = EN_P15_MCASTf;
                } 
                if (type & DRV_STORM_CONTROL_DLF) {
                    dlf_fld_index = EN_P15_DLFf;
                }
                break;
            case 16:
                if (type & DRV_STORM_CONTROL_BCAST) {
                    bcast_fld_index = EN_P16_BCASTf;
                }
                if (type & DRV_STORM_CONTROL_MCAST) {
                    mcast_fld_index = EN_P16_MCASTf;
                } 
                if (type & DRV_STORM_CONTROL_DLF) {
                    dlf_fld_index = EN_P16_DLFf;
                }
                break;
            case 17:
                if (type & DRV_STORM_CONTROL_BCAST) {
                    bcast_fld_index = EN_P17_BCASTf;
                }
                if (type & DRV_STORM_CONTROL_MCAST) {
                    mcast_fld_index = EN_P17_MCASTf;
                } 
                if (type & DRV_STORM_CONTROL_DLF) {
                    dlf_fld_index = EN_P17_DLFf;
                }
                break;
            case 18:
                if (type & DRV_STORM_CONTROL_BCAST) {
                    bcast_fld_index = EN_P18_BCASTf;
                }
                if (type & DRV_STORM_CONTROL_MCAST) {
                    mcast_fld_index = EN_P18_MCASTf;
                } 
                if (type & DRV_STORM_CONTROL_DLF) {
                    dlf_fld_index = EN_P18_DLFf;
                }
                break;
            case 19:
                if (type & DRV_STORM_CONTROL_BCAST) {
                    bcast_fld_index = EN_P19_BCASTf;
                }
                if (type & DRV_STORM_CONTROL_MCAST) {
                    mcast_fld_index = EN_P19_MCASTf;
                } 
                if (type & DRV_STORM_CONTROL_DLF) {
                    dlf_fld_index = EN_P19_DLFf;
                }
                break;
            case 20:
                if (type & DRV_STORM_CONTROL_BCAST) {
                    bcast_fld_index = EN_P20_BCASTf;
                }
                if (type & DRV_STORM_CONTROL_MCAST) {
                    mcast_fld_index = EN_P20_MCASTf;
                } 
                if (type & DRV_STORM_CONTROL_DLF) {
                    dlf_fld_index = EN_P20_DLFf;
                }
                break;
            case 21:
                if (type & DRV_STORM_CONTROL_BCAST) {
                    bcast_fld_index = EN_P21_BCASTf;
                }
                if (type & DRV_STORM_CONTROL_MCAST) {
                    mcast_fld_index = EN_P21_MCASTf;
                } 
                if (type & DRV_STORM_CONTROL_DLF) {
                    dlf_fld_index = EN_P21_DLFf;
                }
                break;
            case 22:
                if (type & DRV_STORM_CONTROL_BCAST) {
                    bcast_fld_index = EN_P22_BCASTf;
                }
                if (type & DRV_STORM_CONTROL_MCAST) {
                    mcast_fld_index = EN_P22_MCASTf;
                } 
                if (type & DRV_STORM_CONTROL_DLF) {
                    dlf_fld_index = EN_P22_DLFf;
                }
                break;
            case 23:
                if (type & DRV_STORM_CONTROL_BCAST) {
                    bcast_fld_index = EN_P23_BCASTf;
                }
                if (type & DRV_STORM_CONTROL_MCAST) {
                    mcast_fld_index = EN_P23_MCASTf;
                } 
                if (type & DRV_STORM_CONTROL_DLF) {
                    dlf_fld_index = EN_P23_DLFf;
                }
                break;
            case 24:
                if (type & DRV_STORM_CONTROL_BCAST) {
                    bcast_fld_index = EN_P24_BCASTf;
                }
                if (type & DRV_STORM_CONTROL_MCAST) {
                    mcast_fld_index = EN_P24_MCASTf;
                } 
                if (type & DRV_STORM_CONTROL_DLF) {
                    dlf_fld_index = EN_P24_DLFf;
                }
                break;
            case 25:
                if (type & DRV_STORM_CONTROL_BCAST) {
                    bcast_fld_index = EN_P25_BCASTf;
                }
                if (type & DRV_STORM_CONTROL_MCAST) {
                    mcast_fld_index = EN_P25_MCASTf;
                } 
                if (type & DRV_STORM_CONTROL_DLF) {
                    dlf_fld_index = EN_P25_DLFf;
                }
                break;
            case 26:
                if (type & DRV_STORM_CONTROL_BCAST) {
                    bcast_fld_index = EN_P26_BCASTf;
                }
                if (type & DRV_STORM_CONTROL_MCAST) {
                    mcast_fld_index = EN_P26_MCASTf;
                } 
                if (type & DRV_STORM_CONTROL_DLF) {
                    dlf_fld_index = EN_P26_DLFf;
                }
                break;
            default:
                rv =SOC_E_PARAM;
        }
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, (uint32 *)&reg_value64, reg_len)) < 0) {
            return rv;
        }
    
        temp = enable;
        if (bcast_fld_index) {
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                (unit, reg_index, (uint32 *)&reg_value64, bcast_fld_index, &temp));
        }
        if (mcast_fld_index) {
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                (unit, reg_index, (uint32 *)&reg_value64, mcast_fld_index, &temp));
        }
        if (dlf_fld_index) {
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                (unit, reg_index, (uint32 *)&reg_value64, dlf_fld_index, &temp));
        }
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, (uint32 *)&reg_value64,reg_len));
        return rv;
    }

    if(SOC_IS_ROBO5348(unit)||SOC_IS_ROBO5347(unit)){
        /* Enable drop_enable bit of bucket 1 */
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, PORT_IRC_CONr, port, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, PORT_IRC_CONr);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, (uint32 *)&reg_value64, reg_len)) < 0) {
            return rv;
        }
       
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, PORT_IRC_CONr, (uint32 *)&reg_value64, PKT_MSK1f, &temp));

        if (type & DRV_STORM_CONTROL_BCAST) {
            if (enable) {
                temp |= BCM5348_SUPPRESSION_BROADCAST_MASK;
            } else {
                temp &= ~BCM5348_SUPPRESSION_BROADCAST_MASK;
            }
        }
        if (type & DRV_STORM_CONTROL_MCAST) {
            if (enable) {
                temp |= (BCM5348_SUPPRESSION_MULTICAST_MASK |
                                BCM5348_SUPPRESSION_MLF_MASK);
            } else {
                temp &= ~(BCM5348_SUPPRESSION_MULTICAST_MASK |
                                    BCM5348_SUPPRESSION_MLF_MASK);
            }
        } 
        if (type & DRV_STORM_CONTROL_DLF) {
            if (enable) {
                temp |= BCM5348_SUPPRESSION_DLF_MASK;
            } else {
                temp &= ~BCM5348_SUPPRESSION_DLF_MASK;
            }
        }
        if (type & DRV_STORM_CONTROL_SALF) {
            if (enable) {
                temp |= BCM5348_SUPPRESSION_UNKNOW_SA_MASK;
            } else {
                temp &= ~BCM5348_SUPPRESSION_UNKNOW_SA_MASK;
            }
        }
        if (type & DRV_STORM_CONTROL_RSV_MCAST) {
            if (enable) {
                temp |= BCM5348_SUPPRESSION_BPDU_MASK;
            } else {
                temp &= ~BCM5348_SUPPRESSION_BPDU_MASK;
            }
        }
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, PORT_IRC_CONr, (uint32 *)&reg_value64, PKT_MSK1f, &temp));

        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, (uint32 *)&reg_value64,reg_len));

        return rv;
    }
    return rv;
}

/*
 *  Function : _drv_storm_control_type_enable_get
 *
 *  Purpose :
 *      Get the status and types of storm control.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      port   :   port number.
 *      type  :   storm control types.
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      
 *
 */
int 
_drv_storm_control_type_enable_get(int unit, uint32 port, uint32 *type)
{
    uint32  reg_addr, reg_len, temp;
    uint64 reg_value64;
    uint32  reg_value;
    uint32  reg_index; 
    uint32 bcast_fld_index = 0, mcast_fld_index = 0, dlf_fld_index = 0;
    int     rv= SOC_E_NONE;

    if(SOC_IS_ROBODINO(unit)){
        /* Enable drop_enable bit of bucket 1 */
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, COMM_IRC_CONr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, COMM_IRC_CONr);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            return rv;
        }
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, COMM_IRC_CONr, &reg_value, PKT_MSK1f, &temp));
        if (temp & SUPPRESSION_BROADCAST_MASK) {
            *type |= DRV_STORM_CONTROL_BCAST;
        }
    
        if (temp & SUPPRESSION_MULTICAST_MASK) {
            *type |= DRV_STORM_CONTROL_MCAST;
        }
    
        if (temp & SUPPRESSION_DLF_MASK) {
            *type |= DRV_STORM_CONTROL_DLF;
        }
        return rv;
    }

    if (SOC_IS_ROBO5324(unit)) {
        if (port <16) {
            reg_index = MCAST_SUP0r;
        } else {
            reg_index = MCAST_SUP1r;
        }
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, reg_index, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, reg_index);
        
        switch (port) {
            case 0:
                bcast_fld_index = EN_P0_BCASTf;
                mcast_fld_index = EN_P0_MCASTf;
                dlf_fld_index = EN_P0_DLFf;
                break;
            case 1:
                bcast_fld_index = EN_P1_BCASTf;
                mcast_fld_index = EN_P1_MCASTf;
                dlf_fld_index = EN_P1_DLFf;
                break;
            case 2:
                bcast_fld_index = EN_P2_BCASTf;
                mcast_fld_index = EN_P2_MCASTf;
                dlf_fld_index = EN_P2_DLFf;
                break;
            case 3:
                bcast_fld_index = EN_P3_BCASTf;
                mcast_fld_index = EN_P3_MCASTf;
                dlf_fld_index = EN_P3_DLFf;
                break;
            case 4:
                bcast_fld_index = EN_P4_BCASTf;
                mcast_fld_index = EN_P4_MCASTf;
                dlf_fld_index = EN_P4_DLFf;
                break;
            case 5:
                bcast_fld_index = EN_P5_BCASTf;
                mcast_fld_index = EN_P5_MCASTf;
                dlf_fld_index = EN_P5_DLFf;
                break;
            case 6:
                bcast_fld_index = EN_P6_BCASTf;
                mcast_fld_index = EN_P6_MCASTf;
                dlf_fld_index = EN_P6_DLFf;
                break;
            case 7:
                bcast_fld_index = EN_P7_BCASTf;
                mcast_fld_index = EN_P7_MCASTf;
                dlf_fld_index = EN_P7_DLFf;
                break;
            case 8:
                bcast_fld_index = EN_P8_BCASTf;
                mcast_fld_index = EN_P8_MCASTf;
                dlf_fld_index = EN_P8_DLFf;
                break;
            case 9:
                bcast_fld_index = EN_P9_BCASTf;
                mcast_fld_index = EN_P9_MCASTf;
                dlf_fld_index = EN_P9_DLFf;
                break;
            case 10:
                bcast_fld_index = EN_P10_BCASTf;
                mcast_fld_index = EN_P10_MCASTf;
                dlf_fld_index = EN_P10_DLFf;
                break;
            case 11:
                bcast_fld_index = EN_P11_BCASTf;
                mcast_fld_index = EN_P11_MCASTf;
                dlf_fld_index = EN_P11_DLFf;
                break;
            case 12:
                bcast_fld_index = EN_P12_BCASTf;
                mcast_fld_index = EN_P12_MCASTf;
                dlf_fld_index = EN_P12_DLFf;
                break;
            case 13:
                bcast_fld_index = EN_P13_BCASTf;
                mcast_fld_index = EN_P13_MCASTf;
                dlf_fld_index = EN_P13_DLFf;
                break;
            case 14:
                bcast_fld_index = EN_P14_BCASTf;
                mcast_fld_index = EN_P14_MCASTf;
                dlf_fld_index = EN_P14_DLFf;
                break;
            case 15:
                bcast_fld_index = EN_P15_BCASTf;
                mcast_fld_index = EN_P15_MCASTf;
                dlf_fld_index = EN_P15_DLFf;
                break;
            case 16:
                bcast_fld_index = EN_P16_BCASTf;
                mcast_fld_index = EN_P16_MCASTf;
                dlf_fld_index = EN_P16_DLFf;
                break;
            case 17:
                bcast_fld_index = EN_P17_BCASTf;
                mcast_fld_index = EN_P17_MCASTf;
                dlf_fld_index = EN_P17_DLFf;
                break;
            case 18:
                bcast_fld_index = EN_P18_BCASTf;
                mcast_fld_index = EN_P18_MCASTf;
                dlf_fld_index = EN_P18_DLFf;
                break;
            case 19:
                bcast_fld_index = EN_P19_BCASTf;
                mcast_fld_index = EN_P19_MCASTf;
                dlf_fld_index = EN_P19_DLFf;
                break;
            case 20:
                bcast_fld_index = EN_P20_BCASTf;
                mcast_fld_index = EN_P20_MCASTf;
                dlf_fld_index = EN_P20_DLFf;
                break;
            case 21:
                bcast_fld_index = EN_P21_BCASTf;
                mcast_fld_index = EN_P21_MCASTf;
                dlf_fld_index = EN_P21_DLFf;
                break;
            case 22:
                bcast_fld_index = EN_P22_BCASTf;
                mcast_fld_index = EN_P22_MCASTf;
                dlf_fld_index = EN_P22_DLFf;
                break;
            case 23:
                bcast_fld_index = EN_P23_BCASTf;
                mcast_fld_index = EN_P23_MCASTf;
                dlf_fld_index = EN_P23_DLFf;
                break;
            case 24:
                bcast_fld_index = EN_P24_BCASTf;
                mcast_fld_index = EN_P24_MCASTf;
                dlf_fld_index = EN_P24_DLFf;
                break;
            case 25:
                bcast_fld_index = EN_P25_BCASTf;
                mcast_fld_index = EN_P25_MCASTf;
                dlf_fld_index = EN_P25_DLFf;
                break;
            case 26:
                bcast_fld_index = EN_P26_BCASTf;
                mcast_fld_index = EN_P26_MCASTf;
                dlf_fld_index = EN_P26_DLFf;
                break;
            default:
                rv =SOC_E_PARAM;
        }
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, (uint32 *)&reg_value64, reg_len)) < 0) {
            return rv;
        }
        /* broadcast type */
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, reg_index, (uint32 *)&reg_value64, bcast_fld_index, &temp));
        if (temp) {
            *type |= DRV_STORM_CONTROL_BCAST;
        }
        /* mulitcast type */
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, reg_index, (uint32 *)&reg_value64, mcast_fld_index, &temp));
        if (temp) {
            *type |= DRV_STORM_CONTROL_MCAST;
        }
        /* dlf type */
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, reg_index, (uint32 *)&reg_value64, dlf_fld_index, &temp));
        if (temp) {
            *type |= DRV_STORM_CONTROL_DLF;
        }
        return rv;
    }

    if(SOC_IS_ROBO5348(unit)||SOC_IS_ROBO5347(unit)){
        /* Enable drop_enable bit of bucket 1 */
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, PORT_IRC_CONr, port, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, PORT_IRC_CONr);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, (uint32 *)&reg_value64, reg_len)) < 0) {
            return rv;
        }
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, PORT_IRC_CONr, (uint32 *)&reg_value64, PKT_MSK1f, &temp));
        if (temp & BCM5348_SUPPRESSION_BROADCAST_MASK) {
            *type |= DRV_STORM_CONTROL_BCAST;
        }
    
        if (temp & BCM5348_SUPPRESSION_MULTICAST_MASK) {
            *type |= DRV_STORM_CONTROL_MCAST;
        }
    
        if (temp & BCM5348_SUPPRESSION_DLF_MASK){
            *type |= DRV_STORM_CONTROL_DLF;
        }

        if (temp & BCM5348_SUPPRESSION_UNKNOW_SA_MASK) {
            *type |= DRV_STORM_CONTROL_SALF;
        }

        if (temp & BCM5348_SUPPRESSION_BPDU_MASK) {
            *type |= DRV_STORM_CONTROL_RSV_MCAST;
        }

        return rv;
    }
    return rv;
}

int 
_drv_bcm5348_storm_control_enable_set(int unit, uint32 port, uint8 enable)
{
    uint32 reg_addr, reg_len, temp;
    uint64 reg_value64;
    int rv = SOC_E_NONE;

    /* Enable drop_enable bit of bucket 1 */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, PORT_IRC_CONr, port, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, PORT_IRC_CONr);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, (uint32 *)&reg_value64, reg_len)) < 0) {
        return rv;
    }
    temp = 1;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, PORT_IRC_CONr, (uint32 *)&reg_value64, DROP_EN1f, &temp));

    /* Enable Ingress rate control bit */
    temp = enable;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, PORT_IRC_CONr, (uint32 *)&reg_value64, ING_RC_ENf, &temp));
   
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, (uint32 *)&reg_value64,reg_len));

    return rv;
}

/*
 *  Function : drv_storm_control_enable_set
 *
 *  Purpose :
 *      Set the status for global storm control of selected port..
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      port   :   port number.
 *      enable   :   status of the storm control. (global)
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      
 *
 */
int 
drv_storm_control_enable_set(int unit, uint32 port, uint8 enable)
{
    uint32  reg_addr, reg_len, temp;
    uint32  reg_value;
    uint64 reg_value64;
    uint32  reg_index, fld_index = 0;
    int     rv= SOC_E_NONE;
    soc_pbmp_t pbmp;
    int     no_que = 0;

    soc_cm_debug(DK_PORT, 
    "drv_storm_control_enable_set: unit = %d, port= %d, %sable\n",
        unit, port, enable ? "en" : "dis");

    if (SOC_IS_ROBO5348(unit)||SOC_IS_ROBO5347(unit)) {
        rv = _drv_bcm5348_storm_control_enable_set(unit, port, enable);
        return rv;
    }

    /* Enable drop_enable bit of bucket 1 */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, COMM_IRC_CONr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, COMM_IRC_CONr);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }
    temp = 1;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, COMM_IRC_CONr, &reg_value, DROP_EN1f, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value,reg_len));

    /* Enable Ingress rate control bit */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, PORT_IRC_CONr, port, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, PORT_IRC_CONr);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    if (SOC_IS_ROBODINO(unit)) {
        temp = enable;
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, PORT_IRC_CONr, &reg_value, ING_RC_ENf, &temp));
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            return rv;
        }
        return rv;
    }

    /* bcm5324 */
    /* If Rate control isn't enabled, 
     * set the rate refresh and bucket size as maxinum. 
     * limit = 1G, bucket size = 130K
     * This is to prevent the packets to block by rate control 
     */
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
        (unit, PORT_IRC_CONr, &reg_value, ING_RC_ENf, &temp));
    if (temp == 0) {
        SOC_PBMP_CLEAR(pbmp);
        SOC_PBMP_PORT_ADD(pbmp, port);
        DRV_RATE_SET
            (unit, pbmp, no_que, DRV_RATE_CONTROL_DIRECTION_INGRESSS, 
                0, 1000 * 1000, 130);
    }


    /* Enable Suppression bit of Storm Control */ 
    if (port <16) {
        reg_index = MCAST_SUP0r;
    } else {
        reg_index = MCAST_SUP1r;
    }
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, reg_index, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, reg_index);

    switch (port) {
        case 0:
            fld_index = EN_P0_SUPf;
            break;
        case 1:
            fld_index = EN_P1_SUPf;
            break;
        case 2:
            fld_index = EN_P2_SUPf;
            break;
        case 3:
            fld_index = EN_P3_SUPf;
            break;
        case 4:
            fld_index = EN_P4_SUPf;
            break;
        case 5:
            fld_index = EN_P5_SUPf;
            break;
        case 6:
            fld_index = EN_P6_SUPf;
            break;
        case 7:
            fld_index = EN_P7_SUPf;
            break;
        case 8:
            fld_index = EN_P8_SUPf;
            break;
        case 9:
            fld_index = EN_P9_SUPf;
            break;
        case 10:
            fld_index = EN_P10_SUPf;
            break;
        case 11:
            fld_index = EN_P11_SUPf;
            break;
        case 12:
            fld_index = EN_P12_SUPf;
            break;
        case 13:
            fld_index = EN_P13_SUPf;
            break;
        case 14:
            fld_index = EN_P14_SUPf;
            break;
        case 15:
            fld_index = EN_P15_SUPf;
            break;
        case 16:
            fld_index = EN_P16_SUPf;
            break;
        case 17:
            fld_index = EN_P17_SUPf;
            break;
        case 18:
            fld_index = EN_P18_SUPf;
            break;
        case 19:
            fld_index = EN_P19_SUPf;
            break;
        case 20:
            fld_index = EN_P20_SUPf;
            break;
        case 21:
            fld_index = EN_P21_SUPf;
            break;
        case 22:
            fld_index = EN_P22_SUPf;
            break;
        case 23:
            fld_index = EN_P23_SUPf;
            break;
        case 24:
            fld_index = EN_P24_SUPf;
            break;
        case 25:
            fld_index = EN_P25_SUPf;
            break;
        case 26:
            fld_index = EN_P26_SUPf;
            break;
        default:
            rv =SOC_E_PARAM;
    }
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, (uint32 *)&reg_value64, reg_len)) < 0) {
        return rv;
    }
    temp = enable;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, reg_index, (uint32 *)&reg_value64, fld_index, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, (uint32 *)&reg_value64,reg_len));
    return rv;
}

/*
 *  Function : drv_storm_control_enable_get
 *
 *  Purpose :
 *      Get the status for global storm control of selected port..
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      port   :   port number.
 *      enable   :   status of the storm control. (global)
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      
 *
 */
int 
drv_storm_control_enable_get(int unit, uint32 port, uint8 *enable)
{
    uint32  reg_addr, reg_len, temp;
    uint64  reg_value64;
    uint32  reg_value;
    uint32  reg_index, fld_index = 0;
    int     rv= SOC_E_NONE;

    if (SOC_IS_ROBO5348(unit)||SOC_IS_ROBO5347(unit)) {
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, PORT_IRC_CONr, port, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, PORT_IRC_CONr);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, (uint32 *)&reg_value64, reg_len)) < 0) {
            return rv;
        }
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, PORT_IRC_CONr, (uint32 *)&reg_value64, ING_RC_ENf, &temp));
        *enable = temp;

        return rv;
    }
    
    /* bcm5396 */
    if (SOC_IS_ROBODINO(unit)) {
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, PORT_IRC_CONr, port, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, PORT_IRC_CONr);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value, reg_len)) < 0) {
            return rv;
        }
         SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
        (unit, PORT_IRC_CONr, &reg_value, ING_RC_ENf, &temp));
         *enable = temp;
         return rv;
    }

    /* bcm5324 */
    if (port <16) {
        reg_index = MCAST_SUP0r;
    } else {
        reg_index = MCAST_SUP1r;
    }
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, reg_index, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, reg_index);

    switch (port) {
        case 0:
            fld_index = EN_P0_SUPf;
            break;
        case 1:
            fld_index = EN_P1_SUPf;
            break;
        case 2:
            fld_index = EN_P2_SUPf;
            break;
        case 3:
            fld_index = EN_P3_SUPf;
            break;
        case 4:
            fld_index = EN_P4_SUPf;
            break;
        case 5:
            fld_index = EN_P5_SUPf;
            break;
        case 6:
            fld_index = EN_P6_SUPf;
            break;
        case 7:
            fld_index = EN_P7_SUPf;
            break;
        case 8:
            fld_index = EN_P8_SUPf;
            break;
        case 9:
            fld_index = EN_P9_SUPf;
            break;
        case 10:
            fld_index = EN_P10_SUPf;
            break;
        case 11:
            fld_index = EN_P11_SUPf;
            break;
        case 12:
            fld_index = EN_P12_SUPf;
            break;
        case 13:
            fld_index = EN_P13_SUPf;
            break;
        case 14:
            fld_index = EN_P14_SUPf;
            break;
        case 15:
            fld_index = EN_P15_SUPf;
            break;
        case 16:
            fld_index = EN_P16_SUPf;
            break;
        case 17:
            fld_index = EN_P17_SUPf;
            break;
        case 18:
            fld_index = EN_P18_SUPf;
            break;
        case 19:
            fld_index = EN_P19_SUPf;
            break;
        case 20:
            fld_index = EN_P20_SUPf;
            break;
        case 21:
            fld_index = EN_P21_SUPf;
            break;
        case 22:
            fld_index = EN_P22_SUPf;
            break;
        case 23:
            fld_index = EN_P23_SUPf;
            break;
        case 24:
            fld_index = EN_P24_SUPf;
            break;
        case 25:
            fld_index = EN_P25_SUPf;
            break;
        case 26:
            fld_index = EN_P26_SUPf;
            break;
        default:
            rv =SOC_E_PARAM;
    }
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, (uint32 *)&reg_value64, reg_len)) < 0) {
        return rv;
    }
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
        (unit, reg_index, (uint32 *)&reg_value64, fld_index, &temp));
    if (temp == 1) {
        *enable = TRUE;
    } else {
        *enable = FALSE;
    }
    soc_cm_debug(DK_PORT, 
    "drv_storm_control_enable_get: unit = %d, port= %d, %sable\n",
        unit, port, *enable ? "en" : "dis");
    return rv;
}

/*
 *  Function : drv_storm_control_set
 *
 *  Purpose :
 *      Set the types and limit value for storm control of selected port..
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      bmp   :   port bitmap.
 *      type   :   types of strom control.
 *      limit   :   limit value of storm control. (Kbits)
 *      burst_size : max burst size. (if size = 0, do nothing)
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      
 *
 */
int 
drv_storm_control_set(int unit, soc_pbmp_t bmp, uint32 type, 
    uint32 limit, uint32 burst_size)
{
    uint32      reg_addr, reg_value, reg_len, temp;
    int         rv= SOC_E_NONE;
    uint32      port;
    uint32      disable_type = 0, burst_kbyte = 0;
    uint64      reg_value64;

    soc_cm_debug(DK_PORT, 
    "drv_storm_control_set: unit = %d, bmp= %x, type = 0x%x, limit = %dK\n",
        unit, SOC_PBMP_WORD_GET(bmp, 0), type, limit);
    PBMP_ITER(bmp, port) {
        /* 
        * Check maximum supported rate limit of FE and GE ports, 
        * respectively.
        */        
        if (SOC_PBMP_MEMBER(PBMP_GE_ALL(unit), port)) {
            if (limit > 1000000) {
                return SOC_E_PARAM;
            }
        } else {
            if (limit > 100000) {
                return SOC_E_PARAM;
            }
        }
        if (limit == 0) { /* Disable storm suppression type */
            SOC_IF_ERROR_RETURN(
                _drv_storm_control_type_enable_set(unit, port, type, FALSE));
            /* need diable ingress rate control ? */
        } else {
            disable_type = DRV_STORM_CONTROL_BCAST | 
                            DRV_STORM_CONTROL_MCAST |
                            DRV_STORM_CONTROL_DLF;
            disable_type ^= type;
            /* set storm suppression type */
            SOC_IF_ERROR_RETURN(
                _drv_storm_control_type_enable_set(unit, port, type, TRUE));
            if (disable_type){
                SOC_IF_ERROR_RETURN(
                    _drv_storm_control_type_enable_set(
                                    unit, port, 
                                    disable_type, FALSE));
            }
            /* set bucket 1 refresh count */
            if (SOC_IS_ROBO5348(unit)||SOC_IS_ROBO5347(unit)) {
                reg_addr = (DRV_SERVICES(unit)->reg_addr)
                    (unit, PORT_IRC_CONr, port, 0);
                reg_len = (DRV_SERVICES(unit)->reg_length_get)
                    (unit, PORT_IRC_CONr);
                if ((rv = (DRV_SERVICES(unit)->reg_read)
                    (unit, reg_addr, (uint32 *)&reg_value64, reg_len)) < 0) {
                    return rv;
                }
            } else {
                reg_addr = (DRV_SERVICES(unit)->reg_addr)
                    (unit, PORT_IRC_CONr, port, 0);
                reg_len = (DRV_SERVICES(unit)->reg_length_get)
                    (unit, PORT_IRC_CONr);
                if ((rv = (DRV_SERVICES(unit)->reg_read)
                    (unit, reg_addr, &reg_value, reg_len)) < 0) {
                    return rv;
                }
            }
            /* burst size */
            if (burst_size) {
                burst_kbyte = (burst_size / 8);
                if (SOC_IS_ROBO5324(unit) || SOC_IS_ROBO5348(unit) || 
                    SOC_IS_ROBO5347(unit)) {
                    if (burst_size > (130 * 8)) { /* 130 KB */
                        return SOC_E_PARAM;
                    }
                    /* burst size */
                    if (burst_kbyte <= 6) { /* 6KB */
                        temp = 0;
                    } else if (burst_kbyte <= 10) { /* 10KB */
                        temp = 1;
                    } else if (burst_kbyte <= 18) { /* 18KB */
                        temp = 2;
                    } else if (burst_kbyte <= 34) { /* 34KB */
                        temp = 3;
                    } else if (burst_kbyte <= 66) { /* 66KB */
                        temp = 4;
                    } else { /* else burst_kbyte <= 130 for 130KB */
                        temp = 5;
                    }
                } else if (SOC_IS_ROBODINO(unit)){
                    if (burst_size > (500 * 8)) { /* 500 KB */
                        return SOC_E_PARAM;
                    }
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
                    } else { /* else burst_kbyte <= 500 for 500KB */
                        temp = 7;
                    }
                }
                if (SOC_IS_ROBO5348(unit)||SOC_IS_ROBO5347(unit)) {
                    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                    (unit, PORT_IRC_CONr, (uint32 *)&reg_value64, 
                    BUCKET_SIZE1f, &temp));
                } else {
                    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                    (unit, PORT_IRC_CONr, &reg_value, 
                    BUCKET_SIZE1f, &temp));
                }
            }

            /* refresh count  (fixed type)*/
            if (limit <= 1792) { /* 64KB ~ 1.792MB */
                temp = ((limit-1) / 64) +1;
            } else if (limit <= 100000){ /* 2MB ~ 100MB */
                temp = (limit /1000 ) + 27;
            } else { 
                /* limit ranges from 102401 to 1048576 for 104MB ~ 1000MB */
                temp = (limit /8000) + 115;
            } 
            if (SOC_IS_ROBO5348(unit)||SOC_IS_ROBO5347(unit)) {
                SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                    (unit, PORT_IRC_CONr, (uint32 *)&reg_value64, REF_CNT1f, &temp));
                SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
                    (unit, reg_addr, (uint32 *)&reg_value64,reg_len));
            } else {
                SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                    (unit, PORT_IRC_CONr, &reg_value, REF_CNT1f, &temp));
                SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
                    (unit, reg_addr, &reg_value,reg_len));
            }
        }
    }
    return rv;
}

/*
 *  Function : drv_storm_control_get
 *
 *  Purpose :
 *      Get the types and limit value for storm control of selected port..
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      bmp   :   port bitmap.
 *      type   :   types of strom control.
 *      limit   :   limit value of storm control. (Kbits)
 *      burst_size : max burst size.
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      
 *
 */
int 
drv_storm_control_get(int unit, uint32 port, uint32 *type, 
    uint32 *limit, uint32 *burst_size)
{
    uint32      reg_addr, reg_value, reg_len, temp;
    int         rv= SOC_E_NONE;
    uint64      reg_value64;

    SOC_IF_ERROR_RETURN(
        _drv_storm_control_type_enable_get(unit, port, type));
    if (!(*type)) {
        *limit = 0;
    } else {
        if (SOC_IS_ROBO5348(unit)||SOC_IS_ROBO5347(unit)) {
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, PORT_IRC_CONr, port, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, PORT_IRC_CONr);
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, (uint32 *)&reg_value64, reg_len)) < 0) {
                return rv;
            }
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                (unit, PORT_IRC_CONr, (uint32 *)&reg_value64, REF_CNT1f, &temp));
        } else {
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, PORT_IRC_CONr, port, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, PORT_IRC_CONr);
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
                return rv;
            }
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                (unit, PORT_IRC_CONr, &reg_value, REF_CNT1f, &temp));
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
        /* Burst size */
        if (SOC_IS_ROBO5348(unit)||SOC_IS_ROBO5347(unit)) {
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                (unit, PORT_IRC_CONr, (uint32 *)&reg_value64, 
                BUCKET_SIZE1f, &temp));
        } else {
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                (unit, PORT_IRC_CONr, &reg_value, 
                BUCKET_SIZE1f, &temp));
        }
        if (SOC_IS_ROBO5324(unit) || SOC_IS_ROBO5348(unit) ||
            SOC_IS_ROBO5347(unit)) {
            switch (temp) {
            case 0:
                *burst_size = 6 * 8; /* 6KB = 48Kbits */
                break;
            case 1:
                *burst_size = 10 * 8; /* 10KB */
                break;
            case 2:
                *burst_size = 18 * 8; /* 18KB */
                break;
            case 3:
                *burst_size = 34 * 8; /* 34KB */
                break;
            case 4:
                *burst_size = 66 * 8; /* 66KB */
                break;
            case 5:
                *burst_size = 130 * 8; /* 130KB */
                break;
            default:
                return SOC_E_INTERNAL;
            }
        }
        if (SOC_IS_ROBODINO(unit)) {
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
    }
    soc_cm_debug(DK_PORT, 
    "drv_storm_control_get: unit = %d, port= %d, type = 0x%x, limit = %dK\n",
        unit, port, *type, *limit);

    return rv;
}
