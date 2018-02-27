/*
 * $Id: time.c 1.24.6.17 Broadcom SDK $
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
 * Time - Broadcom StrataSwitch Time BroadSync API.
 */

#include <bcm/time.h>
#include <bcm/error.h>

#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw/time.h>
#include <bcm_int/esw_dispatch.h>

#define BROAD_SYNC_TIME_CAPTURE_TIMEOUT      (10) /* useconds */
#define BROAD_SYNC_OUTPUT_TOGGLE_TIME_DELAY  (3)  /* seconds */ 

#define SYNT_TIME_SECONDS(unit, id) \
        TIME_INTERFACE_CONFIG(unit, id).time_capture.syntonous.seconds
#define SYNT_TIME_NANOSECONDS(unit, id) \
        TIME_INTERFACE_CONFIG(unit, id).time_capture.syntonous.nanoseconds

/****************************************************************************/
/*                      LOCAL VARIABLES DECLARATION                         */
/****************************************************************************/
static _bcm_time_config_p _bcm_time_config[BCM_MAX_NUM_UNITS] = {NULL};

static bcm_time_spec_t _bcm_time_accuracy_arr[TIME_ACCURACY_CLK_MAX] = {
      {0, COMPILER_64_INIT(0,0),  25},        /* HW value = 32, accuracy up tp 25 nanosec */
      {0, COMPILER_64_INIT(0,0),  100},       /* HW value = 33, accuracy up to 100 nanosec */
      {0, COMPILER_64_INIT(0,0),  250},       /* HW value = 34, accuracy up to 250 nanosec */
      {0, COMPILER_64_INIT(0,0),  1000},      /* HW value = 35, accuracy up to 1 microsec */
      {0, COMPILER_64_INIT(0,0),  2500},      /* HW value = 36, accuracy up to 2.5 microsec */
      {0, COMPILER_64_INIT(0,0),  10000},     /* HW value = 37, accuracy up to 10 microsec */
      {0, COMPILER_64_INIT(0,0),  25000},     /* HW value = 38, accuracy up to 25 microsec */
      {0, COMPILER_64_INIT(0,0),  100000},    /* HW value = 39, accuracy up to 100 microsec */
      {0, COMPILER_64_INIT(0,0),  250000},    /* HW value = 40, accuracy up to 250 microsec */
      {0, COMPILER_64_INIT(0,0),  1000000},   /* HW value = 41, accuracy up to 1 milisec */
      {0, COMPILER_64_INIT(0,0),  2500000},   /* HW value = 42, accuracy up to 2.5 milisec */
      {0, COMPILER_64_INIT(0,0),  10000000},  /* HW value = 43, accuracy up to 10 milisec */
      {0, COMPILER_64_INIT(0,0),  25000000},  /* HW value = 44, accuracy up to 25 milisec */
      {0, COMPILER_64_INIT(0,0),  100000000}, /* HW value = 45, accuracy up to 100 milisec */
      {0, COMPILER_64_INIT(0,0),  250000000}, /* HW value = 46, accuracy up to 250 milisec */
      {0, COMPILER_64_INIT(0,1),  0},         /* HW value = 47, accuracy up to 1 sec */
      {0, COMPILER_64_INIT(0,10), 0},        /* HW value = 48, accuracy up to 10 sec */
      /* HW value = 49, accuracy greater than 10 sec */
      {0, COMPILER_64_INIT(0,TIME_ACCURACY_INFINITE), TIME_ACCURACY_INFINITE},         
      /* HW value = 254 accuracy unknown */
      {0, COMPILER_64_INIT(0,TIME_ACCURACY_UNKNOWN), TIME_ACCURACY_UNKNOWN}          
};

static uint32 _bcm_time_bs_supported_frequencies[TIME_BS_FREQUENCIES_NUM] = {
    TIME_BS_FREQUENCY_1000NS,
    TIME_BS_FREQUENCY_1024NS,
    TIME_BS_FREQUENCY_1544NS,
    TIME_BS_FREQUENCY_2000NS,
    TIME_BS_FREQUENCY_2048NS
};

/****************************************************************************/
/*                      Internal functions implmentation                    */
/****************************************************************************/
STATIC int 
_bcm_esw_time_capture_get (int unit, bcm_time_if_t id, bcm_time_capture_t *time);
/*
 * Function:
 *	  _bcm_esw_time_hw_clear
 * Purpose:
 *	  Internal routine used to clear all HW registers and table to default values
 * Parameters:
 *    unit           - (IN) BCM device number.
 *    intf_id        - (IN) Time interface identifier
 * Returns:
 *	  BCM_E_XXX
 */
STATIC int 
_bcm_esw_time_hw_clear(int unit, bcm_time_if_t intf_id)
{
    uint32      regval;

    if (SOC_REG_IS_VALID(unit, CMIC_BS_DRIFT_RATEr)) {
        /* Reset Drift Rate  */
        READ_CMIC_BS_DRIFT_RATEr(unit, &regval);
        soc_reg_field_set(unit, CMIC_BS_DRIFT_RATEr, &regval, SIGNf, 0);
        soc_reg_field_set(unit, CMIC_BS_DRIFT_RATEr, &regval, FRAC_NSf, 0);
        WRITE_CMIC_BS_DRIFT_RATEr(unit, regval);
    }

    
    if (SOC_REG_IS_VALID(unit, CMIC_BS_OFFSET_ADJUST_0r)) {
        /* Reset Offset Adjust */
        READ_CMIC_BS_OFFSET_ADJUST_0r(unit, &regval);
        soc_reg_field_set(unit, CMIC_BS_OFFSET_ADJUST_0r, &regval, SECONDf, 0);
        WRITE_CMIC_BS_OFFSET_ADJUST_0r(unit, regval);
        READ_CMIC_BS_OFFSET_ADJUST_1r(unit, &regval);
        soc_reg_field_set(
                unit, CMIC_BS_OFFSET_ADJUST_1r, &regval, SIGN_BITf, 0);
        soc_reg_field_set(unit, CMIC_BS_OFFSET_ADJUST_1r, &regval, NSf, 0);
        WRITE_CMIC_BS_OFFSET_ADJUST_1r(unit, regval);
    }

    /* Reset Config register */
#if defined(BCM_KATANA_SUPPORT)
    if (SOC_IS_KATANA(unit)) {
        READ_CMIC_BS_CONFIGr(unit, &regval);
        soc_reg_field_set(
            unit, CMIC_BS_CONFIGr, &regval, MODEf, TIME_MODE_OUTPUT);
        soc_reg_field_set(unit, CMIC_BS_CONFIGr, &regval, BS_TC_OUTPUT_ENABLEf, 0);
        soc_reg_field_set(unit, CMIC_BS_CONFIGr, &regval, BS_CLK_OUTPUT_ENABLEf, 0);
        WRITE_CMIC_BS_CONFIGr(unit, regval);

        READ_CMIC_BS_OUTPUT_TIME_0r(unit, &regval);
        soc_reg_field_set(unit, CMIC_BS_OUTPUT_TIME_0r, &regval, ACCURACYf, 0);
        soc_reg_field_set(unit, CMIC_BS_OUTPUT_TIME_0r, &regval, LOCKf, 0);
        soc_reg_field_set(unit, CMIC_BS_OUTPUT_TIME_0r, &regval, EPOCHf, 0);
        WRITE_CMIC_BS_OUTPUT_TIME_0r(unit, regval);
    } else
#endif /* BCM_KATANA_SUPPORT */
    {
        READ_CMIC_BS_CONFIGr(unit, &regval);
        soc_reg_field_set(unit, CMIC_BS_CONFIGr, &regval, ENABLEf, 0);
        soc_reg_field_set(unit, CMIC_BS_CONFIGr, &regval, MODEf, TIME_MODE_OUTPUT);
        soc_reg_field_set(unit, CMIC_BS_CONFIGr, &regval, TIME_CODE_ENABLEf, 0);
        soc_reg_field_set(unit, CMIC_BS_CONFIGr, &regval, LOCKf, 0);
        soc_reg_field_set(unit, CMIC_BS_CONFIGr, &regval, ACCURACYf, 
                          TIME_ACCURACY_UNKNOWN_HW_VAL);
        soc_reg_field_set(unit, CMIC_BS_CONFIGr, &regval, EPOCHf, 0);
        WRITE_CMIC_BS_CONFIGr(unit, regval);
    }

    /* Reset Clock Control */
    
#if defined(BCM_KATANA_SUPPORT)
    if (SOC_IS_KATANA(unit)) {
        READ_CMIC_BS_CLK_CTRLr(unit, &regval);
        soc_reg_field_set(unit, CMIC_BS_CLK_CTRLr, &regval, ENABLEf, 0);
        WRITE_CMIC_BS_CLK_CTRLr(unit, regval);
    } else 
#endif /* BCM_KATANA_SUPPORT */
    {
        READ_CMIC_BS_CLK_CTRL_0r(unit, &regval);
        soc_reg_field_set(unit, CMIC_BS_CLK_CTRL_0r, &regval, ENABLEf, 0);
        soc_reg_field_set(unit, CMIC_BS_CLK_CTRL_0r, &regval, NSf, 0);
        WRITE_CMIC_BS_CLK_CTRL_0r(unit, regval);
    }

    if (SOC_REG_IS_VALID(unit, CMIC_BS_CLK_CTRL_1r)) {
        READ_CMIC_BS_CLK_CTRL_1r(unit, &regval);
        soc_reg_field_set(unit, CMIC_BS_CLK_CTRL_1r, &regval, FRAC_NSf, 0);
        WRITE_CMIC_BS_CLK_CTRL_1r(unit, regval);
    }

    /* Reset Clock Toggle  */
    if (SOC_REG_IS_VALID(unit, CMIC_BS_CLK_TOGGLE_TIME_0r)) {
        READ_CMIC_BS_CLK_TOGGLE_TIME_0r(unit, &regval);
        soc_reg_field_set(unit, CMIC_BS_CLK_TOGGLE_TIME_0r, &regval, SECf, 0);
        WRITE_CMIC_BS_CLK_TOGGLE_TIME_0r(unit, regval);
    }

    if (SOC_REG_IS_VALID(unit, CMIC_BS_CLK_TOGGLE_TIME_1r)) {
        READ_CMIC_BS_CLK_TOGGLE_TIME_1r(unit, &regval);
        soc_reg_field_set(unit, CMIC_BS_CLK_TOGGLE_TIME_1r, &regval, NSf, 0);
        WRITE_CMIC_BS_CLK_TOGGLE_TIME_1r(unit, regval);
    }
    
    if (SOC_REG_IS_VALID(unit, CMIC_BS_CLK_TOGGLE_TIME_2r)) {
        READ_CMIC_BS_CLK_TOGGLE_TIME_2r(unit, &regval);
        soc_reg_field_set(unit, CMIC_BS_CLK_TOGGLE_TIME_2r, &regval, FRAC_NSf, 0);
        WRITE_CMIC_BS_CLK_TOGGLE_TIME_2r(unit, regval);
    }

    /* Reset HeartBeat */
    READ_CMIC_BS_HEARTBEAT_CTRLr(unit, &regval);
    soc_reg_field_set(unit, CMIC_BS_HEARTBEAT_CTRLr, &regval, ENABLEf, 0);
    if (!(SOC_IS_KATANA(unit))) {
        soc_reg_field_set(unit, CMIC_BS_HEARTBEAT_CTRLr, &regval, THRESHOLDf, 0);
    }
    WRITE_CMIC_BS_HEARTBEAT_CTRLr(unit, regval);

    /* Reset Capture */
#if (defined(BCM_KATANA_SUPPORT))
    if (SOC_IS_KATANA(unit) ) {
        READ_CMIC_TS_TIME_CAPTURE_CTRLr(unit, &regval);
        /* Configure the HW to give interrupt on every heartbeat */
        soc_reg_field_set(unit, CMIC_TS_TIME_CAPTURE_CTRLr, &regval, 
                          INT_ENABLEf, 0);
        /* Configure the HW to capture time on every heartbeat */
        soc_reg_field_set(unit, CMIC_TS_TIME_CAPTURE_CTRLr, &regval, 
                        TIME_CAPTURE_MODEf, TIME_CAPTURE_MODE_HEARTBEAT);
        WRITE_CMIC_TS_TIME_CAPTURE_CTRLr(unit, regval);
    } else
#endif /* defined(BCM_KATANA_SUPPORT) */
    {
        READ_CMIC_BS_CAPTURE_CTRLr(unit, &regval);
        soc_reg_field_set(unit, CMIC_BS_CAPTURE_CTRLr, &regval, INT_ENf, 0);
        soc_reg_field_set(unit, CMIC_BS_CAPTURE_CTRLr, &regval, TIME_CAPTURE_MODEf,
                          TIME_CAPTURE_MODE_DISABLE);
        WRITE_CMIC_BS_CAPTURE_CTRLr(unit, regval);
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *	  _bcm_esw_time_deinit
 * Purpose:
 *	  Internal routine used to free time software module
 *    control structures. 
 * Parameters:
 *    unit           - (IN) BCM device number.
 *    time_cfg_pptr  - (IN) Pointer to pointer to time config structure.
 * Returns:
 *	  BCM_E_XXX
 */
STATIC int
_bcm_esw_time_deinit(int unit, _bcm_time_config_p *time_cfg_pptr)
{

    int                 idx;
    _bcm_time_config_p  time_cfg_ptr;
    soc_control_t       *soc = SOC_CONTROL(unit);

    /* Sanity checks. */
    if (NULL == time_cfg_pptr) {
        return (BCM_E_PARAM);
    }

    time_cfg_ptr = *time_cfg_pptr;
    /* If time config was not allocated we are done. */
    if (NULL == time_cfg_ptr) {
        return (BCM_E_NONE);
    }

    /* Free time interface */
    if (NULL != time_cfg_ptr->intf_arr) {
        for (idx = 0; idx < NUM_TIME_INTERFACE(unit); idx++) {
            if (NULL !=  time_cfg_ptr->intf_arr[idx].user_cb) {
                sal_free(time_cfg_ptr->intf_arr[idx].user_cb);
            }
        }
        sal_free(time_cfg_ptr->intf_arr);
    }

    /* Destroy protection mutex. */
    if (NULL != time_cfg_ptr->mutex) {
        sal_mutex_destroy(time_cfg_ptr->mutex);
    }

    /* If any registered function - deregister */
    soc->time_call_ref_count = 0;
    soc->soc_time_callout = NULL;

    /* Free module configuration structue. */
    sal_free(time_cfg_ptr);
    *time_cfg_pptr = NULL;
    return (BCM_E_NONE);
}

#ifdef BCM_WARM_BOOT_SUPPORT
/*
 * Function:
 *	  _bcm_esw_time_reinit
 * Purpose:
 *	  Internal routine used to reinitialize time module based on HW settings
 *    during Warm boot 
 * Parameters:
 *    unit           - (IN) BCM device number.
 *    intf_id        - (IN) Time interface identifier
 * Returns:
 *	  BCM_E_XXX
 */
STATIC int 
_bcm_esw_time_reinit(int unit, bcm_time_if_t intf_id)
{
    int hb_enable; 

    /* Read the status of heartbeat interrupt */
    BCM_IF_ERROR_RETURN(
        bcm_esw_time_heartbeat_enable_get(unit, intf_id, &hb_enable));

    /* If heartbeat was enabled, restore handling functionality */
    if (hb_enable) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_time_heartbeat_enable_set(unit, intf_id, hb_enable));
    }

    return (BCM_E_NONE);
}
#endif /* BCM_WARM_BOOT_SUPPORT */
/*
 * Function:
 *	  _bcm_esw_time_interface_id_validate
 * Purpose:
 *	  Internal routine used to validate interface identifier 
 * Parameters:
 *    unit           - (IN) BCM device number.
 *    id             - (IN) Interface id to validate
 * Returns:
 *	  BCM_E_XXX
 */
