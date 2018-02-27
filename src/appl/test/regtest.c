/*
 * $Id: regtest.c,v 1.53.2.3 2011/06/03 18:53:15 tusharty Exp $
 * $Copyright: (c) 2005 Broadcom Corp.
 * All Rights Reserved.$
 *
 * Register Tests
 */

#include <sal/types.h>

#include <soc/debug.h>

#include <bcm/link.h>

#include <appl/diag/system.h>
#include <appl/diag/parse.h>
#include <appl/diag/test.h>
#include <bcm_int/control.h>
#ifdef BCM_TRIUMPH2_SUPPORT
#include <soc/triumph2.h>
#endif
#ifdef BCM_KATANA_SUPPORT
#include <soc/katana.h>
#endif
#ifdef BCM_SIRIUS_SUPPORT
#include <soc/sbx/sirius.h>
#endif

#if defined (BCM_ESW_SUPPORT) || defined (BCM_SIRIUS_SUPPORT)
/*
 * this is a special marker that is used in soc_reg_iterate()
 * to indicate that no more variations of the current register
 * should be iterated over.
 */
#define SOC_E_IGNORE -6000

struct reg_data {
    int unit;
    int error;
    int flags;
};

#define	REGTEST_FLAG_MINIMAL	0x0001
#define	REGTEST_FLAG_MASK64	0x0002	/* FIXME: this should go away */

/*
 * reg_test
 *
 * Read/write/addressing tests of all SOC internal register R/W bits
 */
STATIC int
try_reg_value(struct reg_data *rd,
	      soc_regaddrinfo_t *ainfo,
	      char *regname,
	      uint32 pattern,
	      uint64 mask)
{
    uint64		pat64, rd64, wr64, rrd64, notmask;
    char		wr_str[20], mask_str[20], pat_str[20], rrd_str[20];
    int			r;

    /* skip 64b registers in sim */
    if (SAL_BOOT_PLISIM) {
      if (SOC_REG_IS_64(rd->unit,ainfo->reg)) {
	debugk(DK_WARN,"Skipping 64 bit %s register in sim\n",regname);
	return 0;
      }
    }

    if ((r = soc_anyreg_read(rd->unit, ainfo, &rd64)) < 0) {
	debugk(DK_ERR, "ERROR: read reg %s failed: %s\n",
	       regname, soc_errmsg(r));
	return -1;
    }

    COMPILER_64_SET(pat64, pattern, pattern);
    COMPILER_64_AND(pat64, mask);

    notmask = mask;
    COMPILER_64_NOT(notmask);

    wr64 = rd64;
    COMPILER_64_AND(wr64, notmask);
    COMPILER_64_OR(wr64, pat64);

    format_uint64(wr_str, wr64);
    format_uint64(mask_str, mask);

    debugk(DK_TESTS, "Write %s: value %s mask %s\n",
	   regname, wr_str, mask_str);

    if ((r = soc_anyreg_write(rd->unit, ainfo, wr64)) < 0) {
	debugk(DK_ERR, "ERROR: write reg %s failed: %s wrote %s (mask %s)\n",
	       regname, soc_errmsg(r), wr_str, mask_str);
        rd->error = r;
	return -1;
    }

    if ((r = soc_anyreg_read(rd->unit, ainfo, &rrd64)) < 0) {
	debugk(DK_ERR, "ERROR: reread reg %s failed: %s after wrote %s (mask %s)\n",
	       regname, soc_errmsg(r), wr_str, mask_str);
        rd->error = r;
	return -1;
    }

    COMPILER_64_AND(rrd64, mask);
    format_uint64(rrd_str, rrd64);
    format_uint64(pat_str, pat64);

    debugk(DK_TESTS, "Read  %s: value %s expecting %s\n",
	   regname, rrd_str, pat_str);

    if (COMPILER_64_NE(rrd64, pat64)) {
 	debugk(DK_ERR, "ERROR %s: wrote %s read %s (mask %s)\n",
	       regname, pat_str, rrd_str, mask_str);
	rd->error = SOC_E_FAIL;
    }

    /* put the register back the way we found it */
    if ((r = soc_anyreg_write(rd->unit, ainfo, rd64)) < 0) {
	debugk(DK_ERR, "ERROR: rewrite reg %s failed: %s\n",
	       regname, soc_errmsg(r));
        rd->error = r;
	return -1;
    }

    return 0;
}

