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
#include <stdio.h>
#include <string.h>

#include "profiler.h"
#include "gdb.h"
#include "defs.h"

typedef struct prof_scope
{
	unsigned int used;
	unsigned long long charge;
	struct timeval start_time;
	struct gdb_event event;
} prof_scope_t;

typedef struct wallet
{
	unsigned int used;
	unsigned int num_of_scopes;
	unsigned long long charge;
	char name[512];
	prof_scope_t scopes[PROF_MAX_SCOPES];
} wallet_t;

static wallet_t wallets[PROF_MAX_WALLETS];
static int num_of_wallets;
//----------------------------------------------------------------------------------
static int start_charge_cb(void *data)
{
	prof_scope_t *scope = (prof_scope_t *)data;
	gettimeofday(&(scope->start_time), NULL);
	lgdb_log(LOG_DBG, "profiling start event on scope %s \n", scope->event.name);

	return 0;
}

//----------------------------------------------------------------------------------
static int end_charge_cb(void *data)
{
	prof_scope_t *scope = (prof_scope_t *)data;
	struct timeval tv;
	unsigned long long elapsed;

	if (!scope)
		return -1;

	gettimeofday(&tv, NULL);

	elapsed = tv.tv_sec * 1000000 + tv.tv_usec - scope->start_time.tv_sec * 1000000 + scope->start_time.tv_usec; 
	scope->charge += elapsed;

	lgdb_log(LOG_DBG, "profiling end event on scope %s elapsed %llu\n", scope->event.name, elapsed);

	return 0;
}
//----------------------------------------------------------------------------------
void prof_init()
{
	memset(wallets, 0, sizeof(wallets));
}
//----------------------------------------------------------------------------------
int prof_create_wallet(char *name)
{
	int i;

	if (!name) {
		fprintf(lgdb_stdout, "Failed to create wallet. No name was given\n");
		return -1;
	}

	if (num_of_wallets == PROF_MAX_WALLETS) {
		fprintf(lgdb_stdout, "Failed to create wallet. Reached the maximun number of wallets %d\n", PROF_MAX_WALLETS);
		return -1;
	}
	
	for (i = 0; i < PROF_MAX_WALLETS  && wallets[i].used ; ++i);
		
	++num_of_wallets;
	wallets[i].used = 1;
	strcpy(wallets[i].name, name);
	
	fprintf(lgdb_stdout, "Created wallet #%d %s\n", i, name);

	return i;
}
//----------------------------------------------------------------------------------
int prof_delete_wallet(int wallet)
{
	if (wallet < 0 || wallet >= PROF_MAX_WALLETS)
		return -1;

	wallets[wallet].used = 0;

	--num_of_wallets;

	return 0;
}
//----------------------------------------------------------------------------------
int prof_add_scope(int wallet, char * name, char * start, char * end)
{
	wallet_t * w = NULL;
	unsigned int i;
	struct gdb_event * event;

	if (wallet < 0 || wallet >= PROF_MAX_WALLETS) {
		fprintf(lgdb_stdout, "Invalid wallet %d\n", wallet);
		return -1;
	}
	
	w = &wallets[wallet];

	if (!w->used) {
		fprintf(lgdb_stdout, "Wallet %d wasn't created\n", wallet);
		return -1;
	}

	if (w->num_of_scopes == PROF_MAX_SCOPES) {
		fprintf(lgdb_stdout, "Reached the maximun number of scopes for wallet %d\n", PROF_MAX_SCOPES);
		return -1;
	}

	if (!start || !end) {
		fprintf(lgdb_stdout, "Invalid scope\n");
		return -1;
	}

	for (i = 0; i < PROF_MAX_SCOPES  && (w->scopes)[i].used ; ++i);

	w->scopes[i].used = 1;
	++(w->num_of_scopes);

	event = &(w->scopes[i].event);
	event->name = name;
        event->data = &w->scopes[i];
        event->start_callback = start_charge_cb;
        event->end_callback = end_charge_cb;

	// TODO handle fail
	gdb_add_event(event, start, end);

	fprintf(lgdb_stdout, "Add scope #%d to wallet %s.\nStart: %s\nEnd: %s\n", i, w->name, start, end);
	return i;
}
//----------------------------------------------------------------------------------
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

	gdb_remove_event(&(w->scopes[scope].event));

	return 0;
}
//----------------------------------------------------------------------------------
void prof_start()
{
	gdb_continue();
}
//----------------------------------------------------------------------------------
unsigned long long prof_get_charge(int wallet)
{
	int i;
	unsigned long long charge = 0;

	for (i = 0; i < PROF_MAX_SCOPES; ++i) {
		if (wallets[i].used)
			charge += wallets[i].charge;
	}

	return charge;
}
//----------------------------------------------------------------------------------
