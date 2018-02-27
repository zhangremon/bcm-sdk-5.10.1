/*
    Copyright 2001, Broadcom Corporation
    All Rights Reserved.
    
    This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
    the contents of this file may not be disclosed to third parties, copied or
    duplicated in any form, in whole or in part, without the prior written
    permission of Broadcom Corporation.
*/
/*
 * Broadcom Home Networking Division 10/100 Mbit/s Ethernet core.
 *
 * This file implements the chip-specific routines
 * for Broadcom HNBU Sonics SiliconBackplane enet cores.
 *
 * $Id: etc47xx.c 1.2.2.4 Broadcom SDK $
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

#include <shared/et/osl.h>
#include <shared/et/bcmendian.h>
#include <shared/et/bcmutils.h>
#include <shared/et/proto/ethernet.h>
#include <shared/et/et_dbg.h>
#include <shared/et/sbconfig.h>
#include <shared/et/sbpci.h>
#include <shared/et/sbutils.h>
#include <shared/et/nvutils.h>
#include <shared/et/bcmenet47xx.h>
#include <shared/et/et_export.h>        /* for et_phyxx() routines */
#include <shared/et/bcmenetmib.h>
#include <soc/hnddma.h>

#if defined(ROBO_4704)

struct bcm4xxx; /* forward declaration */
#define ch_t    struct bcm4xxx
#include <soc/etc.h>

/* private chip state */
struct bcm4xxx {
    void        *et;        /* pointer to et private state */
    etc_soc_info_t  *etc;       /* pointer to etc public state */

    bcmenetregs_t   *regs;      /* pointer to chip registers */
    void        *dev;       /* device handle */

    void        *etphy; /* pointer to et for shared mdc/mdio contortion */

    uint32      intstatus;  /* saved interrupt condition bits */
    uint32      intmask;    /* current software interrupt mask */

    hnddma_t        *di;        /* dma engine software state */

    bool        mibgood; /* true once mib registers have been cleared */
    bcmenetmib_t   mib;        /* mib statistic counters */
    void        *sbh;       /* sb utils handle */
};

/* local prototypes */
static bool chipid(uint vendor, uint device);
static void *chipattach(etc_soc_info_t *etc, void *dev, void *regsva);
static void chipdetach(ch_t *ch);
static void chipreset(ch_t *ch);
static void chipinit(ch_t *ch, bool full);
static bool chiptx(ch_t *ch, void *p);
static void *chiprx(ch_t *ch);
static void chiprxfill(ch_t *ch);
static int chipgetintrevents(ch_t *ch);
static bool chiperrors(ch_t *ch);
static void chipintrson(ch_t *ch);
static void chipintrsoff(ch_t *ch);
static void chiptxreclaim(ch_t *ch, bool all);
static void chiprxreclaim(ch_t *ch);
static void chipstatsupd(ch_t *ch);
static void chipdumpmib(ch_t *ch, char *buf);
static void chipenablepme(ch_t *ch);
static void chipdisablepme(ch_t *ch);
static void chipphyreset(ch_t *ch, uint phyaddr);
static void chipphyinit(ch_t *ch, uint phyaddr);
static uint16 chipphyrd(ch_t *ch, uint phyaddr, uint reg);
static void chipdump(ch_t *ch, char *buf);
static void chiplongname(ch_t *ch, char *buf, uint bufsize);
static void chipduplexupd(ch_t *ch);
static void chipwrcam(struct bcm4xxx *ch, struct ether_addr *ea, uint camindex);
static void chipphywr(struct bcm4xxx *ch, uint phyaddr, uint reg, uint16 v);
static void chipphyor(struct bcm4xxx *ch, uint phyaddr, uint reg, uint16 v);
static void chipphyand(struct bcm4xxx *ch, uint phyaddr, uint reg, uint16 v);
static void chipphyforce(struct bcm4xxx *ch, uint phyaddr);
static void chipphyadvertise(struct bcm4xxx *ch, uint phyaddr);
static char *chipdumpregs(struct bcm4xxx *ch, bcmenetregs_t *regs, char *buf);
static void chiprxreset(struct bcm4xxx *ch, int id);
static void chiprxinit(struct bcm4xxx *ch, int id);
static bool chiprecover(struct bcm4xxx *ch);
static int chipcfprd(ch_t *ch, void *buf);
static int chipcfpwr(ch_t *ch, void *buf);
static int chipcfpfldrd(ch_t *ch, void *buf);
static int chipcfpfldwr(ch_t *ch, void *buf);
static int chipcfpudfrd(ch_t *ch, void *buf);
static int chipcfpudfwr(ch_t *ch, void *buf);
static int chiprxrateset(ch_t *ch, uint channel, uint pps);
static int chiprxrateget(ch_t *ch, uint channel, uint *pps);
static int chiptxrateset(ch_t *ch, uint channel, uint rate, uint burst);
static int chiptxrateget(ch_t *ch, uint channel, uint *rate, uint *burst);
static int chipflowctrlmodeset(ch_t *ch, uint mode);
static int chipflowctrlmodeget(ch_t *ch, uint *mode);
static int chipflowctrlautoset(ch_t *ch, uint on_thresh, uint off_thresh);
static int chipflowctrlautoget(ch_t *ch, uint *on_thresh, uint *off_thresh);
static int chipflowctrlcpuset(ch_t *ch, uint pause_on);
static int chipflowctrlcpuget(ch_t *ch, uint *pause_on);
static int chipflowctrlrxchanset(ch_t *ch, uint rxchan, uint on_thresh, uint off_thresh);
static int chipflowctrlrxchanget(ch_t *ch, uint rxchan, uint *on_thresh, uint *off_thresh);
static int chiptpidset(ch_t *ch, uint index, uint tpid);
static int chiptpidget(ch_t *ch, uint index, uint *tpid);
static int chippvtagset(ch_t *ch, uint private_tag);
static int chippvtagget(ch_t *ch, uint *private_tag);
static int chiprxsephdrset(ch_t *ch, uint enable);
static int chiprxsephdrget(ch_t *ch, uint *enable);
static int chiptxqosmodeset(ch_t *ch, uint mode);
static int chiptxqosmodeget(ch_t *ch, uint *mode);
static int chiptxqosweightset(ch_t *ch, uint queue, uint weight);
static int chiptxqosweightget(ch_t *ch, uint queue, uint *weight);
/* chip interrupt bit error summary */
/*
 * Default Errors: (I_PC | I_PD | I_DE | I_RU | I_RO | I_XU)
 * Remove underflow, should not be an error
 */
#define I_ERRORS    (I_PC | I_PD | I_DE | I_RO | I_XU)
#define DEF_INTMASK (I_XI | I_RI | I_ERRORS)

#define BCM47XX_ROBO_MII_CORE_UNIT    1
#define BCM47XX_ROBO_MDC_PORT         1
#define BCM47XX_ROBO_PHY_ADDR         1


char *cpu_mac = "00-10-18-04-65-50";

