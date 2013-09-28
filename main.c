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
#include "main.h"
#include "lgdb_cli.h"

#include <getopt.h>

FILE *lgdb_stdout;
FILE *lgdb_stderr;
FILE *lgdb_stdlog;
FILE *lgdb_stdin;

FILE *instream;

static int
start_cli(void)
{
	cli_command_loop();
	return 1;
}

static void
lgdb_init(void)
{
	cmdlist = NULL;
	init_cli_cmds();
}

static int
captured_main(void *data)
{
	struct main_args *context = data;
	int argc = context->argc;
	char **argv = context->argv;

	lgdb_stdout = stdout;
	lgdb_stderr = stderr;
	lgdb_stdlog = lgdb_stderr;
	lgdb_stdin = stdin;

	instream = stdin;

	char pts[10] = "/dev/pts/";

	/* Parse arguments */
	int c;
	static struct option opts[] = 
	{
		{"help", 0, NULL, 'h'},
		{"version", 0, NULL, 'v'},
		{"pts", 1, NULL, 'p'},
		{NULL}
	};

	while ((c = getopt_long(argc, argv, "hv", opts, NULL)) != EOF) {
		switch(c)
		{
		case 'v':
			print_lgdb_version();
			exit(0);
		case 'h':
			print_lgdb_help();
			exit(0);
		case 'p':
			strcat(pts, optarg);
			break;
		default:
			fprintf(lgdb_stderr, "LgDb: Use %s --help for a complete list of options.\n", argv[0]);
			exit(0);
		}
	} 

	if (strlen(pts) == strlen("/dev/pts/")) {
		fprintf(lgdb_stderr, "LgDb: /dev/pts must be given\n");
		exit(0);	
	}

	/* Any arguments left on the command lineis unexpected and would be ignored */
	if (optind < argc)
		fprintf(lgdb_stderr, "LgDb: Excess command line arguments ignored. (%s%s)\n", argv[optind], (optind == argc - 1) ? "" : "...");

	print_lgdb_version();
	
	lgdb_init();

	start_cli();

	// shouldn't get here - exist is through exit command
	fprintf(lgdb_stderr, "How come I got here!!!\n");
	return 0;	
}

int lgdb_main(struct main_args *args)
{
	return captured_main(args);
}
