/*
 * $Id: cint_scanner.l 1.29 Broadcom SDK $
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
 * File:        cint_scanner.l
 * Purpose:     CINT Flex input file
 *
 */

D			[0-9]
L			[a-zA-Z_]
H			[a-fA-F0-9]
E			[Ee][+-]?{D}+
FS			(f|F|l|L)
IS			(u|U|l|L)*


%top {

#include "cint_config.h"
#include "cint_ast.h"
#include "cint_parser.h"

}

%{

#include "cint_config.h"

#ifdef FILENAME_MAX
#define __INCLUDE_MAX_FILENAME FILENAME_MAX
#else
#define __INCLUDE_MAX_FILENAME 128
#endif

#ifndef CINT_MAX_INCLUDE_DEPTH
#define CINT_MAX_INCLUDE_DEPTH 64
#endif

struct cint_extra_type_s {
    char currentFileName[__INCLUDE_MAX_FILENAME+1];
    char *includedFromFile;
    int includedFromLine;
};

#define YY_EXTRA_TYPE	struct cint_extra_type_s *

#define YY_NO_INPUT

#if CINT_CONFIG_INCLUDE_PARSER == 1

#include <stdio.h>
#include "cint_ast.h"
#include "cint_c.tab.h"
#include "cint_interpreter.h"
#include "cint_porting.h"
#include <string.h>

int check_type(yyscan_t scanner); 

int cint_scanner_prompt = 1; 
int cint_scanner_echo = 0; 

#define NOPROMPT() cint_scanner_prompt=0

struct yyguts_t; 
void __yy_input(char* buf, int* result, int max_size, yyscan_t scanner); 

#define YY_INPUT(buf, result, max_size) __yy_input((buf), &(result), (max_size), yyscanner)

%}

/* Rentrant Scanner */
%option yylineno
%option nounput
%option reentrant
%option bison-bridge
%option bison-locations
%option prefix="cint_c_"

/* Exclusive condition while processing an include directive */
%x incl comment

%%
	int commentNesting = 0;

#include                BEGIN(incl);

<incl>[ \t]+    /* Eat whitespace between include and filename */
<incl>[^ \t\n]+ {
    /* yytext is now the include filename */

#if CINT_CONFIG_INCLUDE_XINCLUDE == 0

    CINT_PRINTF
      ("Cannot include '%s': #include support is not available in this configuration\n",
       yytext);
    BEGIN(INITIAL);

#else

    FILE *cyyin = yyin; /* Save current input file. */
    int len;
    char* fpath = NULL; 
    char fname[__INCLUDE_MAX_FILENAME];

    fname[sizeof(fname) - 1] = 0;

    /* Allow quotes and angle brackets in include path */
    if (yytext[0] == '\"' || yytext[0] == '<') {
        CINT_STRNCPY(fname, yytext + 1, sizeof(fname) - 1);
    } else {
        CINT_STRNCPY(fname, yytext, sizeof(fname) - 1);
    }
    len = CINT_STRLEN(fname);
    if (fname[len - 1] == '\"' || fname[len - 1] == '>') {
        fname[len - 1] = 0;
    }
    /*
     * The scanner will allow us to push new files onto the stack until
     * we run out of memory. In reality, we'll probably reach the process
     * limit for maximum opened files first. In either case this way more
     * open files than makes sense. It's more likely that we're run into
     * some sort of recursive include condition. That being the case, we'll
     * check against an abitrary maximum include depth here.
     */
    if ((CINT_MAX_INCLUDE_DEPTH - 1) > yyg->yy_buffer_stack_top) {
        if (cint_interpreter_include_get(fname, &fpath) == CINT_E_NONE) {
            if (fpath) {
                yyin = CINT_FOPEN(fpath, "r");
            } else {
                yyin = CINT_FOPEN(fname, "r");
            }
        } else {
            yyin = 0;
        }

        if (yyin == 0) {
            CINT_PRINTF("Cannot open include file '%s'\n", fpath ? fpath : fname);
            yyin = cyyin;
        } else {
            struct cint_extra_type_s *extra = cint_c_get_extra(yyscanner);

            if (extra) {
                struct cint_extra_type_s *lastInclude = &extra[yyg->yy_buffer_stack_top];
                struct cint_extra_type_s *newInclude = &extra[yyg->yy_buffer_stack_top + 1];
                CINT_STRNCPY(newInclude->currentFileName, fname,
                             __INCLUDE_MAX_FILENAME);
                newInclude->includedFromFile = lastInclude->currentFileName;
                newInclude->includedFromLine = cint_c_get_lineno(yyscanner);
            }
            yypush_buffer_state(yy_create_buffer(yyin, YY_BUF_SIZE, yyscanner),
                                yyscanner);
            cint_c_set_lineno(1, yyscanner);
            YY_CURRENT_BUFFER->yy_is_interactive = 0;
        }
        
        if (fpath) {
            CINT_FREE(fpath);
        }
    } else {
        CINT_PRINTF("Exceeded maximum include file depth of %d at include file '%s'\n",
                    CINT_MAX_INCLUDE_DEPTH, fname);
    }

    /* All success and error condition should reset to INITIAL state */
    BEGIN(INITIAL);

#endif /* CINT_CONFIG_INCLUDE_XINCLUDE */

}