/*
 * Test a register
 * If reg_data.flag can control a minimal test
 */
STATIC int
try_reg(int unit, soc_regaddrinfo_t *ainfo, void *data)
{
    struct reg_data	*rd = data;
    uint64		mask, mask2, mask3;
    char		regname[80];
#ifdef BCM_HAWKEYE_SUPPORT
    uint32              miscconfig;
    int                 meter = 0;
#endif

    if (!SOC_REG_IS_VALID(unit, ainfo->reg)) {
        return SOC_E_IGNORE;		/* invalid register */
    }

    if (SOC_REG_INFO(unit, ainfo->reg).flags &
        (SOC_REG_FLAG_RO|SOC_REG_FLAG_WO)) {
        return SOC_E_IGNORE;		/* no testable bits */
    }

    if (SOC_REG_INFO(unit, ainfo->reg).regtype == soc_portreg &&
	!SOC_PORT_VALID(unit, ainfo->port)) {
	return 0;			/* skip invalid ports */
    }

#ifdef BCM_ENDURO_SUPPORT
    if (SOC_IS_ENDURO(unit) || SOC_IS_HURRICANE(unit) || SOC_IS_KATANA(unit)) {
        if (ainfo->reg == OAM_SEC_NS_COUNTER_64r) {
            debugk(DK_WARN,"Skipping OAM_SEC_NS_COUNTER_64 register\n");
            return 0;               /* skip OAM_SEC_NS_COUNTER_64 register */
        }
    }
#endif    
#ifdef BCM_KATANA_SUPPORT
    if (SOC_IS_KATANA(unit)) {
        if ( !soc_feature(unit,soc_feature_ces)  && (SOC_REG_BLOCK_IS(unit, ainfo->reg, SOC_BLK_CES)) ) {
            return 0;
        }
        if ( !soc_feature(unit,soc_feature_ddr3)  && (SOC_REG_BLOCK_IS(unit, ainfo->reg, SOC_BLK_CI)) ) {
            return 0;
        }
    }
#endif

    /*
     * set mask to read-write bits fields
     * (those that are not marked untestable, reserved, read-only,
     * or write-only)
     */
    if (rd->flags & REGTEST_FLAG_MASK64) {
	mask = soc_reg64_datamask(unit, ainfo->reg, 0);
	mask2 = soc_reg64_datamask(unit, ainfo->reg, SOCF_RES);
	mask3 = soc_reg64_datamask(unit, ainfo->reg, SOCF_RO);
	COMPILER_64_OR(mask2, mask3);
	mask3 = soc_reg64_datamask(unit, ainfo->reg, SOCF_WO);
	COMPILER_64_OR(mask2, mask3);
	COMPILER_64_NOT(mask2);
	COMPILER_64_AND(mask, mask2);
    } else {
	/* FIXME: nsx compiler bug workaround by using volatile */
	volatile uint32	m32;

	m32 = soc_reg_datamask(unit, ainfo->reg, 0);
	m32 &= ~soc_reg_datamask(unit, ainfo->reg, SOCF_RES);
	m32 &= ~soc_reg_datamask(unit, ainfo->reg, SOCF_RO);
	m32 &= ~soc_reg_datamask(unit, ainfo->reg, SOCF_WO);
	m32 &= ~soc_reg_datamask(unit, ainfo->reg, SOCF_W1TC);
	m32 &= ~soc_reg_datamask(unit, ainfo->reg, SOCF_COR);
#ifdef BCM_SIRIUS_SUPPORT
	m32 &= ~soc_reg_datamask(unit, ainfo->reg, SOCF_PUNCH);
	m32 &= ~soc_reg_datamask(unit, ainfo->reg, SOCF_WVTC);
	m32 &= ~soc_reg_datamask(unit, ainfo->reg, SOCF_RWBW);
#endif
	COMPILER_64_SET(mask, 0, m32);
    }

#ifdef BCM_HAWKEYE_SUPPORT
    if (SOC_IS_HAWKEYE(unit) && soc_feature(unit, soc_feature_eee)) {
        if (ainfo->reg == EEE_DELAY_ENTRY_TIMERr) {
        /* The register r/w test should skip bit20 for register 
         *     EEE_DELAY_ENTRY_TIMER.
         * Because bit20 is a constant value before H/W one second delay 
         *     for EEE feature.
         */
            COMPILER_64_SET(mask2, 0, ~0x100000);
            COMPILER_64_AND(mask, mask2);
        }
    }
#endif /* BCM_HAWKEYE_SUPPORT */

    if (COMPILER_64_IS_ZERO(mask)) {
	return SOC_E_IGNORE;		/* no testable bits */
    }

    /*
     * Check if this register is actually implemented in HW for the
     * specified port/cos. If so, the mask is adjusted for the
     * specified port/cos based on what is acutually in HW.
     */
    if (reg_mask_subset(unit, ainfo, &mask)) {
        /* Skip this register. Returning SOC_E_NONE, instead of 
         * SOC_E_IGNORE since we may not want to skip this register
         * all the time (only for certain ports/cos)
         */
        return SOC_E_NONE;
    }

    soc_reg_sprint_addr(unit,  regname, ainfo);

#ifdef BCM_HAWKEYE_SUPPORT
    if (SOC_IS_HAWKEYE(unit) && (ainfo->reg != MISCCONFIGr)) {
        if (READ_MISCCONFIGr(unit, &miscconfig) < 0) {
            test_error(unit, "Miscconfig read failed\n");
             return SOC_E_IGNORE;
        }
        
        meter = soc_reg_field_get(unit, MISCCONFIGr, 
                              miscconfig, METERING_CLK_ENf);
    
        if(meter){
            soc_reg_field_set(unit, MISCCONFIGr, 
                              &miscconfig, METERING_CLK_ENf, 0);
    
            if (WRITE_MISCCONFIGr(unit, miscconfig) < 0) {
                test_error(unit, "Miscconfig setting failed\n");
                 return SOC_E_IGNORE;
            }
        }
    } 
#endif

    /*
     * minimal test
     * just Fs and 5s
     * only do first instance of each register
     * (only first port, cos, array index, and/or block)
     */
    if (rd->flags & REGTEST_FLAG_MINIMAL) {
	if (try_reg_value(rd, ainfo, regname, 0xffffffff, mask) < 0) {
	    return SOC_E_IGNORE;
	}

	if (try_reg_value(rd, ainfo, regname, 0x55555555, mask) < 0) {
	    return SOC_E_IGNORE;
	}
	return SOC_E_IGNORE;	/* skip other than first instance */
    }

    /*
     * full test
     */
    if (try_reg_value(rd, ainfo, regname, 0x00000000, mask) < 0) {
	return SOC_E_IGNORE;
    }

    if (try_reg_value(rd, ainfo, regname, 0xffffffff, mask) < 0) {
	return SOC_E_IGNORE;
    }

    if (try_reg_value(rd, ainfo, regname, 0x55555555, mask) < 0) {
	return SOC_E_IGNORE;
    }

    if (try_reg_value(rd, ainfo, regname, 0xaaaaaaaa, mask) < 0) {
	return SOC_E_IGNORE;
    }

#ifdef BCM_HAWKEYE_SUPPORT
     if (SOC_IS_HAWKEYE(unit) && (ainfo->reg != MISCCONFIGr)) {
        if (WRITE_MISCCONFIGr(unit, miscconfig) < 0) {
            test_error(unit, "Miscconfig setting failed\n");
            return SOC_E_IGNORE;
        }
    }
#endif

    return 0;
}

