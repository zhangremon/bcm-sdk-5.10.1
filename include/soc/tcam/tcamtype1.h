/*
 * $Id: tcamtype1.h 1.22 Broadcom SDK $
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
 * File:      tcamtype1.h
 */

/* microcode location */
#define TYPE1_UCODE_ADDR                 0x06f00000

/* Database register accessing: address[19:18] is device ID */
#define TYPE1_DBREG_BASE(dev)            (((dev) & 0x03) << 18)
/* Indirect mapped address accessing: address[29:28] is device ID */
#define TYPE1_IMA_BASE(dev)              (((dev) & 0x03) << 28)

/*
 * register addresses
 */

/* Database register: Database Configuration Register */
#define TYPE1_DBREG_DBCFG_ADDR(dev)      (0xa0 + TYPE1_DBREG_BASE(dev))

/* Database register: Device ID Register */
#define TYPE1_DBREG_DEVID_ADDR(dev)      (0x08 + TYPE1_DBREG_BASE(dev))

/* Database register: Result Register */
#define TYPE1_DBREG_RESULT_ADDR(dev,i)   (0x10 + (i) + TYPE1_DBREG_BASE(dev))

/* Database register: Logical Table Register */
#define TYPE1_DBREG_LTBL_ADDR(dev,i)     (0x40 + (i) + TYPE1_DBREG_BASE(dev))

/* Database register: Block [0-n] Mask Register [0-7] */
#define TYPE1_DBREG_BMASK_ADDR(dev,blk,mask) (0x820 + (blk) * 0x100 + \
                                              ((blk) >= 8 ? 0x800 : 0) + \
                                              (mask) * 0x10 + \
                                              TYPE1_DBREG_BASE(dev))

/* Database register: Block Configuration Register */
#define TYPE1_DBREG_BCFG_ADDR(dev,blk)   (0x80 + (blk) + TYPE1_DBREG_BASE(dev))

/* Indirectly mapped register: NPU IO Parity and FIFO Control Register */
#define TYPE1_IMREG_QDR_FIFO_CTL_ADDR(dev) (0x0f000008 + TYPE1_IMA_BASE(dev))

/* Indirectly mapped register: Status Register */
#define TYPE1_IMREG_QDR_STATUS_ADDR(dev)   (0x0f000048 + TYPE1_IMA_BASE(dev))

/* Indirectly mapped register: Device Configuration Register */
#define TYPE1_IMREG_DEV_CFG_ADDR(dev)      (0x0f000000 + TYPE1_IMA_BASE(dev))

/* Indirectly mapped register: Clock Control Register */
#define TYPE1_IMREG_CLK_CTL_ADDR(dev)      (0x0f000010 + TYPE1_IMA_BASE(dev))

/* Indirectly mapped register: QDR IO Termination control Register */
#define TYPE1_IMREG_IO_TERM_CTL_ADDR(dev)  (0x0f000038 + TYPE1_IMA_BASE(dev))

/*
 * register bit fields and values
 */
#define MAKE_FIELD(field, val)  (((val) & field## _mask) << field## _oft)

/* Database register: Database Configuration Register */
#define TYPE1_DBREG_DBCFG_MSB_XYMODE_oft        22
#define TYPE1_DBREG_DBCFG_MSB_XYMODE_mask       1
#define TYPE1_ST3_DBREG_DBCFG_LSB_FG_PS_oft     3
#define TYPE1_ST3_DBREG_DBCFG_LSB_FG_PS_mask    0x01
#define TYPE1_ST3_DBREG_DBCFG_LSB_MIN_REV_oft   8
#define TYPE1_ST3_DBREG_DBCFG_LSB_MIN_REV_mask  0xff
#define TYPE1_ST3_DBREG_DBCFG_LSB_MAJ_REV_oft   16
#define TYPE1_ST3_DBREG_DBCFG_LSB_MAJ_REV_mask  0xff
#define TYPE1_ST3_DBREG_DBCFG_LSB_SIZE_oft      24
#define TYPE1_ST3_DBREG_DBCFG_LSB_SIZE_mask     0xff
#define TYPE1_ST3_DBREG_DBCFG_MSB_SIZE_oft      0
#define TYPE1_ST3_DBREG_DBCFG_MSB_SIZE_mask     0xff
#define TYPE1_ST3_DBREG_DBCFG_MSB_FAMILY_oft    20
#define TYPE1_ST3_DBREG_DBCFG_MSB_FAMILY_mask   0x03
#define TYPE1_ST3_DBREG_DBCFG_TOP8_CMP_ANY_oft  1
#define TYPE1_ST3_DBREG_DBCFG_TOP8_CMP_ANY_mask 0x01
#define TYPE1_ST3_DBREG_DBCFG_TOP8_MAP_KEY_oft  3
#define TYPE1_ST3_DBREG_DBCFG_TOP8_MAP_KEY_mask 0x01

