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

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#ifdef HAVE_LIBGEN_H
# include <libgen.h>
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xpost_main.h"
#include "xpost_pathname.h"

static
int checkexepath (const char *exepath, char **pexedir)
{
    char *slash;
    char *exedir;
    int is_installed = 0;

    /* replace windows's \\  so we can compare paths */
    slash = (char *)exepath;
    while (*slash++)
    {
        if (*slash == '\\') *slash = '/';
    }

    /* global exedir is set in ps systemdict as /EXE_DIR */
    exedir = strdup(exepath);
    dirname(exedir);

#ifdef DEBUG_PATHNAME
    printf("exepath: %s\n", exepath);
    printf("dirname: %s\n", exedir);
    printf("PACKAGE_INSTALL_DIR: %s\n", PACKAGE_INSTALL_DIR);
#endif

    /* if path-to-executable is where it should be installed */
    is_installed = strstr(exepath, PACKAGE_INSTALL_DIR) != NULL;

#ifdef DEBUG_PATHNAME
    printf("is_installed: %d\n", is_installed);
#endif

    *pexedir = exedir;
    return is_installed;
}

static
char *appendtocwd (const char *relpath)
{
    char buf[1024];
    if (getcwd(buf, sizeof buf) == NULL)
    {
        perror("getcwd() error");
        return NULL;
    }
    strcat(buf, "/");
    strcat(buf, relpath);
    return strdup(buf);
}

static
int searchpathforargv0(const char *argv0, char **pexedir)
{
    (void)argv0;
    /*
       not implemented.
       This function may be necessary on some obscure unices.
       It is called if there is no other path information,
       ie. argv[0] is a bare name,
       and no /proc/???/exe links are present
    */
    return checkexepath(".", pexedir);
}

static
int checkargv0 (const char *argv0, char **pexedir)
{
#ifdef HAVE_WIN32
    if (argv0[1] == ':' &&
        (argv0[2] == '/' || argv0[2] == '\\'))
#else
    if (argv0[0] == '/') /* absolute path */
#endif
    {
        return checkexepath(argv0, pexedir);
    }
    else if (strchr(argv0, '/') != 0) /* relative path */
    {
        char *tmp;
        int ret;
        tmp = appendtocwd(argv0);
        ret = checkexepath(tmp, pexedir);
        free(tmp);
        return ret;
    }
    else /* no path info: search $PATH */
        return searchpathforargv0(argv0, pexedir);
}

int xpost_is_installed (const char *argv0, char **pexedir)
{
    char buf[1024];
    ssize_t len;
    char *libsptr;

    (void)len; // len and buf are used in some, but not all, compilation paths
    (void)buf;
    printf("argv0: %s\n", argv0);

    /* hack for cygwin and mingw.
       there's this unfortunate ".libs" in there.
    */
    if ((libsptr = strstr(argv0, ".libs/"))) {
        printf("removing '.libs' from pathname\n");
        memmove(libsptr, libsptr+6, strlen(libsptr+6)+1);
        printf("argv0: %s\n", argv0);
        return checkargv0(argv0, pexedir);
    }

#ifdef HAVE_WIN32
    return checkargv0(argv0, pexedir);

    /*
      len = GetModuleFileName(NULL, buf, 1024);
      buf[len] = '\0';
      if (len == 0)
      return -1;
      else
      return checkexepath(buf, pexedir);
    */
#else

    if ((len = readlink("/proc/self/exe", buf, sizeof buf)) != -1)
    {
        buf[len] = '\0';
    }

    if (len == -1)
        if ((len = readlink("/proc/curproc/exe", buf, sizeof buf)) != -1)
            buf[len] = '\0';

    if (len == -1)
        if ((len = readlink("/proc/self/path/a.out", buf, sizeof buf)) != -1)
            buf[len] = '\0';

    if (len == -1)
        return checkargv0(argv0, pexedir);
    else
        return checkexepath(buf, pexedir);
#endif
}

