/****************************************************************
Copyright (C) 1997-1998 Lucent Technologies
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

#define N_FNAMES 100
static char *fnames[N_FNAMES];

#ifdef __cplusplus
extern "C" {
#else
#endif

#ifdef short_ints
typedef short int shortint;
#else
typedef fint shortint;
#endif

 extern void m1init_ ANSI((char *title, ftnlen title_len));
 extern void m2core_ ANSI((fint *mode, fint *mincor));
 extern void m3dflt_ ANSI((fint *mode));
 extern void m3key_ ANSI((char *buffer, char *key, fint *lprnt, fint *lsumm,
	fint *inform, ftnlen buffer_len, ftnlen key_len));
 extern void mifile_ ANSI((fint *mode));
 extern void minos1_ ANSI((real*, fint*));
 extern void minos3_ ANSI((real*, fint*, fint*));
 extern void minos4_ ANSI((fint *m, fint *n, fint *ne, fint *nnconx,
	fint  *nnobjx, fint *nnjacx, char *names,
	shortint *ha, fint *ka, ftnlen names_len));
 extern void misolv_ ANSI((fint *mimode, char *start, fint *mxx, fint *nxx,
	fint *nbxx, fint *nexx, fint *nkax, fint *nnamex, fint *iobjxx,
	real *objadd, real *a, shortint *ha, fint *ka,
	real *bl, real *bu, fint *name1, fint *name2,
	shortint *hs, real *xn, real *pi, real *rc,
	fint *inform, fint *ns, real *z, fint *nwcore, ftnlen start_len));
extern char **xargv, xxxvers[];
#ifdef SYMANTEC
extern int _8087;
#endif

 static int functimes, timing, wantver;
 static int outlev = 1;
 static real tsolve;
#define asl cur_ASL

 extern struct {
	real objtru, suminf;
	fint numinf;
	} cycle2_;

 extern struct { fint jverif[4], lverif[2]; } m8veri_;

 extern struct {
	fint alone, ampl, gams, mint, page1, page2;
	} m1env_;

 extern struct {
	fint iread, iprint, isumm;
	} m1file_;

 extern struct {
	real eps, eps0, eps1, eps2, eps3, eps4, eps5, plinfy;
	} m1eps_;

 extern struct {
    real tlast[5], tsum[5];
    fint numt[5], ltime;
	} m1tim_;

 extern struct {
	fint minlu, maxlu, lena, nbelem, ip, iq, lenc, lenr, locc, locr, iploc,
		iqloc, lua, indc, indr;
	} m2lu1_;

 extern struct {
	real parmlu[30];
	fint luparm[30];
	} m2lu4_;

 extern struct {
	fint maxw, maxz;
	} m2mapz_;

 extern struct {
	real aijtol, bstruc[2];
	fint mlst, mer;
	real aijmin, aijmax;
	fint na0, line, ier[20];
	} m3mps3_;

 extern struct {
	real sclobj, scltol;
	fint lscale;
	} m3scal_;

 extern struct {
	fint kchk, kinv, ksav, klog, ksumm, i1freq, i2freq, msoln;
	} m5freq_;

 extern struct {
	fint maxr, maxs, mbs, nn, nn0, nr, nx;
	} m5len_;

 extern struct {
	real sinf, wtobj;
	fint minimz, ninf, iobj, jobj, kobj;
	} m5lobj_;

 extern struct {
	fint prnt0, prnt1, summ0, summ1, newhed;
	} m5log4_;

 extern struct {
	fint itn, itnlim, nphs, kmodlu, kmodpi;
	} m5lp1_;

 extern struct {
	fint nfcon[4], nfobj[4], nprob, nstat1, nstat2;
	} m8func_;

 extern struct {
	real penpar, rowtol;
	fint ncom, nden, nlag, nmajor, nminor;
	} m8al1_;

 extern struct {
	fint njac, nncon, nncon0, nnjac;
	} m8len_;

#define objsen m5lobj_.minimz

 static char *usage_msg[] = {
 "  where  stub  is from  `ampl -obstub`  or  `ampl -ogstub`.  Assignments",
 "  have the form  u=filename  or  spec=value  in which the integers u are",
 "  file unit numbers that appear in spec assignments (don't use 5 or 6",
 "  for u) and  spec  is a MINOS SPECS file keyword phrase, with keywords",
 "  in the phrase separated by _ (underscore).  Use",
 "	outlev=0 for no options echoed on stdout,",
 "	outlev=1 for neither log nor summary file on stdout (default),",
 "	outlev=2 to see summary output on stdout,",
 "	outlev=3 to see detailed (log file) output on stdout, and",
 "	outlev=4 to get log file plus solution on stdout.",
 "  For outlev <= 2, use 7=logfilename to get the log file.  Assignments",
#ifdef Student_Edition
 "  may also be given in $minos_options .",
#else
 "  may also be given in $minos_options .  No stub ==> read SPECS file on",
 "  stdin, followed (unless the SPECS file says otherwise) by an MPS file.",
#endif
		0 };

 static fint
#ifdef KR_headers
mkey(phrase, len) char *phrase; ftnlen len;
#else
mkey(char *phrase, ftnlen len)
#endif
{
	char key[16];
	static fint lprint, lsumm = 6;
	fint inform = 0;
	m3key_(phrase, key, &lprint, &lsumm, &inform, len, (ftnlen)sizeof(key));
	return (int)inform;
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
	fnames[n] = s = (char*)M1alloc(L+1);
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
	fprintf(f, xxxvers+6);
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

 static keyword keywds[] = {
#ifdef SYMANTEC
	KW("emulate", IK0_val, &_8087, "emulate F.P. arithmetic (Symantec)"),
#endif
	KW("ftimes",  I_val,  &functimes, "report function eval. times"),
	KW("maxfwd",   IA_val, voffset_of(ASL,p.maxfwd_), "max vars in fwd AD of common exprs (default 5)"),
	KW("objno",    L_val,  &m8func_.nprob, "objective number: 0 = none, 1 = first (default)"),
	KW("outlev",  set_outlev, &outlev, "output level; 1 = default"),
	KW("timing",  I_val, &timing,  "report I/O and solution times: 1 = stdout, 2 = stderr, 3 = both"),
	KW("version", I_val, &wantver, "report version"),
	KW("wantsol", WS_val, 0, WS_desc_ASL+5)
	};

 static keyword options[] = {
	KW("t", IK1_val, &functimes, "time function evaluations")
	};

 static Option_Info Oinfo = {
	"minos", "MINOS 5.5", "minos_options", keywds, nkeywds, 1, xxxvers+6,
	usage_msg, mkey, nkey, options, sizeof(options)/sizeof(keyword)
	};

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

	rv = getopts(argv, &Oinfo);
	if (fnames[7])
		m1file_.iprint = 7;
	if (fnames[0] && !freopen(fnames[0], "w", Stderr))
		printf("Can't redirect Stderr to %s\n", fnames[0]);
	fflush(stdout);
	return rv;
	}

#undef asl

 void
envopt_(VOID)
{
	Oinfo.n_keywds = 0;	/* stand-alone usage */
	envopt(xargv);
	}


 static fint
