/*
 * $Id: rate.c 1.53.6.5 Broadcom SDK $
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
 * Rate - Broadcom StrataSwitch Rate Limiting API.
 */

#include <soc/drv.h>

#include <soc/mem.h>
#include <soc/register.h>
#include <soc/memory.h>
#include <soc/drv.h>

#include <bcm/rate.h>
#include <bcm/error.h>

#include <bcm_int/esw_dispatch.h> 

#include <bcm_int/esw/rate.h> 

#define TOKEN_BUCKET_SIZE       64      /* Size of one token bucket in kbits */
#define METERING_TOKEN_MAX      0x7FFFF  /* Max number of supported tokens */
#define METER_BUCKETSIZE_MAX    12
#define BITSINBYTE              8
#define KBYTES                  (1024)
#define MBYTES                  (1024) * KBYTES
#define KBITS_IN_API            1000

#ifdef BCM_TRX_SUPPORT
#define _BCM_TRX_RATE_BCAST_INDEX       0
#define _BCM_TRX_RATE_MCAST_INDEX       1
#define _BCM_TRX_RATE_DLF_INDEX         2

#define _BCM_TRX_RATE_BYTE_MODE         1
#define _BCM_TRX_RATE_PKT_MODE          0

#define _BCM_TRX_RATE_MAX_REFRESH_COUNT 0x7ffff
#define _BCM_TRX_RATE_DFT_PKT_BUCKETSZ  7       /* max allow */
#endif /* BCM_TRX_SUPPORTED */

#if defined(BCM_BANDWIDTH_RATE_METER)

static uint32 kbits_2_bucket_size[METER_BUCKETSIZE_MAX + 1] = {
      4 * KBYTES * BITSINBYTE / KBITS_IN_API,   /* 4 Kbytes bucket size */
      8 * KBYTES * BITSINBYTE / KBITS_IN_API,   /* 8 Kbytes bucket size */
     16 * KBYTES * BITSINBYTE / KBITS_IN_API,   /* 16 Kbytes bucket size */
     32 * KBYTES * BITSINBYTE / KBITS_IN_API,   /* 32 Kbytes bucket size */
     64 * KBYTES * BITSINBYTE / KBITS_IN_API,   /* 64 Kbytes bucket size */
    128 * KBYTES * BITSINBYTE / KBITS_IN_API,   /* 128 Kbytes bucket size */
    256 * KBYTES * BITSINBYTE / KBITS_IN_API,   /* 256 Kbytes bucket size */
    512 * KBYTES * BITSINBYTE / KBITS_IN_API,   /* 512 Kbytes bucket size */
      1 * MBYTES * BITSINBYTE / KBITS_IN_API,   /* 1 Mbytes bucket size */
      2 * MBYTES * BITSINBYTE / KBITS_IN_API,   /* 2 Mbytes bucket size */
      4 * MBYTES * BITSINBYTE / KBITS_IN_API,   /* 4 Mbytes bucket size */
      8 * MBYTES * BITSINBYTE / KBITS_IN_API,   /* 8 Mbytes bucket size */
     16 * MBYTES * BITSINBYTE / KBITS_IN_API    /* 16 Mbytes bucket size */
};

/*
 * Function:
 *      _bcm_kbits_to_bucket_size
 * Purpose:
 *      Translate kbits burst value into bucket size
 * Parameters:
 *      kbits_burst - Value to translate
 *      
 * Returns:
 *      Programable value 
 */
STATIC int 
_bcm_kbits_to_bucket_size(uint32 kbits_burst)
{
    int bckt_sz; 

    for (bckt_sz = 0; bckt_sz <= METER_BUCKETSIZE_MAX; bckt_sz++) {
        if (kbits_burst <= kbits_2_bucket_size[bckt_sz]) {
            break;
        }
    }
    if (bckt_sz > METER_BUCKETSIZE_MAX) {
        bckt_sz = METER_BUCKETSIZE_MAX;
    }

    return bckt_sz;
}

/*
 * Function:
 *      _bcm_bucket_size_to_kbits
 * Purpose:
 *      Translate bucket_size into kbits burst
 * Parameters:
 *      kbits_burst - Value to translate
 *      
 * Returns:
 *      Programable value 
 */
STATIC uint32
_bcm_bucket_size_to_kbits(uint32 bucket_size)
{
    assert(bucket_size <= METER_BUCKETSIZE_MAX);

    return(kbits_2_bucket_size[bucket_size]);
}
#endif /* BCM_BANDWIDTH_RATE_METER */
/*
 * Function:
 *      _bcm_rate_bw_set
 * Purpose:
 *      Helper function to bcm_esw_rate_bandwidth_set
 * Parameters:
 *      unit        - BCM unit
 *      port        - port number
 *      mem         - Memory to program
 *      refresh -   value to configure refresh count with
 *      bucket_size - value to configure bucket size with
 *      
 * Returns:
 *      Programable value 
 */
