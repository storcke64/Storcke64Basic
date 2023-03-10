/***************************************************************************

  gbc_compile.h

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

#ifndef _COMPILE_H
#define _COMPILE_H

#include <sys/stat.h>

#include "gb_array.h"
#include "gb_buffer.h"
#include "gb_table.h"
#include "gb_alloc.h"

#include "gb_limit.h"
#include "gb_reserved.h"
#include "gbc_read.h"
#include "gbc_form.h"

#include "gbc_class.h"

enum {
	MSG_ERROR = 0,
	MSG_WARNING = 1
};

enum {
	JOB_STEP_NONE = 0,
	JOB_STEP_READ = 1,
	JOB_STEP_CODE = 2,
	JOB_STEP_TREE = 3,
	JOB_STEP_OUTPUT = 4
};

typedef
	struct {
		char *name;                        // source file name
		int line;                          // current line number
		int first_line;                    // first line to compile
		int max_line;                      // maximum line number
		char *source;                      // source file contents
		unsigned step : 3;                 // compiler step (JOB_STEP_*)
		unsigned debug : 1;                // if debugging information must be generated
		unsigned trans : 1;                // if translation files must be generated
		unsigned is_module : 1;            // if the source file is a module
		unsigned is_form : 1;              // if the source is a class form
		unsigned is_test : 1;              // if the source is a test module
		unsigned declared : 1;             // ?
		unsigned nobreak : 1;              // no breakpoint
		unsigned exported : 1;             // there are some exported class
		unsigned swap : 1;                 // endianness must be swapped
		unsigned public_module : 1;        // modules symbols are public by default
		unsigned trans_error : 1;          // display error messages in a translatable form
		unsigned no_old_read_syntax : 1;   // do not compile the old read syntax
		unsigned exec : 1;                 // we are compiling for an executable
		unsigned warnings : 1;             // if warnings must be printed
		unsigned check_prefix : 1;         // check if variable prefix matches its datatype
		unsigned _reserved : 14;           // reserved
		char *output;                      // output file
		PATTERN *pattern;                  // lexical analyze
		int pattern_count;                 // number of patterns
		int *pattern_pos;                  // position of a pattern in its line
		PATTERN *current;                  // current pattern
		PATTERN *end;                      // last pattern
		FUNCTION *func;                    // current function being compiled
		CLASS *class;                      // current class being compiled
		char *form;                        // form file name
		const FORM_FAMILY *family;         // form file family
		char *tname;                       // translation file name
		int default_library;               // default library name for extern declarations
		const char **help;                 // help comments array
		int help_first_line;               // line of the first help comment
		char *hname;                       // help file name
		PATTERN *column;                   // where to search the current column
		}
	COMPILE;

#ifndef __GBC_COMPILE_C

EXTERN bool COMP_verbose;
EXTERN COMPILE COMP_current;
EXTERN char *COMP_root;
EXTERN char *COMP_dir;
EXTERN char *COMP_project;
EXTERN char *COMP_project_name;
EXTERN char *COMP_info_path;
EXTERN FORM_FAMILY COMP_form_families[];
EXTERN uint COMP_version;
EXTERN char *COMP_default_namespace;
EXTERN char *COMP_classes;
EXTERN bool COMP_do_not_lock;

#endif

#define JOB (&COMP_current)

void COMPILE_init(void);
void COMPILE_load(void);
void COMPILE_exit(bool can_dump_count);
void COMPILE_begin(const char *file, bool trans, bool debug);
void COMPILE_alloc();
void COMPILE_free();
void COMPILE_end(void);
void COMPILE_export_class(char *name);
void COMPILE_add_class(const char *name, int len);
void COMPILE_end_class(void);
void COMPILE_print(int type, int line, const char *msg, ...);
void COMPILE_create_file(FILE **fw, const char *file);
void COMPILE_add_component(const char *name);

int COMPILE_lock_file(const char *name);
void COMPILE_unlock_file(int fd);
void COMPILE_remove_lock(const char *name);

#define COMPILE_get_column(_pattern) (JOB->pattern_pos[(_pattern) - JOB->pattern])

#define COMPILE_enum_class_start(_name, _len) \
({ \
	_name = COMP_classes; \
	_len = *_name; \
	_name++; \
})

#define COMPILE_enum_class_next(_name, _len) \
({ \
	_name += _name[-1]; \
	_len = *_name; \
	_name++; \
})

#endif
