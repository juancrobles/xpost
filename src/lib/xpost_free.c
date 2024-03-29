/*
 * Xpost - a Level-2 Postscript interpreter
 * Copyright (C) 2013, Michael Joshua Ryan
 * Copyright (C) 2013, Thorsten Behrens
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of the Xpost software product nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "xpost.h"
#include "xpost_log.h"
#include "xpost_memory.h" /* Xpost_Memory_File */
#include "xpost_object.h" /* Xpost_Object */
#include "xpost_free.h"

/*
   initialize the free-list in the memory file.
   free list head is in slot zero
   sz is 0 so gc will ignore it */
int xpost_free_init(Xpost_Memory_File *mem)
{
    unsigned int ent;
    unsigned int val = 0;
    int ret;

    /* allocate the free list head: 4 bytes in ent 0
       allocate additional 1k "scratch" space to protect
       interpreter data from NULL writes
     */
    ret = xpost_memory_table_alloc(mem, 1024, 0, &ent);
    if (!ret)
    {
        return 0;
    }

    /* make sure this is the correct ent */
    assert (ent == XPOST_MEMORY_TABLE_SPECIAL_FREE);

    /* set to zero (== NULL == link-back-to-head) */
    ret = xpost_memory_put(mem, ent, 0, sizeof(unsigned int), &val);
    if (!ret)
    {
        XPOST_LOG_ERR("xpost_free_init cannot access list head");
        return 0;
    }

    /* set zero size to enable guards against NULL writes */
    {
        Xpost_Memory_Table *tab = (Xpost_Memory_Table *)mem->base;
        tab->tab[XPOST_MEMORY_TABLE_SPECIAL_FREE].sz = 0;
    }

    /* make free list available for general memory allocations */
    (void) xpost_memory_register_free_list_alloc_function(mem, xpost_free_alloc);

    return 1;
}

/* free this ent! returns reclaimed size or -1 on error */
int xpost_free_memory_ent(Xpost_Memory_File *mem,
                          unsigned int ent)
{
    Xpost_Memory_Table *tab;
    unsigned int rent = ent; /* relative ent index */
    unsigned int z; /* free list pointer */
    unsigned int a; /* adr associated with ent */
    unsigned int sz; /* sz associated with adr */
    int ret;
    /* return; */

    if (ent < mem->start)
        return 0;

    ret = xpost_memory_table_find_relative(mem, &tab, &rent);
    if (!ret)
    {
        XPOST_LOG_ERR("cannot free ent %u", ent);
        return -1;
    }
    a = tab->tab[rent].adr;
    sz = tab->tab[rent].sz;
    if (sz == 0) return 0; /* do not add zero-size allocations to list */

    if (tab->tab[rent].tag == filetype)
    {
        FILE *fp;
        ret = xpost_memory_get(mem, ent, 0, sizeof(FILE *), &fp);
        if (!ret)
        {
            XPOST_LOG_ERR("cannot load FILE* from VM");
            return -1;
        }
        if (fp &&
            fp != stdin &&
            fp != stdout &&
            fp != stderr)
        {
            tab->tab[rent].tag = 0;
#ifdef DEBUG_FILE
            printf("gc:xpost_free_memory_ent closing FILE* %p\n", fp);
            fflush(stdout);
            /* if (fp < 0x1000) return 0; */
            printf("fclose");
#endif
            fclose(fp);
            fp = NULL;
            ret = xpost_memory_put(mem, ent, 0, sizeof(FILE *), &fp);
            if (!ret)
            {
                XPOST_LOG_ERR("cannot write NULL over FILE* in VM");
                return -1;
            }
        }
    }
    tab->tab[rent].tag = 0;

    ret = xpost_memory_table_get_addr(mem, XPOST_MEMORY_TABLE_SPECIAL_FREE, &z);
    if (!ret)
    {
        XPOST_LOG_ERR("unable to load free list head");
        return -1;
    }
    /* printf("freeing %d bytes\n", xpost_memory_table_get_size(mem, ent)); */

    //while current node < size of ent being added
        //load next ent in list
    while (1)
    {
        unsigned int t;
        unsigned int ta;
        unsigned int tsz;

        /* get the next ent from z node */
        memcpy(&t, mem->base + z, sizeof(unsigned int));

        if (t == 0) /* end of the list */
            break;

        ret = xpost_memory_table_get_size(mem, t, &tsz);
        if (!ret)
        {
            XPOST_LOG_ERR("cannot get size from ent on free list");
            return -1;
        }

        if (tsz > sz) /* this is the place */
            break;

        ret = xpost_memory_table_get_addr(mem, t, &ta);
        if (!ret)
        {
            XPOST_LOG_ERR("cannot get addr from ent on free list");
            return -1;
        }

        z = ta;
    }

    /* copy the current free-list node to the data area of the ent. */
    memcpy(mem->base + a, mem->base + z, sizeof(unsigned int));

    /* copy the ent number into the free-list node */
    memcpy(mem->base + z, &ent, sizeof(unsigned int));

    return sz;
}

/* print a dump of the free list */
void xpost_free_dump(Xpost_Memory_File *mem)
{
    unsigned int e;
    unsigned int z;
    int ret;

    ret = xpost_memory_table_get_addr(mem, XPOST_MEMORY_TABLE_SPECIAL_FREE, &z);
    if (!ret)
    {
        return;
    }

    printf("freelist: ");
    memcpy(&e, mem->base + z, sizeof(unsigned int));
    while (e)
    {
        unsigned int sz;
        ret = xpost_memory_table_get_size(mem, e, &sz);
        if (!ret)
        {
            return;
        }
        printf("%u(%u) ", e, sz);
        ret = xpost_memory_table_get_addr(mem, e, &z);
        if (!ret)
        {
            return;
        }
        memcpy(&e, mem->base + z, sizeof(unsigned int));
    }
}

