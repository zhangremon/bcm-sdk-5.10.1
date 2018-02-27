/*
 * $Id: cint_eval_ast_cint.c 1.12 Broadcom SDK $
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
 * File:        cint_eval_ast_cint.c
 * Purpose:     CINT AST node evaluators
 *
 */

#include "cint_eval_ast_cint.h"
#include "cint_porting.h"
#include "cint_interpreter.h"
#include "cint_internal.h"
#include "cint_eval_asts.h"

/*
 * Command Handlers
 */

#define CCINT_PRINTF CINT_PRINTF("cint> "); CINT_PRINTF


#if CINT_CONFIG_INCLUDE_CINT_LOAD == 1
#include <dlfcn.h>

void
cint_ast_cint_cmd_unload(void *handle)
{
    if (handle && dlclose(handle)) {
        CCINT_PRINTF("%s", dlerror()); 
    }
}
#endif

cint_variable_t* 
__cint_ast_cint_cmd_load(cint_ast_t* ast, const char* argv[])
{
#if CINT_CONFIG_INCLUDE_CINT_LOAD == 0
    cint_ast_error(ast, CINT_E_UNSUPPORTED, "dynamic library loading support was not compiled in."); 
#else
    void* handle;
    const char** s = NULL; 

    for(s = argv; *s; s++) {

        if((handle = dlopen(*s, RTLD_NOW)) == NULL) {
            cint_ast_error(ast, CINT_E_BAD_AST, "loading library '%s': %s", *s, dlerror()); 
        }
        else {
#define STRINGIFY0(x) #x
#define STRINGIFY(x) STRINGIFY0(x)
            void* data;             
            char *sym = STRINGIFY(CINT_LOAD_DATA_SYMBOL);

            if((data = dlsym(handle, sym)) == NULL) {
                cint_ast_error(ast, CINT_E_BAD_AST, "%s", dlerror()); 
                dlclose(handle); 
                return NULL; 
            }       
            else {
                cint_interpreter_add_data(*(cint_data_t**)data, handle); 
            }
        }
    }
#endif /* CINT_CONFIG_INCLUDE_CINT_LOAD */

    return NULL; 
}

cint_variable_t*
__cint_ast_cint_cmd_typecheck(cint_ast_t* ast, const char* argv[])
{
    cint_datatype_checkall(1); 
    return NULL; 
}

cint_variable_t*
__cint_ast_cint_cmd_var(cint_ast_t* ast, const char* argv[])
{
    cint_variable_t* v = NULL; 

    if(*argv == NULL) {
        CCINT_PRINTF("no variable specified"); 
        return NULL; 
    }

    v = cint_variable_find(*argv, 0); 

    if(v) {
        CCINT_PRINTF("variable '%s' is type '%s' @ %p size=%d\n", v->name, cint_datatype_format(&v->dt,0), v->data, v->size);  
    }
    else {        
        CCINT_PRINTF("variable '%s' not found\n", *argv); 
    }
    return NULL; 
}       


static int
__cint_list_datatype_traverse(void* cookie, const cint_datatype_t* dt)
{
    const char* substr = (const char*)cookie; 
    int flags = dt->flags; 

    if(flags & CINT_DATATYPE_F_TYPEDEF) {
        if(substr && CINT_STRSTR(dt->type, substr) == NULL) {
            /* No Match */
            return 0; 
        }
        CINT_PRINTF("%s typedef for %s ", dt->type, cint_datatype_format_pd(&dt->desc, 0)); 
        flags &= ~CINT_DATATYPE_F_TYPEDEF; 
    }
    else if (flags & CINT_DATATYPE_F_CONSTANT) {
        if(substr && CINT_STRSTR(dt->basetype.cp->name, substr) == NULL) {
            /* No Match */
            return 0; 
        }
    }   
    else {
        if(substr && CINT_STRSTR(dt->desc.basetype, substr) == NULL) {
            /* No Match */
            return 0; 
        }
        CINT_PRINTF("%s ", dt->desc.basetype); 
    }

    switch(flags) 
        {
        case CINT_DATATYPE_F_ATOMIC: CINT_PRINTF("(atomic)"); break; 
        case CINT_DATATYPE_F_STRUCT: CINT_PRINTF("(struct)"); break; 
        case CINT_DATATYPE_F_ENUM:     CINT_PRINTF("(enum)"); break; 
        case CINT_DATATYPE_F_FUNC_DYNAMIC:     
        case CINT_DATATYPE_F_FUNC:   CINT_PRINTF("(function)"); break; 
        case CINT_DATATYPE_F_CONSTANT: CINT_PRINTF("%s (constant)", dt->basetype.cp->name); break; 
        case CINT_DATATYPE_F_FUNC_POINTER: CINT_PRINTF("(function pointer)"); break; 
        case CINT_DATATYPE_F_MACRO: CINT_PRINTF("(macro)"); break; 
        case CINT_DATATYPE_F_ITERATOR: CINT_PRINTF("(iterator)"); break; 
        default:  CINT_PRINTF("(unknown datatype flags 0x%x)", flags); break; 
        }
    
    CINT_PRINTF("\n"); 
    return 0; 
}

