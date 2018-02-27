/*
 * $Id: sbx_thin.c 1.9 Broadcom SDK $
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
 * Implementation of SBX thin_* services
 */
/* SBX distribution includes */

#include <assert.h>
#include <soc/error.h>
#include <soc/cm.h>
#include <soc/debug.h>
#include <soc/sbx/sbx_drv.h>
#include <shared/util.h>

#include <soc/sbx/sbWrappers.h>
#include <soc/sbx/glue.h>

#ifdef VXWORKS
#include <vxWorks.h>
#include <sysLib.h>
#endif /* VXWORKS */

#ifndef BE_HOST
#define BE_HOST 0
#endif

static int _thin_disable[SOC_MAX_NUM_DEVICES]={0};

void thin_disable(sbhandle addr, int disable){
  int unit = (int)addr;
  _thin_disable[unit]=disable;
}

sbreg
thin_read32_raw(sbhandle addr, uint32_t offs)
{
    int unit = (int) addr;
    sbreg reg=0;
    reg = _thin_disable[unit]?0xffffffff:CMREAD(unit, offs);
    if (soc_cm_debug_check(DK_REG))
      soc_cm_print("%s: unit=%d addr=0x%08x data=0x%08x\n", FUNCTION_NAME(), (int)addr, offs, reg);
    return reg;
}

sbreg
thin_read32(sbhandle addr, uint32_t offs)
{
    int unit = (int) addr;
    sbreg v = _thin_disable[unit]?0xffffffff:CMREAD(unit, offs);

    /* TEMPORARY */
    if (unit==4){
      int jj=0;
      jj++;
    }

    if ((CMVEC(unit).big_endian_pio && !BE_HOST)
        || (!CMVEC(unit).big_endian_pio && BE_HOST)) {
        v = _shr_swap32(v);
    }

    if (soc_cm_debug_check(DK_REG))
      soc_cm_print("%s: unit=%d addr=0x%08x data=0x%08x\n", FUNCTION_NAME(), (int)addr, offs, v);

    return v;
}

void
thin_write32_raw(sbhandle addr, uint32_t offs, sbreg data)
{
    int unit = (int) addr;
    
    if (soc_cm_debug_check(DK_REG))
      soc_cm_print("%s: unit=%d addr=0x%08x data=0x%08x\n", FUNCTION_NAME(), (int)addr, offs, data);

    if (!_thin_disable[unit])
      CMWRITE(unit, offs, data);
#ifdef __powerpc__
    __asm__ __volatile__ ("eieio; sync");
#endif
}

void
thin_write32(sbhandle addr, uint32_t offs, sbreg data)
{
    int unit = (int) addr;

    if (soc_cm_debug_check(DK_REG))
      soc_cm_print("%s: unit=%d addr=0x%08x data=0x%08x\n", FUNCTION_NAME(), (int)addr, offs, data);

    if ((CMVEC(unit).big_endian_pio && !BE_HOST)
        || (!CMVEC(unit).big_endian_pio && BE_HOST)) {
        data = _shr_swap32(data);
    }
    if (!_thin_disable[unit])
      CMWRITE(unit, offs, data);
#ifdef __powerpc__
    __asm__ __volatile__ ("eieio; sync");
#endif
}

void
thin_no_write(sbhandle addr, uint32_t offs, sbreg data)
{
    if (soc_cm_debug_check(DK_REG))
      soc_cm_print("%s: unit=%d addr=0x%08x write data(0x%08x) abort due to easy reload or warmboot\n", FUNCTION_NAME(), (int)addr, offs, data);
}

void
thin_delay(uint32_t nanosecs)
{
    uint32_t usec = nanosecs / 1000;

#ifdef VXWORKS
	/*
	 * When usec is larger than 1 vxworks tick period (in useconds)
	 * call sal_usleep(), otherwise sal_udelay().
	 * The difference is sal_usleep() calls taskDelay() to swap itself
	 * out of current task status while sal_udelay() does busy spin.
	 * -- hqian 08/30/2007
	 */
	if (usec > SECOND_USEC / sysClkRateGet()) {
		sal_usleep(usec);
	} else {
		/*
		 * usec == 0 seems a bad value, don't know why.
		 * sal_udelay would delay forever. -- hqian 08/30/2007
		 */
		if (usec == 0)
			usec = 1;
		sal_udelay(usec);
	}
#else
    if (usec < 10) {
        usec = 10;
    }
    sal_usleep(usec);
#endif
}

sbStatus_t
gen_thin_malloc(void *clientData, sbFeMallocType_t type, uint32_t size,
                void **memoryP, sbDmaMemoryHandle_t *dmaHandleP,
                uint8_t contig)
{
    int unit = (int) clientData;
    void *memory;

#ifdef PLISIM
    if (SAL_BOOT_PLISIM) {
	type = 0xf;
    }
    if (dmaHandleP) {
        dmaHandleP->handle = (void *)unit;
    }

    /* force default - thin_malloc is being called inconsistently
     *                 ignoring the ClientData set especially from
     *                 sbFe2000TableMgrRegister
     */
#endif

    switch (type) {
    case SB_ALLOC_CLS_DMA:
        assert(dmaHandleP);
        dmaHandleP->handle = (void *) unit;
        memory = soc_cm_salloc(unit, size, "SBX cls");
        break;
    case SB_ALLOC_L2_DMA:
        assert(dmaHandleP);
        dmaHandleP->handle = (void *) unit;
        memory = soc_cm_salloc(unit, size, "SBX L2");
        break;
    case SB_ALLOC_OTHER_DMA:
        assert(dmaHandleP);
        dmaHandleP->handle = (void *) unit;
        memory = soc_cm_salloc(unit, size, "SBX other");
        break;
    case SB_ALLOC_IPV4_DMA:
        assert(dmaHandleP);
        dmaHandleP->handle = (void *) unit;
        memory = soc_cm_salloc(unit, size, "SBX IPv4");
        break;
    case SB_ALLOC_LPM_DMA:
        assert(dmaHandleP);
        dmaHandleP->handle = (void *) unit;
        memory = soc_cm_salloc(unit, size, "SBX LPM");
        break;
    default:
        memory = sal_alloc(size, "SBX MALLOC");
    }

    if (!memory) {
        return SB_MALLOC_FAILED;
    }
    *memoryP = memory;

#ifdef SBX_POLARIS_MEMSET
    sal_memset(memory, 0, size);
#endif

    return SB_OK;
}

