# /****************************************************************
# Copyright (C) 2000 Lucent Technologies
# All Rights Reserved
#
# Permission to use, copy, modify, and distribute this software and
# its documentation for any purpose and without fee is hereby
# granted, provided that the above copyright notice appear in all
# copies and that both that the copyright notice and this
# permission notice and warranty disclaimer appear in supporting
# documentation, and that the name of Lucent or any of its entities
# not be used in advertising or publicity pertaining to
# distribution of the software without specific, written prior
# permission.
#
# LUCENT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
# INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
# IN NO EVENT SHALL LUCENT OR ANY OF ITS ENTITIES BE LIABLE FOR ANY
# SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
# IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
# ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
# THIS SOFTWARE.
# ****************************************************************/

# For making minos.exe with WATCOM 11

# Invoke with "wmake -u -f makefile.wat" .

CC = wcc386
CFLAGS = -fpd -DSVS -bt=nt
S = ..
# "S = .." assumes this directory is solvers/minos .
Ms = src
L = $S\amplsolv.lib $(Ms)\minoswat.lib $S\watf2c.lib
# Adjust "M = src" appropriately if you put the MINOS
# source directory elsewhere.

# Add -DKR_headers to CFLAGS if your C compiler does not
# understand ANSI C function headers, e.g.
#	CFLAGS = -O -DKR_headers
# If things don't run right, you may need to change -O to -g
# so you can poke around with a debugger.
# See $S/makefile for other comments about CFLAGS.

.c.obj:
	$(CC) $(CFLAGS) -I$S $*.c

m = m55.obj m55ver.obj funcon.obj $L

minos.exe: $m
	wcl386 -l=nt -bt=nt -fe=minos.exe $m

m55.obj: $S/asl.h $S/getstub.h
funcon.obj: $S/asl.h

# If necessary, change `uname -sr` in the following sed command to a
# phrase summarizing your machine and operating system.
m55ver.c: m55ver.c0
	sed -f $S\ss.wat m55ver.c0 >m55ver.c
#	echo create m55ver.c by suitably editing m55ver.c0

## Student version of minos...
sminos = sm55.obj sm55ver.obj funcon.obj $S/sjac0dim.obj $L

sminos.exe: $(sminos)
	-del sminos.exe
	$(CC) $(sminos) $L
	ren sm55.exe sminos.exe

$S/sjac0dim.obj:
	cd $S; make sjac0dim.obj

sm55.obj: m55.c

sm55ver.c: m55ver.c
	sed -f sm55ver.ss m55ver.c >sm55ver.c