STATIC int 
_bcm_esw_time_interface_id_validate(int unit, bcm_time_if_t id)
{
    if (0 == TIME_INIT(unit)) {
        return (BCM_E_INIT);
    }
    if (id < 0 || id > TIME_INTERFACE_ID_MAX(unit)) {
        return (BCM_E_PARAM);
    }
    if (NULL == TIME_INTERFACE(unit, id)) {
        return (BCM_E_NOT_FOUND);
    }

    return (BCM_E_NONE);
}
/*
 * Function:
 *	  _bcm_esw_time_interface_input_validate
 * Purpose:
 *	  Internal routine used to validate interface input 
 * Parameters:
 *    unit           - (IN) BCM device number.
 *    intf           - (IN) Interface to validate
 * Returns:
 *	  BCM_E_XXX
 */
STATIC int 
_bcm_esw_time_interface_input_validate(int unit, bcm_time_interface_t *intf)
{
    /* Sanity checks. */
    if (NULL == intf) {
        return (BCM_E_PARAM);
    }
    if (intf->flags & BCM_TIME_WITH_ID) {
        if (intf->id < 0 || intf->id > TIME_INTERFACE_ID_MAX(unit) ) {
            return (BCM_E_PARAM);
        }
    }

    if (intf->flags & BCM_TIME_DRIFT) {
        if (intf->drift.nanoseconds > TIME_DRIFT_MAX) {
            return BCM_E_PARAM;
        }
    }

    if (intf->flags & BCM_TIME_OFFSET) {
        if (intf->offset.nanoseconds > TIME_NANOSEC_MAX) {
            return BCM_E_PARAM;
        }
    }

    return (BCM_E_NONE);
}

/*
 * Function:
 *	  _bcm_esw_time_interface_allocate_id
 * Purpose:
 *	  Internal routine used to allocate time interface id 
 * Parameters:
 *    unit           - (IN) BCM device number.
 *    id             - (OUT) Interface id to be allocated
 * Returns:
 *	  BCM_E_XXX
 */
STATIC int 
_bcm_esw_time_interface_allocate_id(int unit, bcm_time_if_t *id)
{
    int                              idx;  /* Time interfaces iteration index.*/
    _bcm_time_interface_config_p     intf; /* Time interface description.     */

    /* Input parameters check. */
    if (NULL == id) {
        return (BCM_E_PARAM);
    }

    /* Find & allocate time interface. */
    for (idx = 0; idx < TIME_CONFIG(unit)->intf_count; idx++) {
        intf = TIME_CONFIG(unit)->intf_arr + idx;
        if (intf->ref_count) {  /* In use interface */
            continue;
        }
        intf->ref_count++;  /* If founf mark interface as in use */
        *id = intf->time_interface.id; /* Assign ID */
        return (BCM_E_NONE);
    }

    /* No available interfaces */
    return (BCM_E_FULL);
}


/*
 * Function:
 *	  _bcm_esw_time_interface_heartbeat_install
 * Purpose:
 *	  Internal routine used to install interface heartbeat rate into a HW 
 * Parameters:
 *    unit           - (IN) BCM device number.
 *    id             - (IN) Interface id to be installed into a HW
 * Returns:
 *	  BCM_E_XXX
 */
STATIC int 
_bcm_esw_time_interface_heartbeat_install(int unit, bcm_time_if_t id)
{
    uint32                  regval, best_modulo, modulo;
    uint32                  hb_hz;  /* Number of heartbeats in HZ */
    uint32                  ns_hb;  /* Number of nanoseconds per heartbeat */ 
    uint32                  ns_hp;  /* Number of nanosec per half clock cycle*/
    uint32                  threshold; /* Heartbeat threshold */
    bcm_time_interface_t    *intf;
    int                     i, best_modulo_idx;

    intf = TIME_INTERFACE(unit, id);

    hb_hz = (intf->heartbeat_hz > TIME_HEARTBEAT_HZ_MAX) ? 
        TIME_HEARTBEAT_HZ_MAX : intf->heartbeat_hz;
    hb_hz = (intf->heartbeat_hz < TIME_HEARTBEAT_HZ_MIN) ? 
        TIME_HEARTBEAT_HZ_MIN : hb_hz;

    /* Calculate how many nano-seconds in requested hz */
    ns_hb = TIME_HEARTBEAT_NS_HZ / hb_hz;

    /* Find the most accurate period of cycle of a bit clock */
    best_modulo = ns_hb % _bcm_time_bs_supported_frequencies[0];;
    best_modulo_idx = 0;
    for (i = 1; i < TIME_BS_FREQUENCIES_NUM; i++) {
        modulo = ns_hb % _bcm_time_bs_supported_frequencies[i];
        if (modulo < best_modulo) {
            best_modulo = modulo;
            best_modulo_idx = i;
        }
    }

    /* Calculate nanoseconds per half period of cycle */
    ns_hp = _bcm_time_bs_supported_frequencies[best_modulo_idx] / 2;

    /* program the half period of broadsync clock */

#if defined(BCM_KATANA_SUPPORT)
    if (SOC_IS_KATANA(unit)) {
        READ_CMIC_BS_CLK_CTRLr(unit, &regval);
        soc_reg_field_set(unit, CMIC_BS_CLK_CTRLr, &regval, ENABLEf, 1);
        WRITE_CMIC_BS_CLK_CTRLr(unit, regval);

    } else 
#endif /* BCM_KATANA_SUPPORT */
    {
        READ_CMIC_BS_CLK_CTRL_0r(unit, &regval);
        soc_reg_field_set(unit, CMIC_BS_CLK_CTRL_0r, &regval, ENABLEf, 1);
        soc_reg_field_set(unit, CMIC_BS_CLK_CTRL_0r, &regval, NSf, ns_hp);
        WRITE_CMIC_BS_CLK_CTRL_0r(unit, regval);
    }

    /* Calculate the threshold for heartbeat */
    threshold = (ns_hb / _bcm_time_bs_supported_frequencies[best_modulo_idx]);

    /* Install correct hw value */
    READ_CMIC_BS_HEARTBEAT_CTRLr(unit, &regval);
    soc_reg_field_set(unit, CMIC_BS_HEARTBEAT_CTRLr, &regval, ENABLEf, 1); 
    if (!(SOC_IS_KATANA(unit) )) {
        soc_reg_field_set(unit, CMIC_BS_HEARTBEAT_CTRLr, &regval, 
                          THRESHOLDf, threshold); 
    }
    WRITE_CMIC_BS_HEARTBEAT_CTRLr(unit, regval);

#if defined(BCM_KATANA_SUPPORT)
    /* Install heartbeat and external trigger clock interval */
    if ((SOC_IS_KATANA(unit))) {
        uint32 interval;
        /* Install heartbeat up and down interval */ 
        READ_CMIC_BS_HEARTBEAT_UP_DURATIONr(unit, &interval);
        soc_reg_field_set(unit, CMIC_BS_HEARTBEAT_UP_DURATIONr, &interval, 
                          UP_DURATIONf, threshold); 
        WRITE_CMIC_BS_HEARTBEAT_UP_DURATIONr(unit, interval);

        READ_CMIC_BS_HEARTBEAT_DOWN_DURATIONr(unit, &interval);
        soc_reg_field_set(unit, CMIC_BS_HEARTBEAT_DOWN_DURATIONr, &interval, 
                          DOWN_DURATIONf, threshold); 
        WRITE_CMIC_BS_HEARTBEAT_DOWN_DURATIONr(unit, interval);

        
       /* Install divisor for LCPLL trigger interval */
        READ_CMIC_TS_LCPLL_CLK_COUNT_CTRLr(unit, &interval);
        soc_reg_field_set(unit, CMIC_TS_LCPLL_CLK_COUNT_CTRLr, &interval, 
                          DIVISORf, threshold);
        WRITE_CMIC_TS_LCPLL_CLK_COUNT_CTRLr(unit, interval);

       /* Install divisor for Primary L1 trigger interval */
        READ_CMIC_TS_L1_CLK_RECOVERED_CLK_COUNT_CTRLr(unit, &interval);
        soc_reg_field_set(unit,  CMIC_TS_L1_CLK_RECOVERED_CLK_COUNT_CTRLr, &interval, 
                          DIVISORf, threshold);
        WRITE_CMIC_TS_L1_CLK_RECOVERED_CLK_COUNT_CTRLr(unit, interval);

        /* Install divisor for backup L1 trigger interval */
        READ_CMIC_TS_L1_CLK_RECOVERED_CLK_BKUP_COUNT_CTRLr(unit, &interval);
        soc_reg_field_set(unit,  CMIC_TS_L1_CLK_RECOVERED_CLK_BKUP_COUNT_CTRLr, &interval, 
                          DIVISORf, threshold);
        WRITE_CMIC_TS_L1_CLK_RECOVERED_CLK_BKUP_COUNT_CTRLr(unit, interval);

        /* 
         * KT-1381 :
         *  TS_GPIO_LOW = (UP_EVENT_INTERVAL x 4) + 1 TS_REF_CLK_time_period
         *  TS_GPIO_HIGH = (DOWN_EVENT_INTERVAL x 4) + 1 TS_REF_CLK_time_period
         */ 
        threshold = (threshold * 4) + SOC_TIMESYNC_PLL_CLOCK_NS(unit);

        /* Install GPIO timesync trigger interval */
        /* GPIO_EVENT_1 */
        READ_CMIC_TS_GPIO_1_DOWN_EVENT_CTRLr(unit, &interval);
        soc_reg_field_set(unit, CMIC_TS_GPIO_1_DOWN_EVENT_CTRLr, &interval,
                                INTERVAL_LENGTHf, threshold);
        WRITE_CMIC_TS_GPIO_1_DOWN_EVENT_CTRLr(unit, interval);
        READ_CMIC_TS_GPIO_1_UP_EVENT_CTRLr(unit, &interval);
        soc_reg_field_set(unit, CMIC_TS_GPIO_1_UP_EVENT_CTRLr, &interval,
                                INTERVAL_LENGTHf, threshold);
        WRITE_CMIC_TS_GPIO_1_UP_EVENT_CTRLr(unit, interval);

        /* GPIO_EVENT_2 */
        READ_CMIC_TS_GPIO_2_DOWN_EVENT_CTRLr(unit, &interval);
        soc_reg_field_set(unit, CMIC_TS_GPIO_2_DOWN_EVENT_CTRLr, &interval,
                                INTERVAL_LENGTHf, threshold);
        WRITE_CMIC_TS_GPIO_2_DOWN_EVENT_CTRLr(unit, interval);
        READ_CMIC_TS_GPIO_2_UP_EVENT_CTRLr(unit, &interval);
        soc_reg_field_set(unit, CMIC_TS_GPIO_2_UP_EVENT_CTRLr, &interval,
                                INTERVAL_LENGTHf, threshold);
        WRITE_CMIC_TS_GPIO_2_UP_EVENT_CTRLr(unit, interval);

        /* GPIO_EVENT_3 */
        READ_CMIC_TS_GPIO_3_DOWN_EVENT_CTRLr(unit, &interval);
        soc_reg_field_set(unit, CMIC_TS_GPIO_3_DOWN_EVENT_CTRLr, &interval,
                                INTERVAL_LENGTHf, threshold);
        WRITE_CMIC_TS_GPIO_3_DOWN_EVENT_CTRLr(unit, interval);
        READ_CMIC_TS_GPIO_3_UP_EVENT_CTRLr(unit, &interval);
        soc_reg_field_set(unit, CMIC_TS_GPIO_3_UP_EVENT_CTRLr, &interval,
                                INTERVAL_LENGTHf, threshold);
        WRITE_CMIC_TS_GPIO_3_UP_EVENT_CTRLr(unit, interval);

        /* GPIO_EVENT_4 */
        READ_CMIC_TS_GPIO_4_DOWN_EVENT_CTRLr(unit, &interval);
        soc_reg_field_set(unit, CMIC_TS_GPIO_4_DOWN_EVENT_CTRLr, &interval,
                                INTERVAL_LENGTHf, threshold);
        WRITE_CMIC_TS_GPIO_4_DOWN_EVENT_CTRLr(unit, interval);
        READ_CMIC_TS_GPIO_4_UP_EVENT_CTRLr(unit, &interval);
        soc_reg_field_set(unit, CMIC_TS_GPIO_4_UP_EVENT_CTRLr, &interval,
                                INTERVAL_LENGTHf, threshold);
        WRITE_CMIC_TS_GPIO_4_UP_EVENT_CTRLr(unit, interval);

        /* GPIO_EVENT_5 */
        READ_CMIC_TS_GPIO_5_DOWN_EVENT_CTRLr(unit, &interval);
        soc_reg_field_set(unit, CMIC_TS_GPIO_5_DOWN_EVENT_CTRLr, &interval,
                                INTERVAL_LENGTHf, threshold);
        WRITE_CMIC_TS_GPIO_5_DOWN_EVENT_CTRLr(unit, interval);
        READ_CMIC_TS_GPIO_5_UP_EVENT_CTRLr(unit, &interval);
        soc_reg_field_set(unit, CMIC_TS_GPIO_5_UP_EVENT_CTRLr, &interval,
                                INTERVAL_LENGTHf, threshold);
        WRITE_CMIC_TS_GPIO_5_UP_EVENT_CTRLr(unit, interval);
    }
#endif /* defined(BCM_KATANA_SUPPORT) */
    return (BCM_E_NONE);
}


