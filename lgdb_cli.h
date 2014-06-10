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

#ifndef LGDB_CLI_H
#define LGDB_CLI_H

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

extern void print_lgdb_version(void);
extern void print_lgdb_help(void);
extern void cli_command_loop(void);
extern void init_cli_cmds(void);
extern void add_cmd(char *name, void (*func) (char *,int), char *doc, struct cmd_list_element **list);

extern struct cmd_list_element *cmdlist; 
#endif 
