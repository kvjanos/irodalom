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
#ifndef Long
#define Long long
#endif
#ifndef fint
#define fint Long
#endif

#ifdef KR_headers
#ifndef _SIZE_T
#define _SIZE_T
typedef int size_t;
#endif
 extern char *malloc();

 char *
mymalloc_ASL(Len) fint Len;
#else
#include "stdlib.h"

 void *
mymalloc_ASL(fint Len)
#endif
{
#ifdef KR_headers
	char *rv;
#else
	void *rv;
#endif
	size_t len = (size_t) Len;
	if (sizeof(Len) != sizeof(len) && Len != (fint)len) {
		fprintf(stderr,
			"malloc(%lu) failure: problem too large.\n", (long)Len);
		exit(1);
		}
	rv = malloc(len);
	if (!rv) {
		fprintf(stderr,
			"malloc(%lu) failure: ran out of memory.\n", (long)Len);
		exit(1);
		}
	return rv;
	}
