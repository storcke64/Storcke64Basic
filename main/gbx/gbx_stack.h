/***************************************************************************

	gbx_stack.h

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

#ifndef __GBX_STACK_H
#define __GBX_STACK_H

#include "gbx_value.h"
#include "gb_pcode.h"

//#define DEBUG_STACK    1

typedef
	struct {
		void *cp;
		void *fp;
		void *pc;
	}
	STACK_BACKTRACE;

typedef
	struct _stack_context {
		struct _stack_context *next;
		VALUE *bp;        // local variables
		VALUE *pp;        // local parameters
		CLASS *cp;        // current class
		char *op;         // current object
		VALUE *ep;        // error pointer
		FUNCTION *fp;     // current function
		PCODE *pc;        // instruction
		PCODE *ec;        // instruction if error
		PCODE *et;        // TRY save
		VALUE *gp;        // GOSUB stack pointer
		}
	STACK_CONTEXT;

#ifndef __GBX_STACK_C

EXTERN char *STACK_base;
EXTERN size_t STACK_size;
EXTERN char *STACK_limit;
//EXTERN size_t STACK_relocate;

EXTERN uint STACK_frame_count;
EXTERN STACK_CONTEXT *STACK_frame;

EXTERN uintptr_t STACK_process_stack_limit;

EXTERN uint STACK_frame_barrier;

#endif

#define STACK_FOR_EVAL 16

void STACK_init(void);
void STACK_exit(void);

#if DEBUG_STACK
bool STACK_check(int need);
#else

#define STACK_check(_need) \
do { \
	if (((char *)(SP + (_need)) + sizeof(STACK_CONTEXT)) >= STACK_limit) \
		THROW_STACK(); \
	} \
while (0);

#endif

bool STACK_has_error_handler(void);

STACK_BACKTRACE *STACK_get_backtrace(void);
STACK_BACKTRACE *STACK_copy_backtrace(STACK_BACKTRACE *bt);
#define STACK_free_backtrace(_backtrace) FREE(_backtrace)
#define STACK_backtrace_is_end(_bt) ((((intptr_t)((_bt)->cp)) & 1) != 0)
#define STACK_backtrace_set_end(_bt) ((_bt)->cp = (void *)(((intptr_t)((_bt)->cp)) | 1))
#define STACK_backtrace_clear_end(_bt) ((_bt)->cp = (void *)(((intptr_t)((_bt)->cp)) & ~1))


STACK_CONTEXT *STACK_get_frame(uint frame);

#define STACK_get_previous_pc() ((STACK_frame_count == 0) ? NULL : STACK_frame->pc)

#define STACK_get_current() ((STACK_frame_count > 0) ? STACK_frame : NULL)

#define STACK_copy(_dst, _src) *(_dst) = *(_src)

#define STACK_push_frame(_context, _need) \
({ \
	int stack; \
	if ((uintptr_t)&stack < STACK_process_stack_limit) \
		THROW_STACK(); \
	\
	STACK_check(_need); \
	\
	STACK_frame--; \
	\
	STACK_copy(STACK_frame, _context); \
	\
	STACK_frame_count++; \
	STACK_limit -= sizeof(STACK_CONTEXT); \
})

#define STACK_pop_frame(_context) \
({ \
	STACK_copy(_context, STACK_frame); \
	STACK_frame++; \
	STACK_frame_count--; \
	STACK_limit += sizeof(STACK_CONTEXT); \
})

#define STACK_enable_for_eval() STACK_limit += STACK_FOR_EVAL * sizeof(VALUE)
#define STACK_disable_for_eval() STACK_limit -= STACK_FOR_EVAL * sizeof(VALUE)

#define STACK_push_barrier() \
({ \
	int old = STACK_frame_barrier; \
	STACK_frame_barrier = STACK_frame_count + 1; \
	old; \
})

#define STACK_pop_barrier(_old) (STACK_frame_barrier = (_old))

#endif
