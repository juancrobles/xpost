#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# ifndef HAVE__BOOL
#  ifdef __cplusplus
typedef bool _Bool;
#  else
#   define _Bool signed char
#  endif
# endif
# define bool _Bool
# define false 0
# define true 1
# define __bool_true_false_are_defined 1
#endif

#include <assert.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xpost_memory.h"  // mfile
#include "xpost_object.h"  // object
#include "xpost_stack.h"  // stack
#include "itp.h"  // access context struct
#include "xpost_dict.h"  // access dict objects
#include "xpost_string.h"  // access string objects
#include "xpost_error.h"  // double-check prototypes
#include "xpost_name.h"  // create names

//#define EMITONERROR

char *errorname[] = { ERRORS(AS_STR) };

volatile char *errormsg = "";

static int in_onerror;

/* placeholder error function */
/* ultimately, this will do a longjmp back
   to the central loop */
void error(unsigned err,
        char *msg)
{
    context *ctx;


    errormsg = msg;
    if (!initializing && jbmainloopset && !in_onerror) {
        longjmp(jbmainloop, err);
    }

    /* following will become "fallback" code
       if jmpbuf is not set */
    fprintf(stderr, "\nError: %s", errorname[err]);
    fprintf(stderr, "\nObject: ");
    dumpobject(itpdata->ctab[0].currentobject);
    fprintf(stderr, "\nExtra: %s", msg);
    perror("\nlast system error:");

    printf("\nError: %s", errorname[err]);
    printf("\nExtra: %s", msg);

    ctx = &itpdata->ctab[0];
    printf("\nopstack: ");
    dumpstack(ctx->lo, ctx->os);
    printf("\nexecstack: ");
    dumpstack(ctx->lo, ctx->es);
    printf("\ndictstack: ");
    dumpstack(ctx->lo, ctx->ds);

    printf("\nLocal VM: ");
    dumpmfile(ctx->lo);
    dumpmtab(ctx->lo, 0);
    printf("\nGlobal VM: ");
    dumpmfile(ctx->gl);
    dumpmtab(ctx->gl, 0);

    printf("\nGlobal Name Stack: ");
    dumpstack(ctx->gl, adrent(ctx->gl, NAMES));
    printf("\nLocal Name Stack: ");
    dumpstack(ctx->lo, adrent(ctx->lo, NAMES));

    exit(EXIT_FAILURE);
}


/* called by itp:loop() after longjmp from error()
   pushes postscript-level error procedures
   and resumes normal execution.
 */
void onerror(context *ctx,
        unsigned err)
{
    object sd;
    object dollarerror;
    char *errmsg; 
    stack *sp;

    assert(ctx);
    assert(ctx->gl);
    assert(ctx->gl->base);
    assert(ctx->lo);
    assert(ctx->lo->base);

    in_onerror = true;

#ifdef EMITONERROR
    fprintf(stderr, "err: %s\n", errorname[err]);
#endif
    //reset stack
    if (type(ctx->currentobject) == operatortype
            && ctx->currentobject.tag & FOPARGSINHOLD) {
        int n = ctx->currentobject.mark_.pad0;
        int i;
        for (i=0; i < n; i++) {
            push(ctx->lo, ctx->os, bot(ctx->lo, ctx->hold, i));
        }
    }

    //printf("1\n");
    sd = bot(ctx->lo, ctx->ds, 0);
    //printf("2\n");

    dollarerror = bdcget(ctx, sd, consname(ctx, "$error"));
    //printf("3\n");
    errmsg = errormsg;
    //printf("4\n");
    bdcput(ctx, dollarerror,
            consname(ctx, "Extra"),
            consbst(ctx, strlen(errmsg), errmsg));
    //printf("5\n");

    push(ctx->lo, ctx->os, ctx->currentobject);
    //printf("6\n");
    push(ctx->lo, ctx->os, cvlit(consname(ctx, errorname[err])));
    //printf("7\n");
    push(ctx->lo, ctx->es, consname(ctx, "signalerror"));
    //printf("8\n");

    in_onerror = false;
}
