#include "Types.h"
#include "Path.h"
#include "Options.h"
#undef Char
#include "getstub.h"

#define CRIPPLE_SIZE 200
#define CRIPPLE_NNZ 10000

extern Void getOption(Char *buffer, Int line);
extern Void Output_SetStatus(FILE *);
extern Void Output_SetLog(FILE *);
extern char *Path_Version(void);
extern char xxxvers[];
static int  debug, functimes, quitnow, wantfuncs, wantver;
static char *logfile, *statusfile, *optfile;
#ifdef MDEBUG /* debugging malloc */
extern int zapvalue;
#endif

 typedef struct
FuncInfo {
	real	*c_scratch;
	char	*ctype;
	char	*vtype;
	int	*vb;	/* variables with bounds */
	int	n;	/* adjusted problem size */
	int	nvb;	/* number of bounded variables */
	int	nvr;	/* number of ranged variables */
	int	nnz;	/* adjusted number of Jacobian nonzeros */
		} FuncInfo;

 static FuncInfo FI;

 static int numJacEvals = 0;

 static fint
mkey(char *phrase, ftnlen len)
{
	static Int line;
	Not_Used(len);

	getOption(phrase, ++line);
	return 0;
	}

 static char *
Optfile(Option_Info *oi, keyword *kw, char *value)
{
	char *rv = C_val(oi, kw, value);
	if (optfile)
		Options_Read(optfile);
	return rv;
	}

 static char
debug_desc[] = "debug level (default 0): sum of\n\
		1 = show initial z and bounds\n\
		2 = show z and f(z)\n\
		4 = show z and J = f'(z)\n\
		8 = show sparsity of J";

 static keyword
keywds[] = {
	KW("debug", I_val, &debug, debug_desc),
	KW("logfile", C_val, &logfile, "name of log file"),
	KW("maxfwd",  IA_val, voffset_of(ASL, p.maxfwd_), "max vars in fwd AD of common exprs (default 5)"),
	KW("optfile", Optfile, &optfile, "name of options file"),
	KW("statusfile", C_val, &statusfile, "name of status file"),
	KW("version", I_val, &wantver, "report version"),
	KW("wantsol", WS_val, 0, WS_desc_ASL + 5)
#ifdef MDEBUG
	,KW("zapvalue", I_val, &zapvalue, "zapvalue (in debugging malloc)")
#endif
	};

 static
keyword cloptions[] = {
	KW("f", IK1_val, &wantfuncs, "list available user-defined functions"),
	KW("q", IK1_val, &quitnow,   "quit now"),
	KW("t", IK1_val, &functimes, "time function evaluations")
	};

 static
Option_Info Oinfo = {
	"path", 0, "path_options", keywds, nkeywds, 3, 0,
	0, mkey, 0, cloptions, sizeof(cloptions) / sizeof(keyword)
	};

 static void
show_ver(FILE *f)
{
	fprintf(f, "%s", Oinfo.version);
	fflush(f);
	}

enum {
	C_COMPL = 0,
	C_LB = 1,
	C_UB = 2,
	C_RANGE = 3,
	C_EQN = 4,
	C_FREE = 5
	};

 static void
match_bug(void)
{
	fprintf(Stderr, "Unexpected failure in mcp_adj.\n");
	exit(1);
	}

 static int
