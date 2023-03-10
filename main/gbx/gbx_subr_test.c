/***************************************************************************

  gbx_subr_test.c

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

#include "gb_common.h"
#include <math.h>

#include "gbx_value.h"
#include "gbx_subr.h"
#include "gbx_date.h"
#include "gbx_object.h"
#include "gbx_math.h"
#include "gbx_compare.h"

#define STT_NAME   SUBR_case
#define STT_TEST   ==
#define STT_CASE

#include "gbx_subr_test_temp.h"


void SUBR_bit(ushort code)
{
  static void *jump[16] = {
    &&__ERROR, &&__BCLR, &&__BSET, &&__BTST, &&__BCHG, &&__ASL, &&__ASR, &&__ROL,
    &&__ROR, &&__LSL, &&__LSR, &&__ERROR, &&__ERROR, &&__ERROR, &&__ERROR, &&__ERROR
    };
    
	static int nbits[6] = { 0, 0, 8, 16, 32, 64 };

  int64_t val;
  int bit;
  TYPE type;
  int n;
  bool variant;

  SUBR_ENTER_PARAM(2);

  type = PARAM->type;

	variant = TYPE_is_variant(type);
	if (variant)
		type = PARAM->_variant.vtype;
	
	if (type <= T_BOOLEAN || type > T_LONG)
	  THROW(E_TYPE, "Number", TYPE_get_name(type));
	
	VALUE_conv(PARAM, T_LONG);
  val = PARAM->_long.value;

	n = nbits[type];

  bit = SUBR_get_integer(&PARAM[1]);

  if ((bit < 0) || (bit >= n))
    THROW(E_ARG);

  RETURN->type = type;

	goto *jump[code & 0xF];

__BCLR:

	val &= ~(1ULL << bit);
	goto __END;

__BSET:

	val |= (1ULL << bit);
	goto __END;

__BTST:

	RETURN->type = T_BOOLEAN;
	RETURN->_boolean.value = (val & (1ULL << bit)) ? (-1) : 0;
  goto __LEAVE;

__BCHG:

	val ^= (1ULL << bit);
  goto __END;

__ASL:

	{
	  static void *asl_jump[6] = { &&__ERROR, &&__ERROR, &&__ASL_BYTE, &&__ASL_SHORT, &&__ASL_INTEGER, &&__ASL_LONG };

		goto *asl_jump[type];

	__ASL_BYTE:
		val = ((unsigned char)val << bit);
		goto __END_BYTE;

	__ASL_SHORT:
		val = (((short)val << bit) & 0x7FFF) | (((short)val) & 0x8000);
		goto __END_SHORT;

	__ASL_INTEGER:
		val = (((int)val << bit) & 0x7FFFFFFF) | (((int)val) & 0x80000000);
		goto __END_INTEGER;

	__ASL_LONG:
		val = ((val << bit) & 0x7FFFFFFFFFFFFFFFLL) | (val & 0x8000000000000000LL);
		goto __END_LONG;
	}
	
__ASR:

	{
	  static void *asr_jump[6] = { &&__ERROR, &&__ERROR, &&__ASR_BYTE, &&__ASR_SHORT, &&__ASR_INTEGER, &&__ASR_LONG };

		goto *asr_jump[type];

	__ASR_BYTE:
		val = ((unsigned char)val >> bit);
		goto __END_BYTE;

	__ASR_SHORT:
		val = (((short)val >> bit) & 0x7FFF) | (((short)val) & 0x8000);
		goto __END_SHORT;

	__ASR_INTEGER:
		val = (((int)val >> bit) & 0x7FFFFFFF) | (((int)val) & 0x80000000);
		goto __END_INTEGER;

	__ASR_LONG:
		val = ((val >> bit) & 0x7FFFFFFFFFFFFFFFLL) | (val & 0x8000000000000000LL);
		goto __END_LONG;
	}
	
__ROL:

	{
	  static void *rol_jump[6] = { &&__ERROR, &&__ERROR, &&__ROL_BYTE, &&__ROL_SHORT, &&__ROL_INTEGER, &&__ROL_LONG };
	  
	  goto *rol_jump[type];
	  
	__ROL_BYTE:
		val = (val << bit) | (val >> (8 - bit));
		goto __END_BYTE;
	
	__ROL_SHORT:
		val = ((ushort)val << bit) | ((ushort)val >> (16 - bit));
		goto __END_SHORT;
	
	__ROL_INTEGER:
		val = ((uint)val << bit) | ((uint)val >> (32 - bit));
		goto __END_INTEGER;
	
	__ROL_LONG:
		val = ((uint64_t)val << bit) | ((uint64_t)val >> (64 - bit));
		goto __END_LONG;
	}

__ROR:

	{
	  static void *ror_jump[6] = { &&__ERROR, &&__ERROR, &&__ROR_BYTE, &&__ROR_SHORT, &&__ROR_INTEGER, &&__ROR_LONG };
	  
	  goto *ror_jump[type];
	  
	__ROR_BYTE:
		val = (val >> bit) | (val << (8 - bit));
		goto __END_BYTE;
	
	__ROR_SHORT:
		val = ((ushort)val >> bit) | ((ushort)val << (16 - bit));
		goto __END_SHORT;
	
	__ROR_INTEGER:
		val = ((uint)val >> bit) | ((uint)val << (32 - bit));
		goto __END_INTEGER;
	
	__ROR_LONG:
		val = ((uint64_t)val >> bit) | ((uint64_t)val << (64 - bit));
		goto __END_LONG;
	}

__LSL:

	{
	  static void *lsl_jump[6] = { &&__ERROR, &&__ERROR, &&__LSL_BYTE, &&__LSL_SHORT, &&__LSL_INTEGER, &&__LSL_LONG };

		goto *lsl_jump[type];

	__LSL_BYTE:
		val = ((unsigned char)val << bit);
		goto __END_BYTE;

	__LSL_SHORT:
		val = ((unsigned short)val << bit);
		goto __END_SHORT;

	__LSL_INTEGER:
		val = ((unsigned int)val << bit);
		goto __END_INTEGER;

	__LSL_LONG:
		val = ((uint64_t)val << bit);
		goto __END_LONG;
	}
	
__LSR:

	{
	  static void *lsr_jump[6] = { &&__ERROR, &&__ERROR, &&__LSR_BYTE, &&__LSR_SHORT, &&__LSR_INTEGER, &&__LSR_LONG };

		goto *lsr_jump[type];

	__LSR_BYTE:
		val = ((unsigned char)val >> bit);
		goto __END_BYTE;

	__LSR_SHORT:
		val = ((unsigned short)val >> bit);
		goto __END_SHORT;

	__LSR_INTEGER:
		val = ((unsigned int)val >> bit);
		goto __END_INTEGER;

	__LSR_LONG:
		val = ((uint64_t)val >> bit);
		goto __END_LONG;
	}

__ERROR:

	THROW_ILLEGAL();

__END:

	{
  	static void *end_jump[6] = { &&__ERROR, &&__ERROR, &&__END_BYTE, &&__END_SHORT, &&__END_INTEGER, &&__END_LONG };
  	
  	goto *end_jump[type];
  	
	__END_BYTE:
		RETURN->_integer.value = (unsigned int)val & 0xFF;
		goto __END_VARIANT;
  	
	__END_SHORT:
		RETURN->_integer.value = (int)(short)val;
		goto __END_VARIANT;
	
	__END_INTEGER:
		RETURN->_integer.value = (int)val;
		goto __END_VARIANT;
  	
	__END_LONG:
		RETURN->_long.value = val;
		goto __END_VARIANT;
	}
	
__END_VARIANT:

	if (variant)
		VALUE_conv_variant(RETURN);

__LEAVE:

  SUBR_LEAVE();
}


void SUBR_if(ushort code)
{
	int i;
	unsigned char test;
	TYPE type;
	
  SUBR_ENTER_PARAM(3);

  VALUE_conv_boolean(PARAM);
	i = PARAM->_boolean.value ? 1 : 2;
	
	test = code & 0x1F;

	if (!test)
	{
		type = PARAM[1].type;
		if (PARAM[2].type == type && type <= T_VARIANT)
		{
			*PC |= 0x1F;
		}
		else
		{
			type = SUBR_check_good_type(&PARAM[1], 2);
			if (TYPE_is_object(type))
				type = T_OBJECT;
			*PC |= (unsigned char)type;
			
			VALUE_conv(&PARAM[i], type);
		}
	}
	else if (test != 0x1F)
	{
		VALUE_conv(&PARAM[i], (TYPE)test);
	}

	*PARAM = PARAM[i];
	RELEASE(&PARAM[3 - i]);
	SP -= 2;
}


void SUBR_choose(ushort code)
{
  int val;

  SUBR_ENTER();

  VALUE_conv_integer(PARAM);
  val = PARAM->_integer.value;

  if (val >= 1 && val < NPARAM)
  {
    VALUE_conv_variant(&PARAM[val]);
    *RETURN = PARAM[val];
  }
  else
  {
    RETURN->type = T_VARIANT;
    RETURN->_variant.vtype = T_NULL;
  }

  SUBR_LEAVE();
}


void SUBR_near(void)
{
  int result;

  SUBR_ENTER_PARAM(2);

  VALUE_conv_string(&PARAM[0]);
  VALUE_conv_string(&PARAM[1]);

  //result = STRING_comp_value_ignore_case(&PARAM[0], &PARAM[1]) ? -1 : 0;
  result = STRING_equal_ignore_case(PARAM[0]._string.addr + PARAM[0]._string.start, PARAM[0]._string.len, PARAM[1]._string.addr + PARAM[1]._string.start, PARAM[1]._string.len) ? -1 : 0;

  RELEASE_STRING(&PARAM[0]);
  RELEASE_STRING(&PARAM[1]);

  PARAM->type = T_BOOLEAN;
  PARAM->_boolean.value = result;

  SP--;
}


void SUBR_strcomp(ushort code)
{
  int mode = GB_COMP_BINARY;
  /*char *s1, *s2;
  int l1, l2;
  int ret;*/

  SUBR_ENTER();

  VALUE_conv_string(&PARAM[0]);
  VALUE_conv_string(&PARAM[1]);

  if (NPARAM == 3)
    mode = SUBR_get_integer(&PARAM[2]);

	RETURN->_integer.type = T_INTEGER;
	RETURN->_integer.value = (*COMPARE_get_string_func(mode))(PARAM[0]._string.addr + PARAM[0]._string.start, PARAM[0]._string.len, PARAM[1]._string.addr + PARAM[1]._string.start, PARAM[1]._string.len, mode & GB_COMP_NOCASE, FALSE);

  SUBR_LEAVE();
}


