/***************************************************************************

  main.cpp

  The evaluator plug-in

  (c) 2000-2007 Benoit Minisini <gambas@freesurf.fr>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#define __MAIN_C

#include "gb_common.h"
#include "gambas.h"

#include "eval.h"
#include "eval_analyze.h"
#include "main.h"

#include "CExpression.h"
#include "CSystem.h"

/*#define DEBUG*/


GB_INTERFACE GB EXPORT;

void *GB_EVAL_2[] EXPORT = {

  (void *)EVAL_INTERFACE_VERSION,
  (void *)EVAL_analyze,
  (void *)EVAL_new,
  (void *)EVAL_compile,
  (void *)EVAL_expression,
  (void *)EVAL_free,

  NULL
  };


GB_DESC *GB_CLASSES[] EXPORT =
{
  CExpressionDesc,
  CSystemDesc,
  NULL
};


int EXPORT GB_INIT(void)
{
  EVAL_init();

  return FALSE;
}

void EXPORT GB_EXIT()
{
  EVAL_exit();
}


