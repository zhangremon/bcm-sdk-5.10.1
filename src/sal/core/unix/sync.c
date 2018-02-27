/*
 * $Id: sync.c 1.21 Broadcom SDK $
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
 * File: 	sync.c
 * Purpose:	Defines sal routines for mutexes and semaphores
 *
 * Mutex and Binary Semaphore abstraction
 *
 * Note: the SAL mutex abstraction is required to allow the same mutex
 * to be taken recursively by the same thread without deadlock.
 *
 * The POSIX mutex used here has been further abstracted to ensure this.
 */

#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>

#include <assert.h>
#include <sal/core/sync.h>
#include <sal/core/thread.h>

#if defined(LINUX) && defined(__USE_UNIX98)
#define USE_POSIX_RECURSIVE_MUTEX
#endif

#if defined (__STRICT_ANSI__)
#define NO_CONTROL_C
#endif

#ifdef BROADCOM_DEBUG
#ifdef INCLUDE_BCM_SAL_PROFILE
static unsigned int _sal_sem_count_curr;
static unsigned int _sal_sem_count_max;
static unsigned int _sal_mutex_count_curr;
static unsigned int _sal_mutex_count_max;
#define SAL_SEM_RESOURCE_USAGE_INCR(a_curr, a_max, ilock)               \
        a_curr++;                                                       \
        a_max = ((a_curr) > (a_max)) ? (a_curr) : (a_max)
    
#define SAL_SEM_RESOURCE_USAGE_DECR(a_curr, ilock)                      \
        a_curr--

/*
 * Function:
 *      sal_sem_resource_usage_get
 * Purpose:
 *      Provides count of active sem and maximum sem allocation
 * Parameters:
 *      sem_curr - Current semaphore allocation.
 *      sem_max - Maximum semaphore allocation.
 */

void
sal_sem_resource_usage_get(unsigned int *sem_curr, unsigned int *sem_max)
{
    if (sem_curr != NULL) {
        *sem_curr = _sal_sem_count_curr;
    }
    if (sem_max != NULL) {
        *sem_max = _sal_sem_count_max;
    }
}

/*
 * Function:
 *      sal_mutex_resource_usage_get
 * Purpose:
 *      Provides count of active mutex and maximum mutex allocation
 * Parameters:
 *      mutex_curr - Current mutex allocation.
 *      mutex_max - Maximum mutex allocation.
 */

void
sal_mutex_resource_usage_get(unsigned int *mutex_curr, unsigned int *mutex_max)
{
    if (mutex_curr != NULL) {
        *mutex_curr = _sal_mutex_count_curr;
    }
    if (mutex_max != NULL) {
        *mutex_max = _sal_mutex_count_max;
    }
}
#endif
#endif

/*
 * Keyboard interrupt protection
 *
 *   When a thread is running on a console, the user could Control-C
 *   while a mutex is held by the thread.  Control-C results in a signal
 *   that longjmp's somewhere else.  We prevent this from happening by
 *   blocking Control-C signals while any mutex is held.
 */

#ifndef NO_CONTROL_C
static int ctrl_c_depth = 0;
static sigset_t ctrl_c_sigset;
static int ctrl_c_initialized;
#endif

static void
ctrl_c_block(void)
{
#ifndef NO_CONTROL_C
    if (sal_thread_self() == sal_thread_main_get()) {
	if (ctrl_c_depth++ == 0) {
	    sigprocmask(SIG_BLOCK, &ctrl_c_sigset, NULL);
	}
    }
#endif    
}

static void
ctrl_c_unblock(void)
{
#ifndef NO_CONTROL_C
    if (sal_thread_self() == sal_thread_main_get()) {
	assert(ctrl_c_depth > 0);
	if (--ctrl_c_depth == 0) {
	    sigprocmask(SIG_UNBLOCK, &ctrl_c_sigset, NULL);
	}
    }
#endif
}

/*
 * recursive_mutex_t
 *
 *   This is an abstract type built on the POSIX mutex that allows a
 *   mutex to be taken recursively by the same thread without deadlock.
 *
 *   The Linux version of pthreads supports recursive mutexes
 *   (a non-portable extension to posix). In this case, we 
 *   use the Linux support instead of our own. 
 */

typedef struct recursive_mutex_s {
    pthread_mutex_t	mutex;
    char		*desc;
#ifndef USE_POSIX_RECURSIVE_MUTEX
    sal_thread_t	owner;
    int			recurse_count;
#endif
} recursive_mutex_t;



#ifdef netbsd
/*
 * The netbsd pthreads implementation we are using
 * does not seem to have his function
 */
static int
pthread_mutexattr_init(pthread_mutexattr_t* attr)
{
    attr->m_type = PTHREAD_MUTEXTYPE_DEBUG;
    attr->m_flags = 0;
    return 0;
}

#endif /* netbsd */

static sal_mutex_t
_sal_mutex_create(char *desc)
{
    recursive_mutex_t	*rm;
    pthread_mutexattr_t attr;

#ifndef NO_CONTROL_C
    if (!ctrl_c_initialized) {
        sigemptyset(&ctrl_c_sigset);
        sigaddset(&ctrl_c_sigset, SIGINT);
        ctrl_c_initialized = 1;
    }
#endif
    
    if ((rm = malloc(sizeof (recursive_mutex_t))) == NULL) {
	return NULL;
    }

    rm->desc = desc;
    pthread_mutexattr_init(&attr);
#ifdef USE_POSIX_RECURSIVE_MUTEX
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
#else
    rm->owner = 0;
    rm->recurse_count = 0;
#endif
    pthread_mutex_init(&rm->mutex, &attr);
#ifdef BROADCOM_DEBUG
#ifdef INCLUDE_BCM_SAL_PROFILE
        SAL_SEM_RESOURCE_USAGE_INCR(
            _sal_mutex_count_curr,
            _sal_mutex_count_max,
            ilock);
#endif
#endif
    return (sal_mutex_t) rm;
}

