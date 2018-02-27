/*
 * $Id: field.c 1.85.6.3 Broadcom SDK $
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
 * Module: Field Processor APIs
 *
 * Purpose:
 *     'Field Processor' (FP) API for BCM88200 (SBX FE-2000 + Guadalupe-2000)
 *     This is the outer module that covers G2kP2 and G2kP3.
 */

#define _SBX_FE2K_FIELD_H_NEEDED_ TRUE

#include <soc/drv.h>
#include <soc/sbx/sbx_drv.h>

#include <bcm/types.h>
#include <bcm/debug.h>
#include <bcm/error.h>
#include <bcm/field.h>

#include <bcm_int/sbx/fe2000/field.h>

/*
 *  A note about locking:
 *
 *  This module uses one lock per unit (plus there is a lock used by the list
 *  manager when managing 'ranges', plus one lock that is only used to protect
 *  resources global to all units). This unit lock prevents multiple concurrent
 *  transactions against a specific unit. However, this granularity is possibly
 *  suboptimal in some cases (such as those cases where only a group or entry
 *  or range is being manipulated), though not in all (locks for range, entry,
 *  and group would have to be taken when committing changes to the hardware;
 *  for group and entry when inserting/removing entries; &c). It is possible
 *  that a finer granularity could be used, but this means more complicated
 *  locking process and additional resources per unit.  The lock per unit
 *  method was chosen primarily to control complexity, but it also has the
 *  advantage of not needing as many resources.
 */

/******************************************************************************
 *
 *  Local functions and data
 */

/* Global lock, for protecting unit init and detach functions */
static volatile sal_mutex_t _sbx_fe2000_field_glob_lock = NULL;

/* Unit lock and unit specific information needed at global layer */
typedef struct _sbx_fe2000_field_unit_info_s {
    sal_mutex_t lock;                      /* operational lock for this unit */
    soc_sbx_ucode_type_t microcode;        /* this unit's microcode type */
    void *data;                            /* this unit's private data */
} _sbx_fe2000_field_unit_info_t;
const _sbx_fe2000_field_unit_info_t _sbx_fe2000_field_unit_initValue =
    { NULL, SOC_SBX_UCODE_TYPE_NONE, NULL};
static _sbx_fe2000_field_unit_info_t _sbx_fe2000_field[BCM_MAX_NUM_UNITS];

/*
 *  These locals are needed in all of the functions; just type it once.
 */
#define _FE2000_FIELD_COMMON_LOCALS \
    sal_mutex_t          lock;              /* unit lock */ \
    soc_sbx_ucode_type_t microcode;         /* unit microcode version */ \
    void                 *unitData;         /* unit private data */ \
    int                  result             /* working result */

/*
 *  Name
 *    _bcm_fe2000_field_unit_intro
 *  Purpose
 *    Perform initial unit validation and claim unit lock if good
 *  Arguments
 *    (in) int unit = the unit on which we're entering field context
 *    (out) sal_mutex_t *lock = where to put the unit's lock handle
 *    (out) soc_sbx_ucode_type_t *microcode = where to put microcode version
 *    (out) void **unitData = where to put unit's data pointer
 *  Returns
 *    bcm_error_t cast as int
 *       BCM_E_NONE if successful
 *       BCM_E_UNIT if the unit number is invalid
 *       BCM_E_INIT if the unit is not initialised
 *       BCM_E_INTERNAL if there are lock problems
 *       BCM_E_CONFIG if the unit microcode has changed
 *  Notes
 *    If lock is non-NULL on return, it must be released.
 */
static int
_bcm_fe2000_field_unit_intro(int unit,
                             sal_mutex_t *lock,
                             soc_sbx_ucode_type_t *microcode,
                             void **unitData)
{
    FIELD_EVERB((FIELD_MSG1("unit %d intro check\n"), unit));
    /* check unit valid */
    if (!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) {
        FIELD_ERR((FIELD_MSG1("unit %d invalid\n"), unit));
        *lock = NULL;
        return BCM_E_UNIT;
    }
    /* check unit intialised */
    if ((!(_sbx_fe2000_field_glob_lock)) ||
        (!(*lock = _sbx_fe2000_field[unit].lock))) {
        /* unit not initialised */
        FIELD_ERR((FIELD_MSG1("unit %d not initialised\n"), unit));
        *lock = NULL;
        return BCM_E_INIT;
    }
    /* claim unit lock */
    FIELD_EVERB((FIELD_MSG1("take unit %d lock\n"), unit));
    if (sal_mutex_take(*lock, sal_mutex_FOREVER)) {
        /* failed to claim the lock */
        FIELD_ERR((FIELD_MSG1("failed to take unit %d lock\n"), unit));
        *lock = NULL;
        return BCM_E_INTERNAL;
    }
    /* make sure microcode type is as expected */
    *microcode = _sbx_fe2000_field[unit].microcode;
    if (*microcode != SOC_SBX_CONTROL(unit)->ucodetype) {
        FIELD_ERR((FIELD_MSG1("unit %d microcode type changed %d -> %d\n"),
                   unit,
                   _sbx_fe2000_field[unit].microcode,
                   SOC_SBX_CONTROL(unit)->ucodetype));

        return BCM_E_CONFIG;
    }
    /* also get the unit's private data */
    *unitData = _sbx_fe2000_field[unit].data;
    return BCM_E_NONE;
}

/*
 *  Name
 *    _bcm_fe2000_field_unit_outro
 *  Purpose
 *    Perform cleanup and release unit lock
 *  Arguments
 *    (in) int unit = the unit on which we're entering field context
 *    (out) sal_mutex_t lock = lock being held
 *  Returns
 *     int (implied cast from bcm_error_t)
 *       BCM_E_NONE if successful
 *       BCM_E_* appropriately otherwise
 *  Notes
 *    Only releases the lock if it's not NULL.
 */
static int
_bcm_fe2000_field_unit_outro(int resultSoFar, int unit, sal_mutex_t lock)
{
    /* release unit lock */
    FIELD_EVERB((FIELD_MSG1("release unit %d lock\n"), unit));
    if (lock && sal_mutex_give(lock)) {
        /* failed to release the lock */
        FIELD_ERR((FIELD_MSG1("failed to release unit %d lock\n"), unit));
        return BCM_E_INTERNAL;
    }
    return resultSoFar;
}


/******************************************************************************
 *
 *  Implementation exported functions and data
 */


#ifdef BROADCOM_DEBUG
const char *_sbx_fe2000_field_qual_name[bcmFieldQualifyCount] = BCM_FIELD_QUALIFY_STRINGS;
const char *_sbx_fe2000_field_action_name[bcmFieldActionCount] = BCM_FIELD_ACTION_STRINGS;
#endif /* def BROADCOM_DEBUG */

#ifdef BROADCOM_DEBUG
/*
 *   Function
 *      _bcm_fe2000_bcm_debug
 *   Purpose
 *      Shell for BCM_DEBUG in certain places of the code where compiler
 *      issues have forced it to be handled in a different manner.
 *   Parameters
 *      (in) const int flags = BCM_DEBUG print flags
 *      (in) const char *message = BCM_DEBUG message (but already processed)
 *   Notes
 *      In some compilers, the nesting of macros past a certain point causes
 *      them to get 'out of phase' with themselves, and suddenly the rest of
 *      the file doesn't make sense.  Where that has been discovered, we
 *      replace the BCM_DEBUG macro with a call to this function.  Unhappily,
 *      it seems we can't macro it into oblivion when debugging is disabled,
 *      since the nesting still remains.
 */
void
_bcm_fe2000_bcm_debug(const int flags,
                      const char *message)
{
    /* display the message */
    BCM_DEBUG(flags,(message));
}

/*
 *   Function
 *      _bcm_fe2000_field_mac_expand
 *   Purpose
 *      Parse a MAC address from SBX rule format into something displayable.
 *   Parameters
 *      (out) char *result = where to put the MAC address string
 *      (in) int length = size of buffer for MAC address string
 *      (in) uint64 macAddr = MAC address (lower 48b; upper 16b ignored)
 *   Returns
 *      Pointer to the input buffer space.
 *   Notes
 *      The compiler mangling, er, handling of 64b numbers is sreiousyl borkne
 *      if the build is made on x86/32, but seems fine if on x86/64 and BE.
 */
char *
_bcm_fe2000_field_mac_expand(char *result,
                             unsigned int length,
                             uint64 macAddr) {
    static const uint64 guessFormat = 0x123456789ABCDEF0ll;
                           /* 64bLE =   123456789ABCDEF0 -- looks BE here! */
                           /* 32bLE =   9ABCDEF012345678 -- Some NPs, others? */
                           /* 16bLE =   DEF09ABC56781234 -- Old DEC, others? */
                           /*  8bLE =   F0DEBC9A78563412 -- x86, others */
                           /*  4bLE =   0FEDCBA987654321 -- forget it! */
    const uint8 peekChar = *((const uint8*)(&guessFormat));
    uint8 *peekHere = (uint8*)(&macAddr);
    uint8 macParsed[8];
    unsigned int toggle;
    unsigned int index;

    /*
     *  Since the compiler can't deal with simple arithmetic and 64bit numbers,
     *  we shall just parse it from bytes, but we need to guess the direction
     *  and grain size.
     */
    switch (peekChar) {
    case 0xF0:
        toggle = 0x07;
        break;
    case 0xDE:
        toggle = 0x06;
        break;
    case 0x9A:
        toggle = 0x04;
        break;
    case 0x12:
        toggle = 0x00;
        break;
    default:
        toggle = 0x0F;
    }
    if (toggle < 0x08) {
        /* toggle distance < sizeof item, so valid */
        for (index = 0; index < 8; index++) {
            macParsed[index] = peekHere[index ^ toggle];
        }
        sal_snprintf(result,
                     length,
                     "%02X:%02X:%02X:%02X:%02X:%02X",
                     macParsed[2],
                     macParsed[3],
                     macParsed[4],
                     macParsed[5],
                     macParsed[6],
                     macParsed[7]);
    } else {
        /* don't know this one or don't want to bother with it */
        sal_snprintf(result,
                     length,
                     "%s",
                     "??:??:??:??:??:??");
        return result;
    }
    return result;
}

/*
 *  Function
 *    _bcm_fe2000_field_qset_dump
 *  Purpose
 *    Dump qualifier list from a qset
 *  Parameters
 *    (in) bcm_field_qset_t qset = the qset to dump
 *    (in) char *prefix = line prefix
 *  Returns
 *    int (implied cast from bcm_error_t)
 *                  BCM_E_NONE if successful
 *                  BCM_E_* appropriately if not
 *  Notes
 *    No error checking or locking is done here.
 */
int
_bcm_fe2000_field_qset_dump(const bcm_field_qset_t qset,
                            const char *prefix)
{
    bcm_field_qualify_t qualifier;
    unsigned int column = 0;

    /* for each qualifier potentially in the qset  */
    for (qualifier = 0; qualifier < bcmFieldQualifyCount; qualifier++) {
        /* if that qualifier actually is in the qset */
        if (BCM_FIELD_QSET_TEST(qset, qualifier)) {
            /* display the qualifier */
            if (0 == column) {
                /* just starting out */
                FIELD_PRINT(("%s%s",
                             prefix,
                             _sbx_fe2000_field_qual_name[qualifier]));
                column = (sal_strlen(prefix) +
                          sal_strlen(_sbx_fe2000_field_qual_name[qualifier]));
            } else if ((3 + column +
                       sal_strlen(_sbx_fe2000_field_qual_name[qualifier])) >=
                       _SBX_FE2K_FIELD_PAGE_WIDTH) {
                /* this qualifier would wrap */
                FIELD_PRINT((",\n%s%s",
                             prefix,
                             _sbx_fe2000_field_qual_name[qualifier]));
                column = (sal_strlen(prefix) +
                          sal_strlen(_sbx_fe2000_field_qual_name[qualifier]));
            } else {
                /* this qualifier fits on the line */
                FIELD_PRINT((", %s",
                             _sbx_fe2000_field_qual_name[qualifier]));
                column += (2 +
                           sal_strlen(_sbx_fe2000_field_qual_name[qualifier]));
            }
        } /* if (BCM_FIELD_QSET_TEST(thisGroup->qset, qualifier)) */
    } /* for (qualifier = 0; qualifier < bcmFieldQualifyCount; qualifier++) */
    if (0 < column) {
        FIELD_PRINT(("\n"));
    } else {
        FIELD_PRINT(("%s(none)\n", prefix));
    }
    return BCM_E_NONE;
}
#endif /* def BROADCOM_DEBUG */

/*
 *   Function
 *      _bcm_fe2000_compare_entry_priority
 *   Purpose
 *      Compare two entry priorities.  This returns -1 if the first is less
 *      than the second, 0 if the first is equal to the second (or if either
 *      priority is not valid), 1 if the first is greater than the second.
 *   Parameters
 *      (in) int pri1 = first priority to compare
 *      (in) int pri2 = second priority to compare
 *   Returns
 *      signed int = 0 if priorities are equal
 *                   >0 if pri1 is higher priority than pri2
 *                   <0 if pri1 is lower priority than pri2
 *   Notes
 *      Valid priorities are: 0..MAXINT.
 *
 *      The actual mapping of the priorities is (from highest priority to
 *      lowest priority): MAXINT down to 0.  Negative priorities are invalid
 *      for the SBX platform.
 *
 *      If a priority is invalid and the other is valid, the valid one is
 *      considered greater; if they're both invalid, they're considered equal.
 *
 *      Unhappily, this gets called a lot and it involves a lot of branches and
 *      obligatorily sequential decisions as written. It'd be better if this
 *      could be written in assembly, using inline, or made somehow otherwise
 *      more optimal; maybe later?  It does early returns and boolean
 *      short-circuiting where reasonable; this should help some.
 */
signed int
_bcm_fe2000_compare_entry_priority(int pri1,
                                   int pri2)
{
    /* validate parameters */
    /* note that we don't allow *any* negative priorities on this platform */
    if (0 > pri1) {
        /* pri1 is not valid */
        FIELD_ERR((FIELD_MSG1("pri1 (%d) invalid\n"), pri1));
        if (0 > pri2) {
            /* pri2 is not valid either, so equal   */
            FIELD_ERR((FIELD_MSG1("pri2 (%d) invalid\n"), pri1));
            return 0;
        }
        /* pri2 is valid, so it's greater */
        return -1;
    }
    if (0 > pri2) {
        /* pri2 is not valid, so pri1 is greater (since it's valid) */
        FIELD_ERR((FIELD_MSG1("pri2 (%d) invalid\n"), pri1));
        return 1;
    }
    /*
     *  All other cases degenerated into this, with the change from the special
     *  values being negative to them being just really wide-flung positive
     *  values, and the folding of the negative side into TCAM addresses; since
     *  we don't have a TCAM, that went away also.
     */
    return pri1 - pri2;
}

/*
 *   Function
 *      _bcm_fe2000_qset_subset
 *   Purpose
 *      Check whether qset2 is a subset of qset1.
 *   Parameters
 *      (in) bcm_field_qset_t qset1 = qualifier set 1
 *      (in) bcm_field_qset_t qset2 = qualifier set 2
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if qset2 is subset of qset1
 *                    BCM_E_FAIL if qset2 is not subset of qset1
 *   Notes
 *      This checks whether qset2 is a subset, either proper and improper, of
 *      qset1, and returns BCM_E_NONE if qset2 is a subset of qset1, but
 *      returns BCM_E_FAIL if qset2 is not a subset of qset1.  Other errors may
 *      be returned under appropriate conditions.
 */