mcp_adj(ASL *asl)
{
	int *Cvar, *z, *zc, *zs;
	int ct, i, j, k, *ka, L, nc, nv, nvb, nvr;
	real *lu;
	cgrad *cg;
	size_t st, st1;

	/* Match variables to equations and inequalities, adding one
	 * new variable for each simple inequality, including simple
	 * variable bounds, and two new variables for ranges:
	 * range constraints, -Infinity < L <= f(x) <= U < Infinity,
	 * with L < U, are represented by
	 *	f(x) = y complements v
	 *	v	 complements L <= y <= U
	 * in which v and y are new variables.  Ranged variables are
	 * handled similarly, with f(x) := x[i] for variable i if it
	 * has distinct finite lower and upper bounds.   This adds
	 * a new nonzero to column i of the Jacobian matrix.
	 */

	nc = n_con;
	nv = n_var;
	if (!(Cvar = cvar))
		Cvar = cvar = (int * )M1zapalloc(nc * sizeof(int));
	st = nc * sizeof(real);
	st1 = (2 * nc + nv) * sizeof(int);
	if (st < st1)
		st = st1;
	FI.c_scratch = (real *)M1zapalloc(st);
	z = (int *)FI.c_scratch;
	zc = z + nv;
	zs = zc + nc;
	for (i = 0; i < nc; )
		if (j = Cvar[i++])
			z[j-1] = i;

	lu = LUv;

	/* nvb := number of variables with one or two finite bounds. */
	/* nvr := number of ranged variables: two finite bounds */

	for (i = nvb = nvr = 0; i < nv; i++, lu += 2)
		if (!z[i]) {
			if (lu[0] > negInfinity) {
				nvb++;
				if (lu[1] < Infinity)
					nvr++;
				}
			else if (lu[1] < Infinity)
				nvb++;
			}
	FI.nvb = nvb;
	FI.nvr = nvr;
	FI.vb = (int *)M1zapalloc((sizeof(int)+1)*nvb + nc);
	FI.vtype = (char * )(FI.vb + nvb);
	FI.ctype = FI.vtype + nvb;
	if (nvb) {
		lu = LUv;
		for (i = k = 0; i < nv; i++, lu += 2)
			if (!z[i]) {
				ct = 0;
				if (lu[0] > negInfinity)
					ct = lu[1] < Infinity ? C_RANGE : C_LB;
				else if (lu[1] < Infinity)
					ct = C_UB;
				else
					continue;
				FI.vtype[k] = ct;
				FI.vb[k++] = i;
				}
		}

	/* match equations */

	j = nv;
	for (i = k = 0; i < nc; i++) {
		if (!Cvar[i]) {
			lu = LUrhs + (i << 1);
			if (lu[0] < lu[1]) {
				Cvar[i] = ++j;

				/* For range constraints, */
				/* increment j twice... */

				if (lu[0] > negInfinity)
					FI.ctype[i] = lu[1] < Infinity
					     ? (j++,C_RANGE) : C_LB;
				else
					FI.ctype[i] = lu[1] < Infinity
					     ? C_UB : C_FREE;
				}
			else {
				FI.ctype[i] = C_EQN;
				while(z[k])
					k++;
				z[k++] = i + 1;
				Cvar[i] = k;
				}
			}
		}

	/* Prepare zc = permutation of constraints. */

	for (i = 0; i < nc; i++)
		zs[i] = zc[i] = -1;
	for (i = L = 0; i < nc; i++)
		if ((j = --Cvar[i]) < nv) {
			zc[j] = i;
			zs[i] = 0;
			L++;
			}
	if (L < nc) {
		if (L) {
			for (i = j = 0; i < nc; i++) {
				k = zs[i];
				zs[i] = j;
				if (k < 0)
					j++;
				}
			for (i = 0; i < nc; i++)
				if ((j = zc[i]) >= 0)
					zc[i] -= zs[j];
			}

		/* Now for S := {i in 0..nc-1: Cvar[i] < nv}, */
		/* setof{i in S} zc[i] == 0 .. card(S)-1. */
		/* Fill in zc to make a permutation of 0 .. nc-1. */

		for (i = j = k = nc; i-- > 0; )
			if (Cvar[i] >= nv) {
				while (zc[--j] >= 0);
				zc[j] = --k;
				}
		}

	/* Adjust A_colstarts to insert new singleton Jacobian rows */
	/* for ranged variables. */

	ka = A_colstarts + 1;
	j = k = 0;
	L = FI.nvb ? FI.vb[0] : nv;
	for (i = 0; i < nv; i++) {
		z[i] = ka[i] + j - 1;
		if (i == L) {
			j++;
			L = ++k < FI.nvb ? FI.vb[k] : nv;
			}
		ka[i] += j;
		}

	/* Adjust cg->goff for use in Jacobian computations */
	/* -- see sparseJ() below. */

	for (i = nc; i-- > 0; ) {
		j = zc[i];
		for (cg = Cgrad[j]; cg; cg = cg->next)
			cg->goff = z[cg->varno]--;
		}

	FI.nnz = nzc + 2*(nranges + nvr) + nvb;
	return nc + nranges + nvb + nvr;
	}

 static FILE *
Fopen(char *name, char *kind)
{
	FILE *f;
	if (!(f = fopen(name, "w"))) {
		fprintf(Stderr, "Cannot open %s file \"%s\".\n", kind, name);
		exit(2);
		}
	return f;
	}

 int
