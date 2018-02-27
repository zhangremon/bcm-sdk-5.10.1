/*
 * $Id: brd_sbx.h 1.14 Broadcom SDK $
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
 * SBX Reference Boards
 */

#ifndef __BRD_SBX_H__
#define __BRD_SBX_H__

/*
 * Board error codes
 */
typedef enum {
    BOARD_E_NONE,
    BOARD_E_PARAM,
    BOARD_E_FAIL
} brd_error_t;

typedef enum brd_sbx_type_e {
    BOARD_TYPE_METROCORE = 1,     /* Metrocore Linecard */
    BOARD_TYPE_METROCORE_FABRIC,  /* Metrocore Fabric card */
    BOARD_TYPE_LCMODEL,           /* BCMSIM */
    BOARD_TYPE_POLARIS_LC,        /* FE2K LC with Polaris LCMs */
    BOARD_TYPE_POLARIS_FC,        /* Polaris fabric card */
    BOARD_TYPE_FE2KXT_QE2K_POLARIS_LC, /* FE2KXT,QE2K  LC with Polaris LCM */
    BOARD_TYPE_FE2KXT_4X10G_QE2K_POLARIS_LC, /* FE2KXT 4x10G */
    BOARD_TYPE_SIRIUS_SIM = 8,    /* Sirius - single chip SIM */
    BOARD_TYPE_QE2K_BSCRN_LC = 9, /* QE2K - Dual Chip Benchscreen Board. */
    BOARD_TYPE_SIRIUS_IPASS = BOARD_TYPE_SIRIUS_SIM,  /* Sirius Ipass board */
    BOARD_TYPE_POLARIS_IPASS = 11, /* Polaris Ipass board */
    /* Must be the last */
    BOARD_TYPE_MAX
} brd_sbx_type_t;

/*
 *  FPGA Offsets
 */

#define FPGA_BASE  (0x10000)
#define FPGA_PL_BASE  (0x40000)
#define FPGA_MASTER_REG_OFFSET  0x14
#define FPGA_MASTER_MODE_BIT    0x10
#define FPGA_SCI_ROUTING_OFFSET 0x19
#define FPGA_SCI_TO_LCM         0xA0
#define FPGA_SCI_TO_BP          0x50
#define FPGA_FE2K_DLL_ENABLE_OFFSET 0x1a
#define FPGA_FE2K_DLL_ENABLE    0x03
#define FPGA_SLOT_ID_OFFSET     0x11
#define FPGA_ID_OFFSET 0x0
#define FPGA_REVISION_OFFSET  0x1
#define FPGA_BOARD_ID_OFFSET    0x2
#define FPGA_LC_PL_BOARD	0x04
#define FPGA_FC_PL_BOARD	0x05
#define FPGA_BOARD_REV_OFFSET 0x3
#define FPGA_LC_PL_INT_OFFSET         0xa 
#define FPGA_LC_PL_INT                0x08 /* bit 19 of 32 bit reg starting at 0x8 */
#define FPGA_LC_PL_PCI_INT                0xf0 
#define FPGA_LC_PL_SLOT_ID_OFFSET	0x18
#define FPGA_LC_PL_BIB_CHASIS		4
#define FPGA_FC_PL_INT_OFFSET         0x8
#define FPGA_FC_PL_INT                0x1  /* bit 0 of 32 bit reg starting at 0x8 */


extern int
board_preinit(brd_sbx_type_t type);

#endif /* !__BRD_SBX_H__ */