#if defined(BCM_BANDWIDTH_RATE_METER)
STATIC int
_bcm_rate_bw_set(int unit, int port, uint32 mem, uint32 refresh, 
                 int bucket_size)
{
    fp_sc_meter_table_entry_t   entryMeter;

    soc_mem_field32_set(unit, mem, &entryMeter, BUCKETSIZEf, bucket_size);
    soc_mem_field32_set(unit, mem, &entryMeter, REFRESHCOUNTf, refresh);
    soc_mem_field32_set(unit, mem, &entryMeter, BUCKETCOUNTf, 0);

    soc_mem_write(unit, mem, MEM_BLOCK_ALL,port, &entryMeter);

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_rate_bw_get
 * Purpose:
 *      Helper function to bcm_esw_rate_bandwidth_get
 * Parameters:
 *      unit        - BCM unit
 *      port        - BCM port 
 *      mem         - Memory to read 
 *      kbits_sec   - Rate in kilobits (1000 bits) per second.
 *      kbits_burst - Maximum burst size in kilobits(1000 bits)
 *      
 * Returns:
 *      Programable value 
 */
STATIC int
_bcm_rate_bw_get(int unit, int port, uint32 mem, 
                 uint32 *kbits_sec, uint32 *kbits_burst) 
{

    fp_sc_meter_table_entry_t   entryMeter;
    uint32                      bucket_size;

    BCM_IF_ERROR_RETURN(
        soc_mem_read(unit, mem, MEM_BLOCK_ANY, port, &entryMeter));

    *kbits_sec = TOKEN_BUCKET_SIZE * 
        soc_mem_field32_get(unit, mem, &entryMeter, REFRESHCOUNTf);
    bucket_size = soc_mem_field32_get(unit, mem, &entryMeter, BUCKETSIZEf);
    *kbits_burst = _bcm_bucket_size_to_kbits(bucket_size);

    return BCM_E_NONE;
}
#endif
/*
 * Function:
 *      _rate_reg_get
 * Purpose:
 *      Translate reg according to (sub)module (tucana)
 * Parameters:
 *      unit - soc device
 *      reg - The reg to translate
 * Returns:
 *      Register enum.
 */

STATIC soc_reg_t
_rate_reg_get(int unit, int port, soc_reg_t reg)
{
#if defined(BCM_TUCANA_SUPPORT)
    if (!SOC_IS_TUCANA(unit)) {
        return reg;
    }
    /* Translate from generic register to module specific register */
    switch (reg) {
    case BCAST_RATE_CONTROLr:
        if (IS_FE_PORT(unit, port)) {
            return SOC_PORT_MOD1(unit, port) ?
                BCAST_RATE_CONTROL_M1r : BCAST_RATE_CONTROL_M0r;
        } else if (IS_GE_PORT(unit, port)) {
            return BCAST_RATE_CONTROLr;
        }
        break;
    case MCAST_RATE_CONTROLr:
        if (IS_FE_PORT(unit, port)) {
            return SOC_PORT_MOD1(unit, port) ?
                MCAST_RATE_CONTROL_M1r : MCAST_RATE_CONTROL_M0r;
        } else if (IS_GE_PORT(unit, port)) {
            return MCAST_RATE_CONTROLr;
        }
        break;
    case DLF_RATE_CONTROLr:
        if (IS_FE_PORT(unit, port)) {
            return SOC_PORT_MOD1(unit, port) ?
                DLFBC_RATE_CONTROL_M1r : DLFBC_RATE_CONTROL_M0r;
        } else if (IS_GE_PORT(unit, port)) {
            return DLFBC_RATE_CONTROLr;
        }
        break;
    default:
        break;
    }
    return reg;
#else /* BCM_TUCANA_SUPPORT */
    return reg;
#endif
}

/*
 * Get the address of a rate control reg based on generic name.
 * This is mainly for Tucana which needs to map the register based
 * on the port number (to module).  Note that the register is updated
 * too.
 */

STATIC uint32
_rate_reg_addr_get(int unit, int port, soc_reg_t reg)
{
#if defined(BCM_TUCANA_SUPPORT)
    if (!SOC_IS_TUCANA(unit)) {
        return soc_reg_addr(unit, reg, REG_PORT_ANY, port);
    }
    return soc_reg_addr(unit, _rate_reg_get(unit, port, reg), REG_PORT_ANY,
                        IS_FE_PORT(unit, port) ? (port % 32) : (port % 4));
#else /* BCM_TUCANA_SUPPORT */
    return soc_reg_addr(unit, reg, REG_PORT_ANY, port);
#endif
}

STATIC uint32
_rate_reg_index_get(int unit, int port)
{
#if defined(BCM_TUCANA_SUPPORT)
    if (!SOC_IS_TUCANA(unit)) {
        return port;
    }
    return IS_FE_PORT(unit, port) ? (port % 32) : (port % 4);
#else /* BCM_TUCANA_SUPPORT */
    return port;
#endif
}


#ifdef BCM_TRX_SUPPORT
/*  
 * Like most other switching chips, Triumph and Scorpion provide meters 
 * to control Brocast/Multicast/DLF packet storm. And similar to Raptor, 
 * they allow the byte-mode or packet-mode meters. However, unlike Raptor,
 * both modes in Triumph and Scorpion can not be mixed for a given port.
 * Triumph and Scorpion supports a set of 4 meters per ingress port. 
 * By default, if metering enabled, Broadcast packets select the 0th meter,
 * Muliticast packets (including KNOWN/UNKNOWN L2MC/IPMC packets) select 1st 
 * meter, and DLF packets select 2rd meter. If one ingress port want to
 * change the metering mode, it shall first turn off all meters enabled with
 * existing metering mode, and then program the new mode. Different metering 
 * mode in different ports are allowed. 
 */

/* Verify the meter mode per port */
static int 
_bcm_trx_rate_modeset_verify (int unit, int port, int mode)
{
    uint32 ctrlMeter;
    int  byte_mode;

    SOC_IF_ERROR_RETURN(READ_STORM_CONTROL_METER_CONFIGr(unit, port, 
                        &ctrlMeter));
    byte_mode = soc_reg_field_get(unit, STORM_CONTROL_METER_CONFIGr, ctrlMeter,
                                  BYTE_MODEf);
    if (byte_mode != mode) {

        if (SOC_IS_SHADOW(unit)) {
            if (soc_reg_field_get(unit, STORM_CONTROL_METER_CONFIGr, ctrlMeter,
                                  BCAST_ENABLEf) || 
                soc_reg_field_get(unit, STORM_CONTROL_METER_CONFIGr, ctrlMeter,
                                  MC_ENABLEf)) {
                return BCM_E_RESOURCE;
            }
        } else {
            /* Check whether any meter is enabled */
            if (soc_reg_field_get(unit, STORM_CONTROL_METER_CONFIGr, ctrlMeter,
                                  BCAST_ENABLEf) || 
                soc_reg_field_get(unit, STORM_CONTROL_METER_CONFIGr, ctrlMeter,
                                  DLFBC_ENABLEf) ||
                soc_reg_field_get(unit, STORM_CONTROL_METER_CONFIGr, ctrlMeter,
                                  KNOWN_L2MC_ENABLEf) ||
                soc_reg_field_get(unit, STORM_CONTROL_METER_CONFIGr, ctrlMeter,
                                  UNKNOWN_L2MC_ENABLEf) ||
                soc_reg_field_get(unit, STORM_CONTROL_METER_CONFIGr, ctrlMeter,
                                  KNOWN_IPMC_ENABLEf) ||
                soc_reg_field_get(unit, STORM_CONTROL_METER_CONFIGr, ctrlMeter,
                                  UNKNOWN_IPMC_ENABLEf)) {
                return BCM_E_RESOURCE;
            }
        }
    } 

    return BCM_E_NONE;
}

/* Table of Kbits to BucketSize */
static uint32 
_bcm_trx_rate_bucketSz_map [] = {
    ((4   * KBYTES * BITSINBYTE) / KBITS_IN_API),
    ((16  * KBYTES * BITSINBYTE) / KBITS_IN_API),
    ((64  * KBYTES * BITSINBYTE) / KBITS_IN_API),
    ((256 * KBYTES * BITSINBYTE) / KBITS_IN_API),
    ((1   * MBYTES * BITSINBYTE) / KBITS_IN_API),
    ((4   * MBYTES * BITSINBYTE) / KBITS_IN_API),
    ((8   * MBYTES * BITSINBYTE) / KBITS_IN_API),
    ((16  * MBYTES * BITSINBYTE) / KBITS_IN_API)
};   

/* Kbit_to_bucketSz conversion */
static int
_bcm_trx_rate_kbit_to_bucketsz (uint32 kbits)
{
    int ix;
    int maxIndex = sizeof(_bcm_trx_rate_bucketSz_map)/sizeof(uint32) - 1;

    for (ix = 0; ix <= maxIndex; ix++) {
         if (kbits <= _bcm_trx_rate_bucketSz_map[ix]) {
             return ix;
         }
    }

    return (-1);
}

/* Program the FP_STORM_CONTROL_METERS */
static int
_bcm_trx_rate_meter_rate_set (int unit, int port, int index, 
                              int bucketSz, uint32 refreshCnt)
{
    fp_storm_control_meters_entry_t  entryMeter;
    int entryNum;
    soc_mem_t mem;

    mem = FP_STORM_CONTROL_METERSm;
#ifdef BCM_TRIDENT_SUPPORT
    if (SOC_IS_TD_TT(unit) && 
        SOC_MEM_IS_VALID(unit, FP_STORM_CONTROL_METERS_Xm)) {
        if (SOC_INFO(unit).port_group[port] < 2) {
            mem = FP_STORM_CONTROL_METERS_Xm;
        } else {
            mem = FP_STORM_CONTROL_METERS_Ym;
        }
    }
#endif /* BCM_TRIDENT_SUPPORT */

    /* program the FP_STORM_CONTROL_METERS table */
    soc_mem_field32_set(unit, mem, &entryMeter, BUCKETSIZEf, bucketSz);
    soc_mem_field32_set(unit, mem, &entryMeter, REFRESHCOUNTf, refreshCnt);
    soc_mem_field32_set(unit, mem, &entryMeter, BUCKETCOUNTf, 0);

    entryNum = (port << 2) + index;
    BCM_IF_ERROR_RETURN
        (soc_mem_write(unit, mem, MEM_BLOCK_ALL, entryNum, &entryMeter));

    return BCM_E_NONE;
}

static int
_bcm_trx_rate_meter_rate_get(int unit, int port, int mode, int index, 
                             uint32 *pRps, uint32 *pBrs);

/* Set the metering mode */
static int 
_bcm_trx_rate_meter_portmode_set (int unit, int port, int mode,
                                  uint32 flags, uint32 fmask, 
                                  int rps, int brs)
{
    uint32 ctrlMeter;
    int    allDisable = 0;
    int    bitSet;
    int    byte_mode;
    int    bucketSz;
    uint32 bcast_rate, mcast_rate, dlf_rate, max_rate;
    uint32 burst;
    uint32 refreshCnt, updated_refresh_cnt;
    uint32 quantum = 125, old_quantum = 0;

    SOC_IF_ERROR_RETURN
        (READ_STORM_CONTROL_METER_CONFIGr(unit, port, &ctrlMeter));

    byte_mode = soc_reg_field_get(unit, STORM_CONTROL_METER_CONFIGr, 
                                  ctrlMeter, BYTE_MODEf);

    old_quantum = 0;
    bcast_rate = 0;
    mcast_rate = 0;
    dlf_rate = 0;
    if (mode == _BCM_TRX_RATE_PKT_MODE) {
        max_rate = rps;
        if (mode == byte_mode) {
            old_quantum = soc_reg_field_get(unit, STORM_CONTROL_METER_CONFIGr,
                                            ctrlMeter, PACKET_QUANTUMf);
            if (soc_reg_field_get(unit, STORM_CONTROL_METER_CONFIGr, ctrlMeter,
                                  BCAST_ENABLEf) == 1) {
                _bcm_trx_rate_meter_rate_get(unit, port, mode,
                                             _BCM_TRX_RATE_BCAST_INDEX,
                                             &bcast_rate, &burst);
                if (max_rate < bcast_rate) {
                    max_rate = bcast_rate;
                }
            }

            if (SOC_IS_SHADOW(unit)) {
                if (soc_reg_field_get(unit, STORM_CONTROL_METER_CONFIGr, ctrlMeter,
                                      MC_ENABLEf) == 1) {
                    _bcm_trx_rate_meter_rate_get(unit, port, mode,
                                                 _BCM_TRX_RATE_MCAST_INDEX,
                                                 &mcast_rate, &burst);
                    if (max_rate < mcast_rate) {
                        max_rate = mcast_rate;
                    }
                }
            } else {
                if (soc_reg_field_get(unit, STORM_CONTROL_METER_CONFIGr, ctrlMeter,
                                      KNOWN_L2MC_ENABLEf) == 1) {
                    _bcm_trx_rate_meter_rate_get(unit, port, mode,
                                                 _BCM_TRX_RATE_MCAST_INDEX,
                                                 &mcast_rate, &burst);
                    if (max_rate < mcast_rate) {
                        max_rate = mcast_rate;
                    }
                }
            }
            if (!SOC_IS_SHADOW(unit)) {
                if (soc_reg_field_get(unit, STORM_CONTROL_METER_CONFIGr, ctrlMeter,
                                      DLFBC_ENABLEf) == 1) {
                    _bcm_trx_rate_meter_rate_get(unit, port, mode,
                                                 _BCM_TRX_RATE_DLF_INDEX,
                                                 &dlf_rate, &burst);
                    if (max_rate < dlf_rate) {
                        max_rate = dlf_rate;
                    }
                }
            }
        }

        /* brs is ignored in the packet mode, used default value */
        bucketSz = _BCM_TRX_RATE_DFT_PKT_BUCKETSZ;

        /*
         * Each meter is refreshed 64000 times/sec;
         * Each refresh will add (refreshCnt * 2) tokens;
         * Each packet consumes (PACKET_QUANTUM * 16) tokens;
         * refreshCnt = rps * PACKET_QUANTUM * 16 / 64000 / 2;
         *            = rps * PACKET_QUANTUM / 8000;
         *
         * pick quantum which is a factor of 8000 to reduce error
         */
        for (quantum = 4000; quantum >= 125; quantum >>= 1) {
            if (max_rate <=
                _BCM_TRX_RATE_MAX_REFRESH_COUNT * (8000 / quantum)) {
                break;
            }
        }
        if (quantum < 125) {
            for (quantum = 80; quantum >= 5; quantum >>= 1) {
                if (max_rate <=
                    _BCM_TRX_RATE_MAX_REFRESH_COUNT * (8000 / quantum)) {
                    break;
                }
            }
            if (quantum < 5) {
                return BCM_E_PARAM;
            }
        }
        refreshCnt = rps / (8000 / quantum);
    } else {
        /* brs is in unit of Kbits */
        bucketSz = _bcm_trx_rate_kbit_to_bucketsz(brs);
        if (bucketSz == -1) {
            return BCM_E_PARAM;
        }
        /* rps is unit of Kbits, refreshCnt = rps X 1000 / 64000 */
        refreshCnt = rps / 64;
    }

    if (refreshCnt == 0) {
        refreshCnt = 1;
    } else if (refreshCnt > _BCM_TRX_RATE_MAX_REFRESH_COUNT) {
        return BCM_E_PARAM;
    }

    if (byte_mode != mode) {

        /* if metering mode is changed, disable the storm control first */
        soc_reg_field_set(unit, STORM_CONTROL_METER_CONFIGr, &ctrlMeter,
                          BCAST_ENABLEf, 0);
        if (SOC_IS_SHADOW(unit)) {
            soc_reg_field_set(unit, STORM_CONTROL_METER_CONFIGr, &ctrlMeter,
                          MC_ENABLEf, 0);
        } else {
            soc_reg_field_set(unit, STORM_CONTROL_METER_CONFIGr, &ctrlMeter,
                              DLFBC_ENABLEf, 0);
            soc_reg_field_set(unit, STORM_CONTROL_METER_CONFIGr, &ctrlMeter,
                          KNOWN_L2MC_ENABLEf, 0);
            soc_reg_field_set(unit, STORM_CONTROL_METER_CONFIGr, &ctrlMeter,
                          UNKNOWN_L2MC_ENABLEf, 0);
            soc_reg_field_set(unit, STORM_CONTROL_METER_CONFIGr, &ctrlMeter,
                              KNOWN_IPMC_ENABLEf, 0);
            soc_reg_field_set(unit, STORM_CONTROL_METER_CONFIGr, &ctrlMeter,
                              UNKNOWN_IPMC_ENABLEf, 0);
        }

        allDisable = 1;
        /* program new metering mode */
        soc_reg_field_set(unit, STORM_CONTROL_METER_CONFIGr, &ctrlMeter,
                          BYTE_MODEf, mode);

        SOC_IF_ERROR_RETURN(WRITE_STORM_CONTROL_METER_CONFIGr(unit, port,
                        ctrlMeter));
    }

    /* enable/disable BCAST counting */
    if (fmask & BCM_RATE_BCAST) {
        bitSet = 0;
        if (flags & BCM_RATE_BCAST) {
            _bcm_trx_rate_meter_rate_set (unit, port, 
                               _BCM_TRX_RATE_BCAST_INDEX, 
                               bucketSz, refreshCnt);
            bitSet = 1;
        }
        if (bitSet || !allDisable) {
            soc_reg_field_set(unit, STORM_CONTROL_METER_CONFIGr, &ctrlMeter,
                              BCAST_ENABLEf, bitSet);
        }
    } else if (mode == _BCM_TRX_RATE_PKT_MODE && bcast_rate != 0 &&
          quantum != old_quantum) {
        updated_refresh_cnt = bcast_rate / (8000 / quantum);
        _bcm_trx_rate_meter_rate_set(unit, port, _BCM_TRX_RATE_BCAST_INDEX, 
                                     bucketSz, updated_refresh_cnt);
    }

    /* enable/disable MCAST counting */
    if (fmask & BCM_RATE_MCAST) {
        bitSet = 0;
        if (flags & BCM_RATE_MCAST) {
            _bcm_trx_rate_meter_rate_set (unit, port, 
                               _BCM_TRX_RATE_MCAST_INDEX, 
                               bucketSz, refreshCnt);
            bitSet = 1;
        }
        if (bitSet || !allDisable) { 
            if (SOC_IS_SHADOW(unit)) {
                soc_reg_field_set(unit, STORM_CONTROL_METER_CONFIGr, &ctrlMeter,
                              MC_ENABLEf, bitSet);
            } else {
                soc_reg_field_set(unit, STORM_CONTROL_METER_CONFIGr, &ctrlMeter,
                              KNOWN_L2MC_ENABLEf, bitSet);
                soc_reg_field_set(unit, STORM_CONTROL_METER_CONFIGr, &ctrlMeter,
                              UNKNOWN_L2MC_ENABLEf, bitSet);
                soc_reg_field_set(unit, STORM_CONTROL_METER_CONFIGr, &ctrlMeter,
                                  KNOWN_IPMC_ENABLEf, bitSet);
                soc_reg_field_set(unit, STORM_CONTROL_METER_CONFIGr, &ctrlMeter,
                                  UNKNOWN_IPMC_ENABLEf, bitSet);
            }
        }
    } else if (mode == _BCM_TRX_RATE_PKT_MODE && mcast_rate != 0 &&
               quantum != old_quantum) {
        updated_refresh_cnt = mcast_rate / (8000 / quantum);
        _bcm_trx_rate_meter_rate_set(unit, port, _BCM_TRX_RATE_MCAST_INDEX, 
                                     bucketSz, updated_refresh_cnt);
    }

    /* enable/disable DLF counting */
    if (fmask & BCM_RATE_DLF) {
        bitSet = 0;
        if (flags & BCM_RATE_DLF) {
            _bcm_trx_rate_meter_rate_set (unit, port, 
                               _BCM_TRX_RATE_DLF_INDEX, bucketSz,
                               refreshCnt);
           bitSet = 1;
        }

        if (!SOC_IS_SHADOW(unit)) {
            if (bitSet || !allDisable) {
                soc_reg_field_set(unit, STORM_CONTROL_METER_CONFIGr, &ctrlMeter,
                              DLFBC_ENABLEf, bitSet);
            }
        }
    } else if (mode == _BCM_TRX_RATE_PKT_MODE && dlf_rate != 0 &&
               quantum != old_quantum) {
        updated_refresh_cnt = dlf_rate / (8000 / quantum);
        _bcm_trx_rate_meter_rate_set(unit, port, _BCM_TRX_RATE_DLF_INDEX, 
                                     bucketSz, updated_refresh_cnt);
    }

    /* Set PACKET_QUANTUM for optimal precision */ 
    if (mode == _BCM_TRX_RATE_PKT_MODE) {
        soc_reg_field_set(unit, STORM_CONTROL_METER_CONFIGr, &ctrlMeter,
                          PACKET_QUANTUMf, quantum);
    }

    /* write to registers */
    SOC_IF_ERROR_RETURN(WRITE_STORM_CONTROL_METER_CONFIGr(unit, port,
                        ctrlMeter));
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_trx_rate_set
 * Description:
 *      Triumph and Scopiton specific implementation for
 *      bcm_rate_xxx_set APIs
 * Parameters:
 *      unit  -  unit
 *      port  -  port number
 *      mode  -  packet- or byte-based mode
 *      flags -  flags for packet type
 *      fmask -  flags mask for <flags> valid bits
 *      rps   -  rate per second (pkt/sec for packet-based mode
 *                                Kbits/sec for byte-based mode)
 *      brs   -  burst size number (ignored in packet-based mode,
 *                                  Kbits/sec for byte-based mode)  
 *          
 * Return:
 *      BCM_E_XXX
 */
static int
_bcm_trx_rate_set (int unit, int port, int mode, uint32 flags, 
                   uint32 fmask, uint32 rps, uint32 brs)
{
    int res;

    if (!soc_feature(unit,soc_feature_storm_control)) {
        return BCM_E_UNAVAIL;
    }

    if (port != -1 && !SOC_PORT_VALID(unit, port)) {
        return BCM_E_PORT;
    }
    if (!(fmask & BCM_RATE_ALL)) {
        return BCM_E_PARAM;
    }
    assert ((mode == _BCM_TRX_RATE_PKT_MODE) || 
            (mode == _BCM_TRX_RATE_BYTE_MODE));

    /* verify the configured mode is avaialble */
    if (port == -1) { /* all port */
        PBMP_ALL_ITER(unit, port) {
            if ((res = _bcm_trx_rate_modeset_verify (unit, port, 
                      mode)) != BCM_E_NONE)
                return res;
        } 
        PBMP_ALL_ITER(unit, port) {
            if ((res = _bcm_trx_rate_meter_portmode_set (unit, port,
                      mode, flags, fmask, rps, brs)) != BCM_E_NONE)
                return res;
        }
    } else {
        if ((res = _bcm_trx_rate_modeset_verify (unit, port, 
                      mode)) != BCM_E_NONE)
            return res;

        if ((res = _bcm_trx_rate_meter_portmode_set (unit, port, 
                      mode, flags, fmask, rps, brs)) != BCM_E_NONE)
            return res;
    }          
    return BCM_E_NONE;
}

static int
_bcm_trx_rate_meter_rate_get (int unit, int port, int mode, int index, 
                              uint32 *pRps, uint32 *pBrs)
{
    uint32 ctrlMeter;
    fp_storm_control_meters_entry_t entryMeter;
    int    entryNum;
    uint32 bucketSz;
    uint32 refreshCnt;
    uint32 quantum;
    soc_mem_t mem;

    mem = FP_STORM_CONTROL_METERSm;
#ifdef BCM_TRIDENT_SUPPORT
    if (SOC_IS_TD_TT(unit) && 
        SOC_MEM_IS_VALID(unit, FP_STORM_CONTROL_METERS_Xm)) {
        if (SOC_INFO(unit).port_group[port] < 2) {
            mem = FP_STORM_CONTROL_METERS_Xm;
        } else {
            mem = FP_STORM_CONTROL_METERS_Ym;
        }
    }
#endif /* BCM_TRIDENT_SUPPORT */

    entryNum = (port << 2) + index;
    BCM_IF_ERROR_RETURN
        (soc_mem_read(unit, mem, MEM_BLOCK_ANY, entryNum, &entryMeter));

    refreshCnt = soc_mem_field32_get(unit, mem, &entryMeter, REFRESHCOUNTf);
    bucketSz   = soc_mem_field32_get(unit, mem, &entryMeter, BUCKETSIZEf);
    if (mode == _BCM_TRX_RATE_PKT_MODE) {
        SOC_IF_ERROR_RETURN
            (READ_STORM_CONTROL_METER_CONFIGr(unit, port, &ctrlMeter));
        quantum = soc_reg_field_get(unit, STORM_CONTROL_METER_CONFIGr,
                                    ctrlMeter, PACKET_QUANTUMf);

        /*
         * Each meter is refreshed 64000 times/sec;
         * Each refresh will add (refreshCnt * 2) tokens;
         * Each packet consumes (PACKET_QUANTUM * 16) tokens;
         * *pRps = refreshCnt * 2 * 64000 / PACKET_QUANTUM / 16;
         *       = refreshCnt * 8000 / PACKET_QUANTUM;
         */
        *pRps = refreshCnt * 8000 / quantum;
    } else if (mode == _BCM_TRX_RATE_BYTE_MODE) {
         /* 
          * Each meter is refreshed 64000 times/sec;
          * Each refresh will add 2 * refreshCnt tokens;
          * Each token represents 0.5 bit data.
          * *pRps = ((refreshCnt * 2 * 64000) / 2) / (KBITS_IN_API) ->
          * *pRps = refreshCnt * 64;
          */
       *pRps = refreshCnt * 64; 
       *pBrs = _bcm_trx_rate_bucketSz_map[bucketSz]; 

    } else 
        return BCM_E_PARAM;
    
    return BCM_E_NONE;
}

static int
_bcm_trx_rate_get (int unit, int port, int mode, uint32 * pFlags,
                   uint32 fmask, uint32 *pRps, uint32 *pBrs)
{
    uint32 ctrlMeter;
    int    byte_mode;
    uint32 pktType = 0;
    uint32 rate    = 0;
    uint32 burstSz = 0;

    if (!soc_feature(unit,soc_feature_storm_control)) {
        if (pFlags != NULL) {
            *pFlags = pktType;
        }
        return BCM_E_UNAVAIL;
    }

    if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PORT;
    }
    assert ((mode == _BCM_TRX_RATE_PKT_MODE) ||
            (mode == _BCM_TRX_RATE_BYTE_MODE));

    SOC_IF_ERROR_RETURN(READ_STORM_CONTROL_METER_CONFIGr(unit, port, 
                        &ctrlMeter));
    if (fmask != BCM_RATE_ALL && ((fmask-1) & fmask)) {
        return BCM_E_PARAM;
    }
    byte_mode = soc_reg_field_get(unit, STORM_CONTROL_METER_CONFIGr, ctrlMeter,
                                                            BYTE_MODEf);
    if (byte_mode == mode) {
        if (fmask & BCM_RATE_BCAST) {
            if (soc_reg_field_get(unit, STORM_CONTROL_METER_CONFIGr, ctrlMeter,
                                  BCAST_ENABLEf) == 1) {            
                pktType |= BCM_RATE_BCAST;
                _bcm_trx_rate_meter_rate_get (unit, port, mode,
                                  _BCM_TRX_RATE_BCAST_INDEX, &rate, &burstSz);
            }  
        }
        if (fmask & BCM_RATE_MCAST) {
            if (SOC_IS_SHADOW(unit)) {
                if ((soc_reg_field_get(unit, STORM_CONTROL_METER_CONFIGr, ctrlMeter,
                                  MC_ENABLEf) == 1)) {
                    pktType |= BCM_RATE_MCAST;
                    _bcm_trx_rate_meter_rate_get (unit, port, mode,
                                  _BCM_TRX_RATE_MCAST_INDEX, &rate, &burstSz);
                }
            } else {
                if ((soc_reg_field_get(unit, STORM_CONTROL_METER_CONFIGr, ctrlMeter,
                                  KNOWN_L2MC_ENABLEf) == 1) || 
                    (soc_reg_field_get(unit, STORM_CONTROL_METER_CONFIGr, ctrlMeter,
                                  UNKNOWN_L2MC_ENABLEf) == 1) || 
                    (soc_reg_field_get(unit, STORM_CONTROL_METER_CONFIGr, ctrlMeter,
                                  KNOWN_IPMC_ENABLEf) == 1) || 
                    (soc_reg_field_get(unit, STORM_CONTROL_METER_CONFIGr, ctrlMeter,
                                  UNKNOWN_IPMC_ENABLEf) == 1)) {
                    pktType |= BCM_RATE_MCAST;
                    _bcm_trx_rate_meter_rate_get (unit, port, mode,
                                  _BCM_TRX_RATE_MCAST_INDEX, &rate, &burstSz);
                }
            }
        }
        if (!SOC_IS_SHADOW(unit)) {
            if (fmask & BCM_RATE_DLF) {
                if (soc_reg_field_get(unit, STORM_CONTROL_METER_CONFIGr, ctrlMeter,
                                  DLFBC_ENABLEf) == 1) {            
                    pktType |= BCM_RATE_DLF;
                    _bcm_trx_rate_meter_rate_get (unit, port, mode,
                                  _BCM_TRX_RATE_DLF_INDEX, &rate, &burstSz);
                }
            }
        }
    } 

    if (pFlags != NULL) {
        *pFlags = pktType;
    }
    if (pRps != NULL) {
        *pRps = rate;
    }
    if (pBrs != NULL) {
        *pBrs = burstSz;
    }

    return BCM_E_NONE;
}
#endif /* BCM_TRX_SUPPORT */

/*
 * Function:
 *      _bcm_esw_rate_hw_clear
 * Description:
 *      Clear all registers and memories that related to rate limiting 
 * Parameters:
 *      unit - unit number
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_esw_rate_hw_clear(int unit)
{
    uint32 rval = 0, addr;
    bcm_port_t  port;
    int         i;
    soc_mem_t   _bcm_esw_rate_mems[] = {
        FP_SC_BCAST_METER_TABLEm,
        FP_SC_MCAST_METER_TABLEm,
        FP_SC_DLF_METER_TABLEm,
        FP_STORM_CONTROL_METERSm
    };
    soc_reg_t   _bcm_esw_rate_regs[] = {
        SC_BYTE_METER_CONFIGr,
        BCAST_STORM_CONTROLr,
        BCAST_RATE_CONTROLr,
        DLFBC_STORM_CONTROLr,
        DLF_RATE_CONTROLr,
        MCAST_STORM_CONTROLr,
        MCAST_RATE_CONTROLr,
        STORM_CONTROL_METER_CONFIGr
        };

    /* Registers initialization */      
    for (i = 0; i < COUNTOF(_bcm_esw_rate_regs); i++) {
        if (SOC_REG_IS_VALID(unit, _bcm_esw_rate_regs[i])) {
            PBMP_PORT_ITER(unit, port) {
                if (((BCAST_RATE_CONTROLr == _bcm_esw_rate_regs[i]) || 
                    (MCAST_RATE_CONTROLr == _bcm_esw_rate_regs[i]) ||
                    (DLF_RATE_CONTROLr == _bcm_esw_rate_regs[i])) && 
                    !IS_E_PORT(unit,  port)){
                    continue;
                }
                addr = _rate_reg_addr_get(unit, port, _bcm_esw_rate_regs[i]);
                SOC_IF_ERROR_RETURN(soc_reg32_write(unit, addr, rval));
                /* special case handling */
                if (STORM_CONTROL_METER_CONFIGr == _bcm_esw_rate_regs[i]) {
                    soc_field_t fields[] = {PACKET_QUANTUMf, BYTE_MODEf};
                    uint32      values[] = {0x100, 1};

                    BCM_IF_ERROR_RETURN(
                        soc_reg_fields32_modify(unit,  
                                                STORM_CONTROL_METER_CONFIGr, 
                                                port, COUNTOF(fields), 
                                                fields, values));
                }
            }
        }
    }

    /* Memories initialization */
    for (i = 0; i < COUNTOF(_bcm_esw_rate_mems); i++) {
        if (SOC_MEM_IS_VALID(unit, _bcm_esw_rate_mems[i])) {
            BCM_IF_ERROR_RETURN(
                 soc_mem_clear(unit, _bcm_esw_rate_mems[i], COPYNO_ALL, TRUE));
        }
    }

    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_esw_rate_init
 * Description:
 *      Initialize the rate module
 * Parameters:
 *      unit - unit number
 * Returns:
 *      BCM_E_XXX
 */
int 
_bcm_esw_rate_init(int unit)
{
    if(SOC_IS_RCPU_ONLY(unit)) {
        return BCM_E_NONE;
    }

    /* Currently no SW structure required for rate module so only clear HW */
    return _bcm_esw_rate_hw_clear(unit);
}

/*
 * Function:
 *      bcm_rate_type_set
 * Description:
 *      Front end to bcm_*cast_rate_set functions.
 *      Uses a single data structure to write into
 *      all the 3 rate control registers
 * Parameters:
 *      unit - unit number
 *      rl - data structure containing info to be written to the
 *           rate control registers
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_rate_type_set(int unit, bcm_rate_limit_t *rl)
{
    int port;

    COMPILER_REFERENCE(_rate_reg_addr_get); 

    PBMP_E_ITER(unit, port) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_rate_bcast_set(unit, rl->br_bcast_rate, rl->flags, port));
        BCM_IF_ERROR_RETURN
            (bcm_esw_rate_mcast_set(unit, rl->br_mcast_rate, rl->flags, port));
        BCM_IF_ERROR_RETURN
            (bcm_esw_rate_dlfbc_set(unit, rl->br_dlfbc_rate, rl->flags, port));
    }

    return BCM_E_NONE;
}

/* These values are to convert the 960 usec on Lynx to pps. */
#define _LYNX_RATE_NUMERATOR       3
#define _LYNX_RATE_DENOMINATOR  3125

#define _LYNX_PPS_TO_RATE(pps) \
        ((((pps) * _LYNX_RATE_NUMERATOR) + (_LYNX_RATE_DENOMINATOR - 1)) / \
        _LYNX_RATE_DENOMINATOR)

#define _LYNX_RATE_TO_PPS(rate) \
        ((((rate) * _LYNX_RATE_DENOMINATOR) + (_LYNX_RATE_NUMERATOR - 1)) / \
        _LYNX_RATE_NUMERATOR)

/*
 * Function:
 *      bcm_rate_set
 * Description:
 *      Configure rate limit and on/off state of
 *      DLF, MCAST, and BCAST limiting
 * Parameters:
 *      unit - StrataSwitch PCI device unit number
 *      pps - Rate limit value in packets/second
 *      flags - Bitmask with one or more of BCM_RATE_*
 * Returns:
 *      BCM_E_NONE - Success.
 *      BCM_E_INTERNAL - Chip access failure.
 * Notes:
 *      StrataSwitch supports only one rate limit for all 3 types.
 */

int
bcm_esw_rate_set(int unit, int pps, int flags)
{
    uint32      rate;
    int         port;
    soc_reg_t   reg;
    uint32      addr;

#ifdef BCM_XGS3_SWITCH_SUPPORT
#ifdef BCM_TRX_SUPPORT
    if (SOC_IS_TRX(unit)) {
        return (_bcm_trx_rate_set(unit, -1, _BCM_TRX_RATE_PKT_MODE, 
                                  flags, BCM_RATE_ALL, (uint32)pps, -1)); 
    }
#endif /* BCM_TRX_SUPPORT */

    if (SOC_IS_XGS3_SWITCH(unit)) {
        int enable = 0;
        PBMP_ALL_ITER(unit, port) {
            rate = 0;
            enable = (flags & BCM_RATE_MCAST) ? 1 : 0;
            soc_reg_field_set(unit, MCAST_STORM_CONTROLr,
                              &rate, ENABLEf, enable);
            soc_reg_field_set(unit, MCAST_STORM_CONTROLr,
                              &rate, THRESHOLDf, pps);
            SOC_IF_ERROR_RETURN
                (WRITE_MCAST_STORM_CONTROLr(unit, port, rate));

            rate = 0;
            enable = (flags & BCM_RATE_BCAST) ? 1 : 0;
            soc_reg_field_set(unit, BCAST_STORM_CONTROLr,
                              &rate, ENABLEf, enable);
            soc_reg_field_set(unit, BCAST_STORM_CONTROLr,
                              &rate, THRESHOLDf, pps);
            SOC_IF_ERROR_RETURN
                (WRITE_BCAST_STORM_CONTROLr(unit, port, rate));

            rate = 0;
            enable = (flags & BCM_RATE_DLF) ? 1 : 0;
            soc_reg_field_set(unit, DLFBC_STORM_CONTROLr,
                              &rate, ENABLEf, enable);
            soc_reg_field_set(unit, DLFBC_STORM_CONTROLr,
                              &rate, THRESHOLDf, pps);
            SOC_IF_ERROR_RETURN
                (WRITE_DLFBC_STORM_CONTROLr(unit, port, rate));
        }
        return BCM_E_NONE;
    }
#endif

    if (SOC_IS_XGS_SWITCH(unit)) {
        if (soc_feature(unit, soc_feature_fast_rate_limit)) {
            pps = _LYNX_PPS_TO_RATE(pps);
        }

        PBMP_E_ITER(unit, port) {
            rate = 0;
            reg = _rate_reg_get(unit, port, DLF_RATE_CONTROLr);
            soc_reg_field_set(unit, reg, &rate, ENABLEf,
                              (flags & BCM_RATE_DLF) ? 1 : 0);
            soc_reg_field_set(unit, reg, &rate, THRESHOLDf, pps);
            addr = _rate_reg_addr_get(unit, port, DLF_RATE_CONTROLr);
            SOC_IF_ERROR_RETURN(soc_reg32_set(unit, reg, REG_PORT_ANY, 
                                _rate_reg_index_get(unit, port), rate));

            rate = 0;
            reg = _rate_reg_get(unit, port, MCAST_RATE_CONTROLr);
            soc_reg_field_set(unit, reg, &rate, ENABLEf,
                              (flags & BCM_RATE_MCAST) ? 1 : 0);
            soc_reg_field_set(unit, reg, &rate, THRESHOLDf, pps);
            addr = _rate_reg_addr_get(unit, port, MCAST_RATE_CONTROLr);
            SOC_IF_ERROR_RETURN(soc_reg32_set(unit, reg, REG_PORT_ANY, 
                                _rate_reg_index_get(unit, port), rate));

            rate = 0;
            reg = _rate_reg_get(unit, port, BCAST_RATE_CONTROLr);
            soc_reg_field_set(unit, reg, &rate, ENABLEf,
                              (flags & BCM_RATE_BCAST) ? 1 : 0);
            soc_reg_field_set(unit, reg, &rate, THRESHOLDf, pps);
            addr = _rate_reg_addr_get(unit, port, BCAST_RATE_CONTROLr);
            SOC_IF_ERROR_RETURN(soc_reg32_set(unit, reg, REG_PORT_ANY, 
                                _rate_reg_index_get(unit, port), rate));
        }
        return BCM_E_NONE;
    }

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_rate_mcast_set
 * Description:
 *      Configure rate limit for MCAST packets for the given port
 * Parameters:
 *      unit - StrataSwitch PCI device unit number
 *      pps - Rate limit value in packets/second
 *      flags - Bitmask with one or more of BCM_RATE_*
 *      port - Port number for which MCAST limit needs to be set
 * Returns:
 *      BCM_E_NONE - Success.
 *      BCM_E_UNAVAIL - Not supported.
 * Notes:
 *      Individually setting MCAST limit is supported only on 5690 and later.
 */

int
bcm_esw_rate_mcast_set(int unit, int pps, int flags, int port)
{
    uint32 rate;
    int enable, threshold = pps;
    soc_reg_t reg;
    uint32 addr;

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_local_get(unit, port, &port));
    }
    if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PORT;
    }