main(int argc, char **argv)
{
	ASL *asl;
	char *stub;
	FILE *nl, *logfl = NULL, *statusfl = NULL;
	Path *p;
	int i, tc;
	char buf[256];
	typedef struct { char *msg; int code; } Sol_info;
	static Sol_info solinfo[] = {
		{ "Solution found", 0 },
		{ "Not enough progress", 510 },
		{ "Major iteration limit", 400 },
		{ "Cumulative iteration limit", 401 },
		{ "Time limit", 402 },
		{ "Inconsistent bounds", 502 },
		{ "Domain error", 503 },
		{ "Solver error", 501 }
		};

	Not_Used(argc);

	asl = ASL_alloc(ASL_read_fg);

	/* Get version of PATH */
	Oinfo.bsname = stub = Path_Version();
#ifdef Student_Edition
	Oinfo.version = (char*)M1alloc(strlen(xxxvers+6) + strlen(stub) + 8);
	strcpy(Oinfo.version, "Student ");
	sprintf(Oinfo.version+8, xxxvers+6, stub);
#else
	Oinfo.version = (char*)M1alloc(strlen(xxxvers+6) + strlen(stub));
	sprintf(Oinfo.version, xxxvers+6, stub);
#endif

	asl->i.congrd_mode = 2;	/* sparse Jacobians */
	want_xpi0 = 1;	/* want X0 */
	stub = getstub(&argv, &Oinfo);
	if (wantver)
		show_ver(stdout);
	if (wantfuncs)
		show_funcs();
	if (quitnow | wantfuncs)
		exit(0);
	if (!stub)
		usage_ASL(&Oinfo, 1);

	nl = jac0dim(stub, (fint)strlen(stub));
	if ((i = n_eqn + n_cc) != n_var) {
		fprintf(Stderr, "%s requires a square system; this one is %d x %d.\n",
		    Oinfo.bsname, i, n_var);
		exit(1);
		}

	Options_Default();
	Options_SetInfinity(Infinity);

	if (getopts(argv, &Oinfo))
		exit(2);
	if (statusfile)
		Output_SetStatus(statusfl = Fopen(statusfile, "statusfile"));
	if (logfile)
		Output_SetLog(logfl = Fopen(logfile, "logfile"));
	fg_read(nl, 0);
	FI.n = mcp_adj(asl);

	Options_SetMinorIterationLimit(Options_GetCumulativeIterationLimit());
	if (Options_GetMinorIterationLimit() > 2 * FI.n)
		Options_SetMinorIterationLimit(2 * FI.n);
	if (Options_GetCumulativeIterationLimit() == 0) {
		Options_SetCrashIterationLimit(0);
		Options_SetMajorIterationLimit(0);
		}

	Options_Display();
	p = Path_Create(FI.n, FI.nnz);
	tc = Path_Solve(p);
	i = sprintf(buf, "%s: ", Oinfo.bsname);
	if (tc < 1 || tc > sizeof(solinfo) / sizeof(Sol_info)) {
		i += sprintf(buf + i, "Unexpected return code %d", tc);
		solve_result_num = 500;
		}
	else {
		i += sprintf(buf + i, solinfo[--tc].msg);
		solve_result_num = solinfo[tc].code;
		}
	i += sprintf(buf + i, ".\n%d iterations (%d for crash); %d pivots",
	    Path_TotalMajorIterations(p) + Path_TotalCrashIterations(p),
	    Path_TotalCrashIterations(p),
	    Path_TotalMinorIterations(p));
	i += sprintf(buf + i, ".\n%d function, %d gradient evaluations.",
	    Path_TotalFunctionEvaluations(p),
	    Path_TotalJacobianEvaluations(p));
	write_sol(buf, Path_FinalZ(p), 0, &Oinfo);
	Path_Destroy(p);
	if (logfl != NULL)
		fclose(logfl);
	if (statusfl != NULL)
		fclose(statusfl);
	return 0;
	}

 static void
bad_n(char *who, char *what, int n)
{
	fprintf(Stderr,
	    "Bad dimension argument '%s' to %s: got %d, expected %d\nexiting . . .\n",
	    what, who, n, FI.n);
	exit(9);
	}

 static void
bad_nnz(char *who, char *what, int n)
{
	fprintf(Stderr,
	    "Bad dimension argument '%s' to %s: got %d, expected %d\nexiting . . .\n",
	    what, who, n, FI.nnz);
	exit(9);
	}

 static void
zshow(char *what, real *z, int n)
{
	int i;

	printf("\n%s:\n", what);
	for (i = 0; i < n; i++)
		printf("%d\t%.g\n", i, z[i]);
	}

 int