struct chops bcm47xx_et_soc_chops = {
    chipid,
    chipattach,
    chipdetach,
    chipreset,
    chipinit,
    chiptx,
    chiprx,
    chiprxfill,
    chipgetintrevents,
    chiperrors,
    chipintrson,
    chipintrsoff,
    chiptxreclaim,
    chiprxreclaim,
    chipstatsupd,
    chipdumpmib,
    chipenablepme,
    chipdisablepme,
    chipphyreset,
    chipphyrd,
    chipphywr,
    chipdump,
    chiplongname,
    chipduplexupd,
    chiprxreset,
    chiprxinit,
    chiprecover,
    chipcfprd,
    chipcfpwr,
    chipcfpfldrd,
    chipcfpfldwr,
    chipcfpudfrd,
    chipcfpudfwr,
    chiprxrateset,
    chiprxrateget,
    chiptxrateset,
    chiptxrateget,
    chipflowctrlmodeset,
    chipflowctrlmodeget,
    chipflowctrlautoset,
    chipflowctrlautoget,
    chipflowctrlcpuset,
    chipflowctrlcpuget,
    chipflowctrlrxchanset,
    chipflowctrlrxchanget,
    chiptpidset,
    chiptpidget,
    chippvtagset,
    chippvtagget,
    chiprxsephdrset,
    chiprxsephdrget,
    chiptxqosmodeset,
    chiptxqosmodeget,
    chiptxqosweightset,
    chiptxqosweightget
};

static bool
chipid(uint vendor, uint device)
{
    if ((vendor == VENDOR_BROADCOM_ID) &&
        (device == BCM47XX_ENET_ID))
        return (TRUE);

    return (FALSE);
}

static void*
chipattach(etc_soc_info_t *etc, void *dev, void *regsva)
{
    struct bcm4xxx *ch;
    bcmenetregs_t *regs;
    char name[16];

    ET_TRACE(("et%d: chipattach: regsva 0x%x\n", etc->unit, (uint)regsva));

    if ((ch = (struct bcm4xxx*) ET_MALLOC(sizeof (struct bcm4xxx))) == NULL) {
        ET_ERROR(("et%d: chipattach: malloc failed!\n", etc->unit));
        return (NULL);
    }
    bzero((char*)ch, sizeof (struct bcm4xxx));

    ch->etc = etc;
    ch->et = etc->et;

    regs = (bcmenetregs_t*) regsva;
    ch->regs = regs;
    ch->dev = dev;

    etc->hwrxoff = ETC47XX_HWRXOFF;

    /* get sb handle */
    if ((ch->sbh = sb_soc_attach(etc->deviceid, ch->dev, regsva, 
        SB_BUS, NULL, NULL)) == NULL) {
        ET_ERROR(("et%d: chipattach: sb_soc_attach error\n", etc->unit));
        goto fail;
    }

    ASSERT(sb_soc_coreid(ch->sbh) == SB_ENET);

    etc->chip = sb_soc_chip(ch->sbh);
    etc->chiprev = sb_soc_chiprev(ch->sbh);
    etc->nicmode = !(sb_soc_bus(ch->sbh) == SB_BUS);
    etc->coreunit = BCM47XX_ROBO_MII_CORE_UNIT;

    /* dma attach */
    sprintf(name, "et%d", (int)dev);
    if ((ch->di = dma_soc_attach(etc->et, name, 0, dev, 0,
        (void *)&regs->dmaregs.xmt, (void *)&regs->dmaregs.rcv,
        NTXD, NRXD, RXBUFSZ, NRXBUFPOST, ETC47XX_HWRXOFF, 
        (uint*) &et_msg_level, etc->pkt_mem, 
        etc->pkthdr_mem, etc->en_rxsephdr[0])) == NULL) {
        ET_ERROR(("et%d: chipattach: dma_attach failed\n", etc->unit));
        goto fail;
    }
    etc->txavail[TX_Q0] = (uint*) dma_soc_getvar(ch->di, "&txavail");

    bcm_ether_atoe(cpu_mac, (char*) &etc->perm_etheraddr);

    if (ETHER_ISNULLADDR(&etc->perm_etheraddr)) {
        ET_ERROR(("et%d: chipattach: invalid format: %s=%s\n", 
            etc->unit, name, cpu_mac));
        goto fail;
    }
    bcopy((char*)&etc->perm_etheraddr, (char*)&etc->cur_etheraddr, 
        ETHER_ADDR_LEN);

    /*
     * Rather than scan the mdio space looking for phys that
     * respond and then guessing which one is ours and which
     * might be used in the dualphy case by another emac,
     * we instead explicitly require the environment var
     * "et<coreunit>phyaddr=<val>".
     */

    etc->phyaddr = BCM47XX_ROBO_PHY_ADDR;

    /* special 5bit phyaddr value indicating no phy present */
    if (etc->phyaddr == PHY_NONE) {
        ET_ERROR(("et%d: chipattach: phy not present\n", etc->unit));
        goto fail;
    }

    etc->mdcport = BCM47XX_ROBO_MDC_PORT;

    /* reset the enet core */
    chipreset(ch);

    /* set default sofware intmask */
    ch->intmask = DEF_INTMASK;

    /*
     * For the 5222 dual phy shared mdio contortion, our phy is
     * on someone elses mdio pins.  This other enet enet
     * may not yet be attached so we must defer the et_soc_phyfind().
     */
    /* if local phy: reset it once now */
    if (etc->mdcport == etc->coreunit)
        chipphyreset(ch, etc->phyaddr);

    return ((void*) ch);

fail:
    chipdetach(ch);
    return (NULL);
}

static void
chipdetach(struct bcm4xxx *ch)
{
    if (ch == NULL)
        return;

    ET_TRACE(("et%d: chipdetach\n", ch->etc->unit));

    /* put the core back into reset */
    sb_soc_core_disable(ch->sbh, 0);


    /* free dma state */
    dma_soc_detach(ch->di);
    ch->di = NULL;

    /* free sb handle */
    sb_soc_detach(ch->sbh);
    ch->sbh = NULL;

    /* free chip private state */
    ET_MFREE(ch, sizeof (struct bcm4xxx));
}

static void
chiplongname(struct bcm4xxx *ch, char *buf, uint bufsize)
{
    char *s;

    switch (ch->etc->deviceid) {
    case BCM4402_ENET_ID:
        s = "Broadcom BCM4402 10/100 Mbps Ethernet Controller";
        break;
    case BCM4307_ENET_ID:
        s = "Broadcom BCM4307 10/100 Mbps Ethernet Controller";
        break;
#ifdef BCMINTERNAL
    case BCM4310_ENET_ID:
        s = "Broadcom BCM4310 10/100 Mbps Ethernet Controller";
        break;
#endif
    case BCM47XX_ENET_ID:
    default:
        s = "Broadcom BCM47xx 10/100 Mbps Ethernet Controller";
        break;
    }

    strncpy(buf, s, bufsize);
    buf[bufsize - 1] = '\0';
}

static void
chipdump(struct bcm4xxx *ch, char *buf)
{
    buf += sprintf(buf, 
        "regs 0x%x etphy 0x%x ch->intstatus 0x%x intmask 0x%x\n",
        (uint)ch->regs, (uint)ch->etphy, ch->intstatus, ch->intmask);
    buf += sprintf(buf, "\n");

    /* dma engine state */
    buf = dma_soc_dump(ch->di, buf, TRUE);
    buf += sprintf(buf, "\n");

    /* registers */
    buf = chipdumpregs(ch, ch->regs, buf);
    buf += sprintf(buf, "\n");
}

