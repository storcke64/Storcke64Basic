/***************************************************************************

	gbc_read_temp.h

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

#ifndef __GBC_READ_TEMP_H
#define __GBC_READ_TEMP_H

#ifdef __EVAL_READ_C
#define SYMBOL_TABLE EVAL->table
#else
#define SYMBOL_TABLE comp->class->table
#endif

static bool add_number()
{
	unsigned char car;
	const char *start;
	int index;
	char sign;
	PATTERN last_pattern;
	bool has_digit;

	start = source_ptr;
	car = get_char();

	if (car == '-' || car == '+')
	{
		sign = car;
		car = next_char();
		
		if (car == 'I' || car == 'i')
		{
			car = next_char();
			if (car == 'N' || car == 'n')
			{
				car = next_char();
				if (car == 'F' || car == 'f')
				{
					car = next_char();
					add_pattern(RT_RESERVED, RESERVED_find_word(start, 4));
					return FALSE;
				}
			}
			
			goto NOT_A_NUMBER;
		}
	}
	else
		sign = 0;

	if (car == '&')
	{
		car = toupper(next_char());

		if (car == 'H')
			goto READ_HEXA;
		else if (car == 'X')
			goto READ_BINARY;
		else if (car == 'O')
			goto READ_OCTAL;
		else
		{
			source_ptr--;
			goto READ_HEXA;
		}
	}
	else if (car == '%')
		goto READ_BINARY;
	else if (isdigit(car))
		goto READ_NUMBER;
	else
		goto NOT_A_NUMBER;

READ_BINARY:

	has_digit = FALSE;
	for (;;)
	{
		car = next_char();
		if (car != '0' && car != '1')
			break;
		has_digit = TRUE;
	}
	
	goto END_BINARY_HEXA;

READ_OCTAL:

	has_digit = FALSE;
	for (;;)
	{
		car = next_char();
		if (car < '0' || car > '7')
			break;
		has_digit = TRUE;
	}
	
	goto END_BINARY_HEXA;

READ_HEXA:

	has_digit = FALSE;
	for (;;)
	{
		car = next_char();
		if (!isxdigit(car))
			break;
		has_digit = TRUE;
	}

END_BINARY_HEXA:

	if (!has_digit)
		goto NOT_A_NUMBER;

	if (car == '&')
		car = next_char();
	else if (first_car[car] == GOTO_IDENT)
		goto NOT_A_NUMBER;

	goto END;

READ_NUMBER:

	while (isdigit(car))
		car = next_char();

	if (car == '.')
	{
		do
		{
			car = next_char();
		}
		while (isdigit(car));
	}

	if (toupper(car) == 'E')
	{
		car = next_char();
		if (car == '+' || car == '-')
			car = next_char();

		while (isdigit(car))
			car = next_char();
	}
	else if (toupper(car) == 'I')
	{
		car = next_char();
	}

	goto END;

END:

	last_pattern = get_last_pattern();

	if (sign && !PATTERN_is_null(last_pattern) && (!PATTERN_is_reserved(last_pattern) || PATTERN_is(last_pattern, RS_RBRA) || PATTERN_is(last_pattern, RS_RSQR)))
	{
		add_pattern(RT_RESERVED, RESERVED_find_word(&sign, 1));
		index = TABLE_add_symbol(SYMBOL_TABLE, start + 1, source_ptr - start - 1);
		add_pattern(RT_NUMBER, index);
	}
	else
	{
		index = TABLE_add_symbol(SYMBOL_TABLE, start, source_ptr - start);
		add_pattern(RT_NUMBER, index);
	}
	
	return FALSE;
	
NOT_A_NUMBER:
	
	source_ptr = start;
	return TRUE;
}


static void add_identifier()
{
	unsigned char car;
	const char *start;
	int len;
	int index;
	int type;
	int flag;
	PATTERN last_pattern, last_last_pattern;
	bool not_first;
	bool can_be_reserved;
	bool last_identifier, last_type, last_class, last_pub, last_point, last_excl;
#ifdef __EVAL_READ_C
	bool exist = TRUE;
#endif
	
	last_pattern = get_last_pattern();
	
	if (PATTERN_is_reserved(last_pattern))
	{
		flag = RES_get_ident_flag(PATTERN_index(last_pattern));
		if (flag & RSF_PREV)
		{
			last_last_pattern = get_last_last_pattern();
			if (PATTERN_is_reserved(last_last_pattern))
				flag = RES_get_ident_flag(PATTERN_index(last_last_pattern));
			else
				flag = 0;
		}
	}
	else
		flag = 0;

	type = RT_IDENTIFIER;

	last_class = (flag & RSF_CLASS) != 0;
	last_type = (flag & RSF_AS) != 0;
	last_point = PATTERN_is(last_pattern, RS_PT);
	last_excl = PATTERN_is(last_pattern, RS_EXCL);
	
	start = source_ptr;
	
	for(;;)
	{
		source_ptr++;
		if (!ident_car[get_char()])
			break;
	}
	
	if (!last_point && !last_excl && get_char() == ':')
	{
		for(;;)
		{
			source_ptr++;
			if (!ident_car[get_char()])
				break;
		}
	}
	
	if (get_char_offset(-1) == ':')
		source_ptr--;
	
	len = source_ptr - start;
	
	if (last_type)
	{
		source_ptr--;
		
		for(;;)
		{
			source_ptr++;
			len++;
			car = get_char();
			if (car == '[')
			{
				car = get_char_offset(1);
				if (car == ']')
				{
					source_ptr++;
					len++;
					index = TABLE_add_symbol(SYMBOL_TABLE, start, len - 2);
					continue;
				}
			}
			
			len--;
			break;
		}
	}

	not_first = (flag & RSF_POINT) != 0;

	car = get_char();

	//can_be_reserved = !not_first && TABLE_find_symbol(COMP_res_table, &comp->source[start], len, NULL, &index);
	
	can_be_reserved = !not_first && !last_class;
	
	if (can_be_reserved)
	{
		index = RESERVED_find_word(start, len);
		can_be_reserved = (index >= 0);
	}
	
	if (can_be_reserved)
	{
		static void *jump[] = { 
			&&__OTHERS, &&__ME_NEW_LAST_SUPER, &&__CLASS, &&__STRUCT, &&__SUB_PROCEDURE_FUNCTION, &&__CONST_EXTERN, &&__ENUM, &&__READ, &&__DATATYPE 
		};
		
		last_identifier = (flag & RSF_IDENT) != 0;
		last_pub = (flag & RSF_PUB) != 0;

		goto *jump[RES_get_read_switch(index)];
		
		do
		{
		__ME_NEW_LAST_SUPER:
			can_be_reserved = !last_identifier;
			break;
			
		__CLASS:
			can_be_reserved = canres_car[car] && (_begin_line || PATTERN_is(last_pattern, RS_END));
			break;
			
		__STRUCT:
			can_be_reserved = canres_car[car] && (_begin_line || last_pub || PATTERN_is(last_pattern, RS_AS) || PATTERN_is(last_pattern, RS_END) || PATTERN_is(last_pattern, RS_NEW));
			break;
			
		__SUB_PROCEDURE_FUNCTION:
			can_be_reserved = canres_car[car] && (_begin_line || last_pub || PATTERN_is(last_pattern, RS_END));
			break;
		
		__CONST_EXTERN:
			can_be_reserved = canres_car[car] && (_begin_line || last_pub);
			break;
			
		__ENUM:
			can_be_reserved = canres_car[car] && (_begin_line || last_pub);
			break;
			
		__READ:
			can_be_reserved = canres_car[car] && (!last_identifier || PATTERN_is(last_pattern, RS_PROPERTY));
			break;
		
		__DATATYPE:
			if (car == '[' && get_char_offset(1) == ']')
			{
				len += 2;
				source_ptr += 2;
				can_be_reserved = FALSE;
			}
			else
			{
				if (last_type || PATTERN_is(last_pattern, RS_OPEN))
					can_be_reserved = TRUE;
				else
					can_be_reserved = FALSE;
			}
			break;
			
		__OTHERS:
			if (last_type || last_identifier || (PATTERN_is(last_pattern, RS_LBRA) && car == ')' && PATTERN_is_reserved(get_last_last_pattern())))
				can_be_reserved = FALSE;
			else
				can_be_reserved = canres_car[car];
			break;
		}
		while (0);
	}

	if (can_be_reserved)
	{
		type = RT_RESERVED;
		goto __ADD_PATTERN;
	}
	
	if ((flag == 0) && car != '.' && car != '!')
	{
		index = RESERVED_find_subr(start, len);
		if (index >= 0)
		{
			if (COMP_subr_info[index].min_param == 0 || car == '(')
			{
				type = RT_SUBR;
				
#ifdef __EVAL_READ_C
				if (EVAL->custom)
				{
					GB_FUNCTION func;
					GB_VALUE *ret;

					if (!GB.GetFunction(&func, (void *)GB.GetClass(EVAL->parent), "IsSubr", NULL, NULL))
					{
						GB.Push(1, GB_T_STRING, start, len);
						ret = GB.Call(&func, 1, FALSE);
						if (!ret->_boolean.value)
						{
							exist = TABLE_add_symbol_exist(EVAL->table, start, len, &index);
							type = RT_IDENTIFIER;
						}
					}
				}
#endif
				goto __ADD_PATTERN;
			}
		}
	}

	if (last_type)
		type = RT_CLASS;

#ifdef __EVAL_READ_C
	
	if (!EVAL->analyze && PATTERN_is(last_pattern, RS_EXCL))
	{
		index = TABLE_add_symbol(EVAL->string, start, len);
		type = RT_STRING;
	}
	else
	{
		exist = TABLE_add_symbol_exist(EVAL->table, start, len, &index);
	}

#else
	
	if (flag & RSF_EVENT)
	{
		start--;
		len++;
		*((char *)start) = ':';
	}

	if (last_excl)
	{
		index = TABLE_add_symbol(comp->class->string, start, len);
		type = RT_STRING;
	}
	else
		index = TABLE_add_symbol(comp->class->table, start, len);
	
#endif
	
__ADD_PATTERN:

	add_pattern(type, index);
	
#ifdef __EVAL_READ_C
	
	if (EVAL->custom && !exist)
	{
		GB_FUNCTION func;
		GB_VALUE *ret;

		if (!GB.GetFunction(&func, (void *)GB.GetClass(EVAL->parent), "IsIdentifier", NULL, NULL))
		{
			GB.Push(1, GB_T_STRING, start, len);
			ret = GB.Call(&func, 1, FALSE);
			if (!ret->_boolean.value)
				THROW("Unknown symbol");
		}
	}
	
#endif
}


#endif
