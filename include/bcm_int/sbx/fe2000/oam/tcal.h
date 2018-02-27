/*
 * $Id: tcal.h 1.4 Broadcom SDK $
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

#ifndef _BCM_INT_SBX_FE2000_TCAL_H_
#define _BCM_INT_SBX_FE2000_TCAL_H_

#ifdef BCM_FE2000_P3_SUPPORT
#include <soc/sbx/g2p3/g2p3.h>
#endif /* def BCM_FE2000_P3_SUPPORT */

#define _BCM_TCAL_ID_MASK         0xFFFF
#define _BCM_TCAL_INVALID_ID      0xFFFF
#define _BCM_TCAL_ID_VALID(x)    \
       (((x) & _BCM_TCAL_ID_MASK) != _BCM_TCAL_INVALID_ID)


typedef uint32_t tcal_id_t;

/*
 *   Function
 *      _bcm_tcal_init
 *   Purpose
 *      initialize the timer calendar manager
 *   Parameters
 *      (in)  unit          - BCM device number
 *   Returns
 *       BCM_E_*
 */
int _bcm_tcal_init(int unit);

/*
 *   Function
 *      _bcm_tcal_cleanup
 *   Purpose
 *      free any timer calendar resources
 *   Parameters
 *      (in)  unit          - BCM device number
 *   Returns
 *       BCM_E_*
 */
int _bcm_tcal_cleanup(int unit);

/*
 *   Function
 *      _bcm_tcal_update
 *   Purpose
 *      Allocate a BCM8802x timer calendar entry
 *   Parameters
 *      (in)  unit          - BCM8802x BCM device number
 *      (in)  period        - period in ms, for which a timer should expire
 *      (in)  tx_enable     - enable the timer to intiate transmit
 *      (in)  ep_rec_index  - endpoint record index to transmit
 *      (out) tcal_id       - timer calenedar entry to update
 *   Returns
 *       BCM_E_*
 */
int _bcm_tcal_alloc(int unit, int period, int tx_enable,
                    uint32_t ep_rec_index, tcal_id_t *tcal_id);

/*
 *   Function
 *      _bcm_tcal_update
 *   Purpose
 *      Update an existing BCM8802x timer calendar entry, if existing entry
 *      is implemented as multiple entries, they will be freed as required.
 *   Parameters
 *      (in)  unit          - BCM8802x BCM device number
 *      (in)  period        - period in ms, for which a timer should expire
 *      (in)  tx_enable     - enable the timer to intiate transmit
 *      (in)  ep_rec_index  - endpoint record index to transmit
 *      (inout) tcal_id     - timer calenedar entry to update
 *   Returns
 *       BCM_E_*
 */
int _bcm_tcal_update(int unit, int period, int tx_enable, 
                     uint32_t ep_rec_index, tcal_id_t *tcal_id);


/*
 *   Function
 *      _bcm_tcal_free
 *   Purpose
 *      Release the resources associated with the given tcalid
 *   Parameters
 *      (in)  unit          - BCM device number
 *      (in/out)  tcal_id   - tcal_id to free
 *   Returns
 *       BCM_E_*
 */
int _bcm_tcal_free(int unit, tcal_id_t *tcal_id);

/*
 *   Function
 *      _bcm_tcal_interval_decode
 *   Purpose
 *      Convert from interval enumeration to interval in ms.
 *   Parameters
 *      (in)  unit          - BCM device number
 *      (in)  encoded       - Interval enumeration
 *      (out) interval_ms   - Interval in ms
 *   Returns
 *       BCM_E_*
 */
int _bcm_tcal_interval_decode(int unit, uint32_t encoded, int *interval_ms);

/*
 *   Function
 *      _bcm_tcal_interval_encode
 *   Purpose
 *      Convert from interval in ms to an interval enumeration.
 *   Parameters
 *      (in)  unit          - BCM device number
 *      (in)  interval_ms   - Interval in ms
 *      (out) encoded       - Interval enumeration
 *   Returns
 *       BCM_E_*
 */
int _bcm_tcal_interval_encode(int unit, int interval_ms, uint32_t *encoded);

/*
 *   Function
 *      _bcm_tcal_period_get
 *   Purpose
 *      obtains period associated with Tcal
 *   Parameters
 *      (in)  unit          - BCM device number
 *      (in)  tcal_id       - tcal_id
 *      (in/out) period     - timer period
 *      (in/out) tx_enable  - >0 if timer enabled
 *   Returns
 *       BCM_E_*
 */
int
_bcm_tcal_period_get(int unit, tcal_id_t tcal_id, int *period, int *tx_enable);

#endif  /* _BCM_INT_SBX_FE2000_TCAL_H_  */
