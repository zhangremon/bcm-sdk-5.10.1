/*
 * $Id: macsecphy.h 1.5 Broadcom SDK $
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
 * File:        macsecphy.h
 *
 * Header file for MACSEC PHYs
 */
#ifndef SOC_MACSECPHY_H
#define SOC_MACSECPHY_H


#include <soc/phy/phyctrl.h>
#ifdef INCLUDE_MACSEC

#include <bmacsec.h>

typedef bmacsec_dev_addr_t soc_macsec_dev_addr_t;

extern int 
soc_macsecphy_miim_write(soc_macsec_dev_addr_t dev_addr, 
                         uint32 phy_reg_addr, uint16 data);

extern int 
soc_macsecphy_miim_read(soc_macsec_dev_addr_t dev_addr, 
                        uint32 phy_reg_addr, uint16 *data);

/*
 * Create a unique MACSEC portId for specified macsec port
 * connected to unit/port.
 */
#define SOC_MACSEC_PORTID(u, p) (((u) << 16) | ((p) & 0xff))

/*
 * Return port number within BCM unit from macsec portId.
 */
#define SOC_MACSEC_PORTID2UNIT(p)   ((p) >> 16)

/*
 * Return BCM unit number from macsec portId.
 */
#define SOC_MACSEC_PORTID2PORT(p)   ((p) & 0xff)

/*
 * Create MACSEC MDIO address.
 */
#define SOC_MACSECPHY_MDIO_ADDR(unit, mdio, clause45) \
                        ((((unit) & 0xff) << 24)    |       \
                         (((mdio) & 0xff) << 0)     |       \
                         (((clause45) & 0x1) << 8))

#define SOC_MACSECPHY_ADDR2UNIT(a)  (((a) >> 24) & 0xff)

#define SOC_MACSECPHY_ADDR2MDIO(a)  ((a) & 0xff)

#define SOC_MACSECPHY_ADDR_IS_CLAUSE45(a)  (((a) >> 8) & 1)

extern int
soc_macsecphy_init(int unit, soc_port_t port, phy_ctrl_t *pc, 
                   bmacsec_core_t core_type, bmacsec_dev_io_f iofn);



#endif /* INCLUDE_MACSEC */
#endif /* SOC_MACSECPHY_H */