/*
 * Function:
 *	  _bcm_esw_time_interface_accuracy_time2hw
 * Purpose:
 *	  Internal routine used to compute HW accuracy value from interface 
 * Parameters:
 *    unit           - (IN) BCM device number.
 *    id             - (IN) Interface id 
 *    accuracy       - (OUT) HW value to be programmed 
 * Returns:
 *	  BCM_E_XXX
 */
STATIC int 
_bcm_esw_time_interface_accuracy_time2hw(int unit, bcm_time_if_t id, 
                                         uint32 *accuracy)
{
    int                     idx;    /* accuracy itterator */
    bcm_time_interface_t    *intf;

    if (NULL == accuracy) {
        return BCM_E_PARAM;
    }

    intf = TIME_INTERFACE(unit, id);

    /* Find the right accuracy */
    for (idx = 0; idx < TIME_ACCURACY_CLK_MAX; idx++) {
        if (intf->accuracy.nanoseconds <= _bcm_time_accuracy_arr[idx].nanoseconds && 
             COMPILER_64_LO(intf->accuracy.seconds) <= COMPILER_64_LO(_bcm_time_accuracy_arr[idx].seconds) ) {
            break;
        }
    }
    /* if no match - return error */
    if (idx == TIME_ACCURACY_CLK_MAX) {
        return BCM_E_NOT_FOUND;
    }

    /* Return the correct HW value */

    *accuracy = TIME_ACCURACY_SW_IDX_2_HW(idx);

    return (BCM_E_NONE);
}

/*
 * Function:
 *	  _bcm_esw_time_interface_drift_install
 * Purpose:
 *	  Internal routine used to install interface drift into a HW 
 * Parameters:
 *    unit           - (IN) BCM device number.
 *    id             - (IN) Interface id to be installed into a HW
 * Returns:
 *	  BCM_E_XXX
 */
STATIC int 
_bcm_esw_time_interface_drift_install(int unit, bcm_time_if_t id)
{
    uint32 regval, hw_val, sign;
    bcm_time_interface_t    *intf;

#if defined(BCM_KATANA_SUPPORT)
    /* Katana does not support clock drift */
    if (SOC_IS_KATANA(unit)) {
        return BCM_E_UNAVAIL;
    }
#endif /* BCM_KATANA_SUPPORT */

    intf = TIME_INTERFACE(unit, id);
    sign = intf->drift.isnegative;

    /* Requested drift value should not be more then 1/8 of drift denominator */
    if (intf->drift.nanoseconds <= TIME_DRIFT_MAX) {
        hw_val = 8 * intf->drift.nanoseconds;
    } else {
        return (BCM_E_PARAM);
    }
    READ_CMIC_BS_DRIFT_RATEr(unit, &regval);
    soc_reg_field_set(unit, CMIC_BS_DRIFT_RATEr, &regval, SIGNf, sign);
    soc_reg_field_set(unit, CMIC_BS_DRIFT_RATEr, &regval, FRAC_NSf, hw_val);
    WRITE_CMIC_BS_DRIFT_RATEr(unit, regval);

    return (BCM_E_NONE);
}

/*
 * Function:
 *	  _bcm_esw_time_interface_offset_install
 * Purpose:
 *	  Internal routine used to install interface offset into a HW 
 * Parameters:
 *    unit           - (IN) BCM device number.
 *    id             - (IN) Interface id to be installed into a HW
 * Returns:
 *	  BCM_E_XXX
 */
STATIC int 
_bcm_esw_time_interface_offset_install(int unit, bcm_time_if_t id)
{
    uint32 regval, sign, hw_val;
    bcm_time_interface_t    *intf;

#if defined(BCM_KATANA_SUPPORT)
    /* Katana does not support clock offset */
    if (SOC_IS_KATANA(unit)) {
        return BCM_E_UNAVAIL;
    }
#endif /* BCM_KATANA_SUPPORT */

    intf = TIME_INTERFACE(unit, id);
    
    /* Negative value if local clock is faster and need negative adjustment */
    sign = intf->offset.isnegative;
    /* Write second's values into the HW */
    READ_CMIC_BS_OFFSET_ADJUST_0r(unit, &regval);
    hw_val = COMPILER_64_LO(intf->offset.seconds);
    soc_reg_field_set(unit, CMIC_BS_OFFSET_ADJUST_0r, &regval, SECONDf, hw_val);
    WRITE_CMIC_BS_OFFSET_ADJUST_0r(unit, regval);
    /* Write sign and nansecond's values into the HW */
    READ_CMIC_BS_OFFSET_ADJUST_1r(unit, &regval);
    soc_reg_field_set(unit, CMIC_BS_OFFSET_ADJUST_1r, &regval, 
                      SIGN_BITf, sign);
    soc_reg_field_set(unit, CMIC_BS_OFFSET_ADJUST_1r, &regval, NSf, 
                      intf->offset.nanoseconds);
    WRITE_CMIC_BS_OFFSET_ADJUST_1r(unit, regval);

    /* If necessary install the epoch */
    if (COMPILER_64_HI(intf->offset.seconds) > 0) {
        READ_CMIC_BS_CONFIGr(unit, &regval);
        hw_val = COMPILER_64_HI(intf->offset.seconds);
        if (hw_val > TIME_EPOCH_MAX) {
            hw_val = TIME_EPOCH_MAX;
        }
        soc_reg_field_set(unit, CMIC_BS_CONFIGr, &regval, EPOCHf, hw_val);
        WRITE_CMIC_BS_CONFIGr(unit, regval);
    }

    return (BCM_E_NONE);
}

#if defined(BCM_KATANA_SUPPORT)
/*
 * Function:
 *	  _bcm_esw_time_interface_ref_clock_install
 * Purpose:
 *	  Internal routine to install timesync clock divisor to 
 *    enable broadsync reference clock.
 * Parameters:
 *    unit           - (IN) BCM device number.
 *    id             - (IN) Interface id to be installed into a HW
 * Returns:
 *	  BCM_E_XXX
 */
STATIC int 
_bcm_esw_time_interface_ref_clock_install(int unit, bcm_time_if_t id)
{
    uint32 regval, ndiv, hdiv, ref_clk;
    bcm_time_interface_t    *intf;

    intf = TIME_INTERFACE(unit, id);
    
    /* Validate and calculate ts_clk divisor to generate reference clock */
    if (intf->clk_resolution <= 0) {
        return BCM_E_PARAM;
    }
    
    /* Maximum ts_clk frequency is of 25Mhz(40ns) */
    ref_clk = (intf->clk_resolution > TIME_TS_CLK_FREQUENCY_40NS) ?
              TIME_TS_CLK_FREQUENCY_40NS : intf->clk_resolution;
   
    /* Divisor is half period for reference clock */ 
    ndiv = TIME_TS_CLK_FREQUENCY_40NS / ref_clk;
    
    /* Divisor is ceiling of half period */
    hdiv = (ndiv + 1)/2 ? ndiv : 1;

    /* Enable Broadsync reference clock */
    READ_CMIC_BS_REF_CLK_GEN_CTRLr(unit, &regval); 
    soc_reg_field_set(unit, CMIC_BS_REF_CLK_GEN_CTRLr, &regval, ENABLEf, 1);
    soc_reg_field_set(unit, CMIC_BS_REF_CLK_GEN_CTRLr, &regval, DIVISORf, hdiv);
    WRITE_CMIC_BS_REF_CLK_GEN_CTRLr(unit, regval); 

    return (BCM_E_NONE);
}
#endif /* defined(BCM_KATANA_SUPPORT) */

/*
 * Function:
 *	  _bcm_esw_time_interface_install
 * Purpose:
 *	  Internal routine used to install interface settings into a HW 
 * Parameters:
 *    unit           - (IN) BCM device number.
 *    intf_id        - (IN) Interface id to be installed into a HW
 * Returns:
 *	  BCM_E_XXX
 */
