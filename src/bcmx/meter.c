/*
 * $Id: meter.c 1.11 Broadcom SDK $
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
 * File:    bcmx/meter.c
 * Purpose: BCMX Metering APIs
 */

#include <bcm/types.h>

#include <bcmx/meter.h>
#include <bcmx/lport.h>
#include <bcmx/bcmx.h>
#include <bcmx/lplist.h>

#include "bcmx_int.h"

#define BCMX_METER_INIT_CHECK    BCMX_READY_CHECK

#define BCMX_METER_ERROR_CHECK(_unit, _check, _rv)    \
    BCMX_ERROR_CHECK(_unit, _check, _rv)

#define BCMX_METER_DELETE_ERROR_CHECK(_unit, _check, _rv)    \
    BCMX_DELETE_ERROR_CHECK(_unit, _check, _rv)


/*
 * Function:
 *      bcmx_meter_init
 */

int
bcmx_meter_init(void)
{
    int rv = BCM_E_UNAVAIL, tmp_rv;
    int i, bcm_unit;

    BCMX_METER_INIT_CHECK;

    BCMX_UNIT_ITER(bcm_unit, i) {
        tmp_rv = bcm_meter_init(bcm_unit);
        BCM_IF_ERROR_RETURN(BCMX_METER_ERROR_CHECK(bcm_unit, tmp_rv, &rv));
    }

    return rv;
}


/*
 * Function:
 *      bcmx_meter_create
 */

int
bcmx_meter_create(bcmx_lport_t lport, int *mid)
{
    int         bcm_unit;
    bcm_port_t  bcm_port;

    BCMX_METER_INIT_CHECK;

    BCMX_PARAM_NULL_CHECK(mid);

    BCM_IF_ERROR_RETURN
        (_bcmx_dest_to_unit_port(lport, &bcm_unit, &bcm_port,
                                 BCMX_DEST_CONVERT_DEFAULT));

    return bcm_meter_create(bcm_unit, bcm_port, mid);
}


/*
 * Function:
 *      bcmx_meter_delete
 */

int
bcmx_meter_delete(bcmx_lport_t lport, int mid)
{
    int         bcm_unit;
    bcm_port_t  bcm_port;

    BCMX_METER_INIT_CHECK;

    BCM_IF_ERROR_RETURN
        (_bcmx_dest_to_unit_port(lport, &bcm_unit, &bcm_port,
                                 BCMX_DEST_CONVERT_DEFAULT));

    return bcm_meter_delete(bcm_unit, bcm_port, mid);
}


/*
 * Function:
 *      bcmx_meter_delete_all
 */

int
bcmx_meter_delete_all(void)
{
    int rv = BCM_E_UNAVAIL, tmp_rv;
    int i, bcm_unit;

    BCMX_METER_INIT_CHECK;

    BCMX_UNIT_ITER(bcm_unit, i) {
        tmp_rv = bcm_meter_delete_all(bcm_unit);
        BCM_IF_ERROR_RETURN
            (BCMX_METER_DELETE_ERROR_CHECK(bcm_unit, tmp_rv, &rv));
    }

    return rv;
}


/*
 * Function:
 *      bcmx_meter_get
 */

int
bcmx_meter_get(bcmx_lport_t lport,
               int mid,
               uint32 *kbits_sec,
               uint32 *kbits_burst)
{
    int         bcm_unit;
    bcm_port_t  bcm_port;

    BCMX_METER_INIT_CHECK;

    BCMX_PARAM_NULL_CHECK(kbits_sec);
    BCMX_PARAM_NULL_CHECK(kbits_burst);

    BCM_IF_ERROR_RETURN
        (_bcmx_dest_to_unit_port(lport, &bcm_unit, &bcm_port,
                                 BCMX_DEST_CONVERT_DEFAULT));

    return bcm_meter_get(bcm_unit, bcm_port,
                         mid, kbits_sec, kbits_burst);
}


/*
 * Function:
 *      bcmx_meter_set
 */

int
bcmx_meter_set(bcmx_lport_t lport,
               int mid,
               uint32 kbits_sec,
               uint32 kbits_burst)
{
    int         bcm_unit;
    bcm_port_t  bcm_port;

    BCMX_METER_INIT_CHECK;

    BCM_IF_ERROR_RETURN
        (_bcmx_dest_to_unit_port(lport, &bcm_unit, &bcm_port,
                                 BCMX_DEST_CONVERT_DEFAULT));

    return bcm_meter_set(bcm_unit, bcm_port,
                         mid, kbits_sec, kbits_burst);
}


/*
 * Function:
 *      bcmx_ffppacketcounter_set
 */

