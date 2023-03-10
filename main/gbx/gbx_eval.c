/***************************************************************************

	gbx_eval.c

	(c) 2000-2017 Benoît Minisini <benoit.minisini@gambas-basic.org>

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

#define __GBX_EVAL_C

#include "gb_common.h"

#include "gb_array.h"
#include "gbx_string.h"
#include "gbx_class.h"
#include "gbx_exec.h"
#include "gbx_debug.h"
#include "gbx_eval.h"

#include "gbx_c_collection.h"
#include "gbx_api.h"

bool EVAL_debug = FALSE;

static EXPRESSION *EVAL;

static void EVAL_enter()
{
	STACK_push_frame(&EXEC_current, EVAL->func.stack_usage);

	BP = SP;
	PP = SP;
	FP = &EVAL->func;
	PC = EVAL->func.code;

	OP = NULL;
	CP = &EVAL->exec_class;
	EXEC_check_bytecode();

	//AP = ARCH_from_class(CP);

	EP = NULL;
	EC = NULL;
	GP = NULL;

	RP->type = T_VOID;

	PROFILE_ENTER_FUNCTION();
}

static void error_EVAL_exec(void)
{
	PROFILE_LEAVE_FUNCTION();
	STACK_pop_frame(&EXEC_current);
}

static void EVAL_exec()
{
	// We need to push a void frame, because EXEC_leave looks at *PC to know if a return value is expected
	STACK_push_frame(&EXEC_current, 0);

	PC = NULL;
	GP = NULL;

	ON_ERROR(error_EVAL_exec)
	{
		EVAL_enter();
	}
	END_ERROR

	EXEC_function_loop();
	EXEC_move_ret_to_temp();
}

bool EVAL_expression(EXPRESSION *expr, EVAL_FUNCTION func)
{
	int i;
	EVAL_SYMBOL *sym;
	bool debug;
	bool error;
	int nvar;
	EXPRESSION *save_eval;
	/*HASH_TABLE *hash_table;
	char *name;
	CCOL_ENUM enum_state;*/

	save_eval = EVAL;
	EVAL = expr;

	#ifdef DEBUG
	fprintf(stderr, "EVAL: %s\n", EVAL->source);
	#endif

	STACK_enable_for_eval();
	debug = EXEC_debug;
	EXEC_debug = FALSE;
	error = FALSE;

	nvar = EVAL->nvar;

	STACK_check(nvar);

	if (expr->custom)
		nvar--;

	for (i = 0; i < nvar; i++)
	{
		SP->type = T_VARIANT;
		SP->_variant.vtype = T_NULL;
		SP++;

		sym = (EVAL_SYMBOL *)TABLE_get_symbol(EVAL->table, EVAL->var[EVAL->nvar - i - 1]);
		if ((*func)(sym->sym.name, sym->sym.len, (GB_VARIANT *)&SP[-1]))
		{
			GB_Error("Unknown symbol");
			error = TRUE;
			goto __LEAVE;
		}
		
		BORROW(SP - 1);
	}

	if (expr->custom)
	{
		SP->type = T_OBJECT;
		SP->_object.object = expr->parent;
		OBJECT_REF_CHECK(expr->parent);
		SP++;
	}

	TRY
	{
		EVAL_exec();
	}
	CATCH
	{
		error = TRUE;
	}
	END_TRY

__LEAVE:
	
	STACK_disable_for_eval();
	EXEC_debug = debug;
	EVAL = save_eval;
	return error;
}