cint_variable_t*
__cint_ast_cint_cmd_list(cint_ast_t* ast, const char* argv[])
{
    const char** arg = argv; 
    unsigned dtflags = 0; 
    const char* str = NULL; 

    for(arg = argv; *arg; arg++) {
        if(!CINT_STRCMP(*arg, "atomic")) {
            dtflags |= CINT_DATATYPE_F_ATOMIC; 
        }
        else if(!CINT_STRCMP(*arg, "struct")) {
            dtflags |= CINT_DATATYPE_F_STRUCT;
        }
        else if(!CINT_STRCMP(*arg, "enum")) {
            dtflags |= CINT_DATATYPE_F_ENUM; 
        }       
        else if(!CINT_STRCMP(*arg, "fp")) {
            dtflags |= CINT_DATATYPE_F_FUNC_POINTER; 
        }       
        else if(!CINT_STRCMP(*arg, "function")) {
            dtflags |= CINT_DATATYPE_F_FUNC|CINT_DATATYPE_F_FUNC_DYNAMIC; 
        }
        else if(!CINT_STRCMP(*arg, "constant")) {
            dtflags |= CINT_DATATYPE_F_CONSTANT; 
        }
        else if(!CINT_STRCMP(*arg, "macro")) {
            dtflags |= CINT_DATATYPE_F_MACRO; 
        }
        else if(!CINT_STRCMP(*arg, "iterator")) {
            dtflags |= CINT_DATATYPE_F_ITERATOR; 
        }
        else {
            if(str != NULL) {
                cint_ast_error(ast, CINT_E_BAD_AST, "only one search substring may be specified"); 
                return NULL; 
            }
            else {
                str = *arg; 
            }
        }
    }
    
    /* 
     * Traverse all datatypes 
     */
    cint_datatype_traverse(dtflags, __cint_list_datatype_traverse, (void*)str); 
    
    return NULL; 
}



/*
 * Command Handler Dispatch
 */
struct {
    const char* cmd; 
    cint_variable_t* (*handler)(cint_ast_t* ast, const char* args[]); 
} __handler_table[] = {

#define CINT_AST_CINT_CMD_ENTRY(_e) { #_e, __cint_ast_cint_cmd_##_e }, 
#include "cint_eval_ast_cint.h"
    { NULL }
}; 

cint_variable_t* 
cint_eval_ast_Cint(cint_ast_t* ast)
{
    int i; 
    cint_ast_t* cmd; 
    cint_ast_t* args;    
    const char* argv[64] = { NULL }; 
        
    if(ast == NULL) {
        return NULL; 
    }   
    
    cmd = ast->utype.cint.arguments; 
    
    if(cmd == NULL) {
        return NULL; 
    }
    
    /* Process string arguments */
    for(i = 0, args = cmd->next; args; args = args->next, i++) {
        switch(args->ntype)
            {
            case cintAstIdentifier:     argv[i] = args->utype.identifier.s; break; 
            case cintAstString:         argv[i] = args->utype.string.s; break; 
            case cintAstType:           argv[i] = args->utype.type.s; break; 
            default: break; 
            }   
    }   
    argv[i] = NULL; 
    
    for(i = 0; __handler_table[i].cmd; i++) {
        if(!CINT_STRCMP(cmd->utype.identifier.s, __handler_table[i].cmd)) {
            return __handler_table[i].handler(cmd, argv); 
        }
    }   

    return NULL; 
}
