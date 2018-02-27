/*
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
 * $Id: g2xx_pp.c 1.4.110.1 Broadcom SDK $
 *
 *-----------------------------------------------------------------------------*/
#include <soc/sbx/g2xx/g2xx_int.h>
#include <soc/sbx/fe2k_common/sbFe2000CommonDriver.h>
#include <soc/sbx/fe2k/sbFe2000Util.h>
#include <soc/sbx/fe2k/sbFe2000Pp.h> 

#define SB_G2XX_FE_PP_CSTATE_PID_SET(x,y) (x = (x & 0xC0FFFF)| (y << 16))
#define SB_G2XX_FE_PP_CSTATE_PID_GET(x) ((x & (0x3F << 16)) >> 16)

#define SB_G2XX_FE_PP_WIDTH_TO_MASK(X) (\
	       	((X) == 32) ? (0xFFFFFFFF) : ( (1 << (X))  -1) )

static sbStatus_t
soc_sbx_g2xx_pp_broad_shield_check_set_get( soc_sbx_g2xx_state_t *pFe,
                                int eType, uint8_t *pbEnable, uint8_t bSetGet);

static sbStatus_t
soc_sbx_g2xx_pp_rule_enable_set(int unit, uint32_t  uValid, int uCamId, int uEntry);

static sbStatus_t
soc_sbx_g2xx_pp_rule_enable_get(int unit, uint32_t *uValid, int uCamId, int uEntry);

static sbStatus_t
soc_sbx_g2xx_lsm_set_ext( int unit, int uCamId, int uEntry, int uOffset,
         int  nPortId, soc_sbx_g2xx_6_byte_t *pData, soc_sbx_g2xx_6_byte_t *pMask);

static sbStatus_t
soc_sbx_g2xx_lsm_get_ext( int unit, int uCamId, int uEntry, int uOffset,
         int *pPortId, soc_sbx_g2xx_6_byte_t *pData, soc_sbx_g2xx_6_byte_t *pMask);

/*
 * Rule Enable & Disable
 */

sbStatus_t
soc_sbx_g2xx_pp_v4mc_str_sel_match_set(int unit, uint32_t uValid)
{
    int uCamId = SB_G2XX_FE_PP_RULE_S0_ETH_IPMULTICAST_CAM_ID;
    int uEntry = SB_G2XX_FE_PP_RULE_S0_ETH_IPMULTICAST_ENTRY;
    return (soc_sbx_g2xx_pp_rule_enable_set(unit, uValid, uCamId, uEntry));
}

sbStatus_t
soc_sbx_g2xx_pp_v4mc_str_sel_match_get(int unit, uint32_t* uValid)
{
    int uCamId = SB_G2XX_FE_PP_RULE_S0_ETH_IPMULTICAST_CAM_ID;
    int uEntry = SB_G2XX_FE_PP_RULE_S0_ETH_IPMULTICAST_ENTRY;
    return (soc_sbx_g2xx_pp_rule_enable_get(unit, uValid, uCamId, uEntry));
}

sbStatus_t
soc_sbx_g2xx_pp_v4uc_str_sel_match_set(int unit, uint32_t uValid)
{
    sbStatus_t rv;
    int uCamId = SB_G2XX_FE_PP_RULE_S7_L4_NO_HASH_ROUTE_CAM_ID;
    int uEntry = SB_G2XX_FE_PP_RULE_S7_L4_NO_HASH_ROUTE_ENTRY;

    rv = (soc_sbx_g2xx_pp_rule_enable_set(unit, uValid, uCamId, uEntry));

    if (rv != SOC_E_NONE) {
        return rv;
    }

    uCamId = SB_G2XX_FE_PP_RULE_S7_L4_HASH_ROUTE_CAM_ID;
    uEntry = SB_G2XX_FE_PP_RULE_S7_L4_HASH_ROUTE_ENTRY;

    rv = (soc_sbx_g2xx_pp_rule_enable_set(unit, uValid, uCamId, uEntry));

    return rv;
}

sbStatus_t
soc_sbx_g2xx_pp_v4uc_str_sel_match_get(int unit, uint32_t* uValid)
{
    int uCamId = SB_G2XX_FE_PP_RULE_S7_L4_NO_HASH_ROUTE_CAM_ID;
    int uEntry = SB_G2XX_FE_PP_RULE_S7_L4_NO_HASH_ROUTE_ENTRY;

    return (soc_sbx_g2xx_pp_rule_enable_get(unit, uValid, uCamId, uEntry));
}


/*
 * Broad Shield
 */

sbStatus_t
soc_sbx_g2xx_pp_broad_shield_check_get( soc_sbx_g2xx_state_t *pFe,
                             int eType,
                             uint8_t *pbEnable)
{

    return soc_sbx_g2xx_pp_broad_shield_check_set_get(pFe, eType, pbEnable,
                                                      FALSE);
}

sbStatus_t
soc_sbx_g2xx_pp_broad_shield_check_set( soc_sbx_g2xx_state_t *pFe,
                                        int eType,
                             uint8_t bEnable)

{
    return soc_sbx_g2xx_pp_broad_shield_check_set_get(pFe, eType, &bEnable,
                                                      TRUE);
}

/*
 * Initialize
 */

sbStatus_t
soc_sbx_g2xx_pp_init(int unit,
                     sbCommonConfigParams_p_t cParams,
                     sbIngressConfigParams_p_t iParams)
{
  soc_sbx_g2xx_state_t *pFe = SOC_SBX_CONTROL(unit)->drv;
  void *v;
  uint32_t uCamId;
  uint32_t uCamEntry;
  soc_sbx_g2xx_pp_rule_mgr_t *pPpRuleMgr  = NULL;
  soc_sbx_g2xx_pp_cam_db_t   *pCamDb      = NULL;
  uint32 regoff, v0, ped_update=1;
  int rv;
  sbStatus_t err;

  /* reset all the data */
  err = cParams->sbmalloc(cParams->clientData,SB_ALLOC_INTERNAL,
                          sizeof(soc_sbx_g2xx_pp_rule_mgr_t), &v, NULL);
  SB_MEMSET(v, 0, sizeof(soc_sbx_g2xx_pp_rule_mgr_t));

  pFe->pPpMgr = (soc_sbx_g2xx_pp_rule_mgr_t *)v;
  pPpRuleMgr  = (soc_sbx_g2xx_pp_rule_mgr_t *)pFe->pPpMgr;
  pPpRuleMgr->bDebug = FALSE;

  /* Initialize all the default values for the pp hw configuration */
  pPpRuleMgr->IpTunneldb                = 0x0;
  pPpRuleMgr->Layer4Filterdb            = 0x0;
  pPpRuleMgr->LocalStationMatchdb       = 0x0;
  pPpRuleMgr->uProviderTag              = SB_G2XX_FE_PP_DEFAULT_PROVIDER_TAG;
  pPpRuleMgr->uCustomerTag              = SB_G2XX_FE_PP_DEFAULT_CUSTOMER_TAG;
  pPpRuleMgr->uLinkAggregationSeed      = SB_G2XX_FE_PP_DEFAULT_LAGR_SEED;
  pPpRuleMgr->uEthTypeExceptionMaxValue = SB_G2XX_FE_PP_DEFAULT_ETYPE_EXCEPTION_MAX;
  pPpRuleMgr->uEthTypeExceptionMinValue = SB_G2XX_FE_PP_DEFAULT_ETYPE_EXCEPTION_MIN;
  pPpRuleMgr->uIngrStartQid             = SB_G2XX_FE_PP_DEFAULT_INGR_START_QID;
  pPpRuleMgr->uIngrEndQid               = SB_G2XX_FE_PP_DEFAULT_INGR_END_QID;
  pPpRuleMgr->uEgrStartQid              = SB_G2XX_FE_PP_DEFAULT_EGR_START_QID;
  pPpRuleMgr->uEgrEndQid                = SB_G2XX_FE_PP_DEFAULT_EGR_END_QID;


  /* Run Time Number of Rules Check */
  if( ( SB_G2XX_FE_PP_RULE_MAX_CAM0 >= SB_G2XX_FE_PP_NUM_OF_RULES_PER_CAM ) ||
      ( SB_G2XX_FE_PP_RULE_MAX_CAM1 >= SB_G2XX_FE_PP_NUM_OF_RULES_PER_CAM ) ||
      ( SB_G2XX_FE_PP_RULE_MAX_CAM2 >= SB_G2XX_FE_PP_NUM_OF_RULES_PER_CAM ) ||
      ( SB_G2XX_FE_PP_RULE_MAX_CAM3 >= SB_G2XX_FE_PP_NUM_OF_RULES_PER_CAM )  ) {
      SB_LOGV1("Total # CAM0 Rules:%d \n", SB_G2XX_FE_PP_RULE_MAX_CAM0);
      SB_LOGV1("Total # CAM1 Rules:%d \n", SB_G2XX_FE_PP_RULE_MAX_CAM1);
      SB_LOGV1("Total # CAM2 Rules:%d \n", SB_G2XX_FE_PP_RULE_MAX_CAM2);
      SB_LOGV1("Total # CAM3 Rules:%d \n", SB_G2XX_FE_PP_RULE_MAX_CAM3);
      SB_LOGV1("%s FATAL ERROR: Number Of CAM Rules Exceed the Limit \n", __FUNCTION__);
  } else {
      SB_LOGV2("Total # CAM0 Rules:%d \n", SB_G2XX_FE_PP_RULE_MAX_CAM0);
      SB_LOGV2("Total # CAM1 Rules:%d \n", SB_G2XX_FE_PP_RULE_MAX_CAM1);
      SB_LOGV2("Total # CAM2 Rules:%d \n", SB_G2XX_FE_PP_RULE_MAX_CAM2);
      SB_LOGV2("Total # CAM3 Rules:%d \n", SB_G2XX_FE_PP_RULE_MAX_CAM3);
  }

  /* Initialize the cam db */
  for(uCamId=0; uCamId < SB_G2XX_FE_PP_NUM_OF_CAMS; uCamId++) {
    for(uCamEntry=0; uCamEntry < SB_G2XX_FE_PP_NUM_OF_RULES_PER_CAM; uCamEntry++) {
      pCamDb = &pPpRuleMgr->cam[uCamId];
      sbG2XXFePpCamEntry_Init(&pCamDb->cfgdb[uCamEntry]);
      sbG2XXFePpRamEntry_Init(&pCamDb->ramdb[uCamEntry]);
      pCamDb->ramdb[uCamEntry].m_uCheckerType = SB_G2XX_FE_PP_CHECKER_NONE;
      pCamDb->ramdb[uCamEntry].m_uHeaderAType = SB_G2XX_FE_PP_HDR_TYPE_UNKNOWN;
      pCamDb->ramdb[uCamEntry].m_uHeaderBType = SB_G2XX_FE_PP_HDR_TYPE_UNKNOWN;
      pCamDb->ramdb[uCamEntry].m_uStateMask   = 0x0000FF;
    }
  }

  /* Initialize PED header types to conform with PPE usage */
  /* - do not rely on CA/C2_PD_DEFAULT_* values            */

  /* Conditionally disable ped update of ERH length field */
  if (SOC_IS_SBX_FE2KXT(unit)) {
    if (fe2kAsm2IntD__getNc((fe2kAsm2IntD*)pFe->ucode, "G2XX_PED_UPDATE_ERH", &ped_update) > 0){
        return SOC_E_FAIL;
    }
  }
  if (!ped_update) {
    SAND_HAL_FE2000_RMW_FIELD(pFe->regSet, PD_DEBUG, EGRESS_ROUTE_HDR_TYPE, 0xf);
  }

  regoff = 0;
  regoff = regoff * 4 + SAND_HAL_FE2000_REG_OFFSET(pFe->regSet, PD_HDR_CONFIG0);
  v0 = SAND_HAL_FE2000_READ_OFFS(pFe->regSet, regoff);
  v0 = SAND_HAL_FE2000_MOD_FIELD(pFe->regSet, PD_HDR_CONFIG0, BASE_LENGTH, v0, 
                          pFe->erhlen);
  SAND_HAL_FE2000_WRITE_OFFS(pFe->regSet, regoff, v0);

  rv = soc_sbx_g2xx_htype_etype_get(pFe->unit, &regoff);
  regoff = regoff * 4 + SAND_HAL_FE2000_REG_OFFSET(pFe->regSet, PD_HDR_CONFIG0);
  if (rv != SOC_E_NONE) {
    return SB_UCODE_SYMBOL_NOT_FOUND;
  }
  v0 = SAND_HAL_FE2000_READ_OFFS(pFe->regSet, regoff);
  v0 = SAND_HAL_FE2000_MOD_FIELD(pFe->regSet, PD_HDR_CONFIG0, BASE_LENGTH, v0, 
                          SB_G2XX_FE_PP_HDR_LEN_ETYPE(pFe));
  SAND_HAL_FE2000_WRITE_OFFS(pFe->regSet, regoff, v0);

  rv = soc_sbx_g2xx_htype_eth_get(pFe->unit, &regoff);
  regoff = regoff * 4 + SAND_HAL_FE2000_REG_OFFSET(pFe->regSet, PD_HDR_CONFIG0);
  if (rv != SOC_E_NONE) {
    return SB_UCODE_SYMBOL_NOT_FOUND;
  }
  v0 = SAND_HAL_FE2000_READ_OFFS(pFe->regSet, regoff);
  v0 = SAND_HAL_FE2000_MOD_FIELD(pFe->regSet, PD_HDR_CONFIG0, BASE_LENGTH, v0, 
                          SB_G2XX_FE_PP_HDR_LEN_ETH(pFe));
  SAND_HAL_FE2000_WRITE_OFFS(pFe->regSet, regoff, v0);

  rv = soc_sbx_g2xx_htype_vtag_get(pFe->unit, &regoff);
  regoff = regoff * 4 + SAND_HAL_FE2000_REG_OFFSET(pFe->regSet, PD_HDR_CONFIG0);
  if (rv != SOC_E_NONE) {
    return SB_UCODE_SYMBOL_NOT_FOUND;
  }
  v0 = SAND_HAL_FE2000_READ_OFFS(pFe->regSet, regoff);
  v0 = SAND_HAL_FE2000_MOD_FIELD(pFe->regSet, PD_HDR_CONFIG0, LEN_POSN, v0, 0x1FF);
  v0 = SAND_HAL_FE2000_MOD_FIELD(pFe->regSet, PD_HDR_CONFIG0, BASE_LENGTH, v0,
                                 SB_G2XX_FE_PP_HDR_LEN_VLAN(pFe));
  SAND_HAL_FE2000_WRITE_OFFS(pFe->regSet, regoff, v0);

  return SB_OK;
}

