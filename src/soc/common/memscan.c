/*
 * $Id: memscan.c 1.4 Broadcom SDK $
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
 * Memory Error Scan
 *
 * This is an optional module that can be used to detect and correct
 * memory errors in certain static hardware memories (tables).
 *
 * Additional CPU time is required to do the scanning.  Table DMA is
 * used to reduce this overhead.  The application may choose the overall
 * scan rate in entries/sec.
 *
 * There is also a requirement of additional memory needed to store
 * backing caches of the chip memories.  The backing caches are the same
 * ones used by the soc_mem_cache() mechanism.  Note that enabling the
 * memory cache for frequently updates tables can have significant
 * performance benefits.
 *
 * When the memory scanning thread is enabled, it simply scans all
 * memories for which caching is enabled, because these static memories
 * are the ones amenable to software error scanning.
 */

#ifdef INCLUDE_MEM_SCAN

#include <sal/core/libc.h>

#include <soc/mem.h>
#include <soc/debug.h>
#include <soc/cmic.h>
#include <soc/error.h>
#include <soc/drv.h>

STATIC void _soc_mem_scan_thread(void *unit_vp);

/*
 * Function:
 * 	soc_mem_scan_running
 * Purpose:
 *   	Boolean to indicate if the memory scan thread is running
 * Parameters:
 *	unit - unit number.
 *	rate - (OUT) if non-NULL, number of entries scanned per interval
 *	interval - (OUT) if non-NULL, receives current wake-up interval.
 */

int
soc_mem_scan_running(int unit, int *rate, sal_usecs_t *interval)
{
    soc_control_t	*soc = SOC_CONTROL(unit);

    if (soc->mem_scan_pid  != SAL_THREAD_ERROR) {
        if (rate != NULL) {
            *rate = soc->mem_scan_rate;
        }

        if (interval != NULL) {
            *interval = soc->mem_scan_interval;
        }
    }

    return (soc->mem_scan_pid != SAL_THREAD_ERROR);
}

/*
 * Function:
 * 	soc_mem_scan_start
 * Purpose:
 *   	Start memory scan thread
 * Parameters:
 *	unit - unit number.
 *	rate - maximum number of entries to scan each time thread runs
 *	interval - how often the thread should run (microseconds).
 * Returns:
 *	SOC_E_MEMORY if can't create thread.
 */

int
soc_mem_scan_start(int unit, int rate, sal_usecs_t interval)
{
    soc_control_t	*soc = SOC_CONTROL(unit);
    int			pri;

    if (soc->mem_scan_pid != SAL_THREAD_ERROR) {
	SOC_IF_ERROR_RETURN(soc_mem_scan_stop(unit));
    }

    sal_snprintf(soc->mem_scan_name, sizeof (soc->mem_scan_name),
		 "bcmMEM_SCAN.%d", unit);

    soc->mem_scan_rate = rate;
    soc->mem_scan_interval = interval;

    if (interval == 0) {
	return SOC_E_NONE;
    }

    if (soc->mem_scan_pid == SAL_THREAD_ERROR) {
	pri = soc_property_get(unit, spn_MEM_SCAN_THREAD_PRI, 50);
	soc->mem_scan_pid = sal_thread_create(soc->mem_scan_name,
					      SAL_THREAD_STKSZ,
					      pri,
					      _soc_mem_scan_thread,
					      INT_TO_PTR(unit));

	if (soc->mem_scan_pid == SAL_THREAD_ERROR) {
	    soc_cm_debug
		(DK_ERR,
		 "soc_mem_scan_start: Could not start mem_scan thread\n");
	    return SOC_E_MEMORY;
	}
    }

    return SOC_E_NONE;
}

/*
 * Function:
 * 	soc_mem_scan_stop
 * Purpose:
 *   	Stop memory scan thread
 * Parameters:
 *	unit - unit number.
 * Returns:
 *	SOC_E_XXX
 */

