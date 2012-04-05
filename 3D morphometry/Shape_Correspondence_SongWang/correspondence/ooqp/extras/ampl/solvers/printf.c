/****************************************************************
Copyright (C) 1997, 1999, 2001 Lucent Technologies
All Rights Reserved

Permission to use, copy, modify, and distribute this software and
its documentation for any purpose and without fee is hereby
granted, provided that the above copyright notice appear in all
copies and that both that the copyright notice and this
permission notice and warranty disclaimer appear in supporting
documentation, and that the name of Lucent or any of its entities
not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior
permission.

LUCENT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
IN NO EVENT SHALL LUCENT OR ANY OF ITS ENTITIES BE LIABLE FOR ANY
SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
THIS SOFTWARE.
****************************************************************/

/* This implements most of ANSI C's printf, fprintf, and sprintf,
 * omitting L, with %.0g and %.0G giving the shortest decimal string
 * that rounds to the number being converted, and with negative
 * precisions allowed for %f.
 */

#ifdef KR_headers
#include "varargs.h"
#else
#include "stddef.h"
#include "stdarg.h"
#include "stdlib.h"
#endif

#define VA_LIST va_list

#include "stdio1.h"
#include "string.h"
#include "errno.h"

#ifdef KR_headers
#define Const /* const */
#define Voidptr char*
#ifndef size_t__
#define size_t int
#define size_t__
#endif

#else

#define Const const
#define Voidptr void*

#endif

#undef MESS
#ifndef Stderr
#define Stderr stderr
#endif

#ifdef _windows_
#undef PF_BUF
#define MESS
#include "mux0.h"
#define LAST_LEN(x) (f->lastlen = x)
#define stdout_or_err(f) (f == stdout)
#else
#define stdout_or_err(f) (f == Stderr || f == stdout)
#endif

