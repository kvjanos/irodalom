/****************************************************************
Copyright (C) 1999 Lucent Technologies
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

#ifndef _MT
#define _MT 1
#endif

#include <errno.h>
#include <float.h>
#include <math.h>
#include <signal.h>
#include <windows.h>

#undef Need_set_errno
#ifdef errno
#ifndef No_set_errno_ASL
#define Need_set_errno
static int set_errno(int);
#endif /*No_set_errno_ASL*/
#else
#define set_errno(x) x
#endif /*errno*/

#ifdef WATCOM
#define matherr_rettype double
#define _DOMAIN DOMAIN
#define _SING SING
#define _TLOSS TLOSS
#define _OVERFLOW OVERFLOW
#else
#ifndef matherr_rettype
#define matherr_rettype int
#endif
#endif

#ifdef __cplusplus
extern "C" {
 void fpinit_ASL(void);
 }
#endif

 static void
siglistener(void *arg)
{
	DWORD n;
	HANDLE *Sig = (HANDLE*)arg;
	int s[3], sig;
	void(*oldsig)(int);

	for(;;) {
		s[1] = 0;
		if (!ReadFile(Sig[0], s, sizeof(s), &n,0) || n <= 0)
			break;
		if (s[1] > 0 && s[1] != GetCurrentProcessId())
			s[0] = 0;
		/* Must use hard-wired numbers since the brain-dead compiler */
		/* vendors use inconsistent signal numbering... */
		switch(s[0]) {
		  case 2:
			sig = SIGINT;
			goto int_or_break;
		  case 21:
			sig = SIGBREAK;
 int_or_break:
			if ((oldsig = signal(sig,SIG_IGN)) == SIG_IGN) {
				sig = 0;
				break;
				}
			signal(sig, oldsig);
			break;
		  case 15:
			sig = SIGTERM;
			if ((oldsig = signal(sig,SIG_DFL)) != SIG_IGN)
				signal(sig, oldsig);
			break;
		  default:
			sig = 0;
		  }
		if (sig) {
			s[2]++;
			if (s[1] >= 0)
				s[0] = 0;
			}
		WriteFile(Sig[1], s, n, &n, 0);
		if (sig)
			raise(sig);
		Sleep(50);
		}
	}

 static void
siglisten(void)
{
	static HANDLE Sig[2];
	HANDLE h;
	char *s;

	if (s = getenv("SW_sigpipe")) {
		if (!(Sig[0] = (HANDLE)strtoul(s,&s,10))
		 || *s != ','
		 || !(Sig[1] = (HANDLE)strtoul(s+1,&s,10)))
			return;
		if (*s == ',')
			isatty_ASL = (int)strtoul(s+1,&s,10);
		h = (HANDLE)_beginthread(siglistener, 0, Sig);
		SetThreadPriority(h, THREAD_PRIORITY_HIGHEST);
		}
	}

 void
fpinit_ASL(void)
{
	static int first = 1;

#ifndef No_Control87 /* for DEC Alpha */
	_control87(MCW_EM | PC_53, MCW_EM | MCW_PC);
#endif
	if (first) {
		first = 0;
		siglisten();
		}
	}

 matherr_rettype
_matherr( struct _exception *e )
{
	switch(e->type) {
	  case _DOMAIN:
	  case _SING:
		errno = set_errno(EDOM);
		break;
	  case _TLOSS:
	  case _OVERFLOW:
		errno = set_errno(ERANGE);
	  }
	return 0;
	}

#ifdef Need_set_errno

#undef errno
int errno;

 static int
set_errno(int n)
{ return errno = n; }

#endif /*Need_set_errno*/