#ifdef BCM_XGS3_SWITCH_SUPPORT
#ifdef BCM_TRX_SUPPORT
    if (SOC_IS_TRX(unit)) {
        return (_bcm_trx_rate_set(unit, port, _BCM_TRX_RATE_PKT_MODE, 
                             flags, BCM_RATE_MCAST, (uint32)pps, -1)); 
    }
#endif /* BCM_TRX_SUPPORT */

    if (SOC_IS_XGS3_SWITCH(unit)) {
        rate = 0;
        enable = (flags & BCM_RATE_MCAST) ? 1 : 0;
        soc_reg_field_set(unit, MCAST_STORM_CONTROLr,
                          &rate, ENABLEf, enable);
        soc_reg_field_set(unit, MCAST_STORM_CONTROLr,
                          &rate, THRESHOLDf, threshold);
        SOC_IF_ERROR_RETURN(WRITE_MCAST_STORM_CONTROLr(unit, port, rate));
        return BCM_E_NONE;
    }
#endif

    if (SOC_IS_XGS_SWITCH(unit)) {
        if (IS_E_PORT(unit, port)) { /* not IPIC port */
            if (soc_feature(unit, soc_feature_fast_rate_limit)) {
                threshold = _LYNX_PPS_TO_RATE(pps);
            }
            rate = 0;
            enable = (flags & BCM_RATE_MCAST) ? 1 : 0;
            reg = _rate_reg_get(unit, port, MCAST_RATE_CONTROLr);
            soc_reg_field_set(unit, reg, &rate, ENABLEf, enable);
            soc_reg_field_set(unit, reg, &rate, THRESHOLDf, threshold);

            addr = _rate_reg_addr_get(unit, port, MCAST_RATE_CONTROLr);
            SOC_IF_ERROR_RETURN(soc_reg32_set(unit, reg, REG_PORT_ANY, 
                                _rate_reg_index_get(unit, port), rate));
        }
        return BCM_E_NONE;
    }

    /*
     * StrataSwitch device supports only one rate for bcast, mcast, and dlfbc.
     * Must use bcm_rate_set() to set rate on StrataSwitch. 
     */ 

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_rate_bcast_set
 * Description:
 *      Configure rate limit for BCAST packets
 * Parameters:
 *      unit - StrataSwitch PCI device unit number
 *      pps - Rate limit value in packets/second
 *      flags - Bitmask with one or more of BCM_RATE_*
 *      port - Port number for which BCAST limit needs to be set
 * Returns:
 *      BCM_E_NONE - Success.
 *      BCM_E_UNAVAIL - Not supported.
 * Notes:
 *      Individually setting BCAST limit is supported only on 5690 and later.
 */

