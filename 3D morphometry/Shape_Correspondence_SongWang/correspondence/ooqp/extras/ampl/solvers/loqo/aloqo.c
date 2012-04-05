/****************************************************************
Copyright (C) 1997-2000 Lucent Technologies
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

#ifdef __cplusplus
extern "C" {
#endif
#include "loqo.h"
#include "myalloc.h"
#include "choles.h"
#ifdef __cplusplus
	}
#endif

#define STDIO_H_included
#include "asl_pfgh.h"
#include "getstub.h"

 static void nlclose ANSI((void*));
 static void nlinit ANSI((void*));
 static int  nlupdate ANSI((void*));
 static int  nlupdate_fg ANSI((void*));
 static int  nlupdate_h  ANSI((void*));

#ifdef __cplusplus
extern "C" {
#endif

 extern char loqo_bsname[], loqo_version[];

 struct options
{
	double inftol;
	int maxflag;
	int sf_req;
	int v;
	int objno;

	int itnlim;
	double timlim;
	double inftol2;
	double mufactor;
	int pred_corr;
	double steplen;
	double epssol;
	double epsnum;
	double epsdiag;
	double stablty;
	int method;
	int dense;
	int pdf;
	double bndpush;
	int honor_bnds;
	int honor_bnds_init;
	int lp_only;
	int ignore_initsol;
	int zero_initsol;
	int convex;
	int sdp;

	/* names for MPS format */
	char *bounds;
	char *obj;
	char *ranges;
	char *rhs;
	};

 static struct options opns = {1e-6, 0, 8, 0, 1,
	500, 0. /*HUGE_VAL*/, 1.0e+5, -1.0, -1, 0.95, _EPSSOL, _EPSNUM,
	_EPSDIAG, _STABLTY, _MD, _DENSE, _UNSET,
	-1.0, /* bndpush */
	-1, 	/* honor_bnds */
	0, 	/* honor_bnds_init */
	0,	/* lp_only */
	0, 	/* ignore_initsol */
	0,	/* zero_initsol */
	0,	/* convex */
	0	/* sdp */
	};

 static int neval, nheval, time_flag, wantmps, wantver;
 static real ftimes[5];	/* func, grad, constr, Jacobians, Hessians */

 static I_Known maximize = { -1, &opns.maxflag };
 static I_Known primal = { _PRIMAL, &opns.pdf };
 static I_Known dual =	 { _DUAL, &opns.pdf };
 static I_Known mindeg = { _MD, &opns.method };
 static I_Known minlocfil = { _MLF, &opns.method };
 static I_Known noreord = { _NOREORD, &opns.method };

 static keyword
