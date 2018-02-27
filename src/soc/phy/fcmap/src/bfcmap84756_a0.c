/*
 * $Id: bfcmap84756_a0.c 1.1.2.6 Broadcom SDK $
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

#include <bfcmap.h>
#include <bfcmap_int.h>
#include <bfcmap84756_a0.h>
#include <mmi_cmn.h>


#define BFCMAP84756_MAX_PORTS           4
             
#define BFCMAP84756_MAKE_FP_COOKIE(f,i,p)    (((f) << 8) | (p))


/*
 * Check if conflicting flags are set
 */
#define BFCMAP84756_CHECK_CONFLICTING_FLAGS2(v, f1, f2)    \
            ((((v) & ((f1) | (f2))) &&                  \
              (((v) & ((f1) | (f2))) == ((f1) | (f2)))) ? 1 : 0)

/*
 * Check if multiple flags set in the specified value (v).
 */
#define BFCMAP84756_CHECK_MULTIPLE_FLAGS_SET(f)         \
            (((f) && (((f) - 1) & (f))) ? 1 : 0)

/* 
 * Bitmap of valid flags for port config.
 */
#define BFCMAP84756_PORT_CONFIG_FLAG_VALID                      \
        (BFCMAP_PORT_ENABLE                               |\
         0)

/*
 * Validate if Port configuration flags are valid.
 */
#define BFCMAP84756_IS_PORT_CONFIG_VALID(f)                 \
            (((f) & ~BFCMAP84756_PORT_CONFIG_FLAG_VALID) ? 0 : 1)

#define BFCMAP84756_USER_DATA_OFFSET__COUNT     2

#define BFCMAP84756_MIB_CLEAR_THRESHOLD         100

#define BFCMAP84756_VALID_EGRESS_SECURE_MATCH_FLAGS            0

/*
 * LMI / UC communications.
 */
#define BFCMAP84756_ERROR_INTR_MASK          (0xfffc0000)
#define BFCMAP84756_ERROR_INTR_STATUS        (0xfffc)

#define BFCMAP84756_UC_INTR_EN               (0x0080)
#define BFCMAP84756_UC_INTR_STATUS           (0x0080)

#define BFCMAP84756_UC_CMD2_UC_ATTN          (0x2000)

#define BFCMAP_PORT_UC_INT_ENABLE_REG(c) (0x2e)
#define BFCMAP_PORT_UC_INT_STATUS_REG(c) (0x2d)
#define BFCMAP_PORT_UC_CMD_REG(c)        (0x22)
#define BFCMAP_PORT_UC_CMD2_REG(c)       (0x2a)
#define BFCMAP_PORT_LMI_UC_MB(c)         (0x28)
#define BFCMAP_PORT_UC_LMI_MB(c)         (0x29)
#define BFCMAP_PORT_UC_DATA_REG0(c)      (0x40)
#define BFCMAP_PORT_UC_DATA_REG14(c)     (0x4e)
#define BFCMAP_PORT_UC_DATA_REG15(c)     (0x4f)
#define BFCMAP_PORT_UC_DATA_REG16(c)     (0x50)


/************************************************************************
 * Local functions
 ************************************************************************/
/*
extern int bfcmap_int_mmi1_reset(bfcmap_dev_addr_t dev_addr);
*/
extern int bfcmap84756_a0_firmware_download(bfcmap_dev_addr_t dev_addr);

STATIC int bfcmap84756_port_config_set(bfcmap_port_ctrl_t *mpc, 
                                     bfcmap_port_config_t *pCfg);

STATIC int bfcmap84756_port_config_get(bfcmap_port_ctrl_t *mpc, 
                                     bfcmap_port_config_t *pCfg);

extern int 
bfcmap_event(bfcmap_port_ctrl_t *mpc, bfcmap_event_t event, 
             int chanId, int assocId);

/***********************************************************
 * Statistics
 ***********************************************************/
STATIC bfcmap_counter_info_t bfcmap84756_counter_info_tbl[] = {
	/* fc_rxdebug0 */
	{ fc_rxdebug0,  0x00100083, 0,
               BFCMAP_COUNTER_F_PORT | BFCMAP_COUNTER_F_SIZE64 },
	/* fc_rxdebug1 */
	{ fc_rxdebug1,  0x00100084, 0,
               BFCMAP_COUNTER_F_PORT | BFCMAP_COUNTER_F_SIZE64 },
	/* fc_rxunicastpkts */
	{ fc_rxunicastpkts,  0x00100085, 0,
               BFCMAP_COUNTER_F_PORT | BFCMAP_COUNTER_F_SIZE64 },
	/* fc_rxgoodframes */
	{ fc_rxgoodframes,  0x00100086, 0,
               BFCMAP_COUNTER_F_PORT | BFCMAP_COUNTER_F_SIZE64 },
	/* fc_rxbcastpkts */
	{ fc_rxbcastpkts,  0x00100087, 0,
               BFCMAP_COUNTER_F_PORT | BFCMAP_COUNTER_F_SIZE64 },
        /* fc_rxbbcredit0 */
	{ fc_rxbbcredit0,  0x00100088, 0,
               BFCMAP_COUNTER_F_PORT | BFCMAP_COUNTER_F_SIZE64 },
	/* fc_rxinvalidcrc */
	{ fc_rxinvalidcrc,  0x00100089, 0,
               BFCMAP_COUNTER_F_PORT | BFCMAP_COUNTER_F_SIZE64 },
	/* fc_rxframetoolong */
	{ fc_rxframetoolong,  0x0010008a, 0,
               BFCMAP_COUNTER_F_PORT | BFCMAP_COUNTER_F_SIZE64 },
	/* fc_rxtruncframes */
	{ fc_rxtruncframes,  0x0010008b, 0,
               BFCMAP_COUNTER_F_PORT | BFCMAP_COUNTER_F_SIZE64 },
	/* fc_rxdelimitererr */
	{ fc_rxdelimitererr,  0x0010008c, 0,
               BFCMAP_COUNTER_F_PORT | BFCMAP_COUNTER_F_SIZE64 },
	/* fc_rxothererr */
	{ fc_rxothererr,  0x0010008d, 0,
               BFCMAP_COUNTER_F_PORT | BFCMAP_COUNTER_F_SIZE64 },
	/* fc_rxruntframes */
	{ fc_rxruntframes,  0x0010008e, 0,
               BFCMAP_COUNTER_F_PORT | BFCMAP_COUNTER_F_SIZE64 },
	/* fc_rxlipcount */
	{ fc_rxlipcount,  0x0010008f, 0,
               BFCMAP_COUNTER_F_PORT | BFCMAP_COUNTER_F_SIZE64 },
	/* fc_rxnoscount */
	{ fc_rxnoscount,  0x00100090, 0,
               BFCMAP_COUNTER_F_PORT | BFCMAP_COUNTER_F_SIZE64 },
	/* fc_rxerrframes */
	{ fc_rxerrframes,  0x00100091, 0,
               BFCMAP_COUNTER_F_PORT | BFCMAP_COUNTER_F_SIZE64 },
	/* fc_rxdropframes */
	{ fc_rxdropframes,  0x00100092, 0,
               BFCMAP_COUNTER_F_PORT | BFCMAP_COUNTER_F_SIZE64 },
	/* fc_rxlinkfail */
	{ fc_rxlinkfail,  0x00100093, 0,
               BFCMAP_COUNTER_F_PORT | BFCMAP_COUNTER_F_SIZE64 },
	/* fc_rxlosssync */
	{ fc_rxlosssync,  0x00100094, 0,
               BFCMAP_COUNTER_F_PORT | BFCMAP_COUNTER_F_SIZE64 },
	/* fc_rxlosssig */
	{ fc_rxlosssig,  0x00100095, 0,
               BFCMAP_COUNTER_F_PORT | BFCMAP_COUNTER_F_SIZE64 },
	/* fc_rxprimseqerr */
	{ fc_rxprimseqerr,  0x00100096, 0,
               BFCMAP_COUNTER_F_PORT | BFCMAP_COUNTER_F_SIZE64 },
	/* fc_rxinvalidword */
	{ fc_rxinvalidword,  0x00100097, 0,
               BFCMAP_COUNTER_F_PORT | BFCMAP_COUNTER_F_SIZE64 },
	/* fc_rxinvalidset */
	{ fc_rxinvalidset,  0x00100098, 0,
               BFCMAP_COUNTER_F_PORT | BFCMAP_COUNTER_F_SIZE64 },
	/* fc_rxencodedisparity */
	{ fc_rxencodedisparity,  0x00100099, 0,
               BFCMAP_COUNTER_F_PORT | BFCMAP_COUNTER_F_SIZE64 },
	/* fc_rxbyt */
	{ fc_rxbyt,  0x0010009a, 0,
               BFCMAP_COUNTER_F_PORT | BFCMAP_COUNTER_F_SIZE64 },
	/* fc_txdebug0 */
	{ fc_txdebug0,  0x001000d9, 0,
               BFCMAP_COUNTER_F_PORT | BFCMAP_COUNTER_F_SIZE64 },
	/* fc_txdebug1 */
	{ fc_txdebug1,  0x001000da, 0,
               BFCMAP_COUNTER_F_PORT | BFCMAP_COUNTER_F_SIZE64 },
	/* fc_txunicastpkts */
	{ fc_txunicastpkts,  0x001000db, 0,
               BFCMAP_COUNTER_F_PORT | BFCMAP_COUNTER_F_SIZE64 },
	/* fc_txbcastpkts */
	{ fc_txbcastpkts,  0x001000dc, 0,
               BFCMAP_COUNTER_F_PORT | BFCMAP_COUNTER_F_SIZE64 },
	/* fc_txbbcredit0 */
	{ fc_txbbcredit0,  0x001000dd, 0,
               BFCMAP_COUNTER_F_PORT | BFCMAP_COUNTER_F_SIZE64 },
	/* fc_txgoodframes */
	{ fc_txgoodframes,  0x001000de, 0,
               BFCMAP_COUNTER_F_PORT | BFCMAP_COUNTER_F_SIZE64 },
	/* fc_txfifounderrun */
	{ fc_txfifounderrun,  0x001000df, 0,
               BFCMAP_COUNTER_F_PORT | BFCMAP_COUNTER_F_SIZE64 },
	/* fc_txdropframes */
	{ fc_txdropframes,  0x001000e0, 0,
               BFCMAP_COUNTER_F_PORT | BFCMAP_COUNTER_F_SIZE64 },
	/* fc_txbyt */
	{ fc_txbyt,  0x001000e1, 0,
               BFCMAP_COUNTER_F_PORT | BFCMAP_COUNTER_F_SIZE64 },

    { 0, 0, 0, 0 } /* last entry */
};