sbStatus_t
soc_sbx_g2xx_pp_un_init(soc_sbx_g2xx_state_t *pFe, sbCommonConfigParams_p_t cParams, sbIngressConfigParams_p_t iParams) {
  return SB_OK;
}

sbStatus_t
soc_sbx_g2xx_pp_hw_init(int unit, sbFeInitAsyncCallback_f_t cb, void *id)
{
  soc_sbx_g2xx_state_t *pFe = SOC_SBX_CONTROL(unit)->drv;
  sbStatus_t status = SB_OK;
  uint32_t uPpConfig = 0;
  uint32_t uData;
  uint32_t strExcEn = 1;

  uData = SAND_HAL_FE2000_READ(pFe->regSet, PC_REVISION);
  if (!SAND_HAL_IS_FE2KXT(pFe->regSet)) {
    if (SAND_HAL_FE2000_GET_FIELD(pFe->regSet, PC_REVISION, REVISION, uData)==0) strExcEn = 0;
  }

  /* enable the stream selection overwrite */
  uPpConfig = SAND_HAL_FE2000_READ(pFe->regSet,PP_HDR_TYPE_CONFIG0);
  uPpConfig = SAND_HAL_FE2000_MOD_FIELD(pFe->regSet, PP_HDR_TYPE_CONFIG0,STR_VALUE, uPpConfig, SB_G2XX_FE_PP_STREAM_ID_EXCEPTION);
  uPpConfig = SAND_HAL_FE2000_MOD_FIELD(pFe->regSet, PP_HDR_TYPE_CONFIG0,STR_EXCEPTION_EN,
                                 uPpConfig, strExcEn);
  SAND_HAL_FE2000_WRITE(pFe->regSet, PP_HDR_TYPE_CONFIG0, uPpConfig);

  SAND_HAL_FE2000_RMW_FIELD(pFe->regSet, PP_EVENT0_MASK,
                     ENET_VLAN1_EQ_ZERO_DISINT,1);
  SAND_HAL_FE2000_RMW_FIELD(pFe->regSet, PP_EVENT1_MASK,
                     ENET_VLAN0_EQ_ZERO_DISINT,1);
  SAND_HAL_FE2000_RMW_FIELD(pFe->regSet, PP_EVENT1_MASK,
                     ENET_VLAN2_EQ_ZERO_DISINT,1);

  /* Initialize the initial queue state */
  SB_LOGV3("%s: Calling soc_sbx_g2xx_pp_queue_state_initialize\n", __FUNCTION__);
  if( SB_OK != (status = soc_sbx_g2xx_pp_queue_state_initialize(pFe)) ) {
    SB_LOGV3("error: soc_sbx_g2xx_pp_queue_state_initialize\n");
  }

  /* this resets everything to invalid not needed on real hw*/
  SB_LOGV3("%s: Calling soc_sbx_g2xx_pp_cam_reset\n", __FUNCTION__);
  if( SB_OK != (status = soc_sbx_g2xx_pp_cam_reset(unit)) ) {
    SB_LOGV3("error: soc_sbx_g2xx_pp_cam_reset\n");
    cb(id, status);
    return status;
  }

  /* program the parse engine HashEngine base rules */
  SB_LOGV3("%s: Calling soc_sbx_g2xx_pp_hash_templates_init\n", __FUNCTION__);
  if( SB_OK != (status = soc_sbx_g2xx_pp_hash_templates_init(pFe)) ) {
    SB_LOGV3("error: soc_sbx_g2xx_pp_hash_templates_init\n");
    cb(id, status);
    return status;
  }

  /* unprogram & reprogram the parse engine rules */
  SB_LOGV3("%s: Calling soc_sbx_g2xx_pp_rule_encode\n", __FUNCTION__);
  if( SB_OK != (status = soc_sbx_g2xx_pp_rule_encode(unit)) ) {
    SB_LOGV3("error: soc_sbx_g2xx_pp_rule_encode\n");
    cb(id, status);
    return status;
  }

  /* program the parse engine with valid entries */
  SB_LOGV3("%s: Calling soc_sbx_g2xx_pp_cam_default_init\n", __FUNCTION__);
  if( SB_OK != (status = soc_sbx_g2xx_pp_cam_default_init(unit)) ) {
    SB_LOGV3("error: soc_sbx_g2xx_pp_cam_default_init\n");
    cb(id, status);
    return status;
  }

  /* any async completion (or error) up above causes return. If we are
     here we must have completed synchronously (and without errors).
     Simply return status.
   */
  SB_LOGV3("%s: Finished.\n", __FUNCTION__);
  return status;
}

/*
 * Hash Template
 */

