/*
 * $Id: util.c 1.3 Broadcom SDK $
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
 * Driver utility routines
 */

#include <assert.h>
#include <soc/enet.h>
#include <soc/util.h>
#include <sal/types.h>
#include <sal/core/libc.h>
#include <sal/core/thread.h>

/*
 * soc_timeout
 *
 *   These routines implement a polling timer that, in the normal case,
 *   has low overhead, but provides reasonably accurate timeouts for
 *   intervals longer than a millisecond.
 *
 *   min_polls should be chosen so the operation is expected to complete
 *   within min_polls, if possible.  If the operation completes within
 *   min_polls, there is very little overhead.  Otherwise, the routine
 *   starts making O/S calls to check the real time clock and uses an
 *   exponential timeout to avoid hogging the CPU.
 *
 *   Example usage:
 *
 *	soc_timeout_t		to;
 *	sal_usecs_t		timeout_usec = 100000;
 *	int			min_polls = 100;
 *
 *	soc_timeout_init(&to, timeout_usec, min_polls);
 *
 *	while (check_status(thing) != DONE)
 *		if (soc_timeout_check(&to)) {
 *              if (check_status(thing) == DONE) {
 *                  break;
 *              }
 *			printf("Operation timed out\n");
 *			return ERROR;
 *		}
 *
 *   Note that even after timeout the status should be checked
 *   one more time.  Otherwise there is a race condition where an
 *   ill-placed O/S task reschedule could cause a false timeout.
 */

void
soc_timeout_init(soc_timeout_t *to, sal_usecs_t usec, int min_polls)
{
    to->min_polls = min_polls;
    to->usec = usec;
    to->polls = 1;
    to->exp_delay = 1;   /* In case caller sets min_polls < 0 */
}

int
soc_timeout_check(soc_timeout_t *to)
{
    if (++to->polls >= to->min_polls) {
	if (to->min_polls >= 0) {
	    /*
	     * Just exceeded min_polls; calculate expiration time by
	     * consulting O/S real time clock.
	     */

	    to->min_polls = -1;
	    to->expire = SAL_USECS_ADD(sal_time_usecs(), to->usec);
	    to->exp_delay = 1;
	} else {
	    /*
	     * Exceeded min_polls in a previous call.
	     * Consult O/S real time clock to check for expiration.
	     */

	    if (SAL_USECS_SUB(sal_time_usecs(), to->expire) >= 0) {
		return 1;
	    }

	    sal_usleep(to->exp_delay);

	    /* Exponential backoff with 10% maximum latency */

	    if ((to->exp_delay *= 2) > to->usec / 10) {
		to->exp_delay = to->usec / 10;
	    }
	}
    }

    return 0;
}
int
soc_tightdelay_timeout_check(soc_timeout_t *to)
{
    if (++to->polls >= to->min_polls) {
	if (to->min_polls >= 0) {
	    /*
	     * Just exceeded min_polls; calculate expiration time by
	     * consulting O/S real time clock.
	     */

	    to->min_polls = -1;
	    to->expire = SAL_USECS_ADD(sal_time_usecs(), to->usec);
	    to->exp_delay = 1;
	}
        else if (to->expire < SOC_TIGHTLOOP_DELAY_LIMIT_USECS) {
	    /*
	     * Exceeded min_polls in a previous call.
	     * Consult O/S real time clock to check for expiration.
	     */

	    if (SAL_USECS_SUB(sal_time_usecs(), to->expire) >= 0) {
		return 1;
	    }

	    sal_udelay(to->exp_delay);

	    /* Exponential backoff with 10% maximum latency */

	    if ((to->exp_delay *= 2) > to->usec / 10) {
		to->exp_delay = to->usec / 10;
	    }
	}
        else {
	    /*
	     * Exceeded min_polls in a previous call.
	     * Consult O/S real time clock to check for expiration.
	     */

	    if (SAL_USECS_SUB(sal_time_usecs(), to->expire) >= 0) {
		return 1;
	    }

	    sal_usleep(to->exp_delay);

	    /* Exponential backoff with 10% maximum latency */

	    if ((to->exp_delay *= 2) > to->usec / 10) {
		to->exp_delay = to->usec / 10;
	    }
	}
    }

    return 0;
}

