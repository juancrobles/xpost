/* define MMAP and MREMAP for Linux */
/* define MMAP without MREMAP should work for Irix (using AUTOGROW) */
/* no MREMAP under cygwin (we'll see how AUTOGROW handles...)*/
/* define neither to use malloc/realloc/free */
//#define MMAP
//#define MREMAP

#ifdef MMAP
#include<sys/mman.h>
#  ifdef MREMAP
#    define _GNU_SOURCE /* mremap */
#  endif
#endif

void error(char *msg);

extern unsigned pgsz /*= getpagesize()*/; /*=4096 (usually on 32bit)*/

typedef struct {
	int fd;
	/*@dependent@*/ unsigned char *base;
	unsigned used;
	unsigned max;
	unsigned roots[2]; /* low, high : entries */
	unsigned start; /* first "live" entry == roots[1]+1 */
		/* the domain of the collector is entries >= roots[1]+1 */
} mfile;

void dumpmfile(mfile *mem);
void initmem(mfile *mem, char *fname); /* initialize the memory file */
void exitmem(mfile *mem); /* destroy the memory file */
void growmem(mfile *mem, unsigned sz);

/* allocate memory, returns offset in memory file */
unsigned mfalloc(mfile *mem, unsigned sz);


#define TABSZ 6
typedef struct {
	unsigned nexttab; /* next table in chain */
	unsigned nextent; /* next slot in table, or TABSZ if none */
	struct {
		unsigned adr;
		unsigned sz;
		unsigned mark;
	} tab[TABSZ];
} mtab;


/* fields in mtab.tab[].mark */
enum {
	MARKM = 0xFF000000,
	MARKO = 24,
	RFCTM = 0x00FF0000,
	RFCTO = 16,
	LLEVM = 0x0000FF00,
	LLEVO = 8,
	TLEVM = 0x000000FF,
	TLEVO = 0,
};

/* special entries */
enum {
	FREE,   
	VS,
	CTXLIST,
	NAMES, /* these 4 global only */
	NAMET,
	BOGUSNAME,
	OPTAB, 
};

/* dump mtab details to stdout */
void dumpmtab(mfile *mem, unsigned mtabadr);

/* allocate and initialize a new table */
unsigned initmtab(mfile *mem);

/* allocate memory, returns table index */
unsigned mtalloc(mfile *mem, unsigned mtabadr, unsigned sz);

/* find the table and relative entity index for an absolute entity index */
void findtabent(mfile *mem, /*@out@*/ mtab **atab, /*@out@*/ unsigned *aent);

/* get the address from an entity */
unsigned adrent(mfile *mem, unsigned ent);

/* get the size of an entity */
unsigned szent(mfile *mem, unsigned ent);

/* fetch a value from a composite object */
void get(mfile *mem,
		unsigned ent, unsigned offset, unsigned sz,
		void *dest);

/* put a value into a composite object */
void put(mfile *mem,
		unsigned ent, unsigned offset, unsigned sz,
		void *src);