#ifdef __cplusplus
extern "C" {
#endif

 extern char *dtoa ANSI((double, int, int, int*, int*, char **));
 extern void freedtoa ANSI((char*));





#ifdef PF_BUF
#undef LAST_LEN
#define LAST_LEN(x) (f->lastlen = x)
#endif /*PF_BUF*/

#ifdef LAST_LEN
#define LASTLENDCL int lastlen;
#else
#define LAST_LEN(x) (x)
#define LASTLENDCL /*nothing*/
#endif

#ifdef USE_ULDIV
/* This is for avoiding 64-bit divisions on the DEC Alpha, since */
/* they are not portable among variants of OSF1 (DEC's Unix). */

#define ULDIV(a,b) uldiv_ASL(a,(unsigned long)(b))

#ifndef LLBITS
#define LLBITS 6
#endif
#ifndef ULONG
#define ULONG unsigned long
#endif

 static int
klog(ULONG x)
{
	int k, rv = 0;

	if (x > 1L)
	    for(k = 1 << LLBITS-1;;) {
		if (x >= (1L << k)) {
			rv |= k;
			x >>= k;
			}
		if (!(k >>= 1))
			break;
		}
	return rv;
	}

 ULONG
uldiv_ASL(ULONG a, ULONG b)
{
	int ka;
	ULONG c, k;
	static ULONG b0;
	static int kb;

	if (a < b)
		return 0;
	if (b != b0) {
		b0 = b;
		kb = klog(b);
		}
	k = 1;
	if ((ka = klog(a) - kb) > 0) {
		k <<= ka;
		b <<= ka;
		}
	c = 0;
	for(;;) {
		if (a >= b) {
			a -= b;
			c |= k;
			}
		if (!(k >>= 1))
			break;
		a <<= 1;
		}
	return c;
	}

#else
#define ULDIV(a,b) a / b
#endif /* USE_ULDIV */

 typedef struct
Finfo {
	FILE *cf;
	char *ob0, *obe1;
	LASTLENDCL
	} Finfo;

 typedef char *(*Putfunc) ANSI((Finfo*, int*));

#ifdef PF_BUF
FILE *stderr_ASL = (FILE*)&stderr_ASL;
void (*pfbuf_print_ASL) ANSI((char*));
char *pfbuf_ASL;
static char *pfbuf_next;
static size_t pfbuf_len;
extern Char *mymalloc_ASL ANSI((size_t));
extern Char *myralloc_ASL ANSI((void *, size_t));

#undef fflush
#ifdef old_fflush_ASL
#define fflush old_fflush_ASL
#endif

 void
fflush_ASL(FILE *f)
{
	if (f == stderr_ASL) {
		if (pfbuf_ASL && pfbuf_print_ASL) {
			(*pfbuf_print_ASL)(pfbuf_ASL);
			free(pfbuf_ASL);
			pfbuf_ASL = 0;
			}
		}
	else
		fflush(f);
	}

 static void
pf_put(char *buf, int len)
{
	size_t x, y;
	if (!pfbuf_ASL) {
		x = len + 256;
		if (x < 512)
			x = 512;
		pfbuf_ASL = pfbuf_next = (char*)mymalloc_ASL(pfbuf_len = x);
		}
	else if ((y = (pfbuf_next - pfbuf_ASL) + len) >= pfbuf_len) {
		x = pfbuf_len;
		while((x <<= 1) <= y);
		y = pfbuf_next - pfbuf_ASL;
		pfbuf_ASL = (char*)myralloc_ASL(pfbuf_ASL, x);
		pfbuf_next = pfbuf_ASL + y;
		pfbuf_len = x;
		}
	memcpy(pfbuf_next, buf, len);
	pfbuf_next += len;
	*pfbuf_next = 0;
	}

 static char *
pfput(Finfo *f, int *rvp)
{
	int n;
	char *ob0 = f->ob0;
	*rvp += n = (int)(f->obe1 - ob0);
	pf_put(ob0, n);
	return ob0;
	}
#endif /* PF_BUF */

 static char *
Fput
#ifdef KR_headers
	(f, rvp) register Finfo *f; int *rvp;
#else
	(register Finfo *f, int *rvp)
#endif
{
	register char *ob0 = f->ob0;

	*rvp += f->obe1 - ob0;
	*f->obe1 = 0;
	fputs(ob0, f->cf);
	return ob0;
	}


#ifdef _windows_
int stdout_fileno_ASL = 1;

 static char *
Wput
#ifdef KR_headers
	(f, rvp) register Finfo *f; int *rvp;
#else
	(register Finfo *f, int *rvp)
#endif
{
	register char *ob0 = f->ob0;

	*rvp += f->obe1 - ob0;
	*f->obe1 = 0;
	mwrite(ob0, f->obe1 - ob0);
	return ob0;
	}
#endif /*_windows_*/


#define put(x) { *outbuf++ = x; if (outbuf == obe) outbuf = (*fput)(f,&rv); }

 static int
x_sprintf
#ifdef KR_headers
	(obe, fput, f, fmt, ap)
	char *obe, *fmt; Finfo *f; Putfunc fput; va_list ap;
#else
	(char *obe, Putfunc fput, Finfo *f, const char *fmt, va_list ap)
#endif
{
	char *digits, *ob0, *outbuf, *s, *s0, *se;
	Const char *fmt0;
	char buf[32];
	long i;
	unsigned long j, u;
	double x;
	int alt, base, c, decpt, dot, conv, i1, k, lead0, left,
		len, prec, prec1, psign, rv, sgn, sign, width;
	long Ltmp, *ip;
	short sh;
	unsigned short us;
	unsigned int ui;
	static char hex[] = "0123456789abcdef";
	static char Hex[] = "0123456789ABCDEF";

	ob0 = outbuf = f->ob0;
	rv = 0;
	for(;;) {
		for(;;) {
			switch(c = *fmt++) {
				case 0:
					goto done;
				case '%':
					break;
				default:
					put(c)
					continue;
				}
			break;
			}
		alt=dot=lead0=left=len=prec=psign=sign=width=0;
		fmt0 = fmt;
 fmtloop:
		switch(conv = *fmt++) {
			case ' ':
			case '+':
				sign = conv;
				goto fmtloop;
			case '-':
				if (dot)
					psign = 1;
				else
					left = 1;
				goto fmtloop;
			case '#':
				alt = 1;
				goto fmtloop;
			case '0':
				if (!lead0 && !dot) {
					lead0 = 1;
					goto fmtloop;
					}
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				k = conv - '0';
				while((c = *fmt) >= '0' && c <= '9') {
					k = 10*k + c - '0';
					fmt++;
					}
				if (dot)
					prec = psign ? -k : k;
				else
					width = k;
				goto fmtloop;
			case 'h':
				len = 2;
				goto fmtloop;
			case 'l':
				len = 1;
				goto fmtloop;
			case '.':
				dot = 1;
				lead0 = 0;
				goto fmtloop;
			case '*':
				k = va_arg(ap, int);
				if (dot)
					prec = k;
				else {
					if (k < 0) {
						sign = '-';
						k = -k;
						}
					width = k;
					}
				goto fmtloop;
			case 'c':
				c = va_arg(ap, int);
				put(c)
				continue;
			case '%':
				put(conv)
				continue;
			case 'u':
				switch(len) {
				  case 0:
					ui = va_arg(ap, int);
					i = ui;
					break;
				  case 1:
					i = va_arg(ap, long);
					break;
				  case 2:
					us = va_arg(ap, int);
					i = us;
				  }
				sign = 0;
				goto have_i;
			case 'i':
			case 'd':
				switch(len) {
				  case 0:
					k = va_arg(ap, int);
					i = k;
					break;
				  case 1:
					i = va_arg(ap, long);
					break;
				  case 2:
					sh = va_arg(ap, int);
					i = sh;
				  }
				if (i < 0) {
					sign = '-';
					i = -i;
					}
 have_i:
				base = 10;
				u = i;
				digits = hex;
 baseloop:
				s = buf;
				if (!u)
					alt = 0;
				do {
					j = ULDIV(u, base);
					*s++ = digits[u - base*j];
					}
					while(u = j);
				prec -= c = s - buf;
				if (alt && conv == 'o' && prec <= 0)
					prec = 1;
				if ((width -= c) > 0) {
					if (prec > 0)
						width -= prec;
					if (sign)
						width--;
					if (alt == 2)
						width--;
					}
				if (left) {
					if (alt == 2)
						put('0') /* for 0x */
					if (sign)
						put(sign)
					while(--prec >= 0)
						put('0')
					do put(*--s)
						while(s > buf);
					while(--width >= 0)
						put(' ')
					continue;
					}
				if (width > 0) {
					if (lead0) {
						if (alt == 2)
							put('0')
						if (sign)
							put(sign)
						while(--width >= 0)
							put('0')
						goto s_loop;
						}
					else
						while(--width >= 0)
							put(' ')
					}
				if (alt == 2)
					put('0')
				if (sign)
					put(sign)
 s_loop:
				while(--prec >= 0)
					put('0')
				do put(*--s)
					while(s > buf);
				continue;
			case 'n':
				ip = va_arg(ap, long*);
				if (!ip)
					ip = &Ltmp;
				c = outbuf - ob0 + rv;
				switch(len) {
				  case 0:
					*(int*)ip = c;
					break;
				  case 1:
					*ip = c;
					break;
				  case 2:
					*(short*)ip = c;
				  }
				break;
			case 'p':
				len = alt = 1;
				/* no break */
			case 'x':
				digits = hex;
				goto more_x;
			case 'X':
				digits = Hex;
 more_x:
				if (alt) {
					alt = 2;
					sign = conv;
					}
				else
					sign = 0;
				base = 16;
 get_u:
				switch(len) {
				  case 0:
					ui = va_arg(ap, int);
					u = ui;
					break;
				  case 1:
					u = va_arg(ap, long);
					break;
				  case 2:
					us = va_arg(ap, int);
					u = us;
				  }
				if (!u)
					sign = alt = 0;
				goto baseloop;
			case 'o':
				base = 8;
				digits = hex;
				goto get_u;
			case 's':
				s0 = 0;
				s = va_arg(ap, char*);
				if (!s)
					s = "<NULL>";
				if (prec < 0)
					prec = 0;
 have_s:
				if (dot) {
					for(c = 0; c < prec; c++)
						if (!s[c])
							break;
					prec = c;
					}
				else
					prec = strlen(s);
				width -= prec;
				if (!left)
					while(--width >= 0)
						put(' ')
				while(--prec >= 0)
					put(*s++)
				while(--width >= 0)
					put(' ')
				if (s0)
					freedtoa(s0);
				continue;
			case 'f':
				if (!dot)
					prec = 6;
				x = va_arg(ap, double);
				s = s0 = dtoa(x, 3, prec, &decpt, &sgn, &se);
				if (decpt == 9999) {
 fmt9999:
					dot = prec = alt = 0;
					if (*s == 'N')
						goto have_s;
					decpt = strlen(s);
					}
 f_fmt:
				if (sgn && (x||sign))
					sign = '-';
				if ((width -= prec) > 0) {
					if (sign)
						--width;
					if (decpt <= 0)
						width -= 2;
					else {
						width -= decpt;
						if (prec > 0 || alt)
							width--;
						}
					}
				if (width > 0 && !left) {
					if (lead0) {
						if (sign)
							put(sign)
						sign = 0;
						do put('0')
							while(--width > 0);
						}
					else do put(' ')
						while(--width > 0);
					}
				if (sign)
					put(sign)
				if (decpt <= 0) {
					put('0')
					if (prec > 0 || alt)
						put('.')
					while(decpt < 0) {
						put('0')
						prec--;
						decpt++;
						}
					}
				else {
					do {
						if (c = *s)
							s++;
						else
							c = '0';
						put(c)
						}
						while(--decpt > 0);
					if (prec > 0 || alt)
						put('.')
					}
				while(--prec >= 0) {
					if (c = *s)
						s++;
					else
						c = '0';
					put(c)
					}
				while(--width >= 0)
					put(' ')
				freedtoa(s0);
				continue;
			case 'G':
			case 'g':
				if (!dot)
					prec = 6;
				x = va_arg(ap, double);
				if (prec < 0)
					prec = 0;
				s = s0 = dtoa(x, prec ? 2 : 0, prec, &decpt,
					&sgn, &se);
				if (decpt == 9999)
					goto fmt9999;
				c = se - s;
				prec1 = prec;
				if (!prec) {
					prec = c;
					prec1 = c + (s[1] || alt ? 5 : 4);
					/* %.0g gives 10 rather than 1e1 */
					}
				if (decpt > -4 && decpt <= prec1) {
					if (alt)
						prec -= decpt;
					else
						prec = c - decpt;
					if (prec < 0)
						prec = 0;
					goto f_fmt;
					}
				conv -= 2;
				if (!alt && prec > c)
					prec = c;
				--prec;
				goto e_fmt;
			case 'e':
			case 'E':
				if (!dot)
					prec = 6;
				x = va_arg(ap, double);
				if (prec < 0)
					prec = 0;
				s = s0 = dtoa(x, prec ? 2 : 0, prec+1, &decpt,
					&sgn, &se);
				if (decpt == 9999)
					goto fmt9999;
 e_fmt:
				if (sgn && x)
					sign = '-';
				if ((width -= prec + 5) > 0) {
					if (sign)
						--width;
					if (prec || alt)
						--width;
					}
				if ((c = --decpt) < 0)
					c = -c;
				while(c >= 100) {
					--width;
					c /= 10;
					}
				if (width > 0 && !left) {
					if (lead0) {
						if (sign)
							put(sign)
						sign = 0;
						do put('0')
							while(--width > 0);
						}
					else do put(' ')
						while(--width > 0);
					}
				if (sign)
					put(sign)
				put(*s++)
				if (prec || alt)
					put('.')
				while(--prec >= 0) {
					if (c = *s)
						s++;
					else
						c = '0';
					put(c)
					}
				put(conv)
				if (decpt < 0) {
					put('-')
					decpt = -decpt;
					}
				else
					put('+')
				for(c = 2, k = 10; 10*k <= decpt; c++, k *= 10);
				for(;;) {
					i1 = decpt / k;
					put(i1 + '0')
					if (--c <= 0)
						break;
					decpt -= i1*k;
					decpt *= 10;
					}
				while(--width >= 0)
					put(' ')
				freedtoa(s0);
				continue;
			default:
				put('%')
				while(fmt0 < fmt)
					put(*fmt0++)
				continue;
			}
		}
 done:
	*outbuf = 0;
	return LAST_LEN(outbuf - ob0) + rv;
	}

#define Bsize 256

 int
Printf
#ifdef KR_headers
	(va_alist)
 va_dcl
{
	char *fmt;

	va_list ap;
	int rv;
	Finfo f;
	char buf[Bsize];

	va_start(ap);
	fmt = va_arg(ap, char*);
	/*}*/
#else
	(const char *fmt, ...)
{
	va_list ap;
	int rv;
	Finfo f;
	char buf[Bsize];

	va_start(ap, fmt);
#endif
	f.cf = stdout;
	f.ob0 = buf;
	f.obe1 = buf + Bsize - 1;
#ifdef _windows_
	if (fileno(stdout) == stdout_fileno_ASL) {
		rv = x_sprintf(f.obe1, Wput, &f, fmt, ap);
		mwrite(buf, f.lastlen);
		}
	else
#endif
#ifdef PF_BUF
	if (stdout == stderr_ASL) {
		rv = x_sprintf(f.obe1, pfput, &f, fmt, ap);
		pf_put(buf, f.lastlen);
		}
	else
#endif
		{
		rv = x_sprintf(f.obe1, Fput, &f, fmt, ap);
		fputs(buf, stdout);
		}
	va_end(ap);
	return rv;
	}

 static char *
Sput
#ifdef KR_headers
	(f, rvp) Finfo *f; int *rvp;
#else
	(Finfo *f, int *rvp)
#endif
{
	if (Printf("\nBUG! Sput called!\n", f, rvp))
		/* pass vp, rvp and return 0 to shut diagnostics off */
		exit(250);
	return 0;
	}

 int
Sprintf
#ifdef KR_headers
	(va_alist)
 va_dcl
{
	char *s, *fmt;
	va_list ap;
	int rv;
	Finfo f;

	va_start(ap);
	s = va_arg(ap, char*);
	fmt = va_arg(ap, char*);
	/*}*/
#else
	(char *s, const char *fmt, ...)
{
	va_list ap;
	int rv;
	Finfo f;

	va_start(ap, fmt);
#endif
	f.ob0 = s;
	rv = x_sprintf(s, Sput, &f, fmt, ap);
	va_end(ap);
	return rv;
	}

 int
Fprintf
#ifdef KR_headers
	(va_alist)
 va_dcl
{
	FILE *F;
	char *s, *fmt;
	va_list ap;
	int rv;
	Finfo f;
	char buf[Bsize];

	va_start(ap);
	F = va_arg(ap, FILE*);
	fmt = va_arg(ap, char*);
	/*}*/
#else
	(FILE *F, const char *fmt, ...)
{
	va_list ap;
	int rv;
	Finfo f;
	char buf[Bsize];

	va_start(ap, fmt);
#endif
	f.cf = F;
	f.ob0 = buf;
	f.obe1 = buf + Bsize - 1;
#ifdef MESS
	if (stdout_or_err(F)) {
#ifdef _windows_
		if (fileno(stdout) == stdout_fileno_ASL) {
			rv = x_sprintf(f.obe1, Wput, &f, fmt, ap);
			mwrite(buf, f.lastlen);
			}
		else
#endif
#ifdef PF_BUF
		if (F == stderr_ASL) {
			rv = x_sprintf(f.obe1, pfput, &f, fmt, ap);
			pf_put(buf, f.lastlen);
			}
		else
#endif
			{
			rv = x_sprintf(f.obe1, Fput, &f, fmt, ap);
			fputs(buf, F);
			}
		}
	else
#endif /*MESS*/
		{
#ifdef PF_BUF
		if (F == stderr_ASL) {
			rv = x_sprintf(f.obe1, pfput, &f, fmt, ap);
			pf_put(buf, f.lastlen);
			}
	else
#endif
			{
			rv = x_sprintf(f.obe1, Fput, &f, fmt, ap);
			fputs(buf, F);
			}
		}
	va_end(ap);
	return rv;
	}

 int
Vsprintf
#ifdef KR_headers
	(s, fmt, ap) char *s, *fmt; va_list ap;
#else
	(char *s, const char *fmt, va_list ap)
#endif
{
	Finfo f;
	return x_sprintf(f.ob0 = s, Sput, &f, fmt, ap);
	}

 int
Vfprintf
#ifdef KR_headers
	(F, fmt, ap) FILE *F; char *fmt; va_list ap;
#else
	(FILE *F, const char *fmt, va_list ap)
#endif
{
	char buf[Bsize];
	int rv;
	Finfo f;

	f.cf = F;
	f.ob0 = buf;
	f.obe1 = buf + Bsize - 1;
#ifdef MESS
	if (stdout_or_err(F)) {
#ifdef _windows_
		if (fileno(stdout) == stdout_fileno_ASL) {
			rv = x_sprintf(f.obe1, Wput, &f, fmt, ap);
			mwrite(buf, f.lastlen);
			}
		else
#endif
#ifdef PF_BUF
		if (F == stderr_ASL) {
			rv = x_sprintf(f.obe1, pfput, &f, fmt, ap);
			pf_put(buf, f.lastlen);
			}
		else
#endif
			{
			rv = x_sprintf(f.obe1, Fput, &f, fmt, ap);
			fputs(buf, F);
			}
		}
	else
#endif /*MESS*/
		{
#ifdef PF_BUF
		if (F == stderr_ASL) {
			rv = x_sprintf(f.obe1, pfput, &f, fmt, ap);
			pf_put(buf, f.lastlen);
			}
		else
#endif
			{
			rv = x_sprintf(f.obe1, Fput, &f, fmt, ap);
			fputs(buf, F);
			}
		}
	va_end(ap);
	return rv;
	}

 void
Perror
#ifdef KR_headers
	(s) char *s;
#else
	(const char *s)
#endif
{
	if (s && *s)
		Fprintf(Stderr, "%s: ", s);
	Fprintf(Stderr, "%s\n", strerror(errno));
	}


#ifdef __cplusplus
}
#endif
