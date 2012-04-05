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

#include "asl.h"

#ifndef KR_headers
#ifdef __cplusplus
extern "C" {
#endif
 extern void funcon_(fint *MODE, fint *M, fint *N, fint *NJAC,
		real *X, real *F, real *G,
		fint *NSTATE, fint *NPROB, real *Z, fint *NWCORE);
 extern void funobj_(fint *MODE, fint *N,
		real *X, real *F, real *G,
		fint *NSTATE, fint *NPROB, real *Z, fint *NWCORE);
#ifdef __cplusplus
	}
#endif
#endif

#undef scream
 static void
#ifdef KR_headers
scream(fmt, j, k) char *fmt;
#else
scream(char *fmt, int j, int k)
#endif
{
	fprintf(Stderr, fmt, j, k);
	exit(1);
	}
 void
#ifdef KR_headers
funcon_(MODE, M, N, NJAC, X, F, G, NSTATE, NPROB, Z, NWCORE)
 fint *MODE, *M, *N, *NJAC, *NSTATE, *NPROB, *NWCORE;
 register real *X;
 real *F, *G, *Z;
#else
funcon_(fint *MODE, fint *M, fint *N, fint *NJAC,
		register real *X, real *F, real *G,
		fint *NSTATE, fint *NPROB, real *Z, fint *NWCORE)
#endif
{
	ASL *asl = cur_ASL;

	if (*NSTATE) {
		if (*NSTATE != 1)
			return;
		if (*N != c_vars) {
			scream("funcon expected N = %d but got %d\n",
				c_vars, (int)*N);
			 /* suppress warning about unused vars: */
			Not_Used(NJAC);
			Not_Used(Z);
			Not_Used(NWCORE);
			Not_Used(NPROB);
			}
		}
	want_deriv = (int)*MODE & 2;
	xknown(X);
#if 1
	conval(X, F, 0);
	if (want_deriv)
		jacval(X, G, 0);
#else
	{int i, m;
	m = (int)*M;
	for(i = 0; i < m; i++)
		*F++ = conival(i,X,0);
	if (want_deriv)
		for(i = 0; i < m; i++)
			congrd(i,X,G,0);
	}
#endif
	xunknown();
	}

 void
#ifdef KR_headers
funobj_(MODE, N, X, F, G, NSTATE, NPROB, Z, NWCORE)
 fint *MODE, *N, *NSTATE, *NPROB, *NWCORE;
 register real *X;
 real *F, *G, *Z;
#else
funobj_(fint *MODE, fint *N,
		register real *X, real *F, real *G,
		fint *NSTATE, fint *NPROB, real *Z, fint *NWCORE)
#endif
{
	int i;
	ASL *asl = cur_ASL;

	if (*NSTATE) {
		if (*NSTATE != 1)
			return;
		if (*N != o_vars) {
			scream("funobj expected N = %d but got %d\n",
				o_vars, (int)*N);
			printf("", Z, NWCORE); /* use unused vars */
			}
		}
	want_deriv = (int)*MODE & 2;
	i = (int)*NPROB;
	if (i < 0 || i >= n_obj) {
		*F = 0.;
		if (want_deriv)
			memset(G, 0, *N*sizeof(real));
		}
	else {
		xknown(X);
		*F = objval(i,X,0);
		if (want_deriv)
			objgrd(i,X,G,0);
		xunknown();
		}
	}