STATIC int 
_bcm_esw_time_interface_install(int unit, bcm_time_if_t intf_id)
{
    bcm_time_interface_t    *intf;  /* Time interface */
    uint32                  regval; /* For register read and write operations */
    uint32                  hw_val; /* Value to program into a HW */
    uint32                  second, diff, delay = BROAD_SYNC_OUTPUT_TOGGLE_TIME_DELAY;
    int                     enable = 0;
    soc_reg_t               reg; 
    bcm_time_spec_t         toggle_time;

    if (NULL == TIME_INTERFACE(unit, intf_id)) {
        return BCM_E_PARAM;
    }

    intf = TIME_INTERFACE(unit, intf_id);

    READ_CMIC_BS_CONFIGr(unit, &regval);

#if defined(BCM_KATANA_SUPPORT)
    if (SOC_IS_KATANA(unit)) {
        if ((intf->flags & BCM_TIME_ENABLE)) {
            soc_reg_field_set(unit, CMIC_BS_CONFIGr, &regval, BS_CLK_OUTPUT_ENABLEf, 1);
            soc_reg_field_set(unit, CMIC_BS_CONFIGr, &regval, BS_TC_OUTPUT_ENABLEf, 1);
        } else {
            soc_reg_field_set(unit, CMIC_BS_CONFIGr, &regval, BS_CLK_OUTPUT_ENABLEf, 0);
            soc_reg_field_set(unit, CMIC_BS_CONFIGr, &regval, BS_TC_OUTPUT_ENABLEf, 0);
        }

        if ((intf->flags & BCM_TIME_LOCKED)) {
            soc_reg_field32_modify(unit, CMIC_BS_OUTPUT_TIME_0r, REG_PORT_ANY, LOCKf, 1);
        } else {
            soc_reg_field32_modify(unit, CMIC_BS_OUTPUT_TIME_0r, REG_PORT_ANY, LOCKf, 0);
        }

    } else 
#endif /* BCM_KATANA_SUPPORT */
    {
        if (intf->flags & BCM_TIME_ENABLE) {
            soc_reg_field_set(unit, CMIC_BS_CONFIGr, &regval, ENABLEf, 1);
            soc_reg_field_set(unit, CMIC_BS_CONFIGr, &regval, TIME_CODE_ENABLEf, 1);
        } else {
            soc_reg_field_set(unit, CMIC_BS_CONFIGr, &regval, ENABLEf, 0);
            soc_reg_field_set(unit, CMIC_BS_CONFIGr, &regval, TIME_CODE_ENABLEf, 0);
        }
    
        if ((intf->flags & BCM_TIME_LOCKED)) {
            soc_reg_field_set(unit, CMIC_BS_CONFIGr, &regval, LOCKf, 1);
        } else {
            soc_reg_field_set(unit, CMIC_BS_CONFIGr, &regval, LOCKf, 0);
        }
    }

    if (intf->flags & BCM_TIME_INPUT) {
        soc_reg_field_set(unit, CMIC_BS_CONFIGr, &regval, MODEf, 
                          TIME_MODE_INPUT);
    } else {
        soc_reg_field_set(unit, CMIC_BS_CONFIGr, &regval, MODEf, 
                          TIME_MODE_OUTPUT);

        if (intf->flags & BCM_TIME_ACCURACY) {
            BCM_IF_ERROR_RETURN(
                _bcm_esw_time_interface_accuracy_time2hw(unit, intf_id, &hw_val));
            if (SOC_IS_KATANA(unit)) {
                soc_reg_field32_modify(unit, CMIC_BS_OUTPUT_TIME_0r, 
                                       REG_PORT_ANY, ACCURACYf, hw_val);
            } else {
                soc_reg_field_set(unit, CMIC_BS_CONFIGr, &regval, ACCURACYf, hw_val);
            }
        }

        if (intf->flags & BCM_TIME_HEARTBEAT) {
            BCM_IF_ERROR_RETURN(
                _bcm_esw_time_interface_heartbeat_install(unit, intf_id));

            if (!(intf->flags & BCM_TIME_OFFSET)) {
                BCM_IF_ERROR_RETURN(
                    _bcm_esw_time_capture_get(unit, intf_id, TIME_CAPTURE(unit, intf_id)));
                second = COMPILER_64_LO(SYNT_TIME_SECONDS(unit, intf_id));
                second += delay;
                WRITE_CMIC_BS_CLK_TOGGLE_TIME_0r(unit, second);
                WRITE_CMIC_BS_CLK_TOGGLE_TIME_1r(unit, SYNT_TIME_NANOSECONDS(unit, intf_id));
            }
        }
    }
    WRITE_CMIC_BS_CONFIGr(unit, regval);

    if (intf->flags & BCM_TIME_OFFSET) {
        if(SOC_REG_IS_VALID(unit, CMIC_BS_CLK_CTRL_0r)) {
            reg   =   CMIC_BS_CLK_CTRL_0r;
            READ_CMIC_BS_CLK_CTRL_0r(unit, &regval);
        } else {
            reg   =   CMIC_BS_CLK_CTRLr;
            READ_CMIC_BS_CLK_CTRLr(unit, &regval);
        }
        /* Stop broadsync output if already enabled */
        
        enable = soc_reg_field_get(unit, reg, regval, ENABLEf);
        if (1 == enable) {
            soc_reg_field_set(unit, reg, &regval, ENABLEf, 0);
            if (SOC_REG_IS_VALID(unit, CMIC_BS_CLK_CTRL_0r)) {
                WRITE_CMIC_BS_CLK_CTRL_0r(unit, regval);
            } else {
                WRITE_CMIC_BS_CLK_CTRLr(unit, regval);
            }
            BCM_IF_ERROR_RETURN(
                _bcm_esw_time_capture_get(unit, intf_id, TIME_CAPTURE(unit, intf_id)));
        }

        BCM_IF_ERROR_RETURN(
            _bcm_esw_time_interface_offset_install(unit, intf_id));

        if (intf->offset.isnegative) {
            second = COMPILER_64_LO(SYNT_TIME_SECONDS(unit, intf_id)) -
                     COMPILER_64_LO(intf->offset.seconds);

            if (intf->offset.nanoseconds <= SYNT_TIME_NANOSECONDS(unit, intf_id)) {
                toggle_time.nanoseconds = SYNT_TIME_NANOSECONDS(unit, intf_id) -
                                          intf->offset.nanoseconds;
            } else {
                /* Wrapped */
                diff = intf->offset.nanoseconds - SYNT_TIME_NANOSECONDS(unit, intf_id);
                toggle_time.nanoseconds = TIME_NANOSEC_MAX - diff;
                second--;
            }
            COMPILER_64_SET(toggle_time.seconds, 0, second); 
        } else {
            second = COMPILER_64_LO(SYNT_TIME_SECONDS(unit, intf_id)) +
                     COMPILER_64_LO(intf->offset.seconds);
            toggle_time.nanoseconds = 
                SYNT_TIME_NANOSECONDS(unit, intf_id) + intf->offset.nanoseconds;
            if (toggle_time.nanoseconds >= TIME_NANOSEC_MAX) {
                toggle_time.nanoseconds -= TIME_NANOSEC_MAX;
                second++;
            }
            COMPILER_64_SET(toggle_time.seconds, 0, second); 
        }

        if (1 == enable) {
            /* Reenable broadsync output */
            COMPILER_64_TO_32_LO(second, toggle_time.seconds);
            second += delay;
            if (SOC_REG_IS_VALID(unit, CMIC_BS_CLK_TOGGLE_TIME_0r)) {
                WRITE_CMIC_BS_CLK_TOGGLE_TIME_0r(unit, second);
            }

            if (SOC_REG_IS_VALID(unit, CMIC_BS_CLK_TOGGLE_TIME_1r)) {
                WRITE_CMIC_BS_CLK_TOGGLE_TIME_1r(unit, toggle_time.nanoseconds);
            }

            
            if (SOC_REG_IS_VALID(unit, CMIC_BS_CLK_TOGGLE_TIME_1r)) {
                READ_CMIC_BS_CLK_CTRL_0r(unit, &regval);
                soc_reg_field_set(unit, CMIC_BS_CLK_CTRL_0r, &regval, ENABLEf, 1);
                WRITE_CMIC_BS_CLK_CTRL_0r(unit, regval);
            } else {
                READ_CMIC_BS_CLK_CTRLr(unit, &regval);
                soc_reg_field_set(unit, CMIC_BS_CLK_CTRLr, &regval, ENABLEf, 1);
                WRITE_CMIC_BS_CLK_CTRLr(unit, regval);
            }
        }
    }

    if (intf->flags & BCM_TIME_DRIFT) {
        BCM_IF_ERROR_RETURN(
                _bcm_esw_time_interface_drift_install(unit, intf_id));
    }

    return (BCM_E_NONE);
}

/*
 * Function:
 *     _bcm_time_interface_free
 * Purpose:
 *     Free time interface.
 * Parameters:
 *      unit            - (IN) BCM device number. 
 *      intf_id         - (IN) time interface id.
 * Returns:
 *      BCM_X_XXX
 */

