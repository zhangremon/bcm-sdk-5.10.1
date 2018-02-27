/******************************************************************************
 *
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
 * $Id: defs_vn.h 1.5.60.1 Broadcom SDK $
 *
 ******************************************************************************/

/*********************************************************************
 Beg of File: defs_vn.h
*********************************************************************/

#ifndef defs_vn__h_defined
#define defs_vn__h_defined

#include <shared/alloc.h>
#include <sal/core/libc.h>

#ifndef ASSERT_VN
#include <assert.h>
#define ASSERT_VN assert                                                                               
#endif /*ASSERT_VN*/ 

#ifdef NJ_BUILD
#define MALLOC_VN host_malloc
#define FREE_VN host_free
#endif /*NJ_BUILD*/                                                                                          

#ifndef MALLOC_VN
#ifndef __KERNEL__
#include <stdlib.h>/*for the exit()*/
#endif
#define MALLOC_VN sal_alloc
#endif /*MALLOC_VN*/                                                                                           

#ifndef FREE_VN
#ifndef __KERNEL__
#include <stdlib.h>/*for the exit()*/
#endif
#define FREE_VN sal_free
#endif /*MALLOC_VN*/ 

#ifndef STRLEN_VN
#ifndef __KERNEL__
#include <string.h>
#endif
#define STRLEN_VN strlen
#endif  /*STRLEN_VN*/

#ifndef STRNCMP_VN
#ifndef __KERNEL__
#include <string.h>
#endif
#define STRNCMP_VN strncmp 
#endif  /*STRNCMP_VN*/

#ifndef STRNCPY_VN
#ifndef __KERNEL__
#include <string.h>
#endif
#define STRNCPY_VN strncpy
#endif  /*STRNCPY_VN*/

#ifndef MEMSET_VN
#ifndef __KERNEL__
#include <string.h>
#endif
#define MEMSET_VN sal_memset
#endif /*MEMSET_VN*/                                                                                         

#ifndef BOOL_VN
#define BOOL_VN int
#endif /*BOOL_VN*/

#ifndef TRUE_VN
#define TRUE_VN 1
#endif /*TRUE_VN*/

#ifndef FALSE_VN
#define FALSE_VN 0
#endif /*FALSE_VN*/

#ifndef ITEMS_VN
#define ITEMS_VN(x) (sizeof(x)/sizeof(x[0]))
#endif  /*ITEMS_VN*/

#ifndef MIN_VN
#define MIN_VN(x,y) ((x)>(y)?y:x)
#endif  /*MIN_VN*/

#ifndef MAX_VN
#define MAX_VN(x,y) ((x)<(y)?y:x)
#endif  /*MAX_VN*/

#ifndef STRMAX_VN
#define STRMAX_VN 128
#endif /*STRMAX_VN*/

#endif /* defs_vn__h_defined */

/*********************************************************************
 End of File: defs_vn.h
*********************************************************************/
