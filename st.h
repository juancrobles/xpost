
object consstr(mfile *mem, unsigned sz, /*@NULL@*/ char *ini);
object consbst(context *ctx, unsigned sz, /*@NULL@*/ char *ini);
/*@dependent@*/ char *charstr(context *ctx, object S);
void strput(mfile *mem, object s, integer i, integer c);
void bstput(context *ctx, object s, integer i, integer c);
integer strget(mfile *mem, object s, integer i);
integer bstget(context *ctx, object s, integer i);


