/* stack operators */

void Apop (context *ctx, object q);
void AAexch (context *ctx, object x, object y);
void Adup (context *ctx, object x);
void IIroll (context *ctx, object N, object J);
void Zcounttomark (context *ctx);

void initops(context *ctx, object sd);


