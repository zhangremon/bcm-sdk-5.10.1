/*
 * $Id: cmd_cint.c 1.15.44.2 Broadcom SDK $
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
 *
 */
#include <sdk_config.h>

int cmd_cint_c_not_empty; 

#ifdef INCLUDE_LIB_CINT


#if !defined(__KERNEL__) && !defined(VXWORKS)
#include <stdlib.h>
#endif

#include "cint_sdk_atomics.h"
#include <cint_interpreter.h>
#include <appl/diag/autocli.h>
#include <sal/core/alloc.h>
#include <sal/core/libc.h>
#include <sal/appl/io.h>
#include <appl/diag/shell.h>
#include <appl/diag/parse.h>
#include <cint_porting.h>

char cmd_cint_usage[] = "cint usage goes here\n"; 


/* bcm_cint_data.c */
extern cint_data_t bcm_cint_data; 

/* sal_cint_data.c */
extern cint_data_t sal_cint_data; 

/* properties.c */
extern void soc_cint_property_vars_init(void); 

#if defined(BCM_EA_SUPPORT)
/* ea_cint_data */
extern cint_data_t ea_cint_data;
#endif

/* soc_cint_data.c */
extern cint_data_t soc_cint_data;

#ifdef BCM_FE2000_SUPPORT
/* g2p3_cint_data.c */
extern cint_data_t g2p3_cint_data;

#ifdef INCLUDE_TEST
/* sbxpkt_cint_data.c */
extern cint_data_t sbxpkt_cint_data;
#endif
#endif

/* Custom functions */
int bshell(int unit, char* cmd)
{
    return sh_process_command(unit, cmd); 
}
CINT_FWRAPPER_CREATE_RP2(int,int,0,0,
                  bshell, 
                  int,int,unit,0,0,
                  char*,char,cmd,1,0); 

static cint_function_t __cint_cmd_functions[] = 
    {
        CINT_FWRAPPER_ENTRY(bshell), 
        
        CINT_ENTRY_LAST
    }; 

static cint_constants_t __cint_cmd_constants[] = 
    {   
        { "TRUE", 1 }, 
        { "FALSE", 0 }, 
        { NULL }
    }; 

cint_data_t cmd_cint_data = 
    {
        NULL,
        __cint_cmd_functions,
        NULL, 
        NULL, 
        NULL, 
        __cint_cmd_constants, 
        NULL
    }; 


static int
__cint_sdk_data_init(void)
{
    cint_interpreter_add_atomics(cint_sdk_atomics); 
    cint_interpreter_add_data(&cint_sdk_data, NULL); 
    cint_interpreter_add_data(&bcm_cint_data, NULL);     
    cint_interpreter_add_data(&sal_cint_data, NULL); 
    cint_interpreter_add_data(&soc_cint_data, NULL); 
#ifdef BCM_FE2000_SUPPORT
    cint_interpreter_add_data(&g2p3_cint_data, NULL);
#ifdef INCLUDE_TEST
    cint_interpreter_add_data(&sbxpkt_cint_data, NULL);
#endif
#endif
#if defined(BCM_EA_SUPPORT)
    cint_interpreter_add_data(&ea_cint_data, NULL);
#endif
    cint_interpreter_add_data(&cmd_cint_data, NULL); 
    soc_cint_property_vars_init(); 
    return 0; 
}

static int
__cint_event_handler(void* cookie, cint_interpreter_event_t event)
{
    switch(event)
        {
        case cintEventReset:
            {
                /*
                 * Interpreter has reset. Re-register our data
                 */
                __cint_sdk_data_init(); 
                break;
            }
        default:
            {
                /* Nothing */
                break;
            }   
        }       
    return 0; 
}

static void
__cint_init_path(void)
{
    char *path;
#if defined(VXWORKS)
    path = var_get("CINT_INCLUDE_PATH");
#elif !defined(__KERNEL__)
    path = getenv("CINT_INCLUDE_PATH");
#else
    path = NULL;
#endif

    if (path) {
        cint_interpreter_include_set(path); 
    }
}

static int 
__cint_initialize(void)
{ 
    cint_interpreter_init(); 
    cint_interpreter_event_register(__cint_event_handler, NULL);
    __cint_init_path();
    __cint_sdk_data_init(); 
    
    return 0; 
}
                
cmd_result_t
cmd_cint(int unit, args_t* a)
{
    static int init = 0; 
    char* s; 

    if(init == 0) {
        __cint_initialize(); 
        init = 1; 
    }
    
    if(ARG_CUR(a)) {
        /* Load file */
        FILE* fp; 
        s = ARG_GET(a); 
        
        if((fp = sal_fopen(s, "r")) != NULL) {
                
            int argc = 0; 
            char* argv[16]; 
                
            sal_memset(argv, 0, sizeof(argv)); 
            while( (argv[argc] = ARG_GET(a)) ) {
                argc++; 
            }   

            cint_interpreter_parse(fp, NULL, argc, argv); 
            sal_fclose(fp); 
        }   
        else {
            printk("error: could not open file '%s'\n", s); 
        }       
    }   
    else {
        printk("Entering C Interpreter. Type 'exit;' to quit.\n\n"); 
        cint_interpreter_parse(NULL, "cint> ", 0, NULL); 
    }
    return 0; 
}


#endif /* INCLUDE_LIB_CINT */
