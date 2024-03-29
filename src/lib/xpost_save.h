/*
 * Xpost - a Level-2 Postscript interpreter
 * Copyright (C) 2013, Michael Joshua Ryan
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

#ifndef XPOST_SAVE_H
#define XPOST_SAVE_H

/**
 *  @file xpost_save
 *
 *  Each mfile has a special entity (XPOST_MEMORY_TABLE_SPECIAL_SAVE_STACK)
 *  which holds the address of the "save stack". This stack holds save objects.
 *  Save objects are ordinary postscript objects and are available to user programs.
 *
 *  The save object contains an address of a(nother) stack,
 *  this one containing saverec_ structures.
 *  A saverec_ object contains 2 entity numbers, one the source,
 *  the other the copy, of the "saved" array or dictionary.
 *
 *  stashed and stash are the interfaces used by composite objects
 *  to check-if-copying-is-necessary
 *  and copy-the-value-and-add-saverec-to-current-savelevel-stack

 *  Illustration:
 *
 *  mem[mtab[XPOST_MEMORY_TABLE_SPECIAL_SAVE_STACK].adr] = Master Save stack
 *  -- save object = { lev=0, stk=... }
 *  -- save object = { lev=1, stk=... }
 *     -- saverec
 *     -- saverec
 *  -- save object = { lev=2, stk=... }  <-- top of XPOST_MEMORY_TABLE_SPECIAL_SAVE_STACK, current savelevel stack
 *     mem[save.save_.stk] = Save Object's stack
 *     -- saverec
 *     -- saverec
 *     -- saverec = { src=foo_ent, cpy=bar_ent }
 *
 */

/*
 * @brief initialize the save stack for memory file.
 */
int xpost_save_init(Xpost_Memory_File *mem);

/*
 * @brief create a savetype object that represents a snapshot of virtual memory (array and dict) contents,
         and push a new snapshot object on the stack.
 */
Xpost_Object xpost_save_create_snapshot_object(Xpost_Memory_File *mem);

/*
 * @brief check whether an ent is contained in the current snapshot
 */
unsigned xpost_save_ent_is_saved(Xpost_Memory_File *mem, unsigned ent);

/*
 * @brief add ent to current snapshot
 */
int xpost_save_save_ent(Xpost_Memory_File *mem, unsigned tag, unsigned pad, unsigned ent);

/*
 * @brief rewind the stack 1 level, reverting memory to previous snapshot.
 */
void xpost_save_restore_snapshot(Xpost_Memory_File *mem);

#endif
