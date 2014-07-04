/* Logging API (lguest gdb) the virtualization based debugger
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

#ifndef LGDB_LOGGER_H
#define LGDB_LOGGER_H

typedef enum {
	LOG_DBG = 0,
	LOG_ERR    ,
	LOG_MAX
} log_type;

extern unsigned int log_type_enabled[LOG_MAX];

extern void logger_init();
extern void lgdb_print(log_type type, const char *format, ...);

extern void enable_log_type(log_type type, unsigned int enable);

#endif 