int
_bcm_fe2000_qset_subset(bcm_field_qset_t qset1,
                        bcm_field_qset_t qset2)
{
    unsigned int index;                 /* working index for loops */
    int result;                         /* result for caller */

    FIELD_VVERB((FIELD_MSG("(*,*)\n")));

    /* be optimistic */
    result = BCM_E_NONE;

    /* check all qualifiers */
    FIELD_VVERB((FIELD_MSG1("scanning qualifiers\n")));
    for (index = 0;
         (index < bcmFieldQualifyCount) && (BCM_E_NONE == result);
         index++) {
        if (BCM_FIELD_QSET_TEST(qset2,index)) {
            if (!BCM_FIELD_QSET_TEST(qset1,index)) {
                result = BCM_E_FAIL;
                FIELD_ERR((FIELD_MSG1("qualifier %d (%s) in qset 2"
                                      " but not in qset 1\n"),
                           index,
                           _sbx_fe2000_field_qual_name[index]));
            }
        } /* if (BCM_FIELD_QSET_TEST(qset2,index)) */
    } /* for (index = 0; index < bcmFieldQualifyCount; index++) */

    /* then tell the caller the result */
    FIELD_VVERB((FIELD_MSG("(*,*) = %d (%s)\n"),
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}



/******************************************************************************
 *
 *  API exported functions and data
 */

/*
 *   Function
 *      _bcm_fe2000_field_detach
 *   Purpose
 *      Shut down the field APIs.
 *   Parameters
 *      (in) int unit = the unit number
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
static int
_bcm_fe2000_field_detach(int unit,
                         sal_mutex_t lock,
                         soc_sbx_ucode_type_t microcode,
                         void *unitData)
{
    int result = BCM_E_NONE;

    FIELD_VVERB((FIELD_MSG("(%d,%08X,%08X,%08X)\n"),
                 unit,
                 (uint32)lock,
                 (uint32)microcode,
                 (uint32)unitData));

    if (BCM_E_NONE == result) {
        /* detach the unit so nobody else will access it */
        FIELD_EVERB((FIELD_MSG1("detach unit %d\n"), unit));
        _sbx_fe2000_field[unit].lock = NULL;
        _sbx_fe2000_field[unit].data = NULL;
        _sbx_fe2000_field[unit].microcode = SOC_SBX_UCODE_TYPE_NONE;

        /* now tell the microcode specific implementation to detach */
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_detach(unitData);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
        FIELD_EVERB((FIELD_MSG1("destroy unit %d lock\n"), unit));
        sal_mutex_destroy(lock);
    }

    FIELD_VERB((FIELD_MSG("(%d,%08X,%08X,%08X) = %d (%s)\n"),
                unit,
                (uint32)lock,
                (uint32)microcode,
                (uint32)unitData,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

int bcm_fe2000_field_detach(int unit);

/*
 *  Function
 *     bcm_fe2000_field_init
 *  Purpose
 *     Initialise the field APIs.
 *  Parameters
 *     (in) int unit = the unit number
 *  Returns
 *     int (implied cast from bcm_error_t)
 *       BCM_E_NONE if successful
 *       BCM_E_* appropriately if not
 *  Notes
 */
int
bcm_fe2000_field_init(int unit)
{
    sal_mutex_t          lock;              /* unit lock */ 
    soc_sbx_ucode_type_t microcode;         /* unit microcode version */ 
    void                 *unitData;         /* unit private data */ 
    sal_mutex_t tempLock;
    int result = BCM_E_NONE;
    int index;

    FIELD_VERB((FIELD_MSG("(%d) enter\n"), unit));

    /* check unit valid */
    if (!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) {
        FIELD_ERR((FIELD_MSG1("unit %d invalid\n"), unit));
        return BCM_E_UNIT;
    }

    if (_sbx_fe2000_field[unit].lock) {
        result = bcm_fe2000_field_detach(unit);
        BCM_IF_ERROR_RETURN(result);
    }

    /* Make sure the global lock exists */
    if (!_sbx_fe2000_field_glob_lock) {
        /* Global lock does not exist */
        FIELD_EVERB((FIELD_MSG1("create global lock\n")));
        tempLock = sal_mutex_create("fe2000_field_global_lock");
        if (!tempLock) {
            FIELD_ERR((FIELD_MSG1("unable to create global lock\n")));
            return BCM_E_RESOURCE;
        }
        /* Claim the global lock before exposing it */
        FIELD_EVERB((FIELD_MSG1("claim global lock\n")));
        if (sal_mutex_take(tempLock, sal_mutex_FOREVER)) {
            /* something went wrong claiming the lock */
            FIELD_ERR((FIELD_MSG1("unable to claim global lock in create\n")));
            /* free the working lock even though it's defective */
            sal_mutex_destroy(tempLock);
            return BCM_E_INTERNAL;
        }
        /* Set the global lock to the one we now own, if it's still none */
        if (!_sbx_fe2000_field_glob_lock) {
            FIELD_EVERB((FIELD_MSG1("set global lock\n")));
            _sbx_fe2000_field_glob_lock = tempLock;
        }
        /* Let everybody else catch up */
        FIELD_EVERB((FIELD_MSG1("give up timeslice\n")));
        sal_thread_yield();
        /* Check for race condition and compensate if needed */
        if (_sbx_fe2000_field_glob_lock != tempLock) {
            /* somebody came along during the race hole; yield to them */
            FIELD_ERR((FIELD_MSG1("detected race condition on field init for"
                                  " unit %d: trying to compensate\n"),
                       unit));
            /*
             *  We encountered a race condition during the hole beteen testing
             *  whether there was a global lock and setting it.  This hole
             *  seems to be impossible to avoid using C (and may be impossible
             *  to avoid in assembly on any platform that does not support a
             *  move-if-destination-is-zero type of instruction).
             *
             *  We compensate for this condition by destroying the losing copy
             *  of the primary lock (ours, since the other thread stomped ours)
             *  and going on.  We'll pick up the winning copy later (before
             *  filling in the unit data), so there should not be an error here
             *  (though the diagnostic may be useful and we definitely do need
             *  to free our lock in order to avoid leaking it).
             *
             *  If another thread is already waiting on our lock instead of the
             *  winner, it should receive an error during its next timeslice at
             *  the point of the wait, and that will propagate back to the
             *  caller as BCM_E_INTERNAL (there doesn't seem to be anything
             *  better for it).
             *
             *  This is a Bad Thing, but it is not, in itself, unrecoverable.
             *  We therefore continue without any error indication other than
             *  the diagnostic message (and any thread that was waiting on our
             *  lock can retry and should be okay, since it will also pick up
             *  the new global lock).
             *
             *  Unhappily, the race condition check is itself susceptible to
             *  the same condition for which it checks, so there's still a
             *  possibility of leaks and contention here.  Hopefully nobody's
             *  doing parallelised inits of the same subsystem on true parallel
             *  hardware or preemptive timesharing systems...
             */
            sal_mutex_destroy(tempLock);
        } else { /* if (_sbx_fe2000_field_glob_lock != tempLock) */
            /* no obvious race condition */
            /* Clear the global resources */
            FIELD_EVERB((FIELD_MSG1("clear global unit information\n")));
            for (index = 0; index < BCM_MAX_NUM_UNITS; index++) {
                _sbx_fe2000_field[index] = _sbx_fe2000_field_unit_initValue;
            }
            /* release the global lock */
            FIELD_EVERB((FIELD_MSG1("release global lock\n")));
            if (sal_mutex_give(tempLock)) {
                /* something went wrong */
                FIELD_ERR((FIELD_MSG1("unable to release global lock"
                                      " in create\n")));
                return BCM_E_INTERNAL;
            }
        } /* if (_sbx_fe2000_field_glob_lock != tempLock) */
    } /* if (!_sbx_fe2000_field_glob_lock) */

    /* Take the global lock */
    FIELD_EVERB((FIELD_MSG1("take global lock\n")));
    if (sal_mutex_take(_sbx_fe2000_field_glob_lock, sal_mutex_FOREVER)) {
        FIELD_ERR((FIELD_MSG1("unable to take global lock\n")));
        return BCM_E_INTERNAL;
    }

    /* Make sure the unit is not already inited */
    if (_sbx_fe2000_field[unit].lock) {
        /* this unit is already initialised; detach it first */
        FIELD_VERB((FIELD_MSG1("unit %d already inited; detaching first\n"),
                    unit));
        /* check validity of unit and claim unit lock */
        result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
        if (BCM_E_NONE == result) {
            result = _bcm_fe2000_field_detach(unit, lock, microcode, unitData);
        }
        if (BCM_E_NONE != result) {
            FIELD_ERR((FIELD_MSG1("implied detach unit %d failed: %d (%s)\n"),
                       unit,
                       result,
                       _SHR_ERRMSG(result)));
        }
    }

    if (BCM_E_NONE == result) {
        /* Create the unit lock */
        FIELD_EVERB((FIELD_MSG1("create unit %d lock\n"), unit));
        tempLock = sal_mutex_create("fe2000_field_unit_lock");

        /* Initialise the unit */
        _sbx_fe2000_field[unit].microcode = SOC_SBX_CONTROL(unit)->ucodetype;
        switch (_sbx_fe2000_field[unit].microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_init(unit,
                                                &(_sbx_fe2000_field[unit].data));
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }

        if (BCM_E_NONE == result) {
            /* init success; attach the lock to the unit */
            FIELD_EVERB((FIELD_MSG1("set unit %d lock\n"), unit));
            _sbx_fe2000_field[unit].lock = tempLock;
        } else {
            /* init fail; destroy the lock & make sure unit info clear */
            FIELD_ERR((FIELD_MSG1("init unit %d failed: %d (%s)\n"),
                       unit,
                       result,
                       _SHR_ERRMSG(result)));
            FIELD_EVERB((FIELD_MSG1("clear and release unit %d resources\n"),
                         unit));
            _sbx_fe2000_field[unit].lock = NULL;
            _sbx_fe2000_field[unit].data = NULL;
            _sbx_fe2000_field[unit].microcode = SOC_SBX_UCODE_TYPE_NONE;
            sal_mutex_destroy(tempLock);
        }
    }

    /* Release the global lock */
    FIELD_EVERB((FIELD_MSG1("release global lock\n")));
    if (sal_mutex_give(_sbx_fe2000_field_glob_lock)) {
        FIELD_ERR((FIELD_MSG1("unable to release global lock\n")));
        return BCM_E_INTERNAL;
    }

    FIELD_VERB((FIELD_MSG("(%d) return %d (%s)\n"),
                unit,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_detach
 *   Purpose
 *      Shut down the field APIs.
 *   Parameters
 *      (in) int unit = the unit number
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
int
bcm_fe2000_field_detach(int unit)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d)\n"), unit));

    /* check the global lock */
    if (!_sbx_fe2000_field_glob_lock) {
        FIELD_ERR((FIELD_MSG1("no FE2000 field units initialised\n")));
        return BCM_E_INIT;
    }

    /* claim the global lock */
    FIELD_EVERB((FIELD_MSG1("claim global lock\n")));
    if (sal_mutex_take(_sbx_fe2000_field_glob_lock, sal_mutex_FOREVER)) {
        FIELD_ERR((FIELD_MSG1("unable to take global lock\n")));
        return BCM_E_INTERNAL;
    }

    /* check validity of unit and claim unit lock */
    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        result = _bcm_fe2000_field_detach(unit, lock, microcode, unitData);
    }

    /* release globals lock */
    FIELD_EVERB((FIELD_MSG1("release global lock\n")));
    if (sal_mutex_give(_sbx_fe2000_field_glob_lock)) {
        /* failed to release the lock */
        FIELD_ERR((FIELD_MSG1("failed to release global lock\n")));
        return BCM_E_INTERNAL;
    }

    FIELD_VERB((FIELD_MSG("(%d) = %d (%s)\n"),
                unit,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_status_get
 *   Purpose
 *      Get the status of the field APIs.
 *   Parameters
 *      (in) int unit = the unit number
 *      (out) bcm_field_status_t *status = pointer to where to put status
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
int
bcm_fe2000_field_status_get(int unit,
                            bcm_field_status_t *status)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,*)\n"), unit));

    /* check parameters */
    if (!status) {
        /* invalid parameter(s) */
        FIELD_ERR((FIELD_MSG1("NULL out argument\n")));
        return BCM_E_PARAM;
    }

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_status_get(unitData, status);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,*) = %d (%s)\n"),
                unit,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_group_create
 *   Purpose
 *      Create a new group that has the specified qualifier set and priority.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_qset_t qset = the qualifier set
 *      (in) int pri = the priority
 *      (out) _field_group_index *group = where to put the group ID
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Allocates first available group ID.
 *      Can not specify a priority already taken by an existing group.
 *      Can not specify a qualifier that another group in the same stage has.
 */
int
bcm_fe2000_field_group_create(int unit,
                              bcm_field_qset_t qset,
                              int pri,
                              bcm_field_group_t *group)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,*,%d,*)\n"), unit, pri));

    /* check group pointer */
    if (!group) {
        /* NULL pointer for group output */
        FIELD_ERR((FIELD_MSG1("NULL pointer for group\n")));
        return BCM_E_PARAM;
    }

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_group_create(unitData,
                                                        qset,
                                                        pri,
                                                        group);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,*,%d,&(%08X)) = %d (%s)\n"),
                unit,
                pri,
                *group,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_group_create_id
 *   Purpose
 *      Create a new group with the specified ID that has the specified
 *      qualifier set and priority.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_qset_t qset = the qualifier set
 *      (in) int pri = the priority
 *      (in) bcm_field_group_t group = which group ID to use
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Allocates first available group ID.
 *      Can not specify a priority already taken by an existing group.
 *      Can not specify a qualifier that another group in the same stage has.
 */
int
bcm_fe2000_field_group_create_id(int unit,
                                 bcm_field_qset_t qset,
                                 int pri,
                                 bcm_field_group_t group)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,*,%d,%08X)\n"), unit, pri, group));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_group_create_id(unitData,
                                                           qset,
                                                           pri,
                                                           group);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,*,%d,%08X) = %d (%s)\n"),
                unit,
                pri,
                group,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_group_port_create_mode
 *   Purpose
 *      Create a new group that has the specified qualifier set and priority,
 *      and applies only to the specified port.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_port_t port = the port to which the group is to apply
 *      (in) bcm_field_qset_t qset = the qualifier set
 *      (in) int pri = the priority
 *      (in) bcm_field_group_mode_t mode = the group mode
 *      (out) bcm_field_group_t *group = where to put the group ID
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Allocates first available group ID.
 *      Can not specify a priority already taken by an existing group (does
 *      this mean globally or within just the single stage?)
 *      Can not specify a qualifier that another group in the same stage has.
 *      If no stage qualifier, it is assumed to be ingress stage.
 *      Only supports mode bcmFieldGroupModeAuto.
 */
int
bcm_fe2000_field_group_port_create_mode(int unit,
                                        bcm_port_t port,
                                        bcm_field_qset_t qset,
                                        int pri,
                                        bcm_field_group_mode_t mode,
                                        bcm_field_group_t *group)
{
    return BCM_E_UNAVAIL;
}

/*
 *   Function
 *      bcm_fe2000_field_group_port_create_mode_id
 *   Purpose
 *      Create a new group with the specified ID that has the specified
 *      qualifier set and priority, and applies only to the specified port.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_port_t port = the port to which the group is to apply
 *      (in) bcm_field_qset_t qset = the qualifier set
 *      (in) int pri = the priority
 *      (in) bcm_field_group_mode_t mode = the group mode
 *      (in) bcm_field_group_t group = which group ID to use
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Allocates first available group ID.
 *      Can not specify a priority already taken by an existing group.
 *      Can not specify a qualifier that another group in the same stage has.
 *      If no stage qualifier, it is assumed to be bcmFieldQualifyStageIngress.
 *      Only supports mode bcmFieldGroupModeAuto.
 */
int
bcm_fe2000_field_group_port_create_mode_id(int unit,
                                           bcm_port_t port,
                                           bcm_field_qset_t qset,
                                           int pri,
                                           bcm_field_group_mode_t mode,
                                           bcm_field_group_t group)
{
    return BCM_E_UNAVAIL;
}

/*
 *   Function
 *      bcm_fe2000_field_group_install
 *   Purpose
 *      Insert all of a group's entries from the hardware.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_group_t group = which group ID to use
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      This inserts and updates all of the groups entries to the hardware as
 *      appropriate.  No error is asserted for entries already in hardware,
 *      even if the entire group is already in hardware.
 */
int
bcm_fe2000_field_group_install(int unit,
                               bcm_field_group_t group)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X)\n"), unit, group));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_group_install(unitData, group);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X) = %d (%s)\n"),
                unit,
                group,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_group_remove
 *   Purpose
 *      Remove all of a group's entries from the hardware, but do not remove
 *      the entries from the software table.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_group_t group = which group ID to use
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      This removes the group's entries from the hardware, marking them so,
 *      and commits the changes to the hardware.
 */
int
bcm_fe2000_field_group_remove(int unit,
                              bcm_field_group_t group)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X)\n"), unit, group));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_group_remove(unitData, group);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X) = %d (%s)\n"),
                unit,
                group,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_group_flush
 *   Purpose
 *      Remove all of a group's entries from the hardware, remove the group
 *      from the hardware, remove the group's entries from the software, and
 *      remove the group from the software.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_group_t group = which group ID to use
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      This will destroy the field group and all its entries.  No mention is
 *      made that it affects ranges, so they aren't destroyed.  This also
 *      destroys the field group and its entries in hardware.
 */
int
bcm_fe2000_field_group_flush(int unit,
                             bcm_field_group_t group)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X)\n"), unit, group));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_group_flush(unitData, group);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X) = %d (%s)\n"),
                unit,
                group,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_group_set
 *   Purpose
 *      This changes the group's qualifier set so it is the specified set.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_group_t group = which group ID to use
 *      (in) bcm_field_qset_t qset = new qset
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      If there are any entries, all of them must be representable using the
 *      new qualifier set (if not, this fails), plus the new qualifier set can
 *      not change the required pattern type or stage (it will also fail in
 *      these cases).
 *      Updates are always permitted if there are no entries present.
 */
int
bcm_fe2000_field_group_set(int unit,
                           bcm_field_group_t group,
                           bcm_field_qset_t qset)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X,*)\n"), unit, group));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_group_set(unitData, group, qset);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X,*) = %d (%s)\n"),
                unit,
                group,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_group_get
 *   Purpose
 *      Gets the group's qualifier set.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_group_t group = which group ID to use
 *      (out) bcm_field_qset_t *qset = where to put the current qset
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
int
bcm_fe2000_field_group_get(int unit,
                           bcm_field_group_t group,
                           bcm_field_qset_t *qset)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X,*)\n"), unit, group));

    if (!qset) {
        FIELD_ERR((FIELD_MSG1("NULL pointer for qset\n")));
        return BCM_E_PARAM;
    }

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_group_get(unitData, group, qset);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X,*) = %d (%s)\n"),
                unit,
                group,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_group_destroy
 *   Purpose
 *      Destroys a group.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_group_t group = which group ID to use
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      There must be no entries in this group when calling this function.
 */
int
bcm_fe2000_field_group_destroy(int unit,
                               bcm_field_group_t group)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X)\n"), unit, group));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_group_destroy(unitData, group);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X) = %d (%s)\n"),
                unit,
                group,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_group_status_get
 *   Purpose
 *      Gets the group's status.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_group_t group = which group ID to use
 *      (out) bcm_field_group_status_t *status = where to put the status
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
int
bcm_fe2000_field_group_status_get(int unit,
                                  bcm_field_group_t group,
                                  bcm_field_group_status_t *status)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X,*)\n"), unit, group));

    if (!status) {
        FIELD_ERR((FIELD_MSG1("NULL pointer for status\n")));
        return BCM_E_PARAM;
    }

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_group_status_get(unitData,
                                                            group,
                                                            status);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X,*) = %d (%s)\n"),
                unit,
                group,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_range_create
 *   Purpose
 *      Create a descriptor for a range of L4 (specifically, TCP or UDP over
 *      IP) ports.
 *   Parameters
 *      (in) int unit = the unit number
 *      (out) bcm_field_range_t *range = where to put the assigned range ID
 *      (in) uint32 flags = flags for the range
 *      (in) bcm_l4_port_t min = low port number for the range
 *      (in) bcm_l4_port_t max = high port number for the range
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Range ID is always nonzero.
 */
int
bcm_fe2000_field_range_create(int unit,
                              bcm_field_range_t *range,
                              uint32 flags,
                              bcm_l4_port_t min,
                              bcm_l4_port_t max)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,*,%08X,%04X,%04X)\n"),
                unit,
                flags,
                min,
                max));

    if (!range) {
        FIELD_ERR((FIELD_MSG1("NULL pointer for range\n")));
        return BCM_E_PARAM;
    }

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_range_create(unitData,
                                                        range,
                                                        flags,
                                                        min,
                                                        max);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,&(%08X),%08X,%04X,%04X) = %d (%s)\n"),
                unit,
                *range,
                flags,
                min,
                max,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_range_create_id
 *   Purpose
 *      Create a descriptor for a range of L4 (specifically, TCP or UDP over
 *      IP) ports.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_range_t range = the range ID to use
 *      (in) uint32 flags = flags for the range
 *      (in) bcm_l4_port_t min = low port number for the range
 *      (in) bcm_l4_port_t max = high port number for the range
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Range ID is always nonzero.
 */
int
bcm_fe2000_field_range_create_id(int unit,
                                 bcm_field_range_t range,
                                 uint32 flags,
                                 bcm_l4_port_t min,
                                 bcm_l4_port_t max)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X,%08X,%04X,%04X)\n"),
                unit,
                range,
                flags,
                min,
                max));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_range_create_id(unitData,
                                                           range,
                                                           flags,
                                                           min,
                                                           max);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X,%08X,%04X,%04X) = %d (%s)\n"),
                unit,
                range,
                flags,
                min,
                max,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_range_get
 *   Purpose
 *      Create a descriptor for a range of L4 (specifically, TCP or UDP over
 *      IP) ports.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_range_t range = the range ID to use
 *      (in) uint32 *flags = where to put the flags for the range
 *      (in) bcm_l4_port_t *min = where to put range's low port number
 *      (in) bcm_l4_port_t *max = where to put range's high port number
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Range ID is always nonzero.
 */
int
bcm_fe2000_field_range_get(int unit,
                           bcm_field_range_t range,
                           uint32 *flags,
                           bcm_l4_port_t *min,
                           bcm_l4_port_t *max)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X,*,*,*)\n"),
                unit,
                range));

    if ((!flags) || (!min) || (!max)) {
        FIELD_ERR((FIELD_MSG1("NULL pointer for outbound argument\n")));
        return BCM_E_PARAM;
    }

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_range_get(unitData,
                                                     range,
                                                     flags,
                                                     min,
                                                     max);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X,&(%08X),&(%04X),&(%04X)) = %d (%s)\n"),
                unit,
                range,
                *flags,
                *min,
                *max,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_range_destroy
 *   Purpose
 *      Destroy a descriptor for a range of L4 (specifically, TCP or UDP over
 *      IP) ports.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_range_t range = the range ID to destroy
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Range ID is always nonzero.
 */
