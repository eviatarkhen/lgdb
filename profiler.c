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

#define _GNU_SOURCE
#include <sys/time.h>

#include "profiler.h"
#include "gdb.h"
#include "defs.h"

typedef struct prof_scope
{
	char *start;
	char *end;
	unsigned int used;
	unsigned long long charge;
	struct timeval start_time;
} prof_scope_t;

typedef struct wallet
{
	unsigned int used;
	unsigned int num_of_scopes;
	unsigned long long charge;
	prof_scope_t scopes[PROF_MAX_SCOPES];
} wallet_t;

static wallet_t wallets[PROF_MAX_WALLETS];
static int num_of_wallets;

void prof_init()
{
	memset(wallets, 0, sizeof(wallets));
}

int prof_create_wallet()
{
	int i;
	if (num_of_wallets == PROF_MAX_WALLETS)
		return -1;
	
	for (i = 0; i < PROF_MAX_WALLETS  && wallets[i].used ; ++i);
		
	++num_of_wallets;
	wallets[i].used = 1;
	
	return i;
}

int prof_delete_wallet(int wallet)
{
	if (wallet < 0 || wallet >= PROF_MAX_WALLETS)
		return -1;

	wallets[wallet].used = 0;

	--num_of_wallets;

	return 0;
}


int prof_add_scope(int wallet, char *start, char* end)
{
	wallet_t *w = NULL;
	unsigned int i;

	if (wallet < 0 || wallet >= PROF_MAX_WALLETS)
		return -1;
	
	w = &wallets[wallet];

	if (!w->used)
		return -1;

	if (w->num_of_scopes == PROF_MAX_SCOPES)
		return -1;

	if (!start || !end)
		return -1;

	for (i = 0; i < PROF_MAX_SCOPES  && (w->scopes)[i].used ; ++i);

	w->scopes[i].used = 1;
	++(w->num_of_scopes);

//	gdb_add_bp(start);
//	gdb_add_bp(end);

	return i;
}

int prof_remove_scope(int wallet, int scope)
{
	wallet_t *w = NULL;

	if (wallet < 0 || wallet >= PROF_MAX_WALLETS)
		return -1;

	w = &wallets[wallet];

	if (!w->used)
		return -1;

	if (scope < 0 || scope >= PROF_MAX_SCOPES)
		return -1;

	w->scopes[scope].used = 0;

//	gdb_remove_bp(w->scopes[scope].start);
//	gdb_remove_bp(w->scopes[scope].end);

	return 0;
}
