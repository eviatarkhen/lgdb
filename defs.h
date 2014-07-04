/* Basic definitions for LGDB(lguest gdb) the virtualization based debugger
   Copyright (C) 2012, 2011-2012 Eviatar Khen Technologies, Inc.

   This file is part of LGDB.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#ifndef DEFS_H
#define DEFS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lgdb_logger.h"

extern FILE *lgdb_stdout;
extern FILE *lgdb_stderr;
extern FILE *lgdb_stdlog;
extern FILE *lgdb_stdin;
extern FILE *instream;

#define VERSION "1.0.0"

#define PROF_MAX_WALLETS 10
#define PROF_MAX_SCOPES 10

#endif 