int
bcm_fe2000_field_range_destroy(int unit,
                               bcm_field_range_t range)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X)\n"), unit, range));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_range_destroy(unitData, range);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X) = %d (%s)\n"),
                unit,
                range,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_entry_create
 *   Purpose
 *      Create an empty field entry based upon the specified grup
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_group_t group = the group ID to use
 *      (out) bcm_field_entry_t *entry = where to put the entry ID
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      'The field entry identifier is the priority of the entry in the group.'
 *      Yeah, right.  Where's that deed to the oceanfront property within easy
 *      walking distance of Des Moines?
 *
 *      Actually, the field entry identifier has nothing whatsoever to do with
 *      the priority, which is set separately.  Annoyingly, this function can
 *      only insert the entry into the group based upon a priority setting of
 *      BCM_FIELD_ENTRY_PRIO_DEFAULT, and it will be moved later if the user
 *      actually bothers to set the priority.
 */
int
bcm_fe2000_field_entry_create(int unit,
                              bcm_field_group_t group,
                              bcm_field_entry_t *entry)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X,*)\n"), unit, group));

    if (!entry) {
        FIELD_ERR((FIELD_MSG1("NULL pointer for entry\n")));
        return BCM_E_PARAM;
    }

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_entry_create(unitData,
                                                        group,
                                                        entry);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X,&(%08X)) = %d (%s)\n"),
                unit,
                group,
                *entry,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_entry_create_id
 *   Purpose
 *      Create an empty field entry based upon the specified grup
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_group_t group = the group ID to use
 *      (in) bcm_field_entry_t entry = the entry ID to use
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      'The field entry identifier is the priority of the entry in the group.'
 *      Yeah, right.  Where's that deed to the oceanfront property within easy
 *      walking distance of Des Moines?
 *
 *      Actually, the field entry identifier has nothing whatsoever to do with
 *      the priority, which is set separately.  Annoyingly, this function can
 *      only insert the entry into the group based upon a priority setting of
 *      BCM_FIELD_ENTRY_PRIO_DEFAULT, and it will be moved later if the user
 *      actually bothers to set the priority.
 */
int
bcm_fe2000_field_entry_create_id(int unit,
                                 bcm_field_group_t group,
                                 bcm_field_entry_t entry)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X,%08X)\n"), unit, group, entry));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_entry_create_id(unitData,
                                                           group,
                                                           entry);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X,%08X) = %d (%s)\n"),
                unit,
                group,
                entry,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 * Function: bcm_fe2000_field_entry_multi_get
 *
 * Purpose:
 *     Gets an array of a group's entry IDs
 *
 * Parameters:
 *     unit -  (IN) BCM device number.
 *     group - (IN) Field group ID.
 *     entry_size - (IN) Maximum number of entries to return.  Set to 0
 *                       to get the number of entries available
 *     entry_array - (OUT) Pointer to a buffer to fill with the array of
 *                         entry IDs.  Ignored if entry_size is 0
 *     entry_count - (OUT) Returns the number of entries returned in the
 *                         array, or if entry_size was 0, the number of
 *                         entries available

 * Returns:
 *     BCM_E_INIT      - unit not initialized
 *     BCM_E_NOT_FOUND - Group ID not found in unit
 *     BCM_E_NONE      - Success
 */
int
bcm_fe2000_field_entry_multi_get(int unit, bcm_field_group_t group,
    int entry_size, bcm_field_entry_t *entry_array, int *entry_count)
{
    return BCM_E_UNAVAIL;
}

/*
 *   Function
 *      bcm_fe2000_field_entry_destroy
 *   Purpose
 *      Destroy a field entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID to destroy
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Does not remove any associated entries from the hardware.[!?]
 */
int
bcm_fe2000_field_entry_destroy(int unit,
                               bcm_field_entry_t entry)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X)\n"),
                unit,
                entry));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_entry_destroy(unitData, entry);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X) = %d (%s)\n"),
                unit,
                entry,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_entry_destroy_all
 *   Purpose
 *      Destroy all field entries
 *   Parameters
 *      (in) int unit = the unit number
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Does not remove any associated entries from the hardware.[!?]
 */
int
bcm_fe2000_field_entry_destroy_all(int unit)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d)\n"), unit));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_entry_destroy_all(unitData);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d) = %d (%s)\n"),
                unit,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_entry_copy
 *   Purpose
 *      Copy an existing field entry to another one
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t src_entry = the original entry ID
 *      (out) bcm_field_entry_t *dst_entry = where to put the copy entry ID
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      'The field entry identifier is the priority of the entry in the group.'
 *      Yeah, right.  Where's that deed to the oceanfront property within easy
 *      walking distance of Des Moines?
 *
 *      Actually, the field entry identifier has nothing whatsoever to do with
 *      the priority, which is set separately.  Annoyingly, this function will
 *      insert the entry into the group based upon a priority setting of
 *      BCM_FIELD_ENTRY_PRIO_DEFAULT, and it will be moved later if the user
 *      actually bothers to set the priority.
 *
 *      This can only copy the entry within its group, and the copy will be
 *      inserted as the last entry of the original entry's priority.  If the
 *      original entry is participating in counter sharing, so is the copy; if
 *      not, neither is the copy (but if the original had a counter allocated,
 *      so will the copy, though it will be a *different* counter).
 */
int
bcm_fe2000_field_entry_copy(int unit,
                            bcm_field_entry_t src_entry,
                            bcm_field_entry_t *dst_entry)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X,*)\n"), unit, src_entry));

    if (!dst_entry) {
        FIELD_ERR((FIELD_MSG1("NULL pointer for destination entry\n")));
        return BCM_E_PARAM;
    }

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_entry_copy(unitData,
                                                      src_entry,
                                                      dst_entry);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X,&(%08X)) = %d (%s)\n"),
                unit,
                src_entry,
                *dst_entry,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_entry_copy_id
 *   Purpose
 *      Copy an existing field entry to a specific one
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t src_entry = the original entry ID
 *      (in) bcm_field_entry_t dst_entry = the copy entry ID
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      'The field entry identifier is the priority of the entry in the group.'
 *      Yeah, right.  Where's that deed to the oceanfront property within easy
 *      walking distance of Des Moines?
 *
 *      Actually, the field entry identifier has nothing whatsoever to do with
 *      the priority, which is set separately.  Annoyingly, this function will
 *      insert the entry into the group based upon a priority setting of
 *      BCM_FIELD_ENTRY_PRIO_DEFAULT, and it will be moved later if the user
 *      actually bothers to set the priority.
 *
 *      This can only copy the entry within its group, and the copy will be
 *      inserted as the last entry of the priority.  If the original entry is
 *      participating in counter sharing, so is the copy; if not, neither is
 *      the copy (but if the original had a counter allocated, so will the
 *      copy, though it will be a different counter).
 */
int
bcm_fe2000_field_entry_copy_id(int unit,
                               bcm_field_entry_t src_entry,
                               bcm_field_entry_t dst_entry)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X,%08X)\n"), unit, src_entry, dst_entry));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_entry_copy_id(unitData,
                                                         src_entry,
                                                         dst_entry);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X,%08X) = %d (%s)\n"),
                unit,
                src_entry,
                dst_entry,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_entry_install
 *   Purpose
 *      Install a field entry to the hardware
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID to destroy
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      This causes an error if the entry is already in hardware.
 *      This will commit the appropriate database to the hardware.
 */
int
bcm_fe2000_field_entry_install(int unit,
                               bcm_field_entry_t entry)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X)\n"),
                unit,
                entry));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_entry_install(unitData, entry);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X) = %d (%s)\n"),
                unit,
                entry,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_entry_reinstall
 *   Purpose
 *      Reinstall a field entry to the hardware
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID to destroy
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Apprarently, despite the API doc indicating this can only be used to
 *      refresh an entry that is already in hardware, the regression tests
 *      require that this work to install an entry that is not in hardware.
 */
int
bcm_fe2000_field_entry_reinstall(int unit,
                                 bcm_field_entry_t entry)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X)\n"),
                unit,
                entry));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_entry_reinstall(unitData, entry);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X) = %d (%s)\n"),
                unit,
                entry,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_entry_remove
 *   Purpose
 *      Remove a field entry from the hardware
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID to destroy
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      The API doc indicates that this can only be used against an entry that
 *      is already in hardware, but the regression tests require that it work
 *      even if the entry isn't in hardware.
 */
int
bcm_fe2000_field_entry_remove(int unit,
                              bcm_field_entry_t entry)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X)\n"),
                unit,
                entry));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_entry_remove(unitData, entry);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X) = %d (%s)\n"),
                unit,
                entry,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_entry_prio_get
 *   Purpose
 *      Get the priority of a specific entry (within its group)
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (out) int *prio = where to put the entry's priority
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      The field entry identifier is NOT the priority of the entry in the
 *      group on in the system.
 *      Priority is signed; nonnegative numbers are priority order; negative
 *      numbers have special meanings.
 *      Overall sort is:
 *          highest >= numbered >= dontcare >= lowest
 */
int
bcm_fe2000_field_entry_prio_get(int unit,
                                bcm_field_entry_t entry,
                                int *prio)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X,*)\n"),
                unit,
                entry));

    if (!prio) {
        FIELD_ERR((FIELD_MSG1("NULL pointer for priority\n")));
        return BCM_E_PARAM;
    }

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_entry_prio_get(unitData,
                                                          entry,
                                                          prio);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X,&(%d)) = %d (%s)\n"),
                unit,
                entry,
                *prio,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_entry_prio_set
 *   Purpose
 *      Set the priority of a specific entry (within its group)
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) int prio = the entry's new priority
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      The field entry identifier is NOT the priority of the entry in the
 *      group on in the system.
 *      Priority is signed; nonnegative numbers are priority order; negative
 *      numbers have special meanings. Overall sort is:
 *          highest >= numbered >= dontcare >= lowest
 */
int
bcm_fe2000_field_entry_prio_set(int unit,
                                bcm_field_entry_t entry,
                                int prio)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X,%d)\n"), unit, entry, prio));

    /* check parameters */
    if ( ((prio >= _SBX_FE2K_FIELD_ENTRY_PRIO_HIGHEST) &&
         (prio != BCM_FIELD_ENTRY_PRIO_HIGHEST)) ||
         (prio < 0) ) {
        /* invalid priority */
        FIELD_ERR((FIELD_MSG1("invalid priority (%d)\n"), prio));
        return BCM_E_PARAM;
    }

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_entry_prio_set(unitData,
                                                          entry,
                                                          prio);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d) = %d (%s)\n"),
                unit,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_qualify_clear
 *   Purpose
 *      Clear all qualifiers for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
int
bcm_fe2000_field_qualify_clear(int unit,
                               bcm_field_entry_t entry)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X)\n"),
                unit,
                entry));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_qualify_clear(unitData, entry);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X) = %d (%s)\n"),
                unit,
                entry,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_qualify_InPort
 *   Purpose
 *      Set allowed ingress port for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_port_t data = allowed port
 *      (in) bcm_port_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      All-or-nothing mask only on SBX.
 *      Supports GPORTs of various types and will map back to phys port.
 */
int
bcm_fe2000_field_qualify_InPort(int unit,
                                bcm_field_entry_t entry,
                                bcm_port_t data,
                                bcm_port_t mask)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X,%08X,%08X)\n"),
                unit,
                entry,
                data,
                mask));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_qualify_InPort(unitData,
                                                          entry,
                                                          data,
                                                          mask);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X,%08X,%08X) = %d (%s)\n"),
                unit,
                entry,
                data,
                mask,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_qualify_OutPort
 *   Purpose
 *      Set allowed egress port for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_port_t data = allowed port
 *      (in) bcm_port_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      All-or-nothing mask only on SBX.
 *      Supports GPORTs of various types and will map back to phys port.
 */
int
bcm_fe2000_field_qualify_OutPort(int unit,
                                 bcm_field_entry_t entry,
                                 bcm_port_t data,
                                 bcm_port_t mask)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X,%08X,%08X)\n"),
                unit,
                entry,
                data,
                mask));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_qualify_OutPort(unitData,
                                                           entry,
                                                           data,
                                                           mask);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X,%08X,%08X) = %d (%s)\n"),
                unit,
                entry,
                data,
                mask,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_qualify_InPorts
 *   Purpose
 *      Set allowed ingress ports for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_pbmp_t data = allowed ports
 *      (in) bcm_pbmp_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
int
bcm_fe2000_field_qualify_InPorts(int unit,
                                 bcm_field_entry_t entry,
                                 bcm_pbmp_t data,
                                 bcm_pbmp_t mask)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X," FIELD_PBMP_FORMAT "," FIELD_PBMP_FORMAT
                          ")\n"),
                unit,
                entry,
                FIELD_PBMP_SHOW(data),
                FIELD_PBMP_SHOW(mask)));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_qualify_InPorts(unitData,
                                                           entry,
                                                           data,
                                                           mask);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X," FIELD_PBMP_FORMAT "," FIELD_PBMP_FORMAT
                          ") = %d (%s)\n"),
                unit,
                entry,
                FIELD_PBMP_SHOW(data),
                FIELD_PBMP_SHOW(mask),
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_qualify_OutPorts
 *   Purpose
 *      Set allowed egress ports for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_pbmp_t data = allowed ports
 *      (in) bcm_pbmp_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
int
bcm_fe2000_field_qualify_OutPorts(int unit,
                                  bcm_field_entry_t entry,
                                  bcm_pbmp_t data,
                                  bcm_pbmp_t mask)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X," FIELD_PBMP_FORMAT "," FIELD_PBMP_FORMAT
                          ")\n"),
                unit,
                entry,
                FIELD_PBMP_SHOW(data),
                FIELD_PBMP_SHOW(mask)));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_qualify_OutPorts(unitData,
                                                            entry,
                                                            data,
                                                            mask);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X," FIELD_PBMP_FORMAT "," FIELD_PBMP_FORMAT
                          ") = %d (%s)\n"),
                unit,
                entry,
                FIELD_PBMP_SHOW(data),
                FIELD_PBMP_SHOW(mask),
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_InPort_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInPort
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_InPort_get(int unit,
                                    bcm_field_entry_t entry,
                                    bcm_port_t *data,
                                    bcm_port_t *mask)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X,*,*)\n"), unit, entry));

    /* make sure return pointers are valid */
    if ((!data) || (!mask)) {
        FIELD_ERR((FIELD_MSG1("NULL pointer for outbound argument\n")));
        return BCM_E_PARAM;
    }

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_qualify_InPort_get(unitData,
                                                              entry,
                                                              data,
                                                              mask);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X,&(%08X),&(%08X)) = %d (%s)\n"),
                unit,
                entry,
                *data,
                *mask,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_OutPort_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyOutPort
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_OutPort_get(int unit,
                                     bcm_field_entry_t entry,
                                     bcm_port_t *data,
                                     bcm_port_t *mask)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X,*,*)\n"), unit, entry));

    /* make sure return pointers are valid */
    if ((!data) || (!mask)) {
        FIELD_ERR((FIELD_MSG1("NULL pointer for outbound argument\n")));
        return BCM_E_PARAM;
    }

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_qualify_OutPort_get(unitData,
                                                               entry,
                                                               data,
                                                               mask);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X,&(%08X),&(%08X)) = %d (%s)\n"),
                unit,
                entry,
                *data,
                *mask,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_InPorts_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInPorts
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_InPorts_get(int unit,
                                     bcm_field_entry_t entry,
                                     bcm_pbmp_t *data,
                                     bcm_pbmp_t *mask)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X,*,*)\n"), unit, entry));

    /* make sure return pointers are valid */
    if ((!data) || (!mask)) {
        FIELD_ERR((FIELD_MSG1("NULL pointer for outbound argument\n")));
        return BCM_E_PARAM;
    }

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_qualify_InPorts_get(unitData,
                                                               entry,
                                                               data,
                                                               mask);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X,&(" FIELD_PBMP_FORMAT "),&("
                          FIELD_PBMP_FORMAT ")) = %d (%s)\n"),
                unit,
                entry,
                FIELD_PBMP_SHOW(*data),
                FIELD_PBMP_SHOW(*mask),
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_OutPorts_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyOutPorts
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_OutPorts_get(int unit,
                                      bcm_field_entry_t entry,
                                      bcm_pbmp_t *data,
                                      bcm_pbmp_t *mask)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X,*,*)\n"), unit, entry));

    /* make sure return pointers are valid */
    if ((!data) || (!mask)) {
        FIELD_ERR((FIELD_MSG1("NULL pointer for outbound argument\n")));
        return BCM_E_PARAM;
    }

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_qualify_OutPorts_get(unitData,
                                                                entry,
                                                                data,
                                                                mask);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X,&(" FIELD_PBMP_FORMAT "),&("
                          FIELD_PBMP_FORMAT ")) = %d (%s)\n"),
                unit,
                entry,
                FIELD_PBMP_SHOW(*data),
                FIELD_PBMP_SHOW(*mask),
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_qualify_OuterVlanId
 *   Purpose
 *      Set expected outer VLAN for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_vlan_t data = which VID (12 bits)
 *      (in) bcm_vlan_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be either zero or all ones, else BCM_E_PARAM, since SBX
 *      doesn't allow the outer VLAN ID to be masked (all are always
 *      significant if any are significant).
 */
int
bcm_fe2000_field_qualify_OuterVlanId(int unit,
                                     bcm_field_entry_t entry,
                                     bcm_vlan_t data,
                                     bcm_vlan_t mask)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X,%03X,%03X)\n"),
                unit,
                entry,
                data,
                mask));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_qualify_OuterVlanId(unitData,
                                                               entry,
                                                               data,
                                                               mask);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X,%03X,%03X) = %d (%s)\n"),
                unit,
                entry,
                data,
                mask,
                result,
                _SHR_ERRMSG(result)));
    return result;
}


/*
 *   Function
 *      bcm_fe2000_field_qualify_OuterVlanPri
 *   Purpose
 *      Set expected outer VLAN for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_vlan_t data = which Pri (3 bits)
 *      (in) bcm_vlan_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be either zero or all ones, else BCM_E_PARAM, since SBX
 *      doesn't allow the outer VLAN ID to be masked (all are always
 *      significant if any are significant).
 */
int
bcm_fe2000_field_qualify_OuterVlanPri(int unit,
                                      bcm_field_entry_t entry,
                                      uint8 data,
                                      uint8 mask)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X,%01X,%01X)\n"),
                unit,
                entry,
                data,
                mask));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_qualify_OuterVlanPri(unitData,
                                                                entry,
                                                                data,
                                                                mask);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X,%01X,%01X) = %d (%s)\n"),
                unit,
                entry,
                data,
                mask,
                result,
                _SHR_ERRMSG(result)));
    return result;
}


