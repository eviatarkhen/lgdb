/* Top level stuff for LGDB(lguest gdb) the virtualization based debugger
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

#include "defs.h"
#include "lgdb_cli.h"

#include <sys/select.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <assert.h>

#define LGDB_PROMPT "(lgdb) "

struct cmd_list_element
{
        /* Points to next command in this list */
        struct cmd_list_element *next;
        /* Name of this command */
        char *name;
        /* Command's callback */
        void (*function) (char *args, int from_tty);
        /* Documentation for this command */
        char *doc;
};

struct readline_input_state
{
	char *linebuffer;
	char *linebuffer_ptr;
} readline_input_state;

struct cmd_list_element *cmdlist;

//char *saved_command_line;
//int saved_command_line_size = 100;

void
add_cmd(char *name, void (*func) (char *,int), char *doc, struct cmd_list_element **list)
{
	struct cmd_list_element *c = (struct cmd_list_element *) malloc(sizeof(struct cmd_list_element));
	struct cmd_list_element *p;

	if ( !c ) {
		fprintf(lgdb_stderr, "malloc fail with cmd: %s\n", name);
		return;
	}

	if ( *list == NULL ) {
		c->next = *list;
		*list = c;
	}
	else {
		p = *list;
		while ( p->next )
			p = p->next;
		c->next = p->next;
		p->next = c;
	}
	
	c->name = name;
	c->doc  = doc;
	c->function = func;
}

void lgdb_quit(char * c, int i)
{
	exit(0);
}

static void lgdb_cli_prof(char * c, int i)
{
}

void
init_cli_cmds(void)
{
	add_cmd("quit", lgdb_quit, "quit lgdb", &cmdlist);
	add_cmd("q", lgdb_quit, "quit lgdb", &cmdlist);
	add_cmd("profile", lgdb_cli_prof, "quit lgdb", &cmdlist);
}

static void
display_lgdb_prompt(char *display)
{
	if (display)
		fprintf(lgdb_stdout, "%s", display);
	else
		fprintf(lgdb_stdout, "%s", LGDB_PROMPT);
	fflush (lgdb_stdout);	
}

// returns the length of the command part, i.e not including arguments
static int
get_command_length(const char *text)
{
	const char *p = text;

	while (isalnum(*p))
		++p;
	
	return p - text;
}

static struct cmd_list_element *
find_cmd(char *command, int len, struct cmd_list_element *clist)
{
	struct cmd_list_element *found;

	found = (struct cmd_list_element *) NULL;

	for (found = clist; found; found = found->next) {
		if (!strncmp(command, found->name, len) && found->function )
			break;
	}

	return found;
}

static struct cmd_list_element *
lookup_cmd(char **line, struct cmd_list_element *clist)
{
	int command_len, tmp;
	char *command;
	struct cmd_list_element *found = 0;

	/* Extract the command */
	command_len = get_command_length(*line);
	
	if (command_len == 0)
		return 0;

	command = (char *) alloca(command_len + 1);	
	memcpy(command, *line, len);
	command[len] = '\0';

	/* Look it up */
	found = find_cmd(command, len, clist);	

	/* If was found, lower case the command and search again */
	if ( !found ) {
		for (tmp = 0; tmp < command_len; tmp++) {
			char x = command[tmp];

			command[tmp] = isupper(x) ? tolower(x) : x; 
		} 
		found = find_cmd(command, len, clist);	
	}

	*line += command_len;

	if (!found)
		fprintf(lgdb_stdout, "unknown command: %s\n", command);

	return found;
}

static void
execute_command(char *line, int from_stdin)
{
	struct cmd_list_element *c;

	assert(line);

	c = lookup_cmd(&line, cmdlist);

	if (c && c->function)
		c->function(line, from_stdin);
}