function_evaluation(int n, real *z, real *f)
{
	ASL *asl = cur_ASL;
	int *Cvar, i, j, nc, nr;
	real *c, *lu;

	if (n != FI.n)
		bad_n("F", "n", n);
	nc = n_con;
	lu = LUrhs;
	conval(z, c = FI.c_scratch, 0);
	Cvar = cvar;
	for (i = 0; i < nc; i++, lu += 2) {
		j = Cvar[i];
		switch (FI.ctype[i]) {
		case C_COMPL:
			f[j] = c[i];
			break;
		case C_LB:
		case C_EQN:
			f[j] = c[i] - lu[0];
			break;
		case C_UB:
			f[j] = c[i] - lu[1];
			break;
		case C_RANGE:
			f[j] = c[i] - z[j+1];
			f[j+1] = z[j];
			break;
		case C_FREE:
			f[j] = 0;
			}
		}
	nr = n - (FI.nvb + FI.nvr);
	for (i = 0; i < FI.nvb; i++, nr++) {
		j = FI.vb[i];
		lu = LUv + 2*j;
		switch(FI.vtype[i]) {
		  case C_LB:
			f[nr] = z[j] - lu[0];
			break;
		  case C_UB:
			f[nr] = z[j] - lu[1];
			break;
		  case C_RANGE:
			f[nr] = z[j] - z[nr+1];
			++nr;
			f[nr] = z[nr-1];
			}
		}
	if (debug & 2) {
		printf("\nf = F(z):\ni\tz[i]\t\tf[i]\n");
		for (i = 0; i < n; i++)
			printf("%d\t%.g\t%.g\n", i, z[i], f[i]);
		}
	return 0;
	}

 void
sparse_struct(int n, int maxnnz, int *row, int *col, int *len)
{
	ASL *asl = cur_ASL;
	int *Cvar, i, j, k, *ka, nc, nv, *vb, *vbe;
	cgrad *cg;
	char *vt;

	if (n != FI.n)
		bad_n("sparse_struct", "n", n);
	if (maxnnz < FI.nnz)
		bad_nnz("sparse_struct", "nnz", maxnnz);
	nv = n_var;
	ka = A_colstarts;
	for (i = 0; i < nv; i++) {
		col[i] = ka[i] + 1;
		len[i] = ka[i+1] - ka[i];
		}
	nc = n_con;
	Cvar = cvar;
	for (i = 0; i < nc; ) {
		j = Cvar[i] + 1;
		for (cg = Cgrad[i++]; cg; cg = cg->next)
			row[cg->goff] = j;
		}
	j = nzc + FI.nvb;
	for (i = 0; i < nc; i++)
		switch (FI.ctype[i]) {
		case C_LB:
		case C_UB:
		case C_FREE:
			col[nv] = j+1;
			len[nv++] = 0;
			break;

		case C_RANGE:
			row[j++] = k = Cvar[i] + 2;
			col[nv] = j;
			len[nv++] = 1;

			row[j++] = k - 1;
			col[nv] = j;
			len[nv++] = 1;
			}
	if (FI.nvb) {
		k = nc + nranges;
		vb = FI.vb;
		vbe = vb + FI.nvb;
		vt = FI.vtype;
		++ka;
		do {
			i = *vb++;
			if (*vt++ == C_RANGE) {
				row[j++] = k += 2;
				col[nv] = j;
				len[nv++] = 1;

				row[ka[i]-1] = row[j++] = k-1;
				col[nv] = j;
				len[nv++] = 1;
				}
			else {
				col[nv] = j+1;
				len[nv++] = 0;
				row[ka[i]-1] = ++k;
				}
			} while (vb < vbe);
		}
	if (debug & 8) {
		printf("\nJacobian nonzeros:\ni\tcol[i]\trow[i]\n");
		for(i = j = 0; j < n; j++)
			for(k = i + len[j]; i < k; i++)
				printf("%d\t%d\t%d\n", i, j, row[i]);
		}
	}

 int
sparseJ(int n, int maxnnz, double *z, double *J)
{
	ASL *asl = cur_ASL;
	int i, j, *ka;

	if (n != FI.n)
		bad_n("sparseJ", "n", n);
	if (maxnnz < FI.nnz)
		bad_nnz("sparseJ", "nnz", maxnnz);
	if (debug & 4)
		zshow("z in sparseJ", z, n);
	jacval(z, J, 0);
	j = nzc + FI.nvb;
	for(i = FI.nvr + nranges; i > 0; --i) {
		J[j++] = 1;
		J[j++] = -1;
		}
	ka = A_colstarts + 1;
	--J;
	for(i = 0; i < FI.nvb; i++)
		J[ka[FI.vb[i]]] = 1;
	if (debug & 4)
		zshow("J in sparseJ", J+1, FI.nnz);
	return 0;
	}

 void