#ifdef KR_headers
objmunge(M, mp, N, NZ, nzp, hs, lb, ub, A, ha, ja, objadj)
	fint M, *mp, N, NZ, *nzp; shortint *hs;
	real *lb, *ub, *A, *objadj; shortint *ha; fint *ja;
#else
objmunge(fint M, fint *mp, fint N, fint NZ, fint *nzp, shortint *hs,
		real *lb, real *ub, real *A, shortint *ha, fint *ja,
		real *objadj)
#endif
{
	fint i, j, k, na0, ne, nlclim, nlvlim, nz, rv;
	cgrad *cg, **cgx, *ncg;
	ograd *og, **ogp;
	char *h, *he;
	real aijtol, ninf, pinf, t, t1;
	fint *ja1, *kadj, *kadj1;
	real *a1, *a2, *lbe;
	shortint *ha1, *ha2, *hae, si;
	ASL *asl = cur_ASL;

#define objno m8func_.nprob
	rv = ne = na0 = 0;
	i = N*sizeof(fint);
	kadj = (fint *)Malloc(i);
	memset((char *)kadj, 0, i);
	aijtol = m3mps3_.aijtol;
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
	if (objno >= 0 && objno < n_obj) {
		obj_no = objno;
		if (!objsen)
			objsen = objtype[objno] ? -1 : 1;
		if (nl_obj(objno))
			rv = nlvo;
		else
			*objadj = objconst(objno);
		ogp = &Ograd[objno];
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
	h = havex0;
	he = h + N;

	pinf = m1eps_.plinfy;
	ninf = -pinf;

	while(h < he) {
		if ((t = *ub) > pinf)
			t = *ub = pinf;
		if ((t1 = *lb) < ninf)
			t1 = *lb = ninf;
		lb++;
		ub++;
		if (*h++ || t1 <= ninf && t >= pinf)
			si = -1;
		else
			si = t <= 0.;
		*hs++ = si;
		}

	/* adjust for MINOS's surprising notion of bounds */

	lbe = lb + M;
	while(lb < lbe) {
		if ((t = -*lb) > pinf)
			t = pinf;
		if ((t1 = -*ub) < ninf)
			t1 = ninf;
		*lb++ = t1;
		*ub++ = t;
		*hs++ = t1 < t ? -1 : t <= 0.;
		}

	/* insert objective */

	si = (shortint)M;
	if (ne) {
		m5lobj_.iobj = *mp = ++si;
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

	if (nlvlim) {
		if (!Cgrad) {
			Cgrad = (cgrad **)M1alloc(nlclim*sizeof(cgrad *));
			memset((char *)Cgrad, 0, nlclim*sizeof(cgrad *));
			}
		hae = ha + (ja[nlvlim] - 1);
		for(ha1 = ha, k = 0; ha1 < hae;)
			if (*ha1++ <= nlclim)
				k++;
		ncg = (cgrad *)M1alloc(k*sizeof(cgrad));
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
	m3mps3_.na0 = na0;
	c_vars = nlvlim;
	return o_vars = rv;
	}

 static void
#ifdef KR_headers
time_out(tot, f) real tot; FILE *f;
#else
time_out(real tot, FILE *f)
#endif
{
	if (timing) {
		fprintf(f," MINOS times:\n read: %10.2f\n solve: %9.2f",
			m1tim_.tsum[0], tsolve);
		if (m1tim_.tsum[1] < tsolve)
			fprintf(f, "\texcluding minos setup: %.2f",
				m1tim_.tsum[1]);
		fprintf(f, "\n write: %9.2f\n total: %9.2f\n",
			m1tim_.tsum[2], tot);
		}
	if (m1tim_.ltime > 1 && m1tim_.numt[3] | m1tim_.numt[4]) {
		if (timing)
			fprintf(f, "\n");
		if (m1tim_.numt[3])
			fprintf(f,
	" constraints: %9.2f sec. for %ld evaluations, %ld Jacobians\n",
				m1tim_.tsum[3], (long)m1tim_.numt[3],
				(long)m8func_.nfcon[1]);
		if (m1tim_.numt[4])
			fprintf(f,
	" objective: %11.2f sec. for %ld evaluations, %ld gradients\n",
				m1tim_.tsum[4], (long)m1tim_.numt[4],
				(long)m8func_.nfobj[1]);
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

 static shortint
vtrans[] = { 0, 3, 2, 0, 1, 0, 0 },
ctrans[] = { 0, 3, 2, 1, 0, 0, 0 };

 static void
#ifdef KR_headers
hs1_adjust(N, s, ss, trans) fint N; int *s; shortint *ss, *trans;
#else
hs1_adjust(fint N, int *s, shortint *ss, shortint *trans)
#endif
{
	fint i;

	for(i = 0; i < N; i++)
		ss[i] = trans[s[i]];
	}

 static char *
#ifdef KR_headers
hs_adjust(asl, N, M, m1, vss, vs, vsd, csd, A, ha, ja)
	ASL *asl; fint N, M, m1; shortint *vss; int *vs; SufDesc *vsd, *csd;
	real *A; shortint *ha; fint *ja;
#else
hs_adjust(ASL *asl, fint N, fint M, fint m1, shortint *vss, int *vs, SufDesc *vsd, SufDesc *csd, real *A, shortint *ha, fint *ja)
#endif
{
	fint *jae, nerror;
	shortint *hae, i1, i2, is;
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
send_status(N, ss, s, trans, L, U) fint N; shortint *ss; int *s; shortint *trans;
	real *L, *U;
#else
send_status(fint N, shortint *ss, int *s, shortint *trans, real *L, real *U)
#endif
{
	fint i;

	for(i = 0; i < N; i++)
		if (((s[i] = trans[ss[i]]) + 1 & ~1) == 4 && L[i] == U[i])
			s[i] = 5;
	}

#ifndef Student_Edition
 static int
#ifdef KR_headers
file_assignment(s) char *s;
#else
file_assignment(char *s)
#endif
{
	int c;

	if ((c = *s++) < '0' || c > '9')
		return 0;
	while((c = *s++) >= '0' && c <= '9');
	return c == '=';
	}
#endif

static Jmp_buf Jb;
jmp_buf fpe_jmpbuf;
#ifndef Sig_ret_type
#define Sig_ret_type void
#endif

extern Sig_ret_type fpecatch ANSI((int));

 void
MAIN__(VOID)
{
	char *stub;
	real *A, *lb, *pi, *rc, *ub, *z, z1;
	static fint I1 = 1, I2 = 2, I3 = 3, I4 = 4;
	FILE *nl;
	fint INFORM, M, MXROW, MXCOL, N, N1, NB, NO, NS, NZ, m1, nz;
	fint NNCONX, NNOBJX, NNJACX, mincor, nwcore;
	fint i, nint;
	fint *ja;
	shortint *ha, *hs;
	static fint MODE;
	char buf[32], msg[320], *msg1, *start;
	real objadj, t, tout, tstart;
	ASL *asl;
	int *varstat;
	SufDesc *csd, *vsd;
	static shortint	sctrans[] = { 4, 3, 2, 1 },
			svtrans[] = { 3, 4, 2, 1 };
	static char names[] =
#if 0
		"problem objectivrhs     range   bounds  ";
#else
		"                                        ";
#endif
	typedef struct { char *msg; int code; int wantobj; } Sol_info;
	static Sol_info solinfo[] = {
	 {/* 0 */ "optimal solution found", 000, 1},
	 {/* 1 */ "infeasible problem (or bad starting guess)", 200, 0},
	 {/* 2 */ "unbounded (or badly scaled) problem", 300, 0},
	 {/* 3 */ "too many iterations", 400, 1},
	 {/* 4 */ "the objective has not changed for the last %ld iterations", 500, 1},
	 {/* 5 */ "the superbasics limit (%ld) is too small", 520, 1},
	 {/* 6 */ "error evaluating nonlinear expressions", 521, 1},
	 {/* 7 */ "incorrect gradients from funobj", 530, 0},
	 {/* 8 */ "incorrect gradients from funcon", 531, 0},
	 {/* 9 */ "the current point cannot be improved", 501, 1},
	 {/* 10*/ "numerical error: the general constraints\ncannot be satisfied accurately", 201, 1},
	 {/* 11*/ "cannot find superbasic to replace basic variable", 532, 1},
	 {/* 12*/ "basis factorization requested twice in a row", 533, 1},
	 {/* 13*/ "optimal solution found?  Optimality\ntests satisfied, but reduced gradient is large", 100, 1},
	 {/* 14*/ "not enough storage for the basis factors.\nTry rerunning with workspace_(total)=%ld in $minos_options ", 522, 0},
	 {/* 15*/ "error in basis package", 534, 0},
	 {/* 16*/ "singular basis after several factorization attempts", 510, 1},
	 {/* 17*/ "input basis had wrong dimensions", 535, 0},
	 {/* 18*/ "unexpected return code (%ld)", 536, 0},
	 {/* 19*/ "solution aborted", 540, 1},
	 {/* 20*/ "too many major iterations", 401, 1},
	 {/* 21*/ "infeasible problem", 200, 0},
	 };

	tstart = xectim_();
	asl = ASL_alloc(ASL_read_fg);
	asl->i.congrd_mode = 2;	/* sparse Jacobians */
	stub = getstub(&xargv, &Oinfo);
	if (wantver)
		show_ver(stdout);
#ifdef Student_Edition
	if (!stub)
		usage_ASL(&Oinfo, 1);
#else
	if (!stub || file_assignment(stub)) {
		minos1_(&z1, &I1);
		return;
		}
#endif

	nl = jacdim(stub, &M, &N, &NO, &NZ, &MXROW, &MXCOL, (fint)0);

	if (N <= 0) {
		fprintf(Stderr, "%s has no variables\n", filename);
		exit(4);
		}

	suf_declare(suftab, sizeof(suftab)/sizeof(SufDecl));
	NNCONX = n_conjac[1] = nlc;
	NNJACX = nlvc;
	N1 = N + 1;
	m1 = M + 1;
	NB = N + m1;
	nz = NZ + N;
	Fortran = 1;

	LUv = lb = (real *)M1alloc((4*NB + m1 + nz)*sizeof(real)
					+ N1*sizeof(fint)
					+ (NB + nz)*sizeof(shortint) + N);
	LUrhs = lb + N;
	Uvx = ub = LUrhs + m1;
	Urhsx = ub + N;
	X0 = Urhsx + m1;
	A = A_vals = X0 + NB;
	memset((char *)(X0 + N), 0, m1*sizeof(real));
	rc = A + nz;
	pi = pi0 = rc + NB;
	ja = (fint *)(pi + m1);
	ha = (shortint *)(ja + N1);
	hs = ha + nz;
	varstat = (int*)M1alloc(NB*sizeof(int));
	vsd = suf_iput("sstatus", ASL_Sufkind_var, varstat);
	csd = suf_iput("sstatus", ASL_Sufkind_con, varstat + N);
	havex0 = (char *)(hs + NB);
	if (sizeof(int) == sizeof(shortint))
		A_rownos = (int *)ha;
	else
		A_rownos = (int *)M1alloc(NZ*sizeof(int));
	if (sizeof(int) == sizeof(fint))
		A_colstarts = (int *)ja;
	else
		A_colstarts = (int *)M1alloc(N1*sizeof(int));
	Fortran = 1;
	asl->i.nlvog = nlvo;
	fg_read(nl,0);
	if (sizeof(int) != sizeof(shortint)) {
		int *ia, *iae;
		shortint *ha1 = ha;
		ia = A_rownos;
		iae = ia + NZ;
		while(ia < iae)
			*ha1++ = *ia++;
		}
	if (sizeof(int) != sizeof(fint)) {
		fint *ja1 = ja;
		fint *ja1e = ja + N1;
		int *ja2 = A_colstarts;
		while(ja1 < ja1e)
			*ja1++ = *ja2++;
		}

	m3dflt_(&I1);
	objsen = 0;
	m8veri_.lverif[0] = m5freq_.msoln = -11111;
	m5lp1_.itnlim = 99999999;
	objno = n_obj > 0;
	if (functimes)
		timing = 1;
	if (envopt(xargv))
		exit(2);
	m1tim_.ltime = functimes + 1;
	if (objno > n_obj) {
		printf("objno = %ld must be <= %d\n", (long)objno, n_obj);
		exit(2);
		}
	--objno;
	if ((obj_no = (int)m8func_.nprob)
	 && (obj_no < -1 || obj_no >= n_obj)) {
		printf("Bad problem number %d: not in [0,%d]\n",obj_no,n_obj);
		exit(2);
		}

	if (nint = nlogv + niv + nlvbi + nlvci + nlvoi) {
		printf("ignoring integrality of %ld variables\n", (long)nint);
		need_nl = 0;
		}

	if (m5freq_.msoln == -11111)
		m5freq_.msoln = 0;
	if (outlev > 1) {
		if (outlev == 2)
			m1file_.iprint = m1file_.isumm = 6;
		else {
			m1file_.iprint = 6;
			if (outlev > 3)
				m5freq_.msoln = 2;
			}
		}
	if (m8veri_.lverif[0] == -11111
	 && (m1file_.iprint > 0 ||  m1file_.isumm > 0))
		m8veri_.lverif[0] = 0;
	if ((m1file_.isumm == 6 || m1file_.iprint == 6)
	 && need_nl) {
		printf("\n");
		need_nl = 0;
		}
	m8al1_.nden = 2;	/* sparse Jacobian! */
	mifile_(&I1);
	m1init_(msg, (ftnlen)30);
	NNOBJX = objmunge(M, &m1, N, NZ, &nz, hs, lb, ub, A, ha, ja, &objadj);
	start = hs_adjust(asl, N, M, m1, hs, varstat, vsd, csd, A, ha, ja);
	NB = N + m1;
	minos4_(&m1, &N, &nz, &NNCONX, &NNOBJX, &NNJACX, names,
		ha, ja, (ftnlen)8);
	i = m8len_.njac;	/* used not to be necessary */
	m2core_(&I1, &mincor);
	m8len_.njac = i;	/* scrogged by m2core */
	if (mincor < m2mapz_.maxz)
		mincor = m2mapz_.maxz;
	else
		m2mapz_.maxz = mincor;
	nwcore = mincor;
	m2core_(&I4, &i);
	if (i > mincor) {
		fprintf(Stderr,
			"MINOS first wanted %ld words, now it wants %ld\n",
			(long)mincor, (long)i);
		exit(2);
		}

	/* open files mentioned on command line or in $minos_options */
	mifile_(&I2);

	/* show parameters if print_level >= 1 and outlev > 1 */
	if (outlev > 1 && m5log4_.prnt1)
		m3dflt_(&I3);

	/* The following nonsense used not to be necessary. */
	if (outlev == 2)
		m1file_.iprint = m5log4_.prnt0 = m5log4_.prnt1 = 0;
	m1env_.ampl = 1; /* prevent "Solution not printed" */

	z = (real *)M1alloc(mincor*sizeof(real));
	INFORM = NS = 0;
	t = xectim_();
	m1tim_.tsum[0] = t - tstart;
	err_jmp1 = &Jb;
	if (setjmp(Jb.jb))
 longjumped:
		i = 19;
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
		if (objsen < 0)
			negate(M,pi);
		if (m1 > M)
			pi[M] = 0;
		misolv_(&MODE, start, &m1, &N,
			&NB, &nz, &N1, &I1, &m5lobj_.iobj,
			&objadj, A, ha, ja,
			lb, ub, 0, 0,
			hs, X0, pi, rc,
			&INFORM, &NS, z, &mincor, (ftnlen)4);
		if ((i = INFORM) < 0 || i > 13)
			i = INFORM >= 20 && INFORM <= 22 ? INFORM + (14-20)
				: INFORM == 30 ? 17 : 18;
		}
	tout = xectim_();
	tsolve = tout - t;
	if (need_nl && timing & 1) {
		printf("\n");
		need_nl = 0;
		}
	msg1 = msg + Sprintf(msg, wantver ? xxxvers+6 : "MINOS 5.5: ");
	switch(i) {
		case 1:
			if (!nlc)
				i = 21;
			goto have_i;
		case 3:
			if (m5lp1_.itn < m5lp1_.itnlim)
				i = 20;
			goto have_i;
		case 4:
			nint = 2*NB;
			if (nint < 200)
				nint = 200;
			else if (nint > 1000)
				nint = 1000;
			msg1 += Sprintf(msg1, solinfo[4].msg, (long)nint);
			break;
		case 5:
			msg1 += Sprintf(msg1, solinfo[5].msg,
					(long)m5len_.maxs);
			break;
		case 14:	/* imitate computation of minlen in m2bfac */
			INFORM = m2lu1_.nbelem*5/4;
			if (INFORM <= m2lu1_.lena)
				INFORM = m2lu1_.lena;
			else if (INFORM < m2lu4_.luparm[12])
				INFORM = m2lu4_.luparm[12];
			INFORM = m2mapz_.maxz + 3*(INFORM - m2lu1_.lena);
			/* no break */
		default:
 have_i:
			msg1 += Sprintf(msg1, solinfo[i].msg, (long)INFORM);
		}
	msg1 += Sprintf(msg1, ".\n%ld iterations", (long)m5lp1_.itn);
	if (solinfo[i].wantobj) {
		g_fmtop(buf, m5lobj_.iobj ? cycle2_.objtru : objadj);
		msg1 += Sprintf(msg1, ", objective %s", buf);
		}
	solve_result_num = solinfo[i].code;
	if (m8func_.nfcon[0] + m8func_.nfobj[0] > 0 && !functimes) {
		msg1 += Sprintf(msg1, "\nNonlin evals: ");
		if (m8func_.nfobj[0] > 0)
			msg1 += Sprintf(msg1, "obj = %ld, grad = %ld%s",
				(long)m8func_.nfobj[0],
				(long)m8func_.nfobj[1],
				m8func_.nfcon[0] > 0 ? ", " : "");
		if (m8func_.nfcon[0] > 0)
			msg1 += Sprintf(msg1, "constrs = %ld, Jac = %ld",
				(long)m8func_.nfcon[0],
				(long)m8func_.nfcon[1]);
		Sprintf(msg1, ".");
		}
	send_status(N, hs, varstat, svtrans, LUv, Uvx);
	send_status(M, hs+N, varstat+N, sctrans, LUrhs, Urhsx);
	write_sol(msg, X0, pi, &Oinfo);
	if (timing | functimes) {
		t = xectim_();
		m1tim_.tsum[2] = t - tout;
		t -= tstart;
		if (!timing || timing & 1)
			time_out(t, stdout);
		if (timing & 2) {
			fflush(stdout);
			time_out(t, Stderr);
			}
		}
	ASL_free(&asl);	/* for Purify */
	}

#ifndef Student_Edition

 void
#ifdef KR_headers
minos2_(z, nwcore, mincor, inform) real *z; fint *nwcore, *mincor, *inform;
#else
minos2_(real *z, fint *nwcore, fint *mincor, fint *inform)
#endif
{
	static fint I2 = 2;
	ASL *asl = cur_ASL;
	Not_Used(nwcore);
	if (m2mapz_.maxz < *mincor)
		m2mapz_.maxz = *mincor;
	z = (real *)M1alloc(m2mapz_.maxz*sizeof(real));
	m3dflt_(&I2);
	m1tim_.ltime = 1;
	minos3_(z, mincor, inform);
	fflush(stdout);
	fprintf(Stderr, "Solution time = %.2f seconds.", m1tim_.tsum[1]);
	fprintf(Stderr, *inform ? "  Exit code %ld.\n" : "\n", (long)*inform);
	fflush(Stderr);
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
		fprintf(Stderr, "gfname called with i = %d\n", i);
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