int
bcm_esw_rate_bcast_set(int unit, int pps, int flags, int port)
{
    uint32 rate;
    int enable, threshold = pps;
    soc_reg_t reg;
    uint32 addr;

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_local_get(unit, port, &port));
    }
    if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PORT;
    }

#ifdef BCM_XGS3_SWITCH_SUPPORT
#ifdef BCM_TRX_SUPPORT
    if (SOC_IS_TRX(unit)) {
        return (_bcm_trx_rate_set(unit, port, _BCM_TRX_RATE_PKT_MODE, 
                             flags, BCM_RATE_BCAST, (uint32)pps, -1)); 
    }
#endif /* BCM_TRX_SUPPORT */

    if (SOC_IS_XGS3_SWITCH(unit)) {
        rate = 0;
        enable = (flags & BCM_RATE_BCAST) ? 1 : 0;
        soc_reg_field_set(unit, BCAST_STORM_CONTROLr,
                          &rate, ENABLEf, enable);
        soc_reg_field_set(unit, BCAST_STORM_CONTROLr,
                          &rate, THRESHOLDf, threshold);
        SOC_IF_ERROR_RETURN(WRITE_BCAST_STORM_CONTROLr(unit, port, rate));
        return BCM_E_NONE;
    }
#endif

    if (SOC_IS_XGS_SWITCH(unit)) {
        if (IS_E_PORT(unit, port)) { /* not IPIC port */
            if (soc_feature(unit, soc_feature_fast_rate_limit)) {
                threshold = _LYNX_PPS_TO_RATE(pps);
            }
            rate = 0;
            enable = (flags & BCM_RATE_BCAST) ? 1 : 0;
            reg = _rate_reg_get(unit, port, BCAST_RATE_CONTROLr);
            soc_reg_field_set(unit, reg, &rate, ENABLEf, enable);
            soc_reg_field_set(unit, reg, &rate, THRESHOLDf, threshold);

            addr = _rate_reg_addr_get(unit, port, BCAST_RATE_CONTROLr);
            SOC_IF_ERROR_RETURN(soc_reg32_set(unit, reg, REG_PORT_ANY, 
                                _rate_reg_index_get(unit, port), rate));
        }
        return BCM_E_NONE;
    }

    /*
     * StrataSwitch device supports only one rate for bcast, mcast, and dlfbc.
     * Must use bcm_rate_set() to set rate on StrataSwitch. 
     */ 

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_rate_dlfbc_set
 * Description:
 *      Configure rate limit for DLFBC packets
 * Parameters:
 *      unit - StrataSwitch PCI device unit number
 *      pps - Rate limit value in packets/second
 *      flags - Bitmask with one or more of BCM_RATE_*
 *      port - Port number for which DLFBC limit needs to be set
 * Returns:
 *      BCM_E_NONE - Success.
 *      BCM_E_UNAVAIL - Not supported.
 * Notes:
 *      DLFBC: Destination Lookup Failure Broadcast
 *      Individually setting DLFBC limit is supported only on 5690 and later.
 */