void SUBR_is(ushort code)
{
	VALUE *P1 = SP - 2;
	VALUE *P2 = SP - 1;
	void *object;
	CLASS *klass;
	bool res;

	VALUE_conv(P1, T_OBJECT);
	object = P1->_object.object;
	klass = P2->_class.class;

	if (!object)
		res = FALSE;
	else
		res = (OBJECT_class(object) == klass || CLASS_inherits(OBJECT_class(object), klass));

	OBJECT_UNREF(object);

	P1->type = T_BOOLEAN;
	P1->_boolean.value = -(res ^ (code & 1));
	SP--;
}


void SUBR_min_max(ushort code)
{
  static void *jump[] = {
    &&__VARIANT, &&__MIN_BOOLEAN, &&__MIN_BYTE, &&__MIN_SHORT, &&__MIN_INTEGER, &&__MIN_LONG, &&__MIN_SINGLE, &&__MIN_FLOAT, 
		&&__MIN_DATE, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    &&__VARIANT, &&__MAX_BOOLEAN, &&__MAX_BYTE, &&__MAX_SHORT, &&__MAX_INTEGER, &&__MAX_LONG, &&__MAX_SINGLE, &&__MAX_FLOAT, 
		&&__MAX_DATE, NULL, NULL, NULL, NULL, NULL, NULL, NULL
    };

  TYPE type;
  VALUE *P1, *P2;
  void *jump_end;

  P1 = SP - 2;
  P2 = P1 + 1;

  jump_end = &&__END;
  type = code & 0x0F;
  goto *jump[code & 0x1F];

__MIN_BOOLEAN:
__MIN_BYTE:
__MIN_SHORT:
__MIN_INTEGER:

	P1->type = type;

	if (P2->_integer.value < P1->_integer.value)
		P1->_integer.value = P2->_integer.value;

	goto *jump_end;

__MAX_BOOLEAN:
__MAX_BYTE:
__MAX_SHORT:
__MAX_INTEGER:

	P1->type = type;

	if (P2->_integer.value > P1->_integer.value)
		P1->_integer.value = P2->_integer.value;
	
	goto *jump_end;

__MIN_LONG:

  VALUE_conv(P1, T_LONG);
  VALUE_conv(P2, T_LONG);

	if (P2->_long.value < P1->_long.value)
		P1->_long.value = P2->_long.value;
	
	goto *jump_end;

__MAX_LONG:

  VALUE_conv(P1, T_LONG);
  VALUE_conv(P2, T_LONG);

	if (P2->_long.value > P1->_long.value)
		P1->_long.value = P2->_long.value;
	
	goto *jump_end;

__MIN_SINGLE:

  VALUE_conv(P1, T_SINGLE);
  VALUE_conv(P2, T_SINGLE);

	if (P2->_single.value < P1->_single.value)
		P1->_single.value = P2->_single.value;
	
	goto *jump_end;

__MAX_SINGLE:

  VALUE_conv(P1, T_SINGLE);
  VALUE_conv(P2, T_SINGLE);

	if (P2->_single.value > P1->_single.value)
		P1->_single.value = P2->_single.value;
	
	goto *jump_end;

__MIN_FLOAT:

  VALUE_conv_float(P1);
  VALUE_conv_float(P2);

	if (P2->_float.value < P1->_float.value)
		P1->_float.value = P2->_float.value;
	
	goto *jump_end;

__MAX_FLOAT:

  VALUE_conv_float(P1);
  VALUE_conv_float(P2);

	if (P2->_float.value > P1->_float.value)
		P1->_float.value = P2->_float.value;
	
	goto *jump_end;

__MIN_DATE:

  VALUE_conv(P1, T_DATE);
  VALUE_conv(P2, T_DATE);

	if (DATE_comp_value(P1, P2) == 1)
	{
		P1->_date.date = P2->_date.date;
		P1->_date.time = P2->_date.time;
	}

	goto *jump_end;

__MAX_DATE:

  VALUE_conv(P1, T_DATE);
  VALUE_conv(P2, T_DATE);

	if (DATE_comp_value(P1, P2) == -1)
	{
		P1->_date.date = P2->_date.date;
		P1->_date.time = P2->_date.time;
	}

	goto *jump_end;

__VARIANT:

  type = Max(P1->type, P2->type);

  if (TYPE_is_number_date(type))
  {
		code = type + ((code >> 8) == CODE_MAX) * 16;
    *PC |= code;
    goto *jump[code];
  }

  if (TYPE_is_variant(P1->type))
    VARIANT_undo(P1);

  if (TYPE_is_variant(P2->type))
    VARIANT_undo(P2);

  type = Max(P1->type, P2->type);
  
  if (TYPE_is_number_date(type))
  {
    jump_end = &&__VARIANT_END;
		code = type + ((code >> 8) == CODE_MAX) * 16;
    goto *jump[code];
  }

  goto __ERROR;

__ERROR:

  THROW(E_TYPE, "Number or date", TYPE_get_name(type));

__VARIANT_END:

  VALUE_conv_variant(P1);

__END:

  SP--;
}