/*
 * Register read/write test (tr 3)
 */
int
reg_test(int unit, args_t *a, void *pa)
{
    struct reg_data	rd;
    int			r, rv = 0;
    char		*s;
	uint32		tem;

    COMPILER_REFERENCE(pa);

    debugk(DK_TESTS, "Register read/write test\n");

    rd.unit = unit;
    rd.error = SOC_E_NONE;
    rd.flags = 0;
    while ((s = ARG_GET(a)) != NULL) {
	if (sal_strcasecmp(s, "mini") == 0 ||
	    sal_strcasecmp(s, "minimal") == 0) {
	    rd.flags |= REGTEST_FLAG_MINIMAL;
	    continue;
	}
	if (sal_strcasecmp(s, "mask64") == 0 ||
	    sal_strcasecmp(s, "datamask64") == 0) {
	    rd.flags |= REGTEST_FLAG_MASK64;
	    continue;
	}
	debugk(DK_WARN, "WARNING: unknown argument '%s' ignored\n", s);
    }

    if (!SOC_UNIT_VALID(unit)) {
        return SOC_E_UNIT;
    }
    if (BCM_UNIT_VALID(unit)) {
	BCM_IF_ERROR_RETURN(bcm_linkscan_enable_set(unit, 0)); /* disable linkscan */
    }

#ifdef BCM_SIRIUS_SUPPORT
    if (SOC_IS_SIRIUS(unit)) {
	if ((r = soc_sirius_reset(unit)) < 0) {
	    debugk(DK_ERR, "ERROR: Unable to reset unit %d: %s\n",
		   unit, soc_errmsg(r));
	    goto done;
	}
    }
    else
#endif
    {
	if ((r = soc_reset_init(unit)) < 0) {
	    debugk(DK_ERR, "ERROR: Unable to reset unit %d: %s\n",
		   unit, soc_errmsg(r));
	    goto done;
	}
    }
    
    if (SOC_IS_HB_GW(unit) && soc_feature(unit, soc_feature_bigmac_rxcnt_bug)) {
        /*
         * We need to wait for auto-negotiation to complete to ensure
         * that the BigMAC Rx counters will respond correctly.
         */
        sal_usleep(500000);
    }

 	/* When doing E2EFC register/memory read/write tests, it is strongly recommended to set this parity genrate 
	 *  enable bit to 0. This will prevent hardware automatically overwritting the ECC and
	 *  parity field in E2EFC registers/memories contents and failing the tests.
	 */
    if (soc_reg_field_valid(unit, MISCCONFIGr, E2EFC_PARITY_GEN_ENf)) {
		SOC_IF_ERROR_RETURN(READ_MISCCONFIGr(unit, &tem));
		soc_reg_field_set(unit, MISCCONFIGr, &tem, E2EFC_PARITY_GEN_ENf, 0);
		SOC_IF_ERROR_RETURN(WRITE_MISCCONFIGr(unit, tem));
    }
 
    /*
     * If try_reg returns -1, there was a register access failure rather
     * than a bit error.
     *
     * If try_reg returns 0, then rd.error is -1 if there was a bit
     * error.
     */
#ifdef BCM_SHADOW_SUPPORT
    if (SOC_IS_SHADOW(unit)) {
        rv = (soc_reg_field32_modify(unit, IARB_REGS_DEBUGr, REG_PORT_ANY,
                                    DEBUGf, 1));
        rv = (soc_reg_field32_modify(unit, IPARS_REGS_DEBUGr, REG_PORT_ANY,
                                    DEBUGf, 1));
        rv = (soc_reg_field32_modify(unit, IVLAN_REGS_DEBUGr, REG_PORT_ANY,
                                    DEBUGf, 1));
        rv = (soc_reg_field32_modify(unit, ISW1_REGS_DEBUGr, REG_PORT_ANY,
                                    DEBUGf, 1));
        rv = (soc_reg_field32_modify(unit, ISW2_REGS_DEBUGr, REG_PORT_ANY,
                                    DEBUGf, 1));
        rv = (soc_reg_field32_modify(unit, IL_DEBUG_CONFIGr, 9,
                                    IL_TREX2_DEBUG_LOCKf, 1));
        rv = (soc_reg_field32_modify(unit, IL_DEBUG_CONFIGr, 13,
                                    IL_TREX2_DEBUG_LOCKf, 1));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    WRED_THD_1_ENABLE_ECCf, 0));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    WRED_THD_0_ENABLE_ECCf, 0));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    WRED_PORT_THD_0_ENABLE_ECCf, 0));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    WRED_PORT_THD_1_ENABLE_ECCf, 0));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    WRED_CFG_ENABLE_ECCf, 0));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    WRED_PORT_CFG_ENABLE_ECCf, 0));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    MTRO_SHAPE_G0_BUCKET_ENABLE_ECCf, 0));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    MTRO_SHAPE_G0_CONFIG_ENABLE_ECCf, 0));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    MTRO_SHAPE_G1_BUCKET_ENABLE_ECCf, 0));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    MTRO_SHAPE_G1_CONFIG_ENABLE_ECCf, 0));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    MTRO_SHAPE_G2_BUCKET_ENABLE_ECCf, 0));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    MTRO_SHAPE_G2_CONFIG_ENABLE_ECCf, 0));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    MTRO_SHAPE_G3_BUCKET_ENABLE_ECCf, 0));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    MTRO_SHAPE_G3_CONFIG_ENABLE_ECCf, 0));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    MTRO_CPU_PKT_ENABLE_ECCf, 0));
    }
