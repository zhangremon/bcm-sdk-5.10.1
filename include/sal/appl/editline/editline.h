/*
$Id: editline.h 1.2.222.1 Broadcom SDK $
$Copyright: $
*/

#if !defined(_SAL_EDITLINE_H)
#define _SAL_EDITLINE_H

#ifdef INCLUDE_EDITLINE

#if	!defined(CONST)
#if	defined(__STDC__)
#define CONST	const
#else
#define CONST
#endif	/* defined(__STDC__) */
#endif	/* !defined(CONST) */

extern char	*readline(CONST char *prompt);
extern void	add_history(char *p);

/* editline asynchronous callback interface */

/** Called when an end of line is reached. */
typedef void (*rl_vcpfunc_t)(char *line, void *ctx);
/** Called when an end of file is reached. */
typedef void (*rf_vcpfunc_t)(void *ctx);
extern void rl_callback_read_char(CONST char *prompt);

/** Called when asynchronous edit line support is no longer needed. */
extern void rl_callback_handler_remove(void **eolCtx, void **eofCtx);
                                                                               
/** Must be called initially to enable asynchronous edit line support. */
extern void rl_callback_handler_install(CONST char *prompt, 
                                       rl_vcpfunc_t eol_handler, void *eolCtx,
                                       rf_vcpfunc_t eof_handler, void *eofCtx);
#endif /* INCLUDE_EDITLINE */

#endif /* _SAL_EDITLINE_H */