/* Database register: Device ID Register */
#define TYPE1_DBREG_DEVID_ID_oft              0
#define TYPE1_DBREG_DEVID_ID_mask             0x03

/* Database register: Block Configuration Register */
#define TYPE1_DBREG_BCFG_WIDTH_oft            0
#define TYPE1_DBREG_BCFG_WIDTH_mask           0x0f
#define TYPE1_ST1_DBREG_BCFG_LTID_oft         4    /* used by ST2 as well */
#define TYPE1_ST1_DBREG_BCFG_LTID_mask        0x0f
#define TYPE1_ST3_DBREG_BCFG_LTID_oft         4
#define TYPE1_ST3_DBREG_BCFG_LTID_mask        0x1f
#define TYPE1_ST3_DBREG_BCFG_LPWR_oft         9
#define TYPE1_ST3_DBREG_BCFG_LPWR_mask        0x01

#define TYPE1_DBREG_BCFG_WIDTH_72             0
#define TYPE1_DBREG_BCFG_WIDTH_144            1
#define TYPE1_DBREG_BCFG_WIDTH_288            2
#define TYPE1_DBREG_BCFG_WIDTH_576            3

/* Database register: Logical Table Register */
#define TYPE1_DBREG_LTBL_BMASK_oft            0
#define TYPE1_DBREG_LTBL_BMASK_mask           0x07
#define TYPE1_ST1_DBREG_LTBL_LTID_oft         4
#define TYPE1_ST1_DBREG_LTBL_LTID_mask        0x07
#define TYPE1_ST2_DBREG_LTBL_LTID_oft         9
#define TYPE1_ST2_DBREG_LTBL_LTID_mask        0x0f
#define TYPE1_ST3_DBREG_LTBL_BMASK0_oft       0
#define TYPE1_ST3_DBREG_LTBL_BMASK0_mask      0x0f
#define TYPE1_ST3_DBREG_LTBL_LTID0_oft        9
#define TYPE1_ST3_DBREG_LTBL_LTID0_mask       0x1f
#define TYPE1_ST3_DBREG_LTBL_BMASK1_oft       16
#define TYPE1_ST3_DBREG_LTBL_BMASK1_mask      0x0f
#define TYPE1_ST3_DBREG_LTBL_LTID1_oft        25
#define TYPE1_ST3_DBREG_LTBL_LTID1_mask       0x1f
#define TYPE1_ST3_DBREG_LTBL_PCOMP_oft        0     /* bit 32 */
#define TYPE1_ST3_DBREG_LTBL_PCOMP_mask       0x01
#define TYPE1_ST3_DBREG_LTBL_KEY_oft          1     /* bit 33 */
#define TYPE1_ST3_DBREG_LTBL_KEY_mask         0x07

/* QDR interface register: Parity and FIFO Control Register */
#define TYPE1_IMREG_QDR_FIFO_LSB_0_AF_OFF_oft   0
#define TYPE1_IMREG_QDR_FIFO_LSB_0_AF_OFF_mask  0x0f
#define TYPE1_IMREG_QDR_FIFO_LSB_0_AF_ON_oft    4
#define TYPE1_IMREG_QDR_FIFO_LSB_0_AF_ON_mask   0x0f
#define TYPE1_IMREG_QDR_FIFO_LSB_1_AF_OFF_oft   16
#define TYPE1_IMREG_QDR_FIFO_LSB_1_AF_OFF_mask  0x0f
#define TYPE1_IMREG_QDR_FIFO_LSB_1_AF_ON_oft    20
#define TYPE1_IMREG_QDR_FIFO_LSB_1_AF_ON_mask   0x0f
#define TYPE1_IMREG_QDR_FIFO_MSB_RSVD_oft       0
#define TYPE1_IMREG_QDR_FIFO_MSB_RSVD_mask      0x7fff
#define TYPE1_IMREG_QDR_FIFO_MSB_MODE_oft       17
#define TYPE1_IMREG_QDR_FIFO_MSB_MODE_mask      0x01
#define TYPE1_IMREG_QDR_FIFO_MSB_FCI_oft        18
#define TYPE1_IMREG_QDR_FIFO_MSB_FCI_mask       0x01