#endif

    if (soc_reg_iterate(unit, try_reg, &rd) < 0) {
	    debugk(DK_TESTS, "Continuing test.\n");
	    rv = 0;
    } else {
	    rv = rd.error;
    }

	/* Re-Enable Parity Gen */
    if (soc_reg_field_valid(unit, MISCCONFIGr, E2EFC_PARITY_GEN_ENf)) {
		SOC_IF_ERROR_RETURN(READ_MISCCONFIGr(unit, &tem));
		soc_reg_field_set(unit, MISCCONFIGr, &tem, E2EFC_PARITY_GEN_ENf, 1);
		SOC_IF_ERROR_RETURN(WRITE_MISCCONFIGr(unit, tem));
    }

done:
#ifdef BCM_SHADOW_SUPPORT
    if (SOC_IS_SHADOW(unit)) {
        rv = (soc_reg_field32_modify(unit, IARB_REGS_DEBUGr, REG_PORT_ANY,
                                    DEBUGf, 0));
        rv = (soc_reg_field32_modify(unit, IPARS_REGS_DEBUGr, REG_PORT_ANY,
                                    DEBUGf, 0));
        rv = (soc_reg_field32_modify(unit, IVLAN_REGS_DEBUGr, REG_PORT_ANY,
                                    DEBUGf, 0));
        rv = (soc_reg_field32_modify(unit, ISW1_REGS_DEBUGr, REG_PORT_ANY,
                                    DEBUGf, 0));
        rv = (soc_reg_field32_modify(unit, ISW2_REGS_DEBUGr, REG_PORT_ANY,
                                    DEBUGf, 0));
        rv = (soc_reg_field32_modify(unit, IL_DEBUG_CONFIGr, 9,
                                    IL_TREX2_DEBUG_LOCKf, 0));
        rv = (soc_reg_field32_modify(unit, IL_DEBUG_CONFIGr, 13,
                                    IL_TREX2_DEBUG_LOCKf, 0));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    WRED_THD_1_ENABLE_ECCf, 1));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    WRED_THD_0_ENABLE_ECCf, 1));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    WRED_PORT_THD_0_ENABLE_ECCf, 1));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    WRED_PORT_THD_1_ENABLE_ECCf, 1));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    WRED_CFG_ENABLE_ECCf, 1));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    WRED_PORT_CFG_ENABLE_ECCf, 1));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    MTRO_SHAPE_G0_BUCKET_ENABLE_ECCf, 1));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    MTRO_SHAPE_G0_CONFIG_ENABLE_ECCf, 1));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    MTRO_SHAPE_G1_BUCKET_ENABLE_ECCf, 1));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    MTRO_SHAPE_G1_CONFIG_ENABLE_ECCf, 1));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    MTRO_SHAPE_G2_BUCKET_ENABLE_ECCf, 1));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    MTRO_SHAPE_G2_CONFIG_ENABLE_ECCf, 1));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    MTRO_SHAPE_G3_BUCKET_ENABLE_ECCf, 1));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    MTRO_SHAPE_G3_CONFIG_ENABLE_ECCf, 1));
        SOC_IF_ERROR_RETURN
            (soc_reg_field32_modify(unit, MMU_ECC_DEBUG1r, REG_PORT_ANY,
                                    MTRO_CPU_PKT_ENABLE_ECCf, 1));
    }
