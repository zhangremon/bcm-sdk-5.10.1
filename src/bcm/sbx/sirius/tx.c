/*
 * $Id: tx.c 1.12 Broadcom SDK $
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
 * File:        tx.c
 */

#include <soc/debug.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/sbx_txrx.h>
#include <soc/higig.h>

#include <bcm/error.h>
#include <bcm/tx.h>
#include <bcm/debug.h>

#include <bcm_int/control.h>


#define _SS_TX_D(string)   "[%d:%s]: " string, unit, FUNCTION_NAME()

#define TX_DEBUG(flags, stuff)  BCM_DEBUG(flags | BCM_DBG_TX, stuff)
#define TX_OUT(stuff)           TX_DEBUG(BCM_DBG_TX, stuff)
#define TX_WARN(stuff)          TX_DEBUG(BCM_DBG_WARN, stuff)
#define TX_ERR(stuff)           TX_DEBUG(BCM_DBG_ERR, stuff)
#define TX_VERB(stuff)          TX_DEBUG(BCM_DBG_VERBOSE, stuff)

#define _ARRAY_SZ(x) (sizeof(x) / sizeof(x[0]))

#define G2_FE_EGRROUTEHEADER_SIZE_IN_BYTES 12

int sbx_sirius_tx_pkt_setup(int unit, bcm_pkt_t *tx_pkt)
{
    int i, port, hdr_size = 0;
    soc_higig_hdr_t *pkt_hg_hdr = (soc_higig_hdr_t *) tx_pkt->pkt_data->data;
    uint32_t baseQueue = 0;
#ifdef BCM_FE2000_SUPPORT
    uint32_t queue;
#endif

    if (!BCM_PKT_TX_ETHER(tx_pkt)) {
      if (!BCM_PKT_TX_HG_READY(tx_pkt)) {
	  hdr_size = SOC_HIGIG2_HDR_SIZE;
	  sal_memcpy(tx_pkt->_higig, pkt_hg_hdr, hdr_size);
      }
      
      /*
       * Now that the higig header has been moved to the _higig buffer,
       * move the data to the beginning of the buffer
       */
      
      for (i=hdr_size; i < tx_pkt->pkt_data->len; i++) 
	tx_pkt->pkt_data->data[i-hdr_size] = tx_pkt->pkt_data->data[i];
      
      /*
       * zero the remaining buffer 
       */

      sal_memset(&tx_pkt->pkt_data->data[i - hdr_size], 0, hdr_size);
      
      /*
       * Remove the header size from the packet length
       */
      
      tx_pkt->pkt_data->len -= hdr_size;
      tx_pkt->tot_len -= hdr_size;

    } else {

      /*
       * Has SBX RH
       */

      hdr_size = _ARRAY_SZ(tx_pkt->_sbx_rh);
      sal_memset(tx_pkt->_sbx_rh, 0, hdr_size);
	  
      switch (SOC_SBX_CFG(unit)->erh_type) {
          case SOC_SBX_G2P3_ERH_SIRIUS:
              tx_pkt->_sbx_hdr_len = SOC_SBX_G2P3_ERH_LEN_SIRIUS;
          break;
          case SOC_SBX_G2P3_ERH_DEFAULT:
          default:
              tx_pkt->_sbx_hdr_len = SOC_SBX_G2P3_ERH_LEN_DEFAULT;
          break;
      }

      /*
       * If dest_mod uninitialized, 
       * use default
       */
      if (tx_pkt->dest_mod == 0)
	tx_pkt->dest_mod = SBX_QE_BASE_MODID;

      /* should only occur once - */
      BCM_PBMP_ITER(tx_pkt->tx_pbmp, port) { 
        int node;
	
        SOC_SBX_NODE_FROM_MODID(tx_pkt->dest_mod, node);
        if (node < 0) {
	  TX_ERR((_SS_TX_D("invalid destination module: %d; must be a QE modid\n"),
		  tx_pkt->dest_mod));
	  return BCM_E_PARAM;
        }
        baseQueue = SOC_SBX_NODE_PORT_TO_QID(unit,node, port, NUM_COS(unit));
      }
      
#ifdef BCM_FE2000_SUPPORT
      /* set a non-existant source id to avoid split horizion checks on the FE */
      soc_sbx_hdr_field_set(unit, tx_pkt->_sbx_rh, tx_pkt->_sbx_hdr_len,
			    SBX_rhf_sid, SBX_MAX_PORTS);
      
      if (tx_pkt->cos > NUM_COS(unit)) {
        return BCM_E_PARAM;
      }
      
      /* set KSOP */
      soc_sbx_hdr_field_set(unit, tx_pkt->_sbx_rh, tx_pkt->_sbx_hdr_len,
                            SBX_rhf_ksop, SBX_MAX_PORTS);

      /* default queue will be lowest priority (baseQueue + NUM_COS -1;
       * decrease queue number to increas priority
       */
      queue = baseQueue + ((NUM_COS(unit) - 1) - tx_pkt->cos);
      TX_OUT((_SS_TX_D("RH QID=0x%04x cos=%d baseQid=0x%04x\n"),
	      queue, tx_pkt->cos, baseQueue));
      
      soc_sbx_hdr_field_set(unit, tx_pkt->_sbx_rh, tx_pkt->_sbx_hdr_len,
			    SBX_rhf_queue_id, queue);
      if (!BCM_PKT_NO_VLAN_TAG(tx_pkt) &&		\
	  BCM_VLAN_VALID(BCM_PKT_VLAN_ID(tx_pkt))) {
        soc_sbx_hdr_field_set(unit, tx_pkt->_sbx_rh, tx_pkt->_sbx_hdr_len,
			      SBX_rhf_outunion, BCM_PKT_VLAN_ID(tx_pkt));
      } else {
        soc_sbx_hdr_field_set(unit, tx_pkt->_sbx_rh, tx_pkt->_sbx_hdr_len,
			      SBX_rhf_outunion, SBX_RAW_OHI_BASE);
      }
      
      if (tx_pkt->opcode & BCM_PKT_OPCODE_MC) {
        soc_sbx_hdr_field_set(unit, tx_pkt->_sbx_rh, tx_pkt->_sbx_hdr_len,
                              SBX_rhf_mc, TRUE);
      }
      
      soc_sbx_hdr_field_set(unit, tx_pkt->_sbx_rh, hdr_size, SBX_rhf_test, 
			    !!(tx_pkt->flags & BCM_PKT_F_TEST));
      
#endif /* BCM_FE2000_SUPPORT */
      
    }
    return BCM_E_NONE;
}