#define TYPE1_IMREG_QDR_FIFO_MSB_RSVD_VALUE     0x0520

/* QDR interface register: QDR Status Register */
#define TYPE1_IMREG_QDR_STATUS_LSB_5000         0x00005001
#define TYPE1_IMREG_QDR_STATUS_LSB_3280         0x00015001

/* I/O control register: QDR IO Termination control Register */
#define TYPE1_IMREG_IO_TERM_CTL_INIT_LSB        0x000f000f
#define TYPE1_IMREG_IO_TERM_CTL_INIT_MSB        0x04030000

/* Device control register: Device Configuration Register */
#define TYPE1_IMREG_DEV_CFG_LSB_RSVD1_oft       0
#define TYPE1_IMREG_DEV_CFG_LSB_RSVD1_mask      0xff
#define TYPE1_IMREG_DEV_CFG_LSB_UCODE_ENA_oft   12
#define TYPE1_IMREG_DEV_CFG_LSB_UCODE_ENA_mask  0x01
#define TYPE1_IMREG_DEV_CFG_LSB_RSVD2_oft       13
#define TYPE1_IMREG_DEV_CFG_LSB_RSVD2_mask      0x3ff
#define TYPE1_IMREG_DEV_CFG_LSB_NPU1_PSE_oft    30
#define TYPE1_IMREG_DEV_CFG_LSB_NPU1_PSE_mask   0x01
#define TYPE1_IMREG_DEV_CFG_MSB_SW_RST_oft      0
#define TYPE1_IMREG_DEV_CFG_MSB_SW_RST_mask     0xffff

#define TYPE1_ST1_IMREG_DEV_CFG_LSB_RSVD1_VALUE 0x64
#define TYPE1_ST2_IMREG_DEV_CFG_LSB_RSVD1_VALUE 0xa4
#define TYPE1_ST2_IMREG_DEV_CFG_LSB_RSVD2_VALUE 1
#define TYPE1_IMREG_DEV_CFG_MSB_SW_RST_ON       0x0100

/* Clock control register: Clock Control Register */
#define TYPE1_IMREG_CLK_CTL_LSB_SYSTEM_SRC_oft  16
#define TYPE1_IMREG_CLK_CTL_LSB_SYSTEM_SRC_mask 0x07
#define TYPE1_IMREG_CLK_CTL_MSB_SRAM_SRC_oft    16
#define TYPE1_IMREG_CLK_CTL_MSB_SRAM_SRC_mask   0x07

#define TYPE1_IMREG_CLK_CTL_SRC_HCLK            0
#define TYPE1_IMREG_CLK_CTL_SRC_XCLK            1
#define TYPE1_IMREG_CLK_CTL_SRC_N0_K            2
#define TYPE1_IMREG_CLK_CTL_SRC_N0_K2           3
#define TYPE1_IMREG_CLK_CTL_SRC_N1_K            4
#define TYPE1_IMREG_CLK_CTL_SRC_N1_K2           5

/*
 * QDR address bus format (for subtype 1 and 2)
 * +-------+-------+-------+-----+
 * | 19:18 | 17:12 | 11:5  | 4:0 |
 * | IPCMD |  IP   | CNTXT | DW  |
 * +-------+-------+-------+-----+
 */
#define TYPE1_QDR_ADDR_CNTXT_oft              5
#define TYPE1_QDR_ADDR_CNTXT_mask             0x7f
#define TYPE1_QDR_ADDR_IP_oft                 12
#define TYPE1_QDR_ADDR_IP_mask                0x3f
#define TYPE1_QDR_ADDR_IPCMD_oft              18
#define TYPE1_QDR_ADDR_IPCMD_mask             0x03

/* Instruction pointer command (IPCMD) (for subtype 1 and 2) */
#define TYPE1_IPCMD_READ_RESULT               0
#define TYPE1_IPCMD_WRITE_IB                  1
#define TYPE1_IPCMD_EXEC_SECONDARY            2
#define TYPE1_IPCMD_EXEC_PRIMARY              3

