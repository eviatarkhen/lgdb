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

#include "gdb.h"
#include "lgdb_logger.h"

static int outfd[2];
static int infd[2];

static pid_t pid;

static void send_command(char *command)
{
	lgdb_print(LOG_DBG, "Sending command \"%s\" to gdb\n", command);

	strcat(command, "\n");
	if (write(infd[1], command, strlen(command)) < 0) {
		perror("write");
		close(infd[1]);
		close(outfd[0]);
		exit(-8);
	}
}

void gdb_close()
{
	close(infd[1]);
	close(outfd[0]);
	
	kill(pid, SIGTERM);
}

//TODO: add SIGCLD handler

void
gdb_init(char *kernel, char *pts)
{
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
