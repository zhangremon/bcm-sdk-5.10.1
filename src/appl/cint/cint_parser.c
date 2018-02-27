/*
 * $Id: cint_parser.c 1.26 Broadcom SDK $
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
 * File:        cint_parser.c
 * Purpose:     CINT parser
 *
 */

#include "cint_parser.h"
#include "cint_porting.h"

/*
 * These are extern'ed manually to avoid conflicts between the scanner and parser headers. 
 */
extern int cint_c_lex_init(void** scanner); 
int cint_c_scanner_finish(void *scanner);

extern void* cint_c_pstate_new(void); 
extern int   cint_c_pull_parse(void* handle, void* scanner, void* ctrl); 
extern int   cint_c_pstate_delete(void* handle); 


/*
 * Create a C Parser instance
 */
cint_cparser_t* 
cint_cparser_create(void)
{
    cint_cparser_t* cp = CINT_MALLOC(sizeof(*cp)); 
    
    if(cp == NULL) {
        return NULL; 
    }

    CINT_MEMSET(cp, 0, sizeof(*cp)); 

    /* Create the scanner instance */
    cint_c_lex_init(&cp->scanner); 
    
    if(cp->scanner == NULL) {
        cint_cparser_destroy(cp); 
        return NULL; 
    }

    /* Create a parser instance */
    cp->parser = cint_c_pstate_new(); 
    
    if(cp->parser == NULL) {
        cint_cparser_destroy(cp); 
        return NULL; 
    }
    
    return cp; 
}

/*
 * Destroy a C Parser instance
 */
int
cint_cparser_destroy(cint_cparser_t* cp)
{
    if (cp == NULL) {
        return 0;
    }

    if (cp->scanner) {
        cint_c_scanner_finish(cp->scanner);
    }

    if (cp->parser) {
        cint_c_pstate_delete(cp->parser);
    }

    CINT_FREE(cp);

    return 0;
}

int 
cint_cparser_start_handle(cint_cparser_t* cp, void* handle)
{
    extern int cint_c_scanner_start(void*, void*); 
    
    if(cp == NULL) {
        return -1; 
    }

    cint_c_scanner_start(handle, cp->scanner); 
    
    return 0; 
}

int 
cint_cparser_start_string(cint_cparser_t* cp, const char* string)
{
    extern void *cint_c__scan_string(const char *str, void *scanner);

    if (cp == NULL) {
        return -1;
    }

    cint_c__scan_string(string, cp->scanner);

    return 0;
}

cint_ast_t* 
cint_cparser_parse(cint_cparser_t* cp)
{
    cint_ast_t* ast; 

    if(cp == NULL) {
        return NULL; 
    }   
   
    cp->error = cint_c_pull_parse(cp->parser, cp->scanner, cp); 
    ast = cp->result; 
    cp->result = NULL; 

    return ast; 
}

cint_ast_t* 
cint_cparser_parse_string(const char* string)
{
    int status; 
    cint_ast_t* result = NULL; 
    cint_cparser_t* cp = cint_cparser_create(); 
    
    cint_cparser_start_string(cp, string); 

    for(;;) {
        
        status = cint_c_pull_parse(cp->parser, cp->scanner, cp); 
        if(status == 0) {
            /* Successfully parsed a translation unit */
            
            if(cp->result == NULL) {
                /* EOF */
                break; 
            }
            else {
                if(result == NULL) {
                    result = cp->result; 
                }
                else {
                    cint_ast_append(result, cp->result); 
                }
            }
        }
        else {
            /* Error */
            break; 
        }
    }   

    cint_cparser_destroy(cp); 
    return result; 
}

int
cint_cparser_error(cint_cparser_t* cp)
{
    return cp->error;
}