sbStatus_t
soc_sbx_g2xx_pp_hash_templates_init(soc_sbx_g2xx_state_t *pFe)
{
  /* Byte Hash Templates
   * It's a 40B mask in network byte order that we have to
   * configure and it is applied at the checker offset of the
   * current head byte. But in our case of ETH,IPV4, IPV6,TCP,UDP
   * our checker offset is exactly zero. so we are all set.
   */
  uint64_t uuMask =0;
  uint32_t uTemplateIndex =0 ;
  uint32_t uRtHdrOffset =  SOC_SBX_G2XX_ERH_LEN(pFe);
  soc_sbx_g2xx_pp_rule_mgr_t *pPpParams  = pFe->pPpMgr;


  /* SB_G2XX_FE_PP_RTHDR_ETH_HASH_TEMPLATE */
  uuMask = SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(uRtHdrOffset+0)  |
           SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(uRtHdrOffset+1)  |
           SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(uRtHdrOffset+2)  |
           SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(uRtHdrOffset+3)  |
           SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(uRtHdrOffset+4)  |
           SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(uRtHdrOffset+5)  |
           SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(uRtHdrOffset+6)  |
           SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(uRtHdrOffset+7)  |
           SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(uRtHdrOffset+8)  |
           SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(uRtHdrOffset+9)  |
           SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(uRtHdrOffset+10) |
           SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(uRtHdrOffset+11) ;

  uTemplateIndex = SB_G2XX_FE_PP_BYTE_HASH_TINDEX(SB_G2XX_FE_PP_RTHDR_ETH_HASH_TEMPLATE);
  pPpParams->ByteHashdb[uTemplateIndex].udb[0]  = (uuMask >> 32);
  pPpParams->ByteHashdb[uTemplateIndex].udb[1]  = (uuMask & SB_G2XX_FE_PP_WIDTH_TO_MASK(32));
  pPpParams->ByteHashdb[uTemplateIndex].bValid  = TRUE;


  /* Hash: SB_G2XX_FE_PP_ETH_HASH_TEMPLATE */
  uuMask = SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(0)  |
           SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(1)  |
           SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(2)  |
           SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(3)  |
           SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(4)  |
           SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(5)  |
           SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(6)  |
           SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(7)  |
           SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(8)  |
           SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(9)  |
           SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(10) |
           SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(11) ;

  uTemplateIndex = SB_G2XX_FE_PP_BYTE_HASH_TINDEX(SB_G2XX_FE_PP_ETH_HASH_TEMPLATE);
  pPpParams->ByteHashdb[uTemplateIndex].udb[0]  = (uuMask >> 32);
  pPpParams->ByteHashdb[uTemplateIndex].udb[1]  = (uuMask & SB_G2XX_FE_PP_WIDTH_TO_MASK(32));
  pPpParams->ByteHashdb[uTemplateIndex].bValid  = TRUE;

  /* Hash: SB_G2XX_FE_PP_IPV4_HASH_TEMPLATE */
  uuMask = SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(SB_G2XX_FE_IPV4_PROTOCOL_BYTE_OFFSET)  |
         SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(SB_G2XX_FE_IPV4_DA_BYTE_OFFSET)    |
         SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(SB_G2XX_FE_IPV4_DA_BYTE_OFFSET+1)  |
         SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(SB_G2XX_FE_IPV4_DA_BYTE_OFFSET+2)  |
         SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(SB_G2XX_FE_IPV4_DA_BYTE_OFFSET+3)  |
         SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(SB_G2XX_FE_IPV4_SA_BYTE_OFFSET)    |
         SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(SB_G2XX_FE_IPV4_SA_BYTE_OFFSET+1)  |
         SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(SB_G2XX_FE_IPV4_SA_BYTE_OFFSET+2)  |
         SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(SB_G2XX_FE_IPV4_SA_BYTE_OFFSET+3) ;

  uTemplateIndex = SB_G2XX_FE_PP_BYTE_HASH_TINDEX(SB_G2XX_FE_PP_IPV4_HASH_TEMPLATE);
  pPpParams->ByteHashdb[uTemplateIndex].udb[0]  = (uuMask >> 32);
  pPpParams->ByteHashdb[uTemplateIndex].udb[1]  = (uuMask & SB_G2XX_FE_PP_WIDTH_TO_MASK(32));
  pPpParams->ByteHashdb[uTemplateIndex].bValid  = TRUE;

  /* Hash: SB_G2XX_FE_PP_TCP/UDP_HASH_TEMPLATE */
  uuMask = SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(0)   |
           SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(0+1) |
           SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(2)   |
           SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(2+1);

  uTemplateIndex = SB_G2XX_FE_PP_BYTE_HASH_TINDEX(SB_G2XX_FE_PP_TCP_HASH_TEMPLATE);
  pPpParams->ByteHashdb[uTemplateIndex].udb[0]  = (uuMask >> 32);
  pPpParams->ByteHashdb[uTemplateIndex].udb[1]  = (uuMask & SB_G2XX_FE_PP_WIDTH_TO_MASK(32));
  pPpParams->ByteHashdb[uTemplateIndex].bValid  = TRUE;

  /* Bit Hash Templates */

  uuMask = 0xFFF;
  /* uuMask = SB_G2XX_FE_PP_BIT_HASH_MASK_GEN(SB_G2XX_FE_VID_BIT_OFFSET); */
  uTemplateIndex = 0;
  pPpParams->BitHashdb[uTemplateIndex].udb[0]  = 0x0;
  pPpParams->BitHashdb[uTemplateIndex].udb[1]  = 0;
  pPpParams->BitHashdb[uTemplateIndex].udb[2]  = 0;
  pPpParams->BitHashdb[uTemplateIndex].udb[3]  = 0;
  pPpParams->BitHashdb[uTemplateIndex].udb[4]  = 0;
  pPpParams->BitHashdb[uTemplateIndex].udb[5]  = 0;
  pPpParams->BitHashdb[uTemplateIndex].udb[6]  = 0;
  pPpParams->BitHashdb[uTemplateIndex].udb[7]  = 0;
  pPpParams->BitHashdb[uTemplateIndex].udb[8]  = 0;
  pPpParams->BitHashdb[uTemplateIndex].udb[9]  = 0;
  pPpParams->BitHashdb[uTemplateIndex].bValid  = TRUE;

  return SB_OK;
}


sbStatus_t
soc_sbx_g2xx_pp_hash_templates_set (soc_sbx_g2xx_state_t *pFe, uint32_t fieldMask)
{
  /* Byte Hash Templates
   * It's a 40B mask in network byte order that we have to
   * configure and it is applied at the checker offset of the
   * current head byte. But in our case of ETH,IPV4, IPV6,TCP,UDP
   * our checker offset is exactly zero. so we are all set.
   */
  uint64_t uuMask = 0;
  uint32_t uHashId =0;
  uint32_t uTemplateIndex =0 ;
  soc_sbx_g2xx_pp_rule_mgr_t *pPpParams  = pFe->pPpMgr;
  sbStatus_t status = SB_OK;

  /* Hash: SB_G2XX_FE_PP_ETH_HASH_TEMPLATE */
  uuMask = 0;
  if (fieldMask & SB_G2XX_PSC_MAC_DA)
    uuMask =  
           SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(0) |
           SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(1) |
           SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(2) |
           SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(3) |
           SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(4) |
           SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(5);

  if (fieldMask & SB_G2XX_PSC_MAC_SA)
     uuMask = uuMask | 
           SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(6)  |
           SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(7)  |
           SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(8)  |
           SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(9)  |
           SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(10) |
           SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(11);

  uTemplateIndex = SB_G2XX_FE_PP_BYTE_HASH_TINDEX(SB_G2XX_FE_PP_ETH_HASH_TEMPLATE);
  pPpParams->ByteHashdb[uTemplateIndex].udb[0]  = (uuMask >> 32);
  pPpParams->ByteHashdb[uTemplateIndex].udb[1]  = (uuMask & SB_G2XX_FE_PP_WIDTH_TO_MASK(32));
  pPpParams->ByteHashdb[uTemplateIndex].bValid  = TRUE;

  /* Hash: SB_G2XX_FE_PP_IPV4_HASH_TEMPLATE */
  uuMask = 0;
  if (fieldMask & SB_G2XX_PSC_IP_DA)
     uuMask = 
         SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(SB_G2XX_FE_IPV4_DA_BYTE_OFFSET)    |
         SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(SB_G2XX_FE_IPV4_DA_BYTE_OFFSET+1)  |
         SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(SB_G2XX_FE_IPV4_DA_BYTE_OFFSET+2)  |
         SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(SB_G2XX_FE_IPV4_DA_BYTE_OFFSET+3);

  if (fieldMask & SB_G2XX_PSC_IP_SA)
     uuMask = uuMask |
         SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(SB_G2XX_FE_IPV4_SA_BYTE_OFFSET)    |
         SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(SB_G2XX_FE_IPV4_SA_BYTE_OFFSET+1)  |
         SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(SB_G2XX_FE_IPV4_SA_BYTE_OFFSET+2)  |
         SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(SB_G2XX_FE_IPV4_SA_BYTE_OFFSET+3) ;

  uTemplateIndex = SB_G2XX_FE_PP_BYTE_HASH_TINDEX(SB_G2XX_FE_PP_IPV4_HASH_TEMPLATE);
  pPpParams->ByteHashdb[uTemplateIndex].udb[0]  = (uuMask >> 32);
  pPpParams->ByteHashdb[uTemplateIndex].udb[1]  = (uuMask & SB_G2XX_FE_PP_WIDTH_TO_MASK(32));
  pPpParams->ByteHashdb[uTemplateIndex].bValid  = TRUE;


  /* Hash: SB_G2XX_FE_PP_TCP/UDP_HASH_TEMPLATE - L4 Source/Dest Socket */
  uuMask = 0;
  if (fieldMask & SB_G2XX_PSC_L4SS)
     uuMask =  
           SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(0)   |
           SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(0+1);
  if (fieldMask & SB_G2XX_PSC_L4DS)
     uuMask = uuMask | 
           SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(2)   |
           SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(2+1);

  uTemplateIndex = SB_G2XX_FE_PP_BYTE_HASH_TINDEX(SB_G2XX_FE_PP_TCP_HASH_TEMPLATE);
  pPpParams->ByteHashdb[uTemplateIndex].udb[0]  = (uuMask >> 32);
  pPpParams->ByteHashdb[uTemplateIndex].udb[1]  = (uuMask & SB_G2XX_FE_PP_WIDTH_TO_MASK(32));
  pPpParams->ByteHashdb[uTemplateIndex].bValid  = TRUE;

  /* Bit Hash Templates */
  /* The Bit Hash Template is 320 bits long, we only use the first 32 bits to */
  /* handle the 12 bit VID which is not byte aligned.  The VID is in the low 12 */
  /* bits of the first word offset from the current window of the PPE */
  uuMask = 0;
  if (fieldMask & SB_G2XX_PSC_VID)
  uuMask = 0x00000FFF;

  uTemplateIndex = 0;

  pPpParams->BitHashdb[uTemplateIndex].udb[0]  = (uuMask & SB_G2XX_FE_PP_WIDTH_TO_MASK(32));
  pPpParams->BitHashdb[uTemplateIndex].udb[1] = 0;
  pPpParams->BitHashdb[uTemplateIndex].udb[2] = 0;
  pPpParams->BitHashdb[uTemplateIndex].udb[3] = 0;
  pPpParams->BitHashdb[uTemplateIndex].udb[4] = 0;
  pPpParams->BitHashdb[uTemplateIndex].udb[5] = 0;
  pPpParams->BitHashdb[uTemplateIndex].udb[6] = 0;
  pPpParams->BitHashdb[uTemplateIndex].udb[7] = 0;
  pPpParams->BitHashdb[uTemplateIndex].udb[8] = 0;
  pPpParams->BitHashdb[uTemplateIndex].udb[9] = 0;

  pPpParams->BitHashdb[uTemplateIndex].bValid  = TRUE;
  pPpParams->BitHashdb[uTemplateIndex].fieldMask = fieldMask;

  /* program the Bit Hash templates */
  for(uHashId =0 ; uHashId < SB_G2XX_FE_PP_NUM_BIT_HASH_TEMPLATES; uHashId++) {
    if(pPpParams->BitHashdb[uHashId].bValid) {
      status = sbFe2000BitHashConfigWrite(pFe->regSet,uHashId,
                                           &pPpParams->BitHashdb[uHashId].udb[0]);
      if(SB_OK != status) {
        return status;
      }
    }
  }

  /* program the Byte Hash templates */
  for(uHashId =0 ; uHashId < SB_G2XX_FE_PP_NUM_BYTE_HASH_TEMPLATES; uHashId++) {
    if(pPpParams->ByteHashdb[uHashId].bValid) {
      status = sbFe2000ByteHashConfigWrite(pFe->regSet,uHashId,
                                           &pPpParams->ByteHashdb[uHashId].udb[0]);
      if(SB_OK != status) {
        return status;
      }
    }
  }

  return SB_OK;
}