/*
 * Function:
 *      bcm_fe2000_field_qualify_OuterVlanCfi
 * Purpose:
 *       Set match criteria for bcmFieildQualifyOuterVlanCfi
 *                       qualifier in the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (IN) Qualifier match data.
 *      mask - (IN) Qualifier match data.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_OuterVlanCfi(
    int unit,
    bcm_field_entry_t entry,
    uint8 data,
    uint8 mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_InnerVlanId
 * Purpose:
 *       Set match criteria for bcmFieildQualifyInnerVlanId
 *                       qualifier in the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (IN) Qualifier match data.
 *      mask - (IN) Qualifier match data.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_InnerVlanId(
    int unit,
    bcm_field_entry_t entry,
    bcm_vlan_t data,
    bcm_vlan_t mask)
{
    return BCM_E_UNAVAIL;
}


/*
 * Function:
 *      bcm_fe2000_field_qualify_InnerVlanPri
 * Purpose:
 *       Set match criteria for bcmFieildQualifyInnerVlanPri
 *                       qualifier in the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (IN) Qualifier match data.
 *      mask - (IN) Qualifier match data.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_InnerVlanPri(
    int unit,
    bcm_field_entry_t entry,
    uint8 data,
    uint8 mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_InnerVlanCfi
 * Purpose:
 *       Set match criteria for bcmFieildQualifyInnerVlanCfi
 *                       qualifier in the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (IN) Qualifier match data.
 *      mask - (IN) Qualifier match data.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_InnerVlanCfi(
    int unit,
    bcm_field_entry_t entry,
    uint8 data,
    uint8 mask)
{
    return BCM_E_UNAVAIL;
}


/*
 * Function:
 *      bcm_fe2000_field_qualify_OuterVlanId_get
 * Purpose:
 *       Get match criteria for bcmFieildQualifyOuterVlanId
 *                       qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match data.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_OuterVlanId_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_vlan_t *data,
    bcm_vlan_t *mask)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X,%03X,%03X)\n"),
                unit,
                entry,
                *data,
                *mask));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_qualify_OuterVlanId_get(unitData,
                                                               entry,
                                                               data,
                                                               mask);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X,%03X,%03X) = %d (%s)\n"),
                unit,
                entry,
                *data,
                *mask,
                result,
                _SHR_ERRMSG(result)));
    return result;
}


/*
 * Function:
 *      bcm_fe2000_field_qualify_OuterVlanPri_get
 * Purpose:
 *       Get match criteria for bcmFieildQualifyOuterVlanPri
 *                       qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_OuterVlanPri_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X,%01X,%01X)\n"),
                unit,
                entry,
                *data,
                *mask));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_qualify_OuterVlanPri_get(unitData,
                                                                entry,
                                                                data,
                                                                mask);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X,%01X,%01X) = %d (%s)\n"),
                unit,
                entry,
                *data,
                *mask,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_OuterVlanCfi_get
 * Purpose:
 *       Get match criteria for bcmFieildQualifyOuterVlanCfi_get
 *                       qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_OuterVlanCfi_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_InnerVlanId_get
 * Purpose:
 *       Get match criteria for bcmFieildQualifyInnerVlanId
 *                       qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_InnerVlanId_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_vlan_t *data,
    bcm_vlan_t *mask)
{
    return BCM_E_UNAVAIL;
}


/*
 * Function:
 *      bcm_fe2000_field_qualify_InnerVlanPri_get
 * Purpose:
 *       Get match criteria for bcmFieildQualifyInnerVlanPri
 *                       qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_InnerVlanPri_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_InnerVlanCfi_get
 * Purpose:
 *       Get match criteria for bcmFieildQualifyInnerVlanCfi_get
 *                       qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_InnerVlanCfi_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 *   Function
 *      bcm_fe2000_field_qualify_OuterVlan
 *   Purpose
 *      Set expected outer VLAN for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_vlan_t data = which VLAN tag (16 bits)
 *      (in) bcm_vlan_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be all zeroes or ones for supported subfields.  While we
 *      accept a nonzero mask for CFI, it is ignored with a warning.
 *
 *      BCM API docs neglect to metion that this function should apply to the
 *      entire tag, rather than just the VID.
 */
int
bcm_fe2000_field_qualify_OuterVlan(int unit,
                                   bcm_field_entry_t entry,
                                   bcm_vlan_t data,
                                   bcm_vlan_t mask)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X,%04X,%04X)\n"),
                unit,
                entry,
                data,
                mask));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_qualify_OuterVlan(unitData,
                                                             entry,
                                                             data,
                                                             mask);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X,%04X,%04X) = %d (%s)\n"),
                unit,
                entry,
                data,
                mask,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_qualify_EtherType
 *   Purpose
 *      Set expected ethernet type for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) uint16 data = which ethertype
 *      (in) uint16 mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be either zero or all ones, else BCM_E_PARAM, since SBX
 *      doesn't allow the ethernet type to be masked (all are always
 *      significant if any are significant).
 */
int
bcm_fe2000_field_qualify_EtherType(int unit,
                                   bcm_field_entry_t entry,
                                   uint16 data,
                                   uint16 mask)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X,%04X,%04X)\n"),
                unit,
                entry,
                data,
                mask));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_qualify_EtherType(unitData,
                                                             entry,
                                                             data,
                                                             mask);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X,%04X,%04X) = %d (%s)\n"),
                unit,
                entry,
                data,
                mask,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_qualify_IpProtocol
 *   Purpose
 *      Set expected IPv4 protocol type type for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) uint16 data = which ethertype
 *      (in) uint16 mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be either zero or all ones, else BCM_E_PARAM, since SBX
 *      doesn't allow the ethernet type to be masked (all are always
 *      significant if any are significant).
 */
int
bcm_fe2000_field_qualify_IpProtocol(int unit,
                                    bcm_field_entry_t entry,
                                    uint8 data,
                                    uint8 mask)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X,%02X,%02X)\n"),
                unit,
                entry,
                data,
                mask));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_qualify_IpProtocol(unitData,
                                                              entry,
                                                              data,
                                                              mask);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X,%02X,%02X) = %d (%s)\n"),
                unit,
                entry,
                data,
                mask,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_qualify_SrcIp
 *   Purpose
 *      Set expected source IPv4 address for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_ip_t data = which source IPv4 address
 *      (in) bcm_ip_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be prefix style mask.
 */
int
bcm_fe2000_field_qualify_SrcIp(int unit,
                               bcm_field_entry_t entry,
                               bcm_ip_t data,
                               bcm_ip_t mask)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X,%08X,%08X)\n"),
                unit,
                entry,
                data,
                mask));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_qualify_SrcIp(unitData,
                                                         entry,
                                                         data,
                                                         mask);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X,%08X,%08X) = %d (%s)\n"),
                unit,
                entry,
                data,
                mask,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_qualify_DstIp
 *   Purpose
 *      Set expected destination IPv4 address for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_ip_t data = which destination IPv4 address
 *      (in) bcm_ip_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be prefix style mask.
 */
int
bcm_fe2000_field_qualify_DstIp(int unit,
                               bcm_field_entry_t entry,
                               bcm_ip_t data,
                               bcm_ip_t mask)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X,%08X,%08X)\n"),
                unit,
                entry,
                data,
                mask));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_qualify_DstIp(unitData,
                                                         entry,
                                                         data,
                                                         mask);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X,%08X,%08X) = %d (%s)\n"),
                unit,
                entry,
                data,
                mask,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_qualify_DSCP
 *   Purpose
 *      Set expected IPv4 DSCP for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) uint8 data = which DSCP
 *      (in) uint8 mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
int
bcm_fe2000_field_qualify_DSCP(int unit,
                              bcm_field_entry_t entry,
                              uint8 data,
                              uint8 mask)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X,%02X,%02X)\n"),
                unit,
                entry,
                data,
                mask));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_qualify_DSCP(unitData,
                                                        entry,
                                                        data,
                                                        mask);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X,%02X,%02X) = %d (%s)\n"),
                unit,
                entry,
                data,
                mask,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_qualify_Tos
 *   Purpose
 *      Set expected IPv4 ToS for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) uint8 data = which ToS
 *      (in) uint8 mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      API internals say this is same as DSCP above.
 */
int
bcm_fe2000_field_qualify_Tos(int unit,
                             bcm_field_entry_t entry,
                             uint8 data,
                             uint8 mask)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X,%02X,%02X)\n"),
                unit,
                entry,
                data,
                mask));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            
            result = bcm_fe2000_g2p3_field_qualify_DSCP(unitData,
                                                        entry,
                                                        data,
                                                        mask);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X,%02X,%02X) = %d (%s)\n"),
                unit,
                entry,
                data,
                mask,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_qualify_TcpControl
 *   Purpose
 *      Set expected TCP control flags for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) uint8 data = which TCP control bits
 *      (in) uint8 mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Also implies TCP as protocol.
 */
int
bcm_fe2000_field_qualify_TcpControl(int unit,
                                    bcm_field_entry_t entry,
                                    uint8 data,
                                    uint8 mask)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X,%02X,%02X)\n"),
                unit,
                entry,
                data,
                mask));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_qualify_TcpControl(unitData,
                                                              entry,
                                                              data,
                                                              mask);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X,%02X,%02X) = %d (%s)\n"),
                unit,
                entry,
                data,
                mask,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_qualify_RangeCheck
 *   Purpose
 *      Set expected TCP/UDP port range for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_field_range_t range = which ethertype
 *      (in) int invert = whether the range match is to be inverted
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      The range that is specified is read only during this call; it will not
 *      be read later if that range changes; another call to this function will
 *      be required should the range change and the update need to apply.
 *      The invert flag is not supported.
 *      This can't use the helper functions because it is setting a more
 *      complex set of fields under a more complex set of conditions.
 */
int
bcm_fe2000_field_qualify_RangeCheck(int unit,
                                    bcm_field_entry_t entry,
                                    bcm_field_range_t range,
                                    int invert)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X,%08X,%s)\n"),
                unit,
                entry,
                range,
                invert?"TRUE":"FALSE"));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_qualify_RangeCheck(unitData,
                                                              entry,
                                                              range,
                                                              invert);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X,%08X,%s) = %d (%s)\n"),
                unit,
                entry,
                range,
                invert?"TRUE":"FALSE",
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_qualify_SrcMac
 *   Purpose
 *      Set expected source MAC address for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_mac_t data = which source MAC address
 *      (in) bcm_mac_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be either zero or all ones, else BCM_E_PARAM, since SBX
 *      doesn't allow the source MAC address to be masked (all are always
 *      significant if any are significant).
 */
int
bcm_fe2000_field_qualify_SrcMac(int unit,
                                bcm_field_entry_t entry,
                                bcm_mac_t data,
                                bcm_mac_t mask)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X," FIELD_MACA_FORMAT "," FIELD_MACA_FORMAT
                          ")\n"),
                unit,
                entry,
                FIELD_MACA_SHOW(data),
                FIELD_MACA_SHOW(mask)));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_qualify_SrcMac(unitData,
                                                          entry,
                                                          data,
                                                          mask);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X," FIELD_MACA_FORMAT "," FIELD_MACA_FORMAT
                          ") = %d (%s)\n"),
                unit,
                entry,
                FIELD_MACA_SHOW(data),
                FIELD_MACA_SHOW(mask),
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_qualify_DstMac
 *   Purpose
 *      Set expected destination MAC address for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_mac_t data = which destination MAC address
 *      (in) bcm_mac_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be either zero or all ones, else BCM_E_PARAM, since SBX
 *      doesn't allow the destination MAC address to be masked (all are always
 *      significant if any are significant).
 */
int
bcm_fe2000_field_qualify_DstMac(int unit,
                                bcm_field_entry_t entry,
                                bcm_mac_t data,
                                bcm_mac_t mask)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X," FIELD_MACA_FORMAT "," FIELD_MACA_FORMAT
                          ")\n"),
                unit,
                entry,
                FIELD_MACA_SHOW(data),
                FIELD_MACA_SHOW(mask)));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_qualify_DstMac(unitData,
                                                          entry,
                                                          data,
                                                          mask);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X," FIELD_MACA_FORMAT "," FIELD_MACA_FORMAT
                          ") = %d (%s)\n"),
                unit,
                entry,
                FIELD_MACA_SHOW(data),
                FIELD_MACA_SHOW(mask),
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_qualify_Llc
 *   Purpose
 *      Set expected LLC header information for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_field_llc_header_t data = which LLC header information
 *      (in) bcm_field_llc_header_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Masks for data.dsap and data.ssap must be either zero or all ones, and
 *      mask for data.control must be all zeroes, else BCM_E_PARAM, since SBX
 *      doesn't allow other than all-or-nothing for dsap and ssap and doesn't
 *      appear to provide control field.
 */
int
bcm_fe2000_field_qualify_Llc(int unit,
                             bcm_field_entry_t entry,
                             bcm_field_llc_header_t data,
                             bcm_field_llc_header_t mask)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X," FIELD_LLC_F0RMAT "," FIELD_LLC_F0RMAT
                          ")\n"),
                unit,
                entry,
                FIELD_LLC_SHOW(data),
                FIELD_LLC_SHOW(mask)));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X," FIELD_LLC_F0RMAT "," FIELD_LLC_F0RMAT
                          ") = %d (%s)\n"),
                unit,
                entry,
                FIELD_LLC_SHOW(data),
                FIELD_LLC_SHOW(mask),
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_action_add
 *   Purpose
 *      Add a specific action to a specific entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_field_action_t action = the action to add
 *      (in) uint32 param0 = action parameter 0 (some actions)
 *      (in) uint32 param1 = action parameter 1 (some actions)
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      No hardware changes; only software state.
 */
int
bcm_fe2000_field_action_add(int unit,
                            bcm_field_entry_t entry,
                            bcm_field_action_t action,
                            uint32 param0,
                            uint32 param1)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X,%d,%08X,%08X)\n"),
                unit,
                entry,
                action,
                param0,
                param1));

    if ((0 > action) || (bcmFieldActionCount <= action)) {
        FIELD_ERR((FIELD_MSG1("invalid action %d\n"), action));
        return BCM_E_PARAM;
    }

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_action_add(unitData,
                                                      entry,
                                                      action,
                                                      param0,
                                                      param1);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X,%d(%s),%08X,%08X) = %d (%s)\n"),
                unit,
                entry,
                action,
                _sbx_fe2000_field_action_name[action],
                param0,
                param1,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_action_get
 *   Purpose
 *      Get a specific action from a specific entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_field_action_t action = the action to get
 *      (out) uint32 *param0 = action parameter 0 (some actions)
 *      (out) uint32 *param1 = action parameter 1 (some actions)
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      No hardware changes; only software state.
 */
int
bcm_fe2000_field_action_get(int unit,
                            bcm_field_entry_t entry,
                            bcm_field_action_t action,
                            uint32 *param0,
                            uint32 *param1)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X,%d,*,*)\n"),
                unit,
                entry,
                action));

    if ((0 > action) || (bcmFieldActionCount <= action)) {
        FIELD_ERR((FIELD_MSG1("invalid action %d\n"), action));
        return BCM_E_PARAM;
    }
    if ((!param0) || (!param1)) {
        FIELD_ERR((FIELD_MSG1("NULL pointer for param0 or param1\n")));
        return BCM_E_PARAM;
    }

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_action_get(unitData,
                                                      entry,
                                                      action,
                                                      param0,
                                                      param1);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X,%d(%s),&(%08X),&(%08X)) = %d (%s)\n"),
                unit,
                entry,
                action,
                _sbx_fe2000_field_action_name[action],
                *param0,
                *param1,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_action_remove
 *   Purpose
 *      Remove a specific action from a specific entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_field_action_t action = the action to remove
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      No hardware changes; only software state.
 */
int
bcm_fe2000_field_action_remove(int unit,
                               bcm_field_entry_t entry,
                               bcm_field_action_t action)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X,%d)\n"),
                unit,
                entry,
                action));

    if ((0 > action) || (bcmFieldActionCount <= action)) {
        FIELD_ERR((FIELD_MSG1("invalid action %d\n"), action));
        return BCM_E_PARAM;
    }

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_action_remove(unitData,
                                                         entry,
                                                         action);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X,%d(%s)) = %d (%s)\n"),
                unit,
                entry,
                action,
                _sbx_fe2000_field_action_name[action],
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_action_remove_all
 *   Purpose
 *      Remove all actions from a specific entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      No hardware changes; only software state.
 */
int
bcm_fe2000_field_action_remove_all(int unit,
                                   bcm_field_entry_t entry)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X)\n"),
                unit,
                entry));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_action_remove_all(unitData, entry);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X) = %d (%s)\n"),
                unit,
                entry,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_counter_create
 *   Purpose
 *      'Create' a counter for the specified entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
int
bcm_fe2000_field_counter_create(int unit,
                                bcm_field_entry_t entry)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X)\n"),
                unit,
                entry));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_counter_create(unitData, entry);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X) = %d (%s)\n"),
                unit,
                entry,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_counter_share
 *   Purpose
 *      'Share' a counter between src_entry and dst_entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t src_entry = source entry ID
 *      (in) bcm_field_entry_t dst_entry = destination entry ID
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
int
bcm_fe2000_field_counter_share(int unit,
                               bcm_field_entry_t src_entry,
                               bcm_field_entry_t dst_entry)
{
    return BCM_E_UNAVAIL;
}

/*
 *   Function
 *      bcm_fe2000_field_counter_destroy
 *   Purpose
 *      Remove the entry's counter
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      If an entry is not sharing a counter, this merely turns off the counter
 *      for that entry.  If the entry is sharing a counter, it removes that
 *      entry from the sharing list and then disables the counter for that
 *      entry (so the end result is the specified entry has no counter but any
 *      other entries that shared with it are left alone).
 */
int
bcm_fe2000_field_counter_destroy(int unit,
                                 bcm_field_entry_t entry)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X)\n"),
                unit,
                entry));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_counter_destroy(unitData, entry);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X) = %d (%s)\n"),
                unit,
                entry,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_counter_set
 *   Purpose
 *      Set the specified counter to a value
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) int counter_num = which counter (perhaps frame or byte?)
 *      (in) uint64 val = new value for counter
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
int
bcm_fe2000_field_counter_set(int unit,
                             bcm_field_entry_t entry,
                             int counter_num,
                             uint64 val)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X,%d,%016llX)\n"),
                unit,
                entry,
                counter_num,
                val));

    if ((0 > counter_num) || (1 < counter_num)) {
        FIELD_ERR((FIELD_MSG1("counter number %d is not supported\n"),
                   counter_num));
        return BCM_E_PARAM;
    }

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_counter_set(unitData,
                                                       entry,
                                                       counter_num,
                                                       val);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X,%d,%016llX) = %d (%s)\n"),
                unit,
                entry,
                counter_num,
                val,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_counter_set32
 *   Purpose
 *      Set the specified counter to a 32 bit value
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) int counter_num = which counter (perhaps frame or byte?)
 *      (in) uint32 val = new value for counter
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
int
bcm_fe2000_field_counter_set32(int unit,
                               bcm_field_entry_t entry,
                               int counter_num,
                               uint32 val)
{
    /* just let the compiler coerce the number and call the 64b version */
    return bcm_fe2000_field_counter_set(unit, entry, counter_num, val);
}

/*
 *   Function
 *      bcm_fe2000_field_counter_get
 *   Purpose
 *      Get the specified counter's value
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) int counter_num = which counter (perhaps frame or byte?)
 *      (out) uint64 *val = where to put the counter's value
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Overflow when accumulating values is handled by keeping the maximum
 *      possible value.
 */
