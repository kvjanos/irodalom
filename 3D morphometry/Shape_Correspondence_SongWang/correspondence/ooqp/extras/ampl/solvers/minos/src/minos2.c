/****************************************************************
Copyright (C) 1997 Lucent Technologies
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

#include "stdio.h"
#include "string.h"
#ifndef Long
#define Long long
#endif
#ifndef fint
#define fint Long
#endif

typedef fint ftnlen;

#define N_FNAMES 100
char *fnames[N_FNAMES];
extern char **xargv;
int option_echo = 1;

#define Malloc(x) mymalloc_ASL((fint)(x))

 extern struct {
	fint iread, iprint, isumm;
	} m1file_;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef KR_headers
 extern char *getenv(), *mymalloc_ASL();
 extern void m3dflt_(), m3key_(), minos3_();
 extern int Sprintf();
#define VOID /*void*/
#else
#include "stdlib.h"
 extern void m3dflt_(fint*);
 extern void m3key_(char *buffer, char *key, fint *lprnt, fint *lsumm,
	fint *inform, ftnlen buffer_len, ftnlen key_len);
 extern void minos3_(double*, fint*, fint*);
 extern void *mymalloc_ASL(fint);
 extern int Sprintf(char*, const char*, ...);
#define VOID void
#endif

 extern struct {
	fint maxw, maxz;
	} m2mapz_;

 extern struct {
	fint kchk, kinv, ksav, klog, ksumm, i1freq, i2freq, msoln;
	} m5freq_;

 extern struct {
    double tlast[5], tsum[5];
    fint numt[5], ltime;
	} m1tim_;

 int n_badopts;

 static char *
#ifdef KR_headers
get_option(s, mode, cml) char *s; int mode; int cml;
#else
get_option(char *s, int mode, int cml)
#endif
{
	int c, c1, haveq;
	char *bufend, *s0, *s1, *s2, *s3;
	char buf[256], key[16];
	static fint inform, lprint, lsumm = 6;

	for(; (c = *s) <= ' '; s++)
		if (!c)
			return s;
	haveq = 0;
	s0 = s1 = s;
	for(;;) {
		if ((c1 = *++s1) == '=') {
			haveq = 1;
			s1++;
			break;
			}
		if (c1 <= ' ') {
			while(c1 && (c1 = *++s1) <= ' ');
			if (c1 == '=') {
				haveq = 1;
				++s1;
				}
			break;
			}
		}
	while((c1 = *s1) && c1 <= ' ')
		s1++;
	if (!haveq && cml)
		return s1;
	if (c >= '0' && c <= '9') {
		c -= '0';
		while((c1 = *++s) >= '0' && c1 <= '9')
			c = 10*c + c1 - '0';
		if (c1 != '=' && c1 > ' ' || c >= N_FNAMES) {
			while(c1 > ' ')
				c1 = *++s;
 no_filename:
			if (mode <= 1) {
				printf(c >= N_FNAMES ? "Unit number > %d"
						     : "???", N_FNAMES-1);
				printf(" %.*s\n", s-s0, s0);
				}
			return s;
			}
		if (!*(s = s1))
			goto no_filename;
		while(*++s > ' ');
		if (mode <= 1) {
			c1 = s - s1;
			s2 = fnames[c] = (char *)Malloc(c1 + 1);
			strncpy(s2, s1, c1);
			s2[c1] = 0;
			}
		return s;
		}
	if (c >= 'A' && c <= 'Z')
		c += 'a' - 'A';
	s3 = buf;
	bufend = s3 + sizeof(buf);
	for(s2 = s; (c = *s2) > ' '; s2++)
		if (s3 < bufend)
			*s3++ = c == '=' || c == '_' ? ' ' : c;
	if (haveq && s2 < s1) {
		s2 = s1;
		if (s3 < bufend)
			*s3++ = ' ';
		for(; (c = *s2) > ' '; s2++)
			if (s3 < bufend)
				*s3++ = c;
		}
	if (mode != 1) {
		inform = 0;
		m3key_(buf, key, &lprint, &lsumm, &inform,
			(ftnlen)(s3-buf), (ftnlen)sizeof(key));
		if (!inform)
			goto echo;
		n_badopts++;
		}
	return s2;
 echo:
	if (option_echo)
		printf("%.*s\n", s2-s0, s0);
	return s2;
	}

 static int
#ifdef KR_headers
envopt(argv, mode) char **argv; int mode;
#else
envopt(char **argv, int mode)
#endif
{
	char *s;

#if MSDOS
#ifndef KSUMM_INIT
#define KSUMM_INIT 1
#endif
	m5freq_.ksumm = KSUMM_INIT;
#endif
	if (s = getenv("minos_options"))
		while(*s)
			s = get_option(s, mode, 0);
	while(s = *++argv)
		while(*s)
			s = get_option(s, mode, 1);
	if (mode <= 1) {
		if (fnames[7])
			m1file_.iprint = 7;
		if (fnames[0] && !freopen(fnames[0], "w", stderr))
			printf("Can't redirect stderr to %s\n", fnames[0]);
		fflush(stdout);
		}
	return n_badopts;
	}

 void
envopt_(VOID)
{ envopt(xargv, 1); }

#ifndef Student_Edition

 void
#ifdef KR_headers
minos2_(z, nwcore, mincor, inform) double *z; fint *nwcore, *mincor, *inform;
#else
minos2_(double *z, fint *nwcore, fint *mincor, fint *inform)
#endif
{
	static fint I2 = 2;
	envopt(xargv, 2);
	if (m2mapz_.maxz < *mincor)
		m2mapz_.maxz = *mincor;
	z = (double *)Malloc(m2mapz_.maxz*sizeof(double));
	m3dflt_(&I2);
	m1tim_.ltime = 1;
	minos3_(z, mincor, inform);
	fflush(stdout);
	fprintf(stderr, "Solution time = %.2f seconds.", m1tim_.tsum[1]);
	fprintf(stderr, *inform ? "  Exit code %ld.\n" : "\n", (long)*inform);
	fflush(stderr);
	}

#endif

 void
#ifdef KR_headers
gfname_(i0, fname, fname_len)
 fint *i0;
 char *fname;
 fint fname_len;
#else
gfname_(fint *i0, char *fname, ftnlen fname_len)
#endif
{
	int i = *i0, L;
	char *s;

	if (i >= N_FNAMES  || i < 0) {
		fprintf(stderr, "gfname called with i = %d\n", i);
		exit(1);
		}
	if (s = fnames[i])
		L = Sprintf(fname, "%s", s);
	else
		L = Sprintf(fname, "fort.%d", i);
	while (L < fname_len)
		fname[L++] = ' ';
	}

#ifdef __cplusplus
}
#endif