sbStatus_t
soc_sbx_g2xx_pp_hash_templates_recover(soc_sbx_g2xx_state_t *pFe)
{
    sbStatus_t rv = SB_OK;
    soc_sbx_g2xx_pp_rule_mgr_t *pPpParams  = pFe->pPpMgr;
    uint32_t uHashId;
    uint32_t uFieldMask;
    uint64_t uuMask, uuData;

    /* recover the Bit Hash templates */
    for (uHashId = 0 ; uHashId < SB_G2XX_FE_PP_NUM_BIT_HASH_TEMPLATES; uHashId++) {
        rv = sbFe2000BitHashConfigRead(pFe->regSet,uHashId,
                                       pPpParams->BitHashdb[uHashId].udb);
        if (SB_OK != rv) {
            return rv;
        }
    }
    
    /* recover the Byte Hash templates */
    for (uHashId = 0 ; uHashId < SB_G2XX_FE_PP_NUM_BYTE_HASH_TEMPLATES; uHashId++) {

        rv = sbFe2000ByteHashConfigRead(pFe->regSet,uHashId,
                                        pPpParams->ByteHashdb[uHashId].udb);
        if (SB_OK != rv) {
            return rv;
        }
    }
    
    uFieldMask = 0;

    /* Hash: SB_G2XX_FE_PP_ETH_HASH_TEMPLATE */
    uHashId = SB_G2XX_FE_PP_BYTE_HASH_TINDEX(SB_G2XX_FE_PP_ETH_HASH_TEMPLATE);
    uuData = (((uint64_t)pPpParams->ByteHashdb[uHashId].udb[0]  << 32) | 
              ((uint64_t)pPpParams->ByteHashdb[uHashId].udb[1]));
    
    /* SB_G2XX_PSC_MAC_DA psc */    
    uuMask =  
        SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(0)   |
        SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(0+1) |
        SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(0+2) |
        SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(0+3) |
        SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(0+4) |
        SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(0+5);
    
    if (uuData & uuMask) {
        if ((uuData & uuMask) != uuMask) {
            /* the masks changed, update the recovery sequence */
            SOC_ERROR_PRINT((DK_ERR, "PPE Hash incompatible\n"));
            return SB_UCODE_INCOMPATIBLE_ERR_CODE;
        }
        uFieldMask |= SB_G2XX_PSC_MAC_DA;
    }

    /* SB_G2XX_PSC_MAC_SA psc */
    uuMask = 
        SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(6)  |
        SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(7)  |
        SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(8)  |
        SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(9)  |
        SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(10) |
        SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(11);
    
    if (uuData & uuMask) {
        if ((uuData & uuMask) != uuMask) {
            /* the masks changed, update the recovery sequence */
            SOC_ERROR_PRINT((DK_ERR, "PPE Hash incompatible\n"));
            return SB_UCODE_INCOMPATIBLE_ERR_CODE;
        }
        uFieldMask |= SB_G2XX_PSC_MAC_SA;
    }
    

    /* Hash: SB_G2XX_FE_PP_IPV4_HASH_TEMPLATE */
    uHashId = SB_G2XX_FE_PP_BYTE_HASH_TINDEX(SB_G2XX_FE_PP_IPV4_HASH_TEMPLATE);
    uuData = (((uint64_t)pPpParams->ByteHashdb[uHashId].udb[0] << 32) |
              ((uint64_t)pPpParams->ByteHashdb[uHashId].udb[1]));

    /* SB_G2XX_PSC_IP_DA psc */
    uuMask = 
        SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(SB_G2XX_FE_IPV4_DA_BYTE_OFFSET)    |
        SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(SB_G2XX_FE_IPV4_DA_BYTE_OFFSET+1)  |
        SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(SB_G2XX_FE_IPV4_DA_BYTE_OFFSET+2)  |
        SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(SB_G2XX_FE_IPV4_DA_BYTE_OFFSET+3);
    
    if (uuData & uuMask) {
        if ((uuData & uuMask) != uuMask) {
            /* the masks changed, update the recovery sequence */
            SOC_ERROR_PRINT((DK_ERR, "PPE Hash incompatible\n"));
            return SB_UCODE_INCOMPATIBLE_ERR_CODE;
        }
        uFieldMask |= SB_G2XX_PSC_IP_DA;
    }
    
    /* SB_G2XX_PSC_IP_SA psc */
    uuMask =
        SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(SB_G2XX_FE_IPV4_SA_BYTE_OFFSET)    |
        SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(SB_G2XX_FE_IPV4_SA_BYTE_OFFSET+1)  |
        SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(SB_G2XX_FE_IPV4_SA_BYTE_OFFSET+2)  |
        SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(SB_G2XX_FE_IPV4_SA_BYTE_OFFSET+3) ;

    if (uuData & uuMask) {
        if ((uuData & uuMask) != uuMask) {
            /* the masks changed, update the recovery sequence */
            SOC_ERROR_PRINT((DK_ERR, "PPE Hash incompatible\n"));
            return SB_UCODE_INCOMPATIBLE_ERR_CODE;
        }
        uFieldMask |= SB_G2XX_PSC_IP_SA;
    }

    /* Hash: SB_G2XX_FE_PP_TCP/UDP_HASH_TEMPLATE - L4 Source/Dest Socket */
    uHashId = SB_G2XX_FE_PP_BYTE_HASH_TINDEX(SB_G2XX_FE_PP_TCP_HASH_TEMPLATE);
    uuData = (((uint64_t)pPpParams->ByteHashdb[uHashId].udb[0] << 32) | 
              ((uint64_t)pPpParams->ByteHashdb[uHashId].udb[1]));

    /* SB_G2XX_PSC_L4SS psc */
    uuMask =  
        SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(0)   |
        SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(0+1);

    if (uuData & uuMask) {
        if ((uuData & uuMask) != uuMask) {
            /* the masks changed, update the recovery sequence */
            SOC_ERROR_PRINT((DK_ERR, "PPE Hash incompatible\n"));
            return SB_UCODE_INCOMPATIBLE_ERR_CODE;
        }
        uFieldMask |= SB_G2XX_PSC_L4SS;
    }

    /* SB_G2XX_PSC_L4DS psc */
    uuMask =
        SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(2)   |
        SB_G2XX_FE_PP_BYTE_HASH_MASK_GEN(2+1);

    if (uuData & uuMask) {
        if ((uuData & uuMask) != uuMask) {
            /* the masks changed, update the recovery sequence */
            SOC_ERROR_PRINT((DK_ERR, "PPE Hash incompatible\n"));
            return SB_UCODE_INCOMPATIBLE_ERR_CODE;
        }
        uFieldMask |= SB_G2XX_PSC_L4DS;
    }
     
    /* Bit Hash Templates - only recover out of band data; 
     * the data base is recovered above
     */

    uHashId = 0;

    pPpParams->BitHashdb[uHashId].fieldMask = uFieldMask;

    return rv;
}

sbStatus_t
soc_sbx_g2xx_pp_hash_templates_get (soc_sbx_g2xx_state_t *pFe, uint32_t *fieldMask)
{
  soc_sbx_g2xx_pp_rule_mgr_t *pPpParams  = pFe->pPpMgr;

  *fieldMask = pPpParams->BitHashdb[0].fieldMask;
  return SB_OK;
}

/*
 * Queue State
 */

sbStatus_t
soc_sbx_g2xx_pp_queue_state_initialize(soc_sbx_g2xx_state_t *pFe)
{
  sbFe2000Connection_t *c;
  int i, rv;
  soc_sbx_g2xx_p2e_t p2e;
  soc_sbx_g2xx_ep2e_t ep2e;

  for (i = 0; i < pFe->queues->n; i++) {
      c = &pFe->queues->connections[i];
      if (c->bEgress) {
          soc_sbx_g2xx_ep2e_t_init(&ep2e);
          rv = soc_sbx_g2xx_ep2e_set(pFe->unit, c->ulUcodePort, &ep2e);
      } else {
          soc_sbx_g2xx_p2e_t_init(&p2e);
          rv = soc_sbx_g2xx_p2e_set(pFe->unit, c->ulUcodePort, &p2e);
      }
      if (rv) {
          return SB_BAD_ARGUMENT_ERR_CODE;
      }
  }

  return SB_OK;
}

/* this function will go away once the model drvien configuration is ogne */
sbStatus_t
soc_sbx_g2xx_pp_cam_reset(int unit)
{
  soc_sbx_g2xx_state_t *pFe = SOC_SBX_CONTROL(unit)->drv;
  uint32_t uCamId, uCamEntry;
  sbStatus_t status = SB_OK;
  soc_sbx_g2xx_pp_rule_mgr_t *pPpParams  = pFe->pPpMgr;  

  for(uCamId=0; uCamId < SB_G2XX_FE_PP_NUM_OF_CAMS; uCamId++) {
    for(uCamEntry=0; uCamEntry < SB_G2XX_FE_PP_NUM_OF_RULES_PER_CAM; uCamEntry++) {
      status = soc_sbx_g2xx_pp_cam_rule_write(unit, uCamId, uCamEntry,
                                    & (pPpParams->cam[uCamId].cfgdb[uCamEntry]),
                                    & (pPpParams->cam[uCamId].ramdb[uCamEntry]) );
      if(SB_OK != status) {
        break;
      }
    }
  }
  return status;
}

/* recover internal cache from ppe during a warm boot. */
sbStatus_t
soc_sbx_g2xx_pp_cam_db_recover(int unit)
{
  soc_sbx_g2xx_state_t *pFe = SOC_SBX_CONTROL(unit)->drv;
  uint32_t uCamId, uCamEntry;
  sbStatus_t status = SB_OK;
  soc_sbx_g2xx_pp_rule_mgr_t *pPpParams  = pFe->pPpMgr;

  for(uCamId=0; uCamId < SB_G2XX_FE_PP_NUM_OF_CAMS; uCamId++) {
    for(uCamEntry=0; uCamEntry < SB_G2XX_FE_PP_NUM_OF_RULES_PER_CAM; uCamEntry++) {
      status = soc_sbx_g2xx_pp_cam_rule_read(unit, uCamId, uCamEntry,
                                    & (pPpParams->cam[uCamId].cfgdb[uCamEntry]),
                                    & (pPpParams->cam[uCamId].ramdb[uCamEntry]) );
      if(SB_OK != status) {
        break;
      }
    }
  }
  return status;
}

sbStatus_t
soc_sbx_g2xx_pp_cam_default_init(int unit)
{
  soc_sbx_g2xx_state_t *pFe = SOC_SBX_CONTROL(unit)->drv;
  uint32_t uCamId, uCamEntry;
  uint32_t uHashId = 0;
  sbStatus_t status = SB_OK;
  soc_sbx_g2xx_pp_rule_mgr_t *pPpParams  = pFe->pPpMgr;

  for(uCamId=0; uCamId < SB_G2XX_FE_PP_NUM_OF_CAMS; uCamId++) {
    for(uCamEntry=0; uCamEntry < SB_G2XX_FE_PP_NUM_OF_RULES_PER_CAM; uCamEntry++) {

        if(pPpParams->cam[uCamId].cfgdb[uCamEntry].m_uValid == SB_G2XX_FE_PP_MAKE_ENTRY_VALID) {
          status = soc_sbx_g2xx_pp_cam_rule_write(unit,uCamId, uCamEntry,
                                        & (pPpParams->cam[uCamId].cfgdb[uCamEntry]),
                                        & (pPpParams->cam[uCamId].ramdb[uCamEntry]) );
          if(SB_OK != status) {
            break;
          }
        }
    }
  }

  /* program the Bit Hash templates */
  for(uHashId =0 ; uHashId < SB_G2XX_FE_PP_NUM_BIT_HASH_TEMPLATES; uHashId++) {
    if(pPpParams->BitHashdb[uHashId].bValid) {
      status = sbFe2000BitHashConfigWrite(pFe->regSet,uHashId,
                                           &pPpParams->BitHashdb[uHashId].udb[0]);
      if(SB_OK != status) {
        return status;
      }
    }
  }

  /* program the Byte Hash templates */
  for(uHashId =0 ; uHashId < SB_G2XX_FE_PP_NUM_BYTE_HASH_TEMPLATES; uHashId++) {
    if(pPpParams->ByteHashdb[uHashId].bValid) {
      status = sbFe2000ByteHashConfigWrite(pFe->regSet,uHashId,
                                           &pPpParams->ByteHashdb[uHashId].udb[0]);
      if(SB_OK != status) {
        return status;
      }
    }
  }

  return status;
}