STATIC int cl45_dev = 30 ;
/*********************************************************
 * Local functions and structures
 *********************************************************/
STATIC int
_bfcmap_mdio_read(bfcmap_dev_addr_t dev_addr, 
                        buint32_t io_addr, buint16_t *data)
{
    io_addr = BLMI_IO_CL45_ADDRESS(cl45_dev, io_addr);
    return _blmi_mmi_rd_f(dev_addr, io_addr, data);
}

STATIC int
_bfcmap_mdio_write(bfcmap_dev_addr_t dev_addr, 
                        buint32_t io_addr, buint16_t data)
{
    io_addr = BLMI_IO_CL45_ADDRESS(cl45_dev, io_addr);
    return _blmi_mmi_wr_f(dev_addr, io_addr, data);
}

#define BFCMAP_MDIO_READ(dev_addr, r, pd)   \
                    _bfcmap_mdio_read((dev_addr), (r), (pd))

#define BFCMAP_MDIO_WRITE(dev_addr, r, d)   \
                    _bfcmap_mdio_write((dev_addr), (r), (buint16_t) (d))



STATIC int 
_bfcmap84756_send_lmi_uc_msg(bfcmap_port_ctrl_t *mpc, 
                             bfcmap_lmi_uc_cmds_t  msg_id, buint16_t data)
{
    int rv = BFCMAP_E_NONE;
    buint16_t fval;
    bfcmap_dev_addr_t mdio_addr = mpc->uc_dev_addr ;
    
    /* Write 16 bit date associated with the command */
    BFCMAP_MDIO_WRITE(mdio_addr, BFCMAP_PORT_UC_DATA_REG0(mpc), data);

    fval = msg_id;
    BFCMAP_MDIO_WRITE(mdio_addr, BFCMAP_PORT_LMI_UC_MB(mpc), fval);

    /* Interrupt uc for all other cmd other than BFCMAP_UC_FIRMWARE_INIT */
    if (BFCMAP_UC_FIRMWARE_INIT != msg_id) {
        /* Send interrupt to the microcontroller */
        BFCMAP_MDIO_READ(mdio_addr, BFCMAP_PORT_UC_CMD2_REG(mpc), &fval);
        fval |= BFCMAP84756_UC_CMD2_UC_ATTN;
        BFCMAP_MDIO_WRITE(mdio_addr, BFCMAP_PORT_UC_CMD2_REG(mpc), fval);

        fval &= ~BFCMAP84756_UC_CMD2_UC_ATTN;
        BFCMAP_MDIO_WRITE(mdio_addr, BFCMAP_PORT_UC_CMD2_REG(mpc), fval);
    }

    return rv;
}


STATIC bfcmap_counter_cb_t bfcmap84756_counter_cb = {
    16,
    NULL,
    16,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};


STATIC int 
bfcmap84756_mp_vlanmap_move(bfcmap_dev_ctrl_t *mdc,
                    bfcmap_port_ctrl_t *mpc,
                    int from_idx, int to_idx, int num_en,
                    buint32_t cb_data)
{
    BFCMAP_COMPILER_SATISFY(mdc);
    BFCMAP_COMPILER_SATISFY(mpc);
    BFCMAP_COMPILER_SATISFY(from_idx);
    BFCMAP_COMPILER_SATISFY(to_idx);
    BFCMAP_COMPILER_SATISFY(num_en);
    BFCMAP_COMPILER_SATISFY(cb_data);

    return 0;
}

STATIC int 
bfcmap84756_mp_vlanmap_cmp(bfcmap_dev_ctrl_t *mdc,
                    bfcmap_port_ctrl_t *mpc,
                    bfcmap_int_vlanmap_entry_t* src,
                    bfcmap_int_vlanmap_entry_t* dest,
                    buint32_t cb_data)
{
    bfcmap_vlan_vsan_map_t *fl1, *fl2;

    BFCMAP_COMPILER_SATISFY(mdc);
    BFCMAP_COMPILER_SATISFY(cb_data);

    if (src->pc != dest->pc) {
        goto vlanmap_no_match;
    }

    fl1 = &src->vlanmap;
    fl2 = &dest->vlanmap;

    if (fl1->vlan_vid != fl2->vlan_vid) {
        goto vlanmap_no_match;
    }

    if (fl1->vsan_vfid != fl2->vsan_vfid) {
        goto vlanmap_no_match;
    }

    return 0;

vlanmap_no_match:
    return -1;
}


STATIC int 
bfcmap84756_mp_vlanmap_write(bfcmap_dev_ctrl_t *mdc,
                    bfcmap_port_ctrl_t *mpc,
                    bfcmap_int_vlanmap_entry_t* src,
                    int index, buint32_t cb_data)
{
    bfcmap_vlan_vsan_map_t *fl;
    buint32_t       mem_addr;
    buint32_t       fval;
    bcm84756_a0_VLAN_MAPTABLE_0r_t reg;

    BFCMAP_COMPILER_SATISFY(mdc);
    BFCMAP_COMPILER_SATISFY(cb_data);


    fl = &src->vlanmap;

    bcm84756_a0_VLAN_MAPTABLE_0r_CLR(reg);

    bcm84756_a0_VLAN_MAPTABLE_0r_VLAN_VIDf_SET(mpc, reg, fl->vlan_vid);
    bcm84756_a0_VLAN_MAPTABLE_0r_VSAN_VFIDf_SET(mpc, reg, fl->vsan_vfid);

    mem_addr = bcm84756_a0_VLAN_MAPTABLE_0r(mpc);
    mem_addr += index;

    fval = bcm84756_a0_VLAN_MAPTABLE_0r_GET(reg);
    return BFCMAP_REG32_WRITE(mpc, mem_addr, &fval);
}


STATIC int 
bfcmap84756_mp_vlanmap_clear(bfcmap_dev_ctrl_t *mdc,
                    bfcmap_port_ctrl_t *mpc,
                    int index, int num_en, buint32_t cb_data)
{
    buint32_t       mem_addr;
    buint32_t       fval;
    int ii;
    int rv;

    BFCMAP_COMPILER_SATISFY(mdc);
    BFCMAP_COMPILER_SATISFY(cb_data);

    mem_addr = bcm84756_a0_VLAN_MAPTABLE_0r(mpc);
    mem_addr += index;

    fval = 0;
    for (ii = 0 ; ii < num_en ; ii++) {
        rv = BFCMAP_REG32_WRITE(mpc, mem_addr, &fval);
        if (rv) {
            return rv;
        }
        mem_addr += 1;
    }

    return 0;
}


/*
 * Function:
 *      bfcmap84756_device_init
 * Purpose:
 *      Initializes a bfcmap device.
 * Parameters:
 *      mdc         - bfcmap device control strtucture
 * Returns:
 *      BFCMAP_E_SUCCESS - Success
 *      BFCMAP_E_INTERNAL- Failure
 */
STATIC int 
bfcmap84756_device_init(bfcmap_dev_ctrl_t *mdc)
{
    buint32_t       cb_arg;
    int             port, mp_size;
    bfcmap_port_ctrl_t *mpc;
    bfcmap_hw_mapper_vec_t mpvec;

    /* setup MP callback vectors.  */
    mpvec.mp_cmp    = bfcmap84756_mp_vlanmap_cmp;
    mpvec.mp_clear  = bfcmap84756_mp_vlanmap_clear;
    mpvec.mp_move   = bfcmap84756_mp_vlanmap_move;
    mpvec.mp_write  = bfcmap84756_mp_vlanmap_write;

    BFCMAP_LOCK_DEVICE(mdc);

    /*
     * Init all ports.
     */
    for (port = 0; port < BFCMAP84756_MAX_PORTS; port++) {
        mpc = BFCMAP_UNIT_PORT_CONTROL(mdc, port);

        if (mpc->mp == NULL) {
            cb_arg = 0;
            mp_size = 64;
            mpc->mp = bfcmap_int_hw_mapper_create(mpc, 
                                            mp_size, cb_arg, &mpvec);
        }
    }

    if (mdc->counter_info == NULL) {
        mdc->counter_info = bfcmap_int_stat_tbl_create(mdc, 
                                        bfcmap84756_counter_info_tbl);
        mdc->counter_cb = &bfcmap84756_counter_cb;
    }

    BFCMAP_UNLOCK_DEVICE(mdc);

    return BFCMAP_E_NONE;
}

/*
 * Function:
 *      bfcmap84756_device_uninit
 * Purpose:
 *      Initializes a bfcmap device.
 * Parameters:
 *      mdc         - bfcmap device control strtucture
 * Returns:
 *      BFCMAP_E_SUCCESS - Success
 *      BFCMAP_E_INTERNAL- Failure
 */
STATIC int 
bfcmap84756_device_uninit(bfcmap_dev_ctrl_t *mdc)
{
    bfcmap_port_ctrl_t         *mpc;
    int                         port;

    BFCMAP_LOCK_DEVICE(mdc);

    /* Destroy mp instances */
    for (port = 0; port < BFCMAP84756_MAX_PORTS; port++) {
        mpc = BFCMAP_UNIT_PORT_CONTROL(mdc, port);
        if(mpc->mp) {
            BFCMAP_SAL_FREE(mpc->mp);
            mpc->mp = NULL;
        }
    }
        

    if (mdc->counter_info) {
        BFCMAP_SAL_FREE(mdc->counter_info);
        mdc->counter_info = NULL;
    }

    BFCMAP_UNLOCK_DEVICE(mdc);

    return BFCMAP_E_NONE;
}

/*
 * Function:
 *      bfcmap84756_port_speed_set
 * Purpose:
 *      Sets speed on a bfcmap port
 * Parameters:
 *      mpc          - bfcmap port control strtucture
 *      speed        - Speed to set
 * Returns:
 *      BFCMAP_E_SUCCESS - Success
 *      BFCMAP_E_INTERNAL- Failure
 */