keywds[] = {	/* must be alphabetical */
 KW("bndpush",	D_val,	&opns.bndpush,	"initial distance from bounds"),
 KW("bounds",	C_val,	&opns.bounds,	"name of MPS BOUNDS section"),
 KW("convex",	IK1_val,&opns.convex,	"assert problem is convex"),
 KW("dense",	I_val,	&opns.dense,	"dense window threshold"),
 KW("dual",	IK_val,	&dual,		"dual-favored matrix reordering"),
 KW("epsdiag",	D_val,	&opns.epsdiag,	"eps tol diag perturbation"),
 KW("epsnum",	D_val,	&opns.epsnum,	"eps tol for numerical factorization"),
 KW("epssol",	D_val,	&opns.epssol,	"eps tol for forward/backward solve"),
 KW("honor_bnds",I_val, &opns.honor_bnds, "honor bounds on variables"),
 KW("honor_bnds_init",	I_val,&opns.honor_bnds_init,"honor bnds initially"),
 KW("ignore_initsol", IK1_val, &opns.ignore_initsol, "ignore given initial guesses; make automatic choice"),
 KW("inftol",	D_val,	&opns.inftol,	"Feasibility tolerance"),
 KW("inftol2",	D_val,	&opns.inftol2,	"Feasibility tolerance 2"),
 KW("iterlim",	I_val,	&opns.itnlim,	"Iteration Limit (ITERLIM)"),
 KW("lp_only",	IK1_val,&opns.lp_only,	"linearize only---don't quadraticize"),
 KW("max",	IK_val,	&maximize,	"maximize the objective"),
 KW("maximize",	IK_val,	&maximize,	"maximize the objective"),
 KW("maxit",	I_val,	&opns.itnlim,	"Iteration Limit (ITERLIM)"),
 KW("min",	IK1_val,&opns.maxflag,	"minimize the objective"),
 KW("mindeg",	IK_val,	&mindeg,	"minimum degree matrix reordering"),
 KW("minimize",	IK1_val,&opns.maxflag,	"minimize the objective"),
 KW("minlocfil",IK_val,	&minlocfil,	"minimum local fill matrix reordering"),
 KW("mps",	IK1_val,&wantmps,	"read an MPS file, stub.mps or stdin"),
 KW("mufactor",	D_val,	&opns.mufactor,	"mu factor"),
 KW("noreord",IK_val,	&noreord,	"no matrix reordering"),
 KW("obj",	C_val,	&opns.obj,	"name of MPS objective row"),
 KW("objno",	I_val,	&opns.objno,	"objective number: 1= first, 0= none"),
 KW("outlev",	I_val,	&opns.v,	"output level"),
 KW("pred_corr",I_val,	&opns.pred_corr,"pred= 0, pred_corr= 1, unset= -1"),
 KW("primal",	IK_val,	&primal,	"primal-favored matrix reordering"),
 KW("ranges",	C_val,	&opns.ranges,	"name of MPS RANGES section"),
 KW("rhs",	C_val,	&opns.rhs,	"name of MPS right-hand side section"),
 KW("sdp",	IK1_val,&opns.sdp,	"assert problem is an SDP"),
 KW("sigfig",	I_val,	&opns.sf_req,	"significant digits (agreement in primal and dual objectives)"),
 KW("stablty",	D_val,	&opns.stablty,	"mixing factor for matrix reordering"),
 KW("steplen",	D_val,	&opns.steplen,	"step length reduction factor"),
 KW("timing",	I_val,	&time_flag,	"timing destination: 1 = stdout, 2 = stderr, 3 = both"),
 KW("timlim",	D_val,	&opns.timlim,	"Time limit"),
 KW("verbose",	I_val,	&opns.v,	"synonym for outlev"),
 KW("version",	I_val,	&wantver,	"report version"),
 KW("wantsol",	WS_val, 0,		WS_desc_ASL+5),
 KW("zero_initsol", IK1_val, &opns.zero_initsol, "start at the origin")
 };

 static keyword
mflag[1] = KW("m", IK1_val, &wantmps, "MPS input: read stub.spc and stub.mps");

 static char *usage_msg[] = {
	"  where  stub  is from  \"ampl -obstub\"  or  \"ampl -ogstub\".",
	"  -m with no stub ==> read an MPS file on stdin.",
	0 };

 static Option_Info Oinfo = {
	"loqo", loqo_bsname, "loqo_options", keywds, nkeywds, 1,
	loqo_version, usage_msg, 0, 0, mflag, 1 };

#undef INTEGER
#undef DOUBLE
#define INTEGER(x) (int *)Malloc((x)*Sizeof(int))
#define DOUBLE(x) (double *)Malloc((x)*Sizeof(double))

 static int
qtest(char *s) {
	ASL *asl;
	int flag, i;
	fint *colqp, *rowqp;
	real *delsqp;
	FILE *nl;

	asl = ASL_alloc(ASL_read_fg);
	nl = jac0dim(s, (long)strlen(s));
	flag = nlc == 0;
	if (nlo == 0 || flag == 0) {
		fclose(nl);
		goto done;
		}

	qp_read(nl,0);
	if ((i = opns.objno - 1) >= 0 && i < nlo
	 && nqpcheck(i, &rowqp, &colqp, &delsqp) < 0)
		flag = 0;
done:
	ASL_free(&asl);
	return flag;
	}

 int