/* Instruction pointer (IP) (for subtype 1 and 2), customerized by sdk */
#define TYPE1_IP_WRITE_IMA                    0x00
#define TYPE1_IP_READ_IMA                     0x01
#define TYPE1_IP_WRITE_DB_D72                 0x02
#define TYPE1_IP_WRITE_DB_D144                0x03
#define TYPE1_IP_WRITE_DB_M72                 0x04
#define TYPE1_IP_WRITE_DB_M144                0x05
#define TYPE1_IP_READ_DB_D72                  0x06
#define TYPE1_IP_READ_DB_M72                  0x07
#define TYPE1_IP_WRITE_DB_BMR                 0x08
#define TYPE1_IP_WRITE_DB_REG                 0x09
#define TYPE1_IP_READ_DB_REG                  0x0a
#define TYPE1_IP_SET_VALID                    0x0b
#define TYPE1_IP_CLEAR_VALID                  0x0c
#define TYPE1_IP_SEARCH_LPM                   0x0d
#define TYPE1_IP_SEARCH_EPM                   0x0e /* exact prefix match */
#define TYPE1_IP_SEARCH_ACL                   0x0f
#define TYPE1_IP_SEARCH_LPM_H1                0x10
#define TYPE1_IP_SEARCH_EPM_H1                0x11 /* exact prefix match */
#define TYPE1_IP_SEARCH_ACL_H1                0x12
#define TYPE1_IP_NOP                          0x13

/*
 * Opcode (for subtype 3)
 */
#define TYPE1_OPCODE_NOP               0x00
#define TYPE1_OPCODE_ENABLE_NEXT_DEV   0x01
#define TYPE1_OPCODE_SW_RESET          0x02
#define TYPE1_OPCODE_RD_REG            0x40
#define TYPE1_OPCODE_RD_DATA           0x41
#define TYPE1_OPCODE_RD_MASK           0x42
#define TYPE1_OPCODE_CMP_WORD(idx)     (0x80 + (idx))
#define TYPE1_OPCODE_WR_REG            0x93
#define TYPE1_OPCODE_DEL_RECORD        0x94
#define TYPE1_OPCODE_WR_ASDATA         0x05
#define TYPE1_OPCODE_WR_RECORD         0x96
#define TYPE1_OPCODE_SEARCH(ltr)       (0xc0 | (((ltr) & 7) << 3) | \
                                        (((ltr) & 0x08) >> 1) | \
                                        (((ltr) & 0x10) >> 3))


/* the longest instrction format defined by vendor (for subtype 1 and 2) */
#define TYPE1_INST_DATA_SIZE_MAX              10  /* 40 bytes, 10 uint32 */

/* number of entries per block */
#define TYPE1_ST1_ENTRIES_PER_BLK             (32 * 1024)
#define TYPE1_ST2_ENTRIES_PER_BLK             (16 * 1024)
#define TYPE1_ST3_ENTRIES_PER_BLK             (16 * 1024)

/* number of blocks per device */
#define TYPE1_ST1_BLKS_PER_DEV                8
#define TYPE1_ST2_BLKS_PER_DEV                16
#define TYPE1_ST3_BLKS_PER_DEV                16

/*
 * Definitions for configuration, these are customerized by sdk
 */
/* logical table register index for easyrider */
#define TYPE1_ER_LTBL_LPM                     1
#define TYPE1_ER_LTBL_EPM                     2
#define TYPE1_ER_LTBL_ACL                     3
#define TYPE1_ER_LTBL_LPM_H1                  5
#define TYPE1_ER_LTBL_EPM_H1                  6
#define TYPE1_ER_LTBL_ACL_H1                  7

/* logical table ID for easyrider */
#define TYPE1_ER_LTID_LPM                     1   /* LPM in ER, ER_H0 */
#define TYPE1_ER_LTID_ACL                     2   /* ACL in ER, ER_H0 */
#define TYPE1_ER_LTID_LPM_H1                  5   /* LPM in ER_H1 */
#define TYPE1_ER_LTID_ACL_H1                  6   /* ACL in ER_H1 */

/* block mask ID for easyrider */
#define TYPE1_ER_BMASK_EM                     0 /* for exact match */
#define TYPE1_ER_BMASK_LPM                    1 /* for LPM */

/*
 * logical table register index for triumph
 * These number needs to match with ESM hardware (LTR_LTID_usage_060807.xls)
 * even the order is the same as document for easy reading (maybe)
 */