STATIC int 
bfcmap84756_port_speed_set(bfcmap_port_ctrl_t *mpc, bfcmap_port_speed_t speed)
{
    int rv = BFCMAP_E_NONE;
    int fval = 0;

    switch (speed) {
        case BFCMAP_PORT_SPEED_AN:
            fval = 0;
            break;
        case BFCMAP_PORT_SPEED_2GBPS:
            fval = 2;
            break;
        case BFCMAP_PORT_SPEED_4GBPS:
            fval = 4;
            break;
        case BFCMAP_PORT_SPEED_8GBPS:
            fval = 8;
            break;
        default:
            return BFCMAP_E_PARAM;
    }
    /* Send to UC - link Speed */
    rv = _bfcmap84756_send_lmi_uc_msg(mpc, BFCMAP_UC_LINK_SPEED, fval);

    return rv;
}

/*
 * Function:
 *      bfcmap84756_port_config_set
 * Purpose:
 *      Initilaizes the bfcmap port corresponding to the configuration
 *      specified.
 * Parameters:
 *      mpc          - bfcmap port control strtucture
 *      pCfg        - Pointer to port configuration
 * Returns:
 *      BFCMAP_E_SUCCESS - Success
 *      BFCMAP_E_INTERNAL- Failure
 */
STATIC int 
bfcmap84756_port_config_set(bfcmap_port_ctrl_t *mpc, bfcmap_port_config_t *pCfg)
{
    int                                          fval, rv = BFCMAP_E_NONE;
    bcm84756_a0_FCM_FCMON_RX_R_T_TOVr_t          reg_rxtov;
    bcm84756_a0_FCM_FCMON_RX_INTERRUPT_ENABLEr_t reg_ien;
    bcm84756_a0_FCM_DEFAULT_FCOE_SRCMAC_HIr_t    reg_src_hi;
    bcm84756_a0_FCM_DEFAULT_FCOE_SRCMAC_LOr_t    reg_src_lo;
    bcm84756_a0_FCM_DEFAULT_FCOE_DSTMAC_HIr_t    reg_dst_hi;
    bcm84756_a0_FCM_DEFAULT_FCOE_DSTMAC_LOr_t    reg_dst_lo;
    bcm84756_a0_FCM_FCMI_CONFIGr_t               reg_icfg;
    bcm84756_a0_FCM_FCME_CONFIGr_t               reg_ecfg;
    bcm84756_a0_FCM_FCMI_MAC_SRCr_t              reg_imm;
    bcm84756_a0_FCM_FCMI_VFTHDR_PROC_MODEr_t     reg_ivfm;
    bcm84756_a0_FCM_FCMI_VLANTAG_PROC_MODEr_t    reg_ivlm;
    bcm84756_a0_FCM_FCME_VFTHDR_PROC_MODEr_t     reg_evfm;
    bcm84756_a0_FCM_FCME_VLANTAG_PROC_MODEr_t    reg_evlm;
    bcm84756_a0_FCM_DEFAULT_VLAN_TAGr_t          reg_vlan_tag;
    bcm84756_a0_FCM_FCMAC_BBCr_t reg_bbc;

    if (!pCfg) {
        BFCMAP_SAL_DBG_PRINTF("Port configuration is NULL\n");
        return BFCMAP_E_PARAM;
    }

    BFCMAP_LOCK_PORT(mpc);

    /* RX buffer credits : Programming not allowed */

    /* TX buffer Credits */
    bcm84756_a0_READ_FCM_FCMAC_BBCr(mpc, reg_bbc);
    bcm84756_a0_FCM_FCMAC_BBCr_BBC_BB_CREDITf_SET(mpc, reg_bbc, pCfg->tx_buffer_to_buffer_credits);
    bcm84756_a0_WRITE_FCM_FCMAC_BBCr(mpc, reg_bbc);

    /* RX R_T_TOV  */
    if (pCfg->r_t_tov) {
        bcm84756_a0_FCM_FCMON_RX_R_T_TOVr_CLR(reg_rxtov);
        fval = pCfg->r_t_tov ;
        bcm84756_a0_FCM_FCMON_RX_R_T_TOVr_R_T_TOVf_SET(mpc,reg_rxtov, fval);
        bcm84756_a0_WRITE_FCM_FCMON_RX_R_T_TOVr(mpc,reg_rxtov);
    }

    /* VLAN Tag */
    bcm84756_a0_FCM_DEFAULT_VLAN_TAGr_CLR(reg_vlan_tag);
    fval = pCfg->vlan_tag;
    bcm84756_a0_FCM_DEFAULT_VLAN_TAGr_SET(reg_vlan_tag, fval);
    bcm84756_a0_WRITE_FCM_DEFAULT_VLAN_TAGr(mpc, reg_vlan_tag);

    /* Mapper Ingress Config */
    bcm84756_a0_READ_FCM_FCMI_CONFIGr(mpc, reg_icfg);

    /* port mode */
    switch (pCfg->port_mode) {
        case BFCMAP_FCOE_TO_FCOE_MODE: /* FCoE Encap not done */
            fval = 0;
            break;
        case BFCMAP_FCOE_TO_FC_MODE:   /* FCoE encap done */
            fval = 1;
        default:
            break;
    }
    bcm84756_a0_FCM_FCMI_CONFIGr_ENCAP_FC2FCOE_ENf_SET(mpc, reg_icfg, fval);
    fval = (pCfg->mapper_bypass) ? 1 : 0;
    bcm84756_a0_FCM_FCMI_CONFIGr_MAPPER_BYPASS_ENABLEf_SET(mpc, reg_icfg, fval);
    fval = (pCfg->mapper_direction) ? 1 : 0;
    bcm84756_a0_FCM_FCMI_CONFIGr_MAP_TABLE_DIRECTIONf_SET(mpc, reg_icfg, fval);
    fval = (pCfg->map_table_input) ? 1 : 0;
    bcm84756_a0_FCM_FCMI_CONFIGr_MAP_TABLE_INPUTf_SET(mpc, reg_icfg, fval);
    fval = (pCfg->fc_crc_mode) ? 1 : 0;
    bcm84756_a0_FCM_FCMI_CONFIGr_FCS_REGEN_MODEf_SET(mpc, reg_icfg, fval);
    bcm84756_a0_WRITE_FCM_FCMI_CONFIGr(mpc, reg_icfg);

    /* Source MAC */
    bcm84756_a0_READ_FCM_FCMI_MAC_SRCr(mpc, reg_imm);
    bcm84756_a0_FCM_DEFAULT_FCOE_SRCMAC_HIr_CLR(reg_src_hi);
    bcm84756_a0_FCM_DEFAULT_FCOE_SRCMAC_LOr_CLR(reg_src_lo);
    switch (pCfg->src_mac_construct) {
        case BFCMAP_ENCAP_FCOE_FPMA:
            fval = (pCfg->src_fcmap_prefix & 0xffff00) >> 8 ; 
            bcm84756_a0_FCM_DEFAULT_FCOE_SRCMAC_HIr_SRCMACf_SET(mpc, reg_src_hi, fval);
            fval = (pCfg->src_fcmap_prefix & 0xff) >> 24;
            bcm84756_a0_FCM_DEFAULT_FCOE_SRCMAC_LOr_SRCMACf_SET(mpc, reg_src_lo, fval);
            fval = 0; /* Program the SA MAC construct to 0 */
            bcm84756_a0_FCM_FCMI_MAC_SRCr_FCOE_SA_MODEf_SET(mpc, reg_imm, fval);
            bcm84756_a0_WRITE_FCM_FCMI_MAC_SRCr(mpc, reg_imm);
            bcm84756_a0_WRITE_FCM_DEFAULT_FCOE_SRCMAC_HIr(mpc,reg_src_hi);
            bcm84756_a0_WRITE_FCM_DEFAULT_FCOE_SRCMAC_LOr(mpc,reg_src_lo);
            break;
        case BFCMAP_ENCAP_FCOE_ETH_ADDRESS_NULL:
            fval = 0;
            bcm84756_a0_FCM_DEFAULT_FCOE_SRCMAC_HIr_SRCMACf_SET(mpc, reg_src_hi, fval);
            bcm84756_a0_FCM_DEFAULT_FCOE_SRCMAC_LOr_SRCMACf_SET(mpc, reg_src_lo, fval);
            fval = 1; /* Program the SA MAC construct to 1 */
            bcm84756_a0_FCM_FCMI_MAC_SRCr_FCOE_SA_MODEf_SET(mpc, reg_imm, fval);
            bcm84756_a0_WRITE_FCM_FCMI_MAC_SRCr(mpc, reg_imm);
            bcm84756_a0_WRITE_FCM_DEFAULT_FCOE_SRCMAC_HIr(mpc,reg_src_hi);
            bcm84756_a0_WRITE_FCM_DEFAULT_FCOE_SRCMAC_LOr(mpc,reg_src_lo);
            break;
        case BFCMAP_ENCAP_FCOE_ETH_ADDRESS_USER:
            fval = BMAC_TO_32_HI(pCfg->src_mac_addr);
            bcm84756_a0_FCM_DEFAULT_FCOE_SRCMAC_HIr_SRCMACf_SET(mpc, reg_src_hi, fval);
            fval = BMAC_TO_32_LO(pCfg->src_mac_addr);
            bcm84756_a0_FCM_DEFAULT_FCOE_SRCMAC_LOr_SRCMACf_SET(mpc, reg_src_lo, fval);
            fval = 2; /* Program the SA MAC construct to 2 */
            bcm84756_a0_FCM_FCMI_MAC_SRCr_FCOE_SA_MODEf_SET(mpc, reg_imm, fval);
            bcm84756_a0_WRITE_FCM_FCMI_MAC_SRCr(mpc, reg_imm);
            bcm84756_a0_WRITE_FCM_DEFAULT_FCOE_SRCMAC_HIr(mpc,reg_src_hi);
            bcm84756_a0_WRITE_FCM_DEFAULT_FCOE_SRCMAC_LOr(mpc,reg_src_lo);
            break;
    }

    /* Destination MAC */
    bcm84756_a0_READ_FCM_FCMI_MAC_SRCr(mpc, reg_imm);
    bcm84756_a0_FCM_DEFAULT_FCOE_DSTMAC_HIr_CLR(reg_dst_hi);
    bcm84756_a0_FCM_DEFAULT_FCOE_DSTMAC_LOr_CLR(reg_dst_lo);
    switch (pCfg->dst_mac_construct) {
        case BFCMAP_ENCAP_FCOE_FPMA:
            fval = (pCfg->dst_fcmap_prefix & 0xffff00) >> 8;
            bcm84756_a0_FCM_DEFAULT_FCOE_DSTMAC_HIr_DSTMACf_SET(mpc, reg_dst_hi, fval);
            fval = (pCfg->dst_fcmap_prefix & 0xff) >> 24;
            bcm84756_a0_FCM_DEFAULT_FCOE_DSTMAC_LOr_DSTMACf_SET(mpc, reg_dst_lo, fval);
            fval = 0; /* Program the DA MAC construct to 0 */
            bcm84756_a0_FCM_FCMI_MAC_SRCr_FCOE_DA_MODEf_SET(mpc, reg_imm, fval);
            bcm84756_a0_WRITE_FCM_DEFAULT_FCOE_DSTMAC_HIr(mpc,reg_dst_hi);
            bcm84756_a0_WRITE_FCM_DEFAULT_FCOE_DSTMAC_LOr(mpc,reg_dst_lo);
            bcm84756_a0_WRITE_FCM_FCMI_MAC_SRCr(mpc, reg_imm);
            break;
        case BFCMAP_ENCAP_FCOE_ETH_ADDRESS_NULL:
            fval = 0;
            bcm84756_a0_FCM_DEFAULT_FCOE_DSTMAC_HIr_DSTMACf_SET(mpc, reg_dst_hi, fval);
            bcm84756_a0_FCM_DEFAULT_FCOE_DSTMAC_LOr_DSTMACf_SET(mpc, reg_dst_lo, fval);
            fval = 1; /* Program the DA MAC construct to 1 */
            bcm84756_a0_FCM_FCMI_MAC_SRCr_FCOE_DA_MODEf_SET(mpc, reg_imm, fval);
            bcm84756_a0_WRITE_FCM_DEFAULT_FCOE_DSTMAC_HIr(mpc,reg_dst_hi);
            bcm84756_a0_WRITE_FCM_DEFAULT_FCOE_DSTMAC_LOr(mpc,reg_dst_lo);
            bcm84756_a0_WRITE_FCM_FCMI_MAC_SRCr(mpc, reg_imm);
            break;
        case BFCMAP_ENCAP_FCOE_ETH_ADDRESS_USER:
            fval = BMAC_TO_32_HI(pCfg->dst_mac_addr);
            bcm84756_a0_FCM_DEFAULT_FCOE_DSTMAC_HIr_DSTMACf_SET(mpc, reg_dst_hi, fval);
            fval = BMAC_TO_32_LO(pCfg->dst_mac_addr);
            bcm84756_a0_FCM_DEFAULT_FCOE_DSTMAC_LOr_DSTMACf_SET(mpc, reg_dst_lo, fval);
            fval = 2; /* Program the DA MAC construct to 2 */
            bcm84756_a0_FCM_FCMI_MAC_SRCr_FCOE_DA_MODEf_SET(mpc, reg_imm, fval);
            bcm84756_a0_WRITE_FCM_DEFAULT_FCOE_DSTMAC_HIr(mpc,reg_dst_hi);
            bcm84756_a0_WRITE_FCM_DEFAULT_FCOE_DSTMAC_LOr(mpc,reg_dst_lo);
            bcm84756_a0_WRITE_FCM_FCMI_MAC_SRCr(mpc, reg_imm);
            break;
    }

    /* Ingress VLAN header processing mode */
    bcm84756_a0_READ_FCM_FCMI_VLANTAG_PROC_MODEr(mpc, reg_ivlm);
    fval = (pCfg->vlantag_proc_mode) ? 1 : 0;
    bcm84756_a0_FCM_FCMI_VLANTAG_PROC_MODEr_VLANTAG_PRESENCEf_SET(mpc, reg_ivlm, fval);
    bcm84756_a0_WRITE_FCM_FCMI_VLANTAG_PROC_MODEr(mpc, reg_ivlm);

    /* Ingress VFT header processing mode  */
    bcm84756_a0_READ_FCM_FCMI_VFTHDR_PROC_MODEr(mpc, reg_ivfm);
    fval = (pCfg->vfthdr_proc_mode) ? 1 : 0;
    bcm84756_a0_FCM_FCMI_VFTHDR_PROC_MODEr_VFTHDR_PRESENCEf_SET(mpc, reg_ivfm, fval);
    bcm84756_a0_WRITE_FCM_FCMI_VFTHDR_PROC_MODEr(mpc, reg_ivfm);

    /* Mapper Egress Config */
    bcm84756_a0_READ_FCM_FCME_CONFIGr(mpc, reg_ecfg);
    /* port mode */
    switch (pCfg->port_mode) {
        case BFCMAP_FCOE_TO_FCOE_MODE: /* FCoE Decap not done */
            fval = 0;
            break;
        case BFCMAP_FCOE_TO_FC_MODE:   /* FCoE Decap done */
            fval = 1;
        default:
            break;
    }
    bcm84756_a0_FCM_FCME_CONFIGr_DECAP_FCOE2FC_ENABLEf_SET(mpc, reg_ecfg, fval);
    fval = (pCfg->mapper_bypass) ? 1 : 0;
    bcm84756_a0_FCM_FCME_CONFIGr_MAPPER_BYPASS_ENABLEf_SET(mpc, reg_ecfg, fval);
    fval = (pCfg->mapper_direction) ? 1 : 0;
    bcm84756_a0_FCM_FCME_CONFIGr_MAP_TABLE_DIRECTIONf_SET(mpc, reg_ecfg, fval);
    fval = (pCfg->map_table_input) ? 1 : 0;
    bcm84756_a0_FCM_FCME_CONFIGr_MAP_TABLE_INPUTf_SET(mpc, reg_ecfg, fval);
    fval = (pCfg->fc_crc_mode) ? 1 : 0;
    bcm84756_a0_FCM_FCME_CONFIGr_FCS_REGEN_MODEf_SET(mpc, reg_ecfg, fval);
    bcm84756_a0_WRITE_FCM_FCME_CONFIGr(mpc, reg_ecfg);

    /* Egress VLAN header processing mode */
    bcm84756_a0_READ_FCM_FCME_VLANTAG_PROC_MODEr(mpc, reg_evlm);
    fval = (pCfg->vlantag_proc_mode) ? 1 : 0;
    bcm84756_a0_FCM_FCME_VLANTAG_PROC_MODEr_VLANTAG_PRESENCEf_SET(mpc, reg_evlm, fval);
    bcm84756_a0_WRITE_FCM_FCME_VLANTAG_PROC_MODEr(mpc, reg_evlm);

    /* Egress VFT header processing mode  */
    bcm84756_a0_READ_FCM_FCME_VFTHDR_PROC_MODEr(mpc, reg_evfm);
    fval = (pCfg->vfthdr_proc_mode) ? 1 : 0;
    bcm84756_a0_FCM_FCME_VFTHDR_PROC_MODEr_VFTHDR_PRESENCEf_SET(mpc, reg_evfm, fval);
    bcm84756_a0_WRITE_FCM_FCME_VFTHDR_PROC_MODEr(mpc, reg_evfm);

    /* Save the current config. */
    BFCMAP_SAL_MEMCPY(&mpc->cfg, pCfg, sizeof(bfcmap_port_config_t));

    /* Enable Port interrupts */
    if (pCfg->interrupt_enable) {
        BFCMAP_LOCK_DEVICE(mpc->parent);

        bcm84756_a0_FCM_FCMON_RX_INTERRUPT_ENABLEr_CLR(reg_ien);
        fval = 0x3ff ;;
        bcm84756_a0_FCM_FCMON_RX_INTERRUPT_ENABLEr_SET(reg_ien, fval);
        bcm84756_a0_WRITE_FCM_FCMON_RX_INTERRUPT_ENABLEr(mpc,reg_ien);

        BFCMAP_UNLOCK_DEVICE(mpc->parent);
    }

    BFCMAP_UNLOCK_PORT(mpc);
    return rv;
}


