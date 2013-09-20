/* the interpreter
       eval actions
       core interpreter loop
       bank utility function for extracting from the context the mfile relevant to an object
   */

typedef struct {
    unsigned id;
    /*@dependent@*/ mfile *gl, *lo;
    unsigned os, es, ds, hold;  
    unsigned long rand_next;
    unsigned vmmode; 
    unsigned state;
    unsigned quit;
    object currentobject;
} context;

enum { LOCAL, GLOBAL }; //vmmode
#define MAXCONTEXT 10
#define MAXMFILE 10

typedef struct {
    context ctab[MAXCONTEXT];
    unsigned cid;
    mfile gtab[MAXMFILE];
    mfile ltab[MAXMFILE];
} itp;


#include <setjmp.h>
extern itp *itpdata;
extern int initializing;
extern jmp_buf jbmainloop;
extern bool jbmainloopset;

void initctxlist(mfile *mem);
context *ctxcid(unsigned cid);
void initcontext(context *ctx);
void exitcontext(context *ctx);
/*@dependent@*/
mfile *bank(context *ctx, object o);

extern int TRACE;

void inititp(itp *itp);
void exititp(itp *itp);