"/*"			{ commentNesting = 0; BEGIN(comment); }
<comment>[^*/\n]+	{ /* Swallow stuff inside comments */ }
<comment>"*/"		{ if (commentNesting-- == 0) { BEGIN(INITIAL); } }
<comment>"/*"		{ commentNesting++; }
<comment>\n		{ /* Swallow newlines */ }
<comment>(\/|\*)	{ /* Swallow lone "*" and "/" */ }
<comment><<EOF>>	{ CINT_PRINTF("EOF encountered while processing comment.\n"); yyterminate(); }

<<EOF>> {
    if (yyg->yy_buffer_stack_top) {
        yypop_buffer_state(yyscanner);
    } else {
        yyterminate();
    }
}

"//".*$                 { /* Swallow C++ style comments */ }

"auto"                  { NOPROMPT(); return(AUTO);     }
"break"                 { NOPROMPT(); return(BREAK);    }
"case"                  { NOPROMPT(); return(CASE);     }
"char"                  { NOPROMPT(); return(CHAR);     }
"const"                 { NOPROMPT(); return(CONST);    }
"continue"              { NOPROMPT(); return(CONTINUE); }
"default"               { NOPROMPT(); return(DEFAULT);  }
"do"                    { NOPROMPT(); return(DO);       }
"double"                { NOPROMPT(); return(DOUBLE);   }
"else"                  { NOPROMPT(); return(ELSE);     }
"enum"                  { NOPROMPT(); return(ENUM);     }
"extern"                { NOPROMPT(); return(EXTERN);   }
"float"                 { NOPROMPT(); return(FLOAT);    }
"for"                   { NOPROMPT(); return(FOR);      }
"goto"                  { NOPROMPT(); return(GOTO);     }
"if"                    { NOPROMPT(); return(IF);       }
"int"                   { NOPROMPT(); return(INT);      }
"long"                  { NOPROMPT(); return(LONG);     }
"register"              { NOPROMPT(); return(REGISTER); }
"return"                { NOPROMPT(); return(RETURN);   }
"short"                 { NOPROMPT(); return(SHORT);    }
"signed"                { NOPROMPT(); return(SIGNED);   }
"sizeof"                { NOPROMPT(); return(SIZEOF);   }
"static"                { NOPROMPT(); return(STATIC);   }
"struct"                { NOPROMPT(); return(STRUCT);   }
"switch"                { NOPROMPT(); return(SWITCH);   }
"typedef"               { NOPROMPT(); return(TYPEDEF);  }
"union"                 { NOPROMPT(); return(UNION);    }
"unsigned"              { NOPROMPT(); return(UNSIGNED); }
"void"                  { NOPROMPT(); return(T_VOID);   }
"volatile"              { NOPROMPT(); return(VOLATILE); }
"while"                 { NOPROMPT(); return(WHILE);    }
"print"                 { NOPROMPT(); return(PRINT);    }
"cint"                  { NOPROMPT(); return(CINT);     }

{L}({L}|{D})*		{ NOPROMPT(); return(check_type(yyscanner)); }

0[xX]{H}+{IS}?		{ *yylval = cint_ast_constant(yytext, cintAstConstHex);
                          NOPROMPT(); return(CONSTANT); }
0{D}+{IS}?		{ *yylval = cint_ast_constant(yytext, cintAstConstOctal);
                          NOPROMPT(); return(CONSTANT); }
{D}+{IS}?		{ *yylval = cint_ast_constant(yytext, cintAstConstDecimal);
                          NOPROMPT(); return(CONSTANT); }