#ifdef KR_headers
amplin(asl, argv, stub, kp) ASL *asl; char **argv, *stub; LOQO *kp;
#else
amplin(ASL *asl, char **argv, char *stub, LOQO *kp)
#endif
{
	fint M, N, NO, NZ, MXROW, MXCOL;
	int *ia, *ka, *kat, *varsgn;
	double *a, *a1, *ae, *b, *b1, *c, *l, *r, *r1, *u;
	double sgn;
	cgrad *cg, **cgx;
	ograd *og;
	int i, j, n;
	FILE *nl;

	if (!stub)
		usage_ASL(&Oinfo, 1);

	if (qtest(stub)) {
		kp->quadratic = 1;
		if (opns.v) {
			printf("It's a QP.\n");
			need_nl = 0;
			}
		}

	/* cur_ASL is used implicitly by nlinit and the nlupdate */
	/* routines.  It was changed by qtest.  */
	cur_ASL = asl;
	want_xpi0 = 3;
	nl = jacdim(stub, &M, &N, &NO, &NZ, &MXROW, &MXCOL, (ftnlen)strlen(stub));
	LUv = DOUBLE(N);
	Uvx = DOUBLE(N);
	LUrhs = DOUBLE(M);
	Urhsx = DOUBLE(M);
	pfgh_read(nl, ASL_findgroups | ASL_find_co_class);

	kp->c = c = DOUBLE(N);
	memset(c, 0, N*sizeof(double));
	kp->f = 0;
	if ((i = opns.objno - 1) >= 0 && i < n_obj) {
		obj_no = i;
		for(og = Ograd[i]; og; og = og->next)
			c[og->varno] = og->coef;
		if (!opns.maxflag)
			opns.maxflag = objtype[i] ? -1 : 1;
		kp->f = objconst(i);
		hesset(1,i,1,0,nlc);
		}
	else  {
		obj_no = -1;
		if (!opns.maxflag)
			opns.maxflag = -1;
		hesset(1,0,0,0,nlc);
		}

	kp->A = a = DOUBLE(NZ);
	kp->m = M;
	kp->n = N;
	kp->nz = NZ;
	kp->b = b = LUrhs;
	kp->r = r = Urhsx;
	kp->l = l = LUv;
	kp->u = u = Uvx;
	kp->kA = ka = INTEGER(N+1);
	ka[N] = NZ;
	for(i = N; --i >= 0; )
		ka[i] = -1;
	kp->iA = ia = INTEGER(NZ);
	kp->varsgn = varsgn = INTEGER(N);
	kp->kAt = INTEGER(M+1);
	kat = kp->kAt + M;
	j = *kat = NZ;
	i = (int)M;
	cgx = Cgrad + i;
	b1 = b + i;
	r1 = r + i;
	while(--i >= 0) {
		--r1;
		--cgx;
		--b1;
		sgn = 1.;
		if (*b1 <= negInfinity)
			conscale(i, sgn = -1., 0);	/* changes *b1, *r1 */
		if (*r1 >= Infinity)
			*r1 = HUGE_VAL;
		else if (*b1 <= negInfinity) {
			fprintf(Stderr, "Row %d is free!\n", i);
			exit(1);
			}
		else
			*r1 -= *b1;
		for(cg = *cgx; cg; --j, cg = cg->next) {
			ia[ka[cg->varno] = cg->goff] = i;
			a[cg->goff] = sgn*cg->coef;
			}
		*--kat = j;
		}
	n = (int)N;
	for(i = n; --i >= 0; )
		if (ka[i] == -1)
			ka[i] = ka[i+1];
	for(i = 0; i < n; i++) {
		varsgn[i] = 1;
		if (l[i] <= negInfinity && u[i] < Infinity) {
			varscale(i, -1., 0);
			c[i] = -c[i];
			a1 = a + ka[i];
			ae = a + ka[i+1];
			for(; a1 < ae; a1++)
				*a1 = -*a1;
			}
		if (l[i] <= negInfinity)
			l[i] = -HUGE_VAL;
		if (u[i] >= Infinity)
			u[i] = HUGE_VAL;
		}
	if (i = nlogv + niv + nlvbi + nlvci + nlvoi) {
		printf("ignoring integrality of %d variables\n", i);
		need_nl = 0;
		}

	if (!opns.ignore_initsol) {
		if (pi0)
			kp->y = pi0;
		if (opns.zero_initsol)
			CALLOC( kp->x, N, double ) /* macro gives { ... } */
		else if (X0)
			kp->x = X0;
		}

	lagscale(-opns.maxflag,0);

	if (kp->verbose >= 3) {

		/* To assign names from the AMPL model to these labels, put
		 *     "option loqo_auxfiles rc;"
		 * before the solve command or specify
		 *	"option auxfiles rc;"
		 * before "write gfoo;" when writing file goo.nl. */

		MALLOC(kp->collab, n, char *);
		for(i = 0; i < n; i++)
			kp->collab[i] = my_strdup(var_name(i));

		MALLOC(kp->rowlab, M, char *);
		for(i = 0; i < M; i++)
			kp->rowlab[i] = my_strdup(con_name(i));
		}

	return 0;
	}

 void
