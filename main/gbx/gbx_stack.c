/***************************************************************************

  gbx_stack.c

  (c) Benoît Minisini <g4mba5@gmail.com>

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

#define __GBX_STACK_C

#include <sys/resource.h>
#include <sys/mman.h>

#include "gb_common.h"
#include "gb_error.h"
#include "gb_alloc.h"
#include "gbx_exec.h"
#include "gb_error.h"
#include "gbx_string.h"
#include "gbx_stack.h"

char *STACK_base = NULL;
size_t STACK_size;
char *STACK_limit = NULL;
STACK_CONTEXT *STACK_frame;
uint STACK_frame_count;
uint STACK_frame_barrier;

uintptr_t STACK_process_stack_limit;

void STACK_init(void)
{
	int stack;
	struct rlimit limit;
	uintptr_t max;
	
	// Get the maximum stack size allowed
	if (getrlimit(RLIMIT_STACK, &limit))
		ERROR_panic("Cannot get stack size limit");
	
	if (limit.rlim_cur == RLIM_INFINITY)
		max = 64 << 20; // 64 Mb if there is no limit.
	else
		max = (uintptr_t)limit.rlim_cur;
	
	STACK_size = max - sizeof(VALUE) * 256; // some security
	#if DEBUG_STACK
		fprintf(stderr, "STACK_size = %ld\n", STACK_size);
	#endif
	
	STACK_base = mmap(NULL, STACK_size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANON, -1, 0);

	//fprintf(stderr, "Stack = %p %ld\n", STACK_base, STACK_size);

	STACK_process_stack_limit = (uintptr_t)&stack - max + 65536;
	
  STACK_limit = STACK_base + STACK_size;
  STACK_frame = (STACK_CONTEXT *)STACK_limit;
  STACK_frame_count = 0;
	STACK_frame_barrier = 0;
	STACK_limit -= STACK_FOR_EVAL * sizeof(VALUE);

  SP = (VALUE *)STACK_base;
}


void STACK_exit(void)
{
  if (STACK_base)
	{
		munmap(STACK_base, STACK_size);
    STACK_base = NULL;
	}
}

#if DEBUG_STACK
bool STACK_check(int need)
{
  static VALUE *old = NULL;

	fprintf(stderr, "STACK_check: SP = %d need = %d limit = %d\n", (int)(((char *)SP - STACK_base) / sizeof(VALUE)), need, (int)((STACK_limit - STACK_base) / sizeof(VALUE)));
	
  if (SP > old)
  {
    fprintf(stderr, "**** STACK_check: -> %ld bytes\n", ((char *)SP - STACK_base));
    old = SP;
  }
	
  if (((char *)(SP + need) + sizeof(STACK_CONTEXT)) >= STACK_limit)
	{
    THROW_STACK();
		return TRUE;
	}
	else
		return FALSE;
}
#endif

bool STACK_has_error_handler(void)
{
  uint i;
	STACK_CONTEXT *sc;
	uint b = STACK_frame_count - STACK_frame_barrier;

  for (i = 0; i < b; i++)
	{
		sc = &STACK_frame[i];
		if (sc->ec || sc->ep)
			return TRUE;
	}
	
	return FALSE;
}

STACK_CONTEXT *STACK_get_frame(uint frame)
{
	if (frame >= 0 && frame < STACK_frame_count)
		return &STACK_frame[frame];
	else
		return NULL;
}

STACK_BACKTRACE *STACK_get_backtrace(void)
{
	STACK_BACKTRACE *bt, *pbt;
	uint i;
	
	if (STACK_frame_count == 0)
		return NULL;
	
	ALLOC(&bt, sizeof(STACK_BACKTRACE) * (1 + STACK_frame_count));
	
	bt->cp = CP;
	bt->fp = FP;
	bt->pc = PC;
	
	for (i = 0, pbt = &bt[1]; i < STACK_frame_count; i++, pbt++)
	{
		pbt->cp = STACK_frame[i].cp;
		pbt->fp = STACK_frame[i].fp;
		pbt->pc = STACK_frame[i].pc;
	}
	
	// Mark the end of the backtrace
	pbt--;
	STACK_backtrace_set_end(pbt);
	 
	return bt;
}

STACK_BACKTRACE *STACK_copy_backtrace(STACK_BACKTRACE *bt)
{
	STACK_BACKTRACE *copy;
	int i, n;
	
	for (i = 0;; i++)
	{
		if (STACK_backtrace_is_end(&bt[i]))
		{
			n = i + 1;
			break;
		}
	}
	
	ALLOC(&copy, sizeof(STACK_BACKTRACE) * n);
	
	for (i = 0; i <n; i++)
		copy[i] = bt[i];
	
	return copy;
}