sbStatus_t
thin_malloc(void *clientData, sbFeMallocType_t type, uint32_t size,
          void **memoryP, sbDmaMemoryHandle_t *dmaHandleP)
{
    return (gen_thin_malloc(clientData, type, size, memoryP, dmaHandleP, 0));
}

sbStatus_t
thin_contig_malloc(void *clientData, sbFeMallocType_t type, uint32_t size,
          void **memoryP, sbDmaMemoryHandle_t *dmaHandleP)
{
    return (gen_thin_malloc(clientData, type, size, memoryP, dmaHandleP, 1));
}


sbStatus_t
thin_free(void *clientData, sbFeMallocType_t type, uint32_t size,
          void *memory, sbDmaMemoryHandle_t dmaHandle)
{
    int unit = (int) clientData;
    
#ifdef PLISIM
    if (SAL_BOOT_PLISIM) {
	type = 0xf; 
    }
    /* force default - thin_malloc is being called inconsistently
     *                 ignoring the ClientData set especially from
     *                 sbFe2000TableMgrRegister
     */
#endif


    switch (type) {
    case SB_ALLOC_CLS_DMA:
    case SB_ALLOC_L2_DMA:
    case SB_ALLOC_OTHER_DMA:
    case SB_ALLOC_IPV4_DMA:
    case SB_ALLOC_LPM_DMA:
        soc_cm_sfree(unit, memory);
        break;
    default:
        sal_free(memory);
    }

    return SB_OK;
}

sbStatus_t
thin_getHba(sbDmaMemoryHandle_t mHandle, uint32_t *address, uint32_t words,
	  sbFeDmaHostBusAddress_t *hbaP)
{
    int unit = (int) mHandle.handle;

    (*hbaP) = (sbFeDmaHostBusAddress_t) soc_cm_l2p(unit, address);
    return SB_OK;
}

sbStatus_t
thin_writeSync(sbDmaMemoryHandle_t mHandle, uint32_t *address, uint32_t words)
{
    int unit = (int) mHandle.handle;
    int rv;

    rv = soc_cm_sflush(unit, address,  WORDS2BYTES(words));
    assert(rv == SOC_E_NONE);

    return SB_OK;
}

sbStatus_t
thin_readSync(sbDmaMemoryHandle_t mHandle, uint32_t *address, uint32_t words)
{
    int unit = (int) mHandle.handle;
    int rv;

    rv = soc_cm_sinval(unit, address,  WORDS2BYTES(words));
    assert(rv == SOC_E_NONE);

    return SB_OK;
}

int
thin_isrSync(void *data)
{
 
    return sal_splhi();
}

void
thin_isrUnsync(void *data, int token)
{
    sal_spl(token);
}

int
thin_get_pagesize(sbhandle hdl)
{
    /*
     * Doesn't really matter because we do contig malloc of DMAable RAM
     * but making it small should reduce the DMAable RAM allocation
     * Gu 1.8.5b (and probably most fe1k ILibs) have a bug where they require
     * at least 3K pagesize (the classifier compiler barfs otherwise)
     */
    return 4 * 1024;
}

void
sb_log_function (int level, const char* fmt, ...)
{
    /*
     * TODO: could actually implement this, but for now, no
     * printing from SBX libraries
     */
}

/*-------------------------------------------------------------------*
 * thin_host_malloc()
 *
 * Wrapper the thin_malloc() for host calls. There are several
 * fields which do nothing during host-only memory allocation.
 * Managing this state, and performing the requisite error
 * handling is, therefore, pointless. This function makes the
 * system-wide function appear as a normal standard-C malloc()
 *
 * size:  Size in bytes
 *
 * returns: Pointer to allocated memory
 *-------------------------------------------------------------------*/
void*
thin_host_malloc(uint32_t size)
{
  sbStatus_t status;
  void *vp;

  status = thin_malloc(0, SB_ALLOC_INTERNAL, size, &vp, 0);
  if (status != SB_OK)
    return 0;
   return vp;
}

/*-------------------------------------------------------------------*
 * thin_host_free()
 *
 * Wrapper the thin_free() for host calls. There are several
 * fields which do nothing during host-memory de-allocation.
 * Managing this state, and performing the requisite error
 * handling is, therefore, pointless. This function makes the
 * system-wide function appear as a normal standard-C free()
 * 
 * vp : Pointer to allocated memory.
 * 
 * returns: nothing
 *-------------------------------------------------------------------*/
void
thin_host_free(void *vp)
{
  sbDmaMemoryHandle_t dummy;
  dummy.handle = NULL;
  thin_free(0, SB_ALLOC_INTERNAL, 0, vp, dummy);
}
