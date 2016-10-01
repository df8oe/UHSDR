/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/**************************************************************************//*****
 * @file     stdio.c
 * @brief    Implementation of newlib syscall
 ********************************************************************************/

// Optimization enable for this file
#pragma GCC optimize "O3"


#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>

#undef errno
extern int errno;
extern int  _end;
static unsigned char *heap = NULL;

caddr_t _sbrk ( int incr )
{
    unsigned char *prev_heap;

    if (heap == NULL)
    {
        heap = (unsigned char *)&_end;
    }
    prev_heap = heap;

    heap += incr;

    return (caddr_t) prev_heap;
}

int link(char *old, char *new)
{
    return -1;
}

int _close(int file)
{
    return -1;
}

int _fstat(int file, struct stat *st)
{
    st->st_mode = S_IFCHR;
    return 0;
}

int _isatty(int file)
{
    return 1;
}

int _lseek(int file, int ptr, int dir)
{
    return 0;
}

int _read(int file, char *ptr, int len)
{
    return 0;
}

int _write(int file, char *ptr, int len)
{
    return len;
}

void abort(void)
{
    /* Abort called */
    while(1);
}

/* --------------------------------- End Of File ------------------------------ */