#define TYPE1_TR_LTBL_ACL_NONE_FWD_L2         0
#define TYPE1_TR_LTBL_ACL_NONE_FWD_IP4        1
#define TYPE1_TR_LTBL_ACL_NONE_FWD_IP6U       2
#define TYPE1_TR_LTBL_ACL_NONE_FWD_IP6        3
#define TYPE1_TR_LTBL_ACL_L2_FWD_L2           4
#define TYPE1_TR_LTBL_ACL_IP4_FWD_NONE        7
#define TYPE1_TR_LTBL_ACL_IP4_FWD_IP4         8
#define TYPE1_TR_LTBL_ACL_IP6S_FWD_NONE       5
#define TYPE1_TR_LTBL_ACL_IP6S_FWD_IP6U       9
#define TYPE1_TR_LTBL_ACL_IP6F_FWD_NONE       6
#define TYPE1_TR_LTBL_ACL_IP6F_FWD_IP6U       10
#define TYPE1_TR_LTBL_ACL_IP6F_FWD_IP6        11
#define TYPE1_TR_LTBL_ACL_L2C_FWD_NONE        12
#define TYPE1_TR_LTBL_ACL_IP4C_FWD_NONE       16
#define TYPE1_TR_LTBL_ACL_IP6C_FWD_NONE       18
#define TYPE1_TR_LTBL_ACL_L2IP4_FWD_NONE      21
#define TYPE1_TR_LTBL_ACL_L2IP6_FWD_NONE      22
#define TYPE1_TR_LTBL_ACL_NONE_FWD_IP6U_K4    23 /* for subtype 4 @ 500 Mhz */
#define TYPE1_TR_LTBL_ACL_NONE_FWD_IP6_K4     24 /* for subtype 4 @ 500 Mhz */
#define TYPE1_TR_LTBL_ACL_NONE_FWD_L2_K2      23 /* for subtype 4 @ 350 Mhz */
#define TYPE1_TR_LTBL_ACL_NONE_FWD_IP4_K2     24 /* for subtype 4 @ 350 Mhz */
#define TYPE1_TR_LTBL_FWD_IP4_FWD_L2          25 /* for subtype 4 @ 350 Mhz */
#define TYPE1_TR_LTBL_FWD_IP6U_FWD_L2         26 /* for subtype 4 @ 350 Mhz */
#define TYPE1_TR_LTBL_DEV0_TBL144_DEV0_TBL72  30 /* for software test */
#define TYPE1_TR_LTBL_DEV1_TBL144_DEV1_TBL72  31 /* for software test */

/*
 * logical table ID for triumph
 * These are numbers used by AV, s/w can change these to different order
 */
#define TYPE1_TR_LTID_FWD_L2                  14
#define TYPE1_TR_LTID_FWD_IP4                 2
#define TYPE1_TR_LTID_FWD_IP6U                3   /* ipv6 upper 64-bit */
#define TYPE1_TR_LTID_FWD_IP6                 4
#define TYPE1_TR_LTID_ACL_L2                  5
#define TYPE1_TR_LTID_ACL_IP4                 6
#define TYPE1_TR_LTID_ACL_IP6S                7   /* ipv6 64-bit short mode */
#define TYPE1_TR_LTID_ACL_IP6F                8   /* ipv6 64-bit full mode */
#define TYPE1_TR_LTID_ACL_L2C                 9   /* l2 compact (144-bit) */
#define TYPE1_TR_LTID_ACL_IP4C                10  /* ipv4 compact (144-bit) */
#define TYPE1_TR_LTID_ACL_IP6C                11  /* ipv6 compact (144-bit) */
#define TYPE1_TR_LTID_ACL_L2IP4               12
#define TYPE1_TR_LTID_ACL_L2IP6               13
#define TYPE1_TR_LTID_UNUSED                  15  /* for all unused blocks */
/* following test blocks should not co-exist with the normal blocks */
#define TYPE1_TR_LTID_DEV0_TBL72              2   /* for software test */
#define TYPE1_TR_LTID_DEV0_TBL144             3   /* for software test */
#define TYPE1_TR_LTID_DEV1_TBL72              4   /* for software test */
#define TYPE1_TR_LTID_DEV1_TBL144             5   /* for software test */

/* block mask ID for triumph */
#define TYPE1_TR_BMASK                        0
