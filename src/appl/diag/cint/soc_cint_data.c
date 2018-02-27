/*
 * $Id: soc_cint_data.c 1.4.124.1 Broadcom SDK $
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
 * soc_cint_data.c
 *
 * Hand-coded support for a few SAL core routines. 
 *
 */
int soc_core_cint_data_not_empty; 
#include <sdk_config.h>

#if defined(INCLUDE_LIB_CINT)

#include <cint_config.h>
#include <cint_types.h>
#include <cint_porting.h>
#include <soc/property.h>
#include <soc/drv.h>


CINT_FWRAPPER_CREATE_RP1(int, int, 0, 0,
                         soc_init, \
                         int,int,unit,0,0);

CINT_FWRAPPER_CREATE_RP2(char*,char,1,0,
                         soc_property_get_str,
                         int,int,unit,0,0,
                         char*,char,name,1,0); 

CINT_FWRAPPER_CREATE_RP3(uint32,uint32,0,0,
                         soc_property_get,
                         int,int,unit,0,0,
                         char*,char,name,1,0,
                         uint32,uint32,def,0,0); 
CINT_FWRAPPER_CREATE_RP3(pbmp_t,pbmp_t,0,0,
                         soc_property_get_pbmp,
                         int,int,unit,0,0,
                         char*,char,name,1,0,
                         int,int,defneg,0,0); 

CINT_FWRAPPER_CREATE_RP3(pbmp_t,pbmp_t,0,0,
                         soc_property_get_pbmp_default,
                         int,int,unit,0,0,
                         char*,char,name,1,0,
                         pbmp_t,pbmp_t,def,0,0); 

CINT_FWRAPPER_CREATE_RP3(char*,char,1,0,
                         soc_property_port_get_str,                         
                         int,int,unit,0,0,
                         int,int,port,0,0,
                         char*,char,name,1,0);

CINT_FWRAPPER_CREATE_RP4(uint32,uint32,0,0,
                         soc_property_port_get,
                         int,int,unit,0,0,
                         int,int,port,0,0,
                         char*,char,name,1,0,
                         uint32,uint32,def,0,0); 

CINT_FWRAPPER_CREATE_RP5(uint32,uint32,0,0,
                         soc_property_suffix_num_get,
                         int,int,unit,0,0,
                         int,int,tc,0,0,
                         char*,char,name,1,0,
                         char*,char,suffix,1,0,
                         uint32,uint32,def,0,0); 

CINT_FWRAPPER_CREATE_RP4(uint32,uint32,0,0,
                         soc_property_cos_get,
                         int,int,unit,0,0,
                         soc_cos_t,soc_cos_t,cos,0,0,
                         char*,char,name,1,0,
                         uint32,uint32,def,0,0); 

static cint_function_t __cint_soc_functions[] = 
    {
        CINT_FWRAPPER_ENTRY(soc_init), 
        CINT_FWRAPPER_ENTRY(soc_property_get_str), 
        CINT_FWRAPPER_ENTRY(soc_property_get), 
        CINT_FWRAPPER_ENTRY(soc_property_get_pbmp), 
        CINT_FWRAPPER_ENTRY(soc_property_get_pbmp_default), 
        CINT_FWRAPPER_ENTRY(soc_property_port_get_str),
        CINT_FWRAPPER_ENTRY(soc_property_port_get),
        CINT_FWRAPPER_ENTRY(soc_property_suffix_num_get),
        CINT_FWRAPPER_ENTRY(soc_property_cos_get),
        CINT_ENTRY_LAST
        
    }; 
    


cint_data_t soc_cint_data = 
    {
        NULL,
        __cint_soc_functions,
        NULL, 
        NULL, 
        NULL, 
        NULL, 
        NULL
    }; 

#endif /* INCLUDE_LIB_CINT */

    