/*
 * Function:
 *      bfcmap84756_port_config_get
 * Purpose:
 *      Returns the current bfcmap port configuration.
 * Parameters:
 *      mpc          - bfcmap port control strtucture
 *      pCfg        - Pointer to port configuration
 * Returns:
 *      BFCMAP_E_NONE   - Success
 *      BFCMAP_E_PARAM  - if pCfg is NULL
 */
STATIC int 
bfcmap84756_port_config_get(bfcmap_port_ctrl_t *mpc, 
                            bfcmap_port_config_t *pCfg)
{
    buint32_t                  fval;
    buint32_t                  hi;
    buint32_t                  lo;
    bcm84756_a0_FCM_FCMON_RX_R_T_TOVr_t          reg_rxtov;
    bcm84756_a0_FCM_DEFAULT_FCOE_SRCMAC_HIr_t    reg_src_hi;
    bcm84756_a0_FCM_DEFAULT_FCOE_SRCMAC_LOr_t    reg_src_lo;
    bcm84756_a0_FCM_DEFAULT_FCOE_DSTMAC_HIr_t    reg_dst_hi;
    bcm84756_a0_FCM_DEFAULT_FCOE_DSTMAC_LOr_t    reg_dst_lo;
    bcm84756_a0_FCM_FCMI_CONFIGr_t               reg_icfg;
    bcm84756_a0_FCM_FCMI_MAC_SRCr_t              reg_imm;
    bcm84756_a0_FCM_FCMI_VFTHDR_PROC_MODEr_t     reg_ivfm;
    bcm84756_a0_FCM_FCMI_VLANTAG_PROC_MODEr_t    reg_ivlm;
    bcm84756_a0_FCM_FCME_CONFIGr_t               reg_ecfg;
    bcm84756_a0_FCM_DEFAULT_VLAN_TAGr_t          reg_vlan_tag;
    bcm84756_a0_FCM_FCMAC_BBCr_t                 reg_bbc;
    bcm84756_a0_FCM_FCMAC_FRXB0r_t               reg_frxb0;
    bfcmap_dev_addr_t mdio_addr = mpc->uc_dev_addr ;
    buint16_t fval16;
    buint16_t tpid, id, cfi, pri;

    if (!pCfg) {
        return BFCMAP_E_PARAM;
    }

    BFCMAP_SAL_MEMSET(pCfg, 0, sizeof(bfcmap_port_config_t));

    BFCMAP_LOCK_PORT(mpc);

    pCfg->flags = 0;

    /* RX buffer Credits */
    bcm84756_a0_READ_FCM_FCMAC_FRXB0r(mpc, reg_frxb0);
    pCfg->rx_buffer_to_buffer_credits = 
               bcm84756_a0_FCM_FCMAC_FRXB0r_FRXB_FREE_RXB_CNTf_GET(mpc,
               reg_frxb0);

    /* TX buffer Credits */
    bcm84756_a0_READ_FCM_FCMAC_BBCr(mpc, reg_bbc);
    pCfg->tx_buffer_to_buffer_credits =
                  bcm84756_a0_FCM_FCMAC_BBCr_BBC_BB_CREDITf_GET(mpc, reg_bbc);

    /* Read Link Speed from the LMI register */
    /* Firmware updates Register 0x50 once AN completes. */
    if ( BFCMAP_MDIO_READ(mdio_addr, 0x50, &fval16) != BLMI_E_NONE) {
        return BFCMAP_E_FAIL;
    }
    if (fval16 != BFCMAP_UC_LINK_UP) {
        pCfg->speed = BFCMAP_PORT_SPEED_8GBPS;
        pCfg->port_state = BFCMAP_PORT_STATE_LINKDOWN;
    } else {
        pCfg->port_state = BFCMAP_PORT_STATE_ACTIVE;
        if ( BFCMAP_MDIO_READ(mdio_addr, 0x52, &fval16) != BLMI_E_NONE) {
            return BFCMAP_E_FAIL;
        }
        /* port speed */
        switch (fval16) {
            case 8:
                pCfg->speed = BFCMAP_PORT_SPEED_8GBPS;
                break;
            case 4:
                pCfg->speed = BFCMAP_PORT_SPEED_4GBPS;
                break;
            case 2:
            default:
                pCfg->speed = BFCMAP_PORT_SPEED_2GBPS;
                break;
        }
    }

    /* RX R_T_TOV  */
    bcm84756_a0_READ_FCM_FCMON_RX_R_T_TOVr(mpc,reg_rxtov);
    fval = bcm84756_a0_FCM_FCMON_RX_R_T_TOVr_R_T_TOVf_GET(mpc,reg_rxtov);
    pCfg->r_t_tov = fval ;

    /* VLAN Tag */
    bcm84756_a0_READ_FCM_DEFAULT_VLAN_TAGr(mpc, reg_vlan_tag);
    tpid = bcm84756_a0_FCM_DEFAULT_VLAN_TAGr_TPIDf_GET(mpc, reg_vlan_tag);
    id   = bcm84756_a0_FCM_DEFAULT_VLAN_TAGr_VIDf_GET(mpc, reg_vlan_tag);
    cfi  = bcm84756_a0_FCM_DEFAULT_VLAN_TAGr_CFIf_GET(mpc, reg_vlan_tag);
    pri  = bcm84756_a0_FCM_DEFAULT_VLAN_TAGr_PCPf_GET(mpc, reg_vlan_tag);

    pCfg->vlan_tag =  (((tpid & 0xffff) << 16) |
                       ((pri  & 0x007) << 13) |
                       ((cfi  & 0x001) << 12) |
                       ((id   & 0xfff) << 0));


    /* Source MAC */
    bcm84756_a0_READ_FCM_DEFAULT_FCOE_SRCMAC_HIr(mpc,reg_src_hi);
    hi = bcm84756_a0_FCM_DEFAULT_FCOE_SRCMAC_HIr_SRCMACf_GET(mpc, reg_src_hi);

    bcm84756_a0_READ_FCM_DEFAULT_FCOE_SRCMAC_LOr(mpc,reg_src_lo);
    lo = bcm84756_a0_FCM_DEFAULT_FCOE_SRCMAC_LOr_SRCMACf_GET(mpc, reg_src_lo);

    BMAC_BUILD_FROM_32(pCfg->src_mac_addr, hi, lo);
        
    /* Destination MAC */
    bcm84756_a0_READ_FCM_DEFAULT_FCOE_DSTMAC_HIr(mpc,reg_dst_hi);
    hi = bcm84756_a0_FCM_DEFAULT_FCOE_DSTMAC_HIr_DSTMACf_GET(mpc, reg_dst_hi);

    bcm84756_a0_READ_FCM_DEFAULT_FCOE_DSTMAC_LOr(mpc,reg_dst_lo);
    lo = bcm84756_a0_FCM_DEFAULT_FCOE_DSTMAC_LOr_DSTMACf_GET(mpc, reg_dst_lo);

    BMAC_BUILD_FROM_32(pCfg->dst_mac_addr, hi, lo);

    /* Mapper Ingress Config */
    bcm84756_a0_READ_FCM_FCMI_CONFIGr(mpc, reg_icfg);
    fval = bcm84756_a0_FCM_FCMI_CONFIGr_MAPPER_BYPASS_ENABLEf_GET(mpc, reg_icfg);
    pCfg->mapper_bypass = (fval == 1) ? 1 : 0;
    fval = bcm84756_a0_FCM_FCMI_CONFIGr_ENCAP_FC2FCOE_ENf_GET(mpc, reg_icfg);
    pCfg->fc_mapper_mode = (fval == 1) ? 1 : 0;
    pCfg->port_mode =  (fval == 1) ? BFCMAP_FCOE_TO_FC_MODE : BFCMAP_FCOE_TO_FCOE_MODE ;
    fval = bcm84756_a0_FCM_FCMI_CONFIGr_MAP_TABLE_DIRECTIONf_GET(mpc, reg_icfg);
    pCfg->mapper_direction = (fval == 1) ? 1 : 0;
    fval = bcm84756_a0_FCM_FCMI_CONFIGr_MAP_TABLE_INPUTf_GET(mpc, reg_icfg);
    pCfg->map_table_input = (fval == 1) ? 1 : 0;
    fval = bcm84756_a0_FCM_FCMI_CONFIGr_FCS_REGEN_MODEf_GET(mpc, reg_icfg);
    pCfg->fc_crc_mode = (fval == 1) ? 1 : 0;

    bcm84756_a0_READ_FCM_FCMI_MAC_SRCr(mpc, reg_imm);
    fval = bcm84756_a0_FCM_FCMI_MAC_SRCr_FCOE_SA_MODEf_GET(mpc, reg_imm);
    switch (fval) {
        case 0:
            pCfg->src_mac_construct = BFCMAP_ENCAP_FCOE_FPMA;
            bcm84756_a0_READ_FCM_DEFAULT_FCOE_SRCMAC_HIr(mpc,reg_src_hi);
            hi = bcm84756_a0_FCM_DEFAULT_FCOE_SRCMAC_HIr_SRCMACf_GET(mpc, reg_src_hi);
            pCfg->src_fcmap_prefix = (hi & 0xffff) << 8; 

            bcm84756_a0_READ_FCM_DEFAULT_FCOE_SRCMAC_LOr(mpc,reg_src_lo);
            lo = bcm84756_a0_FCM_DEFAULT_FCOE_SRCMAC_LOr_SRCMACf_GET(mpc, reg_src_lo);
            pCfg->src_fcmap_prefix |= (lo & 0xff000000) >> 24;

            break;
        case 1:
            pCfg->src_mac_construct = BFCMAP_ENCAP_FCOE_ETH_ADDRESS_NULL;
            pCfg->src_fcmap_prefix = 0;
            break;
        case 2:
            pCfg->src_fcmap_prefix = 0;
            pCfg->src_mac_construct = BFCMAP_ENCAP_FCOE_ETH_ADDRESS_USER;
            break;
    }

    fval = bcm84756_a0_FCM_FCMI_MAC_SRCr_FCOE_DA_MODEf_GET(mpc, reg_imm);
    switch (fval) {
        case 0:
            pCfg->dst_mac_construct = BFCMAP_ENCAP_FCOE_FPMA;
            bcm84756_a0_READ_FCM_DEFAULT_FCOE_DSTMAC_HIr(mpc,reg_dst_hi);
            hi = bcm84756_a0_FCM_DEFAULT_FCOE_DSTMAC_HIr_DSTMACf_GET(mpc, reg_dst_hi);
            pCfg->dst_fcmap_prefix = (hi & 0xffff) << 8; 

            bcm84756_a0_READ_FCM_DEFAULT_FCOE_DSTMAC_LOr(mpc,reg_dst_lo);
            lo = bcm84756_a0_FCM_DEFAULT_FCOE_DSTMAC_LOr_DSTMACf_GET(mpc, reg_dst_lo);
            pCfg->dst_fcmap_prefix |= (lo & 0xff000000) >> 24;
            break;
        case 1:
            pCfg->dst_mac_construct = BFCMAP_ENCAP_FCOE_ETH_ADDRESS_NULL;
            pCfg->dst_fcmap_prefix = 0;
            break;
        case 2:
            pCfg->dst_mac_construct = BFCMAP_ENCAP_FCOE_ETH_ADDRESS_USER;
            pCfg->dst_fcmap_prefix = 0;
            break;
    }

    /* Ingress VLAN header processing mode */
    bcm84756_a0_READ_FCM_FCMI_VLANTAG_PROC_MODEr(mpc, reg_ivlm);
    fval = bcm84756_a0_FCM_FCMI_VLANTAG_PROC_MODEr_VLANTAG_PRESENCEf_GET(mpc, reg_ivlm);
    pCfg->vlantag_proc_mode = (fval == 1) ? 1 : 0;

    /* Ingress VFT header processing mode  */
    bcm84756_a0_READ_FCM_FCMI_VFTHDR_PROC_MODEr(mpc, reg_ivfm);
    fval = bcm84756_a0_FCM_FCMI_VFTHDR_PROC_MODEr_VFTHDR_PRESENCEf_GET(mpc, reg_ivfm);
    pCfg->vfthdr_proc_mode = (fval == 1) ? 1 : 0;


    /* Mapper Egress Config */
    bcm84756_a0_READ_FCM_FCME_CONFIGr(mpc, reg_ecfg);
    fval = bcm84756_a0_FCM_FCME_CONFIGr_MAPPER_BYPASS_ENABLEf_GET(mpc, reg_ecfg);
    pCfg->mapper_bypass = (fval == 1) ? 1 : 0;
    fval = bcm84756_a0_FCM_FCME_CONFIGr_DECAP_FCOE2FC_ENABLEf_GET(mpc, reg_ecfg);
    pCfg->fc_mapper_mode = (fval == 1) ? 1 : 0;
    pCfg->port_mode =  (fval == 1) ? BFCMAP_FCOE_TO_FC_MODE : BFCMAP_FCOE_TO_FCOE_MODE ;
    fval = bcm84756_a0_FCM_FCME_CONFIGr_MAP_TABLE_DIRECTIONf_GET(mpc, reg_ecfg);
    pCfg->mapper_direction = (fval == 1) ? 1 : 0;
    fval = bcm84756_a0_FCM_FCME_CONFIGr_MAP_TABLE_INPUTf_GET(mpc, reg_ecfg);
    pCfg->map_table_input = (fval == 1) ? 1 : 0;
    fval = bcm84756_a0_FCM_FCME_CONFIGr_FCS_REGEN_MODEf_GET(mpc, reg_ecfg);
    pCfg->fc_crc_mode = (fval == 1) ? 1 : 0;

    BFCMAP_UNLOCK_PORT(mpc);
    return BFCMAP_E_NONE;
}