int
soc_mem_scan_stop(int unit)
{
    soc_control_t	*soc = SOC_CONTROL(unit);
    int			rv = SOC_E_NONE;
    soc_timeout_t	to;

    soc->mem_scan_interval = 0;		/* Request exit */

    /* check if needs support for sirius */
#ifdef BCM_SIRIUS_SUPPORT
    if (SOC_IS_SIRIUS(unit)) {
      return SOC_E_INTERNAL;
    }
#endif

    if (soc->mem_scan_pid != SAL_THREAD_ERROR) {
	/* Wake up thread so it will check the exit flag */
	sal_sem_give(soc->mem_scan_notify);

	/* Give thread a few seconds to wake up and exit */
#ifdef PLISIM
#	define	TOSEC	15
#else
#	define	TOSEC	5
#endif
	soc_timeout_init(&to, TOSEC * 1000000, 0);

	while (soc->mem_scan_pid != SAL_THREAD_ERROR) {
	    if (soc_timeout_check(&to)) {
		soc_cm_debug(DK_ERR,
			     "soc_mem_scan_stop: thread will not exit\n");
		rv = SOC_E_INTERNAL;
		break;
	    }
	}
    }

    return rv;
}

/*
 * Function:
 * 	_soc_mem_scan_thread (internal)
 * Purpose:
 *   	Thread control for L2 shadow table maintenance
 * Parameters:
 *	unit_vp - StrataSwitch unit # (as a void *).
 * Returns:
 *	Nothing
 * Notes:
 *	Exits when mem_scan_interval is set to zero and semaphore is given.
 */

