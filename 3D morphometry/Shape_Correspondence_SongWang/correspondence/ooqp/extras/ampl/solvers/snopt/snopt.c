/****************************************************************
Copyright (C) 1997, 1998, 2001 Lucent Technologies
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

#include "signal.h"
#include "ctype.h"
#include "getstub.h"

/* For (defective) Linux on Alpha chips where infinity arithmetic fails: */
/* compile, e.g., with -DFinite_Plinfy=1e154 */
#ifdef Finite_Plinfy
#undef Infinity
#define Infinity Finite_Plinfy
#endif

#define N_FNAMES 100
static char *fnames[N_FNAMES];

#ifdef __cplusplus
extern "C" {
#endif

typedef void fCon ANSI((fint *MODE, fint *M, fint *N, fint *NJAC,
		real *X, real *F, real *G, fint *NSTATE,
		char *CU, fint *LENCU, fint *IU, fint *LENIU,
		real *RU, fint *LENRU, ftnlen len_cu));

typedef void fObj ANSI((fint *MODE, fint *N,
		real *X, real *F, real *G, fint *NSTATE,
		char *CU, fint *LENCU, fint *IU, fint *LENIU,
		real *RU, fint *LENRU, ftnlen len_cu));

typedef void fQp ANSI((fint*, real*, real*, fint*, char*, fint*,
		fint*, fint*, real*, fint*, ftnlen));

#ifdef KR_headers
 static void funcon();
 static void funobj();
#else
 static fCon funcon;
 static fObj funobj;
#endif
 extern void s1file_ ANSI((fint *, fint*, fint*));
 extern int s3opt_ ANSI((fint*, char*, char*, char*, fint*, real*, fint*,
	fint*, fint*, char*, fint*, fint*, fint*, real*, fint*,
	ftnlen, ftnlen, ftnlen, ftnlen));
 extern int snmem_ ANSI((fint *m, fint *n, fint *ne, fint *negcon,
	fint *nncon, fint *nnjac, fint *nnobj, fint *mincw, fint *miniw,
	fint *minrw, char *cw, fint *lencw, fint *iw, fint *leniw,
	real *rw, fint *lenrw, ftnlen cw_len));
 extern void sninit_  ANSI((fint*, fint*, char*, fint*, fint*,
	fint*, real*, fint*, ftnlen));
 extern void snopt_ ANSI((   char*, fint*, fint*, fint*, fint*, fint*, fint*,
	fint*, fint*, real*, char*, fCon*, fObj*, real*, fint*, fint*, real*,
	real*, char*, fint*, real*, real*, real*, fint*, fint*, fint*, fint*,
	fint*, fint*, real*, real*, char*, fint*, fint*, fint*, real*, fint*,
	char*, fint*, fint*, fint*, real*, fint*,
	ftnlen,ftnlen,ftnlen,ftnlen,ftnlen));
 extern void sntitl_ ANSI((char*, ftnlen));
 extern void sqopt_ ANSI((char *start, fQp*, fint *m, fint *n,
	fint *ne, fint *nname, fint *lenc, fint *ncolh,
	fint *iobj, real *objadd, char *prob,
	real *acol, fint *inda, fint *loca, real *bl, real *bu,
		real *c, char *names,
	fint *helast, fint *hs, real *x, real *pi, real *rc,
	fint *inform, fint *mincw, fint *miniw, fint *minrw,
	fint *ns, fint *ninf, real *sinf, real *obj,
	char *cu, fint *lencu, fint *iu, fint *leniu, real *ru, fint *lenru,
	char *cw, fint *lencw, fint *iw, fint *leniw, real *rw, fint *lenrw,
	ftnlen start_len, ftnlen prob_len, ftnlen names_len, ftnlen cu_len,
	ftnlen cw_len));

#define Aijtol	rw[94]
#define Itn	iw[420]
#define Objsen	iw[86]
#define Objtru	rw[420]
#define Plinfy	rw[69]
#define iPrint	iw[11]
#define iSumm	iw[12]
#define itnlim	iw[88]
#define lDenJ	iw[104]
#define lprSol	iw[83]
#define lvlScl	iw[74]
#define lvlTim	iw[76]
#define maxcw	iw[6]
#define maxiw	iw[4]
#define maxrw	iw[2]
#define maxS	iw[52]
#define nMajor	iw[421]
#define nQNmod	iw[380]
#define neGcon	iw[19]
#define nnCon	iw[22]
#define nnJac	iw[20]
#define nnObj	iw[21]
#define objno	iw[107]

static fint Objno, iObj;
extern char **xargv, xxxvers[];
#ifdef SYMANTEC
extern int _8087;
#endif

 typedef struct
Winfo {
	char *cw;
	fint *iw;
	real *rw;
	fint lencw, leniw, lenrw;
	} Winfo;

 static Jmp_buf Jb;
 static Winfo winfo;
 static char Emsgbuf[96], *Errmsg = Emsgbuf;
 static int Errnum = 599; /* should be assigned if needed */
 static int functimes, wantfuncs, wantver;
 static int nfuncon, nfungrd, nfunjac, nfunobj;
 static int outlev = 1, qpcheck = 1;
 static fint i_exit, last_state, meminc, timing;
 static real Times[6];
 static char Title[32];
#define asl cur_ASL

 static char *usage_msg[] = {
 "  where  stub  is from  `ampl -obstub`  or  `ampl -ogstub`.  Assignments",
 "  have the form  u=filename  or  spec=value  in which the integers u are",
 "  file unit numbers that appear in spec assignments (don't use 5 or 6",
 "  for u) and  spec  is a SNOPT SPECS file keyword phrase, with keywords",
 "  in the phrase separated by _ (underscore).  Use",
 "	outlev=0 for no options echoed on stdout,",
 "	outlev=1 for neither log nor summary file on stdout (default),",
 "	outlev=2 to see summary output on stdout,",
 "	outlev=3 to see detailed (log file) output on stdout, and",
 "	outlev=4 to get log file plus solution on stdout.",
 "  For outlev <= 2, use 7=logfilename to get the log file.  Assignments",
 "  may also be given in $snopt_options .",
		0 };


 static fint
#ifdef KR_headers
mkey(phrase, len) char *phrase; ftnlen len;
#else
mkey(char *phrase, ftnlen len)
#endif
{
	char key[16];
	static fint T = 1, lprint, lsumm = 6;
	char c[8];
	fint i, inform = 0;
	real r;
	s3opt_(&T, phrase, key, c, &i, &r, &lprint, &lsumm, &inform,
		winfo.cw, &winfo.lencw,
		winfo.iw, &winfo.leniw,
		winfo.rw, &winfo.lenrw,
		len, (ftnlen)sizeof(key), (ftnlen)sizeof(c),
		(ftnlen)winfo.lencw);
	return inform;
	}

 static fint
#ifdef KR_headers
nkey(np, fname, L) fint *np; char *fname; ftnlen L;
#else
nkey(fint *np, char *fname, ftnlen L)
#endif
{
	fint n = *np;
	char *s;

	if (n < 0 || n >= N_FNAMES) {
		printf("Unit number > %d\n", N_FNAMES-1);
		return 1;
		}
	fnames[n] = s = (char*)Malloc(L+1);
	memcpy(s, fname, L);
	s[L] = 0;
	return 0;
	}

 static void
#ifdef KR_headers
show_ver(f) FILE *f;
#else
show_ver(FILE *f)
#endif
{
#ifdef Student_Edition
	fprintf(f, "Student ");
#endif
	fprintf(f, "%s", xxxvers+6);
	fflush(f);
	}

 static char *
#ifdef KR_headers
set_outlev(oi, kw, s) Option_Info *oi; keyword *kw; char *s;
#else
set_outlev(Option_Info *oi, keyword *kw, char *s)
#endif
{
	char *rv = I_val(oi,kw,s);
	if (!*(int *)kw->info)
		oi->option_echo = 0;
	return rv;
	}

 static char qpcheck_msg[] =
	"whether to pass LPs and QPs to SQOPT rather than SNOPT:\n\
		0 = no\n\
		1 = yes (default)";

 static keyword keywds[] = {
#ifdef SYMANTEC
	KW("emulate", IK0_val, &_8087, "emulate F.P. arithmetic (Symantec)"),
#endif
	KW("ftimes",  I_val,  &functimes, "report function eval. times"),
	KW("maxfwd",  IA_val, voffset_of(ASL,p.maxfwd_), "max vars in fwd AD of common exprs (default 5)"),
	KW("meminc",  L_val,  &meminc, "increment to minimum memory allocation"),
	KW("objno",   L_val,  &Objno, "objective number: 0 = none, 1 = first (default)"),
	KW("outlev",  set_outlev, &outlev, "output level; 1 = default"),
	KW("qpcheck", I_val, &qpcheck, qpcheck_msg),
	KW("timing",  I_val, &timing,  "report I/O and solution times: 1 = stdout, 2 = stderr, 3 = both"),
	KW("version", I_val, &wantver, "report version"),
	KW("wantsol", WS_val, 0, WS_desc_ASL+5)
	};

 static keyword options[] = {
	KW("f", IK1_val, &wantfuncs, "list available user-defined functions"),
	KW("t", IK1_val, &functimes, "time function evaluations")
	};

 static Option_Info Oinfo = {
	"snopt", Title, "snopt_options", keywds, nkeywds, 1, xxxvers+6,
	usage_msg, mkey, nkey, options, sizeof(options)/sizeof(keyword)
	};

 static void
get_Title(VOID)
{
	/* Get title from sntitl and omit excessive blanks. */

	char *s, *se, *t;
	size_t L;
	int keepblank = 0;

	sntitl_(Title, sizeof(Title));
	s = t = Title;
	se = s + sizeof(Title);
	while(s < se) {
		while(*s > ' ') {
			*t++ = *s++;
			if (s >= se)
				goto done;
			}
		if (!*s || s+1 >= se)
			break;
		if (keepblank || s[1] == ' ') {
			*t++ = *s++;
			keepblank = 0;
			}
		while(*s <= ' ')
			if (!*s || ++s >= se)
				goto done;
		if (*s == '(')/*)*/
			keepblank = 1;
		}
 done:
	*t = 0;
	L = t - Title;
	for(s = xxxvers+6; *s != ','; s++);
#ifdef Student_Edition
	Oinfo.version = se = M1alloc(L + strlen(s) + 9);
	strcpy(se, "Student ");
	se += 8;
#else
	Oinfo.version = se = M1alloc(L + strlen(s) + 1);
#endif
	memcpy(se, Title, L);
	strcpy(se + L, s);
	}

 static SufDecl
suftab[] = {
	{ "sstatus", 0, ASL_Sufkind_var, 1 },
	{ "sstatus", 0, ASL_Sufkind_con, 1 }
	};

 static int
#ifdef KR_headers
envopt(argv) char **argv;
#else
envopt(char **argv)
#endif
{
	int rv;
	fint *iw = winfo.iw;

	rv = getopts(argv, &Oinfo);
	if (fnames[7])
		iPrint = 7;
	if (fnames[0] && !freopen(fnames[0], "w", Stderr))
		printf("Can't redirect Stderr to %s\n", fnames[0]);
	fflush(stdout);
	return rv;
	}

#undef asl

 static fint
#ifdef KR_headers
objmunge(M, mp, N, NZ, nzp, hs, lb, ub, A, ha, ja, objadj, x)
	fint M, *mp, N, NZ, *nzp; fint *hs;
	real *lb, *ub, *A, *objadj, *x; fint *ha; fint *ja;
#else
objmunge(fint M, fint *mp, fint N, fint NZ, fint *nzp, fint *hs,
		real *lb, real *ub, real *A, fint *ha, fint *ja,
		real *objadj, real *x)
#endif
{
	fint i, j, k, na0, ne, nlclim, nlvlim, nz, rv;
	cgrad *cg, **cgx, *ncg;
	ograd *og, **ogp;
	char *h, *he;
	real aijtol, ninf, pinf, t, t1;
	fint *ja1, *kadj, *kadj1;
	real *a1, *a2, *lbe;
	fint *ha1, *ha2, *hae, si;
	fint *iw = winfo.iw;
	real *rw = winfo.rw;
	ASL *asl = cur_ASL;

	rv = ne = na0 = 0;
	i = N*sizeof(fint);
	kadj = (fint *)Malloc(i);
	memset((char *)kadj, 0, i);
	aijtol = Aijtol;
	nz = NZ;
	nlvlim = nlvc;
	nlclim = nlc;

	/* omit tiny components of A */

	if (aijtol > 0) {
		a1 = a2 = A;
		ha1 = ha2 = ha;
		ja1 = ja;
		for(i = 0; i < N; i++) {
			j = *++ja1;
			hae = ha + j - 1;
			while(ha1 < hae) {
				t = *a1++;
				if ((si = *ha1++) > nlclim
				 && i >= nlvlim
				 && (t < 0 ? -t : t) < aijtol) {
					na0++;
					continue;
					}
				*a2++ = t;
				*ha2++ = si;
				}
			*ja1 = j - na0;
			}
		nz -= na0;
		}

	/* find objective, count gradient components */

	*objadj = 0;
	if (Objno >= 0 && Objno < n_obj) {
		if (Objsen == 2)
			Objsen = objtype[Objno] ? -1 : 1;
		if (nl_obj(Objno))
			rv = nlvo;
		else
			*objadj = objconst(Objno);
		ogp = &Ograd[Objno];
		for(og = *ogp; og; og = og->next)
			if (og->varno < rv)
				goto keep;
			else {
				if ((t = og->coef) < 0)
					t = -t;
				if (t >= aijtol) {
				 keep:
					kadj[og->varno] = 1;
					ne++;
					}
				else
					na0++;
				}
		}
	else
		Objsen = 0;
	h = havex0;
	he = h + N;

	pinf = Plinfy;
	ninf = -pinf;

	while(h < he) {
		si = 2;
		if ((t1 = *lb) <= ninf)
			t1 = *lb = ninf;
		else
			si = 0;
		if ((t = *ub) >= pinf)
			t = *ub = pinf;
		else
			si = t <= 0;
		lb++;
		ub++;
		if (*h++ && si != 2)
			if (*x <= t1) {
				*x = t1;
				si = 4;
				}
			else if (*x >= t) {
				*x = t;
				si = 5;
				}
		*hs++ = si;
		x++;
		}

	/* adjust bounds */

	lbe = lb + M;
	while(lb < lbe) {
		si = 2;
		if ((t = *lb) <= ninf)
			t = ninf;
		else
			si = 0;
		if ((t1 = *ub) >= pinf)
			t1 = pinf;
		else
			si = 0;
		*lb++ = t;
		*ub++ = t1;
		*hs++ = si;
		}

	/* insert objective */

	si = M;
	if (ne) {
		iObj = *mp = ++si;
		*lb = ninf;
		*ub = pinf;
		*hs = 0;
		a1 = A + nz;
		ha1 = ha + nz;
		k = nz;
		nz += ne;
		a2 = A + nz;
		ha2 = ha + nz;
		ja1 = ja + N;
		kadj1 = kadj + N;
		for(;;) {
			*ja1-- += ne;
			if (*--kadj1) {
				*kadj1 = a2 - A;
				*--a2 = 0;
				*--ha2 = si;
				if (!--ne)
					break;
				}
			for(j = *ja1; k >= j; k--) {
				*--a2 = *--a1;
				*--ha2 = *--ha1;
				}
			}
		while(og = *ogp)
			if ((i = kadj[og->varno]) && og->varno >= rv) {
				A[i - 1] = og->coef;
				*ogp = og->next;
				}
			else
				ogp = &og->next;
		}
	else
		*mp = M;
	*nzp = nz;
	free((char *)kadj);

	/* adjust for computing Jacobian */

	neGcon = 0;
	if (nlvlim) {
		if (!Cgrad) {
			Cgrad = (cgrad **)Malloc(nlclim*sizeof(cgrad *));
			memset((char *)Cgrad, 0, nlclim*sizeof(cgrad *));
			}
		hae = ha + (ja[nlvlim] - 1);
		for(ha1 = ha, k = 0; ha1 < hae;)
			if (*ha1++ <= nlclim)
				k++;
		neGcon = k;
		ncg = (cgrad *)Malloc(k*sizeof(cgrad));
		i = k = 0;
		ja1 = ja + 1;
		for(a1 = A, ha1 = ha; ha1 < hae; a1++)
			if ((j = *ha1++) <= nlclim) {
				cgx = Cgrad + j - 1;
				cg = ncg++;
				cg->next = *cgx;
				*cgx = cg;
				cg->goff = k++;
				j = ha1 - ha;
				while(j >= *ja1) {
					ja1++;
					i++;
					}
				cg->varno = i;
				cg->coef = *a1;
				*a1 = 0;
				}
		}
	if (na0) {
		printf("%d coefficients less than aij = %g treated as 0.\n",
			na0, aijtol);
		need_nl = 0;
		}
	c_vars = nlvlim;
	return o_vars = rv;
	}

 static void
#ifdef KR_headers
time_out(f) FILE *f;
#else
time_out(FILE *f)
#endif
{
	fprintf(f," SNOPT times:\n read: %10.2f\n solve: %9.2f",
		Times[1] - Times[0], Times[2] - Times[1]);
	fprintf(f, "\n write: %9.2f\n total: %9.2f\n",
		Times[3] - Times[2], Times[3] - Times[0]);
	if (functimes) {
		fprintf(f, "\n");
		if (nfuncon)
			fprintf(f,
	" constraints: %9.2f for %d evaluations, %d Jacobians\n",
				Times[4], nfuncon, nfunjac);
		if (nfunobj)
			fprintf(f,
	" objective: %11.2f for %d evaluations, %d gradients\n",
				Times[5], nfunobj, nfungrd);
		}
	}

 static void
#ifdef KR_headers
negate(M, x) fint M; register real *x;
#else
negate(fint M, register real *x)
#endif
{
	register real *xe;
	for(xe = x + M; x < xe; x++)
		*x = -*x;
	}

 static fint
vtrans[] = { 0, 3, 2, 0, 1, 0, 0 },
ctrans[] = { 0, 3, 2, 1, 0, 0, 0 };

 static void
#ifdef KR_headers
hs1_adjust(N, s, ss, trans) fint N; int *s; fint *ss, *trans;
#else
hs1_adjust(fint N, int *s, fint *ss, fint *trans)
#endif
{
	fint i;

	for(i = 0; i < N; i++)
		ss[i] = trans[s[i]];
	}

 static char *
#ifdef KR_headers
hs_adjust(asl, N, M, m1, vss, vs, vsd, csd, A, ha, ja)
	ASL *asl; fint N, M, m1; fint *vss; int *vs; SufDesc *vsd, *csd;
	real *A; fint *ha; fint *ja;
#else
hs_adjust(ASL *asl, fint N, fint M, fint m1, fint *vss, int *vs, SufDesc *vsd, SufDesc *csd, real *A, fint *ha, fint *ja)
#endif
{
	fint *jae, nerror;
	fint *hae, i1, i2, is;
	int nlin;
	real t, *x, *xe;

	if (!(vsd->kind & ASL_Sufkind_input)
	 || !(csd->kind & ASL_Sufkind_input))
		return "Cold";
	hs1_adjust(N, vs, vss, vtrans);
	hs1_adjust(M, vs+N, vss+N, ctrans);
	if (m1 > M)
		vss[M+N] = 3;
	/* Why can't minos compute the initial slacks? */
	/* Then we could eliminate the following mess... */
	if (nlc) {
		nerror = 0;
		conval(X0, x = X0+N, &nerror);
		if (nerror)
			memset(X0+N, 0, nlc*sizeof(real));
		else
			for(xe = x + nlc; x < xe; x++)
				*x = -*x;
		}
	if ((nlin = n_con - nlc) > 0) {
		x = X0 + N + nlc;
		i1 = nlc + 1;
		i2 = n_con;
		memset(x, 0, nlin*sizeof(real));
		hae = ha;
		xe = X0;
		for(jae = ja + N; ja < jae; ja++) {
			hae += ja[1] - ja[0];
			for(t = *xe++; ha < hae; A++)
				if ((is = *ha++) >= i1 && is <= i2)
					x[is - i1] -= t**A;
			}
		}
	return "Warm";
	}

 static void
#ifdef KR_headers
send_status(N, ss, s, trans, L, U) fint N; fint *ss, *trans; int *s; real *L, *U;
#else
send_status(fint N, fint *ss, int *s, fint *trans, real *L, real *U)
#endif
{
	fint i;

	for(i = 0; i < N; i++)
		if (((s[i] = trans[ss[i]]) + 1 & ~1) == 4 && L[i] == U[i])
			s[i] = 5;
	}

#undef scream
#undef asl

 static void
#ifdef KR_headers
scream(fmt, j, k) char *fmt;
#else
scream(char *fmt, int j, int k)
#endif
{
	fprintf(Stderr, fmt, j, k);
	longjmp(Jb.jb, 1);
	}

 static fint *colq, ncolh, *rowq;
 static real *delsq;

 static void
#ifdef KR_headers
qpHx(ncolH, x, Hx, nState, cu, lencu, iu, leniu, ru, lenru, culen)
	char *cu; fint *iu, *lencu, *leniu, *lenru, *nState, *ncolH;
	ftnlen culen; real *Hx, *ru, *x;
#else
qpHx(fint *ncolH, real *x, real *Hx, fint *nState,
	char *cu, fint *lencu, fint *iu, fint *leniu, real *ru, fint *lenru,
	ftnlen culen)
#endif
{
	fint *cq, i, n, *rq, *rqe;
	real *h, t, *xe;

	n = *ncolH;
	if (n != ncolh) {
		scream("qpHx expected ncolH = %d but got %d\n",
			(int)n, (int)ncolh);
		/* suppress warning about unused variables: */
		Not_Used(nState);
		Not_Used(cu);
		Not_Used(lencu);
		Not_Used(iu);
		Not_Used(leniu);
		Not_Used(ru);
		Not_Used(lenru);
		Not_Used(culen);
		}
	for(i = 0; i < n; i++)
		Hx[i] = 0;
	xe = x + n;
	cq = colq;
	rq = rowq;
	h = delsq;
	while(x < xe) {
		t = *x++;
		rqe = rowq + *++cq;
		while(rq < rqe)
			Hx[*rq++] += t**h++;
		}
	}

jmp_buf fpe_jmpbuf;
#ifndef Sig_ret_type
#define Sig_ret_type void
#endif

extern Sig_ret_type fpecatch ANSI((int));

 void
MAIN__(VOID)
{
	ASL *asl;
	FILE *nl;
	SufDesc *csd, *vsd;
	char *cw, *em, *msg1, *stub, *s, *start;
	char Prob[8], buf[32], msg[360];
	fint *ha, *helast, *hs,*iw, *ja;
	fint INFORM, M, MXROW, MXCOL, N, N1, NB, NO, NS, NZ;
	fint i, m1, mincw, miniw, minrw, nInf, nint, nz, nzh;
	int qpsol, *varstat;
	real *A, *lb, *pi, *rc, *ub;
	real c0, nlobj, objadj, sInf, *rw;
	size_t L0;
	typedef struct { char *msg; int code, wantobj; } Sol_info;
	Sol_info *SI;
	static Sol_info solinfo[] = {
	 { /*0*/ "Optimal solution found", 0, 1 },
	 { /*1*/ "Infeasible problem (or bad starting guess)", 200, 0 },
	 { /*2*/ "Unbounded (or badly scaled) problem", 300, 0 },
	 { /*3*/ "Too many iterations", 400, 1 },
	 { /*4*/ "Primal feasible solution; could not satisfy dual feasibility", 100, 1 },
	 { /*5*/ "The superbasics limit (%ld) is too small", 520, 1 },
	 { /*6*/ "Error evaluating nonlinear expressions", 501, 1 },
	 { /*7*/ "Incorrect gradients from funobj", 502, 0 },
	 { /*8*/ "Incorrect gradients from funcon", 503, 0 },
	 { /*9*/ "The current point cannot be improved", 101, 1 },
	 { /*10*/ "Numerical error: the general constraints\ncannot be satisfied accurately", 201, 1 },
	 { /*11*/ "Arithmetic error", 504, 1 },
	 { /*12*/ "Surprise function evaluation error", 505, 1 },
	 { /*13/20)*/ "Not enough storage for the basis factors", 506, 1 },
	 { /*14/21*/ "Error in basis package", 507, 0 },
	 { /*15/22*/ "Singular basis after several factorization attempts", 508, 0 },
	 { /*16/30*/ "Input basis had wrong dimensions", 509, 0 },
	 { /*17/32*/ "System error: wrong number of basic variables", 510, 0 },
	 { /*18:42*/ "Too little real storage", 511, 0 },
	 { /*19:43*/ "Too little integer storage", 512, 0 },
	 { /*20*/ "Error evaluating initial objective value", 513, 0 },
	 { /*21*/ "Error evaluating initial constraint values", 514, 0 },
	 { /*22*/ "BUG! unexpected return %ld", 515, 0 }
		};
	static char names[8] = "        ";
	static fint I0 = 0, I1 = 1, openF = 1;
	static fint	sctrans[] = { 4, 3, 2, 1 },
			svtrans[] = { 3, 4, 2, 1 };

	Times[0] = xectim_();

	asl = ASL_alloc(ASL_read_fg);
	get_Title();
	Times[4] = Times[5] = 0;
	winfo.lencw = winfo.leniw = winfo.lenrw = 500;
	L0 = 500*(sizeof(fint) + sizeof(real) + 8);
	winfo.rw = rw = (real*)Malloc(L0);
	winfo.iw = iw = (fint*)(rw + 500);
	winfo.cw = cw = (char*)(iw + 500);
	memset(rw, 0, L0);
	sninit_(&I0, &I0, cw, &winfo.lencw, iw, &winfo.leniw,
		rw, &winfo.lenrw, winfo.lencw);
	Plinfy = Infinity;
	lDenJ = 2;
	lvlTim = 1;
	asl->i.congrd_mode = 2;	/* sparse Jacobians */
	stub = getstub(&xargv, &Oinfo);
	if (wantver)
		show_ver(stdout);
	if (wantfuncs)
		show_funcs();
	if (wantfuncs)
		exit(0);
	if (!stub)
		usage_ASL(&Oinfo, 1);

	nl = jacdim(stub, &M, &N, &NO, &NZ, &MXROW, &MXCOL, (fint)strlen(stub));

	if (N <= 0) {
		sprintf(Errmsg = Emsgbuf, "%s has no variables\n", filename);
		Errnum = 516;
 bailout:
		solve_result_num = Errnum;
		write_sol(Errmsg, 0, 0, &Oinfo);
		exit(0);
		}

	suf_declare(suftab, sizeof(suftab)/sizeof(SufDecl));
	itnlim = 99999999;
	nnCon = n_conjac[1] = nlc;
	nnJac = nlvc;
	N1 = N + 1;
	m1 = M + 1;
	NB = N + m1;
	nz = NZ + N;
	Fortran = 1;

	LUv = lb = (real *)Malloc((4*NB + m1 + nz)*sizeof(real)
					+ (N1 + NB + nz)*sizeof(fint) + N);
	LUrhs = lb + N;
	Uvx = ub = LUrhs + m1;
	Urhsx = ub + N;
	X0 = Urhsx + m1;
	A = A_vals = X0 + NB;
	memset((char *)(X0 + N), 0, m1*sizeof(real));
	rc = A + nz;
	pi = pi0 = rc + NB;
	ja = (fint *)(pi + m1);
	ha = ja + N1;
	hs = ha + nz;
	varstat = (int*)M1alloc(NB*sizeof(int));
	vsd = suf_iput("sstatus", ASL_Sufkind_var, varstat);
	csd = suf_iput("sstatus", ASL_Sufkind_con, varstat + N);
	havex0 = (char *)(hs + NB);
	if (sizeof(int) == sizeof(fint))
		A_rownos = (int *)ha;
	else
		A_rownos = (int *)Malloc(NZ*sizeof(int));
	if (sizeof(int) == sizeof(fint))
		A_colstarts = (int *)ja;
	else
		A_colstarts = (int *)Malloc(N1*sizeof(int));
	Fortran = 1;
	asl->i.nlvog = nlvo;
	Objsen = 2;
	Objno = n_obj > 0;
	meminc = 20*(M + N);
	nQNmod = 20;
	lvlTim = -999;
	if (setjmp(Jb.jb))
		goto bailout;
	if (envopt(xargv)) {
		Errmsg = "Bad options";
		Errnum = 517;
		goto bailout;
		}
	if (Objno > n_obj || Objno < 0) {
		sprintf(Errmsg = Emsgbuf, "Objno = %ld must be in [0, %d]",
			(long)Objno, n_obj);
		Errnum = 518;
		goto bailout;
		}
	--Objno;
	objno = obj_no = Objno;
	if ((qpsol = qpcheck && nlc == 0) && nlo > 0) {
		qp_read(nl, 0);
		nzh = nqpcheck((int)Objno, &rowq, &colq, &delsq);
		ncolh = 0;	/* not necessary -- it's file static */
		if (nzh > 0) {
			/* undo Fortran numbering and compute ncolh */
			for(i = 1; i <= N; i++)
				if (colq[i-1]-- < colq[i])
					ncolh = i;
			i = colq[N]--;
			while(--i >= 0)
				rowq[i]--;
			}
		else if (nzh < 0)
			switch(nzh + 2) {
			 case 0: /* unlikely */
				Errmsg = "Objective involves division by 0";
				Errnum = 519;
				goto bailout;

			 case 1: /* nonlinear */
				qpsol = 0;
				qp_opify();
				break;

			 default:
				sprintf(Errmsg = Emsgbuf,
					"Unexpected return %ld from nqpcheck\n",
					(long)nzh);
				Errnum = 521;
				goto bailout;
			 }
		}
	else
		fg_read(nl,0);

	if (sizeof(int) != sizeof(fint)) {
		fint *ja1 = ja;
		fint *ja1e = ja + N1;
		int *ja2 = A_colstarts;
		while(ja1 < ja1e)
			*ja1++ = *ja2++;
		}

	if (nint = nlogv + niv + nlvbi + nlvci + nlvoi) {
		printf("ignoring integrality of %ld variables\n", (long)nint);
		need_nl = 0;
		}

	nnObj = objmunge(M,&m1,N,NZ,&nz,hs,lb,ub,A,ha,ja,&objadj,X0);
	if (qpsol) {
		if (nnObj < ncolh)
			nnObj = ncolh;
		i = N + m1;
		helast = (fint*)M1zapalloc(i*sizeof(fint));
		while(--i >= N)
			helast[i] = 3;
		}
	start = hs_adjust(asl, N, M, m1, hs, varstat, vsd, csd, A, ha, ja);
	if ((i = nnObj) < nnJac)
		i = nnJac;
	if (lvlScl < 0)
		lvlScl = i > 0 ? 1 : 2;

	if (outlev > 1) {
		if (lvlTim == -999)
			lvlTim = 3;
		if (outlev == 2)
			iSumm = 6;
		else {
			iPrint = 6;
			lprSol = 0;
			if (outlev > 3)
				lprSol = 2;
			}
		}
	else if (lvlTim == -999)
		lvlTim = iPrint == 0 && iSumm == 0 ? 0 : 3;

	if ((iSumm == 6 || iPrint == 6) && need_nl) {
		printf("\n");
		need_nl = 0;
		}
	s1file_(&openF, iw, &winfo.leniw);
	mincw = miniw = minrw = 501;
	snmem_(&m1, &N, &nz, &neGcon, &nnCon, &nnJac, &nnObj,
		&mincw, &miniw, &minrw, cw, &winfo.lencw,
		iw, &winfo.leniw, rw, &winfo.lenrw, (ftnlen)8);
	if (meminc <= 0)
		meminc = 20*(M + N);
	maxiw = winfo.leniw = miniw + 2*meminc;
	maxrw = winfo.lenrw = minrw + meminc;
	maxcw = winfo.lencw = mincw;
	mincw = miniw = minrw = 501;
	winfo.rw = (real*)Malloc(
				  winfo.leniw*sizeof(fint)
				+ winfo.lenrw*sizeof(real)
				+ winfo.lencw*8);
	winfo.iw = (fint*)(winfo.rw + winfo.lenrw);
	winfo.cw = (char*)(winfo.iw + winfo.leniw);
	memcpy(winfo.rw, rw, 500*sizeof(real));
	memcpy(winfo.iw, iw, 500*sizeof(fint));
	memcpy(winfo.cw, cw, 500*8);
	free(rw);
	rw = winfo.rw;
	iw = winfo.iw;
	cw = winfo.cw;
	*stub_end = 0;
	i = strlen(s = basename(stub));
	if (i > 8)
		i = 8;
	memcpy(Prob, s, i);
	while(i < 8)
		Prob[i++] = ' ';
	INFORM = NS = 0;
	err_jmp1 = &Jb;
	if (setjmp(Jb.jb))
 longjumped:
		i = 12;
	else {
		if (setjmp(fpe_jmpbuf)) {
			report_where(asl);
			printf("\nFloating point error.\n");
			fflush(stdout);
			need_nl = 0;
			goto longjumped;
			}
#ifdef SYMANTEC
		if (_8087)
#endif
		signal(SIGFPE, fpecatch);
		if (Objsen < 0)
			negate(M,pi);
		if (m1 > M)
			pi[M] = 0;

		Times[1] = xectim_();
		if (qpsol)
		  sqopt_(start, qpHx, &m1,
			&N, &nz, &I1, &I0, &ncolh,
			&iObj, &objadj, Prob,
			A, ha, ja, lb, ub, &c0, names,
			helast, hs, X0, pi, rc,
			&INFORM, &mincw, &miniw, &minrw,
			&NS, &nInf, &sInf, &nlobj,
			0, &I0, 0, &I0, 0, &I0,
			cw, &winfo.lencw, iw, &winfo.leniw, rw, &winfo.lenrw,
			(ftnlen)4, (ftnlen)8, (ftnlen)8, (ftnlen)0,
			winfo.lencw);
		else
		  snopt_(start, &m1, &N, &nz, &I1,
			&nnCon, &nnObj, &nnJac,
			&iObj, &objadj, Prob,
			funcon, funobj,
			A, ha, ja, lb, ub, names,
			hs, X0, pi, rc,
			&INFORM, &mincw, &miniw, &minrw,
			&NS, &nInf, &sInf, &nlobj,
			0, &I0, 0, &I0, 0, &I0,
			cw, &winfo.lencw, iw, &winfo.leniw, rw, &winfo.lenrw,
			(ftnlen)4, (ftnlen)8, (ftnlen)8, (ftnlen)0,
			winfo.lencw);
		i = INFORM;
		}
	Times[2] = xectim_();
	if (functimes && !timing)
		timing = 1;
	if (need_nl && timing & 1) {
		printf("\n");
		need_nl = 0;
		}
	msg1 = msg + (wantver	? Sprintf(msg, "%s", xxxvers+6)
				: Sprintf(msg, "%s\n", Title));
	switch(i) {
		case 5:
			INFORM = maxS; break;
		case 20:
		case 21:
		case 22:
			i -= 7; break;
		case 30:
			i = 16; break;
		case 32:
			i = 17; break;
		case 42:
		case 43:
			i -= 24; break;
		case -1:
			if (last_state == 1) {
				i = i_exit;
				break;
				}
		default:
			if (i < 0 || i > 12)
				i = 22;
		}
	SI = solinfo + i;
	solve_result_num = SI->code;
	em = SI->msg;
	if (i == 3 && Itn >= nMajor && Itn < itnlim)
		em = "too many major iterations";
	msg1 += Sprintf(msg1, em, (long)INFORM);
	msg1 += Sprintf(msg1, ".\n%ld iterations", Itn);
	if (SI->wantobj) {
		g_fmtop(buf, Objtru);
		msg1 += Sprintf(msg1, ", objective %s", buf);
		}
	else if (i == 1) {
		g_fmtop(buf, sInf);
		msg1 += Sprintf(msg1, ", infeasibility sum %s", buf);
		}
	if (nfuncon + nfunobj) {
		msg1 += Sprintf(msg1, "\nNonlin evals: ");
		if (nfunobj)
			msg1 += Sprintf(msg1, "obj = %d, grad = %d%s",
				nfunobj, nfungrd, nfuncon ? ", " : "");
		if (nfuncon)
			msg1 += Sprintf(msg1, "constrs = %d, Jac = %d",
				nfuncon, nfunjac);
		Sprintf(msg1, ".");
		}
	send_status(N, hs, varstat, svtrans, LUv, Uvx);
	send_status(M, hs+N, varstat+N, sctrans, LUrhs, Urhsx);
	write_sol(msg, X0, pi, &Oinfo);
	if (timing) {
		Times[3] = xectim_();
		if (timing & 1)
			time_out(stdout);
		if (timing & 2) {
			fflush(stdout);
			time_out(Stderr);
			}
		}
	}

 void
#ifdef KR_headers
getfnm_(i0, fname, last, fname_len)
 fint *i0, *last;
 char *fname;
 fint fname_len;
#else
getfnm_(fint *i0, char *fname, fint *last, ftnlen fname_len)
#endif
{
	int i = *i0, L;
	char *s;

	if (i >= N_FNAMES  || i < 0) {
		sprintf(Errmsg = Emsgbuf, "File assignment nn=filename with nn = %d not in [0, %d]", i, N_FNAMES-1);
		Errnum = 517;
		longjmp(Jb.jb, 1);
		}
	if (s = fnames[i])
		L = Sprintf(fname, "%s", s);
	else
		L = Sprintf(fname, "fort.%d", i);
	*last = L;
	while (L < fname_len)
		fname[L++] = ' ';
	}

 static void
#ifdef KR_headers
funcon(MODE, M, N, NJAC, X, F, G, NSTATE, CU, LENCU, IU, LENIU,
	RU, LENRU, len_cu)
 fint *MODE, *M, *N, *NJAC, *NSTATE, *IU, *LENCU, *LENIU, *LENRU;
 real *X, *F, *G, *RU; char *CU; ftnlen len_cu;
#else
funcon(fint *MODE, fint *M, fint *N, fint *NJAC,
		real *X, real *F, real *G, fint *NSTATE,
		char *CU, fint *LENCU, fint *IU, fint *LENIU,
		real *RU, fint *LENRU, ftnlen len_cu)
#endif
{
	real t0;
	int wantder;
	ASL *asl;
	fint nerror = 0;

	if (functimes)
		t0 = xectim_();

	asl = cur_ASL;
	if (last_state = *NSTATE) {
		if (*NSTATE != 1)
			return;
		i_exit = 21;
		if (*N != c_vars) {
			scream("funcon expected N = %d but got %d\n",
				c_vars, (int)*N);
			/* suppress warning about unused vars: */
			Not_Used(M);
			Not_Used(NJAC);
			Not_Used(CU);
			Not_Used(LENCU);
			Not_Used(IU);
			Not_Used(LENIU);
			Not_Used(RU);
			Not_Used(LENRU);
			Not_Used(len_cu);
			}
		}
	nfuncon++;
	wantder = 0;
	if ((int)*MODE & 2) {
		wantder = 1;
		nfunjac++;
		}
	want_deriv = wantder;
	xknowne(X, &nerror);
#if 1
	if (nerror <= 0)
		conval(X, F, &nerror);
	if (nerror > 0) {
		*MODE = -1;
		return;
		}
	if (wantder)
		jacval(X, G, 0);
#else
	{int i, m;
	m = (int)*M;
	for(i = 0; i < m; i++)
		*F++ = conival(i,X,0);
	if (wantder)
		for(i = 0; i < m; i++)
			congrd(i,X,G,0);
	}
#endif
	xunknown();
	if (functimes)
		Times[4] += xectim_() - t0;
	}

 static void
#ifdef KR_headers
funobj(MODE, N, X, F, G, NSTATE, CU, LENCU, IU, LENIU, RU, LENRU, len_cu)
 fint *MODE, *N, *NSTATE, *IU, *LENCU, *LENIU, *LENRU;
 real *X, *F, *G, *RU; char *CU; ftnlen len_cu;
#else
funobj(fint *MODE, fint *N,
		real *X, real *F, real *G, fint *NSTATE,
		char *CU, fint *LENCU, fint *IU, fint *LENIU,
		real *RU, fint *LENRU, ftnlen len_cu)
#endif
{
	int i, wantder;
	real t0;
	fint nerror = 0;
	ASL *asl = cur_ASL;

	if (functimes)
		t0 = xectim_();
	if (last_state = *NSTATE) {
		if (*NSTATE != 1)
			return;
		i_exit = 20;
		if (*N != o_vars) {
			scream("funobj expected N = %d but got %d\n",
				o_vars, (int)*N);
			/* suppress warning about unused vars: */
			Not_Used(CU);
			Not_Used(LENCU);
			Not_Used(IU);
			Not_Used(LENIU);
			Not_Used(RU);
			Not_Used(LENRU);
			Not_Used(len_cu);
			}
		}
	nfunobj++;
	wantder = 0;
	if ((int)*MODE & 2) {
		nfungrd++;
		wantder = 1;
		}
	want_deriv = wantder;
	i = Objno;
	if (i < 0 || i >= n_obj) {
		*F = 0.;
		if (wantder)
			memset(G, 0, *N*sizeof(real));
		}
	else {
		xknowne(X, &nerror);
		if (nerror >= 0)
			*F = objval(i,X,&nerror);
		if (nerror > 0) {
			*MODE = -1;
			return;
			}
		if (wantder)
			objgrd(i,X,G,0);
		xunknown();
		}
	if (functimes)
		Times[5] += xectim_() - t0;
	}

#ifdef __cplusplus
}
#endif