/*
 * Function:
 *      bfcmap84756_port_init
 * Purpose:
 *      Initializes a bfcmap port.
 * Parameters:
 *      mpc          - bfcmap port control strtucture
 *      pCfg        - Pointer to port configuration
 * Returns:
 *      BFCMAP_E_SUCCESS - Success
 *      BFCMAP_E_INTERNAL- Failure
 */
STATIC int 
bfcmap84756_port_init(bfcmap_port_ctrl_t *mpc, bfcmap_port_config_t *pCfg)
{
    int                  rv = BFCMAP_E_NONE;
    buint32_t      fval;
    int sval;
    /* buint16_t      cksum; */  /* Disabling checksum checks */
    buint16_t      ver;
    
    bcm84756_a0_FCM_FCMAC_PCFGr_t  reg_pcfg;
    bcm84756_a0_FCM_FCMAC_PCFG1r_t  reg_pcfg1;
    bcm84756_a0_FCM_FCMON_RX_CONTROLr_t  reg_rxc;
    bcm84756_a0_FCM_LINK_RATEr_t  reg_link;
    bcm84756_a0_FCM_FCMAC_MINFRMLEN_CONFIGr_t  reg_minf;
    bcm84756_a0_FCM_FCMAC_MAXFRMLEN_CONFIGr_t  reg_maxf;
    bcm84756_a0_FCM_FCMAC_BADRXCHAR_SUBSTVALr_t reg_badc;
    bcm84756_a0_FCM_FCMAC_RTTRr_t  reg_rttr;
    bcm84756_a0_FCM_FCMON_RX_R_T_TOVr_t reg_rxtov;
    bcm84756_a0_FCM_PCS_CONFIGr_t   reg_pcs;
    bcm84756_a0_FCM_FCMAC_FCFGr_t  reg_fcfg;
    bcm84756_a0_FCM_FCMAC_FRXB0r_t  reg_frxb0;
    bcm84756_a0_FCM_FCMAC_BBCr_t    reg_bbc;
    bcm84756_a0_FCM_FCGEN_TX_CONTROLr_t  reg_txc;
    bcm84756_a0_FCM_TX_DISABLEr_t reg_txdisable;
#if 0
    bcm84756_a0_MPORT_SW_XPORT_RSV_MASKr_t reg_rsv_mask; 
    bcm84756_a0_MPORT_LN_XPORT_RSV_MASKr_t reg_ln_rsv_mask; 
    bcm84756_a0_SP_INGRESS_MTU0_1r_t reg_ingress_mtu0_1; 
    bcm84756_a0_SP_MTU0r_t reg_sp_mtu0; 
#endif
    bcm84756_a0_TX_BUFFER_CONFIGr_t reg_tx_buffer; 
    bcm84756_a0_RX_BUFFER_CONFIGr_t reg_rx_buffer; 
    bcm84756_a0_FCM_FCME_CONFIGr_t reg_fcme_config; 
    bcm84756_a0_FCM_FCMI_CONFIGr_t reg_fcmi_config; 
    bcm84756_a0_FCM_FCME_VFTHDR_PROC_MODEr_t     reg_evfm;
    bcm84756_a0_FCM_FCMI_VFTHDR_PROC_MODEr_t     reg_ivfm;
    bcm84756_a0_FCM_FCMI_VLANTAG_PROC_MODEr_t    reg_ivlm;
    bcm84756_a0_FCM_FCME_VLANTAG_PROC_MODEr_t    reg_evlm;
    bcm84756_a0_FCM_FCMI_MAC_SRCr_t              reg_imm;
    bcm84756_a0_FCM_DEFAULT_FCOE_SRCMAC_HIr_t    reg_src_hi;
    bcm84756_a0_FCM_DEFAULT_FCOE_SRCMAC_LOr_t    reg_src_lo;
    bcm84756_a0_FCM_DEFAULT_FCOE_DSTMAC_HIr_t    reg_dst_hi;
    bcm84756_a0_FCM_DEFAULT_FCOE_DSTMAC_LOr_t    reg_dst_lo;
    bcm84756_a0_FCM_DEFAULT_VLAN_TAGr_t reg_vlan_tag;
    bcm84756_a0_FCM_DEFAULT_VFTHDRr_t reg_vfthdr;
    bfcmap_dev_addr_t mdio_addr = mpc->uc_dev_addr;
    

    BFCMAP_LOCK_PORT(mpc);

    /* Enable Tx and RX, and DISABLE L_PORT */
    bcm84756_a0_FCM_FCMAC_PCFGr_CLR(reg_pcfg);
    bcm84756_a0_FCM_FCMAC_PCFGr_PCFG_RXEf_SET(mpc,reg_pcfg, 1);
    bcm84756_a0_FCM_FCMAC_PCFGr_PCFG_LPDf_SET(mpc,reg_pcfg, 1);
    bcm84756_a0_FCM_FCMAC_PCFGr_PCFG_TXEf_SET(mpc,reg_pcfg, 1);
    bcm84756_a0_WRITE_FCM_FCMAC_PCFGr(mpc,reg_pcfg);

    /* PCFG1 = 0x0 */
    bcm84756_a0_FCM_FCMAC_PCFG1r_CLR(reg_pcfg1);
    bcm84756_a0_WRITE_FCM_FCMAC_PCFG1r(mpc,reg_pcfg1);

    /* Word Sync on K28.5; Comma detection Window 100us */
    bcm84756_a0_FCM_FCMON_RX_CONTROLr_CLR(reg_rxc);
    bcm84756_a0_FCM_FCMON_RX_CONTROLr_RX10B_BIT_ORDERf_SET(mpc,reg_rxc, 0);
    bcm84756_a0_WRITE_FCM_FCMON_RX_CONTROLr(mpc,reg_rxc);

    /* TX control TX speed=8G  */
    bcm84756_a0_FCM_FCGEN_TX_CONTROLr_CLR(reg_txc);
    bcm84756_a0_FCM_FCGEN_TX_CONTROLr_FORCE_SPEED_NOT_ACQUIREDf_SET(mpc,reg_txc, 1);
    bcm84756_a0_FCM_FCGEN_TX_CONTROLr_TRANSMIT_LSB_FIRSTf_SET(mpc,reg_txc, 1);
    bcm84756_a0_WRITE_FCM_FCGEN_TX_CONTROLr(mpc,reg_txc);

    /* 8G speed on TX and RX */
    bcm84756_a0_FCM_LINK_RATEr_CLR(reg_link);
    bcm84756_a0_FCM_LINK_RATEr_RX_RATEf_SET(mpc,reg_link, 2);
    bcm84756_a0_FCM_LINK_RATEr_TX_RATEf_SET(mpc,reg_link, 2);
    bcm84756_a0_WRITE_FCM_LINK_RATEr(mpc,reg_link);

    /* TX Disable = 0*/
    bcm84756_a0_FCM_TX_DISABLEr_CLR(reg_txdisable);
    bcm84756_a0_WRITE_FCM_TX_DISABLEr(mpc,reg_txdisable);

    /* MINFRMLEN_CONFIG = 9 words  */
    bcm84756_a0_FCM_FCMAC_MINFRMLEN_CONFIGr_CLR(reg_minf);
    bcm84756_a0_FCM_FCMAC_MINFRMLEN_CONFIGr_MINLENf_SET(mpc,reg_minf, 9);
    bcm84756_a0_WRITE_FCM_FCMAC_MINFRMLEN_CONFIGr(mpc,reg_minf);

    /* MAX frame config = 2156 bytes = ? words  */
    bcm84756_a0_FCM_FCMAC_MAXFRMLEN_CONFIGr_CLR(reg_maxf);
    bcm84756_a0_FCM_FCMAC_MAXFRMLEN_CONFIGr_MAXLENf_SET(mpc,reg_maxf, 0x21b);
    bcm84756_a0_WRITE_FCM_FCMAC_MAXFRMLEN_CONFIGr(mpc,reg_maxf);

    /* BAD Character Substitute value */
    bcm84756_a0_FCM_FCMAC_BADRXCHAR_SUBSTVALr_CLR(reg_badc);
    bcm84756_a0_FCM_FCMAC_BADRXCHAR_SUBSTVALr_CODE_8B_SUBSTf_SET(mpc,reg_badc, 0);
    bcm84756_a0_WRITE_FCM_FCMAC_BADRXCHAR_SUBSTVALr(mpc,reg_badc);

    /* R_T_TOV  */
    bcm84756_a0_FCM_FCMAC_RTTRr_CLR(reg_rttr);
    bcm84756_a0_FCM_FCMAC_RTTRr_RTTR_R_T_TOVf_SET(mpc,reg_rttr, 0x64);
    bcm84756_a0_WRITE_FCM_FCMAC_RTTRr(mpc,reg_rttr);

    /* RX R_T_TOV  */
    bcm84756_a0_FCM_FCMON_RX_R_T_TOVr_CLR(reg_rxtov);
    fval = 61035;
    bcm84756_a0_FCM_FCMON_RX_R_T_TOVr_R_T_TOVf_SET(mpc,reg_rxtov, fval);
    bcm84756_a0_WRITE_FCM_FCMON_RX_R_T_TOVr(mpc,reg_rxtov);

    /* PCS = 0x0 */
    bcm84756_a0_FCM_PCS_CONFIGr_CLR(reg_pcs);
    bcm84756_a0_WRITE_FCM_PCS_CONFIGr(mpc,reg_pcs);

    /*  force config : Force in OLDP */
    bcm84756_a0_FCM_FCMAC_FCFGr_CLR(reg_fcfg);
    bcm84756_a0_FCM_FCMAC_FCFGr_CFG_FORCE_OLDPf_SET(mpc,reg_fcfg, 1);
    bcm84756_a0_WRITE_FCM_FCMAC_FCFGr(mpc,reg_fcfg);

    bcm84756_a0_FCM_FCMAC_FCFGr_CLR(reg_fcfg);
    bcm84756_a0_FCM_FCMAC_FCFGr_CFG_FORCE_OLDPf_SET(mpc,reg_fcfg, 0);
    bcm84756_a0_WRITE_FCM_FCMAC_FCFGr(mpc,reg_fcfg);

    /* Configure R_RDY */
    bcm84756_a0_FCM_FCMAC_FRXB0r_CLR(reg_frxb0);
    bcm84756_a0_FCM_FCMAC_FRXB0r_FRXB_FREE_RXB_CNTf_SET(mpc, reg_frxb0, 15);
    bcm84756_a0_WRITE_FCM_FCMAC_FRXB0r(mpc,reg_frxb0);

    /*  force config : Force in OL1D */
    bcm84756_a0_FCM_FCMAC_FCFGr_CLR(reg_fcfg);
    bcm84756_a0_FCM_FCMAC_FCFGr_CFG_FORCE_OL1Df_SET(mpc,reg_fcfg, 1);
    bcm84756_a0_WRITE_FCM_FCMAC_FCFGr(mpc,reg_fcfg);

    bcm84756_a0_FCM_FCMAC_FCFGr_CLR(reg_fcfg);
    bcm84756_a0_FCM_FCMAC_FCFGr_CFG_FORCE_OLDPf_SET(mpc,reg_fcfg, 0);
    bcm84756_a0_WRITE_FCM_FCMAC_FCFGr(mpc,reg_fcfg);

    /* Config BBC      */
    bcm84756_a0_FCM_FCMAC_BBCr_CLR(reg_bbc);
    fval = 0x20;
    bcm84756_a0_FCM_FCMAC_BBCr_BBC_BB_CREDITf_SET(mpc,reg_bbc, fval);
    bcm84756_a0_WRITE_FCM_FCMAC_BBCr(mpc,reg_bbc);

#if 0
    /* Set RSV Masks */
    bcm84756_a0_MPORT_SW_XPORT_RSV_MASKr_CLR(reg_rsv_mask); 
    bcm84756_a0_MPORT_SW_XPORT_RSV_MASKr_XPORT_RSV_MASKf_SET(mpc, reg_rsv_mask, 0x10054); 
    bcm84756_a0_WRITE_MPORT_SW_XPORT_RSV_MASKr(mpc,reg_rsv_mask);

    bcm84756_a0_MPORT_LN_XPORT_RSV_MASKr_CLR(reg_ln_rsv_mask); 
    bcm84756_a0_MPORT_LN_XPORT_RSV_MASKr_XPORT_RSV_MASKf_SET(mpc, reg_ln_rsv_mask, 0x30054); 
    bcm84756_a0_WRITE_MPORT_LN_XPORT_RSV_MASKr(mpc,reg_ln_rsv_mask);

    bcm84756_a0_SP_INGRESS_MTU0_1r_CLR(reg_ingress_mtu0_1); 
    bcm84756_a0_SP_INGRESS_MTU0_1r_MTU0f_SET(mpc, reg_ingress_mtu0_1, 0x1400); 
    bcm84756_a0_WRITE_SP_INGRESS_MTU0_1r(reg_ingress_mtu0_1); 

    bcm84756_a0_SP_MTU0r_CLR(reg_sp_mtu0); 
    bcm84756_a0_SP_MTU0r_MTUf_SET(mpc, reg_sp_mtu0, 0x1400); 
    bcm84756_a0_WRITE_SP_MTU0r_MTUf(mpc, reg_sp_mtu0); 
#endif

    bcm84756_a0_TX_BUFFER_CONFIGr_CLR(reg_tx_buffer); 
    bcm84756_a0_TX_BUFFER_CONFIGr_HEADROOMf_SET(mpc, reg_tx_buffer, 0); 
    bcm84756_a0_TX_BUFFER_CONFIGr_THRESHOLDf_SET(mpc, reg_tx_buffer, 2); 
    bcm84756_a0_WRITE_TX_BUFFER_CONFIGr(mpc, reg_tx_buffer); 

    bcm84756_a0_RX_BUFFER_CONFIGr_CLR(reg_rx_buffer); 
    bcm84756_a0_RX_BUFFER_CONFIGr_HEADROOMf_SET(mpc, reg_rx_buffer, 0); 
    bcm84756_a0_RX_BUFFER_CONFIGr_THRESHOLDf_SET(mpc, reg_rx_buffer, 44); 
    bcm84756_a0_WRITE_RX_BUFFER_CONFIGr(mpc, reg_rx_buffer); 

    bcm84756_a0_FCM_FCME_CONFIGr_CLR(reg_fcme_config); 
    bcm84756_a0_FCM_FCME_CONFIGr_FCS_REGEN_MODEf_SET(mpc, reg_fcme_config, 0); 
    bcm84756_a0_FCM_FCME_CONFIGr_FCS_EOF_MODEf_SET(mpc, reg_fcme_config, 0); 
    bcm84756_a0_FCM_FCME_CONFIGr_FCS_FIELD_PRESENTf_SET(mpc, reg_fcme_config, 1); 
    bcm84756_a0_FCM_FCME_CONFIGr_DBG_IGNORE_CRCf_SET(mpc, reg_fcme_config, 0); 
    bcm84756_a0_FCM_FCME_CONFIGr_MAP_TABLE_INPUTf_SET(mpc, reg_fcme_config, 0); 
    bcm84756_a0_FCM_FCME_CONFIGr_MAP_TABLE_DIRECTIONf_SET(mpc, reg_fcme_config, 0); 
    bcm84756_a0_FCM_FCME_CONFIGr_DECAP_FCOE2FC_ENABLEf_SET(mpc, reg_fcme_config, 1); 
    bcm84756_a0_WRITE_FCM_FCME_CONFIGr(mpc, reg_fcme_config); 

    bcm84756_a0_FCM_FCMI_CONFIGr_CLR(reg_fcmi_config); 
    bcm84756_a0_FCM_FCMI_CONFIGr_DBG_IGNORE_CRCf_SET(mpc, reg_fcmi_config, 0); 
    bcm84756_a0_FCM_FCMI_CONFIGr_FCS_EOF_MODEf_SET(mpc, reg_fcmi_config, 0); 
    bcm84756_a0_FCM_FCMI_CONFIGr_ENCAP_FC2FCOE_ENf_SET(mpc, reg_fcmi_config, 1); 
    bcm84756_a0_FCM_FCMI_CONFIGr_MAP_TABLE_INPUTf_SET(mpc, reg_fcmi_config, 0); 
    bcm84756_a0_FCM_FCMI_CONFIGr_FCS_FIELD_PRESENTf_SET(mpc, reg_fcmi_config, 1); 
    bcm84756_a0_WRITE_FCM_FCMI_CONFIGr(mpc, reg_fcmi_config); 

    /* Egress VFT header processing mode  */
    bcm84756_a0_FCM_FCME_VFTHDR_PROC_MODEr_CLR(reg_evfm);
    bcm84756_a0_WRITE_FCM_FCME_VFTHDR_PROC_MODEr(mpc, reg_evfm);

    /* Egress VLAN header processing mode */
    bcm84756_a0_FCM_FCME_VLANTAG_PROC_MODEr_CLR(reg_evlm);
    bcm84756_a0_WRITE_FCM_FCME_VLANTAG_PROC_MODEr(mpc, reg_evlm);

    /* Egress VFT header processing mode  */
    bcm84756_a0_FCM_FCMI_VFTHDR_PROC_MODEr_CLR(reg_ivfm);
    bcm84756_a0_WRITE_FCM_FCMI_VFTHDR_PROC_MODEr(mpc, reg_ivfm);

    /* Ingress VLAN header processing mode */
    bcm84756_a0_FCM_FCMI_VLANTAG_PROC_MODEr_CLR(reg_ivlm);
    fval = 0x1;
    bcm84756_a0_FCM_FCMI_VLANTAG_PROC_MODEr_VLANTAG_PRESENCEf_SET(mpc, reg_ivlm, fval);
    fval = 0x2;
    bcm84756_a0_FCM_FCMI_VLANTAG_PROC_MODEr_VLANTAG_MAPSRCf_SET(mpc, reg_ivlm, fval);
    bcm84756_a0_WRITE_FCM_FCMI_VLANTAG_PROC_MODEr(mpc, reg_ivlm);

    /* FPMA Processing Mode */
    bcm84756_a0_FCM_FCMI_MAC_SRCr_CLR(reg_imm);
    bcm84756_a0_FCM_FCMI_MAC_SRCr_FCOE_SA_MODEf_SET(mpc, reg_imm, 0);
    bcm84756_a0_FCM_FCMI_MAC_SRCr_FCOE_DA_MODEf_SET(mpc, reg_imm, 0);
    bcm84756_a0_WRITE_FCM_FCMI_MAC_SRCr(mpc, reg_imm);


    bcm84756_a0_FCM_DEFAULT_VFTHDRr_CLR(reg_vfthdr);
    bcm84756_a0_FCM_DEFAULT_VFTHDRr_HOPCNTf_SET(mpc, reg_vfthdr, 0xa);
    bcm84756_a0_WRITE_FCM_DEFAULT_VFTHDRr(mpc, reg_vfthdr);

    /* Source MAC */
    bcm84756_a0_FCM_DEFAULT_FCOE_SRCMAC_HIr_CLR(reg_src_hi);
    bcm84756_a0_FCM_DEFAULT_FCOE_SRCMAC_HIr_SRCMACf_SET(mpc, reg_src_hi, 0);
    bcm84756_a0_WRITE_FCM_DEFAULT_FCOE_SRCMAC_HIr(mpc,reg_src_hi);

    bcm84756_a0_FCM_DEFAULT_FCOE_SRCMAC_LOr_CLR(reg_src_lo);
    bcm84756_a0_FCM_DEFAULT_FCOE_SRCMAC_LOr_SRCMACf_SET(mpc, reg_src_lo, 0xFCFCFC);
    bcm84756_a0_WRITE_FCM_DEFAULT_FCOE_SRCMAC_LOr(mpc,reg_src_lo);
        
    /* Destination MAC */
    bcm84756_a0_FCM_DEFAULT_FCOE_DSTMAC_HIr_CLR(reg_dst_hi);
    bcm84756_a0_FCM_DEFAULT_FCOE_DSTMAC_HIr_DSTMACf_SET(mpc, reg_dst_hi, 0x00);
    bcm84756_a0_WRITE_FCM_DEFAULT_FCOE_DSTMAC_HIr(mpc,reg_dst_hi);

    bcm84756_a0_FCM_DEFAULT_FCOE_DSTMAC_LOr_CLR(reg_dst_lo);
    bcm84756_a0_FCM_DEFAULT_FCOE_DSTMAC_LOr_DSTMACf_SET(mpc, reg_dst_lo, 2);
    bcm84756_a0_WRITE_FCM_DEFAULT_FCOE_DSTMAC_LOr(mpc,reg_dst_lo);

    bcm84756_a0_FCM_DEFAULT_VLAN_TAGr_CLR(reg_vlan_tag);
    bcm84756_a0_FCM_DEFAULT_VLAN_TAGr_PCPf_SET(mpc, reg_vlan_tag, 3);
    bcm84756_a0_FCM_DEFAULT_VLAN_TAGr_CFIf_SET(mpc, reg_vlan_tag, 0);
    bcm84756_a0_FCM_DEFAULT_VLAN_TAGr_VIDf_SET(mpc, reg_vlan_tag, 2);
    bcm84756_a0_FCM_DEFAULT_VLAN_TAGr_TPIDf_SET(mpc, reg_vlan_tag, 0x8100);
    bcm84756_a0_WRITE_FCM_DEFAULT_VLAN_TAGr(mpc, reg_vlan_tag);


    /* Enable Port interrupts */
    BFCMAP_LOCK_DEVICE(mpc->parent);

    sval = BFCMAP84756_UC_INTR_EN ;
    BFCMAP_MDIO_WRITE(mdio_addr, BFCMAP_PORT_UC_INT_ENABLE_REG(mpc), sval);

    BFCMAP_UNLOCK_DEVICE(mpc->parent);

    /* Clear counters */
    bfcmap_int_stat_clear(mpc);

    /* Send to UC Firmware Download data */
    bfcmap84756_a0_firmware_download(mdio_addr);
    BFCMAP_SAL_USLEEP(100000);

#if 0 /* Disabling Checksum */
    /* Verify checksum */
    BFCMAP_MDIO_READ(mdio_addr, BFCMAP_PORT_UC_DATA_REG15(mpc), &cksum);
    if (cksum != 0x600d) {
        BFCMAP_SAL_PRINTF("FC firmware checksum incorrect %x , download failed.\n", cksum);
        return BFCMAP_E_FAIL;
    }
#endif
    BFCMAP_MDIO_READ(mdio_addr, BFCMAP_PORT_UC_DATA_REG14(mpc), &ver);
    BFCMAP_SAL_PRINTF("BCM84756:FC firmware version:%4X \n", ver);

    /* Send to UC Firmware Init MB message */
    rv = _bfcmap84756_send_lmi_uc_msg(mpc, BFCMAP_UC_FIRMWARE_INIT, 0);

    BFCMAP_SAL_USLEEP(1000);

/* error: */
    /* Port unlock */
    BFCMAP_UNLOCK_PORT(mpc);

    return rv;
}

