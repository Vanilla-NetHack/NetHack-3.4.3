/* Copyright (C) 1982, 1984, 1986 by Manx Software Systems */
#ifndef _STDIO_H
#define _STDIO_H

#define fgetc getc
#define fputc putc
#ifndef NULL
#define NULL 0L
#endif
#define EOF -1


#define BUFSIZ 1024
#define MAXSTREAM       20

#define _BUSY   0x01
#define _ALLBUF 0x02
#define _DIRTY  0x04
#define _EOF    0x08
#define _IOERR  0x10
#define _TEMP   0x20    /* temporary file (delete on close) */

typedef struct {
        char *_bp;              /* current position in buffer */
        char *_bend;            /* last character in buffer + 1 */
        char *_buff;            /* address of buffer */
        char _flags;            /* open mode, etc. */
        char _unit;             /* token returned by open */
        char _bytbuf;           /* single byte buffer for unbuffer streams */
        short _buflen;          /* length of buffer */
        char *_tmpname;         /* name of file for temporaries */
} FILE;

extern FILE Cbuffs[];
FILE *fopen();
long ftell();

#define stdin       (&Cbuffs[0])
#define stdout      (&Cbuffs[1])
#define stderr      (&Cbuffs[2])
#define feof(fp) (((fp)->_flags&_EOF)!=0)
#define ferror(fp) (((fp)->_flags&_IOERR)!=0)
#define clearerr(fp) ((fp)->_flags &= ~(_IOERR|_EOF))
#define fileno(fp) ((fp)->_unit)
#define rewind(fp) fseek(fp, 0L, 0)

#define P_tmpdir        ""
#define L_tmpnam        40

/*
 *  The following must be done for NetHack:
 */

#define FFLUSH(fp) flsh_(fp,-1)     /* Was fflush */

FILE *freopen();
char *gets();

#define getchar()   WindowGetchar()
#define putchar(c)  WindowPutchar(c)
#define puts(s)     WindowPuts(s)
#define fputs(s,f)  WindowFPuts(s)
#define printf      WindowPrintf
#define fflush(fp)  WindowFlush()

#define xputs(s)    WindowFPuts(s)
#define xputc(c)    WindowPutchar(c)

#endif
