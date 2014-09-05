/* logdb logger API
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
#include <stdarg.h>
#include <sys/time.h>
#include <time.h>
#include "lgdb_logger.h"

static FILE* log_file;

unsigned int log_type_enabled[LOG_MAX] = { 1, 1 };

void logger_init()
{
	log_file = fopen("lgdb.log", "a+");	
}

static char* type_to_str(log_type type)
{
	switch(type)
	{
	case LOG_DBG:
		return "DEBUG";
	case LOG_ERR:
		return "ERROR";
	default:
		break;
	}
	return "Unknown";
}
void enable_log_type(log_type type, unsigned int enable)
{
	log_type_enabled[type] = enable;
}

void lgdb_log(log_type type, const char *format, ...)
{
	va_list args;
	struct timeval tv;
	struct tm ts;
	char buf[80];

        if (!log_type_enabled[type])
		return;

	gettimeofday(&tv, NULL);
	ts = *localtime(&tv.tv_sec);
	strftime(buf, sizeof(buf), "%b %d %H:%M:%S", &ts);
	fprintf(log_file, "%s %s : [%lu.%lu] ", buf, type_to_str(type), tv.tv_sec, tv.tv_usec / 1000);
	va_start(args, format);
	vfprintf(log_file, format, args);
	va_end(args);
	fflush(log_file);
}
