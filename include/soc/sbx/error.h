/*
 * $Id: error.h 1.1 Broadcom SDK $
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
 * Module: Error translation
 */

#ifndef _SOC_SBX_ERROR_H_
#define _SOC_SBX_ERROR_H_

#include <bcm/debug.h>
#include <bcm/error.h>
#include <soc/sbx/sbx_drv.h>

#include <soc/sbx/sbStatus.h>
#include <soc/sbx/sbElibStatus.h>


#define BCM_SBX_IF_ERROR_RETURN(op)                                     \
    do                            {                                     \
        int __rv__;                                                     \
        if ((__rv__ = translate_sbx_result((op))) < 0)                  \
            return (__rv__);                                            \
    } while (0)


#define BCM_SBX_ELIB_IF_ERROR_RETURN(op)                                \
    do                            {                                     \
        int __rv__;                                                     \
        if ((__rv__ = translate_sbx_elib_result((op))) < 0)             \
            return (__rv__);                                            \
    } while (0)

/* A common debug message prefix */
#define _SBX_D(_u, string)   "[%d:%s]: " string, _u, FUNCTION_NAME()

/* A common unknown microcode warning message */
#define SBX_UNKNOWN_UCODE_WARN(_u)  \
    BCM_DEBUG(BCM_DBG_WARN, (_SBX_D(_u, "uCode type %d is not supported\n"), \
                                 SOC_SBX_CONTROL(_u)->ucodetype));


/*
 *   Function
 *      translate_sbx_result
 *   Purpose
 *      Translate a Sandburst result code into a Broadcom result code
 *   Parameters
 *      (in) sbStatus_t result = Sandburst result to translate
 *   Returns
 *      bcm_error_t = Closest reasonable Broadcom result
 *   Notes
 *      If there is no reasonably close error, it just guesses and returns
 *      BCM_E_FAIL instead of something more specific.  It is possible that
 *      this result is wrong if there's a 'pending' or similar result that is
 *      not included in the switch statement in this function.  Therefore,
 *      this must be updated whenever BCM_E_* is expanded or SB_* results are
 *      expanded, in order to ensure consistency to the intended behaviour.
 */
int
translate_sbx_result(const sbStatus_t result);

/*
 *   Function
 *      translate_sbx_elib_result
 *   Purpose
 *      Translate a Sandburst ELib result code into a Broadcom result code
 *   Parameters
 *      (in) sbElibStatus_et result = Sandburst ELib result to translate
 *   Returns
 *      bcm_error_t = Closest reasonable Broadcom result
 *   Notes
 *      If there is no reasonably close error, it just guesses and returns
 *      BCM_E_FAIL instead of something more specific.  It is possible that
 *      this result is wrong if there's a 'pending' or similar result that is
 *      not included in the switch statement in this function.  Therefore,
 *      this must be updated whenever BCM_E_* is expanded or SB_* results are
 *      expanded, in order to ensure consistency to the intended behaviour.
 */
int
translate_sbx_elib_result(const sbElibStatus_et result);

/*
 *   Function
 *      sbx_result_string
 *   Purpose
 *      Get the text representation of a Sandburst result code.
 *   Parameters
 *      (in) sbStatus_t sbxResult = Sandburst result to translate
 *   Returns
 *      const char * = Text String explaining the error.
 *   Notes
 *      Straight ripoff of the (unavailable) sbGetFeIlibStatusString()
 */
const char *
sbx_result_string(sbStatus_t status);

#endif /* _SOC_SBX_ERROR_H_ */
