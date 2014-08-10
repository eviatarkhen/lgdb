/* Main interface for LGDB(lguest gdb) the virtualization based debugger
   Copyright (C) 2012, 2007-2012 Eviatar Khen Technologies, Inc.

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

#ifndef PROFILER_H
#define PROFILER_H

extern void prof_init();

extern int prof_create_wallet(char *name);
extern int prof_delete_wallet(int wallet);

extern int prof_add_scope(int wallet, char *start, char* end);
extern int prof_remove_scope(int wallet, int scope);

extern void prof_start();

extern unsigned long long prof_get_charge(int wallet);
#endif 