int
bcm_esw_rate_dlfbc_set(int unit, int pps, int flags, int port)
{
    uint32 rate;
    int enable, threshold = pps;
    soc_reg_t reg;
    uint32 addr;

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_local_get(unit, port, &port));
    }
    if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PORT;
    }

#ifdef BCM_XGS3_SWITCH_SUPPORT
#ifdef BCM_TRX_SUPPORT
    if (SOC_IS_TRX(unit)) {
        return (_bcm_trx_rate_set(unit, port, _BCM_TRX_RATE_PKT_MODE, 
                             flags, BCM_RATE_DLF, (uint32)pps, -1)); 
    }
#endif /* BCM_TRX_SUPPORT */

    if (SOC_IS_XGS3_SWITCH(unit)) {
        rate = 0;
        enable = (flags & BCM_RATE_DLF) ? 1 : 0;
        soc_reg_field_set(unit, DLFBC_STORM_CONTROLr,
                          &rate, ENABLEf, enable);
        soc_reg_field_set(unit, DLFBC_STORM_CONTROLr,
                          &rate, THRESHOLDf, threshold);
        SOC_IF_ERROR_RETURN(WRITE_DLFBC_STORM_CONTROLr(unit, port, rate));
        return BCM_E_NONE;
    }
#endif

    if (SOC_IS_XGS_SWITCH(unit)) {
        if (IS_E_PORT(unit, port)) { /* not IPIC port */
            if (soc_feature(unit, soc_feature_fast_rate_limit)) {
                threshold = _LYNX_PPS_TO_RATE(pps);
            }
            rate = 0;
            enable = (flags & BCM_RATE_DLF) ? 1 : 0;
            reg = _rate_reg_get(unit, port, DLF_RATE_CONTROLr);
            soc_reg_field_set(unit, reg, &rate, ENABLEf, enable);
            soc_reg_field_set(unit, reg, &rate, THRESHOLDf, threshold);

            addr = _rate_reg_addr_get(unit, port, DLF_RATE_CONTROLr);
            SOC_IF_ERROR_RETURN(soc_reg32_set(unit, reg, REG_PORT_ANY, 
                                _rate_reg_index_get(unit, port), rate));
        }
        return BCM_E_NONE;
    }

    /*
     * StrataSwitch device supports only one rate for bcast, mcast, and dlfbc.
     * Must use bcm_rate_set() to set rate on StrataSwitch. 
     */ 

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_rate_mcast_get
 * Description:
 *      Get rate limit for MCAST packets
 * Parameters:
 *      unit - StrataSwitch PCI device unit number
 *      pps - (OUT) Rate limit value in packets/second
 *      flags - (OUT) Bitmask with one or more of BCM_RATE_*
 *      port - Port number for which MCAST limit is requested
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Actually returns the MCAST rate for the given port for DRACO.
 *      However for STRATA this routine returns the MCAST for EPIC0,
 *      assuming all the PICS are programmed the same value.
 */