L?'(\\.|[^\\'])+'	{ *yylval = cint_ast_constant(yytext, cintAstConstChar);
                          NOPROMPT(); return(CONSTANT); }

{D}+{E}{FS}?		{ *yylval = cint_ast_constant(yytext, cintAstConstFloat);
                          NOPROMPT(); return(CONSTANT); }
{D}*"."{D}+({E})?{FS}?	{ *yylval = cint_ast_constant(yytext, cintAstConstFloat);
                          NOPROMPT(); return(CONSTANT); }
{D}+"."{D}*({E})?{FS}?	{ *yylval = cint_ast_constant(yytext, cintAstConstFloat);
                          NOPROMPT(); return(CONSTANT); }

L?\"(\\.|[^\\"])*\"	{ *yylval = cint_ast_string(yytext); NOPROMPT(); return(STRING_LITERAL); }

"..."                   { NOPROMPT(); return(ELLIPSIS);     }
">>="                   { NOPROMPT(); return(RIGHT_ASSIGN); }
"<<="                   { NOPROMPT(); return(LEFT_ASSIGN);  }
"+="                    { NOPROMPT(); return(ADD_ASSIGN);   }
"-="                    { NOPROMPT(); return(SUB_ASSIGN);   }
"*="                    { NOPROMPT(); return(MUL_ASSIGN);   }
"/="                    { NOPROMPT(); return(DIV_ASSIGN);   }
"%="                    { NOPROMPT(); return(MOD_ASSIGN);   }
"&="                    { NOPROMPT(); return(AND_ASSIGN);   }
"^="                    { NOPROMPT(); return(XOR_ASSIGN);   }
"|="                    { NOPROMPT(); return(OR_ASSIGN);    }
">>"                    { NOPROMPT(); return(RIGHT_OP);     }
"<<"                    { NOPROMPT(); return(LEFT_OP);      }
"++"                    { NOPROMPT(); return(INC_OP);       }
"--"                    { NOPROMPT(); return(DEC_OP);       }
"->"                    { NOPROMPT(); return(PTR_OP);       }
"&&"                    { NOPROMPT(); return(AND_OP);       }
"||"                    { NOPROMPT(); return(OR_OP);        }
"<="                    { NOPROMPT(); return(LE_OP);        }
">="                    { NOPROMPT(); return(GE_OP);        }
"=="                    { NOPROMPT(); return(EQ_OP);        }
"!="                    { NOPROMPT(); return(NE_OP);        }
";"                     { NOPROMPT(); return(';');          }
("{"|"<%")              { NOPROMPT(); return('{');          }
("}"|"%>")              { NOPROMPT(); return('}');          }
","                     { NOPROMPT(); return(',');          }
":"                     { NOPROMPT(); return(':');          }
"="                     { NOPROMPT(); return('=');          }
"("                     { NOPROMPT(); return('(');          }
")"                     { NOPROMPT(); return(')');          }
("["|"<:")              { NOPROMPT(); return('[');          }
("]"|":>")              { NOPROMPT(); return(']');          }
"."                     { NOPROMPT(); return('.');          }
"&"                     { NOPROMPT(); return('&');          }
"!"                     { NOPROMPT(); return('!');          }
"~"                     { NOPROMPT(); return('~');          }
"-"                     { NOPROMPT(); return('-');          }
"+"                     { NOPROMPT(); return('+');          }
"*"                     { NOPROMPT(); return('*');          }
"/"                     { NOPROMPT(); return('/');          }
"%"                     { NOPROMPT(); return('%');          }
"<"                     { NOPROMPT(); return('<');          }
">"                     { NOPROMPT(); return('>');          }
"^"                     { NOPROMPT(); return('^');          }
"|"                     { NOPROMPT(); return('|');          }
"?"                     { NOPROMPT(); return('?');          }

[ \t\v\f]+		{ /* Ignore white space */ }
\n+			{ /* Swallow newlines, scanner will count them */ }
.			{ /* Swallow unknown characters [DEFAULT RULE] */ }

%%

#include "cint_error.h"

int yywrap(yyscan_t yyscanner)
{
    cint_errno = CINT_E_EXIT; 
    return(1);
}

void cpp_comment(void)
{

}