STATIC void
_soc_mem_scan_thread(void *unit_vp)
{
    int			unit = PTR_TO_INT(unit_vp);
    soc_control_t	*soc = SOC_CONTROL(unit);
    int			rv;
    int			interval;
    int			chunk_size;
    int			entries_interval;
    int			entries_pass;
    uint32		*read_buf = NULL;
    uint32		mask[SOC_MAX_MEM_WORDS];
    soc_mem_t		mem;
    int			entry_dw;
    int			blk;
    int			idx, idx_count, i, dw;
    uint32		*cache;
    uint8		*vmap;

    chunk_size = soc_property_get(unit, spn_MEM_SCAN_CHUNK_SIZE, 256);

    read_buf = soc_cm_salloc(unit,
			     chunk_size * SOC_MAX_MEM_WORDS * 4,
			     "mem_scan_new");

    if (read_buf == NULL) {
	soc_cm_debug(DK_ERR,
		     "soc_mem_scan_thread: not enough memory, exiting\n");
        soc_event_generate(unit, SOC_SWITCH_EVENT_THREAD_ERROR, 
                           SOC_SWITCH_EVENT_THREAD_MEMSCAN, __LINE__, 
                           SOC_E_MEMORY);
	goto cleanup_exit;
    }

    /*
     * Implement the sleep using a semaphore timeout so if the task is
     * requested to exit, it can do so immediately.
     */

    entries_interval = 0;

    while ((interval = soc->mem_scan_interval) != 0) {
	entries_pass = 0;

	for (mem = 0; mem < NUM_SOC_MEM; mem++) {
	    if (!SOC_MEM_IS_VALID(unit, mem)) {
		continue;
	    }

	    entry_dw = soc_mem_entry_words(unit, mem);
	    soc_mem_datamask_get(unit, mem, mask);

	    SOC_MEM_BLOCK_ITER(unit, mem, blk) {
		if (soc->mem_scan_interval == 0) {
		    break;
		}

		for (idx = soc_mem_index_min(unit, mem);
		     idx <= soc_mem_index_max(unit, mem);
		     idx += idx_count) {
		    if (soc->mem_scan_interval == 0) {
			break;
		    }

		    idx_count = soc_mem_index_count(unit, mem) - idx;

		    if (idx_count > chunk_size) {
			idx_count = chunk_size;
		    }

		    if (entries_interval + idx_count > soc->mem_scan_rate) {
			idx_count = soc->mem_scan_rate - entries_interval;
		    }

		    MEM_LOCK(unit, mem);

    /*    coverity[negative_returns : FALSE]    */
		    cache = SOC_MEM_STATE(unit, mem).cache[blk];
		    vmap = SOC_MEM_STATE(unit, mem).vmap[blk];

		    if (cache == NULL) {
			/* Cache disabled (possibly since last iteration) */
			MEM_UNLOCK(unit, mem);
			continue;
		    }

		    /* Temporarily disable cache for raw hardware read */

		    SOC_MEM_STATE(unit, mem).cache[blk] = NULL;

		    rv = soc_mem_read_range(unit, mem, blk,
					    idx, idx + idx_count - 1,
					    read_buf);

		    SOC_MEM_STATE(unit, mem).cache[blk] = cache;
		    
		    if (rv < 0) {
                        soc_cm_debug(DK_ERR,"soc_mem_scan_thread: read failed: %s\n",
				     soc_errmsg(rv));
                        soc_event_generate(unit, SOC_SWITCH_EVENT_THREAD_ERROR, 
                                           SOC_SWITCH_EVENT_THREAD_MEMSCAN, 
                                           __LINE__, rv);
                        MEM_UNLOCK(unit, mem);
			    goto cleanup_exit;
			}
        
                    soc_cm_debug(DK_TESTS, "Scan: unit=%d %s.%s[%d-%d]\n", unit, 
                                 SOC_MEM_NAME(unit, mem), 
                                 SOC_BLOCK_NAME(unit, blk),
				 idx, idx + idx_count - 1);

		    for (i = 0; i < idx_count; i++) {
			if (!CACHE_VMAP_TST(vmap, idx + i)) {
			    continue;
			}
			for (dw = 0; dw < entry_dw; dw++) {
                            if (((read_buf[i * entry_dw + dw] ^ cache[(idx + i)
                                   * entry_dw + dw]) &
				 mask[dw]) != 0) {
				break;
			    }
			}
			if (dw < entry_dw) {
			    char errstr[80 + SOC_MAX_MEM_WORDS * 9];
                            soc_cm_debug(DK_WARN, "Memory error detected on unit %d "
                                         "in %s.%s[%d]\n", unit, 
                                         SOC_MEM_NAME(unit, mem),
                                         SOC_BLOCK_NAME(unit, blk), idx + i);
			    errstr[0] = 0;
			    for (dw = 0; dw < entry_dw; dw++) {
                                sal_sprintf(errstr + sal_strlen(errstr)," %08x", 
					    cache[(idx + i) * entry_dw + dw]);
        
                            }
			    soc_cm_debug(DK_WARN, "    WAS:%s\n", errstr);

			    errstr[0] = 0;
			    for (dw = 0; dw < entry_dw; dw++) {
                                sal_sprintf(errstr + sal_strlen(errstr), " %08x", 
                                            read_buf[i * entry_dw + dw]);
                            }
			    soc_cm_debug(DK_WARN, "    BAD:%s\n", errstr);

                            if ((rv = soc_mem_write (unit, mem, blk, idx + i, 
                                                     cache + (idx + i) * entry_dw)) < 0) {
                                soc_cm_debug(DK_WARN, "    CORRECTION FAILED: %s\n", 
                                             soc_errmsg(rv));
                                soc_event_generate(unit, 
                                                   SOC_SWITCH_EVENT_THREAD_ERROR, 
                                                   SOC_SWITCH_EVENT_THREAD_MEMSCAN,
                                                    __LINE__, rv);
				MEM_UNLOCK(unit, mem);
				goto cleanup_exit;
			    } else {
                                soc_cm_debug(DK_WARN, "    Corrected by writing back cached data\n");
                            }
                        } 
    
			entries_pass++;
		    }

		    MEM_UNLOCK(unit, mem);

		    entries_interval += idx_count;

		    if (entries_interval == soc->mem_scan_rate) {
			sal_sem_take(soc->mem_scan_notify, interval);
			entries_interval = 0;
		    }
		}
	    }
	}

	soc_cm_debug(DK_TESTS,
		     "Done: %d entries checked\n",
		     entries_pass);

	if (soc->mem_scan_interval != 0) {
	    /* Extra sleep in main loop, in case no caches are enabled. */
	    sal_sem_take(soc->mem_scan_notify, interval);
	    entries_interval = 0;
	}
    }

 cleanup_exit:

    if (read_buf != NULL) {
	soc_cm_sfree(unit, read_buf);
    }

    soc->mem_scan_pid = SAL_THREAD_ERROR;
    sal_thread_exit(0);
}

#endif	/* INCLUDE_MEM_SCAN */
