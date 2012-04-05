/****************************************************************
Copyright (C) 1999-2001 Lucent Technologies
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

int isatty_ASL; /* for use with "sw" under NT */

#ifndef MSpc
#ifdef MSDOS
#define MSpc
#else

#ifdef _WIN32
#undef WIN32
#define WIN32
#endif

#ifdef WIN32
#define MSpc
#endif
#endif /*MSDOS*/
#endif /*MSpc*/

#ifdef KR_headers
#define Void /*void*/
#else
#define Void void
#endif

#ifdef __cplusplus
extern "C" {
 void fpinit_ASL(Void);
 }
#endif
#undef FP_INIT_DONE

#ifndef ASL_NO_FP_INIT

#ifdef __linux__
#ifndef NO_fpu_control
#define FP_INIT_DONE
#include "fpu_control.h"

#ifdef __alpha__
#ifndef USE_setfpucw
#define __setfpucw(x) __fpu_control = (x)
#endif
#endif

#ifndef _FPU_SETCW
#undef  Can_use__setfpucw
#define Can_use__setfpucw
#endif

 void
fpinit_ASL(Void)
{
#ifdef Can_use__setfpucw /* Has __setfpucw gone missing from S.u.S.E. 6.3? */
	__setfpucw(_FPU_IEEE - _FPU_EXTENDED + _FPU_DOUBLE);
#else
	__fpu_control = _FPU_IEEE - _FPU_EXTENDED + _FPU_DOUBLE;
	_FPU_SETCW(__fpu_control);
#endif
	}
#endif /* NO_fpu_control */
#endif /* __linux__ */

#ifdef sgi
#ifndef _ABIO32
#define FP_INIT_DONE
#include <sys/fpu.h>

 void
fpinit_ASL(Void)
{
	union fpc_csr f;
	f.fc_word = get_fpc_csr();
	f.fc_struct.flush = 0;
	set_fpc_csr(f.fc_word);
	}
#endif
#endif

#ifdef MSpc
#ifndef No_Control87
#include "float.h"
#ifdef SYMANTEC
extern int _8087;
#define MCW_EM _MCW_EM
#define PC_53 _PC_53
#endif

#define FP_INIT_DONE
 void
fpinit_ASL(Void)
{
#ifdef SYMANTEC
	if (_8087)
#endif
	_control87(MCW_EM | PC_53, MCW_EM | MCW_PC);
	}
#endif
#endif

#ifdef __sun
#ifdef __i386
extern
#ifdef __cplusplus
	"C"
#endif
	int fpsetprec(int);

#define PC_24	0
#define PC_53	0x200
#define PC_64	0x300

#define FP_INIT_DONE
 void
fpinit_ASL(Void)
{	fpsetprec(PC_53);
	}
#endif
#endif /* __i386 __sun */

/* Currently, FP_PD is the default on FreeBSD, but enabled traps */
/* can cause surprises, so we restore the default IEEE mask. */
#ifdef __FreeBSD__
#include "floatingpoint.h"
#define FP_INIT_DONE
 void
fpinit_ASL(Void)
{
	fpsetprec(FP_PD);
	fpsetmask(0);
	}
#endif /* __FreeBSD__ */

#endif /* ASL_NO_FP_INIT */

#ifndef FP_INIT_DONE
void fpinit_ASL(Void) {}
#endif
