/* tunnel to kgdb.
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <search.h>

#include "defs.h"
#include "gdb.h"


static int outfd[2];
static int infd[2];

static pid_t pid;
static bool gdb_enabled;

static void gdb_start(char*, char*);

static void send_command(char *command)
{
	lgdb_log(LOG_DBG, "Sending command \"%s\" to gdb\n", command);

	if (!gdb_enabled)
		return;

	strcat(command, "\n");
	if (write(infd[1], command, strlen(command)) < 0) {
		perror("write");
		close(infd[1]);
		close(outfd[0]);
		exit(-8);
	}
}

static size_t calc_max_num_events()
{
	return PROF_MAX_WALLETS * PROF_MAX_SCOPES * 2;
}

void gdb_close()
{
	close(infd[1]);
	close(outfd[0]);
	
	kill(pid, SIGTERM);
}


void gdb_init(char *kernel, char *pts)
{
	gdb_enabled = false;

	if (!hcreate(calc_max_num_events()))
		exit(-1);

        if (access(kernel, F_OK) == -1) {
                fprintf(lgdb_stderr, "LgDb:  kernel binary %s does not exists.\n", kernel);
		return;
        }

	if (access(pts, F_OK) == -1) {
                fprintf(lgdb_stderr, "LgDb:  pseudo ternimal %s does not exists.\n", pts);
		return;
        }

	gdb_start(kernel, pts);
}

//TODO: add SIGCLD handler
static void gdb_start(char *kernel, char *pts)
{
	gdb_enabled = true;
	if (pipe(infd) < 0) {
		perror("pipe: infd");
		exit(-2);
	}

	if (pipe(outfd) < 0) {
		perror("pipe: outfd");
		close(infd[0]);
		close(infd[1]);
		exit(-3);
	}

	if ((pid = fork()) < 0) {
		perror("fork");
		close(infd[0]);
		close(infd[1]);
		close(outfd[0]);
		close(outfd[1]);
		exit(-4);
	} else if (pid == 0) {     //child
		if (dup2(infd[0], STDIN_FILENO) < 0) {
			perror("dup2: infd");
			close(infd[0]);
			close(infd[1]);
			close(outfd[0]);
			close(outfd[1]);
			exit(-5);
		}

		if (dup2(outfd[1], STDOUT_FILENO) < 0) {
			perror("dup2: outfd");
			close(infd[0]);
			close(infd[1]);
			close(outfd[0]);
			close(outfd[1]);
			exit(-6);
		}

		close(infd[0]);
		close(infd[1]);
		close(outfd[0]);
		close(outfd[1]);

		if (execl("/usr/bin/gdb", "gdb", kernel, (char *)0) < 0) {
			perror("execl");
			exit(-7);
		}
	}
	
	close(infd[0]);
	close(outfd[1]);

	sleep(5);
	char command[100];

	memset(command, 0, sizeof(command));
	sprintf(command, "%s", "set logging on");

	send_command(command);

	memset(command, 0, sizeof(command));
	sprintf(command, "%s %s", "target remote", pts);

	send_command(command);
	
}

int gdb_add_event(struct gdb_event *event)
{
	char command[500];
	ENTRY e, *ep;

	if (!event)
		return -1;

	e.key = event->name;
	e.data = (void *)event;
	ep = hsearch(e, ENTER);
	if (!ep) {
		lgdb_log(LOG_ERR, "failed to add event %s\n", event->name);
		return -1;
	}

	memset(command, 0, sizeof(command));
	sprintf(command, "b %s", event->name);
	send_command(command);

	return 0;
}

int gdb_remove_event(struct gdb_event *event)
{
	if (!event)
		return -1;

	//TODO replace hsearsh with something else as it does not support removal
	return 0;
}

static void gdb_continue()
{
	send_command("c");
}