/*
 * Function:
 *      bfcmap84756_port_uninit
 * Purpose:
 *      Initializes a bfcmap port.
 * Parameters:
 *      mpc          - bfcmap port control strtucture
 *      pCfg        - Pointer to port configuration
 * Returns:
 *      BFCMAP_E_SUCCESS - Success
 *      BFCMAP_E_INTERNAL- Failure
 */
STATIC int 
bfcmap84756_port_uninit(bfcmap_port_ctrl_t *mpc)
{
    int                                     rv = BFCMAP_E_NONE;
    buint16_t sval;
    bfcmap_dev_addr_t mdio_addr = mpc->uc_dev_addr ;
    
    /* Disable Port interrupts */
    BFCMAP_LOCK_PORT(mpc);
    BFCMAP_LOCK_DEVICE(mpc->parent);

    sval = 0;
    BFCMAP_MDIO_WRITE(mdio_addr, BFCMAP_PORT_UC_INT_ENABLE_REG(mpc), sval);

    BFCMAP_UNLOCK_DEVICE(mpc->parent);
    BFCMAP_UNLOCK_PORT(mpc);

    return rv;
}


/*
 * Function:
 *      bfcmap84756_port_reset
 * Purpose:
 *      Issues a link reset on the port.
 * Parameters:
 *      mpc          - bfcmap port control strtucture
 * Returns:
 *      BFCMAP_E_SUCCESS - Success
 *      BFCMAP_E_INTERNAL- Failure
 */