/*
 * 'hook' functions
 */

int
soc_sbx_g2xx_p2e20_set_hook(int unit, int *sset, int *idx, int port,
                          soc_sbx_g2xx_p2e20_t *e)
{
  int s;
  soc_sbx_g2xx_state_t *fe = SOC_SBX_CONTROL(unit)->drv;
  int idx0;
  *sset = 0;

  e->port = port;
  e->pport = port;
  s = soc_sbx_g2xx_htype_eth_get(unit, &e->htype);
  if (s) {
    *sset = 1;
    return s;
  }
  e->mask[0] = 0xff;
  e->mask[1] = 0x03;
  e->mask[2] = 0x0;
  e->mask[3] = 0x0;
  e->mask[4] = 0xff;
  e->mask[5] = 0xff;
  e->mask[6] = 0xff;
  e->mask[7] = 0x03;

  idx0 = fe->queues->port2iqid[port];
  if (idx0 == SB_FE2000_SWS_UNINITIALIZED_QID) {
    *sset = 1;
    return SOC_E_PARAM;
  }

  *idx = idx0;

  return SOC_E_NONE;
}

int
soc_sbx_g2xx_p2e20_get_hook(int unit, int *sset, int *idx, int port)
{
  soc_sbx_g2xx_state_t *fe = SOC_SBX_CONTROL(unit)->drv;
  int idx0;
  *sset = 0;

  idx0 = fe->queues->port2iqid[port];
  if (idx0 == SB_FE2000_SWS_UNINITIALIZED_QID) {
    *sset = 1;
    return SOC_E_PARAM;
  }

  *idx = idx0;

  return SOC_E_NONE;
}

int
soc_sbx_g2xx_ep2e20_set_hook(int unit, int *sset, int *idx, int port,
                          soc_sbx_g2xx_ep2e20_t *e)
{
  int s;
  soc_sbx_g2xx_state_t *fe = SOC_SBX_CONTROL(unit)->drv;
  int idx0;
  *sset = 0;

  e->state = 0x80;

  e->port = port;
  s = soc_sbx_g2xx_htype_erh_get(unit, &e->htype);
  if (s) {
    *sset = 1;
    return s;
  }
  e->mask[0] = 0xff;
  e->mask[1] = 0xff;
  e->mask[2] = 0x0;
  e->mask[3] = 0x0;
  e->mask[4] = 0xff;
  e->mask[5] = 0xff;
  e->mask[6] = 0xff;
  e->mask[7] = 0x03;

  idx0 = fe->queues->port2eqid[port];
  if (idx0 == SB_FE2000_SWS_UNINITIALIZED_QID) {
    *sset = 1;
    return SOC_E_PARAM;
  }

  *idx = idx0;

  return SOC_E_NONE;
}

int
soc_sbx_g2xx_ep2e20_get_hook(int unit, int *sset, int *idx, int port)
{
  soc_sbx_g2xx_state_t *fe = SOC_SBX_CONTROL(unit)->drv;
  int idx0;
  *sset = 0;

  idx0 = fe->queues->port2eqid[port];
  if (idx0 == SB_FE2000_SWS_UNINITIALIZED_QID) {
    *sset = 1;
    return SOC_E_PARAM;
  }

  *idx = idx0;

  return SOC_E_NONE;
}
int
soc_sbx_g2xx_p2e25_set_hook(int unit, int *sset, int *idx, int port,
                          soc_sbx_g2xx_p2e25_t *e)
{
  int s;
  soc_sbx_g2xx_state_t *fe = SOC_SBX_CONTROL(unit)->drv;
  int idx0;
  *sset = 0;

  e->port = port;
  e->pport = port;
  s = soc_sbx_g2xx_htype_eth_get(unit, &e->htype);
  if (s) {
    *sset = 1;
    return s;
  }
  e->mask[0] = 0xff;
  e->mask[1] = 0x03;
  e->mask[2] = 0x0;
  e->mask[3] = 0x0;
  e->mask[4] = 0xff;
  e->mask[5] = 0xff;
  e->mask[6] = 0xff;
  e->mask[7] = 0x03;

  idx0 = fe->queues->port2iqid[port];
  if (idx0 == SB_FE2000_SWS_UNINITIALIZED_QID) {
    *sset = 1;
    return SOC_E_PARAM;
  }

  *idx = idx0;

  return SOC_E_NONE;
}

int
soc_sbx_g2xx_p2e25_get_hook(int unit, int *sset, int *idx, int port)
{
  soc_sbx_g2xx_state_t *fe = SOC_SBX_CONTROL(unit)->drv;
  int idx0;
  *sset = 0;

  idx0 = fe->queues->port2iqid[port];
  if (idx0 == SB_FE2000_SWS_UNINITIALIZED_QID) {
    *sset = 1;
    return SOC_E_PARAM;
  }

  *idx = idx0;

  return SOC_E_NONE;
}

int
soc_sbx_g2xx_ep2e25_set_hook(int unit, int *sset, int *idx, int port,
                          soc_sbx_g2xx_ep2e25_t *e)
{
  int s;
  soc_sbx_g2xx_state_t *fe = SOC_SBX_CONTROL(unit)->drv;
  int idx0;
  *sset = 0;

  e->state = 0x80;

  e->port = port;
  s = soc_sbx_g2xx_htype_erh_get(unit, &e->htype);
  if (s) {
    *sset = 1;
    return s;
  }
  e->mask[0] = 0xff;
  e->mask[1] = 0xff;
  e->mask[2] = 0x0;
  e->mask[3] = 0x0;
  e->mask[4] = 0xff;
  e->mask[5] = 0xff;
  e->mask[6] = 0xff;
  e->mask[7] = 0x03;

  idx0 = fe->queues->port2eqid[port];
  if (idx0 == SB_FE2000_SWS_UNINITIALIZED_QID) {
    *sset = 1;
    return SOC_E_PARAM;
  }

  *idx = idx0;

  return SOC_E_NONE;
}

int
soc_sbx_g2xx_ep2e25_get_hook(int unit, int *sset, int *idx, int port)
{
  soc_sbx_g2xx_state_t *fe = SOC_SBX_CONTROL(unit)->drv;
  int idx0;
  *sset = 0;

  idx0 = fe->queues->port2eqid[port];
  if (idx0 == SB_FE2000_SWS_UNINITIALIZED_QID) {
    *sset = 1;
    return SOC_E_PARAM;
  }

  *idx = idx0;

  return SOC_E_NONE;
}

/*
 * 'ext' functions
 */

int 
soc_sbx_g2xx_lsmac_set_ext(int unit, int ilsi, soc_sbx_g2xx_lsmac_t *e)
{
    sbStatus_t rv = SOC_E_NONE;
    int uCamId = SB_G2XX_FE_PP_LSM_MATCH_CAM_ID;
    int uEntry = ilsi;
    int uOffset = 0;
    int nPortId = (e->useport) ? e->port : SB_G2XX_FE_PP_PORT_GLOBAL;
    soc_sbx_g2xx_6_byte_t uData;
    soc_sbx_g2xx_6_byte_t uMask;

    uData[0] = e->mac[0];
    uData[1] = e->mac[1];
    uData[2] = e->mac[2];
    uData[3] = e->mac[3];
    uData[4] = e->mac[4];
    uData[5] = e->mac[5];

    uMask[0] = 0xFF;
    uMask[1] = 0xFF;
    uMask[2] = 0xFF;
    uMask[3] = 0xFF;
    uMask[4] = 0xFF;
    uMask[5] = 0xFF;

    rv = soc_sbx_g2xx_lsm_set_ext(unit, uCamId, uEntry, uOffset, nPortId, &uData, &uMask);

    return rv;
}

int 
soc_sbx_g2xx_lsmac_get_ext(int unit, int ilsi, soc_sbx_g2xx_lsmac_t *e)
{
    sbStatus_t rv = SOC_E_NONE;
    int uCamId = SB_G2XX_FE_PP_LSM_MATCH_CAM_ID;
    int uEntry = ilsi;
    int uOffset = 0;
    int nPortId = SB_G2XX_FE_PP_PORT_GLOBAL;
    soc_sbx_g2xx_6_byte_t uData;
    soc_sbx_g2xx_6_byte_t uMask;

    uData[0] = 0x0;
    uData[1] = 0x0;
    uData[2] = 0x0;
    uData[3] = 0x0;
    uData[4] = 0x0;
    uData[5] = 0x0;

    rv = soc_sbx_g2xx_lsm_get_ext(unit, uCamId, uEntry, uOffset, &nPortId, &uData, &uMask);

    e->mac[0] = uData[0];
    e->mac[1] = uData[1];
    e->mac[2] = uData[2];
    e->mac[3] = uData[3];
    e->mac[4] = uData[4];
    e->mac[5] = uData[5];

    e->port = nPortId;
    e->useport = (nPortId == SB_G2XX_FE_PP_PORT_GLOBAL) ? 0 : 1;

    return rv;
}


int 
soc_sbx_g2xx_tpid_set_ext(int unit, int itpidi, soc_sbx_g2xx_tpid_t *e)
{
    int rv;
    sbStatus_t status;
    uint32_t tpid;
    uint32_t tpid_cur;

    tpid = e->tpid;

    switch (itpidi) {
    case SB_G2XX_FE_CTPID_INDEX:
        /*
         * Don't support setting CTPID yet; need ppe support.
         * However, return OK if given TPID value matches
         * current fixed CTPID value.
         */
        rv = soc_sbx_g2xx_tpid0_get(unit, &tpid_cur);
        if (rv != SOC_E_NONE) {
            return rv;
        }
        if (tpid != tpid_cur) {
            rv = SOC_E_PARAM;
        } else {
            rv = SOC_E_NONE;
        }
        
#if 0
        rv = soc_sbx_g2xx_tpid0_set(unit, tpid);
        if (rv != SOC_E_NONE) {
            return rv;
        }
        rv = soc_sbx_g2xx_tpid0_set(unit, tpid << 16 | 0x0fff);
        if (rv != SOC_E_NONE) {
            return rv;
        }
#endif

        break;
    case SB_G2XX_FE_STPID0_INDEX:
        rv = soc_sbx_g2xx_tpid1_set(unit, tpid);
        if (rv != SOC_E_NONE) {
            return rv;
        }
        status = soc_sbx_g2xx_pp_tpid_config(unit, 0, tpid);
        rv = soc_sbx_translate_status(status);
        break;
    case SB_G2XX_FE_STPID1_INDEX:
        rv = soc_sbx_g2xx_tpid2_set(unit, tpid);
        if (rv != SOC_E_NONE) {
            return rv;
        }
        status = soc_sbx_g2xx_pp_tpid_config(unit, 1, tpid);
        rv = soc_sbx_translate_status(status);
        break;
    default:
        rv = SOC_E_PARAM;
    }

    return rv;
}