int
bcmx_ffppacketcounter_set(bcmx_lport_t lport,
                          int mid,
                          uint64 val)
{
    int         bcm_unit;
    bcm_port_t  bcm_port;

    BCMX_METER_INIT_CHECK;

    BCM_IF_ERROR_RETURN
        (_bcmx_dest_to_unit_port(lport, &bcm_unit, &bcm_port,
                                 BCMX_DEST_CONVERT_DEFAULT));

    return bcm_ffppacketcounter_set(bcm_unit, bcm_port,
                                    mid, val);
}


/*
 * Function:
 *      bcmx_ffppacketcounter_get
 */

int
bcmx_ffppacketcounter_get(bcmx_lport_t lport,
                          int mid,
                          uint64 *val)
{
    int         bcm_unit;
    bcm_port_t  bcm_port;

    BCMX_METER_INIT_CHECK;

    BCMX_PARAM_NULL_CHECK(val);

    BCM_IF_ERROR_RETURN
        (_bcmx_dest_to_unit_port(lport, &bcm_unit, &bcm_port,
                                 BCMX_DEST_CONVERT_DEFAULT));

    return bcm_ffppacketcounter_get(bcm_unit, bcm_port,
                                    mid, val);
}


/*
 * Function:
 *      bcmx_ffpcounter_init
 */

int
bcmx_ffpcounter_init(void)
{
    int rv = BCM_E_UNAVAIL, tmp_rv;
    int i, bcm_unit;

    BCMX_METER_INIT_CHECK;

    BCMX_UNIT_ITER(bcm_unit, i) {
        tmp_rv = bcm_ffpcounter_init(bcm_unit);
        BCM_IF_ERROR_RETURN(BCMX_METER_ERROR_CHECK(bcm_unit, tmp_rv, &rv));
    }

    return rv;
}


/*
 * Function:
 *      bcmx_ffpcounter_create
 */

int
bcmx_ffpcounter_create(bcmx_lport_t lport, int *ffpcounterid)
{
    int         bcm_unit;
    bcm_port_t  bcm_port;

    BCMX_METER_INIT_CHECK;

    BCMX_PARAM_NULL_CHECK(ffpcounterid);

    BCM_IF_ERROR_RETURN
        (_bcmx_dest_to_unit_port(lport, &bcm_unit, &bcm_port,
                                 BCMX_DEST_CONVERT_DEFAULT));

    return bcm_ffpcounter_create(bcm_unit, bcm_port,
                                 ffpcounterid);
}


/*
 * Function:
 *      bcmx_ffpcounter_delete
 */

int
bcmx_ffpcounter_delete(bcmx_lport_t lport, int ffpcounterid)
{
    int         bcm_unit;
    bcm_port_t  bcm_port;

    BCMX_METER_INIT_CHECK;

    BCM_IF_ERROR_RETURN
        (_bcmx_dest_to_unit_port(lport, &bcm_unit, &bcm_port,
                                 BCMX_DEST_CONVERT_DEFAULT));

    return bcm_ffpcounter_delete(bcm_unit, bcm_port,
                                 ffpcounterid);
}


/*
 * Function:
 *      bcmx_ffpcounter_delete_all
 */

int
bcmx_ffpcounter_delete_all(void)
{
    int rv = BCM_E_UNAVAIL, tmp_rv;
    int i, bcm_unit;

    BCMX_METER_INIT_CHECK;

    BCMX_UNIT_ITER(bcm_unit, i) {
        tmp_rv = bcm_ffpcounter_delete_all(bcm_unit);
        BCM_IF_ERROR_RETURN
            (BCMX_METER_DELETE_ERROR_CHECK(bcm_unit, tmp_rv, &rv));
    }

    return rv;
}


/*
 * Function:
 *      bcmx_ffpcounter_set
 */

int
bcmx_ffpcounter_set(bcmx_lport_t lport,
                    int ffpcounterid,
                    uint64 val)
{
    int         bcm_unit;
    bcm_port_t  bcm_port;

    BCMX_METER_INIT_CHECK;

    BCM_IF_ERROR_RETURN
        (_bcmx_dest_to_unit_port(lport, &bcm_unit, &bcm_port,
                                 BCMX_DEST_CONVERT_DEFAULT));

    return bcm_ffpcounter_set(bcm_unit, bcm_port,
                              ffpcounterid, val);
}


/*
 * Function:
 *      bcmx_ffpcounter_get
 */

int
bcmx_ffpcounter_get(bcmx_lport_t lport,
                    int ffpcounterid,
                    uint64 * val)
{
    int         bcm_unit;
    bcm_port_t  bcm_port;

    BCMX_METER_INIT_CHECK;

    BCMX_PARAM_NULL_CHECK(val);

    BCM_IF_ERROR_RETURN
        (_bcmx_dest_to_unit_port(lport, &bcm_unit, &bcm_port,
                                 BCMX_DEST_CONVERT_DEFAULT));

    return bcm_ffpcounter_get(bcm_unit, bcm_port,
                              ffpcounterid, val);
}