int
bcm_fe2000_field_counter_get(int unit,
                             bcm_field_entry_t entry,
                             int counter_num,
                             uint64 *valp)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X,%d,*)\n"),
                unit,
                entry,
                counter_num));

    if (!valp) {
        FIELD_ERR((FIELD_MSG1("NULL pointer for value\n")));
        return BCM_E_PARAM;
    }
    if ((0 > counter_num) || (1 < counter_num)) {
        FIELD_ERR((FIELD_MSG1("counter number %d is not supported\n"),
                   counter_num));
        return BCM_E_PARAM;
    }

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_counter_get(unitData,
                                                       entry,
                                                       counter_num,
                                                       valp);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X,%d,&(%016llX)) = %d (%s)\n"),
                unit,
                entry,
                counter_num,
                *valp,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_counter_get32
 *   Purpose
 *      Get the specified counter's low 32 bits value
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) int counter_num = which counter (perhaps frame or byte?)
 *      (out) uint32 *val = where to put the counter's value
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Overflow when accumulating values is handled by keeping the maximum
 *      possible value.
 */
int
bcm_fe2000_field_counter_get32(int unit,
                               bcm_field_entry_t entry,
                               int counter_num,
                               uint32 *valp)
{
    int                     result;             /* result for caller */
    uint64                  temp;               /* accumulated frames */

    /* get the function above to do the work */
    result = bcm_fe2000_field_counter_get(unit, entry, counter_num, &temp);

    /* now strip the entry down to what we can return from here */
    if (BCM_E_NONE == result) {
        if (temp <= 0xFFFFFFFFul) {
            /* the amount to return fits */
            *valp = (temp & 0xFFFFFFFFul);
        } else { /* if (temp <= 0xFFFFFFFFul) */
            /* the amount to return does not fit */
            FIELD_WARN((FIELD_MSG1("returned value saturated 32b number\n")));
            *valp = 0xFFFFFFFFul;
        } /* if (temp <= 0xFFFFFFFFul) */
    } /* if (BCM_E_NONE == result) */

    FIELD_VERB((FIELD_MSG("(%d,%08X,%d,&(%08X)) = %d (%s)\n"),
                unit,
                entry,
                counter_num,
                *valp,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_entry_policer_attach
 *   Purpose
 *      Attach a policer to a specified entry, at the given heirarchical level
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) int level = level (for heirarchical policing)
 *      (in) bcm_policer_t policer = which policer to attach to the entry
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Assumes policer 0 = no policer and does not allow it to be set.
 *      Policers must be managed by caller.
 */
int
bcm_fe2000_field_entry_policer_attach(int unit,
                                      bcm_field_entry_t entry_id,
                                      int level,
                                      bcm_policer_t policer_id)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X,%d,%08X)\n"),
                unit,
                entry_id,
                level,
                policer_id));

    if (!policer_id) {
        /* can't set policer zero (use clear instead) */
        FIELD_ERR((FIELD_MSG1("can't set policer 0 (no policer)\n")));
        return BCM_E_PARAM;
    }

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_entry_policer_attach(unitData,
                                                                entry_id,
                                                                level,
                                                                policer_id);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X,%d,%08X) = %d (%s)\n"),
                unit,
                entry_id,
                level,
                policer_id,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_entry_policer_detach
 *   Purpose
 *      Remove the policer used by the specified entry at the given level
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) int level = level (for heirarchical policing)
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Assumes policer 0 = no policer and calls that 'empty'.
 */
int
bcm_fe2000_field_entry_policer_detach(int unit,
                                      bcm_field_entry_t entry_id,
                                      int level)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X,%d)\n"),
                unit,
                entry_id,
                level));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_entry_policer_detach(unitData,
                                                                entry_id,
                                                                level);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X,%d) = %d (%s)\n"),
                unit,
                entry_id,
                level,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_field_entry_policer_detach_all
 *   Purpose
 *      Remove all policers used by the specified entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Does not return an error if a level has no policer.
 *      Policers must be managed by caller.
 */
int
bcm_fe2000_field_entry_policer_detach_all(int unit,
                                          bcm_field_entry_t entry_id)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X)\n"),
                unit,
                entry_id));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_entry_policer_detach_all(unitData,
                                                                    entry_id);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X) = %d (%s)\n"),
                unit,
                entry_id,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_policer_get
 *   Purpose
 *      Get the policer used by the specified entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (out) bcm_policer_t *policer = where to put the policer ID
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Assumes policer 0 = no policer and calls that 'empty'.
 *      Policers must be managed by caller.
 */