int 
soc_sbx_g2xx_tpid_get_ext(int unit, int itpidi, soc_sbx_g2xx_tpid_t *e)
{
    int rv;

    switch (itpidi) {
    case SB_G2XX_FE_CTPID_INDEX:
        rv = soc_sbx_g2xx_tpid0_get(unit, &e->tpid); break;
    case SB_G2XX_FE_STPID0_INDEX:
        rv = soc_sbx_g2xx_tpid1_get(unit, &e->tpid); break;
    case SB_G2XX_FE_STPID1_INDEX:
        rv = soc_sbx_g2xx_tpid2_get(unit, &e->tpid); break;
    default:
        rv = SOC_E_PARAM;
    }

    return rv;
}

int soc_sbx_g2xx_p2e_set_ext(int unit,
                int iport,
                soc_sbx_g2xx_p2e_t *e)
{
    if (SAND_HAL_IS_FE2KXT(unit)){
        return soc_sbx_g2xx_p2e25_set(unit, iport, (soc_sbx_g2xx_p2e25_t*)e);
    }else{
        return soc_sbx_g2xx_p2e20_set(unit, iport, (soc_sbx_g2xx_p2e20_t*)e);
    }

}

int soc_sbx_g2xx_p2e_get_ext(int unit,
                int iport,
                soc_sbx_g2xx_p2e_t *e)
{
    if (SAND_HAL_IS_FE2KXT(unit)){
        return soc_sbx_g2xx_p2e25_get(unit, iport, (soc_sbx_g2xx_p2e25_t*)e);
    }else{
        return soc_sbx_g2xx_p2e20_get(unit, iport, (soc_sbx_g2xx_p2e20_t*)e);
    }
}

int soc_sbx_g2xx_ep2e_set_ext(int unit,
                int iport,
                soc_sbx_g2xx_ep2e_t *e)
{
    if (SAND_HAL_IS_FE2KXT(unit)){
        return soc_sbx_g2xx_ep2e25_set(unit, iport, (soc_sbx_g2xx_ep2e25_t*)e);
    }else{
        return soc_sbx_g2xx_ep2e20_set(unit, iport, (soc_sbx_g2xx_ep2e20_t*)e);
    }
}

int soc_sbx_g2xx_ep2e_get_ext(int unit,
                int iport,
                soc_sbx_g2xx_ep2e_t *e)
{
    if (SAND_HAL_IS_FE2KXT(unit)){
        return soc_sbx_g2xx_ep2e25_get(unit, iport, (soc_sbx_g2xx_ep2e25_t*)e);
    }else{
        return soc_sbx_g2xx_ep2e20_get(unit, iport, (soc_sbx_g2xx_ep2e20_t*)e);
    }
}





/*
 * Private Functions
 */

static sbStatus_t
soc_sbx_g2xx_pp_rule_enable_set(int unit, uint32_t uValid, int uCamId, int uEntry)
{
    sbStatus_t rv = SOC_E_NONE;
    soc_sbx_g2xx_state_t *pFe = SOC_SBX_CONTROL(unit)->drv;
    soc_sbx_g2xx_pp_rule_mgr_t *pPpMgr = pFe->pPpMgr;
    pPpMgr->cam[uCamId].cfgdb[uEntry].m_uValid = (uValid > 0)
                                               ? SB_G2XX_FE_PP_MAKE_ENTRY_VALID
                                               : SB_G2XX_FE_PP_MAKE_ENTRY_INVALID;
    rv = soc_sbx_g2xx_pp_cam_rule_write(unit,uCamId, uEntry,
                                        & (pPpMgr->cam[uCamId].cfgdb[uEntry]),
                                        & (pPpMgr->cam[uCamId].ramdb[uEntry]) );
    return rv;
}

static sbStatus_t
soc_sbx_g2xx_pp_rule_enable_get(int unit, uint32_t *uValid, int uCamId, int uEntry)
{
    sbStatus_t rv = SOC_E_NONE;
    soc_sbx_g2xx_state_t *pFe = SOC_SBX_CONTROL(unit)->drv;
    soc_sbx_g2xx_pp_rule_mgr_t *pPpMgr = pFe->pPpMgr;
    rv = soc_sbx_g2xx_pp_cam_rule_read(unit,uCamId, uEntry,
                                        & (pPpMgr->cam[uCamId].cfgdb[uEntry]),
                                        & (pPpMgr->cam[uCamId].ramdb[uEntry]) );
    *uValid = (pPpMgr->cam[uCamId].cfgdb[uEntry].m_uValid == SB_G2XX_FE_PP_MAKE_ENTRY_VALID) ? 1 : 0;
    return rv;
}

static sbStatus_t
soc_sbx_g2xx_lsm_set_ext(
    int unit, int uCamId, int uEntry, int uOffset, int nPortId,
    soc_sbx_g2xx_6_byte_t *pData,
    soc_sbx_g2xx_6_byte_t *pMask)
{
    sbStatus_t rv = SOC_E_NONE;

    soc_sbx_g2xx_state_t *pFe = SOC_SBX_CONTROL(unit)->drv;
    soc_sbx_g2xx_pp_rule_mgr_t *pPpMgr = pFe->pPpMgr;

    sbG2XXFePpCamEntry_t *pCamEntry;
    sbG2XXFePpRamEntry_t *pRamEntry;

    pCamEntry = &(pPpMgr->cam[uCamId].cfgdb[uEntry]);
    pRamEntry = &(pPpMgr->cam[uCamId].ramdb[uEntry]);

    pCamEntry->m_uMask5 = 0x0;
    pCamEntry->m_uMask4 = 0x0;
    pCamEntry->m_uMask3 = 0x0;
    pCamEntry->m_uMask2 = 0x0;
    pCamEntry->m_uMask1 = 0x0;
    pCamEntry->m_uMask0 = 0x0;
    
    pCamEntry->m_uData5 = 0x0;
    pCamEntry->m_uData4 = 0x0;
    pCamEntry->m_uData3 = 0x0;
    pCamEntry->m_uData2 = 0x0;
    pCamEntry->m_uData1 = 0x0;
    pCamEntry->m_uData0 = 0x0;
    
    if (nPortId == SB_G2XX_FE_PP_PORT_GLOBAL) {
        SB_G2XX_FE_PP_CSTATE_PID_SET(pCamEntry->m_uMask6, 0x0);
        SB_G2XX_FE_PP_CSTATE_PID_SET(pCamEntry->m_uData6, SB_G2XX_FE_PP_PORT_GLOBAL);
    } else {
        SB_G2XX_FE_PP_CSTATE_PID_SET(pCamEntry->m_uMask6, 0x3F);
        SB_G2XX_FE_PP_CSTATE_PID_SET(pCamEntry->m_uData6, nPortId);
    }

    if (uOffset == 0) {
        pCamEntry->m_uMask5 = (*pMask)[0] << 24
                     | (*pMask)[1] << 16
                     | (*pMask)[2] <<  8
                     | (*pMask)[3] <<  0;
        pCamEntry->m_uMask4 = (*pMask)[4] << 24
                     | (*pMask)[5] << 16
                     | 0x00        <<  8
                     | 0x00        <<  0;
    
        pCamEntry->m_uData5 = (*pData)[0] << 24
                     | (*pData)[1] << 16
                     | (*pData)[2] <<  8
                     | (*pData)[3] <<  0;
        pCamEntry->m_uData4 = (*pData)[4] << 24
                     | (*pData)[5] << 16
                     | 0x00        <<  8
                     | 0x00        <<  0;

    } else if (uOffset == 12) { /* 12B ERH */

        pCamEntry->m_uMask5 = 0x00000000; /* ERH[11:8] */
        pCamEntry->m_uMask4 = 0x00000000; /* ERH[ 7:4] */
        pCamEntry->m_uMask3 = 0x00000000; /* ERH[ 3:0] */
        pCamEntry->m_uMask2 = (*pMask)[0] << 24
                     | (*pMask)[1] << 16
                     | (*pMask)[2] <<  8
                     | (*pMask)[3] <<  0;
        pCamEntry->m_uMask1 = (*pMask)[4] << 24
                     | (*pMask)[5] << 16
                     | 0x00        <<  8
                     | 0x00        <<  0;
  
        pCamEntry->m_uData5 = 0x00000000; /* ERH[11:8] */
        pCamEntry->m_uData4 = 0x00000000; /* ERH[ 7:4] */
        pCamEntry->m_uData3 = 0x00000000; /* ERH[ 3:0] */
        pCamEntry->m_uData2 = (*pData)[0]  << 24
                     | (*pData)[1]  << 16
                     | (*pData)[2]  <<  8
                     | (*pData)[3]  <<  0;
        pCamEntry->m_uData1 = (*pData)[4]  << 24
                     | (*pData)[5]  << 16
                     | 0x00         <<  8
                     | 0x00         <<  0;
    } else {
        /* Need to adjust for QESS ERH */
        /* program the mask and data */
        return SOC_E_UNAVAIL;
    }

    rv = soc_sbx_g2xx_pp_cam_rule_write(unit, uCamId, uEntry, pCamEntry, pRamEntry);
    return rv;
}