int
bcm_esw_rate_mcast_get(int unit, int *pps, int *flags, int port)
{
    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_local_get(unit, port, &port));
    }
    if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PORT;
    }

#ifdef BCM_XGS3_SWITCH_SUPPORT
#ifdef BCM_TRX_SUPPORT
    if (SOC_IS_TRX(unit)) {
        return (_bcm_trx_rate_get(unit, port, _BCM_TRX_RATE_PKT_MODE, 
                 (uint32 *)flags, BCM_RATE_MCAST, (uint32 *)pps, NULL));
    }
#endif /* BCM_TRX_SUPPORT */

    if (SOC_IS_XGS3_SWITCH(unit)) {
        uint32 rate;

        SOC_IF_ERROR_RETURN(READ_MCAST_STORM_CONTROLr(unit, port, &rate));
        *flags = 0;
        *pps = 0;
        if (soc_reg_field_get(unit, MCAST_STORM_CONTROLr, rate, ENABLEf)) {
            *flags |= BCM_RATE_MCAST;
            *pps = soc_reg_field_get(unit, MCAST_STORM_CONTROLr,
                                     rate, THRESHOLDf);
        } 
        return BCM_E_NONE;
    }
#endif
    if (SOC_IS_XGS_SWITCH(unit)) {
        uint32 rate;
        uint32 addr;
        soc_reg_t reg;

        if (IS_E_PORT(unit, port)) { /* not IPIC port */
            reg = _rate_reg_get(unit, port, MCAST_RATE_CONTROLr);
            addr = _rate_reg_addr_get(unit, port, reg);
            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, reg, REG_PORT_ANY, 
                                _rate_reg_index_get(unit, port), &rate));
            *pps = 0;
            *flags = 0;
            if (soc_reg_field_get(unit, reg, rate, ENABLEf)) {
                *flags |= BCM_RATE_MCAST;
                *pps = soc_reg_field_get(unit, reg, rate, THRESHOLDf);
                if (soc_feature(unit, soc_feature_fast_rate_limit)) {
                    *pps = _LYNX_RATE_TO_PPS(*pps);
                }
            }
        }
        return BCM_E_NONE;
    }

    return bcm_esw_rate_get(unit, pps, flags);
}

/*
 * Function:
 *      bcm_rate_dlfbc_get
 * Description:
 *      Get rate limit for DLFBC packets
 * Parameters:
 *      unit - StrataSwitch PCI device unit number
 *      pps - (OUT) Rate limit value in packets/second
 *      flags - (OUT) Bitmask with one or more of BCM_RATE_*
 *      port - Port number for which DLFBC limit is requested
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      DLFBC: Destination Lookup Failure Broadcast
 *      Actually returns the DLFBC rate for the given port for DRACO.
 *      However for STRATA this routine returns the DLFBC for EPIC0,
 *      assuming all the PICS are programmed the same value.
 */