int
bcm_fe2000_field_entry_policer_get(int unit,
                                   bcm_field_entry_t entry_id,
                                   int level,
                                   bcm_policer_t *policer_id)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X,%d,*)\n"),
                unit,
                entry_id,
                level));

    if (!policer_id) {
        FIELD_ERR((FIELD_MSG1("NULL pointer to policer ID\n")));
        return BCM_E_PARAM;
    }

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_entry_policer_get(unitData,
                                                             entry_id,
                                                             level,
                                                             policer_id);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X,%d,&(%08X)) = %d (%s)\n"),
                unit,
                entry_id,
                level,
                *policer_id,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_show
 *   Purpose
 *      Dump all field information for the unit
 *   Parameters
 *      (in) int unit = the unit number
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
int
bcm_fe2000_field_show(int unit,
                      const char *pfx)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_EVERB((FIELD_MSG("(%d,*)\n"), unit));

    if (!pfx) {
        pfx = "";
    }

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_show(unitData, pfx);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_EVERB((FIELD_MSG("(%d,\"%s\") = %d (%s)\n"),
                unit,
                pfx,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_entry_dump
 *   Purpose
 *      Dump information about the specified entry to debug output
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
int
bcm_fe2000_field_entry_dump(int unit,
                            bcm_field_entry_t entry)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_EVERB((FIELD_MSG("(%d,%08X)\n"), unit, entry));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_entry_dump(unitData, entry);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_EVERB((FIELD_MSG("(%d,%08X) = %d (%s)\n"),
                unit,
                entry,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_fe2000_field_group_dump
 *   Purpose
 *      Dump information about the specified group to debug output
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_group_t group = the group ID
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
int
bcm_fe2000_field_group_dump(int unit,
                            bcm_field_group_t group)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_EVERB((FIELD_MSG("(%d,%08X)\n"), unit, group));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_group_dump(unitData, group);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_EVERB((FIELD_MSG("(%d,%08X) = %d (%s)\n"),
                unit,
                group,
                result,
                _SHR_ERRMSG(result)));
    return result;
}









/******************************************************************************
 *
 *  Unimplemented API features
 */

/*
 *   Function
 *      bcm_fe2000_field_control_get
 *   Purpose
 *      Get the value of a unit-wide field control.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_control_t control = the control to be read
 *      (out) uint32 *state = pointer to where to put the state
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Support is not planned for SBX.
 */
int
bcm_fe2000_field_control_get(int unit,
                             bcm_field_control_t control,
                             uint32 *state)
{
    return BCM_E_UNAVAIL;
}

/*
 *   Function
 *      bcm_fe2000_field_control_set
 *   Purpose
 *      Set the status of the field APIs.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_control_t control = the control to be written
 *      (in) uint32 *state = new state
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Support is not planned for SBX.
 */
int
bcm_fe2000_field_control_set(int unit,
                             bcm_field_control_t control,
                             uint32 state)
{
    return BCM_E_UNAVAIL;
}

/*
 *   Function
 *      bcm_fe2000_field_group_create_mode
 *   Purpose
 *      Create a new group spanning the specified width, that has the specified
 *      qualifier set and priority.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_qset_t qset = the qualifier set
 *      (in) int pri = the priority
 *      (in) bcm_mode_t mode = the mode (width) of the group
 *      (out) bcm_field_group_t *group = where to put the group ID
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Allocates first available group ID.
 *      Can not specify a priority already taken by an existing group.
 *      Can not specify a qualifier that another group in the same stage has.
 *      If no stage qualifier, it is assumed to be bcmFieldQualifyStageIngress.
 *      Not supported on SBX?
 */
int
bcm_fe2000_field_group_create_mode(int unit,
                                   bcm_field_qset_t qset,
                                   int pri,
                                   bcm_field_group_mode_t mode,
                                   bcm_field_group_t *group)
{
    return BCM_E_UNAVAIL;
}

/*
 *   Function
 *      bcm_fe2000_field_group_create_mode_id
 *   Purpose
 *      Create a new group using the specified ID, spanning the specified
 *      width, that has the specified qualifier set and priority.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_qset_t qset = the qualifier set
 *      (in) int pri = the priority
 *      (in) bcm_mode_t mode = the mode (width) of the group
 *      (in) bcm_field_group_t group = which group ID to use
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Allocates first available group ID.
 *      Can not specify a priority already taken by an existing group.
 *      Can not specify a qualifier that another group in the same stage has.
 *      If no stage qualifier, it is assumed to be bcmFieldQualifyStageIngress.
 *      Not supported on SBX?
 */
int
bcm_fe2000_field_group_create_mode_id(int unit,
                                      bcm_field_qset_t qset,
                                      int pri,
                                      bcm_field_group_mode_t mode,
                                      bcm_field_group_t group)
{
    return BCM_E_UNAVAIL;
}

/*
 *   Function
 *      bcm_fe2000_field_group_ports_create_mode
 *   Purpose
 *      Create a new group spanning the specified width, that has the specified
 *      qualifier set and priority, and affects only the specified ports.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_pbmp_t port = the ports
 *      (in) bcm_field_qset_t qset = the qualifier set
 *      (in) int pri = the priority
 *      (in) bcm_mode_t mode = the mode (width) of the group
 *      (out) bcm_field_group_t *group = where to put the group ID
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Allocates first available group ID.
 *      Can not specify a priority already taken by an existing group.
 *      Can not specify a qualifier that another group in the same stage has.
 *      If no stage qualifier, it is assumed to be bcmFieldQualifyStageIngress.
 *      Not supported on SBX?
 */
int
bcm_fe2000_field_group_ports_create_mode(int unit,
                                         bcm_pbmp_t pbmp,
                                         bcm_field_qset_t qset,
                                         int pri,
                                         bcm_field_group_mode_t mode,
                                         bcm_field_group_t *group)
{
    return BCM_E_UNAVAIL;
}

/*
 *   Function
 *      bcm_fe2000_field_group_ports_create_mode_id
 *   Purpose
 *      Create a new group using the specified ID, spanning the specified
 *      width, that has the specified qualifier set and priority, and affects
 *      only the specified ports.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_pbmp_t port = the ports
 *      (in) bcm_field_qset_t qset = the qualifier set
 *      (in) int pri = the priority
 *      (in) bcm_mode_t mode = the mode (width) of the group
 *      (in) bcm_field_group_t group = which group ID to use
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Allocates first available group ID.
 *      Can not specify a priority already taken by an existing group.
 *      Can not specify a qualifier that another group in the same stage has.
 *      If no stage qualifier, it is assumed to be bcmFieldQualifyStageIngress.
 *      Not supported on SBX?
 */
int
bcm_fe2000_field_group_ports_create_mode_id(int unit,
                                            bcm_pbmp_t pbmp,
                                            bcm_field_qset_t qset,
                                            int pri,
                                            bcm_field_group_mode_t mode,
                                            bcm_field_group_t group)
{
    return BCM_E_UNAVAIL;
}

/*
 *   Function
 *      bcm_fe2000_field_group_mode_get
 *   Purpose
 *      Get the mode (width) of a group.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_group_t group = which group ID to use
 *      (out) bcm_mode_t *mode = the mode (width) of the group
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Not supported on SBX?
 */
int
bcm_fe2000_field_group_mode_get(int unit,
                                bcm_field_group_t group,
                                bcm_field_group_mode_t *mode)
{
    return BCM_E_UNAVAIL;
}

/*
 *   Function
 *      bcm_fe2000_field_group_enable_set
 *   Purpose
 *      This turns a group on or off.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_group_t group = which group ID to use
 *      (in) int enable = new enable state
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Not supported on SBX?
 */
int
bcm_fe2000_field_group_enable_set(int unit,
                                  bcm_field_group_t group,
                                  int enable)
{
    return BCM_E_UNAVAIL;
}

/*
 *   Function
 *      bcm_fe2000_field_group_enable_get
 *   Purpose
 *      This gets whether a group is on or off.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_group_t group = which group ID to use
 *      (in) int *enable = where to put current enable state
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Not supported on SBX?
 */
int
bcm_fe2000_field_group_enable_get(int unit,
                                  bcm_field_group_t group,
                                  int *enable)
{
    return BCM_E_UNAVAIL;
}

/*
 *   Function
 *      bcm_fe2000_field_range_group_create
 *   Purpose
 *      Create a descriptor for a range of L4 (specifically, TCP or UDP over
 *      IP) ports.
 *   Parameters
 *      (in) int unit = the unit number
 *      (out) bcm_field_range_t *range = where to put the assigned range ID
 *      (in) uint32 flags = flags for the range
 *      (in) bcm_l4_port_t min = low port number for the range
 *      (in) bcm_l4_port_t max = high port number for the range
 *      (in) bcm_if_group_t group = L3 interface group for this range
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Range ID is always nonzero.
 *      Not supported on SBX?
 */
int
bcm_fe2000_field_range_group_create(int unit,
                                    bcm_field_range_t *range,
                                    uint32 flags,
                                    bcm_l4_port_t min,
                                    bcm_l4_port_t max,
                                    bcm_if_group_t group)
{
    return BCM_E_UNAVAIL;
}

/*
 *   Function
 *      bcm_fe2000_field_range_group_create_id
 *   Purpose
 *      Create a descriptor for a range of L4 (specifically, TCP or UDP over
 *      IP) ports.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_range_t range = the range ID to use
 *      (in) uint32 flags = flags for the range
 *      (in) bcm_l4_port_t min = low port number for the range
 *      (in) bcm_l4_port_t max = high port number for the range
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Range ID is always nonzero.
 *      Not supported on SBX?
 */
int
bcm_fe2000_field_range_group_create_id(int unit,
                                       bcm_field_range_t range,
                                       uint32 flags,
                                       bcm_l4_port_t min,
                                       bcm_l4_port_t max,
                                       bcm_if_group_t group)
{
    return BCM_E_UNAVAIL;
}

/* User-Defined Fields */

int
bcm_fe2000_field_udf_spec_set(int unit,
                              bcm_field_udf_spec_t *udf_spec,
                              uint32 flags,
                              uint32 offset)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_udf_spec_get(int unit,
                              bcm_field_udf_spec_t *udf_spec,
                              uint32 *flags,
                              uint32 *offset)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_udf_create(int unit,
                            bcm_field_udf_spec_t *udf_spec,
                            bcm_field_udf_t *udf_id)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_udf_create_id(int unit,
                               bcm_field_udf_spec_t *udf_spec,
                               bcm_field_udf_t udf_id)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_udf_ethertype_set(int unit,
                                   int index,
                                   bcm_port_frametype_t frametype,
                                   bcm_port_ethertype_t ethertype)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_udf_ethertype_get(int unit,
                                   int index,
                                   bcm_port_frametype_t *frametype,
                                   bcm_port_ethertype_t *ethertype)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_udf_ipprotocol_set(int unit,
                                    int index,
                                    uint32 flags,
                                    uint8 proto)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_udf_ipprotocol_get(int unit,
                                    int index,
                                    uint32 *flags,
                                    uint8 *proto)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_udf_get(int unit,
                         bcm_field_udf_spec_t *udf_spec,
                         bcm_field_udf_t udf_id)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_udf_destroy(int unit,
                             bcm_field_udf_t udf_id)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qset_add_udf(int unit,
                              bcm_field_qset_t *qset,
                              bcm_field_udf_t udf_id)
{
    return BCM_E_UNAVAIL;
}

/*
 *   Function
 *      bcm_fe2000_field_resync
 *   Purpose
 *      Purge hardware of all field entries not replayed after 'warm boot'.
 *   Parameters
 *      (in) int unit = the unit number
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Not supported on SBX?
 */
int
bcm_fe2000_field_resync(int unit)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_data_qualifier_create
 * Purpose:
 *      Create an data/offset based qualifier.
 * Parameters:
 *      unit           - (IN) bcm device.
 *      data_qualifier - (IN) Qualifier descriptor:
 *                           such as packet type, byte offset, etc.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_fe2000_field_data_qualifier_create(int unit,
                                 bcm_field_data_qualifier_t *data_qualifier)
{
    return (BCM_E_UNAVAIL);
}


/*
 * Function:
 *      bcm_fe2000_field_data_qualifier_destroy
 * Purpose:
 *      Destroy an data/offset based qualifier.
 * Parameters:
 *      unit     - (IN)  bcm device.
 *      qual_id  - (IN)  Data qualifier id.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_fe2000_field_data_qualifier_destroy(int unit, int qual_id)
{
    return (BCM_E_UNAVAIL);
}

/*
 * Function:
 *      bcm_fe2000_field_data_qualifier_destroy_all
 * Purpose:
 *      Delete all data/offset based qualifiers.
 * Parameters:
 *      unit          - (IN)  bcm device.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_fe2000_field_data_qualifier_destroy_all(int unit)
{
    return (BCM_E_UNAVAIL);
}

/*
 * Function:
 *      bcm_fe2000_field_qset_data_qualifier_add
 * Purpose:
 *      Add field data qualifier to group qset.
 * Parameters:
 *      unit          - (IN) bcm device.
 *      qset          - (IN/OUT) Group qualifier set.
 *      qualifier_id  - (IN) Data qualifier id.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_fe2000_field_qset_data_qualifier_add(int unit, bcm_field_qset_t *qset,
                                      int qual_id)
{
    return (BCM_E_UNAVAIL);
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_data
 * Purpose:
 *      Set data/mask in the search engine for entry field data qualifier.
 * Parameters:
 *      unit          - (IN) bcm device.
 *      eid           - (IN) Entry id.
 *      qual_id       - (IN) Data qualifier id.
 *      data          - (IN) Match data.
 *      mask          - (IN) Match mask.
 *      length        - (IN) Length of data & mask arrays.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_fe2000_field_qualify_data(int unit, bcm_field_entry_t eid, int qual_id,
                           uint8 *data, uint8 *mask, uint16 length)
{
    return (BCM_E_UNAVAIL);
}

/*
 * Function:
 *      bcm_fe2000_field_data_qualifier_ethertype_add
 * Purpose:
 *      Add ethertype based offset to data qualifier object.
 * Parameters:
 *      unit       - (IN) bcm device.
 *      qual_id    - (IN) Data qualifier id.
 *      etype      - (IN) Ethertype based offset specification.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_fe2000_field_data_qualifier_ethertype_add(int unit,  int qual_id,
                                 bcm_field_data_ethertype_t *etype)
{
    return (BCM_E_UNAVAIL);
}

/*
 * Function:
 *      bcm_fe2000_field_data_qualifier_ethertype_delete
 * Purpose:
 *      Remove ethertype based offset from data qualifier object.
 * Parameters:
 *      unit       - (IN) bcm device.
 *      qual_id    - (IN) Data qualifier id.
 *      etype      - (IN) Ethertype based offset specification.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_fe2000_field_data_qualifier_ethertype_delete(int unit, int qual_id,
                                 bcm_field_data_ethertype_t *etype)
{
    return (BCM_E_UNAVAIL);
}

/*
 * Function:
 *      bcm_fe2000_field_data_qualifier_ip_protocol_add
 * Purpose:
 *      Add ip protocol based offset to data qualifier object.
 * Parameters:
 *      unit        - (IN) bcm device.
 *      qual_id     - (IN) Data qualifier id.
 *      ip_protocol - (IN) Ethertype based offset specification.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_fe2000_field_data_qualifier_ip_protocol_add(int unit, int qual_id,
                                 bcm_field_data_ip_protocol_t *ip_protocol)
{
    return (BCM_E_UNAVAIL);
}

/*
 * Function:
 *      bcm_fe2000_field_data_qualifier_ip_protocol_delete
 * Purpose:
 *      Remove ip protocol based offset from data qualifier object.
 * Parameters:
 *      unit        - (IN) bcm device.
 *      qual_id     - (IN) Data qualifier id.
 *      ip_protocol - (IN) Ethertype based offset specification.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_fe2000_field_data_qualifier_ip_protocol_delete(int unit, int qual_id,
                                 bcm_field_data_ip_protocol_t *ip_protocol)
{
    return (BCM_E_UNAVAIL);
}


/*
 * Function:
 *      bcm_fe2000_field_data_qualifier_packet_format_add
 * Purpose:
 *      Add packet format based offset to data qualifier object.
 * Parameters:
 *      unit          - (IN) bcm device.
 *      qual_id       - (IN) Data qualifier id.
 *      packet_format - (IN) Ethertype based offset specification.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_fe2000_field_data_qualifier_packet_format_add(int unit, int qual_id,
                                 bcm_field_data_packet_format_t *packet_format)
{
    return (BCM_E_UNAVAIL);
}

/*
 * Function:
 *      bcm_fe2000_field_data_qualifier_packet_format_delete
 * Purpose:
 *      Remove packet format based offset from data qualifier object.
 * Parameters:
 *      unit           - (IN) bcm device.
 *      qual_id        - (IN) Data qualifier id.
 *      packet_format  - (IN) Ethertype based offset specification.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_fe2000_field_data_qualifier_packet_format_delete(int unit, int qual_id,
                                 bcm_field_data_packet_format_t *packet_format)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_fe2000_field_qualify_Drop(int unit,
                              bcm_field_entry_t entry,
                              uint8 data,
                              uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_SrcModid(int unit,
                                  bcm_field_entry_t entry,
                                  bcm_module_t data,
                                  bcm_module_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_SrcPortTgid(int unit,
                                     bcm_field_entry_t entry,
                                     bcm_port_t data,
                                     bcm_port_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_SrcPort(int unit,
                                 bcm_field_entry_t entry,
                                 bcm_module_t data_modid,
                                 bcm_module_t mask_modid,
                                 bcm_port_t   data_port,
                                 bcm_port_t   mask_port)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_SrcTrunk(int unit,
                                  bcm_field_entry_t entry,
                                  bcm_trunk_t data,
                                  bcm_trunk_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_DstModid(int unit,
                                  bcm_field_entry_t entry,
                                  bcm_module_t data,
                                  bcm_module_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_DstPortTgid(int unit,
                                     bcm_field_entry_t entry,
                                     bcm_port_t data,
                                     bcm_port_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_DstPort(int unit,
                                 bcm_field_entry_t entry,
                                 bcm_module_t data_modid,
                                 bcm_module_t mask_modid,
                                 bcm_port_t   data_port,
                                 bcm_port_t   mask_port)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_DstTrunk(int unit,
                                  bcm_field_entry_t entry,
                                  bcm_trunk_t data,
                                  bcm_trunk_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_L4SrcPort(int unit,
                                   bcm_field_entry_t entry,
                                   bcm_l4_port_t data,
                                   bcm_l4_port_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_L4DstPort(int unit,
                                   bcm_field_entry_t entry,
                                   bcm_l4_port_t data,
                                   bcm_l4_port_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_InnerVlan(int unit,
                                   bcm_field_entry_t entry,
                                   bcm_vlan_t data,
                                   bcm_vlan_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_ExtensionHeaderSubCode(int unit,
                                                bcm_field_entry_t entry,
                                                uint8 i,uint8 j)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_ExtensionHeaderType(int unit,
                                             bcm_field_entry_t entry,
                                             uint8 i,uint8 j)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_ExtensionHeader2Type(int unit,
                                             bcm_field_entry_t entry,
                                             uint8 i,uint8 j)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_InnerSrcIp(int unit,
                                    bcm_field_entry_t entry,
                                    bcm_ip_t data,
                                    bcm_ip_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_InnerDstIp(int unit,
                                    bcm_field_entry_t entry,
                                    bcm_ip_t data,
                                    bcm_ip_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_InnerSrcIp6(int unit,
                                     bcm_field_entry_t entry,
                                     bcm_ip6_t data,
                                     bcm_ip6_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_InnerDstIp6(int unit,
                                     bcm_field_entry_t entry,
                                     bcm_ip6_t data,
                                     bcm_ip6_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_InnerSrcIp6High(int unit,
                                         bcm_field_entry_t entry,
                                         bcm_ip6_t data,
                                         bcm_ip6_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_InnerDstIp6High(int unit,
                                         bcm_field_entry_t entry,
                                         bcm_ip6_t data,
                                         bcm_ip6_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_InnerTos(int unit,
                                   bcm_field_entry_t entry,
                                   uint8 data,
                                   uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_InnerDSCP(int unit,
                                   bcm_field_entry_t entry,
                                   uint8 data,
                                   uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_InnerIpProtocol(int unit,
                                         bcm_field_entry_t entry,
                                         uint8 data,
                                         uint8 mask)
{
    return BCM_E_UNAVAIL;
}


int
bcm_fe2000_field_qualify_InnerIpFrag(int unit,
                                     bcm_field_entry_t entry,
                                     bcm_field_IpFrag_t frag_info)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_InnerTtl(int unit,
                                  bcm_field_entry_t entry,
                                  uint8 data,
                                  uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_EqualL4Port(int unit,
                                     bcm_field_entry_t entry,
                                     uint32 flag)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_LookupStatus(int unit,
                                      bcm_field_entry_t entry,
                                      uint32 data,
                                      uint32 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_DosAttack(int unit,
                                   bcm_field_entry_t entry,
                                   uint8 data,
                                   uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_IpmcStarGroupHit(int unit,
                                          bcm_field_entry_t entry,
                                          uint8 data,
                                          uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_MyStationHit(int unit,
                                      bcm_field_entry_t entry,
                                      uint8 data,
                                      uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_L2PayloadFirstEightBytes(int unit, bcm_field_entry_t entry,
                                                  uint32 data1, uint32 data2, 
                                                  uint32 mask1, uint32 mask2)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_fe2000_field_qualify_L3DestRouteHit(int unit,
                                        bcm_field_entry_t entry,
                                        uint8 data,
                                        uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_L3DestHostHit(int unit,
                                       bcm_field_entry_t entry,
                                       uint8 data,
                                       uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_L3SrcHostHit(int unit,
                                      bcm_field_entry_t entry,
                                      uint8 data,
                                      uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_L2CacheHit(int unit,
                                 bcm_field_entry_t entry,
                                 uint8 data,
                                 uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_L2StationMove(int unit,
                                    bcm_field_entry_t entry,
                                    uint8 data,
                                    uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_L2DestHit(int unit,
                                   bcm_field_entry_t entry,
                                   uint8 data,
                                   uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_L2SrcStatic(int unit,
                                     bcm_field_entry_t entry,
                                     uint8 data,
                                     uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_L2SrcHit(int unit,
                                  bcm_field_entry_t entry,
                                  uint8 data,
                                  uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_IngressStpState(int unit,
                                         bcm_field_entry_t entry,
                                         uint8 data,
                                         uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_ForwardingVlanValid(int unit,
                                             bcm_field_entry_t entry,
                                             uint8 data,
                                             uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_SrcVirtualPortValid(int unit,
                                             bcm_field_entry_t entry,
                                             uint8 data,
                                             uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_DstL3EgressNextHops(int unit,
                                             bcm_field_entry_t entry,
                                             uint32 data,
                                             uint32 mask)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_fe2000_field_qualify_VlanTranslationHit(int unit,
                                            bcm_field_entry_t entry,
                                            uint8 data,
                                            uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_IpInfo(int unit,
                                bcm_field_entry_t entry,
                                uint32 data,
                                uint32 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_PacketRes(int unit,
                                   bcm_field_entry_t entry,
                                   uint32 data,
                                   uint32 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_IpFlags(int unit,
                                 bcm_field_entry_t entry,
                                 uint8 data,
                                 uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_TcpSequenceZero(int unit,
                                         bcm_field_entry_t entry,
                                         uint32 flag)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_TcpHeaderSize(int unit,
                                       bcm_field_entry_t entry,
                                       uint8 data,
                                       uint8 mask)
{
    return BCM_E_UNAVAIL;
}


int bcm_fe2000_field_qualify_IpAuth(int unit,
                 bcm_field_entry_t entry ,
                 uint8 a,
                 uint8 b)
{
  return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_Ttl(int unit,
                             bcm_field_entry_t entry,
                             uint8 data,
                             uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_SrcIp6(int unit,
                                bcm_field_entry_t entry,
                                bcm_ip6_t data,
                                bcm_ip6_t mask)
{
    _FE2000_FIELD_COMMON_LOCALS;

    if (!SOC_IS_SBX_FE2KXT(unit)){
        return BCM_E_UNAVAIL;
    }

    FIELD_VERB((FIELD_MSG("(%d,%08X," FIELD_IPV6A_FORMAT "," FIELD_IPV6A_FORMAT
                          ")\n"),
                unit,
                entry,
                FIELD_IPV6A_SHOW(data),
                FIELD_IPV6A_SHOW(mask)));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_qualify_SrcIp6(unitData,
                                                          entry,
                                                          data,
                                                          mask);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X," FIELD_IPV6A_FORMAT "," FIELD_IPV6A_FORMAT
                          ") = %d (%s)\n"),
                unit,
                entry,
                FIELD_IPV6A_SHOW(data),
                FIELD_IPV6A_SHOW(mask),
                result,
                _SHR_ERRMSG(result)));

    return result;
}

int
bcm_fe2000_field_qualify_DstIp6(int unit,
                                bcm_field_entry_t entry,
                                bcm_ip6_t data,
                                bcm_ip6_t mask)
{
    _FE2000_FIELD_COMMON_LOCALS;

    if (!SOC_IS_SBX_FE2KXT(unit)){
        return BCM_E_UNAVAIL;
    }

    FIELD_VERB((FIELD_MSG("(%d,%08X," FIELD_IPV6A_FORMAT "," FIELD_IPV6A_FORMAT
                          ")\n"),
                unit,
                entry,
                FIELD_IPV6A_SHOW(data),
                FIELD_IPV6A_SHOW(mask)));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_qualify_DstIp6(unitData,
                                                          entry,
                                                          data,
                                                          mask);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X," FIELD_IPV6A_FORMAT "," FIELD_IPV6A_FORMAT
                          ") = %d (%s)\n"),
                unit,
                entry,
                FIELD_IPV6A_SHOW(data),
                FIELD_IPV6A_SHOW(mask),
                result,
                _SHR_ERRMSG(result)));

    return result;
}

int
bcm_fe2000_field_qualify_SrcIp6High(int unit,
                                    bcm_field_entry_t entry,
                                    bcm_ip6_t data,
                                    bcm_ip6_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_SrcIpEqualDstIp(int unit,
                                         bcm_field_entry_t entry,
                                         uint32 flag)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_DstIp6High(int unit,
                                    bcm_field_entry_t entry,
                                    bcm_ip6_t data,
                                    bcm_ip6_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_Ip6NextHeader(int unit,
                                       bcm_field_entry_t entry,
                                       uint8 data,
                                       uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_Ip6TrafficClass(int unit,
                                         bcm_field_entry_t entry,
                                         uint8 data,
                                         uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_Ip6FlowLabel(int unit,
                                      bcm_field_entry_t entry,
                                      uint32 data,
                                      uint32 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_Ip6HopLimit(int unit,
                                     bcm_field_entry_t entry,
                                     uint8 data,
                                     uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_PacketFormat(int unit,
                                      bcm_field_entry_t entry,
                                      uint32 data,
                                      uint32 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_SrcMacGroup(int unit,
                                     bcm_field_entry_t entry,
                                     uint32 data,
                                     uint32 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_ForwardingType(int unit, bcm_field_entry_t entry,
                                        bcm_field_ForwardingType_t data)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_fe2000_field_qualify_IpType(int unit,
                                bcm_field_entry_t entry,
                                bcm_field_IpType_t type)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_L2Format(int unit,
                                  bcm_field_entry_t entry,
                                  bcm_field_L2Format_t type)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_VlanFormat(int unit,
                                    bcm_field_entry_t entry,
                                    uint8 data,
                                    uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_MHOpcode(int unit,
                                  bcm_field_entry_t entry,
                                  uint8 data,
                                  uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_UserDefined(int unit,
                                     bcm_field_entry_t entry,
                                     bcm_field_udf_t udf_id,
                                     uint8 data[],
                                     uint8 mask[])
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_Decap(int unit,
                               bcm_field_entry_t entry,
                               bcm_field_decap_t decap)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_HiGig(int unit,
                               bcm_field_entry_t entry,
                               uint8 data,
                               uint8 mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_DstHiGig
 * Purpose:
 *      Qualify on HiGig destination packets.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (IN) Qualifier match data.
 *      mask - (IN) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_fe2000_field_qualify_DstHiGig(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 data, 
    uint8 mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_DstHiGig_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstHiGig
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_fe2000_field_qualify_DstHiGig_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

int
bcm_fe2000_field_qualify_Stage(int unit,
                               bcm_field_entry_t entry,
                               bcm_field_stage_t stage)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_L3IntfGroup(int unit,
                                     bcm_field_entry_t entry,
                                     bcm_if_group_t data,
                                     bcm_if_group_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_InterfaceClassL2(int unit, bcm_field_entry_t entry,
                                        uint32 data, uint32 mask)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_fe2000_field_qualify_InterfaceClassL3(int unit, bcm_field_entry_t entry,
                                        uint32 data, uint32 mask)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_fe2000_field_qualify_InterfaceClassPort(int unit, bcm_field_entry_t entry,
                                        uint32 data, uint32 mask)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_fe2000_field_qualify_SrcClassL2(int unit, bcm_field_entry_t entry,
                                  uint32 data, uint32 mask)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_fe2000_field_qualify_SrcClassL3(int unit, bcm_field_entry_t entry,
                                  uint32 data, uint32 mask)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_fe2000_field_qualify_SrcClassField(int unit, bcm_field_entry_t entry,
                                  uint32 data, uint32 mask)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_fe2000_field_qualify_DstClassL2(int unit, bcm_field_entry_t entry,
                                  uint32 data, uint32 mask)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_fe2000_field_qualify_DstClassL3(int unit, bcm_field_entry_t entry,
                                  uint32 data, uint32 mask)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_fe2000_field_qualify_DstClassField(int unit, bcm_field_entry_t entry,
                                  uint32 data, uint32 mask)
{
    return (BCM_E_UNAVAIL);
}


int
bcm_fe2000_field_qualify_IpProtocolCommon(int unit,
                                          bcm_field_entry_t entry,
                                          bcm_field_IpProtocolCommon_t protocol)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_Snap(int unit,
                              bcm_field_entry_t entry,
                              bcm_field_snap_header_t data,
                              bcm_field_snap_header_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_InnerTpid(int unit,
                                   bcm_field_entry_t entry,
                                   uint16 tpid)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_OuterTpid(int unit,
                                   bcm_field_entry_t entry,
                                   uint16 tpid)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_PortClass(int unit,
                                   bcm_field_entry_t entry,
                                   uint32 data,
                                   uint32 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_L3Routable(int unit,
                                    bcm_field_entry_t entry,
                                    uint8 data,
                                    uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_IpFrag(int unit,
                                bcm_field_entry_t entry,
                                bcm_field_IpFrag_t frag_info)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_LookupClass0(int unit,
                                      bcm_field_entry_t entry,
                                      uint32 data,
                                      uint32 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_Vrf(int unit,
                             bcm_field_entry_t entry,
                             uint32 data,
                             uint32 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_L3Ingress(int unit,
                             bcm_field_entry_t entry,
                             uint32 data,
                             uint32 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_L4Ports(int unit,
                                 bcm_field_entry_t entry,
                                 uint8 data,
                                 uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_MirrorCopy(int unit,
                                    bcm_field_entry_t entry,
                                    uint8 data,
                                    uint8 mask)
{
    return BCM_E_UNAVAIL;
}


int
bcm_fe2000_field_qualify_TunnelTerminated(int unit,
                                          bcm_field_entry_t entry,
                                          uint8 data,
                                          uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_MplsTerminated(int unit,
                                        bcm_field_entry_t entry,
                                        uint8 data,
                                        uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_action_ports_add(int unit,
                                  bcm_field_entry_t entry,
                                  bcm_field_action_t action,
                                  bcm_pbmp_t pbmp)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_action_ports_get(int unit,
                                  bcm_field_entry_t entry,
                                  bcm_field_action_t action,
                                  bcm_pbmp_t *pbmp)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_action_delete(int unit,
                               bcm_field_entry_t entry,
                               bcm_field_action_t action,
                               uint32 param0,
                               uint32 param1)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_meter_create(int unit,
                              bcm_field_entry_t entry)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_meter_share(int unit,
                             bcm_field_entry_t src_entry,
                             bcm_field_entry_t dst_entry)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_meter_destroy(int unit,
                               bcm_field_entry_t entry)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_meter_set(int unit,
                           bcm_field_entry_t entry,
                           int meter_num,
                           uint32 kbits_sec,
                           uint32 kbits_burst)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_meter_get(int unit,
                           bcm_field_entry_t entry,
                           int meter_num,
                           uint32 *kbits_sec,
                           uint32 *kbits_burst)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_group_compress(int unit, bcm_field_group_t group)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_fe2000_field_group_priority_set(int unit, bcm_field_group_t group,
                                 int priority)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_fe2000_field_group_priority_get(int unit, bcm_field_group_t group,
                                 int *priority)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_fe2000_field_qualify_BigIcmpCheck(int unit,
                                      bcm_field_entry_t entry,
                                      uint32 flag,
                                      uint32 size)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_IcmpTypeCode(int unit,
                                      bcm_field_entry_t entry,
                                      uint16 data,
                                      uint16 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_IgmpTypeMaxRespTime(int unit,
                                             bcm_field_entry_t entry,
                                             uint16 data,
                                             uint16 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_stat_multi_get(int unit,
                                int stat_id,
                                int nstat,
                                bcm_field_stat_t *stat_arr,
                                int nvalue,
                                uint64 *value_arr)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_stat_multi_get32(int unit,
                                  int stat_id,
                                  int nstat,
                                  bcm_field_stat_t *stat_arr,
                                  int nvalue,
                                  uint32 *value_arr)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_entry_stat_attach(int unit,
                                   bcm_field_entry_t entry,
                                   int stat_id)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_entry_stat_detach(int unit,
                                   bcm_field_entry_t entry,
                                   int stat_id)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_entry_stat_get(int unit,
                                bcm_field_entry_t entry,
                                int *stat_id)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_stat_config_get(int unit,
                                 int stat_id,
                                 int nstat,
                                 bcm_field_stat_t *stat_arr)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_stat_create(int unit,
                             bcm_field_group_t group,
                             int nstat,
                             bcm_field_stat_t *stat_arr,
                             int *stat_id)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_stat_create_id(int unit,
                             bcm_field_group_t group,
                             int nstat,
                             bcm_field_stat_t *stat_arr,
                             int stat_id)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_InnerL4DstPort(int unit, bcm_field_entry_t entry,
                                bcm_l4_port_t data, bcm_l4_port_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_InnerL4SrcPort(int unit, bcm_field_entry_t entry,
                                bcm_l4_port_t data, bcm_l4_port_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_InnerIpType(int unit, bcm_field_entry_t entry,
                             bcm_field_IpType_t type)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_InnerIpProtocolCommon(int unit, bcm_field_entry_t entry,
                                       bcm_field_IpProtocolCommon_t protocol)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_qualify_InnerIp6FlowLabel(int unit, bcm_field_entry_t entry,
                                   uint32 data, uint32 mask)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_fe2000_field_qualify_DstL3Egress(int unit,
                               bcm_field_entry_t entry,
                               bcm_if_t if_id)
{
    return BCM_E_UNAVAIL;
}


int
bcm_fe2000_field_qualify_DstMulticastGroup(int unit,
                                 bcm_field_entry_t entry,
                                 bcm_gport_t group)
{
    return BCM_E_UNAVAIL;
}


int
bcm_fe2000_field_qualify_SrcMplsGport(int unit,
                               bcm_field_entry_t entry,
                               bcm_gport_t mpls_port_id)
{
    return BCM_E_UNAVAIL;
}


int
bcm_fe2000_field_qualify_DstMplsGport(int unit,
                               bcm_field_entry_t entry,
                               bcm_gport_t mpls_port_id)
{
    return BCM_E_UNAVAIL;
}


int
bcm_fe2000_field_qualify_SrcMimGport(int unit,
                              bcm_field_entry_t entry,
                              bcm_gport_t mim_port_id)
{
    return BCM_E_UNAVAIL;
}


int
bcm_fe2000_field_qualify_DstMimGport(int unit,
                              bcm_field_entry_t entry,
                              bcm_gport_t mim_port_id)
{
    return BCM_E_UNAVAIL;
}



int
bcm_fe2000_field_qualify_SrcWlanGport(int unit,
                               bcm_field_entry_t entry,
                               bcm_gport_t wlan_port_id)
{
    return BCM_E_UNAVAIL;
}


int
bcm_fe2000_field_qualify_DstWlanGport(int unit,
                               bcm_field_entry_t entry,
                               bcm_gport_t wlan_port_id)
{
    return BCM_E_UNAVAIL;
}


int
bcm_fe2000_field_qualify_Loopback(int unit,
                               bcm_field_entry_t entry,
                               uint8 data,
                               uint8 mask)
{
    return BCM_E_UNAVAIL;
}



int
bcm_fe2000_field_qualify_LoopbackType(int unit,
                                   bcm_field_entry_t entry,
                                   bcm_field_LoopbackType_t loopback_type)
{
    return BCM_E_UNAVAIL;
}


int
bcm_fe2000_field_qualify_TunnelType(int unit,
                                 bcm_field_entry_t entry,
                                 bcm_field_TunnelType_t tunnel_type)
{
    return BCM_E_UNAVAIL;
}



/*
 * Function:
 *      bcm_fe2000_field_qualify_LoopbackType_get
 * Purpose:
 *      Get loopback type field qualification from a field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) Field entry id.
 *      loopback_type - (OUT) Loopback type.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_LoopbackType_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_field_LoopbackType_t *loopback_type)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_TunnelType_get
 * Purpose:
 *      Get tunnel type field qualification from a field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) Field entry id.
 *      tunnel_type - (OUT) Tunnel type.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_TunnelType_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_field_TunnelType_t *tunnel_type)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_DstL3Egress_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstL3Egress
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      if_id - (OUT) L3 forwarding object.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_DstL3Egress_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_if_t *if_id)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_DstMulticastGroup_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstMulticastGroup
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      group - (OUT) Multicast group id.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_DstMulticastGroup_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_gport_t *group)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_SrcMplsGport_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcMplsGport
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      mpls_port_id - (OUT) Mpls port id.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_SrcMplsGport_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_gport_t *mpls_port_id)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_DstMplsGport_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstMplsGport
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      mpls_port_id - (OUT) Mpls port id.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_DstMplsGport_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_gport_t *mpls_port_id)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_SrcMimGport_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcMimGport
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      mim_port_id - (OUT) Mim port id.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_SrcMimGport_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_gport_t *mim_port_id)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_DstMimGport_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstMimGport
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      mim_port_id - (OUT) Mim port id.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_DstMimGport_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_gport_t *mim_port_id)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_SrcWlanGport_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcWlanGport
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      wlan_port_id - (OUT) Wlan port id.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_SrcWlanGport_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_gport_t *wlan_port_id)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_DstWlanGport_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstWlanGport
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      wlan_port_id - (OUT) Wlan port id.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_DstWlanGport_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_gport_t *wlan_port_id)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_Loopback_get
 * Purpose:
 *      Get loopback field qualification from  a field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) Field entry id.
 *      data - (OUT) Data to qualify with.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_Loopback_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_Drop_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDrop
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_Drop_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_SrcModid_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcModid
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_SrcModid_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_module_t *data,
    bcm_module_t *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_SrcPortTgid_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcPortTgid
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_SrcPortTgid_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_port_t *data,
    bcm_port_t *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_SrcPort_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcPort
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data_modid - (OUT) Qualifier module id.
 *      mask_modid - (OUT) Qualifier module id mask.
 *      data_port - (OUT) Qualifier port id.
 *      mask_port - (OUT) Qualifier port id mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_SrcPort_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_module_t *data_modid,
    bcm_module_t *mask_modid,
    bcm_port_t *data_port,
    bcm_port_t *mask_port)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_SrcTrunk_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcTrunk
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_SrcTrunk_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_trunk_t *data,
    bcm_trunk_t *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_DstModid_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstModid
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_DstModid_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_module_t *data,
    bcm_module_t *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_DstPortTgid_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstPortTgid
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_DstPortTgid_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_port_t *data,
    bcm_port_t *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_DstPort_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstPort
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data_modid - (OUT) Qualifier module id.
 *      mask_modid - (OUT) Qualifier module id mask.
 *      data_port - (OUT) Qualifier port id.
 *      mask_port - (OUT) Qualifier port id mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_DstPort_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_module_t *data_modid,
    bcm_module_t *mask_modid,
    bcm_port_t *data_port,
    bcm_port_t *mask_port)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_DstTrunk_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstTrunk
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_DstTrunk_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_trunk_t *data,
    bcm_trunk_t *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_InnerL4SrcPort_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInnerL4SrcPort
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_InnerL4SrcPort_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_l4_port_t *data,
    bcm_l4_port_t *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_InnerL4DstPort_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInnerL4DstPort
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_InnerL4DstPort_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_l4_port_t *data,
    bcm_l4_port_t *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_L4SrcPort_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyL4SrcPort
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_L4SrcPort_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_l4_port_t *data,
    bcm_l4_port_t *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_L4DstPort_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyL4DstPort
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_L4DstPort_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_l4_port_t *data,
    bcm_l4_port_t *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_OuterVlan_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyOuterVlan
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_OuterVlan_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_vlan_t *data,
    bcm_vlan_t *mask)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X,%04X,%04X)\n"),
                unit,
                entry,
                *data,
                *mask));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_qualify_OuterVlan_get(unitData,
                                                             entry,
                                                             data,
                                                             mask);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X,%04X,%04X) = %d (%s)\n"),
                unit,
                entry,
                *data,
                *mask,
                result,
                _SHR_ERRMSG(result)));

    return result;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_InnerVlan_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInnerVlan
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_InnerVlan_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_vlan_t *data,
    bcm_vlan_t *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_EtherType_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyEtherType
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_EtherType_get(
    int unit,
    bcm_field_entry_t entry,
    uint16 *data,
    uint16 *mask)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X,%04X,%04X)\n"),
                unit,
                entry,
                *data,
                *mask));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_qualify_EtherType_get(unitData,
                                                             entry,
                                                             data,
                                                             mask);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X,%04X,%04X) = %d (%s)\n"),
                unit,
                entry,
                *data,
                *mask,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_EqualL4Port_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyEqualL4Port
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      flag - (OUT) Qualifier flags.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_EqualL4Port_get(
    int unit,
    bcm_field_entry_t entry,
    uint32 *flag)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_IpProtocol_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIpProtocol
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_IpProtocol_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X,%02X,%02X)\n"),
                unit,
                entry,
                *data,
                *mask));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_qualify_IpProtocol_get(unitData,
                                                              entry,
                                                              data,
                                                              mask);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X,%02X,%02X) = %d (%s)\n"),
                unit,
                entry,
                *data,
                *mask,
                result,
                _SHR_ERRMSG(result)));
    return result;
}


/*
 * Function:
 *      bcm_fe2000_field_qualify_IpInfo_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIpInfo
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_IpInfo_get(
    int unit,
    bcm_field_entry_t entry,
    uint32 *data,
    uint32 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_PacketRes_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyPacketRes
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_PacketRes_get(
    int unit,
    bcm_field_entry_t entry,
    uint32 *data,
    uint32 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_SrcIp_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcIp
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_SrcIp_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_ip_t *data,
    bcm_ip_t *mask)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X,%08X,%08X)\n"),
                unit,
                entry,
                *data,
                *mask));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_qualify_SrcIp_get(unitData,
                                                         entry,
                                                         data,
                                                         mask);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X,%08X,%08X) = %d (%s)\n"),
                unit,
                entry,
                *data,
                *mask,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_DstIp_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstIp
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_DstIp_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_ip_t *data,
    bcm_ip_t *mask)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X,%08X,%08X)\n"),
                unit,
                entry,
                *data,
                *mask));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_qualify_DstIp_get(unitData,
                                                         entry,
                                                         data,
                                                         mask);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X,%08X,%08X) = %d (%s)\n"),
                unit,
                entry,
                *data,
                *mask,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_DSCP_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDSCP
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_DSCP_get(int unit,
                              bcm_field_entry_t entry,
                              uint8 *data,
                              uint8 *mask)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X,%02X,%02X)\n"),
                unit,
                entry,
                *data,
                *mask));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_qualify_DSCP_get(unitData,
                                                        entry,
                                                        data,
                                                        mask);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X,%02X,%02X) = %d (%s)\n"),
                unit,
                entry,
                *data,
                *mask,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_Tos_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyTos
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_Tos_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_IpFlags_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIpFlags
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_IpFlags_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_TcpControl_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyTcpControl
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_TcpControl_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X,%02X,%02X)\n"),
                unit,
                entry,
                *data,
                *mask));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_qualify_TcpControl_get(unitData,
                                                              entry,
                                                              data,
                                                              mask);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X,%02X,%02X) = %d (%s)\n"),
                unit,
                entry,
                *data,
                *mask,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_TcpSequenceZero_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyTcpSequenceZero
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      flag - (OUT) Qualifier match flags.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_TcpSequenceZero_get(
    int unit,
    bcm_field_entry_t entry,
    uint32 *flag)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_TcpHeaderSize_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyTcpHeaderSize
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_TcpHeaderSize_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_Ttl_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyTtl
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_Ttl_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_RangeCheck_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyRangeCheck
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      max_count - (IN) Max entries to fill.
 *      range - (OUT) Range checkers array.
 *      invert - (OUT) Range checkers invert array.
 *      count - (OUT) Number of filled range checkers.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_RangeCheck_get(
    int unit,
    bcm_field_entry_t entry,
    int max_count,
    bcm_field_range_t *range,
    int *invert,
    int *count)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X,%08X,%08X,%08X,%s)\n"),
                unit,
                entry,
                max_count,
                *range,
                *invert?"TRUE":"FALSE",
                *count));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_qualify_RangeCheck_get(unitData,
                                                              entry,
                                                              max_count,
                                                              range,
                                                              invert,
                                                              count);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X,%08X,%s) = %d (%s)\n"),
                unit,
                entry,
                *range,
                *invert?"TRUE":"FALSE",
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_SrcIp6_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcIp6
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_SrcIp6_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_ip6_t *data,
    bcm_ip6_t *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_DstIp6_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstIp6
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_DstIp6_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_ip6_t *data,
    bcm_ip6_t *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_SrcIp6High_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcIp6High
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_SrcIp6High_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_ip6_t *data,
    bcm_ip6_t *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_SrcIpEqualDstIp_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcIpEqualDstIp
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      flag - (OUT) Qualifier match flags.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_SrcIpEqualDstIp_get(
    int unit,
    bcm_field_entry_t entry,
    uint32 *flag)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_DstIp6High_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstIp6High
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_DstIp6High_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_ip6_t *data,
    bcm_ip6_t *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_Ip6NextHeader_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIp6NextHeader
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_Ip6NextHeader_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_Ip6TrafficClass_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIp6TrafficClass
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_Ip6TrafficClass_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_InnerIp6FlowLabel_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInnerIp6FlowLabel
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_InnerIp6FlowLabel_get(
    int unit,
    bcm_field_entry_t entry,
    uint32 *data,
    uint32 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_Ip6FlowLabel_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIp6FlowLabel
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_Ip6FlowLabel_get(
    int unit,
    bcm_field_entry_t entry,
    uint32 *data,
    uint32 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_Ip6HopLimit_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIp6HopLimit
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_Ip6HopLimit_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_SrcMac_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcMac
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_SrcMac_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_mac_t *data,
    bcm_mac_t *mask)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X," FIELD_MACA_FORMAT "," FIELD_MACA_FORMAT
                          ")\n"),
                unit,
                entry,
                FIELD_MACA_SHOW(*data),
                FIELD_MACA_SHOW(*mask)));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_qualify_SrcMac_get(unitData,
                                                          entry,
                                                          data,
                                                          mask);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X," FIELD_MACA_FORMAT "," FIELD_MACA_FORMAT
                          ") = %d (%s)\n"),
                unit,
                entry,
                FIELD_MACA_SHOW(*data),
                FIELD_MACA_SHOW(*mask),
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_DstMac_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstMac
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_DstMac_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_mac_t *data,
    bcm_mac_t *mask)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X," FIELD_MACA_FORMAT "," FIELD_MACA_FORMAT
                          ")\n"),
                unit,
                entry,
                FIELD_MACA_SHOW(*data),
                FIELD_MACA_SHOW(*mask)));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_qualify_DstMac_get(unitData,
                                                          entry,
                                                          data,
                                                          mask);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X," FIELD_MACA_FORMAT "," FIELD_MACA_FORMAT
                          ") = %d (%s)\n"),
                unit,
                entry,
                FIELD_MACA_SHOW(*data),
                FIELD_MACA_SHOW(*mask),
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_PacketFormat_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyPacketFormat
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_PacketFormat_get(
    int unit,
    bcm_field_entry_t entry,
    uint32 *data,
    uint32 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_SrcMacGroup_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcMacGroup
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_SrcMacGroup_get(
    int unit,
    bcm_field_entry_t entry,
    uint32 *data,
    uint32 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_InnerIpType_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInnerIpType
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      type - (OUT) Inner ip header ip type.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_InnerIpType_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_field_IpType_t *type)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_ForwardingType_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyForwardingType
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      type - (OUT) Qualifier match forwarding type.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_ForwardingType_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_field_ForwardingType_t *type)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_IpType_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIpType
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      type - (OUT) Qualifier match ip type.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_IpType_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_field_IpType_t *type)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_L2Format_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyL2Format
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      type - (OUT) Qualifier match l2 format.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_L2Format_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_field_L2Format_t *type)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_VlanFormat_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyVlanFormat
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_VlanFormat_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_MHOpcode_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyMHOpcode
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_MHOpcode_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_UserDefined_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyUserDefined
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      udf_id - (IN) Udf spec id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_UserDefined_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_field_udf_t udf_id,
    uint8 data[BCM_FIELD_USER_FIELD_SIZE],
    uint8 mask[BCM_FIELD_USER_FIELD_SIZE])
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_HiGig_get
 * Purpose:
 *      Qualify on HiGig packets.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_HiGig_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_InterfaceClassPort_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInterfaceClassPort
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_InterfaceClassPort_get(
    int unit,
    bcm_field_entry_t entry,
    uint32 *data,
    uint32 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_InterfaceClassL2_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInterfaceClassL2
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_InterfaceClassL2_get(
    int unit,
    bcm_field_entry_t entry,
    uint32 *data,
    uint32 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_InterfaceClassL3_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInterfaceClassL3
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_InterfaceClassL3_get(
    int unit,
    bcm_field_entry_t entry,
    uint32 *data,
    uint32 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_SrcClassL2_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcClassL2
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_SrcClassL2_get(
    int unit,
    bcm_field_entry_t entry,
    uint32 *data,
    uint32 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_SrcClassL3_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcClassL3
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_SrcClassL3_get(
    int unit,
    bcm_field_entry_t entry,
    uint32 *data,
    uint32 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_SrcClassField_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcClassField
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_SrcClassField_get(
    int unit,
    bcm_field_entry_t entry,
    uint32 *data,
    uint32 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_DstClassL2_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstClassL2
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_DstClassL2_get(
    int unit,
    bcm_field_entry_t entry,
    uint32 *data,
    uint32 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_DstClassL3_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstClassL3
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_DstClassL3_get(
    int unit,
    bcm_field_entry_t entry,
    uint32 *data,
    uint32 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_DstClassField_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstClassField
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_DstClassField_get(
    int unit,
    bcm_field_entry_t entry,
    uint32 *data,
    uint32 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_L3IntfGroup_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyL3IntfGroup
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_L3IntfGroup_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_if_group_t *data,
    bcm_if_group_t *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_IpProtocolCommon_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIpProtocolCommon
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      protocol - (OUT) Qualifier protocol encoding.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_IpProtocolCommon_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_field_IpProtocolCommon_t *protocol)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_InnerIpProtocolCommon_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInnerIpProtocolCommon
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      protocol - (OUT) Qualifier inner ip protocol encodnig.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_InnerIpProtocolCommon_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_field_IpProtocolCommon_t *protocol)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_Snap_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySnap
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_Snap_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_field_snap_header_t *data,
    bcm_field_snap_header_t *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_Llc_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyLlc
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_Llc_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_field_llc_header_t *data,
    bcm_field_llc_header_t *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_InnerTpid_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInnerTpid
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      tpid - (OUT) Qualifier tpid.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_InnerTpid_get(
    int unit,
    bcm_field_entry_t entry,
    uint16 *tpid)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_OuterTpid_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyOuterTpid
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      tpid - (OUT) Qualifier tpid.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_OuterTpid_get(
    int unit,
    bcm_field_entry_t entry,
    uint16 *tpid)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_PortClass_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyPortClass
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_PortClass_get(
    int unit,
    bcm_field_entry_t entry,
    uint32 *data,
    uint32 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_L3Routable_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyL3Routable
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_L3Routable_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_IpFrag_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIpFrag
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      frag_info - (OUT) Qualifier ip framentation encoding.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_IpFrag_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_field_IpFrag_t *frag_info)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_LookupClass0_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyLookupClass0
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_LookupClass0_get(
    int unit,
    bcm_field_entry_t entry,
    uint32 *data,
    uint32 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_Vrf_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyVrf
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_Vrf_get(
    int unit,
    bcm_field_entry_t entry,
    uint32 *data,
    uint32 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_L3Ingress_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyL3Ingress
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_L3Ingress_get(
    int unit,
    bcm_field_entry_t entry,
    uint32 *data,
    uint32 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_ExtensionHeaderType_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyExtensionHeaderType
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_ExtensionHeaderType_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_ExtensionHeaderSubCode_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyExtensionHeaderSubCode
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_ExtensionHeaderSubCode_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_ExtensionHeader2Type_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyExtensionHeaderType
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_ExtensionHeader2Type_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_L4Ports_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyL4Ports
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_L4Ports_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_MirrorCopy_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyMirrorCopy
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_MirrorCopy_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_TunnelTerminated_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyTunnelTerminated
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_TunnelTerminated_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_MplsTerminated_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyMplsTerminated
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_MplsTerminated_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_InnerSrcIp_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInnerSrcIp
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_InnerSrcIp_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_ip_t *data,
    bcm_ip_t *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_InnerDstIp_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInnerDstIp
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_InnerDstIp_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_ip_t *data,
    bcm_ip_t *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_InnerSrcIp6_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInnerSrcIp6
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_InnerSrcIp6_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_ip6_t *data,
    bcm_ip6_t *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_InnerDstIp6_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInnerDstIp6
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_InnerDstIp6_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_ip6_t *data,
    bcm_ip6_t *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_InnerSrcIp6High_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInnerSrcIp6High
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_InnerSrcIp6High_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_ip6_t *data,
    bcm_ip6_t *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_InnerDstIp6High_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInnerDstIp6High
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_InnerDstIp6High_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_ip6_t *data,
    bcm_ip6_t *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_InnerTtl_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInnerTtl
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_InnerTtl_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_InnerTos_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInnerDSCP
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_InnerTos_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_InnerDSCP_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInnerDSCP
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_InnerDSCP_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_InnerIpProtocol_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInnerIpProtocol
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_InnerIpProtocol_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_InnerIpFrag_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInnerIpFrag
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      frag_info - (OUT) Inner ip header fragmentation info.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_InnerIpFrag_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_field_IpFrag_t *frag_info)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_DosAttack_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDosAttack
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_DosAttack_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_IpmcStarGroupHit_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIpmcStarGroupHit
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_IpmcStarGroupHit_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_MyStationHit_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyMyStationHit
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_MyStationHit_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_L2PayloadFirstEightBytes_get
 * Purpose:
 *      Get match criteria for bcmFieldQualifyL2PayloadFirstEightBytes
 *          qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data1 - (OUT) Qualifier first four bytes of match data.
 *      data2 - (OUT) Qualifier last four bytes of match data.
 *      mask1 - (OUT) Qualifier first four bytes of match mask.
 *      mask2 - (OUT) Qualifier last four bytes of match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_L2PayloadFirstEightBytes_get(
    int unit,
    bcm_field_entry_t entry,
    uint32 *data1,
    uint32 *data2,
    uint32 *mask1,
    uint32 *mask2)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_L3DestRouteHit_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyL3DestRouteHit
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_L3DestRouteHit_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_L3DestHostHit_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyL3DestHostHit
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_L3DestHostHit_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_L3SrcHostHit_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyL3SrcHostHit
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_L3SrcHostHit_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_L2CacheHit_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyL2CacheHit
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_L2CacheHit_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_L2StationMove_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyL2StationMove
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_L2StationMove_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_L2DestHit_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyL2DestHit
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_L2DestHit_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_L2SrcStatic_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyL2SrcStatic
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_L2SrcStatic_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_L2SrcHit_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyL2SrcHit
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_L2SrcHit_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_IngressStpState_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIngressStpState
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_IngressStpState_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_ForwardingVlanValid_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyForwardingVlanValid
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_ForwardingVlanValid_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    return BCM_E_UNAVAIL;
}

/* Function: bcm_fe2000_field_group_config_create
 *
 * Purpose:
 *     Create a group with a mode (single, double, etc.), a port bitmap,
 *     group size and a Group ID. 
 * Parameters:
 *     unit - BCM device number.
 *     group_config - Group create attributes namely:
 *          flags       - (IN) Bits indicate which parameters have been
 *                             passed to API and should be used during group
 *                             creation.
 *          qset        - (IN) Field qualifier set
 *          priority    - (IN) Priority within allowable range,
 *                             or BCM_FIELD_GROUP_PRIO_ANY to automatically
 *                             assign a priority; each priority value may be
 *                             used only once
 *          mode        - (IN) Group mode (single, double, triple or Auto-wide)
 *          ports       - (IN) Ports where group is defined
 *          group       - (IN/OUT) Requested Group ID. If Group ID is not set,
 *                              then API allocates and returns the created
 *                              Group ID.
 * Returns:
 *     BCM_E_INIT      - BCM unit not initialized
 *     BCM_E_RESOURCE  - no select codes will satisfy qualifier set
 *     BCM_E_NONE      - Success
 */
int
bcm_fe2000_field_group_config_create(int unit,
    bcm_field_group_config_t *group_config) 
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_SrcVirtualPortValid_get
 * Purpose:
 *      Get match criteria for bcmFieldQualifySrcVirtualPortValid
 *      qualifier from the field entry.
 * Parameters:
 *      unit    - (IN) Unit number.
 *      entry   - (IN) BCM field entry id.
 *      data    - (OUT) Qualifier match data.
 *      mask    - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_SrcVirtualPortValid_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_DstL3EgressNextHops_get
 * Purpose:
 *      Get match criteria for bcmFieldQualifyDstL3EgressNextHops
 *      qualifier from the field entry.
 * Parameters:
 *      unit    - (IN) Unit number.
 *      entry   - (IN) BCM field entry id.
 *      data    - (OUT) Qualifier match data.
 *      mask    - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_DstL3EgressNextHops_get(int unit,
    bcm_field_entry_t entry,
    uint32 *data,
    uint32 *mask)
{
    return (BCM_E_UNAVAIL);
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_VlanTranslationHit_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyVlanTranslationHit
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_VlanTranslationHit_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_IpAuth_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIpAuth
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_IpAuth_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_BigIcmpCheck_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyBigIcmpCheck
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      flag - (OUT) Flag.
 *      size - (OUT) Size.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_BigIcmpCheck_get(
    int unit,
    bcm_field_entry_t entry,
    uint32 *flag,
    uint32 *size)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_IcmpTypeCode_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIcmpTypeCode
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_IcmpTypeCode_get(
    int unit,
    bcm_field_entry_t entry,
    uint16 *data,
    uint16 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_IgmpTypeMaxRespTime_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIgmpTypeMaxRespTime
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_IgmpTypeMaxRespTime_get(
    int unit,
    bcm_field_entry_t entry,
    uint16 *data,
    uint16 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_sbx_field_qualifier_delete
 * Purpose:
 *      Remove match criteria from a field processor entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      qual_id - (IN) BCM field qualifier id.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualifier_delete(
    int unit,
    bcm_field_entry_t entry,
    bcm_field_qualify_t qual_id)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_TranslatedVlanFormat
 * Purpose:
 *      Set match criteria for bcmFieildQualifyTranslatedVlanFormat
 *                     qualifier in the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (IN) Qualifier match data.
 *      mask - (IN) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_TranslatedVlanFormat(
    int unit,
    bcm_field_entry_t entry,
    uint8 data,
    uint8 mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_IntPriority
 * Purpose:
 *      Set match criteria for bcmFieildQualifyIntPriority
 *                     qualifier in the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (IN) Qualifier match data.
 *      mask - (IN) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_IntPriority(
    int unit,
    bcm_field_entry_t entry,
    uint8 data,
    uint8 mask)
{
    return BCM_E_UNAVAIL;
}


/*
 * Function:
 *      bcm_fe2000_field_qualify_Color
 * Purpose:
 *      Set match criteria for bcmFieildQualifyColor
 *                     qualifier in the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      color - (IN) Qualifier match color.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_Color(
    int unit,
    bcm_field_entry_t entry,
    uint8 color)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_FibreChanOuter
 * Purpose:
 *      Add Fibre Channel outer header type field qualification to a field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) Field entry id.
 *      fibre_chan_type - (IN) Data to qualify with.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_fe2000_field_qualify_FibreChanOuter(int unit, 
                             bcm_field_entry_t entry, 
                             bcm_field_FibreChan_t fibre_chan_type)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_FibreChanInner
 * Purpose:
 *      Add Fibre Channel inner header type field qualification to a field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) Field entry id.
 *      fibre_chan_type - (IN) Data to qualify with.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_fe2000_field_qualify_FibreChanInner(int unit, 
                             bcm_field_entry_t entry, 
                             bcm_field_FibreChan_t fibre_chan_type)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_VnTag
 * Purpose:
 *      Add NIV VN tag field qualification to a field entry.
 * Parameters:
 *      unit  - (IN) Unit number.
 *      entry - (IN) Field entry id.
 *      data  - (IN) Qualifier match data.
 *      mask  - (IN) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_VnTag(int unit, bcm_field_entry_t entry,
                            uint32 data, uint32 mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_CnTag
 * Purpose:
 *      Add QCN CN tag field qualification to a field entry.
 * Parameters:
 *      unit  - (IN) Unit number.
 *      entry - (IN) Field entry id.
 *      data  - (IN) Qualifier match data.
 *      mask  - (IN) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_CnTag(int unit, bcm_field_entry_t entry,
                            uint32 data, uint32 mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_FabricQueueTag
 * Purpose:
 *      Add Fabric Queue tag field qualification to a field entry.
 * Parameters:
 *      unit  - (IN) Unit number.
 *      entry - (IN) Field entry id.
 *      data  - (IN) Qualifier match data.
 *      mask  - (IN) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_FabricQueueTag(int unit, bcm_field_entry_t entry,
                                     uint32 data, uint32 mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_SrcModPortGport
 * Purpose:
 *      Set match criteria for bcmFieildQualifySrcModPortGport
 *                     qualifier in the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (IN) Qualifier match gport.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_SrcModPortGport(
    int unit,
    bcm_field_entry_t entry,
    bcm_gport_t data)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_SrcModuleGport
 * Purpose:
 *      Set match criteria for bcmFieildQualifySrcModuleGport
 *                     qualifier in the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (IN) Qualifier match gport.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_SrcModuleGport(
    int unit,
    bcm_field_entry_t entry,
    bcm_gport_t data)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_TranslatedVlanFormat_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyTranslatedVlanFormat
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_TranslatedVlanFormat_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_IntPriority_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIntPriority
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_IntPriority_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_Color_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyColor
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      color - (OUT) Qualifier match color.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_Color_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *color)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_FibreChannOuter_get
 * Purpose:
 *      Get Fibre Channel outer header type field qualification from a field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) Field entry id.
 *      fibre_chan_type - (OUT) Fibre Channel header type.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_fe2000_field_qualify_FibreChanOuter_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_field_FibreChan_t *fibre_chan_type)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_FibreChannInner_get
 * Purpose:
 *      Get Fibre Channel inner header type field qualification from a field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) Field entry id.
 *      fibre_chan_type - (OUT) Fibre Channel header type.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_fe2000_field_qualify_FibreChanInner_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_field_FibreChan_t *fibre_chan_type)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_VnTag_get
 * Purpose:
 *      Get match criteria for bcmFieldQualifyVnTag
 *      qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_fe2000_field_qualify_VnTag_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_CnTag_get
 * Purpose:
 *      Get match criteria for bcmFieldQualifyCnTag
 *      qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_fe2000_field_qualify_CnTag_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_FabricQueueTag_get
 * Purpose:
 *      Get match criteria for bcmFieldQualifyFabricQueueTag
 *      qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_fe2000_field_qualify_FabricQueueTag_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_SrcModPortGport_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcModPortGport
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match gport.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_SrcModPortGport_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_gport_t *data)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_SrcModuleGport_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcModuleGport
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match gport.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_SrcModuleGport_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_gport_t *data)
{
    return BCM_E_UNAVAIL;
}

/* Function: bcm_fe2000_field_group_wlan_create_mode
 *
 * Purpose:
 *     Create a wlan field group with a mode (single, double, etc.).
 *
 * Parameters:
 *     unit - BCM device number.
 *     qset - Field qualifier set
 *     pri  - Priority within allowable range,
 *            or BCM_FIELD_GROUP_PRIO_ANY to automatically assign a
 *            priority; each priority value may be used only once
 *    mode  - Group mode (single, double, triple or Auto-wide)
 *    group - (OUT) field Group ID
 *
 * Returns:
 *     BCM_E_INIT      - BCM unit not initialized
 *     BCM_E_RESOURCE  - no select codes will satisfy qualifier set
 *     BCM_E_NONE      - Success
 */
int
bcm_fe2000_field_group_wlan_create_mode(int unit, bcm_field_qset_t qset, int pri,
                                     bcm_field_group_mode_t mode,
                                     bcm_field_group_t *group)
{
    return BCM_E_UNAVAIL;
}

/* Function: bcm_fe2000_field_group_wlan_create_mode_id
 *
 * Purpose:
 *     Create a wlan field group with a mode (single, double, etc.).
 *
 * Parameters:
 *     unit - BCM device number.
 *     qset - Field qualifier set
 *     pri  - Priority within allowable range,
 *            or BCM_FIELD_GROUP_PRIO_ANY to automatically assign a
 *            priority; each priority value may be used only once
 *    mode  - Group mode (single, double, triple or Auto-wide)
 *    group - (OUT) field Group ID
 *
 * Returns:
 *     BCM_E_INIT      - BCM unit not initialized
 *     BCM_E_RESOURCE  - no select codes will satisfy qualifier set
 *     BCM_E_NONE      - Success
 */
int
bcm_fe2000_field_group_wlan_create_mode_id(int unit, bcm_field_qset_t qset, int pri,
                                        bcm_field_group_mode_t mode,
                                        bcm_field_group_t group)
{
    return BCM_E_UNAVAIL;
}


/*
 * Function:
 *      bcm_fe2000_field_action_mac_add
 * Purpose:
 *      Add an action to a field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) Field entry Id.
 *      action - (IN) Field action id.
 *      mac - (IN) Field action parameter.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_fe2000_field_action_mac_add(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_field_action_t action, 
    bcm_mac_t mac)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_fe2000_field_action_mac_get
 * Purpose:
 *      Retrieve the parameters for an action previously added to a
 *      field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) Field entry Id.
 *      action - (IN) Field action id.
 *      mac - (IN) Field action argument.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_fe2000_field_action_mac_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_field_action_t action, 
    bcm_mac_t *mac)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_fe2000_field_group_traverse
 * Purpose:
 *      Traverse all the fp groups in the system, calling a specified
 *      callback for each one
 * Parameters:
 *      unit - (IN) Unit number.
 *      callback - (IN) A pointer to the callback function to call for each fp group
 *      user_data - (IN) Pointer to user data to supply in the callback
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_fe2000_field_group_traverse(int unit, bcm_field_group_traverse_cb callback,
                             void *user_data)
{
    return BCM_E_UNAVAIL; 
}


/*
 * Function:
 *      bcm_fe2000_field_qualify_ForwardingVlanId_get
 * Purpose:
 *       Get match criteria for bcmFieildQualifyForwardingVlanId
 *                       qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match data.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_fe2000_field_qualify_ForwardingVlanId_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_vlan_t *data, 
    bcm_vlan_t *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_Vpn_get
 * Purpose:
 *       Get match criteria for bcmFieildQualifyVpn
 *                       qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match data.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_fe2000_field_qualify_Vpn_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_vpn_t *data, 
    bcm_vpn_t *mask)
{
    return BCM_E_UNAVAIL; 
}


/*
 * Function:
 *      bcm_fe2000_field_qualify_ForwardingVlanId
 * Purpose:
 *      Set match criteria for bcmFieildQualifyForwardingVlanId
 *      qualifier in the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (IN) Qualifier match data.
 *      mask - (IN) Qualifier match data.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_fe2000_field_qualify_ForwardingVlanId(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_vlan_t data, 
    bcm_vlan_t mask)
{
    return BCM_E_UNAVAIL; 
}


/*
 * Function:
 *      bcm_fe2000_field_qualify_Vpn
 * Purpose:
 *      Set match criteria for bcmFieildQualifyVpn
 *      qualifier in the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (IN) Qualifier match data.
 *      mask - (IN) Qualifier match data.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_fe2000_field_qualify_Vpn(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_vpn_t data, 
    bcm_vpn_t mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_SrcIp6Low
 * Purpose:
 *      Set match criteria for bcmFieildQualifySrcIp6Low
 *                     qualifier in the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_SrcIp6Low(int unit, bcm_field_entry_t entry,
                                 bcm_ip6_t data, bcm_ip6_t mask)
{
    return (BCM_E_UNAVAIL);
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_DstIp6Low
 * Purpose:
 *      Set match criteria for bcmFieildQualifyDstIp6Low
 *                     qualifier in the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_field_qualify_DstIp6Low(int unit, bcm_field_entry_t entry,
                                 bcm_ip6_t data, bcm_ip6_t mask)
{
    return (BCM_E_UNAVAIL);
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_SrcIp6Low_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcIp6Low
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_fe2000_field_qualify_SrcIp6Low_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_ip6_t *data, 
    bcm_ip6_t *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_DstIp6Low_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstIp6Low
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_fe2000_field_qualify_DstIp6Low_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_ip6_t *data, 
    bcm_ip6_t *mask)
{
    return BCM_E_UNAVAIL; 
}
/*
 * Function:
 *      bcm_fe2000_field_qualify_FlowId
 * Purpose:
 *      Set match criteria for bcmFieildQualifyFlowId
 *      qualifier in the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (IN) Qualifier match data.
 *      mask - (IN) Qualifier match data.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */

int 
bcm_fe2000_field_qualify_FlowId(int unit, bcm_field_entry_t entry,
                 uint16 data, uint16 mask)
{
    return BCM_E_UNAVAIL; 
}
/*
 * Function:
 *      bcm_fe2000_field_qualify_InVPort
 * Purpose:
 *      Set match criteria for bcmFieildQualifyInVPort
 *      qualifier in the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (IN) Qualifier match data.
 *      mask - (IN) Qualifier match data.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */

int 
bcm_fe2000_field_qualify_InVPort(int unit, bcm_field_entry_t entry,
                 uint8 data, uint8 mask)
{
    return BCM_E_UNAVAIL; 
}
/*
 * Function:
 *      bcm_fe2000_field_qualify_OutVPort
 * Purpose:
 *      Set match criteria for bcmFieildQualifyOutVPort
 *      qualifier in the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (IN) Qualifier match data.
 *      mask - (IN) Qualifier match data.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */

int 
bcm_fe2000_field_qualify_OutVPort(int unit, bcm_field_entry_t entry,
                 uint8 data, uint8 mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_FlowId_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyFlowId
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_fe2000_field_qualify_FlowId_get(int unit, bcm_field_entry_t entry,
                 uint16 *data, uint16 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_InVPort_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInVPort
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_fe2000_field_qualify_InVPort_get(int unit, bcm_field_entry_t entry,
                 uint8 *data, uint8 *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_field_qualify_OutVPort_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyOutVPort
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_fe2000_field_qualify_OutVPort_get(int unit, bcm_field_entry_t entry,
                 uint8 *data, uint8 *mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_field_wb_state_sync(int unit, int sync)
{
    _FE2000_FIELD_COMMON_LOCALS;

    FIELD_VERB((FIELD_MSG("(%d,%08X)\n"),
                unit, sync));

    result = _bcm_fe2000_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            result = bcm_fe2000_g2p3_field_wb_state_sync(unitData, sync);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            FIELD_ERR((FIELD_MSG1("unsupported microcode on unit %d\n"), unit));
            result = _SBX_FE2K_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_fe2000_field_unit_outro(result, unit, lock);

    FIELD_VERB((FIELD_MSG("(%d,%08X) = %d (%s)\n"),
                unit,
                sync,
                result,
                _SHR_ERRMSG(result)));
    return result;
}
