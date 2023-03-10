/***************************************************************************

	gbc_trans_common.h

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

#ifndef __GBC_TRANS_COMMMON_H
#define __GBC_TRANS_COMMMON_H

typedef
	struct {
		TYPE type;
		int index;
		PATTERN *optional;
		short value;
		unsigned ignore : 1;
		unsigned byref : 1;
		}
	TRANS_PARAM;

typedef
	PATTERN TRANS_TREE;

typedef
	struct {
		int type;
		int ival;
		int64_t lval;
		double dval;
		bool complex;
		}
	TRANS_NUMBER;

typedef
	struct {
		TYPE type;
		int ndim;
		int dim[MAX_ARRAY_DIM];
		}
	TRANS_ARRAY;

typedef
	struct {
		int index;                  // index in symbol table
		TYPE type;                  // data type
		int value;                  // value or pattern index
		TRANS_ARRAY array;          // array dimensions
		PATTERN *init;              // initialization code
		int64_t lvalue;             // The value of a LONG constant
		unsigned is_new : 1;        // if something must be instanciated
		unsigned is_integer : 1;    // if the constant is an integer
		unsigned is_embedded : 1;   // if it is an embedded array
		unsigned no_warning : 1;    // The symbol name is between braces
		}
	TRANS_DECL;

typedef
	struct {
		int index;
		TYPE type;
		short nparam;
		unsigned vararg : 1;
		unsigned no_warning : 1;    // The symbol name is between braces
		unsigned fast : 1;
		unsigned unsafe : 1;
		unsigned _reserved : 12;
		TRANS_PARAM param[MAX_PARAM_FUNC];
		PATTERN *start;
		int line;
		uint64_t byref;
		}
	TRANS_FUNC;

typedef
	struct {
		int index;
		TYPE type;
		short nparam;
		short _reserved;
		TRANS_PARAM param[MAX_PARAM_FUNC];
		}
	TRANS_EVENT;

typedef
	struct {
		int index;
		TYPE type;
		short nparam;
		unsigned vararg : 1;
		unsigned no_warning : 1;    // The symbol name is between braces
		unsigned _reserved : 14;
		TRANS_PARAM param[MAX_PARAM_FUNC];
		int library;
		int alias;
		}
	TRANS_EXTERN;

typedef
	struct {
		int index;
		TYPE type;
		int line;
		int comment;
		int synonymous[3];
		int use;
		unsigned nsynonymous : 3;
		unsigned read_only : 1;
		unsigned write_only : 1;
		}
	TRANS_PROPERTY;

typedef
	struct {
		int type;
		int value;
		int state;
		short local;
		short id;
		short loop_var;
		short _reserved[3];
		ushort *pos;
		ushort *pos_break;
		ushort *pos_continue;
		}
	TRANS_CTRL;

typedef
	struct {
		int index;
		int line;
		ushort pos;
		short ctrl_id;
		unsigned gosub : 1;
		unsigned on_goto : 1;
		unsigned _reserved : 30;
		}
	TRANS_GOTO;

typedef
	struct {
		int index;
		ushort pos;
		short ctrl_id;
		}
	TRANS_LABEL;

typedef
	struct {
		RESERVED_ID id;
		void (*func)();
		bool no_try;
		}
	TRANS_STATEMENT;

#endif