problem_size(int *n, int *nnz)
{
#ifdef Student_Edition
	ASL *asl = cur_ASL;

	if (FI.n  > CRIPPLE_SIZE || FI.nnz > CRIPPLE_NNZ) {
		fflush(stdout);
		fprintf(Stderr,
"\nSorry, the student edition is limited to\n\
%d variables and %d Jacobian nonzeros.\n\
%sou have %d variables and %d nonzeros.\n",
			CRIPPLE_SIZE, CRIPPLE_NNZ,
			n_var != FI.n || nzc != FI.nnz
			? "After problem adjustments,\ny" : "Y",
			FI.n, FI.nnz);
		exit(1);
		}
#endif
	*n = FI.n;
	*nnz = FI.nnz;
	}

 void
bounds(int n, real *z, real *lower, real *upper)
{
	ASL *asl = cur_ASL;
	int i, j, k, nc, nv;
	real Lb, Ub, *lu;

	if (n != FI.n)
		bad_n("bounds", "n", n);
	nv = n_var;
	lu = LUv;
	if (X0)
		memcpy(z, X0, nv * sizeof(real));
	else
		memset(z, 0, nv * sizeof(real));
	for (i = 0; i < nv; i++, lu += 2) {
		lower[i] = lu[0];
		upper[i] = lu[1];
		}
	k = nc = n_con;
	for (i = 0; i < nc; i++)
		if ((j = cvar[i]) >= nv) {
			switch (FI.ctype[i]) {
			case C_LB:
				Lb = 0;
				Ub = Infinity;
				break;
			case C_UB:
				Lb = negInfinity;
				Ub = 0;
				break;
			case C_RANGE:
				lu = LUrhs + (i << 1);
				k = j + 1;
				z[k] = lu[0] + 0.5 * (lu[1] - lu[0]);
				lower[k] = lu[0];
				upper[k] = lu[1];
				/* no break; */
			case C_EQN:
				Lb = negInfinity;
				Ub = Infinity;
				break;
			case C_FREE:
				Lb = 0;
				Ub = 0;
				break;
			default:
				fprintf(Stderr,
				    "bounds: FI.ctype[%d] = %d\n",
				    i, FI.ctype[i]);
				exit(1);
				}
			z[j] = 0;
			lower[j] = Lb;
			upper[j] = Ub;
			}
	for(i = 0, k = n - FI.nvb - FI.nvr; k < n; i++, k++) {
		z[k] = 0;
		switch(FI.vtype[i]) {
		  case C_LB:
			lower[k] = 0;
			upper[k] = Infinity;
			break;
		  case C_UB:
			lower[k] = negInfinity;
			upper[k] = 0;
			break;
		  case C_RANGE:
			lower[k] = negInfinity;
			upper[k] = Infinity;
			z[++k] = 0;
			j = FI.vb[i];
			lu = LUv + 2*j;
			lower[k] = lu[0];
			upper[k] = lu[1];
			break;
		  default:
			fprintf(Stderr, "bounds: FI.vtype[%d] = %d\n",
				i, FI.vtype[i]);
			exit(1);
		  }
		}
	if (debug & 1) {
		printf("i\tz[i]\tlower[i]\tupper[i]\n");
		for(i = 0; i < n; i++)
			printf("%d\t%.g\t%.g\t%.g\n", i, z[i],
				lower[i], upper[i]);
		}
	return;
	}

 void
variable_name(int variable, char *buffer, int buffer_size)
{
	ASL * asl = cur_ASL;

	strncpy(buffer, var_name(variable - 1), buffer_size - 1);
	buffer[buffer_size-1] = '\0';
	return;
	}

 void
constraint_name(int constraint, char *buffer, int buffer_size)
{
	ASL * asl = cur_ASL;

	strncpy(buffer, con_name(constraint - 1), buffer_size - 1);
	buffer[buffer_size-1] = '\0';
	return;
	}

 int
jacobian_evaluation(int n, double *z, int  wantf, double *f, int *nnz,
	int *col_start, int *col_len, int *row, double *data)
{
	int	err;

	if (wantf)
		err = function_evaluation(n, z, f);
	else
		err = 0;

	if (numJacEvals == 0)
		sparse_struct(n, *nnz, row, col_start, col_len);
	numJacEvals++;
	err += sparseJ(n, *nnz, z, data);

	*nnz = col_start[n - 1] + col_len[n - 1] - 1;
	return err;
	}