STATIC int 
bfcmap84756_port_reset(bfcmap_port_ctrl_t *mpc)
{
    int rv = BFCMAP_E_NONE;

    /* Send to UC - link reset */
    rv = _bfcmap84756_send_lmi_uc_msg(mpc, BFCMAP_UC_LINK_RESET, 0);

    return rv;
}

/*
 * Function:
 *      bfcmap84756_port_shutdown
 * Purpose:
 *      Disables the port
 * Parameters:
 *      mpc          - bfcmap port control strtucture
 * Returns:
 *      BFCMAP_E_SUCCESS - Success
 *      BFCMAP_E_INTERNAL- Failure
 */
STATIC int 
bfcmap84756_port_shutdown(bfcmap_port_ctrl_t *mpc)
{
    int rv = BFCMAP_E_NONE;

    /* Send to UC - link shutdown */
    rv = _bfcmap84756_send_lmi_uc_msg(mpc, BFCMAP_UC_LINK_ENABLE, 0);

    return rv;
}

/*
 * Function:
 *      bfcmap84756_port_link_enable
 * Purpose:
 *      Enable the port
 * Parameters:
 *      mpc          - bfcmap port control strtucture
 * Returns:
 *      BFCMAP_E_SUCCESS - Success
 *      BFCMAP_E_INTERNAL- Failure
 */