STATIC int 
_bcm_time_interface_free(int unit, bcm_time_if_t intf_id) 
{
    _bcm_time_interface_config_p  intf_cfg; /* Time interface config.*/

    intf_cfg = &TIME_INTERFACE_CONFIG(unit, intf_id); 

    if (intf_cfg->ref_count > 0) {
        intf_cfg->ref_count--;
    }

    if (0 == intf_cfg->ref_count) {
        sal_memset(&intf_cfg->time_interface, 0, sizeof(bcm_time_interface_t));
        intf_cfg->time_interface.id = intf_id;
    }

    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_esw_time_accuracy_parse
 * Purpose:
 *      Internal routine to parse accuracy hw value into bcm_time_spec_t format
 * Parameters:
 *      unit        - (IN) StrataSwitch Unit #.
 *      accuracy    - (IN) Accuracy HW value 
 *      time        - (OUT) bcm_time_spec_t structure to contain accuracy 
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 *
 */
STATIC int 
_bcm_esw_time_accuracy_parse(int unit, uint32 accuracy, bcm_time_spec_t *time)
{
    if (accuracy < TIME_ACCURACY_LOW_HW_VAL || 
        (accuracy > TIME_ACCURACY_HIGH_HW_VAL && 
         accuracy != TIME_ACCURACY_UNKNOWN_HW_VAL)) {
        return (BCM_E_PARAM);
    }

    time->isnegative = 
        _bcm_time_accuracy_arr[TIME_ACCURACY_HW_2_SW_IDX(accuracy)].isnegative;
    time->nanoseconds = 
        _bcm_time_accuracy_arr[TIME_ACCURACY_HW_2_SW_IDX(accuracy)].nanoseconds;
    time->seconds = 
        _bcm_time_accuracy_arr[TIME_ACCURACY_HW_2_SW_IDX(accuracy)].seconds;
    time->isnegative = 0;   /* Can't be negative */

    return (BCM_E_NONE);
}


/*
 * Function:
 *      _bcm_esw_time_input_parse
 * Purpose:
 *      Internal routine to parse input time information stored in 3 registeres
 * Parameters:
 *      unit    - (IN) StrataSwitch Unit #.
 *      data0   - (IN) Data stored in register 0 conrain input time information
 *      data1   - (IN) Data stored in register 1 conrain input time information
 *      data2   - (IN) Data stored in register 2 conrain input time information
 *      time    - (OUT) Structure to contain input time information
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 *
 */
STATIC int
_bcm_esw_time_input_parse(int unit, uint32 data0, uint32 data1, uint32 data2, 
                          bcm_time_capture_t *time)
{
    uint32 accuracy, sec_hi, sec_lo;

    if (data0 >> 31) {
        time->flags |= BCM_TIME_CAPTURE_LOCKED;
    }
    time->received.isnegative = 0;
    sec_hi = ((data0 << 1) >> 1);
    sec_lo = (data1 >> 14);
    COMPILER_64_SET(time->received.seconds, sec_hi, sec_lo);  
    time->received.nanoseconds = (data1 << 16);
    time->received.nanoseconds |= (data2 >> 8);

    accuracy = ((data2 << 24) >> 24);

    return _bcm_esw_time_accuracy_parse(unit,accuracy, 
                                        &(time->received_accuracy));
}

/*
 * Function:
 *      _bcm_esw_time_capture_counter_read
 * Purpose:
 *      Internal routine to read HW clocks
 * Parameters:
 *      unit    - (IN) StrataSwitch Unit #.
 *      id      - (IN) Time interface identifier
 *      time    - (OUT) Structure to contain HW clocks values
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 *
 */

STATIC int 
_bcm_esw_time_capture_counter_read(int unit, bcm_time_if_t id, 
                                   bcm_time_capture_t *time)
{
    uint32                  regval;
    bcm_time_interface_t    *intf;
    
#if defined(BCM_KATANA_SUPPORT)
    if (SOC_IS_KATANA(unit)) {
        /* Read free running time capture */
        READ_CMIC_BS_OUTPUT_TIME_1r(unit, &regval);
        COMPILER_64_SET(time->free.seconds, 0,  
            soc_reg_field_get(unit, CMIC_BS_OUTPUT_TIME_1r, 
                              regval, SECOND_TIMEf)); 
        READ_CMIC_BS_OUTPUT_TIME_2r(unit, &regval);
        time->free.nanoseconds =  
          soc_reg_field_get(unit, CMIC_BS_OUTPUT_TIME_2r, 
                            regval, NS_TIMEf); 
    } else 
#endif /* BCM_KATANA_SUPPORT */
    {
        /* Read free running time capture */
        READ_CMIC_BS_CAPTURE_FREE_RUN_TIME_0r(unit, &regval);
        COMPILER_64_SET(time->free.seconds, 0,  
            soc_reg_field_get(unit, CMIC_BS_CAPTURE_FREE_RUN_TIME_0r, 
                              regval, SECONDf)); 
        READ_CMIC_BS_CAPTURE_FREE_RUN_TIME_1r(unit, &regval);
        time->free.nanoseconds =  
         soc_reg_field_get(unit, CMIC_BS_CAPTURE_FREE_RUN_TIME_1r, regval, NSf); 
        /* Read syntonous time capture */
        READ_CMIC_BS_CAPTURE_SYNT_TIME_0r(unit, &regval);
        COMPILER_64_SET(time->syntonous.seconds, 0,
         soc_reg_field_get(unit, CMIC_BS_CAPTURE_SYNT_TIME_0r, regval, SECONDf));
        READ_CMIC_BS_CAPTURE_SYNT_TIME_1r(unit, &regval);
        time->syntonous.nanoseconds = 
            soc_reg_field_get(unit, CMIC_BS_CAPTURE_SYNT_TIME_1r, regval, NSf);
        /* Read synchronous time capture */
        READ_CMIC_BS_CAPTURE_SYNC_TIME_0r(unit, &regval);
        COMPILER_64_SET(time->synchronous.seconds, 0, 
         soc_reg_field_get(unit, CMIC_BS_CAPTURE_SYNC_TIME_0r, regval, SECONDf));
        READ_CMIC_BS_CAPTURE_SYNC_TIME_1r(unit, &regval);
        time->synchronous.nanoseconds = 
            soc_reg_field_get(unit, CMIC_BS_CAPTURE_SYNC_TIME_1r, regval, NSf);
    }

    time->free.isnegative = time->synchronous.isnegative = 
        time->syntonous.isnegative = 0;

    /* If interface is in input mode read time provided by the master */
    intf = TIME_INTERFACE(unit, id);
    if (intf->flags & BCM_TIME_INPUT) {
        uint32  data0, data1, data2;

        READ_CMIC_BS_INPUT_TIME_2r(unit, &regval);
        if (soc_reg_field_get(unit, CMIC_BS_INPUT_TIME_2r, regval, 
                              CHECKSUM_ERRORf)) {
            return BCM_E_INTERNAL;
        }

        data2 = soc_reg_field_get(unit, CMIC_BS_INPUT_TIME_2r, regval, DATAf);
        READ_CMIC_BS_INPUT_TIME_1r(unit, &regval);
        data1 = soc_reg_field_get(unit, CMIC_BS_INPUT_TIME_1r, regval, DATAf);
        READ_CMIC_BS_INPUT_TIME_0r(unit, &regval);
        data0 = soc_reg_field_get(unit, CMIC_BS_INPUT_TIME_0r, regval, DATAf);

        BCM_IF_ERROR_RETURN(
            _bcm_esw_time_input_parse(unit, data0, data1, data2, time));
    }

    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_esw_time_capture_get
 * Purpose:
 *      Internal routine to read HW clocks
 * Parameters:
 *      unit    - (IN) StrataSwitch Unit #.
 *      id      - (IN) Time interface identifier
 *      time    - (OUT) Structure to contain HW clocks values
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 *
 */
STATIC int 
_bcm_esw_time_capture_get (int unit, bcm_time_if_t id, bcm_time_capture_t *time)
{
    uint32          regval, orgval; /* To keep register value */
    int             hw_complete;    /* HW read completion indicator*/
    soc_timeout_t   timeout;        /* Timeout in case of HW error */

#if defined(BCM_KATANA_SUPPORT)
    if (SOC_IS_KATANA(unit)) {
        /* Read capture mode */
        READ_CMIC_TS_TIME_CAPTURE_CTRLr(unit, &regval);
        /* If in interrupt mode, return time captured on last interrupt */
        if (TIME_CAPTURE_MODE_HEARTBEAT == soc_reg_field_get(unit, 
                      CMIC_TS_TIME_CAPTURE_CTRLr, regval, TIME_CAPTURE_MODEf)) {
            return _bcm_esw_time_capture_counter_read(unit, id, time);
        }
        /* if different than interrupt remember original capture mode */
        orgval = regval;

        /* Program HW to disable time capture */
        soc_reg_field_set(unit, CMIC_TS_TIME_CAPTURE_CTRLr, &regval, 
                            TIME_CAPTURE_MODEf, TIME_CAPTURE_MODE_DISABLE);
        if (orgval != regval) {
            WRITE_CMIC_TS_TIME_CAPTURE_CTRLr(unit, regval);
        }
        
        /* Program HW to capture time immediately */
        soc_reg_field_set(unit, CMIC_TS_TIME_CAPTURE_CTRLr, &regval, 
                        TIME_CAPTURE_MODEf,TIME_CAPTURE_MODE_IMMEDIATE);
        WRITE_CMIC_TS_TIME_CAPTURE_CTRLr(unit, regval);

        /* Wait for HW time capture completion */
        hw_complete = 0;
        soc_timeout_init(&timeout, BROAD_SYNC_TIME_CAPTURE_TIMEOUT, 0);

        while (!hw_complete) {
            READ_CMIC_TS_CAPTURE_STATUSr(unit, &regval);
            hw_complete = soc_reg_field_get(unit, CMIC_TS_CAPTURE_STATUSr,
                                            regval, TIME_CAPTURE_COMPLETEf); 
            if (soc_timeout_check(&timeout)) {
                return (BCM_E_TIMEOUT);
            }
        }
        /* Read the HW time */
        BCM_IF_ERROR_RETURN(
            _bcm_esw_time_capture_counter_read(unit, id, time));

        /* Program HW to original time capture value */
        WRITE_CMIC_TS_TIME_CAPTURE_CTRLr(unit, orgval);

    } else
#endif /*(defined(BCM_KATANA_SUPPORT) */
    {
        /* Read capture mode */
        READ_CMIC_BS_CAPTURE_CTRLr(unit, &regval);
        /* If in interrupt mode, return time captured on last interrupt */
        if (TIME_CAPTURE_MODE_HEARTBEAT == soc_reg_field_get(unit, 
                            CMIC_BS_CAPTURE_CTRLr, regval, TIME_CAPTURE_MODEf)) {
            return _bcm_esw_time_capture_counter_read(unit, id, time);
        }
        /* if different than interrupt remember original capture mode */
        orgval = regval;

        /* Program HW to disable time capture */
        soc_reg_field_set(unit, CMIC_BS_CAPTURE_CTRLr, &regval, TIME_CAPTURE_MODEf,
                          TIME_CAPTURE_MODE_DISABLE);
        if (orgval != regval) {
            WRITE_CMIC_BS_CAPTURE_CTRLr(unit, regval);
        }
        
        /* Program HW to capture time immediately */
        soc_reg_field_set(unit, CMIC_BS_CAPTURE_CTRLr, &regval, TIME_CAPTURE_MODEf,
                          TIME_CAPTURE_MODE_IMMEDIATE);
        WRITE_CMIC_BS_CAPTURE_CTRLr(unit, regval);
 
       /* Wait for HW time capture completion */
        hw_complete = 0;
        soc_timeout_init(&timeout, BROAD_SYNC_TIME_CAPTURE_TIMEOUT, 0);

        while (!hw_complete) {
            READ_CMIC_BS_CAPTURE_STATUSr(unit, &regval);
            hw_complete = soc_reg_field_get(unit, CMIC_BS_CAPTURE_STATUSr, regval,
                                            TIME_CAPTURE_COMPLETEf); 
            if (soc_timeout_check(&timeout)) {
                return (BCM_E_TIMEOUT);
            }
        }
        /* Read the HW time */
        BCM_IF_ERROR_RETURN(
            _bcm_esw_time_capture_counter_read(unit, id, time));

        /* Program HW to original time capture value */
        WRITE_CMIC_BS_CAPTURE_CTRLr(unit, orgval);
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_esw_time_interface_offset_get
 * Purpose:
 *      Internal routine to read HW offset value and convert it into 
 *      bcm_time_spec_t structure 
 * Parameters:
 *      unit    -  (IN) StrataSwitch Unit #.
 *      id      -  (IN) Time interface identifier
 *      offset  - (OUT) Time interface  offset 
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 *
 */
STATIC int 
_bcm_esw_time_interface_offset_get(int unit, bcm_time_if_t id, 
                                   bcm_time_spec_t *offset)
{
    uint32 regval, sec, epoch;

#if defined(BCM_KATANA_SUPPORT)
    /* Katana does not support clock offset */
    if (SOC_IS_KATANA(unit)) {
        return BCM_E_UNAVAIL;
    }
#endif /* BCM_KATANA_SUPPORT */

    READ_CMIC_BS_OFFSET_ADJUST_0r(unit, &regval);
    sec = soc_reg_field_get(unit, CMIC_BS_OFFSET_ADJUST_0r, regval, SECONDf);
    READ_CMIC_BS_OFFSET_ADJUST_1r(unit, &regval);
    offset->nanoseconds= soc_reg_field_get(unit, CMIC_BS_OFFSET_ADJUST_1r,
                                           regval, NSf); 
    offset->isnegative = soc_reg_field_get(unit, CMIC_BS_OFFSET_ADJUST_1r, 
                                           regval, SIGN_BITf);
    /* If the epoch was installed read it */
    READ_CMIC_BS_CONFIGr(unit, &regval);
    epoch = soc_reg_field_get(unit,CMIC_BS_CONFIGr, regval, EPOCHf);

    COMPILER_64_SET(offset->seconds, epoch, sec);

    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_esw_time_interface_drift_get
 * Purpose:
 *      Internal routine to read HW drift value and convert it into 
 *      bcm_time_spec_t structure 
 * Parameters:
 *      unit    -  (IN) StrataSwitch Unit #.
 *      id      -  (IN) Time interface identifier
 *      drift   - (OUT) Time interface  drift 
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 *
 */
STATIC int 
_bcm_esw_time_interface_drift_get(int unit, bcm_time_if_t id,
                                  bcm_time_spec_t *drift)
{
    uint32 regval, val, ns;

#if defined(BCM_KATANA_SUPPORT)
    /* Katana does not support clock drift */
    if (SOC_IS_KATANA(unit)) {
        return BCM_E_UNAVAIL;
    }
#endif /* BCM_KATANA_SUPPORT */

    
    READ_CMIC_BS_DRIFT_RATEr(unit, &regval);
    val = soc_reg_field_get(unit, CMIC_BS_DRIFT_RATEr, regval, FRAC_NSf);

    ns = (val / 8);

    drift->nanoseconds = (ns > TIME_DRIFT_MAX) ? TIME_DRIFT_MAX : ns;

    drift->isnegative = soc_reg_field_get(unit, CMIC_BS_DRIFT_RATEr, 
                                          regval, SIGNf);
    return (BCM_E_NONE);
}

#if defined(BCM_KATANA_SUPPORT)
/*
 * Function:
 *      _bcm_esw_time_interface_ref_clock_get
 * Purpose:
 *      Internal routine to read ref clock divisor 
 * Parameters:
 *      unit    -  (IN) StrataSwitch Unit #.
 *      id      -  (IN) Time interface identifier
 *      divisor - (OUT) Time interface  ref clock  resolution
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 *
 */
STATIC int 
_bcm_esw_time_interface_ref_clock_get(int unit, bcm_time_if_t id,
                                  int *clk_resolution)
{
    uint32 regval, val, hdiv = 0;
   
    READ_CMIC_BS_REF_CLK_GEN_CTRLr(unit, &regval); 
    val = soc_reg_field_get(unit, CMIC_BS_REF_CLK_GEN_CTRLr, regval, ENABLEf);
    if (val) {
        /* Divisor is half period for reference clock */ 
        hdiv = soc_reg_field_get(unit, CMIC_BS_REF_CLK_GEN_CTRLr, regval, DIVISORf);
        *clk_resolution = TIME_TS_CLK_FREQUENCY_40NS / (hdiv * 2);
    } else {
        *clk_resolution = 0;
    }
 
    return (BCM_E_NONE);
}
#endif /* defined(BCM_KATANA_SUPPORT) */

/*
 * Function:
 *      _bcm_esw_time_interface_accuracy_get
 * Purpose:
 *      Internal routine to read HW accuracy value and convert it into 
 *      bcm_time_spec_t structure 
 * Parameters:
 *      unit        -  (IN) StrataSwitch Unit #.
 *      id          -  (IN) Time interface identifier
 *      accuracy    - (OUT) Time interface  accuracy 
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 *
 */
STATIC int 
_bcm_esw_time_interface_accuracy_get(int unit, bcm_time_if_t id,
                                     bcm_time_spec_t *accuracy)
{
    uint32  regval, val; 

#if defined(BCM_KATANA_SUPPORT)
    if (SOC_IS_KATANA(unit)) {
        READ_CMIC_BS_OUTPUT_TIME_0r(unit, &regval);
        val = soc_reg_field_get(unit, CMIC_BS_OUTPUT_TIME_0r, regval, ACCURACYf);
    } else 
#endif /* BCM_KATANA_SUPPORT */
    {
        READ_CMIC_BS_CONFIGr(unit, &regval);
        val = soc_reg_field_get(unit, CMIC_BS_CONFIGr, regval, ACCURACYf);
    }

    return _bcm_esw_time_accuracy_parse(unit, val, accuracy);
}

/*
 * Function:
 *      _bcm_esw_time_interface_get
 * Purpose:
 *      Internal routine to get a time sync interface by id
 * Parameters:
 *      unit -  (IN) StrataSwitch Unit #.
 *      id   -  (IN) Time interface identifier
 *      intf - (IN/OUT) Time Sync Interface to get
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 *
 */
STATIC int 
_bcm_esw_time_interface_get(int unit, bcm_time_if_t id, bcm_time_interface_t *intf)
{
    uint32                  regval;
    bcm_time_interface_t    *intf_ptr;

    intf_ptr = TIME_INTERFACE(unit, id);
    intf_ptr->flags = intf->flags;
    intf_ptr->id = id;
    READ_CMIC_BS_CONFIGr(unit, &regval);

    /* Update output flags */
    if (TIME_MODE_INPUT == soc_reg_field_get(unit, CMIC_BS_CONFIGr, 
                                             regval, MODEf)) {
        intf_ptr->flags |= BCM_TIME_INPUT;
    } else {
        intf_ptr->flags &= ~BCM_TIME_INPUT;
    }

#if defined(BCM_KATANA_SUPPORT)
    if (SOC_IS_KATANA(unit)) {
       if (soc_reg_field_get(unit, CMIC_BS_CONFIGr, 
                             regval, BS_CLK_OUTPUT_ENABLEf)) {
            intf_ptr->flags |= BCM_TIME_ENABLE;
        } else {
            intf_ptr->flags &= ~BCM_TIME_ENABLE;
        }

        READ_CMIC_BS_OUTPUT_TIME_0r(unit, &regval);
        if (soc_reg_field_get(unit, CMIC_BS_OUTPUT_TIME_0r,
                              regval, LOCKf)) {
            intf_ptr->flags |= BCM_TIME_LOCKED;
        } else {
            intf_ptr->flags &= ~BCM_TIME_LOCKED;
        }
    } else
#endif /* BCM_KATANA_SUPPORT */
    {
        if (soc_reg_field_get(unit, CMIC_BS_CONFIGr, regval, ENABLEf)) {
            intf_ptr->flags |= BCM_TIME_ENABLE;
        } else {
            intf_ptr->flags &= ~BCM_TIME_ENABLE;
        }
        if (soc_reg_field_get(unit, CMIC_BS_CONFIGr, regval, LOCKf)) {
            intf_ptr->flags |= BCM_TIME_LOCKED;
        } else {
            intf_ptr->flags &= ~BCM_TIME_LOCKED;
        }
    }

    if (intf->flags & BCM_TIME_ACCURACY) {
        BCM_IF_ERROR_RETURN(
            _bcm_esw_time_interface_accuracy_get(unit, id, 
                                                 &(intf_ptr->accuracy)));
    }
    
    if (intf->flags & BCM_TIME_OFFSET) {
        BCM_IF_ERROR_RETURN(
            _bcm_esw_time_interface_offset_get(unit, id, &(intf_ptr->offset)));
    }

    if (intf->flags & BCM_TIME_DRIFT) {
        BCM_IF_ERROR_RETURN(
             _bcm_esw_time_interface_drift_get(unit, id, &(intf_ptr->drift)));
    }

#if defined(BCM_KATANA_SUPPORT)
    if (SOC_IS_KATANA(unit)) {
        if (intf->flags & BCM_TIME_REF_CLOCK) {
            BCM_IF_ERROR_RETURN(
                _bcm_esw_time_interface_ref_clock_get(unit, id,
                                            &(intf_ptr->clk_resolution)));
        }
    }
#endif /* defined(BCM_KATANA_SUPPORT) */

    *intf = *(TIME_INTERFACE(unit, id));

    return (BCM_E_NONE);
}

/*
 * Function:    
 *      _bcm_esw_time_hw_interrupt_dflt
 * Purpose:     
 *      Default handler for broadsync heartbeat interrupt
 * Parameters:  
 *      unit - StrataSwitch unit #.
 * Returns:     
 *      Nothing
 */

STATIC void
_bcm_esw_time_hw_interrupt_dflt(int unit)
{
    uint32  regval;

    /* Due to HW constrains we need to reinable the interrupt on every rising */
    /* edge and update the capture mode */
#if defined(BCM_KATANA_SUPPORT)
    if (SOC_IS_KATANA(unit)) {
        READ_CMIC_TS_TIME_CAPTURE_CTRLr(unit, &regval);
        WRITE_CMIC_TS_TIME_CAPTURE_CTRLr(unit, 0);
        WRITE_CMIC_TS_TIME_CAPTURE_CTRLr(unit, regval);
    } else
#endif
    {
        READ_CMIC_BS_CAPTURE_CTRLr(unit, &regval);
        WRITE_CMIC_BS_CAPTURE_CTRLr(unit, 0);
        WRITE_CMIC_BS_CAPTURE_CTRLr(unit, regval);
    }
    return;
}

/*
 * Function:    
 *      _bcm_esw_time_hw_interrupt
 * Purpose:     
 *      Handles broadsync heartbeat interrupt
 * Parameters:  
 *      unit - StrataSwitch unit #.
 * Returns:     
 *      Nothing
 */
STATIC void
_bcm_esw_time_hw_interrupt(int unit)
{
    void                    *u_data; 
    bcm_time_heartbeat_cb   u_cb;
    int                     idx;    /* interface itterator */
    bcm_time_capture_t      time;
    
    for (idx = 0; idx < NUM_TIME_INTERFACE(unit); idx++) {
        if (NULL != TIME_INTERFACE(unit, idx) &&
            NULL != TIME_INTERFACE_CONFIG(unit,idx).user_cb) {
            u_cb = TIME_INTERFACE_CONFIG(unit,idx).user_cb->heartbeat_cb;
            u_data = TIME_INTERFACE_CONFIG(unit,idx).user_cb->user_data;
            _bcm_esw_time_capture_counter_read(unit, idx, &time);
            if (NULL != u_cb) {
                u_cb(unit, idx, &time, u_data);
            }
        }
    }

    _bcm_esw_time_hw_interrupt_dflt(unit);

    return;
}


/****************************************************************************/
/*                      API functions implmentation                         */
/****************************************************************************/

/*
 * Function:
 *      bcm_esw_time_init
 * Purpose:
 *      Initialize time module 
 * Parameters:
 *      unit - (IN) StrataSwitch Unit #.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 *
 */
int 
bcm_esw_time_init (int unit)
{
    _bcm_time_config_p      time_cfg_ptr;   /* Pointer to Time module config */     
    bcm_time_interface_t    *intf;          /* Time interfaces iterator.     */
    int                     alloc_sz;       /* Memory allocation size.       */
    int                     idx;            /* Time interface array iterator */
    int                     rv;             /* Return Value                  */
    soc_control_t *soc = SOC_CONTROL(unit); /* Soc control structure         */

    
    /* Chek if time feature is supported on a device */
    if (!soc_feature(unit, soc_feature_time_support)) {
        return (BCM_E_UNAVAIL);
    }

    /* If already initialized then deinitialize time module */
    if (TIME_INIT(unit)) {
        _bcm_esw_time_deinit(unit, &TIME_CONFIG(unit));
    }

    /* Allocate time config structure. */
    alloc_sz = sizeof(_bcm_time_config_t);
    time_cfg_ptr = sal_alloc(alloc_sz, "Time module");
    if (NULL == time_cfg_ptr) {
        return (BCM_E_MEMORY);
    }
    sal_memset(time_cfg_ptr, 0, alloc_sz);

    /* Currently only one interface per unit */
    time_cfg_ptr->intf_count = NUM_TIME_INTERFACE(unit); 

    /* Allocate memory for all time interfaces, supported */
    alloc_sz = time_cfg_ptr->intf_count * sizeof(_bcm_time_interface_config_t);
    time_cfg_ptr->intf_arr = sal_alloc(alloc_sz, "Time Interfaces");
    if (NULL == time_cfg_ptr->intf_arr) {
        _bcm_esw_time_deinit(unit, &time_cfg_ptr);
        return (BCM_E_MEMORY);
    }
    sal_memset(time_cfg_ptr->intf_arr, 0, alloc_sz);
    for (idx = 0; idx < time_cfg_ptr->intf_count; idx++) {
        intf = &time_cfg_ptr->intf_arr[idx].time_interface;
        intf->id = idx;
    }

    /* For each time interface allocate memory for tuser_cb */
    alloc_sz = sizeof(_bcm_time_user_cb_t);
    for (idx = 0; idx < NUM_TIME_INTERFACE(unit); idx++) {
        time_cfg_ptr->intf_arr[idx].user_cb = 
            sal_alloc(alloc_sz, "Time Interface User Callback");
        if (NULL == time_cfg_ptr->intf_arr[idx].user_cb) {
            _bcm_esw_time_deinit(unit,  &time_cfg_ptr);
            return (BCM_E_MEMORY);
        }
        sal_memset(time_cfg_ptr->intf_arr[idx].user_cb, 0, alloc_sz);
    }

    /* Interrupt handling function initialization */
    soc->time_call_ref_count = 0;
    soc->soc_time_callout = NULL;

    /* Create protection mutex. */
    time_cfg_ptr->mutex = sal_mutex_create("Time mutex");
    if (NULL == time_cfg_ptr->mutex) {
        _bcm_esw_time_deinit(unit, &time_cfg_ptr);
        return (BCM_E_MEMORY);
    } 

    sal_mutex_take(time_cfg_ptr->mutex, sal_mutex_FOREVER);

    TIME_CONFIG(unit) = time_cfg_ptr;

    /* Clear memories/registers on Cold boot only */
    if (!SOC_WARM_BOOT(unit)) {
    for (idx = 0; idx < NUM_TIME_INTERFACE(unit); idx++) {
        rv  = _bcm_esw_time_hw_clear(unit, idx);
        if (BCM_FAILURE(rv)) {
            TIME_UNLOCK(unit);
            _bcm_esw_time_deinit(unit, &time_cfg_ptr);
            TIME_CONFIG(unit) = NULL;
            return (BCM_E_MEMORY);
        }
    }
    } else {
        /* If Warm boot reinitialize module based on HW state */
#ifdef BCM_WARM_BOOT_SUPPORT
        for (idx = 0; idx < NUM_TIME_INTERFACE(unit); idx++) {
            rv = _bcm_esw_time_reinit(unit, idx);
            if (BCM_FAILURE(rv)) {
                TIME_UNLOCK(unit);
                _bcm_esw_time_deinit(unit, &time_cfg_ptr);
                TIME_CONFIG(unit) = NULL;
                return (rv);
            }
        }

#endif /* BCM_WARM_BOOT_SUPPORT */
    }

    TIME_UNLOCK(unit);

    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_esw_time_deinit
 * Purpose:
 *      Uninitialize time module 
 * Parameters:
 *      unit - (IN) StrataSwitch Unit #.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 *
 */
int 
bcm_esw_time_deinit (int unit)
{
    /* Chek if time feature is supported on a device */
    if (!soc_feature(unit, soc_feature_time_support)) {
        return (BCM_E_UNAVAIL);
    }

    if (0 == TIME_INIT(unit)) {
        return (BCM_E_INIT);
    }

    return _bcm_esw_time_deinit(unit, &TIME_CONFIG(unit));
}

/*
 * Function:
 *      bcm_esw_time_interface_add
 * Purpose:
 *      Adding a time sync interface to a specified unit 
 * Parameters:
 *      unit - (IN) StrataSwitch Unit #.
 *      intf - (IN/OUT) Time Sync Interface
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 *
 */
int 
bcm_esw_time_interface_add (int unit, bcm_time_interface_t *intf)
{
    int             rv;     /* Return value */

    /* Chek if time feature is supported on a device */
    if (!soc_feature(unit, soc_feature_time_support)) {
        return (BCM_E_UNAVAIL);
    }

    BCM_IF_ERROR_RETURN(
        _bcm_esw_time_interface_input_validate(unit, intf));

    if (TIME_CONFIG(unit) == NULL) {
        return (BCM_E_INIT);
    }

    TIME_LOCK(unit);
    if (intf->flags & BCM_TIME_WITH_ID) {
        /* If interface already been in use */
        if (0 != TIME_INTERFACE_CONFIG_REF_COUNT(unit, intf->id)) {
            if (0 == (intf->flags & BCM_TIME_REPLACE)) {
                return BCM_E_EXISTS;
            }
        } else {
            TIME_INTERFACE_CONFIG_REF_COUNT(unit, intf->id) = 1;
        }
    } else {
        rv = _bcm_esw_time_interface_allocate_id(unit, &(intf->id));
        if (BCM_FAILURE(rv)) {
            TIME_UNLOCK(unit);
            return rv;
        }
    }

#if defined(BCM_KATANA_SUPPORT)
    if (SOC_IS_KATANA(unit)) {
        if (intf->flags & BCM_TIME_REF_CLOCK) {
            rv = _bcm_esw_time_interface_ref_clock_install(unit, intf->id);
            if (BCM_FAILURE(rv)) {
                TIME_INTERFACE_CONFIG_REF_COUNT(unit, intf->id) -= 1;
                TIME_UNLOCK(unit);
                return rv;
            }
        }
    }
#endif /*  defined(BCM_KATANA_SUPPORT) */

    /* Set time interface configuration. */
    *(TIME_INTERFACE(unit, intf->id)) = *intf; 

    /* Install the interface into the HW */
    rv = _bcm_esw_time_interface_install(unit, intf->id);
    if (BCM_FAILURE(rv)) {
        TIME_INTERFACE_CONFIG_REF_COUNT(unit, intf->id) -= 1;
        TIME_UNLOCK(unit);
        return rv;
    }

    TIME_UNLOCK(unit);

    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_esw_time_interface_delete
 * Purpose:
 *      Deleting a time sync interface from unit 
 * Parameters:
 *      unit    - (IN) StrataSwitch Unit #.
 *      intf_id - (IN) Time Sync Interface id to remove
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 *
 */
int 
bcm_esw_time_interface_delete (int unit, bcm_time_if_t intf_id)
{
    int rv;   /* Operation return status. */

    /* Chek if time feature is supported on a device */
    if (!soc_feature(unit, soc_feature_time_support)) {
        return (BCM_E_UNAVAIL);
    }

    /* Initialization check. */
    BCM_IF_ERROR_RETURN(
        _bcm_esw_time_interface_id_validate(unit, intf_id));

    TIME_LOCK(unit);

    /* If interface still in use return an Error */
    if (1 < TIME_INTERFACE_CONFIG_REF_COUNT(unit, intf_id)) {
        TIME_UNLOCK(unit);
        return (BCM_E_BUSY);
    }

    /* Free the interface */
    rv = _bcm_time_interface_free(unit, intf_id); 
    if (BCM_FAILURE(rv)) {
        TIME_UNLOCK(unit);
        return (rv);
    }

    rv = _bcm_esw_time_hw_clear(unit, intf_id);

    TIME_UNLOCK(unit);
    return (rv);
}

/*
 * Function:
 *      bcm_esw_time_interface_get
 * Purpose:
 *      Get a time sync interface on a specified unit 
 * Parameters:
 *      unit -  (IN) StrataSwitch Unit #.
 *      intf - (IN/OUT) Time Sync Interface to get
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 *
 */
int 
bcm_esw_time_interface_get (int unit, bcm_time_interface_t *intf)
{
    int rv; 

    /* Chek if time feature is supported on a device */
    if (!soc_feature(unit, soc_feature_time_support)) {
        return (BCM_E_UNAVAIL);
    }

    /* Validation checks */
    if (NULL == intf) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(
        _bcm_esw_time_interface_id_validate(unit, intf->id));

    TIME_LOCK(unit);
    rv = _bcm_esw_time_interface_get(unit, intf->id, intf);
    TIME_UNLOCK(unit);

    return (rv);
}

/*
 * Function:
 *      bcm_esw_time_interface_delete_all
 * Purpose:
 *      Deleting all time sync interfaces on a unit 
 * Parameters:
 *      unit    - (IN) StrataSwitch Unit #.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 *
 */
int 
bcm_esw_time_interface_delete_all (int unit)
{
    bcm_time_if_t   intf;

    /* Chek if time feature is supported on a device */
    if (!soc_feature(unit, soc_feature_time_support)) {
        return (BCM_E_UNAVAIL);
    }

    /* Initialization check */
    if (0 == TIME_INIT(unit)) {
        return BCM_E_INIT;
    }

    for (intf = 0; intf < NUM_TIME_INTERFACE(unit); intf++ ) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_time_interface_delete(unit, intf));
    }

    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_esw_time_interface_traverse
 * Purpose:
 *      Itterates over all time sync interfaces and calls given callback 
 * Parameters:
 *      unit        - (IN) StrataSwitch Unit #.
 *      cb          - (IN) Call back function
 *      user_data   - (IN) void pointer to store any user information
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 *
 */
int 
bcm_esw_time_interface_traverse (int unit, bcm_time_interface_traverse_cb cb, 
                                 void *user_data)
{
    bcm_time_if_t   intf;
    
    /* Chek if time feature is supported on a device */
    if (!soc_feature(unit, soc_feature_time_support)) {
        return (BCM_E_UNAVAIL);
    }

    /* Initialization check. */
    if (0 == TIME_INIT(unit)) {
        return BCM_E_INIT;
    }
    
    if (NULL == cb) {
        return (BCM_E_PARAM);
    }

    for (intf = 0; intf < NUM_TIME_INTERFACE(unit); intf++ ) {
        if (NULL != TIME_INTERFACE(unit, intf)) {
            BCM_IF_ERROR_RETURN(cb(unit, TIME_INTERFACE(unit, intf), user_data));
        }
    }
    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_esw_time_capture_get
 * Purpose:
 *      Gets a time captured by HW clock
 * Parameters:
 *      unit    - (IN) StrataSwitch Unit #.
 *      id      - (IN) Time interface identifier 
 *      time    - (OUT) Structure to contain HW clocks values
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 *
 */
int 
bcm_esw_time_capture_get (int unit, bcm_time_if_t id, bcm_time_capture_t *time)
{

    int rv;   /* Operation return status. */

    /* Chek if time feature is supported on a device */
    if (!soc_feature(unit, soc_feature_time_support)) {
        return (BCM_E_UNAVAIL);
    }

    /* Initialization check. */
    BCM_IF_ERROR_RETURN(
        _bcm_esw_time_interface_id_validate(unit, id));

    if (NULL == time) {
        return (BCM_E_PARAM);
    }
    if (NULL == TIME_CAPTURE(unit, id)) {
        return (BCM_E_NOT_FOUND);
    }

    TIME_LOCK(unit);
    rv = _bcm_esw_time_capture_get(unit, id, TIME_CAPTURE(unit, id));
    if (BCM_FAILURE(rv)) {
        TIME_UNLOCK(unit);
        return (rv);
    }

    *time = *(TIME_CAPTURE(unit, id));

    TIME_UNLOCK(unit);

    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_esw_time_heartbeat_enable_set
 * Purpose:
 *      Enables/Disables interrupt handling for each heartbeat provided by a 
 *      HW clock
 * Parameters:
 *      unit    - (IN) StrataSwitch Unit #.
 *      id      - (IN) Time Sync Interface Id
 *      enable  - (IN) Enable/Disable parameter
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 *
 */
int 
bcm_esw_time_heartbeat_enable_set (int unit, bcm_time_if_t id, int enable)
{
    uint32          regval;
    soc_control_t	*soc = SOC_CONTROL(unit);

    /* Chek if time feature is supported on a device */
    if (!soc_feature(unit, soc_feature_time_support)) {
        return (BCM_E_UNAVAIL);
    }

    /* Param validation check. */
    BCM_IF_ERROR_RETURN(
        _bcm_esw_time_interface_id_validate(unit, id));

    TIME_LOCK(unit);

#if (defined(BCM_KATANA_SUPPORT))
    if (SOC_IS_KATANA(unit)) {
        READ_CMIC_TS_TIME_CAPTURE_CTRLr(unit, &regval);
        /* Configure the HW to give interrupt on every heartbeat */
        soc_reg_field_set(unit, CMIC_TS_TIME_CAPTURE_CTRLr, &regval, INT_ENABLEf, 
                          (enable) ? 1: 0);
        /* Configure the HW to capture time on every heartbeat */
        soc_reg_field_set(unit, CMIC_TS_TIME_CAPTURE_CTRLr, &regval, TIME_CAPTURE_MODEf,
                          (enable) ? TIME_CAPTURE_MODE_HEARTBEAT : 
                          TIME_CAPTURE_MODE_DISABLE);
        WRITE_CMIC_TS_TIME_CAPTURE_CTRLr(unit, regval);
    } else
#endif /* defined(BCM_KATANA_SUPPORT) */
    {
        READ_CMIC_BS_CAPTURE_CTRLr(unit, &regval);
        /* Configure the HW to give interrupt on every heartbeat */
        soc_reg_field_set(unit, CMIC_BS_CAPTURE_CTRLr, &regval, INT_ENf, 
                          (enable) ? 1: 0);
        /* Configure the HW to capture time on every heartbeat */
        soc_reg_field_set(unit, CMIC_BS_CAPTURE_CTRLr, &regval, TIME_CAPTURE_MODEf,
                          (enable) ? TIME_CAPTURE_MODE_HEARTBEAT : 
                          TIME_CAPTURE_MODE_DISABLE);
        WRITE_CMIC_BS_CAPTURE_CTRLr(unit, regval);
    }

    TIME_UNLOCK(unit);

    /* Enable/disable broadsync interrupt */
    if (enable) {
        soc_intr_enable(unit, IRQ_BROADSYNC_INTR);
        if (!soc->time_call_ref_count) {
            soc->soc_time_callout = _bcm_esw_time_hw_interrupt_dflt;
        }
        
    } else {
        soc_intr_disable(unit, IRQ_BROADSYNC_INTR);
        if (!soc->time_call_ref_count) {
            soc->soc_time_callout = NULL;
        }
    }
    
    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_esw_time_heartbeat_enable_get
 * Purpose:
 *      Gets interrupt handling status for each heartbeat provided by a 
 *      HW clock
 * Parameters:
 *      unit    - (IN) StrataSwitch Unit #.
 *      id      - (IN) Time Sync Interface Id
 *      enable  - (OUT) Enable status of interrupt handling
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 *
 */
int 
bcm_esw_time_heartbeat_enable_get (int unit, bcm_time_if_t id, int *enable)
{
    uint32      regval;

    /* Chek if time feature is supported on a device */
    if (!soc_feature(unit, soc_feature_time_support)) {
        return (BCM_E_UNAVAIL);
    }

    /* Param validation check. */
    BCM_IF_ERROR_RETURN(
        _bcm_esw_time_interface_id_validate(unit, id));

    /* Read HW Configuration */

#if (defined(BCM_KATANA_SUPPORT))
    if (SOC_IS_KATANA(unit)) {
        READ_CMIC_TS_TIME_CAPTURE_CTRLr(unit, &regval);
        *enable = soc_reg_field_get(unit, CMIC_TS_TIME_CAPTURE_CTRLr,
                                    regval, INT_ENABLEf); 
    } else
#endif /* defined(BCM_KATANA_SUPPORT) */
    {
        READ_CMIC_BS_CAPTURE_CTRLr(unit, &regval);
        *enable = soc_reg_field_get(unit, CMIC_BS_CAPTURE_CTRLr, regval, INT_ENf); 
    }

    return (BCM_E_NONE);
}


#if (defined(BCM_KATANA_SUPPORT))
/*
 * Function:
 *      _bcm_esw_time_trigger_to_timestamp_mode
 * Purpose: 
 *      Convert capture mode flags to time control capture modes
 *
 * Parameters: 
 *      unit       - (IN) mode_flags
 *      
 * Returns:
 *      Timesync captude modes 
 * Notes:
 */
STATIC uint32
_bcm_esw_time_trigger_to_timestamp_mode (int unit, uint32 mode_flags)
{
    uint32 capture_mode = 0;
    uint32 regval = 0;
    int    bit_cnt;

    /* processing on user mode flags reduces the dependencies on 
     * hardware bit maps, byte shift of mode flags
     * should have been efficient 
     */
    for (bit_cnt = 0; bit_cnt < sizeof(uint32); bit_cnt++)
    {
        switch (mode_flags & (1 << bit_cnt))
        {
            case  BCM_TIME_CAPTURE_GPIO_0:
                    capture_mode |=  TIME_CAPTURE_MODE_GPIO_0;
                    READ_CMIC_TS_GPIO_1_DOWN_EVENT_CTRLr(unit, &regval);
                    soc_reg_field_set(unit, CMIC_TS_GPIO_1_DOWN_EVENT_CTRLr, 
                                      &regval, ENABLEf, 1);
                    WRITE_CMIC_TS_GPIO_1_DOWN_EVENT_CTRLr(unit, regval);
                    READ_CMIC_TS_GPIO_1_UP_EVENT_CTRLr(unit, &regval);
                    soc_reg_field_set(unit, CMIC_TS_GPIO_1_UP_EVENT_CTRLr,
                                      &regval, ENABLEf, 1);
                    WRITE_CMIC_TS_GPIO_1_UP_EVENT_CTRLr(unit, regval);
                    break;

            case  BCM_TIME_CAPTURE_GPIO_1:
                    capture_mode |=  TIME_CAPTURE_MODE_GPIO_1;
                    READ_CMIC_TS_GPIO_1_DOWN_EVENT_CTRLr(unit, &regval);
                    soc_reg_field_set(unit, CMIC_TS_GPIO_1_DOWN_EVENT_CTRLr,
                                      &regval, ENABLEf, 1);
                    WRITE_CMIC_TS_GPIO_1_DOWN_EVENT_CTRLr(unit, regval);
                    READ_CMIC_TS_GPIO_1_UP_EVENT_CTRLr(unit, &regval);
                    soc_reg_field_set(unit, CMIC_TS_GPIO_1_UP_EVENT_CTRLr,
                                      &regval, ENABLEf, 1);
                    WRITE_CMIC_TS_GPIO_1_UP_EVENT_CTRLr(unit, regval);
                    break;

            case  BCM_TIME_CAPTURE_GPIO_2:
                    capture_mode |=  TIME_CAPTURE_MODE_GPIO_2;
                    READ_CMIC_TS_GPIO_2_DOWN_EVENT_CTRLr(unit, &regval);
                    soc_reg_field_set(unit, CMIC_TS_GPIO_2_DOWN_EVENT_CTRLr,
                                      &regval, ENABLEf, 1);
                    WRITE_CMIC_TS_GPIO_2_DOWN_EVENT_CTRLr(unit, regval);
                    READ_CMIC_TS_GPIO_2_UP_EVENT_CTRLr(unit, &regval);
                    soc_reg_field_set(unit, CMIC_TS_GPIO_2_UP_EVENT_CTRLr,
                                      &regval, ENABLEf, 1);
                    WRITE_CMIC_TS_GPIO_2_UP_EVENT_CTRLr(unit, regval);
                    break;

            case  BCM_TIME_CAPTURE_GPIO_3:
                    capture_mode |=  TIME_CAPTURE_MODE_GPIO_3;
                    READ_CMIC_TS_GPIO_3_DOWN_EVENT_CTRLr(unit, &regval);
                    soc_reg_field_set(unit, CMIC_TS_GPIO_3_DOWN_EVENT_CTRLr,
                                      &regval, ENABLEf, 1);
                    WRITE_CMIC_TS_GPIO_3_DOWN_EVENT_CTRLr(unit, regval);
                    READ_CMIC_TS_GPIO_3_UP_EVENT_CTRLr(unit, &regval);
                    soc_reg_field_set(unit, CMIC_TS_GPIO_3_UP_EVENT_CTRLr,
                                      &regval, ENABLEf, 1);
                    WRITE_CMIC_TS_GPIO_3_UP_EVENT_CTRLr(unit, regval);
                    break;

            case  BCM_TIME_CAPTURE_GPIO_4:
                    capture_mode |=  TIME_CAPTURE_MODE_GPIO_4;
                    READ_CMIC_TS_GPIO_4_DOWN_EVENT_CTRLr(unit, &regval);
                    soc_reg_field_set(unit, CMIC_TS_GPIO_4_DOWN_EVENT_CTRLr,
                                      &regval, ENABLEf, 1);
                    WRITE_CMIC_TS_GPIO_4_DOWN_EVENT_CTRLr(unit, regval);
                    READ_CMIC_TS_GPIO_4_UP_EVENT_CTRLr(unit, &regval);
                    soc_reg_field_set(unit, CMIC_TS_GPIO_4_UP_EVENT_CTRLr,
                                      &regval, ENABLEf, 1);
                    WRITE_CMIC_TS_GPIO_4_UP_EVENT_CTRLr(unit, regval);
                    break;

            case  BCM_TIME_CAPTURE_GPIO_5:
                    capture_mode |=  TIME_CAPTURE_MODE_GPIO_5;
                    READ_CMIC_TS_GPIO_5_DOWN_EVENT_CTRLr(unit, &regval);
                    soc_reg_field_set(unit, CMIC_TS_GPIO_5_DOWN_EVENT_CTRLr,
                                      &regval, ENABLEf, 1);
                    WRITE_CMIC_TS_GPIO_5_DOWN_EVENT_CTRLr(unit, regval);
                    READ_CMIC_TS_GPIO_5_UP_EVENT_CTRLr(unit, &regval);
                    soc_reg_field_set(unit, CMIC_TS_GPIO_5_UP_EVENT_CTRLr,
                                      &regval, ENABLEf, 1);
                    WRITE_CMIC_TS_GPIO_5_UP_EVENT_CTRLr(unit, regval);
                    break;

            case BCM_TIME_CAPTURE_L1_CLOCK_PRIMARY:
                    capture_mode |=  TIME_CAPTURE_MODE_L1_CLOCK_PRIMARY;
                    READ_CMIC_TS_L1_CLK_RECOVERED_CLK_COUNT_CTRLr(unit, &regval);
                    soc_reg_field_set(unit, CMIC_TS_L1_CLK_RECOVERED_CLK_COUNT_CTRLr, &regval, 
                                      ENABLEf, 1);
                    WRITE_CMIC_TS_L1_CLK_RECOVERED_CLK_COUNT_CTRLr(unit, regval);
                    break;

            case BCM_TIME_CAPTURE_L1_CLOCK_SECONDARY:
                    capture_mode |=  TIME_CAPTURE_MODE_L1_CLOCK_SECONDARY;
                    READ_CMIC_TS_L1_CLK_RECOVERED_CLK_BKUP_COUNT_CTRLr(unit, &regval);
                    soc_reg_field_set(unit, CMIC_TS_L1_CLK_RECOVERED_CLK_BKUP_COUNT_CTRLr, &regval, 
                                      ENABLEf, 1);
                    WRITE_CMIC_TS_L1_CLK_RECOVERED_CLK_BKUP_COUNT_CTRLr(unit, regval);
                    break;

            case BCM_TIME_CAPTURE_LCPLL:
                    capture_mode |=  TIME_CAPTURE_MODE_LCPLL;
                    READ_CMIC_TS_LCPLL_CLK_COUNT_CTRLr(unit, &regval);
                    soc_reg_field_set(unit, CMIC_TS_LCPLL_CLK_COUNT_CTRLr, 
                                        &regval, ENABLEf, 1);
                    WRITE_CMIC_TS_LCPLL_CLK_COUNT_CTRLr(unit, regval);
                    break;

            case BCM_TIME_CAPTURE_IP_DM_0:
                    capture_mode |=  TIME_CAPTURE_MODE_IP_DM_0;
                    break;

            case BCM_TIME_CAPTURE_IP_DM_1:
                    capture_mode |= TIME_CAPTURE_MODE_IP_DM_1;
                    break;

            default:
                    break;
        }
    }
    return capture_mode;
}

/*
 * Function:
 *      _bcm_esw_time_trigger_from_timestamp_mode
 * Purpose: 
 *      Convert Timesync capture modes to mode flags 
 *
 * Parameters: 
 *      unit       - (IN) capture_mode
 *      
 * Returns:
 *      Capture mode flags
 * Notes:
 */

STATIC uint32
_bcm_esw_time_trigger_from_timestamp_mode (uint32 capture_mode)
{
    uint32 user_flags = 0;
    int    bit_cnt;

    /* processing on user mode flags reduces the dependencies on 
     * hardware bit maps, byte shift of mode flags
     * should have been efficient 
     */
    for (bit_cnt = 0; bit_cnt < sizeof(uint32); bit_cnt++)
    {
        switch (capture_mode & (1 << bit_cnt))
        {
            case  TIME_CAPTURE_MODE_GPIO_0:
                    user_flags |=  BCM_TIME_CAPTURE_GPIO_0;
                    break;
            case  TIME_CAPTURE_MODE_GPIO_1:
                    user_flags |=  BCM_TIME_CAPTURE_GPIO_1;
                    break;
            case  TIME_CAPTURE_MODE_GPIO_2:
                    user_flags |=  BCM_TIME_CAPTURE_GPIO_2;
                    break;
            case  TIME_CAPTURE_MODE_GPIO_3:
                    user_flags |=  BCM_TIME_CAPTURE_GPIO_3;
                    break;
            case  TIME_CAPTURE_MODE_GPIO_4:
                    user_flags |=  BCM_TIME_CAPTURE_GPIO_4;
                    break;
            case  TIME_CAPTURE_MODE_GPIO_5:
                    user_flags |=  BCM_TIME_CAPTURE_GPIO_5;
                    break;
            case TIME_CAPTURE_MODE_L1_CLOCK_PRIMARY:
                    user_flags |=  BCM_TIME_CAPTURE_L1_CLOCK_PRIMARY;
                    break;
            case TIME_CAPTURE_MODE_L1_CLOCK_SECONDARY:
                    user_flags |=  BCM_TIME_CAPTURE_L1_CLOCK_SECONDARY;
                    break;
            case TIME_CAPTURE_MODE_LCPLL:
                    user_flags |=  BCM_TIME_CAPTURE_LCPLL;
                    break;
            case TIME_CAPTURE_MODE_IP_DM_0:
                    user_flags |=  BCM_TIME_CAPTURE_IP_DM_0;
                    break;
            case TIME_CAPTURE_MODE_IP_DM_1:
                    user_flags |= BCM_TIME_CAPTURE_IP_DM_1;
                    break;
            default:
                    break;
        }
    }
    return user_flags;
}

#endif /* (defined(BCM_KATANA_SUPPORT) */

/*
 * Function:
 *      bcm_esw_time_trigger_enable_set
 * Purpose:
 *      Enables/Disables interrupt handling for external triggers
 * Parameters:
 *      unit        - (IN) StrataSwitch Unit #.
 *      id          - (IN) Time Sync Interface Id
 *      mode_flags  - (IN) Enable/Disable parameter
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 *
 */
int 
bcm_esw_time_trigger_enable_set (int unit, bcm_time_if_t id, uint32 mode_flags)
{

#if (defined(BCM_KATANA_SUPPORT))
    uint32          regval = 0, capture_mode = 0;
    soc_control_t	*soc = SOC_CONTROL(unit);
#endif

    /* Chek if time feature is supported on a device */
    if (!(soc_feature(unit, soc_feature_time_support) && 
         SOC_IS_KATANA(unit))) {
        return (BCM_E_UNAVAIL);
    }

    /* Param validation check. */
    BCM_IF_ERROR_RETURN(
        _bcm_esw_time_interface_id_validate(unit, id));


#if (defined(BCM_KATANA_SUPPORT))
    /* 
     *   Read time capture control capture modes
     *   Convert to hardware cature mode bitmap
     *   Enable interrupt for all capture mode bitmap.
     *   Write time capture mode
     *
     *   if for ALL capture mode, ignore individual capture modes.
     *   Enable Interrupts.
     */

    TIME_LOCK(unit);

    READ_CMIC_TS_TIME_CAPTURE_CTRLr(unit, &regval);

    if (mode_flags & BCM_TIME_CAPTURE_ALL) {
        /* Enable all Time capture mechanisms */
        soc_reg_field_set(unit, CMIC_TS_TIME_CAPTURE_CTRLr, &regval, 
                            TIME_CAPTURE_ENABLEf, 1 );
        
        /* Return and  ignore setting of individual capture modes */
        return BCM_E_NONE;
    }

    /* Set time capture modes */
    capture_mode = _bcm_esw_time_trigger_to_timestamp_mode (unit, mode_flags);

    /* Configure the HW with capture modes */
    soc_reg_field_set(unit, CMIC_TS_TIME_CAPTURE_CTRLr, &regval, 
                    TIME_CAPTURE_MODEf, (capture_mode) ? capture_mode : 
                    TIME_CAPTURE_MODE_DISABLE);

    /* Configure the HW to trigger interrupt for capture events */
    soc_reg_field_set(unit, CMIC_TS_TIME_CAPTURE_CTRLr, &regval, INT_ENABLEf, 
                      (capture_mode) ? capture_mode : 0);
    
    WRITE_CMIC_TS_TIME_CAPTURE_CTRLr(unit, regval);

    TIME_UNLOCK(unit);

    /* Enable/disable broadsync interrupt */
    if (capture_mode) {
        /* Enable Interrupt */
        soc_intr_enable(unit, IRQ_BROADSYNC_INTR);
        if (!soc->time_call_ref_count) {
            soc->soc_time_callout = _bcm_esw_time_hw_interrupt_dflt;
        }
        
    } else {
        /* Check on the references and disable interrupt and handler */
        soc_intr_disable(unit, IRQ_BROADSYNC_INTR);
        if (!soc->time_call_ref_count) {
            soc->soc_time_callout = NULL;
        }
    }
#endif /* defined(BCM_KATANA_SUPPORT) */
    
    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_esw_time_trigger_enable_get
 * Purpose:
 *      Gets interrupt handling status for each heartbeat provided by a 
 *      HW clock
 * Parameters:
 *      unit    - (IN) StrataSwitch Unit #.
 *      id      - (IN) Time Sync Interface Id
 *      enable  - (OUT) Enable status of interrupt handling
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 *
 */
int 
bcm_esw_time_trigger_enable_get (int unit, bcm_time_if_t id, uint32 *mode_flags)
{
#if defined(BCM_KATANA_SUPPORT)
    uint32      regval, capture_mode = 0;
#endif

    /* Chek if time feature is supported on a device */
    if (!(soc_feature(unit, soc_feature_time_support) && 
          SOC_IS_KATANA(unit)))  {
        return (BCM_E_UNAVAIL);
    }

    /* Param validation check. */
    BCM_IF_ERROR_RETURN(
        _bcm_esw_time_interface_id_validate(unit, id));

#if defined(BCM_KATANA_SUPPORT)
    /* Read HW Configuration */
    READ_CMIC_TS_TIME_CAPTURE_CTRLr(unit, &regval);

    /* Get interrupt enabled capture modes */
     capture_mode = soc_reg_field_get(unit, CMIC_TS_TIME_CAPTURE_CTRLr, regval, INT_ENABLEf); 

    *mode_flags = _bcm_esw_time_trigger_from_timestamp_mode (capture_mode);
#endif /* defined(BCM_KATANA_SUPPORT) */
    
    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_esw_time_heartbeat_register
 * Purpose:
 *      Registers a call back function to be called on each heartbeat
 * Parameters:
 *      unit        - (IN) StrataSwitch Unit #.
 *      id          - (IN) Time Sync Interface Id
 *      f           - (IN) Function to register
 *      user_data   - (IN) void pointer to store any user information
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 *
 */
int 
bcm_esw_time_heartbeat_register (int unit, bcm_time_if_t id, bcm_time_heartbeat_cb f,
                                 void *user_data)
{
    soc_control_t	*soc = SOC_CONTROL(unit);

    /* Chek if time feature is supported on a device */
    if (!soc_feature(unit, soc_feature_time_support)) {
        return (BCM_E_UNAVAIL);
    }

    /* Param validation check. */
    BCM_IF_ERROR_RETURN(
        _bcm_esw_time_interface_id_validate(unit, id));
    if (NULL == f) {
        return (BCM_E_PARAM);
    }

    TIME_LOCK(unit);
    /* If HW interrupt handler been registered protect from race condition */
    if (soc->time_call_ref_count) {
        soc->soc_time_callout = _bcm_esw_time_hw_interrupt_dflt;
    }

    /* Register user call back */
    TIME_INTERFACE_CONFIG(unit, id).user_cb->heartbeat_cb = f;
    TIME_INTERFACE_CONFIG(unit, id).user_cb->user_data = user_data;

    /* After registering user cb function and user_data turn on 
    HW interrupt handler */
    soc->soc_time_callout = _bcm_esw_time_hw_interrupt;
    /* Only single call back at a time is currently supported */
    soc->time_call_ref_count = 1; 

    TIME_UNLOCK(unit);
    
    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_esw_time_heartbeat_unregister
 * Purpose:
 *      Unregisters a call back function to be called on each heartbeat
 * Parameters:
 *      unit        - (IN) StrataSwitch Unit #.
 *      id          - (IN) Time Sync Interface Id
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 *
 */
int 
bcm_esw_time_heartbeat_unregister (int unit, bcm_time_if_t id)
{
    soc_control_t	*soc = SOC_CONTROL(unit);

    /* Chek if time feature is supported on a device */
    if (!soc_feature(unit, soc_feature_time_support)) {
        return (BCM_E_UNAVAIL);
    }

    /* Param validation check. */
    BCM_IF_ERROR_RETURN(
        _bcm_esw_time_interface_id_validate(unit, id));

    TIME_LOCK(unit);

    if (--soc->time_call_ref_count <= 0) {
        soc->time_call_ref_count = 0;
        soc->soc_time_callout = _bcm_esw_time_hw_interrupt_dflt;
    }
    TIME_INTERFACE_CONFIG(unit, id).user_cb->heartbeat_cb = NULL;
    TIME_INTERFACE_CONFIG(unit, id).user_cb->user_data = NULL;

    TIME_UNLOCK(unit);
    
    return (BCM_E_NONE);
}


#ifdef BCM_WARM_BOOT_SUPPORT_SW_DUMP
/*
 * Function:
 *     _bcm_time_sw_dump
 * Purpose:
 *     Displays time information maintained by software.
 * Parameters:
 *     unit - Device unit number
 * Returns:
 *     None
 */
void
_bcm_time_sw_dump(int unit)
{
    soc_control_t	*soc = SOC_CONTROL(unit);

    soc_cm_print("\nSW Information TIME - Unit %d\n", unit);
    soc_cm_print(" Time call reference counter = %d\n", soc->time_call_ref_count);
    if (NULL != soc->soc_time_callout) {
        soc_cm_print(" Time interrupt handler address is = %p\n",
                     (void *)(unsigned long)soc->soc_time_callout);
    } else {
        soc_cm_print(" Time interrupt handler is NULL \n");
    }
}
#endif /* BCM_WARM_BOOT_SUPPORT_SW_DUMP */