int
bcm_esw_rate_dlfbc_get(int unit, int *pps, int *flags, int port)
{
    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_local_get(unit, port, &port));
    }
    if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PORT;
    }

#ifdef BCM_XGS3_SWITCH_SUPPORT
#ifdef BCM_TRX_SUPPORT
    if (SOC_IS_TRX(unit)) {
        return (_bcm_trx_rate_get(unit, port, _BCM_TRX_RATE_PKT_MODE, 
                     (uint32 *)flags, BCM_RATE_DLF, (uint32 *)pps, NULL));
    }
#endif /* BCM_TRX_SUPPORT */
    if (SOC_IS_XGS3_SWITCH(unit)) {
        uint32 rate;

        SOC_IF_ERROR_RETURN(READ_DLFBC_STORM_CONTROLr(unit, port, &rate));
 
        *flags = 0;
        *pps = 0;
        if (soc_reg_field_get(unit, DLFBC_STORM_CONTROLr, rate, ENABLEf)) {
            *flags |= BCM_RATE_DLF;
            *pps = soc_reg_field_get(unit, DLFBC_STORM_CONTROLr,
                                     rate, THRESHOLDf);
        }
        return BCM_E_NONE;
    }
#endif

    if (SOC_IS_XGS_SWITCH(unit)) {
        uint32 rate;
        soc_reg_t reg;
        uint32 addr;

        if (IS_E_PORT(unit, port)) { /* not IPIC port */
            reg = _rate_reg_get(unit, port, DLF_RATE_CONTROLr);
            addr = _rate_reg_addr_get(unit, port, reg);
            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, reg, REG_PORT_ANY, 
                                _rate_reg_index_get(unit, port), &rate));

            *pps = 0;
            *flags = 0;
            if (soc_reg_field_get(unit, reg, rate, ENABLEf)) {
                *flags |= BCM_RATE_DLF;
                *pps = soc_reg_field_get(unit, reg, rate, THRESHOLDf);
                if (soc_feature(unit, soc_feature_fast_rate_limit)) {
                    *pps = _LYNX_RATE_TO_PPS(*pps);
                } 
            }
        }
        return BCM_E_NONE;
    }

    return bcm_esw_rate_get(unit, pps, flags);
}

/*
 * Function:
 *      bcm_rate_bcast_get
 * Description:
 *      Get rate limit for BCAST packets
 * Parameters:
 *      unit - StrataSwitch PCI device unit number
 *      pps - (OUT) Rate limit value in packets/second
 *      flags - (OUT) Bitmask with one or more of BCM_RATE_*
 *      port - Port number for which BCAST limit is requested
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Actually returns the BCAST rate for the given port for DRACO.
 *      However for STRATA this routine returns the BCAST for EPIC0,
 *      assuming all the PICS are programmed the same value.
 */

int
bcm_esw_rate_bcast_get(int unit, int *pps, int *flags, int port)
{
    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_local_get(unit, port, &port));
    }
    if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PORT;
    }

#ifdef BCM_XGS3_SWITCH_SUPPORT
#ifdef BCM_TRX_SUPPORT
    if (SOC_IS_TRX(unit)) {
        return (_bcm_trx_rate_get(unit, port, _BCM_TRX_RATE_PKT_MODE, 
                    (uint32 *)flags, BCM_RATE_BCAST, (uint32 *)pps, NULL));
    }
#endif /* BCM_TRX_SUPPORT */
    if (SOC_IS_XGS3_SWITCH(unit)) {
        uint32 rate;

        SOC_IF_ERROR_RETURN(READ_BCAST_STORM_CONTROLr(unit, port, &rate));
        *flags = 0;
        *pps = 0;
        if (soc_reg_field_get(unit, BCAST_STORM_CONTROLr, rate, ENABLEf)) {
            *flags |= BCM_RATE_BCAST;
            *pps = soc_reg_field_get(unit, BCAST_STORM_CONTROLr,
                                     rate, THRESHOLDf);
        }
        return BCM_E_NONE;
    }
#endif

    if (SOC_IS_XGS_SWITCH(unit)) {
        uint32 rate;
        soc_reg_t reg;
        uint32 addr;

        if (IS_E_PORT(unit, port)) { /* not IPIC port */
            reg = _rate_reg_get(unit, port, BCAST_RATE_CONTROLr);
            addr = _rate_reg_addr_get(unit, port, reg);
            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, reg, REG_PORT_ANY, 
                                _rate_reg_index_get(unit, port), &rate));

            *pps = 0;
            *flags = 0;
            if (soc_reg_field_get(unit, reg, rate, ENABLEf)) {
                *flags |= BCM_RATE_BCAST;
                *pps = soc_reg_field_get(unit, reg, rate, THRESHOLDf);
                if (soc_feature(unit, soc_feature_fast_rate_limit)) {
                    *pps = _LYNX_RATE_TO_PPS(*pps);
                }
            }
        }
        return BCM_E_NONE;
    }

    return bcm_esw_rate_get(unit, pps, flags);
}

/*
 * Function:
 *      bcm_rate_type_get
 * Description:
 *      Front end to bcm_*cast_rate_get functions.
 *      Uses a single data structure to read from
 *      all the 3 rate control registers
 * Parameters:
 *      unit - unit number
 *      rl - (OUT) data structure containing info to be acquired from the
 *           rate control registers
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_esw_rate_type_get(int unit, bcm_rate_limit_t *rl)
{
    int port;
    int flags = 0;

    bcm_rate_limit_t_init(rl);

    /* get first enabled ethernet port */
    PBMP_E_ITER(unit, port) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_rate_bcast_get(unit, &rl->br_bcast_rate, &flags, port));
        rl->flags |= flags;
        BCM_IF_ERROR_RETURN
            (bcm_esw_rate_mcast_get(unit, &rl->br_mcast_rate, &flags, port));
        rl->flags |= flags;
        BCM_IF_ERROR_RETURN
            (bcm_esw_rate_dlfbc_get(unit, &rl->br_dlfbc_rate, &flags, port));
        rl->flags |= flags;
        break;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_rate_get
 * Description:
 *      Get rate limit and on/off state of
 *      DLF, MCAST, and BCAST limiting
 * Parameters:
 *      unit - StrataSwitch PCI device unit number
 *      pps - (OUT) Place to store returned rate limit value
 *      flags - (OUT) Place to store returned flag bitmask with
 *              one or more of BCM_RATE_*
 * Returns:
 *      BCM_E_NONE - Success.
 *      BCM_E_INTERNAL - Chip access failure.
 * Notes:
 *      Actually returns the rate for BCAST only, but assumes the
 *      bcm_rate_set call was used so DLF, MCAST and BCAST should be equal.
 */

int
bcm_esw_rate_get(int unit, int *pps, int *flags)
{
    uint32      rate;
    soc_reg_t reg;
    int port;

#ifdef BCM_XGS3_SWITCH_SUPPORT
    if (SOC_IS_XGS3_SWITCH(unit)) {
        PBMP_ALL_ITER(unit, port) {
            break;
        }
#ifdef BCM_TRX_SUPPORT
        if (SOC_IS_TRX(unit)) {
            return (_bcm_trx_rate_get(unit, port, _BCM_TRX_RATE_PKT_MODE, 
                         (uint32 *)flags, BCM_RATE_ALL, (uint32 *)pps, NULL));
        }
#endif /* BCM_TRX_SUPPORT */

        *flags = 0;
        SOC_IF_ERROR_RETURN(READ_BCAST_STORM_CONTROLr(unit, port, &rate));
        if (soc_reg_field_get(unit, BCAST_STORM_CONTROLr, rate, ENABLEf)) {
            *pps = soc_reg_field_get(unit, BCAST_STORM_CONTROLr, rate, THRESHOLDf);
            *flags |= BCM_RATE_BCAST;
        }
        SOC_IF_ERROR_RETURN(READ_MCAST_STORM_CONTROLr(unit, port, &rate));
        if (soc_reg_field_get(unit, MCAST_STORM_CONTROLr, rate, ENABLEf)) {
            *pps = soc_reg_field_get(unit, MCAST_STORM_CONTROLr, rate, THRESHOLDf);
            *flags |= BCM_RATE_MCAST;
        }
        SOC_IF_ERROR_RETURN(READ_DLFBC_STORM_CONTROLr(unit, port, &rate));
        if (soc_reg_field_get(unit, DLFBC_STORM_CONTROLr, rate, ENABLEf)) {
            *pps = soc_reg_field_get(unit, DLFBC_STORM_CONTROLr, rate, THRESHOLDf);
            *flags |= BCM_RATE_DLF;
        }
        return BCM_E_NONE;
    }
#endif

    if (SOC_IS_XGS_SWITCH(unit)) {
        /* Get first ether port */
        PBMP_E_ITER(unit, port) {
            break;
        }

        reg = _rate_reg_get(unit, port, BCAST_RATE_CONTROLr);
        SOC_IF_ERROR_RETURN(soc_reg_read_any_block(unit, reg, &rate));
        *pps = soc_reg_field_get(unit, reg, rate, THRESHOLDf);
        if (soc_feature(unit, soc_feature_fast_rate_limit)) {
            *pps = _LYNX_RATE_TO_PPS(*pps);
        }
        *flags = 0;
        if (soc_reg_field_get(unit, reg, rate, ENABLEf)) {
            *flags |= BCM_RATE_BCAST;
        }

        reg = _rate_reg_get(unit, port, MCAST_RATE_CONTROLr);
        SOC_IF_ERROR_RETURN(soc_reg_read_any_block(unit, reg, &rate));
        if (soc_reg_field_get(unit, reg, rate, ENABLEf)) {
            *flags |= BCM_RATE_MCAST;
        }

        reg = _rate_reg_get(unit, port, DLF_RATE_CONTROLr);
        SOC_IF_ERROR_RETURN(soc_reg_read_any_block(unit, reg, &rate));
        if (soc_reg_field_get(unit, reg, rate, ENABLEf)) {
            *flags |= BCM_RATE_DLF;
        }

        return BCM_E_NONE;
    }

    return BCM_E_UNAVAIL;
}