#endif

    if (rv < 0) {
	test_error(unit, "Register read/write test failed\n");
    }

    return rv;
}

/*
 * rval_test
 *
 * Reset SOC and compare reset values of all SOC registers with regsfile
 */

STATIC int
rval_test_proc(int unit, soc_regaddrinfo_t *ainfo, void *data)
{
    struct reg_data	*rd = data;
    char		buf[80];
    uint64		value, rmsk, rval, chk;
    char                val_str[20], rmsk_str[20], rval_str[20];
    int			r;

    SOC_REG_RST_MSK_GET(unit, ainfo->reg, rmsk);

    if (rval_test_skip_reg(unit, ainfo)) {
        soc_reg_sprint_addr(unit, buf, ainfo);
        debugk(DK_WARN,"Skipping register %s\n", buf);
        return 0;
    }

#ifdef BCM_ENDURO_SUPPORT
    if (SOC_IS_ENDURO(unit) || SOC_IS_HURRICANE(unit)) {
        if (ainfo->reg == OAM_SEC_NS_COUNTER_64r) {
            debugk(DK_WARN,"Skipping OAM_SEC_NS_COUNTER_64 register\n");
            return 0;                /* skip OAM_SEC_NS_COUNTER_64 register */
        }
    }
#endif    

#ifdef BCM_HAWKEYE_SUPPORT
    if (SOC_IS_HAWKEYE(unit)) {
        if (ainfo->reg == MAC_MODEr) {
            debugk(DK_WARN,"Skipping MAC_MODE register\n");
            return 0;                /* skip MAC_MODE register */
        }
    }
#endif    
#ifdef BCM_KATANA_SUPPORT
        if (SOC_IS_KATANA(unit)) {
            if ( !soc_feature(unit,soc_feature_ces)  && (SOC_REG_BLOCK_IS(unit, ainfo->reg, SOC_BLK_CES)) ) {
                return 0;
            }
            if ( !soc_feature(unit,soc_feature_ddr3)  && (SOC_REG_BLOCK_IS(unit, ainfo->reg, SOC_BLK_CI)) ) {
                return 0;
            }
        }
#endif    
    
    if (SAL_BOOT_PLISIM) {
      if (SOC_REG_IS_64(unit, ainfo->reg)) {
	soc_reg_sprint_addr(unit,buf,ainfo);
	debugk(DK_WARN,"Skipping 64 bit %s register in sim\n",buf);
	return 0;
      }
    }

    /*
     * Check if this register is actually implemented in HW for the
     * specified port/cos. If so, the mask is adjusted for the
     * specified port/cos based on what is acutually in HW.
     */
    if (reg_mask_subset(unit, ainfo, &rmsk)) {
        return 0;
    }

    if (COMPILER_64_IS_ZERO(rmsk)) {
	return 0;		/* No reset value */
    }

    SOC_REG_RST_VAL_GET(unit, ainfo->reg, rval);
    soc_reg_sprint_addr(unit, buf, ainfo);

    if ((r = soc_anyreg_read(rd->unit, ainfo, &value)) < 0) {
	debugk(DK_ERR, "ERROR: read reg %s (0x%x) failed: %s\n",
	       buf, ainfo->addr, soc_errmsg(r));
        rd->error = r;
	return -1;
    }

    format_uint64(val_str, value);
    format_uint64(rmsk_str, rmsk);
    format_uint64(rval_str, rval);
    debugk(DK_TESTS, "Read %s: reset mask %s, reset value %s, read %s\n",
	   buf, rmsk_str, rval_str, val_str);

    /* Check reset value is correct */
    COMPILER_64_ZERO(chk);
    COMPILER_64_ADD_64(chk, value);
    COMPILER_64_XOR(chk, rval);
    COMPILER_64_AND(chk, rmsk);

    if (!COMPILER_64_IS_ZERO(chk)) {
        COMPILER_64_AND(rval, rmsk);
        format_uint64(rval_str, rval);
        COMPILER_64_AND(value, rmsk);
        format_uint64(val_str, value);
	debugk(DK_ERR, "ERROR: %s: expected %s, got %s, reset mask %s\n",
	       buf, rval_str, val_str, rmsk_str);
	rd->error = SOC_E_FAIL;
    }

    return 0;
}