sal_usecs_t
soc_timeout_elapsed(soc_timeout_t *to)
{
    sal_usecs_t		start_time;

    start_time = SAL_USECS_SUB(to->expire, to->usec);

    return SAL_USECS_SUB(sal_time_usecs(), start_time);
}


/*
 * Function:
 *	soc_ntohl_load
 * Purpose:
 *	Load a 32-bit value and convert from network to host byte order.
 * Parameters:
 *	a - Address to load from
 * Returns:
 *	32-bit value.
 * Notes:
 *	Unaligned addresses are handled (even if no swap is needed).
 */
uint32
soc_ntohl_load(const void *a)
{
    uint32	v;

#if defined(LE_HOST)
    v =  ((uint8 *)a)[0] << 0;
    v |= ((uint8 *)a)[1] << 8;
    v |= ((uint8 *)a)[2] << 16;
    v |= ((uint8 *)a)[3] << 24;
#else /* BE_HOST */
    v =  ((uint8 *)a)[0] << 24;
    v |= ((uint8 *)a)[1] << 16;
    v |= ((uint8 *)a)[2] << 8;
    v |= ((uint8 *)a)[3] << 0;
#endif /* BE_HOST */

    return(v);
}

/*
 * Function:
 *	soc_ntohs_load
 * Purpose:
 *	Load a 16-bit value and convert from network to host byte order.
 * Parameters:
 *	a - Address to load from
 * Returns:
 *	16-bit value.
 * Notes:
 *	Unaligned addresses are handled (even if no swap is needed).
 */
uint16
soc_ntohs_load(const void *a)
{
    uint16	v;

#if defined(LE_HOST)
    v =  ((uint8 *)a)[0] << 0;
    v |= ((uint8 *)a)[1] << 8;
#else /* BE_HOST */
    v =  ((uint8 *)a)[0] << 8;
    v |= ((uint8 *)a)[1] << 0;
#endif /* BE_HOST */

    return(v);
}

/*
 * Function:
 *	soc_htonl_store
 * Purpose:
 *	Convert a 32-bit value from host to network byte order and store.
 * Parameters:
 *	a - Address to store to
 *	v - 32-bit value to store
 * Returns:
 *	Original value of v
 * Notes:
 *	Unaligned addresses are handled (even if no swap is needed).
 */
uint32
soc_htonl_store(void *a, uint32 v)
{
#if defined(LE_HOST)
    ((uint8 *)a)[0] = v >> 0;
    ((uint8 *)a)[1] = v >> 8;
    ((uint8 *)a)[2] = v >> 16;
    ((uint8 *)a)[3] = v >> 24;
#else /* BE_HOST */
    ((uint8 *)a)[0] = v >> 24;
    ((uint8 *)a)[1] = v >> 16;
    ((uint8 *)a)[2] = v >> 8;
    ((uint8 *)a)[3] = v >> 0;
#endif /* BE_HOST */

    return(v);
}

/*
 * Function:
 *	soc_htons_store
 * Purpose:
 *	Convert a 16-bit value from host to network byte order and store.
 * Parameters:
 *	a - Address to store to
 *	v - 16-bit value to store
 * Returns:
 *	Original value of v
 * Notes:
 *	Unaligned addresses are handled (even if no swap is needed).
 */
uint16
soc_htons_store(void *a, uint16 v)
{
#if defined(LE_HOST)
    ((uint8 *)a)[0] = v >> 0;
    ((uint8 *)a)[1] = v >> 8;
#else /* BE_HOST */
    ((uint8 *)a)[0] = v >> 8;
    ((uint8 *)a)[1] = v >> 0;
#endif /* BE_HOST */

    return(v);
}