/* scan the free list for a suitably-sized bit of memory,

   if the allocator falls back to fresh memory XPOST_GARBAGE_COLLECTION_PERIOD times,
        it triggers a collection.
    Returns 1 on success, 0 on failure, 2 to request garbage collection and re-call.
 */
int xpost_free_alloc(Xpost_Memory_File *mem,
                     unsigned int sz,
                     unsigned int tag,
                     unsigned int *entity)
{
    unsigned int z;
    unsigned int e;                     /* working pointer */
    static int period = XPOST_GARBAGE_COLLECTION_PERIOD;
    int ret;

    ret = xpost_memory_table_get_addr(mem, XPOST_MEMORY_TABLE_SPECIAL_FREE, &z); /* free pointer */
    if (!ret)
    {
        XPOST_LOG_ERR("unable to load free list head");
        return 0;
    }

    memcpy(&e, mem->base+z, sizeof(unsigned int)); /* e = *z */
    while (e) /* e is not zero */
    {
        unsigned int tsz;
        if (e > XPOST_OBJECT_COMP_MAX_ENT)
        {
            unsigned int zero = 0;
            XPOST_LOG_ERR("ent number %u exceeds object storage max %u",
                    e, XPOST_OBJECT_COMP_MAX_ENT);
            /* bad element found: discard free list */
            xpost_memory_put(mem, 0, 0, sizeof zero, &zero);
            return 2; /* request collection to fill the list */
        }
        ret = xpost_memory_table_get_size(mem, e, &tsz);
        if (!ret)
        {
            XPOST_LOG_ERR("cannot retrieve size of ent %u", e);
            return 0;
        }

        /* if this ent is sufficient to hold sz,
           but does not waste more than sz bytes, use it */
        if (tsz >= sz)
        {
            Xpost_Memory_Table *tab;
            unsigned int ent;
            unsigned int ad;

            /* if this ent is too big */
            if (tsz * XPOST_FREE_ACCEPT_DENOM > sz * XPOST_FREE_ACCEPT_OVERSIZE)
            {
                return 0; /* early exit to _new allocator since free list is sorted */
            }

            ret = xpost_memory_table_get_addr(mem, e, &ad);
            if (!ret)
            {
                XPOST_LOG_ERR("cannot retrieve address of ent %u", e);
                return 0;
            }
            memcpy(mem->base + z, mem->base + ad, sizeof(unsigned int));
            ent = e;
            ret = xpost_memory_table_find_relative(mem, &tab, &ent);
            if (!ret)
            {
                XPOST_LOG_ERR("cannot find table for ent %u", e);
                return 0;
            }
            tab->tab[ent].tag = tag;
            *entity = e;
            return 1; /* found, return SUCCESS */
        }
        ret = xpost_memory_table_get_addr(mem, e, &z);
        if (!ret)
        {
            XPOST_LOG_ERR("cannot retrieve address for ent %u", e);
            return 0;
        }
        memcpy(&e, mem->base + z, sizeof(unsigned int));
    }
    /* finished scanning free list */

    if (--period == 0) /* check garbage-collection control */
    {
        period = XPOST_GARBAGE_COLLECTION_PERIOD;
        return 2; /* not found, request garbage-collection and try-again */
        /* collect(mem, 1, 0); */
        /* goto try_again; */
    }

    return 0; /* not found, fall-back to _new allocator */
}

/*
   use the free-list and tables to now provide a realloc for
   "raw" vm addresses (mem->base offsets rather than ents).

   Allocate new entry, copy data, steal its adr, stash old adr, free it.

   Currently this is only used to re-size signature blocks in the operator table.
 */
unsigned int xpost_free_realloc(Xpost_Memory_File *mem,
                                unsigned int oldadr,
                                unsigned int oldsize,
                                unsigned int newsize)
{
    Xpost_Memory_Table *tab = NULL;
    unsigned int newadr;
    unsigned int ent;
    unsigned int rent; /* relative ent */
    int ret;

#ifdef DEBUGFREE
    printf("xpost_free_realloc: ");
    printf("initial ");
    xpost_free_dump(mem);
#endif

    /* allocate new entry */
    ret = xpost_memory_table_alloc(mem, newsize, 0, &ent);
    if (!ret)
    {
        XPOST_LOG_ERR("cannot allocate new memory");
        return 0;
    }
    rent = ent;
    xpost_memory_table_find_relative(mem, &tab, &rent);
    if (!ret)
    {
        XPOST_LOG_ERR("cannot find table for ent %u", ent);
        return 0;
    }

    /* steal its adr */
    newadr = tab->tab[rent].adr;

    /* copy data */
    memcpy(mem->base + newadr, mem->base + oldadr, oldsize);

    /* stash old adr */
    tab->tab[rent].adr = oldadr;
    tab->tab[rent].sz = oldsize;

    /* free it */
    (void) xpost_free_memory_ent(mem, ent);

#ifdef DEBUGFREE
    printf("final ");
    xpost_free_dump(mem);
    printf("\n");
    dumpmtab(mem, 0);
    fflush(NULL);
#endif

    return newadr;
}
