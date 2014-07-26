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

#ifndef XPOST_F_H
#define XPOST_F_H

/*
   a filetype object uses .mark_.padw to store the ent
   for the FILE *
   */

Xpost_Object xpost_file_cons(Xpost_Memory_File *mem, /*@NULL@*/ FILE *fp);
int xpost_file_open(Xpost_Memory_File *mem, char *fn, char *mode, Xpost_Object *retval);
FILE *xpost_file_get_file_pointer(Xpost_Memory_File *mem, Xpost_Object f);
int xpost_file_get_status(Xpost_Memory_File *mem, Xpost_Object f);
int xpost_file_get_bytes_available(Xpost_Memory_File *mem, Xpost_Object f, int *retval);
int xpost_file_close(Xpost_Memory_File *mem, Xpost_Object f);
Xpost_Object xpost_file_read_byte(Xpost_Memory_File *mem, Xpost_Object f);
int xpost_file_write_byte(Xpost_Memory_File *mem, Xpost_Object f, Xpost_Object b);

#endif