/*
 * Register reset value test (tr 1)
 */
int
rval_test(int unit, args_t *a, void *pa)
{
    struct reg_data	rd;
    int			r, rv = -1;
    char		*s;

    COMPILER_REFERENCE(pa);

    debugk(DK_TESTS, "Register reset value test\n");

    rd.unit = unit;
    rd.error = SOC_E_NONE;
    rd.flags = 0;
    while ((s = ARG_GET(a)) != NULL) {
	debugk(DK_WARN, "WARNING: unknown argument '%s' ignored\n", s);
    }

    if (!SOC_UNIT_VALID(unit)) {
        return SOC_E_UNIT;
    }
    if (BCM_UNIT_VALID(unit)) {
	BCM_IF_ERROR_RETURN(bcm_linkscan_enable_set(unit, 0)); /* disable linkscan */
    }

#ifdef BCM_TRIUMPH2_SUPPORT
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) ||
        SOC_IS_VALKYRIE2(unit)) {
        soc_triumph2_pipe_mem_clear(unit);
    }
#endif /* BCM_TRIUMPH2_SUPPORT */

#ifdef BCM_SIRIUS_SUPPORT
    if (SOC_IS_SIRIUS(unit)) {
	if ((r = soc_sirius_reset(unit)) < 0) {
	    debugk(DK_ERR, "ERROR: Unable to reset unit %d: %s\n",
		   unit, soc_errmsg(r));
	    goto done;
	}
    }
    else