static sbStatus_t
soc_sbx_g2xx_lsm_get_ext(
    int unit, int uCamId, int uEntry, int uOffset, int *pPortId,
    soc_sbx_g2xx_6_byte_t *pData,
    soc_sbx_g2xx_6_byte_t *pMask)
{
    sbStatus_t rv = SOC_E_NONE;

    soc_sbx_g2xx_state_t *pFe = SOC_SBX_CONTROL(unit)->drv;
    soc_sbx_g2xx_pp_rule_mgr_t *pPpMgr = pFe->pPpMgr;

    sbG2XXFePpCamEntry_t *pCamEntry;
    sbG2XXFePpRamEntry_t *pRamEntry;

    pCamEntry = &(pPpMgr->cam[uCamId].cfgdb[uEntry]);
    pRamEntry = &(pPpMgr->cam[uCamId].ramdb[uEntry]);

    *pPortId = SB_G2XX_FE_PP_CSTATE_PID_GET(pCamEntry->m_uData6);

    if (uOffset == 0) {
        (*pData)[0] = (uint32_t)((pCamEntry->m_uData5 >> 24) & 0xFF);
        (*pData)[1] = (uint32_t)((pCamEntry->m_uData5 >> 16) & 0xFF);
        (*pData)[2] = (uint32_t)((pCamEntry->m_uData5 >>  8) & 0xFF);
        (*pData)[3] = (uint32_t)((pCamEntry->m_uData5 >>  0) & 0xFF);
        (*pData)[4] = (uint32_t)((pCamEntry->m_uData4 >> 24) & 0xFF);
        (*pData)[5] = (uint32_t)((pCamEntry->m_uData4 >> 16) & 0xFF);
    
        (*pMask)[0] = (uint32_t)((pCamEntry->m_uMask5 >> 24) & 0xFF);
        (*pMask)[1] = (uint32_t)((pCamEntry->m_uMask5 >> 16) & 0xFF);
        (*pMask)[2] = (uint32_t)((pCamEntry->m_uMask5 >>  8) & 0xFF);
        (*pMask)[3] = (uint32_t)((pCamEntry->m_uMask5 >>  0) & 0xFF);
        (*pMask)[4] = (uint32_t)((pCamEntry->m_uMask4 >> 24) & 0xFF);
        (*pMask)[5] = (uint32_t)((pCamEntry->m_uMask4 >> 16) & 0xFF);
    } else if (uOffset == 12) {
                               /* pCamEntry->m_uData5 = ERH[11:8] */
                               /* pCamEntry->m_uData4 = ERH[ 7:4] */
                               /* pCamEntry->m_uData3 = ERH[ 3:0] */
        (*pData)[0] = (uint32_t)((pCamEntry->m_uData2 >> 24) & 0xFF);
        (*pData)[1] = (uint32_t)((pCamEntry->m_uData2 >> 16) & 0xFF);
        (*pData)[2] = (uint32_t)((pCamEntry->m_uData2 >>  8) & 0xFF);
        (*pData)[3] = (uint32_t)((pCamEntry->m_uData2 >>  0) & 0xFF);
        (*pData)[4] = (uint32_t)((pCamEntry->m_uData1 >> 24) & 0xFF);
        (*pData)[5] = (uint32_t)((pCamEntry->m_uData1 >> 16) & 0xFF);
                               /* pCamEntry->m_uMask5 = ERH[11:8] */
                               /* pCamEntry->m_uMask4 = ERH[ 7:4] */
                               /* pCamEntry->m_uMask3 = ERH[ 3:0] */
        (*pMask)[0] = (uint32_t)((pCamEntry->m_uMask2 >> 24) & 0xFF);
        (*pMask)[1] = (uint32_t)((pCamEntry->m_uMask2 >> 16) & 0xFF);
        (*pMask)[2] = (uint32_t)((pCamEntry->m_uMask2 >>  8) & 0xFF);
        (*pMask)[3] = (uint32_t)((pCamEntry->m_uMask2 >>  0) & 0xFF);
        (*pMask)[4] = (uint32_t)((pCamEntry->m_uMask1 >> 24) & 0xFF);
        (*pMask)[5] = (uint32_t)((pCamEntry->m_uMask1 >> 16) & 0xFF);
    } else {
        /* Need to adjust for QESS ERH */
        /* Program the mask and data!! */
        return SOC_E_UNAVAIL;
    }
    
    return rv;
}


static sbStatus_t
soc_sbx_g2xx_pp_broad_shield_check_set_get( soc_sbx_g2xx_state_t *pFe,
                                int eType,
                                uint8_t *pbEnable,
                                uint8_t bSetGet)
{
  sbStatus_t status = SB_OK;
  uint32_t uEvent0 =0, uEvent1 = 0;
  uint32_t uMask   =0, uRegId  = 0;

  uEvent0 = SAND_HAL_FE2000_READ(pFe->regSet, PP_EVENT0_MASK);
  uEvent1 = SAND_HAL_FE2000_READ(pFe->regSet, PP_EVENT1_MASK);

  if(eType == SB_FE2000_INV_PPP_ADDR_CTL) {
    uMask = SB_G2XX_FE_PP_EVENT0_MASK(pFe->regSet, INV_PPP_ADDR_CTL);
  } else if(eType == SB_FE2000_INV_PPP_PID) {
    uMask = SB_G2XX_FE_PP_EVENT0_MASK(pFe->regSet, INV_PPP_PID);
  } else if(eType == SB_FE2000_ENET_VLAN_EQUAL_3FF_TAG1) {
    uMask = SB_G2XX_FE_PP_EVENT1_MASK(pFe->regSet, ENET_VLAN0_EQ_3FF);
    uRegId = 1;
  } else if(eType == SB_FE2000_ENET_VLAN_EQUAL_3FF_TAG2) {
    uMask = SB_G2XX_FE_PP_EVENT0_MASK(pFe->regSet, ENET_VLAN1_EQ_3FF);
  } else if(eType == SB_FE2000_ENET_VLAN_EQUAL_3FF_TAG3) {
    uMask = SB_G2XX_FE_PP_EVENT1_MASK(pFe->regSet, ENET_VLAN2_EQ_3FF);
    uRegId = 1;
  } else if(eType == SB_FE2000_ENET_SMAC_EQ_DMAC) {
    uMask = SB_G2XX_FE_PP_EVENT0_MASK(pFe->regSet, ENET_SMAC_EQ_DMAC);
  } else if(eType == SB_FE2000_INV_GRE_RES0) {
    uMask = SB_G2XX_FE_PP_EVENT0_MASK(pFe->regSet, INV_GRE_RES0);
  } else if(eType == SB_FE2000_UNK_MPLS_LBL_LABEL0) {
    uMask = SB_G2XX_FE_PP_EVENT1_MASK(pFe->regSet, UNK_MPLS0_LBL);
    uRegId = 1;
  } else if(eType == SB_FE2000_UNK_MPLS_LBL_LABEL1) {
    uMask = SB_G2XX_FE_PP_EVENT1_MASK(pFe->regSet, UNK_MPLS1_LBL);
    uRegId = 1;
  } else if(eType == SB_FE2000_UNK_MPLS_LBL_LABEL2) {
    uMask = SB_G2XX_FE_PP_EVENT1_MASK(pFe->regSet, UNK_MPLS2_LBL);
    uRegId = 1;
  } else if(eType == SB_FE2000_ENET_SMAC_EQ_DMAC_ZERO) {
    uMask = SB_G2XX_FE_PP_EVENT0_MASK(pFe->regSet, ENET_MAC_EQ_ZERO);
  } else if(eType == SB_FE2000_ENET_TYPE_BETWEEN_1501_AND_1536) {
    uMask = SB_G2XX_FE_PP_EVENT0_MASK(pFe->regSet, ENET_TYPE_VALUE);
  } else if(eType == SB_FE2000_ENET_SMAC_EQ_MULTICAST) {
    uMask = SB_G2XX_FE_PP_EVENT1_MASK(pFe->regSet, ENET_SMAC_MCAST);
    uRegId =1;
  } else if(eType == SB_FE2000_IPV4_RUNT_PKT) {
    uMask = SB_G2XX_FE_PP_EVENT0_MASK(pFe->regSet, IPV4_RUNT_PKT);
  } else if(eType == SB_FE2000_IPV4_OPTIONS) {
    uMask = SB_G2XX_FE_PP_EVENT0_MASK(pFe->regSet, IPV4_OPTIONS);
  } else if(eType == SB_FE2000_INV_IPV4_CHECKSUM) {
    uMask = SB_G2XX_FE_PP_EVENT0_MASK(pFe->regSet, INV_IPV4_CHECKSUM);
  } else if(eType == SB_FE2000_INV_IPV4_VER) {
    uMask = SB_G2XX_FE_PP_EVENT0_MASK(pFe->regSet, INV_IPV4_VER);
  } else if(eType == SB_FE2000_INV_IPV4_RUNT_HDR) {
    uMask = SB_G2XX_FE_PP_EVENT0_MASK(pFe->regSet, IPV4_RUNT_HDR);
  } else if(eType == SB_FE2000_INV_IPV4_LEN_ERR) {
    uMask = SB_G2XX_FE_PP_EVENT0_MASK(pFe->regSet, IPV4_LEN_ERR);
  } else if(eType == SB_FE2000_INV_IPV4_PKT_LEN_ERR) {
    uMask = SB_G2XX_FE_PP_EVENT0_MASK(pFe->regSet, IPV4_PKT_LEN_ERR);
  } else if(eType == SB_FE2000_INV_IPV4_SA) {
    uMask = SB_G2XX_FE_PP_EVENT0_MASK(pFe->regSet, IPV4_INV_SA);
  } else if(eType == SB_FE2000_INV_IPV4_DA) {
    uMask = SB_G2XX_FE_PP_EVENT0_MASK(pFe->regSet, IPV4_INV_DA);
  } else if(eType == SB_FE2000_INV_IPV4_SA_EQ_DA) {
    uMask = SB_G2XX_FE_PP_EVENT0_MASK(pFe->regSet, IPV4_SA_EQ_DA);
  } else if(eType == SB_FE2000_INV_IPV4_SA_OR_DA_IS_LOOPBACK) {
    uMask = SB_G2XX_FE_PP_EVENT0_MASK(pFe->regSet, IPV4_SA_OR_DA_LPBK);
  } else if(eType == SB_FE2000_INV_IPV4_SA_OR_DA_MARTIN_ADDRESS) {
    uMask = SB_G2XX_FE_PP_EVENT1_MASK(pFe->regSet, IPV4_SA_OR_DA_MARTIAN);
    uRegId  =1;
  } else if( (eType == SB_FE2000_IPV4_USR_ADDR_0)  ||
             (eType == SB_FE2000_IPV4_USR_ADDR_1)  ||
             (eType == SB_FE2000_IPV4_USR_ADDR_2)  ||
             (eType == SB_FE2000_IPV4_USR_ADDR_3)  ||
             (eType == SB_FE2000_IPV4_USR_ADDR_4)  ||
             (eType == SB_FE2000_IPV4_USR_ADDR_5)  ||
             (eType == SB_FE2000_IPV4_USR_ADDR_6)  ||
             (eType == SB_FE2000_IPV4_USR_ADDR_7)  ) {
    uMask = SB_G2XX_FE_PP_EVENT1_MASK(pFe->regSet, IPV4_SA_OR_DA_MATCH);
    uRegId = 1;
  } else if(eType == SB_FE2000_IPV4_FRAG_ICMP_PROTOCOL) {
#define WRITE_PP_IPV4_ICMP_PROTOCOL(unit,value) \
    (SAND_HAL_IS_FE2KXT((int)(unit)) \
     ? SAND_HAL_WRITE_OFFS((unit),SAND_HAL_REG_OFFSET(C2,PP_IPV4_ICMP_PROTOCOL),(value)) \
     : SAND_HAL_WRITE_OFFS((unit),SAND_HAL_REG_OFFSET(CA,PP_IPV4_ICMP_PROTOOOL),(value)) )
#define READ_PP_IPV4_ICMP_PROTOCOL(unit) \
    (SAND_HAL_IS_FE2KXT((int)unit) \
    ? SAND_HAL_READ_OFFS((unit), SAND_HAL_REG_OFFSET(C2,PP_IPV4_ICMP_PROTOCOL)) \
    : SAND_HAL_READ_OFFS((unit), SAND_HAL_REG_OFFSET(CA,PP_IPV4_ICMP_PROTOOOL)) )
    if(bSetGet) {
      WRITE_PP_IPV4_ICMP_PROTOCOL(pFe->regSet, (*pbEnable == TRUE)? 1:0xff);
    } else {
      *pbEnable = (READ_PP_IPV4_ICMP_PROTOCOL(pFe->regSet) == 1)? TRUE:FALSE;
    }
  } else if(eType == SB_FE2000_IPV6_RUNT_PKT) {
    uMask = SB_G2XX_FE_PP_EVENT0_MASK(pFe->regSet, IPV6_RUNT_PKT);
  } else if(eType == SB_FE2000_INV_IPV6_SA) {
    uMask = SB_G2XX_FE_PP_EVENT0_MASK(pFe->regSet, IPV6_INV_SA);
  } else if(eType == SB_FE2000_INV_IPV6_DA) {
    uMask = SB_G2XX_FE_PP_EVENT0_MASK(pFe->regSet, IPV6_INV_DA);
  } else if(eType == SB_FE2000_IPV6_SA_EQ_DA) {
    uMask = SB_G2XX_FE_PP_EVENT0_MASK(pFe->regSet, IPV6_SA_EQ_DA);
  } else if( (eType == SB_FE2000_IPV6_USR_ADDR_0) ||
             (eType == SB_FE2000_IPV6_USR_ADDR_1) ||
             (eType == SB_FE2000_IPV6_USR_ADDR_2) ||
             (eType == SB_FE2000_IPV6_USR_ADDR_3)  ) {
    uMask = SB_G2XX_FE_PP_EVENT1_MASK(pFe->regSet, IPV6_SA_OR_DA_MATCH);
    uRegId  = 1;
  } else if(eType == SB_FE2000_TCP_OR_UDP_DP_EQUAL_SP) {
    uMask  = SB_G2XX_FE_PP_EVENT0_MASK(pFe->regSet, UDP_SP_EQ_DP);
    uMask |= SB_G2XX_FE_PP_EVENT0_MASK(pFe->regSet, TCP_SP_EQ_DP);
  } else if(eType == SB_FE2000_TCP_SQ_EQ_ZERO_AND_FLAG_ZERO) {
    uMask = SB_G2XX_FE_PP_EVENT0_MASK(pFe->regSet, TCP_NULL_SCAN);
  } else if(eType == SB_FE2000_TCP_SQ_EQ_ZERO_AND_FIN_URG_PSH_ZERO) {
    uMask = SB_G2XX_FE_PP_EVENT0_MASK(pFe->regSet, TCP_XMAS_SCAN);
  } else if(eType == SB_FE2000_TCP_SYN_AND_FIN_BOTH_SET) {
    uMask = SB_G2XX_FE_PP_EVENT0_MASK(pFe->regSet, TCP_SYN_FIN);
  } else if(eType == SB_FE2000_L4_TINY_FRAG) {
    uMask = SB_G2XX_FE_PP_EVENT0_MASK(pFe->regSet, TCP_TINY_FRAG);
  } else if(eType == SB_FE2000_L4_SYN_SPORT_LT_1024) {
    uMask = SB_G2XX_FE_PP_EVENT1_MASK(pFe->regSet, SYN_SPORT_LT_1024);
    uRegId = 1;
  } else {
    /* error */
  }

  if(bSetGet) {
    if(*pbEnable) {
      if(uRegId == 0) {
        uEvent0 &= ~uMask;
        SAND_HAL_FE2000_WRITE(pFe->regSet,PP_EVENT0_MASK, uEvent0);
      } else {
        uEvent1 &= ~uMask;
        SAND_HAL_FE2000_WRITE(pFe->regSet,PP_EVENT1_MASK, uEvent1);
      }
    } else {
      if(uRegId == 0) {
        uEvent0 |= uMask;
        SAND_HAL_FE2000_WRITE(pFe->regSet,PP_EVENT0_MASK, uEvent0);
      } else {
        uEvent1 |= uMask;
        SAND_HAL_FE2000_WRITE(pFe->regSet,PP_EVENT1_MASK, uEvent1);
      }
    }
  } else { /* get */
    if(uRegId == 0) {
      *pbEnable = (uEvent0 & uMask) ? FALSE : TRUE;
    } else {
      *pbEnable = (uEvent1 & uMask) ? FALSE : TRUE;
    }
  }

  return status;
}


