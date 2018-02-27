
/*****************************************************************************
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
 *
 * $Id: sbFe2000Common.c 1.10.60.1 Broadcom SDK $
 * ******************************************************************************/
#include "glue.h"
#include "soc/drv.h"
#include "soc/sbx/sbx_drv.h"
#include "hal_user.h"
#include "hal_ca_auto.h"
#include "hal_c2_auto.h"
#include "hal_ca_c2.h"
#include "sbWrappers.h"
#include "sbFe2000CommonUtil.h"


#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif


void
sbFe2000Reset(sbhandle userDeviceHandle)
{
  sbFe2000UtilApplySoftReset(userDeviceHandle);
  thin_delay(SB_FE2000_200_MSEC_K);
  sbFe2000UtilReleaseSoftReset(userDeviceHandle);
}

int
sbFe2000TranslateBlock(int blk_type, int blk_number)
{
    int i = 0;

    switch (blk_type) {
    case SOC_BLK_GPORT:
        i = SB_FE2000_IF_PTYPE_AGM0;
        break;
    case SOC_BLK_XPORT:
        if (blk_number >= SB_FE2000_NUM_XG_INTERFACES) {
            i = SB_FE2000_IF_PTYPE_AGM0 - SB_FE2000_NUM_XG_INTERFACES;
        }else{
            i = SB_FE2000_IF_PTYPE_XGM0;
        }
        break;
    case SOC_BLK_SPI:
        i = SB_FE2000_IF_PTYPE_SPI0;
        break;
    case SOC_BLK_CMIC:
        i = SB_FE2000_IF_PTYPE_PCI;
    }

    return i + blk_number;
}


int ceilx(double x, double factor) 
{
    return (int)((x + factor) / factor) * factor;
}


#define FE2K_SWS_PAGE_SIZE   192     /* 192B per sws page */
#define FE2K_SWS_LATENCY     1000    /* ns */
#define FE2K_PKT_RATE_64     672     /* 64B pkt rate at 1G */

/*
 *  Function:
 *    sbFe2000SwsConfigCompute
 *  Purpose:
 *    Compute the optimal thresholds, min/max pages for ingress/egress sws
 *   queues for a port, given a rate and MTU.
 *  Parameters:
 *    (IN)   hpp_freq      - HPP core frequency, in MHz
 *    (IN)   epoch         - ucode epoch length
 *    (IN)   contexts      - number contexts supported by device (C1=12 C2=16)
 *    (IN)   mtu           - MTU for this port
 *    (IN)   kbits_sec     - port rate
 *    (OUT)  mac_to_hpp    - Queue params for the direction indicated
 *    (OUT)  hpp_to_spi    - Queue params for the direction indicated
 *    (OUT)  spi_to_hpp    - Queue params for the direction indicated
 *    (OUT)  hpp_to_mac    - Queue params for the direction indicated
 */