#ifdef KR_headers
amplout(asl, kp, status) ASL *asl; LOQO *kp; int status;
#else
amplout(ASL *asl, LOQO *kp, int status)
#endif
{
	char buf[32], hbuf[256];
	int i;
	double *y, *ye;
	typedef struct { char *msg; int code; } Sol_info;
	static Sol_info solinfo[] = {
		{ /* 0 */ "optimal solution", 0 },
		{ /* 1 */ "suboptimal solution", 100 },
		{ /* 2 */ "iteration limit", 400 },
		{ /* 3 */ "primal infeasible", 201 },
		{ /* 4 */ "dual infeasible", 202 },
		{ /* 5 */ "primal and/or dual infeasible", 203 },
		{ /* 6 */ "primal infeasible -- inconsistent equations", 210 },
		{ /* 7 */ "primal unbounded", 301 },
		{ /* 8 */ "dual unbounded", 302 },
		{ /* 9 */ "resource limit", 500 },
		{ "??? LOQO bug", 510 }
		};

	if (status < 0 || status > 9)
		status = 9;
	solve_result_num = solinfo[status].code;
	if (wantver)
		i = Sprintf(hbuf, "LOQO %s, ASL(%ld):\n", loqo_version,
			ASLdate_ASL);
	else
		i = Sprintf(hbuf, "%s: ", loqo_bsname);
	i += Sprintf(hbuf+i, "%s (%d %siterations, %d evaluations)",
			solinfo[status].msg, kp->iter,
			kp->quadratic ? "QP " : "", neval);
	if (kp->max < 0) {
		for(y = kp->y, ye = y + kp->m; y < ye; y++)
			*y = -*y;
		}
	if (status < 3) {
		g_fmtop(buf, kp->primal_obj);
		i += Sprintf(hbuf+i, "\nprimal objective %s", buf);
		g_fmtop(buf, kp->dual_obj);
		i += Sprintf(hbuf+i, "\n  dual objective %s", buf);
		}
	write_sol(hbuf, kp->x, kp->y, &Oinfo);
	}

 static double Times[4];

 static void
show_times(VOID)
{
	int i, j, nn[5];
	FILE *f;
	real t;
	static char *what[5] = { "function", "gradient", "constraint",
				 "Jacobian", "Hessian" };

	Times[3] = xectim_();
	for(i = 1; i <= 2; i++)
	    if (time_flag & i) {
		f = i == 1 ? stdout : stderr;
		fprintf(f,
		"\nTimes (seconds):\nInput =  %g\nSolve =  %g\nOutput = %g\n",
			Times[1] - Times[0], Times[2] - Times[1],
			Times[3] - Times[2]);

		fprintf(f, "\nnn nonlinear evaluations = t seconds:\n");
		nn[0] = nn[1] = nn[2] = nn[3] = neval;
		nn[4] = nheval;
		t = 0;
		for(j = 0; j < 5; j++) {
			fprintf(f, "\t%4d %10ss = %g\n",
				nn[j], what[j], ftimes[j]);
			t += ftimes[j];
			}
		fprintf(f, "Total nonlinear evaluation seconds = %g\n", t);
		}
	}

 static void
#ifdef KR_headers
namecpy(s, t) char *s; char *t;
#else
namecpy(char *s, char *t)
#endif
{
	if (t) {
		strncpy(s,t,10);
		s[10] = 0;
		}
	else
		*s = 0;
	}

 void