/* cint_current_line()
 *
 * Get current input line from the scanner. Used to construct error messages
 * issued from the parser.
 *
 * Inputs:
 * yyscanner: Pointer to scanner control block.
 * lineBuffer: Character buffer to receive current scan line.
 * lineLen: Size of character buffer.
 * column: Pointer to integer that will receive starting column of current
 *         token in scan line.
 * tokLen: Pointer to integer that will receive length of current token.
 *
 * If current scan line doesn't fit into lineBuffer, trailing characters
 * are truncated. If current token is beyond the truncated buffer, column
 * and tokLen are set to zero.
 */
char *
cint_current_line(yyscan_t yyscanner, char *const lineBuffer, const int lineLen,
                int *column, int *tokLen, char **curFile, int *curLine)
{
    struct yyguts_t *yyg = (struct yyguts_t *) yyscanner;
    struct cint_extra_type_s *extraInfo = cint_c_get_extra(yyscanner);
    const char *buffStart = yyg->yy_buffer_stack[yyg->yy_buffer_stack_top]->yy_ch_buf;
    const char *buffEnd = &buffStart[yyg->yy_n_chars];
    const char *copyEnd = &lineBuffer[lineLen - 1];     /* Leave room for terminator */
    char *copy = lineBuffer;
    char *strPtr;
    int tokenLen;

    *column = 0;
    *tokLen = 0;
    tokenLen = 0;

    if (*(yyg->yytext_r)) {
        /* Back up to start of current line */
        for (strPtr = yyg->yytext_r; (strPtr >= buffStart) && (*strPtr != '\n');
             strPtr--);
        strPtr++;
        /* Copy current line. */
        for (; (strPtr < buffEnd) && (copy < copyEnd) && (*strPtr != '\n'); strPtr++) {
            if (*strPtr) {
                *copy++ = *strPtr;
                if (strPtr < yyg->yytext_r) {
                    /* Count columns before current token. */
                    if (*strPtr == '\t') {
                        /* Deal with tabs */
                        *column = ((*column + 8) / 8) * 8;
                    } else {
                        *column += 1;
                    }
                } else {
                    /* else count token characters */
                    tokenLen++;
                }
            } else {
                /*
                 * Scanner puts a null terminator at end of current token in
                 * current line.  The saved character is held in "yy_hold_char".
                 */
                *tokLen = tokenLen;
                if (yyg->yy_hold_char == '\n') {
                    /* Held character is the EOL character */
                    break;
                }
                *copy++ = yyg->yy_hold_char;
            }
        }
    }
    *copy = 0;  /* Null terminate */

    if (YY_CURRENT_BUFFER->yy_is_interactive) {
        *curLine = 0;
        *curFile = NULL;
    } else {
        *curLine = cint_c_get_lineno(yyscanner);
        *curFile =
          (extraInfo && extraInfo[yyg->yy_buffer_stack_top].currentFileName[0]) ?
          extraInfo[yyg->yy_buffer_stack_top].currentFileName : NULL;
    }
    return lineBuffer;
}

int check_type(yyscan_t scanner)
{
    int rv;
    struct yyguts_t* yyg = (struct yyguts_t*)scanner; 

    char* _yytext = yyget_text(scanner); 

    if(cint_interpreter_is_type(_yytext)) {
        rv = TYPE_NAME;
        *yylval = cint_ast_type(_yytext); 
    }
    else if(cint_interpreter_is_iterator(_yytext, yylval)) {
        rv = ITERATOR; 
    }
    else if(cint_interpreter_is_macro(_yytext, yylval)) {
        rv = MACRO; 

    }
    else {
        rv = IDENTIFIER;
        *yylval = cint_ast_identifier(_yytext); 
    }
    return rv;
}


/*******************************************************************************
 *
 * YY_INPUT Functions
 *
 *
 ******************************************************************************/

/* The default YY_INPUT behavior  */
void 
__yy_input_default(char* buf, int* result, int max_size, yyscan_t yyscanner)
{
    struct yyguts_t* yyg = (struct yyguts_t*) yyscanner; 

    if ( YY_CURRENT_BUFFER->yy_is_interactive ) {
        int c = '*', n; 
        for ( n = 0; n < max_size && 
                  (c = getc( yyin )) != EOF && c != '\n'; ++n ) {
            buf[n] = (char) c; 
        }       
        if ( c == '\n' ) {
            buf[n++] = (char) c; 
        }       
        
        if ( c == EOF && ferror( yyin ) ) {
            YY_FATAL_ERROR( "input in flex scanner failed" ); 
        }       
        *result = n; 
    } 
    else if ( ((*result = fread( buf, 1, max_size, yyin )) == 0) && ferror( yyin ) ) {
        YY_FATAL_ERROR( "input in flex scanner failed" );
    }   
}       