int
sbFe2000SwsConfigCompute(uint32_t hpp_freq, uint32_t epoch,
                         uint32_t contexts, uint32_t mtu,
                         uint32_t kbits_sec, uint32_t higig,
                         sbFe2000QmParams_t *mac_to_hpp,
                         sbFe2000QmParams_t *hpp_to_spi,
                         sbFe2000QmParams_t *spi_to_hpp,
                         sbFe2000QmParams_t *hpp_to_mac)
{

    double   clock_period;
    uint32_t page_sz     = FE2K_SWS_PAGE_SIZE;
    uint32_t sws_latency = FE2K_SWS_LATENCY;
    double   pkt_rate_64 = FE2K_PKT_RATE_64;
    uint32_t overshoot = 64 * 16;
    uint32_t frames_per_epoch = 48;
    uint32_t batch_latency, factor;
    double   dev_latency;
    uint32_t jumbo_pages = ceilx((mtu / page_sz), 1);
    double   min_thresh, jumbo_thresh, output_thresh;
    uint32_t lines_per_ts;
    uint32_t ts_pages;
    uint32_t e2ecc_period = 256;

    clock_period = (1.0 / hpp_freq * 1000.0);
    /*
     * adjust overshoot based on interface type and speed
     * the default value above works for SPI interfaces
     */
    if (higig) {
        if (kbits_sec < 10e6 ) {
            overshoot = (((mtu + 20)/(64+20))*page_sz) + 
                        ((e2ecc_period * 10 * clock_period)/(8 * 84)) * page_sz +
                         mtu + 158; 
        }else{
            overshoot = (((mtu + 20)/(64+20))*page_sz) +
                        ((e2ecc_period * 10 * clock_period)/(8 * 84)) * page_sz +
                        mtu + 1259;
        }
    }

    /* could make lines_per_ts a soc property, for now hardcode 
     * to a reasonable value
     */
    lines_per_ts = 208;

    batch_latency = 2 * epoch * clock_period;
    dev_latency = (2.0 * sws_latency + 
                   (contexts * epoch * clock_period) + batch_latency);

    jumbo_thresh = (double)jumbo_pages + (dev_latency / (page_sz * 8.0)) * (kbits_sec / 1e6);
    min_thresh   = 1.0 + (double)dev_latency / pkt_rate_64  * (kbits_sec / 1e6);

    ts_pages = ((16 * lines_per_ts) / 64) / 2;
    output_thresh = (dev_latency / pkt_rate_64 * (kbits_sec / 1e6)) + ts_pages;

    mac_to_hpp->min_pages = jumbo_pages;

    /* Less than 1G treated different - bounded by MTU, mostly */
    if (kbits_sec < 1e6 ) {
        factor = 1;
    } else {
        factor = 10;
    }

    spi_to_hpp->threshold = ceilx((MAX(jumbo_thresh, min_thresh)), factor);
    spi_to_hpp->min_pages = jumbo_pages;
    spi_to_hpp->max_pages = spi_to_hpp->threshold + ceilx(overshoot/page_sz, 10);

    hpp_to_mac->threshold = ceilx(output_thresh, 10);

    /* 10G treated different */
    if (kbits_sec >= 10e6) {
        mac_to_hpp->threshold = ceilx(MAX(jumbo_thresh, min_thresh), 10);

        hpp_to_spi->min_pages = frames_per_epoch * 5;

        if (!higig) 
        {
            spi_to_hpp->max_pages = (spi_to_hpp->threshold + 
                                     ceilx(overshoot/page_sz, 1) * 10);
        }


        hpp_to_mac->min_pages = 10;
        hpp_to_mac->max_pages = hpp_to_mac->threshold + 64;
    } else {

        mac_to_hpp->threshold = ceilx(MAX(jumbo_thresh, min_thresh), 20);

        hpp_to_spi->min_pages = frames_per_epoch;

        hpp_to_mac->min_pages = 10;
        hpp_to_mac->max_pages = hpp_to_mac->threshold + 8;
    }

    mac_to_hpp->max_pages = jumbo_pages + mac_to_hpp->threshold;

    hpp_to_spi->threshold = 0;
    hpp_to_spi->max_pages = hpp_to_spi->min_pages;

    return SOC_E_NONE;
}



/*
 *  Function:
 *    sbFe2000SwsConfigGet
 *  Purpose:
 *    Compute the rate and mtu based on configured params
 *  Parameters:
 *    (IN)   hpp_freq      - HPP core frequency, in MHz
 *    (IN)   epoch         - ucode epoch length
 *    (IN)   contexts      - number contexts supported by device (C1=12 C2=16)
 *    (IN)   hpp_to_mac_thresh - used to compute configured port 
 *    (IN)   spi_to_hpp_min_pages - used to compute MTU
 *    (OUT)  mtu           - MTU for this port
 *    (OUT)  kbits_sec     - port rate
 */
int
sbFe2000SwsConfigGet(uint32_t hpp_freq, uint32_t epoch, uint32_t contexts,
                     uint32_t hpp_to_mac_thresh, uint32_t spi_to_hpp_min_pages,
                     uint32_t *mtu, uint32_t *kbits_sec)
{
    double   clock_period;
    uint32_t page_sz     = FE2K_SWS_PAGE_SIZE;
    uint32_t sws_latency = FE2K_SWS_LATENCY;
    double   pkt_rate_64 = FE2K_PKT_RATE_64;
    uint32_t batch_latency;
    double   dev_latency;
  
    clock_period = (1.0 / hpp_freq * 1000.0);
    batch_latency = 2 * epoch * clock_period;
    dev_latency = (2.0 * sws_latency + 
                   (contexts * epoch * clock_period) + batch_latency);
 
    *mtu = spi_to_hpp_min_pages * page_sz;
    *kbits_sec = hpp_to_mac_thresh / dev_latency * pkt_rate_64 * 1e6;

    return SOC_E_NONE;
}