// handle a command line
// if the command ends with '\' read another command line
// if the line is emply do nothing
// ignore spaces
// if starts with '#' handle as a comment
static void
command_line_handler(char *rl)
{
	static char *linebuffer = 0;
	static unsigned linelength = 0;
	static int more_to_come = 0;
	char *p;
	char *p1;
	char *nline;
	//char got_eof = 0;

	assert(rl && rl != (char *)EOF );

	if ( linebuffer == 0 ) {
		linelength = 80;
		linebuffer = (char *)malloc(linelength);
		if ( !linebuffer ) {
			perror("command_line_handler: malloc");
			exit(-1);
		}
	}

	p = linebuffer;

	if ( more_to_come ) {
		 strcpy (linebuffer, readline_input_state.linebuffer);
		 p = readline_input_state.linebuffer_ptr;
		 free (readline_input_state.linebuffer);
		 more_to_come = 0;
	}

	if ( strlen(rl) + 1 + (p - linebuffer) > linelength ) {
		linelength = strlen(rl) + 1 + (p - linebuffer);
		nline = (char *)realloc(linebuffer, linelength);
		if ( !nline ) {
			perror("command_line_handler: realloc");
			exit(-1);
		}
		p += nline - linebuffer;
		linebuffer = nline;
	}
	
	p1 = rl;

	/* copy the readline */
	while ( *p1 )
		*p++ = *p1++;

	free(rl); /* this was allocated at readline */
	
	if (p > linebuffer && *(p - 1) == '\\') {
		*p = '\0';
		p--;                      /* Put on top of '\'.  */

		readline_input_state.linebuffer = strdup (linebuffer);
		readline_input_state.linebuffer_ptr = p;

		/* We will not invoke a execute_command if there is more
		   input expected to complete the command.  So, we need to
		   print an empty prompt here.  */
		more_to_come = 1;
		display_lgdb_prompt ("");
		return;
	}

	/* If we just got an empty line, do nothing - TODO: repeat the last command */
	if ( p == linebuffer && *p != '\\') {
		//command_handler(saved_command_line);
		display_lgdb_prompt (0);
		return;
	}

	// remove starting spaces
	for (p1 = linebuffer; *p1 == ' ' || *p1 == '\t'; p1++);

	// if empty - do nothing
	if ( !*p1 ) {
		//command_handler(saved_command_line);
		display_lgdb_prompt (0);
		return;
	}

	*p = 0;

	/* commnet */
	if ( *p1 == '#' )
	{
	//	*p1 = '\0';
		display_lgdb_prompt (0);
		return;
	}

	//if (linelength > saved_command_line_size)
	//{
	//  saved_command_line = realloc (saved_command_line, linelength);
	//  saved_command_line_size = linelength;
	//}
	//strcpy (saved_command_line, linebuffer);
	//if (!more_to_come)
	//{
	//    command_handler (saved_command_line);
	//    display_lgdb_prompt (0);
	//}
	//return;

	execute_command(linebuffer, stdin == instream);
	display_lgdb_prompt (0);
	return;
}

// read chars until \n or EOF
static void
lgdb_readline(void)
{
	int c;
	char *line;
	unsigned int input_offset = 0;
	int line_size = 80;

	line = (char *) malloc(line_size);
	if (!line) {
		perror("lgdb_readline: malloc");
		exit(-2);
	}

	while(1) {
		c = fgetc(instream);

		if (c == EOF) {
			if ( input_offset > 0)
				break;
			free(line);
			command_line_handler(0);
			return;
		}
	
		if ( c == '\n' ) {
			if ( input_offset > 0 && line[input_offset-1] == '\r' )
				--input_offset;
			break;
		}

		line[input_offset++] = c;
		while (input_offset >= line_size) {
			line_size *= 2;
			line = (char *)realloc(line, line_size);
		}
	}

	line[input_offset++] = '\0';
	command_line_handler(line);
}

// cli main loop: wait for events on stdin
static void
start_event_loop(void)
{
	while (1) {
		int instream_fd = fileno(instream);

		/* wait with select on input stream */
		fd_set readfds;

		FD_ZERO(&readfds);
		FD_SET(instream_fd, &readfds);
		if (select(instream_fd+1, &readfds, NULL, NULL, NULL) == -1 && errno != EINTR) {
			perror("select");
			exit(-1);
		}

		/* read from input stream */
		lgdb_readline();	
	}
}

void
cli_command_loop(void)
{
        display_lgdb_prompt(0);
	start_event_loop();
}