#ifdef KR_headers
update_kp(kp) LOQO *kp;
#else
update_kp(LOQO *kp)
#endif
{
	kp->inftol = opns.inftol;
	kp->sf_req = opns.sf_req;
	kp->verbose = opns.v;
	kp->max = opns.maxflag ? opns.maxflag : 1;
	kp->itnlim = opns.itnlim;
	kp->timlim = opns.timlim;
	kp->inftol2 = opns.inftol2;
	kp->mufactor = opns.mufactor;
	kp->pred_corr = opns.pred_corr;
	kp->steplen = opns.steplen;
	kp->epssol = opns.epssol;
	kp->epsnum = opns.epsnum;
	kp->epsdiag = opns.epsdiag;
	kp->stablty = opns.stablty;
	kp->method = opns.method;
	kp->dense = opns.dense;
	kp->pdf = opns.pdf;
	kp->bndpush = opns.bndpush;
	kp->honor_bnds = opns.honor_bnds;
	kp->honor_bnds_init = opns.honor_bnds_init;
	kp->convex = opns.convex;
	kp->sdp = opns.sdp;
	namecpy(kp->obj, opns.obj);
	namecpy(kp->rhs, opns.rhs);
	namecpy(kp->ranges, opns.ranges);
	namecpy(kp->bounds, opns.bounds);
	}

#ifdef __cplusplus
}
#endif

 int
#ifdef KR_headers
main(argc, argv) char **argv;
#else
main(int argc, char **argv)
#endif
{
	ASL	*asl;
	int	namelen, status;
	char	*av[2], **av0, *stub;
	char	fname[128];	/* solution file name */
	LOQO	*kp;
	FILE 	*f;

	asl = ASL_alloc(ASL_read_pfgh);
	kp = openlp();
	if (kp == NULL) {
		fprintf(stderr,"Bug: openlp failure!\n");
		return 1;
		}

	Times[0] = xectim_();
	opns.timlim = HUGE_VAL;
	av0 = argv;
	stub = getstub(&argv, &Oinfo);
	if (wantver) {
		printf("LOQO %s\n", loqo_version);
		need_nl = 0;
		}
	if (getopts(argv, &Oinfo))
		return 1;
	if (!wantmps) {
		if (amplin(asl,av0,stub,kp))
			return 1;
		update_kp(kp);
		kp->h_init = nlinit;
		kp->h_update = nlupdate;
		kp->h_update_fg = nlupdate_fg;
		kp->h_update_h = nlupdate_h;
		kp->h_close = nlclose;
		Times[1] = xectim_();
		status = solvelp(kp);
		/*
		{
		char stub_mps[100];
		sprintf(stub_mps, "%s.mps", stub);
		sprintf(kp->name,"%s", stub);
		printf("%s \n", stub_mps);
		writelp(kp,stub_mps);
		}
		*/
		Times[2] = xectim_();
		amplout(asl, kp, status);
		show_times();
		return 0;
		}
	if (stub) {
		argc = 0;
		namelen = strlen(stub);
		av[0] = (char*)Malloc(namelen+5);
		strcpy(av[0], stub);
		strcpy(av[0] + namelen, ".spc");
		if (f = fopen(av[0],"r")) {
			fclose(f);
			argc = 1;
			av[1] = (char *) Malloc(namelen+5);
			}
		strcpy(av[argc], stub);
		strcpy(av[argc++]+namelen, ".mps");
		}
	else {
		av[0] = "-";
		argc = 1;
		}

	printf("%s\n", loqo_version);
	fflush(stdout);

	update_kp(kp);
	readlp(argc,av,kp);
	Times[1] = xectim_();

	if (kp->qnz == 0) {
		printf("m=%d, n=%d, nz=%d \n", kp->m, kp->n, kp->nz);
		/* if (prep(kp)==0) { */
			status = solvelp(kp);
			/* unprep(kp);
		} else {
			status = 3;
		}
		*/
	} else {
		status = solvelp(kp);
	}
	Times[2] = xectim_();

	strncpy(fname, kp->name, sizeof(fname)-5);
	strcat(fname, ".out");
	writesol(kp,fname);

	show_times();
	return 0;
	}