STATIC int 
bfcmap84756_port_link_enable(bfcmap_port_ctrl_t *mpc)
{
    int rv = BFCMAP_E_NONE;

    /* Send to UC - link Enable */
    rv = _bfcmap84756_send_lmi_uc_msg(mpc, BFCMAP_UC_LINK_ENABLE, 1);

    return rv;
}


/*
 * Function:
 *      bfcmap84756_port_link_bounce
 * Purpose:
 *      Bounce the link
 * Parameters:
 *      mpc          - bfcmap port control strtucture
 * Returns:
 *      BFCMAP_E_SUCCESS - Success
 *      BFCMAP_E_INTERNAL- Failure
 */
STATIC int 
bfcmap84756_port_link_bounce(bfcmap_port_ctrl_t *mpc)
{
    int rv = BFCMAP_E_NONE;

    /* Send to UC - link Bounce */
    rv = _bfcmap84756_send_lmi_uc_msg(mpc, BFCMAP_UC_LINK_BOUNCE, 0);

    return rv;
}


STATIC int
_bfcmap84756_handle_pending_events(bfcmap_port_ctrl_t *mpc)
{
    buint16_t                                       sval;
    buint16_t                                       fval;
    buint16_t                                       port;
    bfcmap_dev_addr_t mdio_addr = mpc->uc_dev_addr ;
    
    BFCMAP_MDIO_READ(mdio_addr, BFCMAP_PORT_UC_INT_STATUS_REG(mpc), &sval);

    /* Hardcode till the issue of interrupt is fixed */
    sval = BFCMAP84756_UC_INTR_STATUS;
    if (sval & BFCMAP84756_UC_INTR_STATUS) {

        BFCMAP_MDIO_READ(mdio_addr, BFCMAP_PORT_UC_LMI_MB(mpc), &port);
        if (port == BFCMAP_PORT(mpc)) {

#if 0
            /* make port invalid */
            port = 0xdead;
            BFCMAP_MDIO_WRITE(mdio_addr, BFCMAP_PORT_UC_DATA_REG16(mpc), port);
#endif

            BFCMAP_MDIO_READ(mdio_addr, BFCMAP_PORT_UC_DATA_REG16(mpc), &fval);

            switch (fval) {
                case BFCMAP_UC_LINK_UP:
                    fval = BFCMAP_EVENT_FC_LINK_INIT;
                    break;
                case BFCMAP_UC_LINK_FAILURE:
                default:
                    fval = BFCMAP_EVENT_FC_LINK_DOWN;
                    break;
            }

            bfcmap_event(mpc, fval, -1, -1);
        }
    }
    

    if (sval & 0xffff) {
        /* clear interrupts */
    }
    return 0;
}

STATIC int 
bfcmap84756_event_handler(bfcmap_dev_ctrl_t *mdc)
{
    int                     port;
    bfcmap_port_ctrl_t     *mpc;

    for (port = 0; port < BFCMAP84756_MAX_PORTS; port++) {
        mpc = BFCMAP_UNIT_PORT_CONTROL(mdc, port);
        if ((mpc->f & BFCMAP_CTRL_FLAG_ATTACHED) == 0) {
            continue;
        }
        _bfcmap84756_handle_pending_events(mpc);
    }
    return 0;
}



/************************************************************
 * BFCMAP84756 API table
 ************************************************************/

bfcmap_drv_t bfcmap84756_a0_drv = {
    bfcmap84756_device_init,
    bfcmap84756_device_uninit,
    bfcmap84756_port_config_set,
    bfcmap84756_port_config_get,
    bfcmap84756_port_init,
    bfcmap84756_port_uninit,
    bfcmap84756_port_reset,
    bfcmap84756_port_shutdown,
    bfcmap84756_port_link_enable,
    bfcmap84756_port_speed_set,
    bfcmap84756_port_link_bounce,
    bfcmap84756_event_handler,
    NULL, NULL
};