#define PRREG(name) \
    buf += sprintf(buf, #name " 0x%x ", R_REG(ch->dev, &regs->name))
#define PRMIBREG(name) \
    buf += sprintf(buf, #name " 0x%x ", R_REG(ch->dev, &regs->mib.name))

static char*
chipdumpregs(struct bcm4xxx *ch, bcmenetregs_t *regs, char *buf)
{
    uint phyaddr;

    phyaddr = ch->etc->phyaddr;

    PRREG(devcontrol); PRREG(biststatus); PRREG(wakeuplength);
    buf += sprintf(buf, "\n");
    PRREG(intstatus); PRREG(intmask); PRREG(gptimer);
    buf += sprintf(buf, "\n");
    PRREG(emactxmaxburstlen); PRREG(emacrxmaxburstlen);
    PRREG(emaccontrol); PRREG(emacflowcontrol);
    buf += sprintf(buf, "\n");
    PRREG(intrecvlazy);
    buf += sprintf(buf, "\n");

    /* emac registers */
    PRREG(rxconfig); PRREG(rxmaxlength); PRREG(txmaxlength);
    buf += sprintf(buf, "\n");
    PRREG(mdiocontrol); PRREG(camcontrol); PRREG(enetcontrol);
    buf += sprintf(buf, "\n");
    PRREG(txcontrol); PRREG(txwatermark); PRREG(mibcontrol);
    buf += sprintf(buf, "\n");

    /* mib registers */
    PRMIBREG(tx_good_octets); PRMIBREG(tx_good_pkts); 
    PRMIBREG(tx_octets); PRMIBREG(tx_pkts);
    buf += sprintf(buf, "\n");
    PRMIBREG(tx_broadcast_pkts); PRMIBREG(tx_multicast_pkts);
    buf += sprintf(buf, "\n");
    PRMIBREG(tx_jabber_pkts); PRMIBREG(tx_oversize_pkts); 
    PRMIBREG(tx_fragment_pkts);
    buf += sprintf(buf, "\n");
    PRMIBREG(tx_underruns); PRMIBREG(tx_total_cols); PRMIBREG(tx_single_cols);
    buf += sprintf(buf, "\n");
    PRMIBREG(tx_multiple_cols); PRMIBREG(tx_excessive_cols); 
    PRMIBREG(tx_late_cols);
    buf += sprintf(buf, "\n");
    PRMIBREG(tx_defered); PRMIBREG(tx_carrier_lost); PRMIBREG(tx_pause_pkts);
    buf += sprintf(buf, "\n");

    PRMIBREG(rx_good_octets); PRMIBREG(rx_good_pkts); 
    PRMIBREG(rx_octets); PRMIBREG(rx_pkts);
    buf += sprintf(buf, "\n");
    PRMIBREG(rx_broadcast_pkts); PRMIBREG(rx_multicast_pkts);
    buf += sprintf(buf, "\n");
    PRMIBREG(rx_jabber_pkts); PRMIBREG(rx_oversize_pkts); 
    PRMIBREG(rx_fragment_pkts);
    buf += sprintf(buf, "\n");
    PRMIBREG(rx_missed_pkts); PRMIBREG(rx_crc_align_errs); 
    PRMIBREG(rx_undersize);
    buf += sprintf(buf, "\n");
    PRMIBREG(rx_crc_errs); PRMIBREG(rx_align_errs); PRMIBREG(rx_symbol_errs);
    buf += sprintf(buf, "\n");
    PRMIBREG(rx_pause_pkts); PRMIBREG(rx_nonpause_pkts);
    buf += sprintf(buf, "\n");
    buf += sprintf(buf, "\n");

    if (phyaddr != PHY_NOMDC) {
        /* print a few interesting phy registers */
        buf += sprintf(buf, "phy0 0x%x phy1 0x%x phy2 0x%x phy3 0x%x\n",
                   chipphyrd(ch, phyaddr, 0),
                   chipphyrd(ch, phyaddr, 1),
                   chipphyrd(ch, phyaddr, 2),
                   chipphyrd(ch, phyaddr, 3));
        buf += sprintf(buf, "phy4 0x%x phy5 0x%x phy24 0x%x phy25 0x%x\n",
                   chipphyrd(ch, phyaddr, 4),
                   chipphyrd(ch, phyaddr, 5),
                   chipphyrd(ch, phyaddr, 24),
                   chipphyrd(ch, phyaddr, 25));
    }

    return (buf);
}

static void
chipreset(struct bcm4xxx *ch)
{
    bcmenetregs_t *regs;
    /* uint32 offset; */

    ET_TRACE(("et%d: chipreset\n", ch->etc->unit));
#ifdef MDC_MDIO_SUPPORT
    SPI_LOCK;
#endif /* MDC_MDIO_SUPPORT */


    regs = ch->regs;

    if (!sb_soc_iscoreup(ch->sbh)) {
        /*
        if (!ch->etc->nicmode)
            sb_pci_setup(ch->sbh, &offset, (1 << sb_coreidx(ch->sbh)));
        */
        /* power on reset: reset the enet core */
        sb_soc_core_reset(ch->sbh, 0);
        goto chipinreset;
    }

    /* read counters before resetting the chip */
    if (ch->mibgood)
        chipstatsupd(ch);

    /* reset the dma engines */
    dma_soc_txreset(ch->di);
    dma_soc_rxreset(ch->di);

    /* reset core */
    /* 4402A0 requires PR3714 workaround */
    if ((ch->etc->deviceid == BCM4402_ENET_ID) && (ch->etc->chiprev == 0)) {
        W_REG(ch->dev, &regs->enetcontrol, EC_ES);
        SPINWAIT((R_REG(ch->dev, &regs->enetcontrol) & EC_ES), 100);
        ASSERT((R_REG(ch->dev, &regs->enetcontrol) & EC_ES) == 0);
        W_REG(ch->dev, &regs->intmask, 0);
        W_REG(ch->dev, &regs->intstatus, R_REG(ch->dev, &regs->intstatus));
        ASSERT(R_REG(ch->dev, &regs->intstatus) == 0);
        W_REG(ch->dev, &regs->biststatus, 0);
        W_REG(ch->dev, &regs->wakeuplength, 0);
        W_REG(ch->dev, &regs->gptimer, 0);
        W_REG(ch->dev, &regs->emaccontrol, 0);
        W_REG(ch->dev, &regs->emacflowcontrol, 0);
        W_REG(ch->dev, &regs->intrecvlazy, 0);
    } else
        sb_soc_core_reset(ch->sbh, 0);

chipinreset:

    /* must clear mib registers by hand */
    W_REG(ch->dev, &regs->mibcontrol, EMC_RZ);
    (void) R_REG(ch->dev, &regs->mib.tx_broadcast_pkts);
    (void) R_REG(ch->dev, &regs->mib.tx_multicast_pkts);
    (void) R_REG(ch->dev, &regs->mib.tx_len_64);
    (void) R_REG(ch->dev, &regs->mib.tx_len_65_to_127);
    (void) R_REG(ch->dev, &regs->mib.tx_len_128_to_255);
    (void) R_REG(ch->dev, &regs->mib.tx_len_256_to_511);
    (void) R_REG(ch->dev, &regs->mib.tx_len_512_to_1023);
    (void) R_REG(ch->dev, &regs->mib.tx_len_1024_to_max);
    (void) R_REG(ch->dev, &regs->mib.tx_jabber_pkts);
    (void) R_REG(ch->dev, &regs->mib.tx_oversize_pkts);
    (void) R_REG(ch->dev, &regs->mib.tx_fragment_pkts);
    (void) R_REG(ch->dev, &regs->mib.tx_underruns);
    (void) R_REG(ch->dev, &regs->mib.tx_total_cols);
    (void) R_REG(ch->dev, &regs->mib.tx_single_cols);
    (void) R_REG(ch->dev, &regs->mib.tx_multiple_cols);
    (void) R_REG(ch->dev, &regs->mib.tx_excessive_cols);
    (void) R_REG(ch->dev, &regs->mib.tx_late_cols);
    (void) R_REG(ch->dev, &regs->mib.tx_defered);
    (void) R_REG(ch->dev, &regs->mib.tx_carrier_lost);
    (void) R_REG(ch->dev, &regs->mib.tx_pause_pkts);
    (void) R_REG(ch->dev, &regs->mib.rx_broadcast_pkts);
    (void) R_REG(ch->dev, &regs->mib.rx_multicast_pkts);
    (void) R_REG(ch->dev, &regs->mib.rx_len_64);
    (void) R_REG(ch->dev, &regs->mib.rx_len_65_to_127);
    (void) R_REG(ch->dev, &regs->mib.rx_len_128_to_255);
    (void) R_REG(ch->dev, &regs->mib.rx_len_256_to_511);
    (void) R_REG(ch->dev, &regs->mib.rx_len_512_to_1023);
    (void) R_REG(ch->dev, &regs->mib.rx_len_1024_to_max);
    (void) R_REG(ch->dev, &regs->mib.rx_jabber_pkts);
    (void) R_REG(ch->dev, &regs->mib.rx_oversize_pkts);
    (void) R_REG(ch->dev, &regs->mib.rx_fragment_pkts);
    (void) R_REG(ch->dev, &regs->mib.rx_missed_pkts);
    (void) R_REG(ch->dev, &regs->mib.rx_crc_align_errs);
    (void) R_REG(ch->dev, &regs->mib.rx_undersize);
    (void) R_REG(ch->dev, &regs->mib.rx_crc_errs);
    (void) R_REG(ch->dev, &regs->mib.rx_align_errs);
    (void) R_REG(ch->dev, &regs->mib.rx_symbol_errs);
    (void) R_REG(ch->dev, &regs->mib.rx_pause_pkts);
    (void) R_REG(ch->dev, &regs->mib.rx_nonpause_pkts);
    ch->mibgood = TRUE;

    /*
     * We want the phy registers to be accessible even when
     * the driver is "downed" so initialize MDC preamble, frequency,
     * and whether internal or external phy here.
     */
    if (ch->etc->deviceid == BCM47XX_ENET_ID) {
        /* 4710A0 has a 100Mhz SB clock and external phy */
        W_REG(ch->dev, &regs->mdiocontrol, 0x94);
    } else if (ch->etc->deviceid == BCM4402_ENET_ID) {
        /* 4402 has 62.5Mhz SB clock and internal phy */
        W_REG(ch->dev, &regs->mdiocontrol, 0x8d);
#ifdef BCMINTERNAL
    } else if (ch->etc->deviceid == BCM4307_ENET_ID) {
        /* 4307/4309 have 88Mhz SB clock and external phy */
        W_REG(ch->dev, &regs->mdiocontrol, 0x92);
    } else if (ch->etc->deviceid == BCM4310_ENET_ID) {
        /* 4310 has a 124.8Mhz SB clock and external phy */
        W_REG(ch->dev, &regs->mdiocontrol, 0x99);
#else
    } else if (ch->etc->deviceid == BCM4307_ENET_ID) {
        /* 4307 has 88 MHz SB clock and external phy */
        W_REG(ch->dev, &regs->mdiocontrol, 0x92);
#endif
    } else
        ASSERT(0);

    /* some chips have internal phy, some don't */
    if (!(R_REG(ch->dev, &regs->devcontrol) & DC_IP)) {
        W_REG(ch->dev, &regs->enetcontrol, EC_EP);
    } else if (R_REG(ch->dev, &regs->devcontrol) & DC_ER) {
        AND_REG(ch->dev, &regs->devcontrol, ~DC_ER);
        OSL_DELAY(100);
        chipphyinit(ch, ch->etc->phyaddr);
    }

    /* clear persistent sw intstatus */
    ch->intstatus = 0;
#ifdef MDC_MDIO_SUPPORT
    SPI_UNLOCK;
#endif /* MDC_MDIO_SUPPORT */

}

/*
 * Initialize all the chip registers.  If dma mode, init tx and rx dma engines
 * but leave the devcontrol tx and rx (fifos) disabled.
 */
static void
chipinit(struct bcm4xxx *ch, bool full)
{
    etc_soc_info_t *etc;
    bcmenetregs_t *regs;
    uint idx;
    uint i;

    regs = ch->regs;
    etc = ch->etc;
    idx = 0;

    ET_TRACE(("et%d: chipinit\n", etc->unit));

    /* Do timeout fixup */
    sb_soc_core_tofixup(ch->sbh);

    /* enable crc32 generation */
    OR_REG(ch->dev, &regs->emaccontrol, EMC_CG);

    /* enable one rx interrupt per received frame */
    W_REG(ch->dev, &regs->intrecvlazy, (1 << IRL_FC_SHIFT));

    /* enable 802.3x tx flow control (honor received PAUSE frames) */
    W_REG(ch->dev, &regs->rxconfig, ERC_FE | ERC_UF);

    /* initialize CAM */
    if (etc->promisc)
        OR_REG(ch->dev, &regs->rxconfig, ERC_PE);
    else {
        /* our local address */
        chipwrcam(ch, &etc->cur_etheraddr, idx++);

        /* allmulti or a list of discrete multicast addresses */
        if (etc->allmulti)
            OR_REG(ch->dev, &regs->rxconfig, ERC_AM);
        else if (etc->nmulticast) {
            for (i = 0; i < etc->nmulticast; i++)
                chipwrcam(ch, &etc->multicast[i], idx++);
        }

        /* enable cam */
        OR_REG(ch->dev, &regs->camcontrol, CC_CE);
    }

    /* optionally enable mac-level loopback */
    if (etc->loopbk)
        OR_REG(ch->dev, &regs->rxconfig, ERC_LE);

    /* set max frame lengths - account for possible vlan tag */
    W_REG(ch->dev, &regs->rxmaxlength, ETHER_MAX_LEN + 32);
    W_REG(ch->dev, &regs->txmaxlength, ETHER_MAX_LEN + 32);

    /* set tx watermark */
    W_REG(ch->dev, &regs->txwatermark, 56);

    /*
     * Optionally, disable phy autonegotiation and force our speed/duplex
     * or constrain our advertised capabilities.
     */
    if (etc->forcespeed != ET_AUTO)
        chipphyforce(ch, etc->phyaddr);
    else if (etc->advertise && etc->needautoneg)
        chipphyadvertise(ch, etc->phyaddr);

    /* initialize the tx and rx dma channels */
    dma_soc_txinit(ch->di);
    dma_soc_rxinit(ch->di);

    if (full) {
        /* post dma receive buffers */
        dma_soc_rxfill(ch->di);
    }

    /* lastly, enable interrupts */
    W_REG(ch->dev, &regs->intmask, ch->intmask);

#ifndef MDC_MDIO_SUPPORT
    /* set EMAC MIIStatusContol 0 to stop MDC/MDIO clock*/
    W_REG(ch->dev,&regs->mdiocontrol,0x0);
#endif

    /* turn on the emac */
    OR_REG(ch->dev, &regs->enetcontrol, EC_EE);
}

/* dma transmit */
static bool
chiptx(struct bcm4xxx *ch, void *p0)
{
    int error;

    ET_TRACE(("et%d: chiptx\n", ch->etc->unit));
    ET_LOG("et%d: chiptx", ch->etc->unit, 0);

    error = dma_soc_txfast(ch->di, p0, TRUE);

    if (error) {
        ET_ERROR(("et%d: chiptx: out of txds\n", ch->etc->unit));
        ch->etc->txnobuf++;
        return FALSE;
    }
    return TRUE;
}

/* reclaim complete transmit descriptors and packets */
static void
chiptxreclaim(struct bcm4xxx *ch, bool forceall)
{
    ET_TRACE(("et%d: chiptxreclaim\n", ch->etc->unit));
    dma_soc_txreclaim(ch->di, forceall);
    ch->intstatus &= ~I_XI;
}

/* dma receive: returns a pointer to the next frame received, or NULL if there are no more */
static void*
chiprx(struct bcm4xxx *ch)
{
    void *p;

    ET_TRACE(("et%d: chiprx\n", ch->etc->unit));
    ET_LOG("et%d: chiprx", ch->etc->unit, 0);

    if ((p = dma_soc_rx(ch->di)) == NULL)
        ch->intstatus &= ~I_RI;

    return (p);
}

/* reclaim completes dma transmit descriptors and packets */
static void
chiprxreclaim(struct bcm4xxx *ch)
{
    ET_TRACE(("et%d: chiprxreclaim\n", ch->etc->unit));
    dma_soc_rxreclaim(ch->di);
    ch->intstatus &= ~I_RI;
}

/* allocate and post dma receive buffers */
static void
chiprxfill(struct bcm4xxx *ch)
{
    ET_TRACE(("et%d: chiprxfill\n", ch->etc->unit));
    ET_LOG("et%d: chiprx", ch->etc->unit, 0);
    dma_soc_rxfill(ch->di);
}

/* get current and pending interrupt events */
static int
chipgetintrevents(struct bcm4xxx *ch)
{
    bcmenetregs_t *regs;
    uint32 intstatus;
    int events;
    uint32 statelow;

    regs = ch->regs;
    events = 0;

    /* Check if chip is during init stage */
    statelow = R_REG(ch->dev, &regs->sbconfig.sbtmstatelow);
    if ((statelow & SBTML_RESET) || !(statelow & SBTML_CLK)) {
        soc_cm_print("EMAC reset is set!\n");
        return 0;
    }

    /* read the interrupt status register */
    intstatus = R_REG(ch->dev, &regs->intstatus);

    /* if there are new events */
    if (intstatus & ch->intmask)
        events |= INTR_NEW;

    /* or new bits into persistent intstatus */
    intstatus = (ch->intstatus |= intstatus);

    /* return if no events */
    if (intstatus == 0)
        return (0);

    /* clear non-error interrupt conditions */
    W_REG(ch->dev, &regs->intstatus, intstatus);

    /* convert chip-specific intstatus bits into generic intr event bits */
    if (intstatus & I_RI)
        events |= INTR_RX;
    if (intstatus & I_XI)
        events |= INTR_TX;
    if (intstatus & I_ERRORS)
        events |= INTR_ERROR;
    if (intstatus & I_TO)
        events |= INTR_TO;

    return (events);
}

/* enable chip interrupts */
static void
chipintrson(struct bcm4xxx *ch)
{
    ch->intmask = DEF_INTMASK;
    W_REG(ch->dev, &ch->regs->intmask, ch->intmask);
}

/* disable chip interrupts */
static void
chipintrsoff(struct bcm4xxx *ch)
{
    ch->intmask = 0;
    W_REG(ch->dev, &ch->regs->intmask, ch->intmask);
    (void) R_REG(ch->dev, &ch->regs->intmask);  /* sync readback */
}

/* return true of caller should re-initialize, otherwise false */
static bool
chiperrors(struct bcm4xxx *ch)
{
    uint32 intstatus;
    etc_soc_info_t *etc;

    etc = ch->etc;

    intstatus = ch->intstatus;
    ch->intstatus &= ~(I_ERRORS);

    ET_TRACE(("et%d: chiperrors: intstatus 0x%x\n", etc->unit, intstatus));

    if (intstatus & I_PC) {
        ET_ERROR(("et%d: descriptor error\n", etc->unit));
        etc->dmade++;
    }

    if (intstatus & I_PD) {
        ET_ERROR(("et%d: data error\n", etc->unit));
        etc->dmada++;
    }

    if (intstatus & I_DE) {
        ET_ERROR(("et%d: descriptor protocol error\n", etc->unit));
        etc->dmape++;
    }

    if (intstatus & I_RU) {
        ET_ERROR(("et%d: receive descriptor underflow\n", etc->unit));
        etc->rxdmauflo++;
    }

    if (intstatus & I_RO) {
        ET_ERROR(("et%d: receive fifo overflow\n", etc->unit));
        etc->rxoflo++;
    }

    if (intstatus & I_XU) {
        ET_ERROR(("et%d: transmit fifo underflow\n", etc->unit));
        etc->txuflo++;
    }

    return (TRUE);
}

static void
chipwrcam(struct bcm4xxx *ch, struct ether_addr *ea, uint camindex)
{
    uint32 w;

    ASSERT((R_REG(ch->dev, &ch->regs->camcontrol) & (CC_CB | CC_CE)) == 0);

    w = (ea->octet[2] << 24) | (ea->octet[3] << 16) | (ea->octet[4] << 8)
        | ea->octet[5];
    W_REG(ch->dev, &ch->regs->camdatalo, w);
    w = CD_V | (ea->octet[0] << 8) | ea->octet[1];
    W_REG(ch->dev, &ch->regs->camdatahi, w);
    W_REG(ch->dev, &ch->regs->camcontrol, ((camindex << CC_INDEX_SHIFT) | CC_WR));

    /* spin until done */
    SPINWAIT((R_REG(ch->dev, &ch->regs->camcontrol) & CC_CB), 1000);
    ASSERT((R_REG(ch->dev, &ch->regs->camcontrol) & CC_CB) == 0);
}

static void
chipstatsupd(struct bcm4xxx *ch)
{
    etc_soc_info_t *etc;
    bcmenetregs_t *regs;
    bcmenetmib_t *m;

    etc = ch->etc;
    regs = ch->regs;
    m = &ch->mib;

    /*
     * mib counters are clear-on-read.
     * Don't bother using the pkt and octet counters since they are only
     * 16bits and wrap too quickly to be useful.
     */
    m->tx_broadcast_pkts += R_REG(ch->dev, &regs->mib.tx_broadcast_pkts);
    m->tx_multicast_pkts += R_REG(ch->dev, &regs->mib.tx_multicast_pkts);
    m->tx_len_64 += R_REG(ch->dev, &regs->mib.tx_len_64);
    m->tx_len_65_to_127 += R_REG(ch->dev, &regs->mib.tx_len_65_to_127);
    m->tx_len_128_to_255 += R_REG(ch->dev, &regs->mib.tx_len_128_to_255);
    m->tx_len_256_to_511 += R_REG(ch->dev, &regs->mib.tx_len_256_to_511);
    m->tx_len_512_to_1023 += 
        R_REG(ch->dev, &regs->mib.tx_len_512_to_1023);
    m->tx_len_1024_to_max += 
        R_REG(ch->dev, &regs->mib.tx_len_1024_to_max);
    m->tx_jabber_pkts += R_REG(ch->dev, &regs->mib.tx_jabber_pkts);
    m->tx_oversize_pkts += R_REG(ch->dev, &regs->mib.tx_oversize_pkts);
    m->tx_fragment_pkts += R_REG(ch->dev, &regs->mib.tx_fragment_pkts);
    m->tx_underruns += R_REG(ch->dev, &regs->mib.tx_underruns);
    m->tx_total_cols += R_REG(ch->dev, &regs->mib.tx_total_cols);
    m->tx_single_cols += R_REG(ch->dev, &regs->mib.tx_single_cols);
    m->tx_multiple_cols += R_REG(ch->dev, &regs->mib.tx_multiple_cols);
    m->tx_excessive_cols += R_REG(ch->dev, &regs->mib.tx_excessive_cols);
    m->tx_late_cols += R_REG(ch->dev, &regs->mib.tx_late_cols);
    m->tx_defered += R_REG(ch->dev, &regs->mib.tx_defered);
    m->tx_carrier_lost += R_REG(ch->dev, &regs->mib.tx_carrier_lost);
    m->tx_pause_pkts += R_REG(ch->dev, &regs->mib.tx_pause_pkts);
    m->rx_broadcast_pkts += R_REG(ch->dev, &regs->mib.rx_broadcast_pkts);
    m->rx_multicast_pkts += R_REG(ch->dev, &regs->mib.rx_multicast_pkts);
    m->rx_len_64 += R_REG(ch->dev, &regs->mib.rx_len_64);
    m->rx_len_65_to_127 += R_REG(ch->dev, &regs->mib.rx_len_65_to_127);
    m->rx_len_128_to_255 += R_REG(ch->dev, &regs->mib.rx_len_128_to_255);
    m->rx_len_256_to_511 += R_REG(ch->dev, &regs->mib.rx_len_256_to_511);
    m->rx_len_512_to_1023 += 
        R_REG(ch->dev, &regs->mib.rx_len_512_to_1023);
    m->rx_len_1024_to_max += 
        R_REG(ch->dev, &regs->mib.rx_len_1024_to_max);
    m->rx_jabber_pkts += R_REG(ch->dev, &regs->mib.rx_jabber_pkts);
    m->rx_oversize_pkts += R_REG(ch->dev, &regs->mib.rx_oversize_pkts);
    m->rx_fragment_pkts += R_REG(ch->dev, &regs->mib.rx_fragment_pkts);
    m->rx_missed_pkts += R_REG(ch->dev, &regs->mib.rx_missed_pkts);
    m->rx_crc_align_errs += R_REG(ch->dev, &regs->mib.rx_crc_align_errs);
    m->rx_undersize += R_REG(ch->dev, &regs->mib.rx_undersize);
    m->rx_crc_errs += R_REG(ch->dev, &regs->mib.rx_crc_errs);
    m->rx_align_errs += R_REG(ch->dev, &regs->mib.rx_align_errs);
    m->rx_symbol_errs += R_REG(ch->dev, &regs->mib.rx_symbol_errs);
    m->rx_pause_pkts += R_REG(ch->dev, &regs->mib.rx_pause_pkts);
    m->rx_nonpause_pkts += R_REG(ch->dev, &regs->mib.rx_nonpause_pkts);

    /*
     * Aggregate transmit and receive errors that probably resulted
     * in the loss of a frame are computed on the fly.
     *
     * We seem to get lots of tx_carrier_lost errors when flipping
     * speed modes so don't count these as tx errors.
     *
     * Arbitrarily lump the non-specific dma errors as tx errors.
     */
    etc->txerror = m->tx_jabber_pkts + m->tx_oversize_pkts
        + m->tx_underruns + m->tx_excessive_cols
        + m->tx_late_cols + etc->txnobuf + etc->dmade
        + etc->dmada + etc->dmape + etc->txuflo + etc->txnobuf;
    etc->rxerror = m->rx_jabber_pkts + m->rx_oversize_pkts
        + m->rx_missed_pkts + m->rx_crc_align_errs
        + m->rx_undersize + m->rx_crc_errs
        + m->rx_align_errs + m->rx_symbol_errs
        + etc->rxnobuf + etc->rxdmauflo + etc->rxoflo + etc->rxbadlen;
}

static void
chipenablepme(struct bcm4xxx *ch)
{
    bcmenetregs_t *regs;

    regs = ch->regs;

    /* enable chip wakeup pattern matching */
    OR_REG(ch->dev, &regs->devcontrol, DC_PM);

    /* enable sonics bus PME */
    sb_soc_coreflags(ch->sbh, SBTML_PE, SBTML_PE);
}

static void
chipdisablepme(struct bcm4xxx *ch)
{
    bcmenetregs_t *regs;

    regs = ch->regs;

    AND_REG(ch->dev, &regs->devcontrol, ~DC_PM);
    sb_soc_coreflags(ch->sbh, SBTML_PE, 0);
}

static void
chipduplexupd(struct bcm4xxx *ch)
{
    uint32 txcontrol;

    txcontrol = R_REG(ch->dev, &ch->regs->txcontrol);
    if (ch->etc->duplex && !(txcontrol & EXC_FD))
        OR_REG(ch->dev, &ch->regs->txcontrol, EXC_FD);
    else if (!ch->etc->duplex && (txcontrol & EXC_FD))
        AND_REG(ch->dev, &ch->regs->txcontrol, ~EXC_FD);
}

static void
chipdumpmib(ch_t *ch, char *buf)
{
    bcmenetmib_t *m;

    m = &ch->mib;

    buf += sprintf((char *) buf, "tx_good_octets %d tx_good_pkts %d \
        tx_octets %d\n",
        m->tx_good_octets, m->tx_good_pkts, m->tx_octets);
    
    buf += sprintf((char *) buf, "tx_pkts %d tx_broadcast_pkts %d \
        tx_multicast_pkts %d\n",
        m->tx_pkts, m->tx_broadcast_pkts, m->tx_multicast_pkts);

    buf += sprintf((char *) buf, "tx_len_64 %d tx_len_65_to_127 %d \
        tx_len_128_to_255 %d\n",
        m->tx_len_64, m->tx_len_65_to_127, m->tx_len_128_to_255);
    
    buf += sprintf((char *) buf, "tx_len_256_to_511 %d tx_len_512_to_1023 %d \
        tx_len_1024_to_max %d\n",
        m->tx_len_256_to_511, m->tx_len_512_to_1023, m->tx_len_1024_to_max);
    
    buf += sprintf((char *) buf, "tx_jabber_pkts %d tx_oversize_pkts %d \
        tx_fragment_pkts %d\n",
        m->tx_jabber_pkts, m->tx_oversize_pkts, m->tx_fragment_pkts);
    
    buf += sprintf((char *) buf, "tx_underruns %d tx_total_cols %d \
        tx_single_cols %d\n",
        m->tx_underruns, m->tx_total_cols, m->tx_single_cols);
    
    buf += sprintf((char *) buf, "tx_multiple_cols %d tx_excessive_cols %d \
        tx_late_cols %d\n",
        m->tx_multiple_cols, m->tx_excessive_cols, m->tx_late_cols);
    
    buf += sprintf((char *) buf, "tx_defered %d tx_carrier_lost %d \
        tx_pause_pkts %d\n",
        m->tx_defered, m->tx_carrier_lost, m->tx_pause_pkts);

    buf += sprintf((char *) buf, "rx_good_octets %d rx_good_pkts %d \
        rx_octets %d\n",
        m->rx_good_octets, m->rx_good_pkts, m->rx_octets);
        
    buf += sprintf((char *) buf, "rx_broadcast_pkts %d rx_multicast_pkts %d \
        rx_pkts %d\n",
        m->rx_broadcast_pkts, m->rx_multicast_pkts, m->rx_pkts);
    
    buf += sprintf((char *) buf, "rx_len_64 %d rx_len_65_to_127 %d \
        rx_len_128_to_255 %d\n",
        m->rx_len_64, m->rx_len_65_to_127, m->rx_len_128_to_255);
    
    buf += sprintf((char *) buf, "rx_len_256_to_511 %d rx_len_512_to_1023 %d \
        rx_len_1024_to_max %d\n",
        m->rx_len_256_to_511, m->rx_len_512_to_1023, m->rx_len_1024_to_max);
    
    buf += sprintf((char *) buf, "rx_jabber_pkts %d rx_oversize_pkts %d \
        rx_fragment_pkts %d\n",
        m->rx_jabber_pkts, m->rx_oversize_pkts, m->rx_fragment_pkts);
    
    buf += sprintf((char *) buf, "rx_missed_pkts %d rx_crc_align_errs %d \
        rx_undersize %d\n",
        m->rx_missed_pkts, m->rx_crc_align_errs, m->rx_undersize);
    
    buf += sprintf((char *) buf, "rx_crc_errs %d rx_align_errs %d \
        rx_symbol_errs %d\n",
        m->rx_crc_errs, m->rx_align_errs, m->rx_symbol_errs);
    
    buf += sprintf((char *) buf, "rx_pause_pkts %d rx_nonpause_pkts %d \n",
        m->rx_pause_pkts, m->rx_nonpause_pkts);
      
}


#define PHYRD_TIMEOUT 500
static uint16
chipphyrd(struct bcm4xxx *ch, uint phyaddr, uint reg)
{
    bcmenetregs_t *regs;
    uint16 process = 1;
    uint16 chipphyrd_timeout_count = 0;

    ASSERT(reg < 32);

    /*
     * BCM5222 dualphy shared mdio contortion.
     * remote phy: another emac controls our phy.
     */
    if (ch->etc->mdcport != ch->etc->coreunit) {
        if (ch->etphy == NULL) {
            ch->etphy = et_soc_phyfind(ch->et, ch->etc->mdcport);

            /* first time reset */
            if (ch->etphy)
                chipphyreset(ch, ch->etc->phyaddr);
        }
        if (ch->etphy)
            return (et_soc_phyrd(ch->etphy, phyaddr, reg));
        else
            return (0xffff);
    }

    /* local phy: our emac controls our phy */

    regs = ch->regs;

    /* clear mii_int */
    W_REG(ch->dev, &regs->emacintstatus, EI_MII);

    /* issue the read */
    W_REG(ch->dev, &regs->mdiodata,  (MD_SB_START | MD_OP_READ | 
    (phyaddr << MD_PMD_SHIFT) | (reg << MD_RA_SHIFT) | MD_TA_VALID));

    /* wait for it to complete */
    while (process) {
        if ((R_REG(ch->dev, &regs->emacintstatus) & EI_MII) != 0) {
            process = 0;
        }
        chipphyrd_timeout_count ++;
        
        if (chipphyrd_timeout_count > PHYRD_TIMEOUT) {
            break;
        }
    }
    if ((R_REG(ch->dev, &regs->emacintstatus) & EI_MII) == 0) {
        ET_ERROR(("et%d: chipphyrd: did not complete\n", ch->etc->unit));
    }

    return (R_REG(ch->dev, &regs->mdiodata) & MD_DATA_MASK);
}

static void
chipphywr(struct bcm4xxx *ch, uint phyaddr, uint reg, uint16 v)
{
    bcmenetregs_t *regs;
    uint16 process = 1;
    uint16 chipphyrd_timeout_count = 0;

    ASSERT(reg < 32);

    /*
     * BCM5222 dualphy shared mdio contortion.
     * remote phy: another emac controls our phy.
     */
    if (ch->etc->mdcport != ch->etc->coreunit) {
        if (ch->etphy == NULL)
            ch->etphy = et_soc_phyfind(ch->et, ch->etc->mdcport);
        if (ch->etphy)
            et_soc_phywr(ch->etphy, phyaddr, reg, v);
        return;
    }

    /* local phy: our emac controls our phy */

    regs = ch->regs;

    /* clear mii_int */
    W_REG(ch->dev, &regs->emacintstatus, EI_MII);


    /* issue the write */
    W_REG(ch->dev, &regs->mdiodata,  (MD_SB_START | MD_OP_WRITE 
    | (phyaddr << MD_PMD_SHIFT) | (reg << MD_RA_SHIFT) | MD_TA_VALID | v));

    /* wait for it to complete */
    while (process) {
        if ((R_REG(ch->dev, &regs->emacintstatus) & EI_MII) != 0) {
            process = 0;
        }
        chipphyrd_timeout_count ++;
        
        if (chipphyrd_timeout_count > PHYRD_TIMEOUT) {
            break;
        }
    }
    if ((R_REG(ch->dev, &regs->emacintstatus) & EI_MII) == 0) {
        ET_ERROR(("et%d: chipphywr: did not complete\n", ch->etc->unit));
    }
}

static void
chipphyor(struct bcm4xxx *ch, uint phyaddr, uint reg, uint16 v)
{
    uint16 tmp;

    tmp = chipphyrd(ch, phyaddr, reg);
    tmp |= v;
    chipphywr(ch, phyaddr, reg, tmp);
}

static void
chipphyand(struct bcm4xxx *ch, uint phyaddr, uint reg, uint16 v)
{
    uint16 tmp;

    tmp = chipphyrd(ch, phyaddr, reg);
    tmp &= v;
    chipphywr(ch, phyaddr, reg, tmp);
}

static void
chipphyreset(struct bcm4xxx *ch, uint phyaddr)
{
    if (phyaddr == PHY_NOMDC) return;

    chipphywr(ch, phyaddr, 0, CTL_RESET);
    OSL_DELAY(100);
    if (chipphyrd(ch, phyaddr, 0) & CTL_RESET) {
        ET_ERROR(("et%d: chipphyreset: reset not complete\n", ch->etc->unit));
    }

    chipphyinit(ch, phyaddr);
}

static void
chipphyinit(struct bcm4xxx *ch, uint phyaddr)
{
    uint    phyid = 0;

    /* enable activity led */
    chipphyand(ch, phyaddr, 26, 0x7fff);

    /* enable traffic meter led mode */
    chipphyor(ch, phyaddr, 27, (1 << 6));

    /* PR 10115 detecting MELCO specific link pulse. */  
    phyid = chipphyrd( ch, phyaddr, 0x2);
    phyid |=  chipphyrd( ch, phyaddr, 0x3) << 16;
    if( phyid == 0x55210022) {
        chipphywr( ch, phyaddr, 30, 
            (uint16) (chipphyrd( ch, phyaddr, 30 ) | 0x3000));
        chipphywr( ch, phyaddr, 22, 
            (uint16) (chipphyrd( ch, phyaddr, 22 ) & 0xffdf));
    }
}

static void
chipphyforce(struct bcm4xxx *ch, uint phyaddr)
{
    etc_soc_info_t *etc;
    uint16 ctl;

    if (phyaddr == PHY_NOMDC) return;

    etc = ch->etc;

    if (etc->forcespeed == ET_AUTO)
        return;

    ctl = chipphyrd(ch, phyaddr, 0);
    ctl &= ~(CTL_SPEED | CTL_ANENAB | CTL_DUPLEX);

    switch (etc->forcespeed) {
    case ET_10HALF:
        break;

    case ET_10FULL:
        ctl |= CTL_DUPLEX;
        break;

    case ET_100HALF:
        ctl |= CTL_SPEED;
        break;

    case ET_100FULL:
        ctl |= (CTL_SPEED | CTL_DUPLEX);
        break;
    }

    chipphywr(ch, phyaddr, 0, ctl);
}

/* set selected capability bits in autonegotiation advertisement */
static void
chipphyadvertise(struct bcm4xxx *ch, uint phyaddr)
{
    etc_soc_info_t *etc;
    uint16 adv;

    if (phyaddr == PHY_NOMDC) return;

    etc = ch->etc;

    if ((etc->forcespeed != ET_AUTO) || !etc->needautoneg)
        return;

    ASSERT(etc->advertise);

    /* reset our advertised capabilitity bits */
    adv = chipphyrd(ch, phyaddr, 4);
    adv &= ~(ADV_100FULL | ADV_100HALF | ADV_10FULL | ADV_10HALF);
    adv |= etc->advertise;
    chipphywr(ch, phyaddr, 4, adv);

    /* restart autonegotiation */
    chipphyor(ch, phyaddr, 0, CTL_RESTART);

    etc->needautoneg = FALSE;
}

static void 
chiprxreset(struct bcm4xxx *ch, int id)
{
    dma_soc_rxreset(ch->di);
    dma_soc_rxreclaim(ch->di);
}

static void 
chiprxinit(struct bcm4xxx *ch, int id)
{
    if(!dma_soc_rxenabled(ch->di)) {
        dma_soc_rxinit(ch->di);
    }
}

static bool 
chiprecover(struct bcm4xxx *ch)
{
    /*
     * Currently, only RX fifo overflow can be recovered.
     */
    if ((R_REG(ch->dev, &ch->regs->intstatus) & I_ERRORS) == I_RO) {
        
        /* Disable EMAC by clearing EMACEnable in EnetControl */
        OR_REG(ch->dev, &ch->regs->enetcontrol, EC_ED);
        SPINWAIT((R_REG(ch->dev, &ch->regs->enetcontrol) & EC_ED), 100);
        
        /* Reset RX in DMA engine */
        dma_soc_rxreset(ch->di);
        dma_soc_rxenable(ch->di);
        
        /* Recycle the unused descriptors */
        dma_soc_rxrecycle(ch->di);
        
        /* Enable EMAC by setting EMACEnable in EnetControl */
        OR_REG(ch->dev, &ch->regs->enetcontrol, EC_EE);

        return TRUE;
    }
    
    return FALSE;
}


static int 
chipcfprd(ch_t *ch, void *buf)
{
    return FALSE;
}

static int 
chipcfpwr(ch_t *ch, void *buf)
{
    return FALSE;
}

static int 
chipcfpfldrd(ch_t *ch, void *buf)
{
    return FALSE;
}
static int 
chipcfpfldwr(ch_t *ch, void *buf)
{
    return FALSE;
}

static int 
chipcfpudfrd(ch_t *ch, void *buf)
{
    return FALSE;
}

static int 
chipcfpudfwr(ch_t *ch, void *buf)
{
    return FALSE;
}

static int 
chiprxrateset(ch_t *ch, uint channel, uint pps)
{
    return FALSE;
}

static int 
chiprxrateget(ch_t *ch, uint channel, uint *pps)
{
    return FALSE;
}

static int 
chiptxrateset(ch_t *ch, uint channel, uint rate, uint burst)
{
    return FALSE;
}

static int 
chiptxrateget(ch_t *ch, uint channel, uint *rate, uint *burst)
{
    return FALSE;
}

static int 
chipflowctrlmodeset(ch_t *ch, uint mode)
{
    return FALSE;
}

static int 
chipflowctrlmodeget(ch_t *ch, uint *mode)
{
    return FALSE;
}

static int 
chipflowctrlautoset(ch_t *ch, uint on_thresh, uint off_thresh)
{
    return FALSE;
}

static int 
chipflowctrlautoget(ch_t *ch, uint *on_thresh, uint *off_thresh)
{
    return FALSE;
}

static int 
chipflowctrlcpuset(ch_t *ch, uint pause_on)
{
    return FALSE;
}

static int 
chipflowctrlcpuget(ch_t *ch, uint *pause_on)
{
    return FALSE;
}

static int 
chipflowctrlrxchanset(ch_t *ch, uint rxchan, uint on_thresh, uint off_thresh)
{
    return FALSE;
}

static int 
chipflowctrlrxchanget(ch_t *ch, uint rxchan, uint *on_thresh, uint *off_thresh)
{
    return FALSE;
}

static int 
chiptpidset(ch_t *ch, uint index, uint tpid)
{
    return FALSE;
}

static int 
chiptpidget(ch_t *ch, uint index, uint *tpid)
{
    return FALSE;
}

static int 
chippvtagset(ch_t *ch, uint private_tag)
{
    return FALSE;
}

static int 
chippvtagget(ch_t *ch, uint *private_tag)
{
    return FALSE;
}

static int 
chiprxsephdrset(ch_t *ch, uint enable)
{
    return FALSE;
}

static int 
chiprxsephdrget(ch_t *ch, uint *enable)
{
    return FALSE;
}

static int 
chiptxqosmodeset(ch_t *ch, uint mode)
{
    return FALSE;
}

static int 
chiptxqosmodeget(ch_t *ch, uint *mode)
{
    return FALSE;
}

static int 
chiptxqosweightset(ch_t *ch, uint queue, uint weight)
{
    return FALSE;
}

static int 
chiptxqosweightget(ch_t *ch, uint queue, uint *weight)
{
    return FALSE;
}

#endif /* defined(ROBO_4704) */