sbStatus_t
soc_sbx_g2xx_pp_cam_rule_write(int unit,
                     uint32_t uCamId,
                     uint32_t uCamEntryIndex,
                     sbG2XXFePpCamEntry_t *pCamEntry,
                     sbG2XXFePpRamEntry_t *pRamEntry)
{
  soc_sbx_g2xx_state_t *pFe = SOC_SBX_CONTROL(unit)->drv;
  uint32_t cam_size = (!SOC_IS_SBX_FE2KXT(unit))
	            ? SB_ZF_CAPPCAMCONFIGURATIONENTRY_SIZE_IN_WORDS
	            : SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_SIZE_IN_WORDS;
  uint32_t uCfgBuffer[SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_SIZE_IN_WORDS];
  uint32_t ram_size = (!SOC_IS_SBX_FE2KXT(unit))
	            ? SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_SIZE_IN_WORDS
	            : SB_ZF_C2PPCAMRAMCONFIGURATIONINSTANCE0ENTRY_SIZE_IN_WORDS;
  uint32_t uCfgRamBuffer[SB_ZF_C2PPCAMRAMCONFIGURATIONINSTANCE0ENTRY_SIZE_IN_WORDS];
  uint32_t addr = ((uCamId << SAND_HAL_FE2000_FIELD_SHIFT(pFe->regSet, PP_TCAM_CONFIG, MEM_ACC_CTRL_INSTANCE)) | uCamEntryIndex);

  sbG2XXFePpCamEntry_Pack(unit, pCamEntry, (uint8_t*)uCfgBuffer);
  sbG2XXFePpRamEntry_Pack(unit, pRamEntry, (uint8_t*)uCfgRamBuffer);

  /*
   * We write to
   * case 1:- cam ram if we are enabling the rule.
   * else we
   * case 2:- disable the rule first i.e cam config first
   * and then cam ram config.
   */

  if(pCamEntry->m_uValid == SB_G2XX_FE_PP_MAKE_ENTRY_VALID ) {
    /* case 1:- */
    /* write cam ram config first */
    if (sbFe2000UtilWriteIndir(pFe->regSet, TRUE, TRUE,
                               SAND_HAL_FE2000_REG_OFFSET(pFe->regSet, PP_CAM_RAM_MEM_ACC_CTRL),
                               addr, ram_size,
                               uCfgRamBuffer)) {
      return SB_TIMEOUT_ERR_CODE;
    }
    if (sbFe2000UtilWriteIndir(pFe->regSet, TRUE, TRUE,
                               SAND_HAL_FE2000_REG_OFFSET(pFe->regSet, PP_TCAM_CONFIG_MEM_ACC_CTRL),
                               addr, cam_size,
                               uCfgBuffer)) {
      return SB_TIMEOUT_ERR_CODE;
    }
  } else {
    /* case 2:- */
    /* we need to write Cam configuration first to disable first */
    if (sbFe2000UtilWriteIndir(pFe->regSet, TRUE, TRUE,
                               SAND_HAL_FE2000_REG_OFFSET(pFe->regSet, PP_TCAM_CONFIG_MEM_ACC_CTRL),
                               addr, cam_size,
                               uCfgBuffer)) {
      return SB_TIMEOUT_ERR_CODE;
    }
    if (sbFe2000UtilWriteIndir(pFe->regSet, TRUE, TRUE,
                               SAND_HAL_FE2000_REG_OFFSET(pFe->regSet, PP_CAM_RAM_MEM_ACC_CTRL),
                               addr, ram_size,
                               uCfgRamBuffer)) {
      return SB_TIMEOUT_ERR_CODE;
    }
  }
  return SB_OK;
}

sbStatus_t
soc_sbx_g2xx_pp_cam_rule_read(int unit,
                     uint32_t uCamId,
                     uint32_t uCamEntryIndex,
                     sbG2XXFePpCamEntry_t *pCamEntry,
                     sbG2XXFePpRamEntry_t *pRamEntry)
{
  soc_sbx_g2xx_state_t *pFe = SOC_SBX_CONTROL(unit)->drv;
  uint32_t cam_size = (!SOC_IS_SBX_FE2KXT(unit))
	            ? SB_ZF_CAPPCAMCONFIGURATIONENTRY_SIZE_IN_WORDS
	            : SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_SIZE_IN_WORDS;
  uint32_t uCfgBuffer[SB_ZF_C2PPCAMCONFIGURATIONINSTANCE0ENTRY_SIZE_IN_WORDS];
  uint32_t ram_size = (!SOC_IS_SBX_FE2KXT(unit))
	            ? SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_SIZE_IN_WORDS
	            : SB_ZF_C2PPCAMRAMCONFIGURATIONINSTANCE0ENTRY_SIZE_IN_WORDS;
  uint32_t uCfgRamBuffer[SB_ZF_C2PPCAMRAMCONFIGURATIONINSTANCE0ENTRY_SIZE_IN_WORDS];
  uint32_t addr = ((uCamId << SAND_HAL_FE2000_FIELD_SHIFT(pFe->regSet, PP_TCAM_CONFIG, MEM_ACC_CTRL_INSTANCE)) | uCamEntryIndex);


  if (sbFe2000UtilReadIndir(pFe->regSet, TRUE, TRUE,
                            SAND_HAL_FE2000_REG_OFFSET(pFe->regSet, PP_TCAM_CONFIG_MEM_ACC_CTRL),
                            addr, cam_size,
                            uCfgBuffer)) {
    return SB_TIMEOUT_ERR_CODE;
  }
  if (sbFe2000UtilReadIndir(pFe->regSet, TRUE, TRUE,
                            SAND_HAL_FE2000_REG_OFFSET(pFe->regSet, PP_CAM_RAM_MEM_ACC_CTRL), addr,
                            ram_size,
                            uCfgRamBuffer)) {
    return SB_TIMEOUT_ERR_CODE;
  }

  sbG2XXFePpCamEntry_Unpack(unit, pCamEntry, (uint8_t *) &uCfgBuffer[0]);
  sbG2XXFePpRamEntry_Unpack(unit, pRamEntry, (uint8_t *) &uCfgRamBuffer[0]);
  return SB_OK;
}

sbStatus_t
soc_sbx_g2xx_pp_psc_set (int unit, uint32_t fieldMask) {
    return soc_sbx_g2xx_pp_psc_config(unit, fieldMask);
}

/* END */