/*
 * Mutex and semaphore abstraction
 */

sal_mutex_t
sal_mutex_create(char *desc)
{
#ifdef SAL_GLOBAL_MUTEX
    static sal_mutex_t _m = NULL;
    if (!_m) {
	_m = _sal_mutex_create("sal_global_mutex");
	assert(_m);
    }
    if (strcmp(desc, "spl mutex")) {
	return _m;
    }
#endif
    return _sal_mutex_create(desc);

}

void
sal_mutex_destroy(sal_mutex_t m)
{
    recursive_mutex_t	*rm = (recursive_mutex_t *) m;

    assert(rm);

    pthread_mutex_destroy(&rm->mutex);

    free(rm);
#ifdef BROADCOM_DEBUG
#ifdef INCLUDE_BCM_SAL_PROFILE
        SAL_SEM_RESOURCE_USAGE_DECR(
            _sal_mutex_count_curr,
            ilock);
#endif
#endif
}

int
sal_mutex_take(sal_mutex_t m, int usec)
{
    recursive_mutex_t	*rm = (recursive_mutex_t *) m;
    int			err;
#ifndef USE_POSIX_RECURSIVE_MUTEX
    sal_thread_t	myself = sal_thread_self();
#endif

    assert(rm);

#ifndef USE_POSIX_RECURSIVE_MUTEX
    if (rm->owner == myself) {
	rm->recurse_count++;
	return 0;
    }
#endif

    ctrl_c_block();

    if (usec == sal_mutex_FOREVER) {
	do {
	    err = pthread_mutex_lock(&rm->mutex);
	} while (err != 0 && errno == EINTR);
    } else {
	int		time_wait = 1;

	/* Retry algorithm with exponential backoff */

	for (;;) {
	    err = pthread_mutex_trylock(&rm->mutex);

	    if (err != EBUSY) {
		break;		/* Done (0), or error other than EBUSY */
	    }

	    if (time_wait > usec) {
		time_wait = usec;
	    }

	    sal_usleep(time_wait);

	    usec -= time_wait;

	    if (usec == 0) {
		err = ETIMEDOUT;
		break;
	    }

	    if ((time_wait *= 2) > 100000) {
		time_wait = 100000;
	    }
	}
    }

    if (err) {
	ctrl_c_unblock();
	return -1;
    }

#ifndef USE_POSIX_RECURSIVE_MUTEX
    assert(rm->owner == 0);
    rm->owner = myself;
#endif

    return 0;
}

int
sal_mutex_give(sal_mutex_t m)
{
    recursive_mutex_t	*rm = (recursive_mutex_t *) m;
    int			err;

    assert(rm);

#ifndef USE_POSIX_RECURSIVE_MUTEX
    if ((rm->owner != sal_thread_self())) {
        assert(rm->owner == sal_thread_self());
    }
    if (rm->recurse_count > 0) {
	rm->recurse_count--;
	return 0;
    }
    rm->owner = 0;
#endif
    err = pthread_mutex_unlock(&rm->mutex);
    ctrl_c_unblock();
    return err ? -1 : 0;
}

#ifndef LINUX_SAL_SEM_OVERRIDE

sal_sem_t
sal_sem_create(char *desc, int binary, int initial_count)
{
    sem_t *s = NULL;

    /* Ignore binary for now */

    if ((s = malloc(sizeof (sem_t))) != 0) {
	sem_init(s, 0, initial_count);
    }

#ifdef BROADCOM_DEBUG
#ifdef INCLUDE_BCM_SAL_PROFILE
        SAL_SEM_RESOURCE_USAGE_INCR(
            _sal_sem_count_curr,
            _sal_sem_count_max,
            ilock);
#endif
#endif

    return (sal_sem_t) s;
}

void
sal_sem_destroy(sal_sem_t b)
{
    sem_t		*s = (sem_t *) b;

    assert(s);

    sem_destroy(s);

    free(s);

#ifdef BROADCOM_DEBUG
#ifdef INCLUDE_BCM_SAL_PROFILE
        SAL_SEM_RESOURCE_USAGE_DECR(
            _sal_sem_count_curr,
            ilock);
#endif
#endif
}

int
sal_sem_take(sal_sem_t b, int usec)
{
    sem_t		*s = (sem_t *) b;
    int			err;

    if (usec == sal_sem_FOREVER) {
	do {
	    err = sem_wait(s);
	} while (err != 0 && errno == EINTR);
    } else {
	int		time_wait = 1;

	/* Retry algorithm with exponential backoff */

	for (;;) {
	    if (sem_trywait(s) == 0) {
		err = 0;
		break;
	    }

	    if (errno != EAGAIN && errno != EINTR) {
		err = errno;
		break;
	    }

	    if (time_wait > usec) {
		time_wait = usec;
	    }

	    sal_usleep(time_wait);

	    usec -= time_wait;

	    if (usec == 0) {
		err = ETIMEDOUT;
		break;
	    }

	    if ((time_wait *= 2) > 100000) {
		time_wait = 100000;
	    }
	}
    }

    return err ? -1 : 0;
}

int
sal_sem_give(sal_sem_t b)
{
    sem_t		*s = (sem_t *) b;
    int			err;

    err = sem_post(s);

    return err ? -1 : 0;
}

#endif /* LINUX_SAL_SEM_OVERRIDE */
