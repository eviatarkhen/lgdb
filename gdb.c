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
#include <assert.h>
#include <errno.h>
#include <ctype.h>

#include "defs.h"
#include "gdb.h"

#define MAX_BP 100


static int outfd[2];
static int infd[2];
static void * bp_to_callback[MAX_BP];

static pid_t pid;
static bool gdb_enabled;

static void gdb_start(char*, char*);
//--------------------------------------------------------------------------------
static size_t wait_for_response(char * response)
{
	size_t bytes_read = 0;
        size_t bytes_read_total = 0;
	char * buf;

	if (response) 
		buf = response;
	else {
		buf = malloc(1000);
		if (!buf) {
			perror("malloc");
			exit(-10);
		}
	}
		
	do
	{
		bytes_read = read(outfd[0], buf + bytes_read_total, 1000);
		if (bytes_read == -1) {
			if (errno != EAGAIN) {
				perror("read");
				exit(-10);
			}
			else {
				if (!bytes_read_total) {
					usleep(100);
					continue;
				}
				break;
			}
		}
		bytes_read_total += bytes_read;
	}
	while (bytes_read > 0);

	buf[bytes_read_total] = '\0';
	lgdb_log(LOG_DBG, "read from gdb \"%s\"\n", buf);

	if (!response) {
		free(buf);
		return 0;
	}

	return bytes_read_total;
}
//--------------------------------------------------------------------------------
static void send_command(char * command, bool should_respond, char * response)
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

	if (should_respond)
		wait_for_response(response);
}
//--------------------------------------------------------------------------------
void gdb_init(char *kernel, char *pts)
{
	gdb_enabled = false;

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
//--------------------------------------------------------------------------------
static int extract_bp_number(const char * line)
{
	char * bp_start = NULL;
	char * bp_end = NULL;

	assert(line);
	assert(!strncmp(line, "Breakpoint ", strlen("Breakpoint ")));

	bp_start = (char *)line + strlen("Breakpoint "); 	
	bp_end = bp_start;
	while (isdigit(*bp_end))
		++bp_end;

	*bp_end = '\0';

	return atoi(bp_start);
}
//--------------------------------------------------------------------------------
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
	char command[1000];

	fcntl(outfd[0], F_SETFL, O_NONBLOCK);

	wait_for_response(NULL);

	memset(command, 0, sizeof(command));
	sprintf(command, "%s", "set logging on");

	send_command(command, true, NULL);

	memset(command, 0, sizeof(command));
	sprintf(command, "%s %s", "target remote", pts);

	send_command(command, true, NULL);
	
}
//--------------------------------------------------------------------------------
void gdb_close()
{
	close(infd[1]);
	close(outfd[0]);
	
	kill(pid, SIGTERM);
}
//--------------------------------------------------------------------------------
void gdb_continue()
{
	char response[512];
	unsigned int bp = 0;
	struct gdb_event * event = NULL;
	int (* callback)(void * data) = NULL;

	send_command("c", true, response);

	bp = extract_bp_number(response);
	if (bp < 1 || bp > MAX_BP) {
		lgdb_log(LOG_DBG, "got an invalid event, reponse %s\n", response);
		return;
	}
	 
	callback = bp_to_callback[bp];
	assert(callback);

	callback(event->data);
}
//--------------------------------------------------------------------------------
int gdb_add_event(struct gdb_event * event, char * start, char * end)
{
	char command[512];
	char response[512];

	if (!event || ! start || !end)
		return -1;

	memset(command, 0, sizeof(command));
	sprintf(command, "b %s", start);
	send_command(command, true, response);
	if ((event->bp_start = extract_bp_number(response)) < 1)
		return -1; 

	bp_to_callback[event->bp_start] = event->start_callback;

	memset(command, 0, sizeof(command));
	sprintf(command, "b %s", end);
	send_command(command, true, response);
	if ((event->bp_end = extract_bp_number(response)) < 1)
		return -1; 

	bp_to_callback[event->bp_end] = event->end_callback;

	lgdb_log(LOG_DBG, "added the event \"%s\" with start at bp %s, end at bp %s\n", event->name, start, end);
	return 0;
}
//--------------------------------------------------------------------------------
int gdb_remove_event(struct gdb_event * event)
{
	if (!event)
		return -1;

	//TODO replace hsearsh with something else as it does not support removal
	return 0;
}
//--------------------------------------------------------------------------------
void gdb_send_command(char * command)
{
	char response[512];

	send_command(command, true, response);

	fprintf(lgdb_stdout, "%s\n", response);
}
//--------------------------------------------------------------------------------