/*
 * Function:
 *      bcm_esw_rate_bandwidth_get
 * Description:
 *      Get rate bandwidth limiting parameters
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      flags - Bitmask with one of the following:
 *              BCM_RATE_BCAST
 *              BCM_RATE_MCAST
 *              BCM_RATE_DLF
 *      kbits_sec - Rate in kilobits (1000 bits) per second.
 *                  Rate of 0 disabled rate limiting.
 *      kbits_burst - Maximum burst size in kilobits(1000 bits)
 * Returns:
 *      BCM_E_NONE - Success.
 *      BCM_E_UNAVAIL - Not supported
 *      BCM_E_XXX - Error.
*/

int 
bcm_esw_rate_bandwidth_get(int unit, bcm_port_t port, int flags, 
                       uint32 *kbits_sec, uint32 *kbits_burst)
{

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_local_get(unit, port, &port));
    }
#ifdef BCM_TRX_SUPPORT
    if (SOC_IS_TRX(unit)) {
        return (_bcm_trx_rate_get(unit, port, _BCM_TRX_RATE_BYTE_MODE, 
                                  NULL, (uint32)flags, kbits_sec, 
                                  kbits_burst));
    } 
#endif /* BCM_TRX_SUPPORT */

#if defined(BCM_BANDWIDTH_RATE_METER)
    if (soc_feature(unit,soc_feature_storm_control)) {
        uint32  enable_entry;

        if (!SOC_PORT_VALID(unit, port)) {
            return BCM_E_PORT;
        }

        *kbits_sec = *kbits_burst = 0;
        BCM_IF_ERROR_RETURN(
            READ_SC_BYTE_METER_CONFIGr(unit, port, &enable_entry));

        if (flags & BCM_RATE_BCAST) {
            if (soc_reg_field_get(unit, SC_BYTE_METER_CONFIGr, enable_entry,
                                  BCAST_ENABLEf)) {
                _bcm_rate_bw_get(unit, port, FP_SC_BCAST_METER_TABLEm, 
                                 kbits_sec, kbits_burst);
            }
        } else if (flags & BCM_RATE_MCAST) {
            if (soc_reg_field_get(unit, SC_BYTE_METER_CONFIGr, enable_entry,
                                  MCAST_ENABLEf)) {
                _bcm_rate_bw_get(unit, port, FP_SC_MCAST_METER_TABLEm, 
                                 kbits_sec, kbits_burst);
            }
        } else if (flags & BCM_RATE_DLF) {
            if (!SOC_IS_SHADOW(unit)) {
                if (soc_reg_field_get(unit, SC_BYTE_METER_CONFIGr, enable_entry,
                                      DLFBC_ENABLEf)) {
                    _bcm_rate_bw_get(unit, port, FP_SC_DLF_METER_TABLEm,
                                     kbits_sec, kbits_burst);
                }
            }
        }

        return BCM_E_NONE;
    }
#endif
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_esw_rate_bandwidth_set
 * Description:
 *      Set rate bandwidth limiting parameters
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      flags - Bitmask with one of the following:
 *              BCM_RATE_BCAST
 *              BCM_RATE_MCAST
 *              BCM_RATE_DLF
 *      kbits_sec - Rate in kilobits (1000 bits) per second.
 *                  Rate of 0 disables rate limiting.
 *      kbits_burst - Maximum burst size in kilobits(1000 bits)
 * Returns:
 *      BCM_E_NONE - Success.
 *      BCM_E_UNAVAIL - Not supported
 *      BCM_E_XXX - Error.
*/

int 
bcm_esw_rate_bandwidth_set(int unit, bcm_port_t port, int flags, 
                       uint32 kbits_sec, uint32 kbits_burst)
{
    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_local_get(unit, port, &port));
    }
#ifdef BCM_TRX_SUPPORT
    if (SOC_IS_TRX(unit)) {
        uint32 fmask = flags;

        if (!kbits_sec) { /* disable storm control */
            flags = 0;
        }
        return (_bcm_trx_rate_set(unit, port, _BCM_TRX_RATE_BYTE_MODE, 
                                flags, fmask, kbits_sec, kbits_burst)); 
    } 
#endif /* BCM_TRX_SUPPORT */

#if defined(BCM_BANDWIDTH_RATE_METER)

    if (soc_feature(unit,soc_feature_storm_control)) {
        uint32      enable_entry, old_entry;
        uint32      refresh;

        if (!SOC_PORT_VALID(unit, port)) {
            return BCM_E_PORT;
        }
        if (kbits_sec > METERING_TOKEN_MAX * TOKEN_BUCKET_SIZE) {
            return BCM_E_PARAM;
        }
        if (kbits_burst > kbits_2_bucket_size[METER_BUCKETSIZE_MAX]) {
            return BCM_E_PARAM;
        }

        if (!(flags & BCM_RATE_ALL)) {
            return BCM_E_PARAM;
        }

    /*    coverity[equality_cond]    */
        if (!kbits_sec || !kbits_burst) { 
            /* metering is disabled on port - program tables to max values */
            BCM_IF_ERROR_RETURN(
                READ_SC_BYTE_METER_CONFIGr(unit, port, &enable_entry));
            
            old_entry = enable_entry;
            if (flags & BCM_RATE_BCAST) {
                soc_reg_field_set(unit, SC_BYTE_METER_CONFIGr, &enable_entry,
                                  BCAST_ENABLEf, 0);

                _bcm_rate_bw_set(unit, port, FP_SC_BCAST_METER_TABLEm, 
                                 METERING_TOKEN_MAX, METER_BUCKETSIZE_MAX); 
            }
            if (flags & BCM_RATE_MCAST) {
                soc_reg_field_set(unit, SC_BYTE_METER_CONFIGr, &enable_entry,
                                  MCAST_ENABLEf, 0);

                _bcm_rate_bw_set(unit, port, FP_SC_MCAST_METER_TABLEm, 
                                 METERING_TOKEN_MAX, METER_BUCKETSIZE_MAX); 
            }
            if (flags & BCM_RATE_DLF) {
                if (!SOC_IS_SHADOW(unit)) {
                    soc_reg_field_set(unit, SC_BYTE_METER_CONFIGr, &enable_entry,
                                      DLFBC_ENABLEf, 0);

                    _bcm_rate_bw_set(unit, port, FP_SC_DLF_METER_TABLEm, 
                                     METERING_TOKEN_MAX, METER_BUCKETSIZE_MAX); 
                }
            }
            if (old_entry != enable_entry) {
                BCM_IF_ERROR_RETURN(
                    WRITE_SC_BYTE_METER_CONFIGr(unit, port, enable_entry));
            }

            return BCM_E_NONE; 
        }

        refresh = (kbits_sec + TOKEN_BUCKET_SIZE - 1) / TOKEN_BUCKET_SIZE ;
        if ( refresh > METERING_TOKEN_MAX){
            refresh = METERING_TOKEN_MAX; 
        }

        BCM_IF_ERROR_RETURN(
            READ_SC_BYTE_METER_CONFIGr(unit, port, &enable_entry));

        old_entry = enable_entry;

        if (flags & BCM_RATE_BCAST) {
            soc_reg_field_set(unit, SC_BYTE_METER_CONFIGr, &enable_entry,
                              BCAST_ENABLEf, 1);

            _bcm_rate_bw_set(unit, port, FP_SC_BCAST_METER_TABLEm, refresh, 
                             _bcm_kbits_to_bucket_size(kbits_burst)); 
        }
        if (flags & BCM_RATE_MCAST) {
            soc_reg_field_set(unit, SC_BYTE_METER_CONFIGr, &enable_entry,
                              MCAST_ENABLEf, 1);

            _bcm_rate_bw_set(unit, port, FP_SC_MCAST_METER_TABLEm, refresh, 
                             _bcm_kbits_to_bucket_size(kbits_burst)); 
        }
        if (flags & BCM_RATE_DLF) {
            if (!SOC_IS_SHADOW(unit)) {
                soc_reg_field_set(unit, SC_BYTE_METER_CONFIGr, &enable_entry,
                                  DLFBC_ENABLEf, 1);

                _bcm_rate_bw_set(unit, port, FP_SC_DLF_METER_TABLEm, refresh, 
                                 _bcm_kbits_to_bucket_size(kbits_burst)); 
            }
        }
        if (old_entry != enable_entry) {
            BCM_IF_ERROR_RETURN(
                WRITE_SC_BYTE_METER_CONFIGr(unit, port, enable_entry));
        }

        return BCM_E_NONE;
    }
#endif

    return BCM_E_UNAVAIL;
}