/* The default YY_INPUT behavior with optional character echo */
void 
__yy_input_default_echo(char* buf, int* result, int max_size, yyscan_t yyscanner)
{
    struct yyguts_t* yyg = (struct yyguts_t*) yyscanner; 

    if ( YY_CURRENT_BUFFER->yy_is_interactive ) {
        int c = '*', n; 
        for ( n = 0; n < max_size && 
                  (c = getc( yyin )) != EOF && c != '\n'; 
              ++n ) {
            buf[n] = (char) c; 
            if(cint_scanner_echo==0) CINT_PRINTF("%c", c); 
        }       
        if ( c == '\n' ) {
            buf[n++] = (char) c; 
            if(cint_scanner_echo==0) CINT_PRINTF("%c", c); 
        }       
        
        if ( c == EOF && ferror( yyin ) ) {
            YY_FATAL_ERROR( "input in flex scanner failed" ); 
        }       
        *result = n; 
    } 
    else if ( ((*result = fread( buf, 1, max_size, yyin )) == 0) && ferror( yyin ) ) {
        YY_FATAL_ERROR( "input in flex scanner failed" );
    }   
}       


#if CINT_CONFIG_INCLUDE_PARSER_READLINE == 1

/* YY_INPUT behavior using READLINE */
void 
__yy_input_readline(char* buf, int* result, int max_size, yyscan_t yyscanner)
{
    struct yyguts_t* yyg = (struct yyguts_t*) yyscanner; 

    char* b; 
    extern int cint_prompt(void); 

    if( YY_CURRENT_BUFFER->yy_is_interactive && yyin == stdin) {
        if(cint_scanner_prompt) {
            cint_prompt(); 
        }
        b = CINT_READLINE(""); 
        if(b) { 
            /* Copy readline buffer */
            CINT_STRNCPY(buf, b, max_size-1); 

#if defined(CINT_ADD_HISTORY) && CINT_CONFIG_INCLUDE_PARSER_ADD_HISTORY == 1
            CINT_ADD_HISTORY(buf); 
#endif
            /* Restore newline. This is necessary to complete some scanner rules */
            CINT_STRCAT(buf, "\n"); 
            
            *result = CINT_STRLEN(buf); 

            CINT_FREE(b); 
        } 
        else { 
            /* Ctrl-D/EOF */
            char *e = "exit;";
            int len = CINT_STRLEN(e);
            if (max_size > len) {
                /* buf is malloc'ed and may not be initialized, so
                   string copy operations here may make valgrind complain */
                CINT_MEMCPY(buf, e, len+1); 
                *result = CINT_STRLEN(buf);
            }
        } 
    } 
    else {
        if ( ((*result = fread( buf, 1, max_size, yyin )) == 0) && ferror( yyin ) ) { 
            YY_FATAL_ERROR( "input in flex scanner failed" ); 
        } 
    }
}

#endif



/*
 * This is the YY_INPUT function called by the scanner. 
 */

void
__yy_input(char* buf, int* result, int max_size, yyscan_t scanner)
{
#if CINT_CONFIG_INCLUDE_PARSER_READLINE == 1
    __yy_input_readline(buf, result, max_size, scanner); 
#else
    __yy_input_default(buf, result, max_size, scanner); 
#endif
}


void 
cint_c_scanner_start(FILE* handle, void* scanner)
{
    struct cint_extra_type_s *extraInfo;

    if (handle == 0) {
        /* Defaults to standard in */
        handle = stdin;
    }
    cint_c_restart(handle, scanner);
    extraInfo = cint_c_get_extra(scanner);
    if (extraInfo == 0) {
        extraInfo =
          cint_c_alloc(sizeof(struct cint_extra_type_s) * CINT_MAX_INCLUDE_DEPTH,
                       scanner);
        extraInfo->currentFileName[0] = 0;
        extraInfo->includedFromFile = NULL;
        extraInfo->includedFromLine = -1;
        cint_c_set_extra(extraInfo, scanner);
    }
}

int
cint_c_scanner_finish(void *scanner)
{
    struct cint_extra_type_s *extraInfo = cint_c_get_extra(scanner);

    if (extraInfo) {
        cint_c_free(extraInfo, scanner);
    }
    return cint_c_lex_destroy(scanner);
}

#else /* CINT_CONFIG_INCLUDE_PARSER */
int cint_lexer_c_not_empty; 
#endif