#define asl cur_ASL

static int *
#ifdef KR_headers
ficopy(n, f) int n; fint *f;
#else
ficopy(int n, fint *f)
#endif
{
	int *i, *ie, *rv;

	if (sizeof(int) == sizeof(fint))
		rv = (int*)f;
	else {	/* a good compiler will eliminate this code in most cases... */
		i = rv = (int*)Malloc(n*sizeof(int));
		ie = i + n;
		while(i < ie)
			*i++ = *f++;
		}
	return rv;
	}

 static void
#ifdef KR_headers
nlinit(vlp) Char *vlp;
#else
nlinit(Char *vlp)
#endif
{
	LOQO *lp;
	int i, m, n;
	fint nz;

	lp = (LOQO*)vlp;
	m = lp->m;
	n = lp->n;

	if (!opns.lp_only) {
	    nz = sphsetup(obj_no,0,m>0,0);
	    lp->kQ = ficopy(n+1, sputinfo->hcolstarts);
	    lp->iQ = ficopy((int)nz, sputinfo->hrownos);
	    lp->qnz = sputinfo->hcolstarts[n];
	    REALLOC( lp->Q, lp->qnz, double );
	    for (i=0; i<lp->qnz; i++) {lp->Q[i] = 0.0;}
	}

	REALLOC(  lp->R,   m, double );
	REALLOC( lp->b0,   m, double );

	for (i=0; i<m; i++) lp->b0[i] = lp->b[i];

	if (lp->quadratic) {
	    if (lp->convex || lp->qnz==0) {
		if (lp->bndpush < 0) { lp->bndpush = 100.0; }
		lp->honor_bnds = FALSE;
		lp->pred_corr = TRUE;
		lp->mufactor = 0.0;
		lp->convex = TRUE;
	    }
	}
}

 static void
#ifdef KR_headers
nlclose(lp) Char *lp;
#else
nlclose(Char *lp)
#endif
{ Not_Used(lp); }

 static int
#ifdef KR_headers
nlupdate(vlp) Char *vlp;
#else
nlupdate(Char *vlp)
#endif
{
	LOQO *lp;
	cgrad *cg;
	int i, j, k, n;
	int *iAt, *kAt, *iQ, *kQ;
	double f, val;
	double *c, *x, *y, *A, *At, *J, *Q;
	fint nerror = 0;
	real ft[6];
	static int *iwork=NULL;
	static double *Hf_a=NULL;

	neval++;
	lp = (LOQO*)vlp;
	n = lp->n;
	iAt = lp->iAt;
	kAt = lp->kAt;
	iQ = lp->iQ;
	kQ = lp->kQ;
	c = lp->c;
	x = lp->x;
	y = lp->y;
	if (!lp->m)
		y = 0;
	A = lp->A;
	At = lp->At;
	Q = lp->Q;

	/*----------------------------------------------------+
	| Nonlinear constraint stuff			     */

	ft[0] = xectim_();
	n_conjac[1] = nlc;
	conval(x, lp->R, &nerror);
	ft[1] = xectim_();
	if (nerror) return 1;
	jacval(x, A, 0);
	for (J = At, i=0; i<nlc; i++) {
		for(cg = Cgrad[i]; cg; cg = cg->next)
			*J++ = A[cg->goff];

		val = lp->R[i];
		for (k=kAt[i]; k<kAt[i+1]; k++) {
		    val -= At[k] * x[ iAt[k] ];
		}
		lp->b[i] = lp->b0[i] - val;
	}
	ft[2] = ft[3] = ft[4] = xectim_();

	/*----------------------------------------------------+
	| Nonlinear objective stuff			     */

	REALLOC( iwork, n+1, int );
	REALLOC( Hf_a, n, double );

	if (obj_no >= 0) {
		f = objval(obj_no, x, &nerror);
		lp->primal_obj = f;
		/**/
		if (nerror)
			return 1;
			/**/
		ft[3] = ft[4] = xectim_();
		if (lp->qnz > 0) {
			objgrd(obj_no, x, c, 0);
			ft[4] = xectim_();
			}
		}
	else
		f = 0;

	ft[5] = ft[4];
	if (!opns.lp_only && lp->qnz>0) {
		nheval++;
		sphes(Q, obj_no, 0, y);
		ft[5] = xectim_();
		}

	/*----------------------------------------------------+
	| adjust f <- f(a) - a^T grad f(a) + a^T Hess f(a) a
	|	 c <- grad(a) - Hess f(a) a 		     */

	if (!opns.lp_only && lp->qnz>0) {
	    smx(n,n,Q,kQ,iQ,x,Hf_a);

	    f += - dotprod(c,x,n) + dotprod(x,Hf_a,n)/2;
	    lp->f = f;

	    for (j=0; j<n; j++) c[j] -= Hf_a[j];
	} else {
	    f += - dotprod(c,x,n);
	    lp->f = f;
	}
	ftimes[0] += ft[3] - ft[2];
	ftimes[1] += ft[4] - ft[3];
	ftimes[2] += ft[1] - ft[0];
	ftimes[3] += ft[2] - ft[1];
	ftimes[4] += ft[5] - ft[4];
	return 0;
}

 static int
