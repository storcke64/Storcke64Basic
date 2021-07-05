/***************************************************************************

  gbx_c_task.c

  (c) 2000-2017 Benoît Minisini <g4mba5@gmail.com>

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

#define __GBX_C_TASK_C

#include "gbx_info.h"

#ifndef GBX_INFO

#include <errno.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#include "gb_common.h"
#include "gb_list.h"
#include "gb_file.h"
#include "gb_error.h"
#include "gbx_api.h"
#include "gbx_exec.h"
#include "gbx_string.h"
#include "gbx_signal.h"
#include "gbx_event.h"
#include "gbx_component.h"

#include "gbx_c_file.h"
#include "gbx_c_task.h"

//#define DEBUG_ME 1
#ifdef __CYGWIN__
#define FIONREAD TIOCINQ
#endif

DECLARE_EVENT(EVENT_Read);
DECLARE_EVENT(EVENT_Error);
DECLARE_EVENT(EVENT_Kill);

enum {
	CHILD_OK = 0,
	CHILD_ERROR = 1,
	CHILD_STDOUT = 2,
	CHILD_STDERR = 3,
	CHILD_RETURN = 4
};

static SIGNAL_CALLBACK *_signal_handler = NULL;
static CTASK *_task_list = NULL;
static int _task_count = 0;

//-------------------------------------------------------------------------

static void cleanup_task(CTASK *_object);
static void stop_task(CTASK *_object);
static void close_fd(int *pfd);

static void has_forked(void)
{
	CTASK *task;
	pid_t pid = getpid();
	STREAM *stream;
	
	FILE_init();
	EXEC_debug = FALSE;
	EXEC_task = TRUE;
	if (EXEC_profile)
		DEBUG.Profile.Cancel();
	EXEC_profile = FALSE;
	EXEC_profile_instr = FALSE;

	EXEC_Hook.loop = NULL;
	EXEC_Hook.wait = NULL;
	EXEC_Hook.timer = NULL;
	EXEC_Hook.watch = NULL;
	EXEC_Hook.post = NULL;
	//EXEC_Hook.quit = NULL;
	
	stream = CSTREAM_TO_STREAM(CFILE_out);

	stream->common.eol = 0;
	STREAM_blocking(stream, TRUE);

	SIGNAL_has_forked();
	
	task = _task_list;
	while (task)
	{
		if (task->pid != pid)
		{
			close_fd(&task->fd_out);
			close_fd(&task->fd_err);
		}
		
		task = task->list.next;
	}
}

static void callback_child(int signum, intptr_t data)
{
	CTASK *_object, *next;
	int status;
	
	#if DEBUG_ME
	fprintf(stderr, ">> callback_child\n");
	#endif
	
	_object = _task_list;
	while (_object)
	{
		next = THIS->list.next;
		if (wait4(THIS->pid, &status, WNOHANG, NULL) == THIS->pid)
		{
			THIS->status = status;
			stop_task(THIS);
		}
		_object = next;
	}

	#if DEBUG_ME
	fprintf(stderr, "<< callback_child\n");
	#endif
}

static int get_readable(int fd)
{
	int len;
	
	if (ioctl(fd, FIONREAD, &len) < 0 || len <= 0)
		return 0;
	else
		return len;
}

static bool callback_read(int fd, int type, CTASK *_object)
{
	int len;
	char *data;
	char *p;
	int n;
	
	//fprintf(stderr, "callback_read: %d %p\n", fd, THIS);
	
	len = get_readable(fd);
	if (len == 0)
		return TRUE;
	
	data = STRING_new(NULL, len);
	p = data;
	
	while (len > 0)
	{
		n = read(fd, p, len);
		if (n < 0)
		{
			if (errno == EINTR)
				continue;
			else
				break;
		}
		len -= n;
		p += n;
	}
	
	GB_Raise(THIS, EVENT_Read, 1, GB_T_STRING, data, STRING_length(data) - len);
	
	STRING_free(&data);
	return FALSE;
}


static int callback_error(int fd, int type, CTASK *_object)
{
	char buffer[256];
	int n;

	//fprintf(stderr, "callback_error: %d %p\n", fd, THIS);

	n = read(fd, buffer, sizeof(buffer));

	if (n <= 0)
		return TRUE;

	GB_Raise(THIS, EVENT_Error, 1, GB_T_STRING, buffer, n);
	return FALSE;
}

static bool create_return_directory(void)
{
	static bool mkdir_done = FALSE;

	char buf[PATH_MAX];
	
	if (mkdir_done)
		return FALSE;

	sprintf(buf, RETURN_DIR_PATTERN, getuid(), getpid());
	
	if (mkdir(buf, S_IRWXU) != 0)
	{
		GB_Error("Cannot create task return directory");
		return TRUE;
	}
	
	mkdir_done = TRUE;
	return FALSE;
}

static void init_task(void)
{
	_task_count++;

	if (_task_count > 1)
		return;
	
	_signal_handler = SIGNAL_register(SIGCHLD, callback_child, 0);
}

static void exit_task(void)
{
	_task_count--;

	if (_task_count > 0)
		return;
	
	SIGNAL_unregister(SIGCHLD, _signal_handler);
	_signal_handler = NULL;
}

static void prepare_task(CTASK *_object)
{
	THIS->fd_out = -1;
	THIS->fd_err = -1;
}

static void exit_child(int ret)
{
	FILE_exit();
	_exit(ret);
}

static bool start_task(CTASK *_object)
{
	const char *err = NULL;
	pid_t pid;
	sigset_t sig, old;
	GB_FUNCTION func;
	int fd_out[2], fd_err[2];
	bool has_read, has_error;
	GB_VALUE *ret;
	char buf[PATH_MAX];
	FILE *f;
	
	if (EXEC_task)
		return TRUE;

	if (THIS->stopped)
	{
		cleanup_task(THIS);
		return TRUE;
	}
	
	init_task();

	LIST_insert(&_task_list, THIS, &THIS->list);

	// Create pipes
	
	has_read = GB_CanRaise(THIS, EVENT_Read);
	has_error = GB_CanRaise(THIS, EVENT_Error);
	
	if (has_read && pipe(fd_out) != 0)
		goto __ERROR;

	if (has_error && pipe(fd_err) != 0)
		goto __ERROR;

	COMPONENT_before_fork();

	// Block SIGCHLD

	sigemptyset(&sig);

	sigaddset(&sig, SIGCHLD);
	sigprocmask(SIG_BLOCK, &sig, &old);

	pid = fork();
	if (pid == (-1))
	{
		stop_task(THIS);
		sigprocmask(SIG_SETMASK, &old, NULL);
		goto __ERROR;
	}

	if (pid)
	{
		#if DEBUG_ME
		fprintf(stderr, "start_task: %p %d\n", THIS, pid);
		#endif
		THIS->pid = pid;

		if (has_read)
		{
			close(fd_out[1]);
			THIS->fd_out = fd_out[0];

			GB_Watch(THIS->fd_out, GB_WATCH_READ, (void *)callback_read, (intptr_t)THIS);
		}

		if (has_error)
		{
			close(fd_err[1]);
			THIS->fd_err = fd_err[0];

			fcntl(THIS->fd_err, F_SETFL, fcntl(THIS->fd_err, F_GETFL) | O_NONBLOCK);
			GB_Watch(THIS->fd_err, GB_WATCH_READ, (void *)callback_error, (intptr_t)THIS);
		}

		sigprocmask(SIG_SETMASK, &old, NULL);
	}
	else // child task
	{
		THIS->child = TRUE;
		THIS->pid = getpid();
		
		sigprocmask(SIG_SETMASK, &old, NULL);
		
		if (has_read)
		{
			close(fd_out[0]);

			if (dup2(fd_out[1], STDOUT_FILENO) == -1)
				exit_child(CHILD_STDOUT);
			
			setlinebuf(stdout);
		}
		else
			close(CHILD_STDOUT);

		if (has_error)
		{
			close(fd_err[0]);

			if (dup2(fd_err[1], STDERR_FILENO) == -1)
				exit_child(CHILD_STDERR);
			
			setlinebuf(stderr);
		}
		else
			close(CHILD_STDERR);

		has_forked(); // After the redirection
		
		GB_GetFunction(&func, THIS, "Main", "", NULL);
		
		TRY
		{
			ret = GB_Call(&func, 0, FALSE);
			if (ret->type != GB_T_VOID)
			{
				sprintf(buf, RETURN_FILE_PATTERN, getuid(), getppid(), getpid());
				#if DEBUG_ME
				fprintf(stderr, "serialize to: %s\n", buf);
				#endif
				GB_ReturnConvVariant();
				if (GB_Serialize(buf, ret))
				{
					#if DEBUG_ME
					fprintf(stderr, "gb.task: serialization has failed\n");
					#endif
					exit_child(CHILD_RETURN);
				}
			}
		}
		CATCH
		{
			if (ERROR_current->info.code && ERROR_current->info.code != E_ABORT)
			{
				sprintf(buf, RETURN_FILE_PATTERN, getuid(), getppid(), getpid());
				f = fopen(buf, "w+");
				if (f)
				{
					ERROR_print_at(f, FALSE, FALSE);
					fclose(f);
				}
				
				exit_child(CHILD_ERROR);
			}
		}
		END_TRY
		
		exit_child(CHILD_OK);
	}
	
	return FALSE;

__ERROR:

	// TODO: as the routine is posted, nobody will see the error!
	if (!err)
		err = strerror(errno);
	fprintf(stderr, "gb.task: cannot run task: %s\n", err);
	GB_Error("Cannot run task: &1", err);
	
	return TRUE;
}

static void close_fd(int *pfd)
{
	int fd = *pfd;
	
	if (fd >= 0)
	{
		GB_Watch(fd, GB_WATCH_NONE, NULL, 0);
		close(fd);
		*pfd = -1;
	}
}

static bool get_return_value(CTASK *_object, bool cleanup)
{
	char path[PATH_MAX];
	GB_VALUE value;
	bool fail = FALSE;
	struct stat info;
	char *err = NULL;
	int fd;
	int n;
	
	sprintf(path, RETURN_FILE_PATTERN, getuid(), getpid(), THIS->pid);
	
	if (!cleanup)
	{
		switch (THIS->status)
		{
			case CHILD_OK:
				
				#if DEBUG_ME
				fprintf(stderr,"unserialize from: %s\n", path);
				#endif
				if (!THIS->got_value)
				{
					if (stat(path, &info))
					{
						GB_Error((char *)E_NRETURN);
						return TRUE;
					}
					
					fail = GB_UnSerialize(path, &value);
					if (!fail)
						GB_StoreVariant(&value._variant, &THIS->ret);
					THIS->got_value = TRUE;
				}
				break;
				
			case CHILD_ERROR:
				
				if (stat(path, &info))
				{
					fail = TRUE;
				}
				else
				{
					err = STRING_new_temp(NULL, info.st_size);
					
					fd = open(path, O_RDONLY);
					if (fd < 0)
					{
						fail = TRUE;
						break;
					}
					else
					{
						for(;;)
						{
							n = read(fd, err, info.st_size);
							if (n == info.st_size || errno != EINTR)
								break;
						}
						close(fd);
						
						if (n == info.st_size)
							GB_Error("Task has failed: &1", err);
						else
							fail = TRUE;
					}
				}
				
				if (fail)
					GB_Error("Unable to get task error");
				
				break;
		}
	}

	unlink(path);
	
	return fail;
}

static void cleanup_task(CTASK *_object)
{
	//printf("cleanup task %p\n", THIS); fflush(stdout);

	OBJECT_UNREF(_object);
}

static void stop_task(CTASK *_object)
{
	int len;
	GB_RAISE_HANDLER handler;
	
	#if DEBUG_ME
	fprintf(stderr, "stop_task: %p %d\n", THIS, THIS->pid);
	#endif
	
	THIS->stopped = TRUE;
	
	// Remove task temporary files
	FILE_remove_temp_file_pid(THIS->pid);

	// Flush standard error
	if (THIS->fd_err >= 0)
		while (callback_error(THIS->fd_err, 0, THIS) == 0);

	// Flush standard output
	if (THIS->fd_out >= 0)
	{
		for(;;)
		{
			len = get_readable(THIS->fd_out);
			if (len <= 0)
				break;
			
			if (callback_read(THIS->fd_out, 0, THIS))
				break;
		}
	}

	close_fd(&THIS->fd_out);
	close_fd(&THIS->fd_err);
	
	LIST_remove(&_task_list, THIS, &THIS->list);
	exit_task();

	//printf("Kill event...\n"); fflush(stdout);
	
	if (GB_CanRaise(THIS, EVENT_Kill))
	{
		handler.callback = (GB_CALLBACK)cleanup_task;
		handler.data = (intptr_t)THIS;
	
		GB_RaiseBegin(&handler);
		GB_Raise(THIS, EVENT_Kill, 0);
		GB_RaiseEnd(&handler);
	}

	cleanup_task(THIS);
}

//-------------------------------------------------------------------------

BEGIN_METHOD_VOID(Task_new)

	GB_FUNCTION func;
	
	THIS->ret.type = GB_T_NULL;

	if (EXEC_task)
	{
		GB_Error("A task cannot create other tasks");
		return;
	}
	
	if (create_return_directory())
		return;
	
	if (GB_GetFunction(&func, THIS, "Main", "", NULL))
		return;
	
	if (_task_count > MAX_TASK)
	{
		GB_Error("Too many tasks");
		return;
	}
	
	prepare_task(THIS);
	
	OBJECT_REF(THIS);
	EVENT_post((GB_CALLBACK)start_task, (intptr_t)THIS);
	
END_METHOD

BEGIN_METHOD_VOID(Task_free)

	get_return_value(THIS, TRUE);
	GB_StoreVariant(NULL, &THIS->ret);

END_METHOD

BEGIN_PROPERTY(Task_Handle)

	GB_ReturnInteger(THIS->pid);

END_PROPERTY


BEGIN_METHOD_VOID(Task_Stop)

	if (THIS->pid > 0)
		kill(THIS->pid, SIGKILL);
	else
		THIS->stopped = TRUE;

END_METHOD

static void error_Task_Wait(CTASK *task)
{
	OBJECT_UNREF(task);
}

BEGIN_METHOD_VOID(Task_Wait)

	OBJECT_REF(THIS);
	
	//printf("Task_Wait: %p\n", THIS); fflush(stdout);
	
	ON_ERROR_1(error_Task_Wait, THIS)
	{
		for(;;)
		{
			//printf("GB_Wait\n"); fflush(stdout);
			GB_Wait(0);
			//printf("stopped = %d\n", THIS->stopped); fflush(stdout);
			if (THIS->stopped)
				break;
			//printf("sleep\n"); fflush(stdout);
			sleep(10);
		}
	}
	END_ERROR
	
	OBJECT_UNREF(_object);

END_METHOD

BEGIN_PROPERTY(Task_Value)

	if (!THIS->child && THIS->stopped)
	{
		if (WIFEXITED(THIS->status))
		{
			switch (WEXITSTATUS(THIS->status))
			{
				case CHILD_OK:

					if (get_return_value(THIS, FALSE))
						break;
					GB_ReturnVariant(&THIS->ret);
					return;
					
				case CHILD_STDOUT:
					
					GB_Error("Unable to redirect task standard output");
					return;
					
				case CHILD_STDERR:
					
					GB_Error("Unable to redirect task error output");
					return;
					
				case CHILD_RETURN:
					
					GB_Error("Unable to serialize task return value");
					return;
					
				case CHILD_ERROR:
					
					get_return_value(THIS, FALSE);
					break;
			}
		}
		else if (WIFSIGNALED(THIS->status))
		{
			GB_Error("Task has aborted: &1", strsignal(WTERMSIG(THIS->status)));
			return;
		}
	}
	
	GB_ReturnNull();
	GB_ReturnConvVariant();
	
END_PROPERTY

BEGIN_PROPERTY(Task_Running)

	GB_ReturnBoolean(!THIS->stopped);

END_PROPERTY

//-------------------------------------------------------------------------

#endif

GB_DESC TaskDesc[] =
{
	GB_DECLARE("Task", sizeof(CTASK)), GB_NOT_CREATABLE(),
	
	GB_METHOD("_new", NULL, Task_new, NULL),
	GB_METHOD("_free", NULL, Task_free, NULL),

	GB_PROPERTY_READ("Handle", "i", Task_Handle),
	GB_PROPERTY_READ("Value", "v", Task_Value),
	GB_PROPERTY_READ("Running", "b", Task_Running),

	GB_METHOD("Stop", NULL, Task_Stop, NULL),
	GB_METHOD("Kill", NULL, Task_Stop, NULL),
	GB_METHOD("Wait", NULL, Task_Wait, NULL),

	GB_EVENT("Read", NULL, "(Data)s", &EVENT_Read),
	GB_EVENT("Error", NULL, "(Data)s", &EVENT_Error),
	GB_EVENT("Kill", NULL, NULL, &EVENT_Kill),

	GB_END_DECLARE
};
