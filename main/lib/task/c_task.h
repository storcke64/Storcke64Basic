/***************************************************************************

  c_task.h

  (c) 2000-2012 Benoît Minisini <gambas@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

#ifndef __C_TASK_H
#define __C_TASK_H

#include <sys/types.h>
#include <signal.h>

#include "main.h"

#ifndef __C_TASK_C
extern GB_DESC TaskDesc[];
#endif

typedef
	struct {
		GB_BASE ob;
		GB_LIST list;
		pid_t pid;
		int fd_out;
		int fd_err;
		int status;
		volatile sig_atomic_t stopped;
		unsigned something_read : 1;
		unsigned child : 1;
	}
	CTASK;

#define THIS ((CTASK *)_object)

#define RETURN_DIR_PATTERN "/tmp/gambas.%d/%d/task"
#define RETURN_FILE_PATTERN "/tmp/gambas.%d/%d/task/%d"

#endif