#endif
    {
	if ((r = soc_reset_init(unit)) < 0) {
	    debugk(DK_ERR, "ERROR: Unable to reset unit %d: %s\n",
		   unit, soc_errmsg(r));
	    goto done;
	}
    }

#ifdef BCM_TRIUMPH2_SUPPORT
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) 
        || SOC_IS_VALKYRIE2(unit) || SOC_IS_ENDURO(unit)
        || SOC_IS_HURRICANE(unit)) {
        /* reset the register XQPORT_XGXS_NEWCTL_REG to 0x0 */
        soc_port_t port;
        PBMP_PORT_ITER(unit, port) {
            switch(port) {
            case 26:
            case 27:
            case 28:
            case 29:
                if(SOC_IS_ENDURO(unit) || SOC_IS_HURRICANE(unit)) {
                    SOC_IF_ERROR_RETURN(WRITE_XQPORT_XGXS_NEWCTL_REGr(
                                                       unit, port, 0x0));
                } else {
                    SOC_IF_ERROR_RETURN(WRITE_XPORT_XGXS_NEWCTL_REGr(
                                                       unit, port, 0x0));
                }
                break;
            case 30:
            case 34:
            case 38:
            case 42:
            case 46:
            case 50:
                SOC_IF_ERROR_RETURN(WRITE_XQPORT_XGXS_NEWCTL_REGr(unit, port, 0x0));
                break;
            default:
                break;
            }
        }
    }
#endif

    sal_usleep(10000);

    if (soc_reg_iterate(unit, rval_test_proc, &rd) < 0) {
	goto done;
    }

    rv = rd.error;

 done:
    if (rv < 0) {
	test_error(unit, "Register reset value test failed\n");
    }

#ifdef BCM_SIRIUS_SUPPORT
    if (SOC_IS_SIRIUS(unit)) {
	if ((r = soc_sirius_reset(unit)) < 0) {
	    debugk(DK_ERR, "ERROR: Unable to reset unit %d: %s\n",
		   unit, soc_errmsg(r));
	}
    }
    else
#endif
    {
	if ((r = soc_reset_init(unit)) < 0) {
	    debugk(DK_ERR, "ERROR: Unable to reset unit %d: %s\n",
		   unit, soc_errmsg(r));
	}
    }

    return rv;
}

#endif /* BCM_ESW_SUPPORT || BCM_SIRIUS_SUPPORT */