#ifdef KR_headers
nlupdate_fg(vlp) Char *vlp;
#else
nlupdate_fg(Char *vlp)
#endif
{
	LOQO *lp;
	int m,n;
	double f;
	double *c, *x;
	real ft[4];
	fint nerror = 0;
	static int *iwork=NULL;

	neval++;
	lp = (LOQO*)vlp;
	m = lp->m;
	n = lp->n;
	c = lp->c;
	x = lp->x;

	/*----------------------------------------------------+
	| Nonlinear constraint stuff			     */

	ft[0] = xectim_();
	n_conjac[1] = m;
	conval(x, lp->R, &nerror);
	ft[1] = xectim_();
	if (nerror) return 1;

	/*----------------------------------------------------+
	| Nonlinear objective stuff			     */

	REALLOC( iwork, n+1, int );

	if (obj_no >= 0) {
		f = objval(obj_no, x, &nerror);
		lp->primal_obj = f;
		ftimes[0] += (ft[2] = xectim_()) - ft[1];
		/**/
		if (nerror)
			return 1;
			/**/
		if (lp->qnz>0) {
			objgrd(obj_no, x, c, 0);
			ft[3] = xectim_();
			ftimes[1] += ft[3] - ft[2];
			}
		}
	else
		f = 0;

	/*----------------------------------------------------+
	| adjust f <- f(a) - a^T grad f(a)                   */

	f += - dotprod(c,x,n);
	lp->f = f;

	return 0;
}

 static int
#ifdef KR_headers
nlupdate_h(vlp) Char *vlp;
#else
nlupdate_h(Char *vlp)
#endif
{
	LOQO *lp;
	int j, n;
	int *iQ, *kQ;
	double f, fx;
	double *c, *x, *y, *Q;
	static double *Hf_a=NULL;

	lp = (LOQO*)vlp;
	n = lp->n;
	iQ = lp->iQ;
	kQ = lp->kQ;
	c = lp->c;
	x = lp->x;
	y = lp->y;
	if (!lp->m)
		y = 0;
	Q = lp->Q;

	if (!opns.lp_only) {
		nheval++;
		f = xectim_();
		sphes(Q, obj_no, 0, y);
		ftimes[4] += f - xectim_();
		}

	REALLOC( Hf_a, n, double );
	fx = lp->primal_obj;

	/*----------------------------------------------------+
	| adjust f <- f(a) - a^T grad f(a) + a^T Hess f(a) a
	|	 c <- grad(a) - Hess f(a) a 		     */

	if (!opns.lp_only && lp->qnz>0) {
	    smx(n,n,Q,kQ,iQ,x,Hf_a);

	    f = fx - dotprod(c,x,n) + dotprod(x,Hf_a,n)/2;
	    lp->f = f;

	    for (j=0; j<n; j++) c[j] -= Hf_a[j];
	} else {
	    f = fx - dotprod(c,x,n);
	    lp->f = f;
	}
	return 0;
}
