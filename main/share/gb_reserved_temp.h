/***************************************************************************

  gb_reserved_temp.h

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
#include "gb_common_case.h"
#include "gb_pcode.h"
#include "gb_type_common.h"
#include "gb_reserved.h"

/* If this file is modified, don't forget to update GAMBAS_PCODE_VERSION in acinclude.m4 if needed */

#include "gb_reserved_keyword.h"

int SUBR_VarPtr;
int SUBR_IsMissing;
int SUBR_Mid;
int SUBR_MidS;
int SUBR_SizeOf;

static uchar _operator_table[256] = { 0 };

static int get_index(const char *subr_name)
{
	return RESERVED_find_subr(subr_name, strlen(subr_name));
}

void RESERVED_init(void)
{
	COMP_INFO *info;
	SUBR_INFO *subr;
	int len;
	int i;
	
	/* Reserved words symbol table */
	
	//TABLE_create(&COMP_res_table, 0, TF_IGNORE_CASE);
	for (info = &COMP_res_info[0], i = 0; info->name; info++, i++)
	{
		len = strlen(info->name);
		if (len == 1)
			_operator_table[(uint)*info->name] = i;
		
		//TABLE_add_symbol(COMP_res_table, info->name, len, &index);
	}
	
	/* Subroutines table */

	//TABLE_create(&COMP_subr_table, 0, TF_IGNORE_CASE);
	for (subr = &COMP_subr_info[0]; subr->name; subr++)
	{
		if (subr->max_param == 0)
			subr->max_param = subr->min_param;

		//TABLE_add_symbol(COMP_subr_table, subr->name, strlen(subr->name), &index);
	}

	SUBR_VarPtr = get_index("VarPtr");
	SUBR_IsMissing = get_index("IsMissing");
	SUBR_Mid = get_index("Mid");
	SUBR_MidS = get_index("Mid$");
	SUBR_SizeOf = get_index("SizeOf");
}


void RESERVED_exit(void)
{
	//TABLE_delete(&COMP_res_table);
	//TABLE_delete(&COMP_subr_table);
}


SUBR_INFO *SUBR_get(const char *subr_name)
{
	int index = get_index(subr_name);

	if (index == NO_SYMBOL)
		return NULL;
	else
		return &COMP_subr_info[index];
}


SUBR_INFO *SUBR_get_from_opcode(ushort opcode, ushort optype)
{
	SUBR_INFO *si;

	for (si = COMP_subr_info; si->name; si++)
	{
		if (si->opcode == opcode)
		{
			if (si->min_param < si->max_param)
				return si;
			else if (si->optype == optype)
				return si;
		}
	}

	/*ERROR_panic("SUBR_get_from_opcode: SUBR not found !");*/
	return NULL;
}


int RESERVED_get_from_opcode(ushort code)
{
	COMP_INFO *ci;
	int n;
	
	code = code & 0xFF00;

	for (ci = COMP_res_info, n = 0; ci->name; ci++, n++)
	{
		if (ci->code == code)
			return n;
	}

	return -1;
}


int RESERVED_find_word(const char *word, int len)
{
	int ind;
	
	if (len == 1)
	{
		ind = _operator_table[(uint)*word];
		if (ind)
			return ind;
		else
			return -1;
	}
	
	// No symbol longer than 10 characters in the table
	
	if (len > 10)
		return -1;
	
	// Now find it

	static void *jump[] = {
		&&__20, &&__21, &&__22, &&__23, &&__24, &&__25, &&__26, &&__27, 
		&&__28, &&__29, &&__2A, &&__2B, &&__2C, &&__2D, &&__2E, &&__2F, 
		&&__30, &&__31, &&__32, &&__33, &&__34, &&__35, &&__36, &&__37, 
		&&__38, &&__39, &&__3A, &&__3B, &&__3C, &&__3D, &&__3E, &&__3F, 
		&&__40, &&__41, &&__42, &&__43, &&__44, &&__45, &&__46, &&__47, 
		&&__48, &&__49, &&__4A, &&__4B, &&__4C, &&__4D, &&__4E, &&__4F, 
		&&__50, &&__51, &&__52, &&__53, &&__54, &&__55, &&__56, &&__57, 
		&&__58, &&__59, &&__5A, &&__5B, &&__5C, &&__5D, &&__5E, &&__5F, 
		&&__60, &&__61, &&__62, &&__63, &&__64, &&__65, &&__66, &&__67, 
		&&__68, &&__69, &&__6A, &&__6B, &&__6C, &&__6D, &&__6E, &&__6F, 
		&&__70, &&__71, &&__72, &&__73, &&__74, &&__75, &&__76, &&__77, 
		&&__78, &&__79, &&__7A, &&__7B, &&__7C, &&__7D, &&__7E, 
	};

	goto *jump[*word - 32];

__20:
	return -1;
__21:
	return -1;
__22:
	return -1;
__23:
	if (len == 3 && tolower(word[1]) == 'i' && tolower(word[2]) == 'f') return 123;
	if (len == 5 && tolower(word[1]) == 'e' && tolower(word[2]) == 'l' && tolower(word[3]) == 's' && tolower(word[4]) == 'e') return 124;
	if (len == 5 && tolower(word[1]) == 'l' && tolower(word[2]) == 'i' && tolower(word[3]) == 'n' && tolower(word[4]) == 'e') return 127;
	if (len == 6 && tolower(word[1]) == 'c' && tolower(word[2]) == 'o' && tolower(word[3]) == 'n' && tolower(word[4]) == 's' && tolower(word[5]) == 't') return 126;
	if (len == 6 && tolower(word[1]) == 'e' && tolower(word[2]) == 'n' && tolower(word[3]) == 'd' && tolower(word[4]) == 'i' && tolower(word[5]) == 'f') return 125;
	if (len == 7 && tolower(word[1]) == 's' && tolower(word[2]) == 'c' && tolower(word[3]) == 'r' && tolower(word[4]) == 'i' && tolower(word[5]) == 'p' && tolower(word[6]) == 't') return 129;
	if (len == 8 && tolower(word[1]) == 'i' && tolower(word[2]) == 'n' && tolower(word[3]) == 'c' && tolower(word[4]) == 'l' && tolower(word[5]) == 'u' && tolower(word[6]) == 'd' && tolower(word[7]) == 'e') return 128;
	return -1;
__24:
	return -1;
__25:
	if (len == 2 && word[1] == '=') return 190;
	return -1;
__26:
	if (len == 2 && word[1] == '/') return 151;
	if (len == 2 && word[1] == '=') return 191;
	if (len == 3 && word[1] == '/' && word[2] == '=') return 192;
	return -1;
__27:
	return -1;
__28:
	return -1;
__29:
	return -1;
__2A:
	if (len == 2 && word[1] == '=') return 187;
	return -1;
__2B:
	if (len == 2 && word[1] == '=') return 185;
	if (len == 4 && tolower(word[1]) == 'i' && tolower(word[2]) == 'n' && tolower(word[3]) == 'f') return 82;
	return -1;
__2C:
	return -1;
__2D:
	if (len == 2 && word[1] == '=') return 186;
	if (len == 4 && tolower(word[1]) == 'i' && tolower(word[2]) == 'n' && tolower(word[3]) == 'f') return 83;
	return -1;
__2E:
	if (len == 3 && word[1] == '.' && word[2] == '.') return 133;
	return -1;
__2F:
	if (len == 2 && word[1] == '=') return 188;
	return -1;
__30:
	return -1;
__31:
	return -1;
__32:
	return -1;
__33:
	return -1;
__34:
	return -1;
__35:
	return -1;
__36:
	return -1;
__37:
	return -1;
__38:
	return -1;
__39:
	return -1;
__3A:
	return -1;
__3B:
	return -1;
__3C:
	if (len == 2 && word[1] == '=') return 155;
	if (len == 2 && word[1] == '>') return 156;
	return -1;
__3D:
	if (len == 2 && word[1] == '=') return 140;
	return -1;
__3E:
	if (len == 2 && word[1] == '=') return 154;
	return -1;
__3F:
	return -1;
__40:
	return -1;
__41:
__61:
	if (len == 2 && tolower(word[1]) == 's') return 27;
	if (len == 3 && tolower(word[1]) == 'n' && tolower(word[2]) == 'd') return 159;
	if (len == 3 && tolower(word[1]) == 's' && tolower(word[2]) == 'l') return 164;
	if (len == 3 && tolower(word[1]) == 's' && tolower(word[2]) == 'r') return 166;
	if (len == 6 && tolower(word[1]) == 'p' && tolower(word[2]) == 'p' && tolower(word[3]) == 'e' && tolower(word[4]) == 'n' && tolower(word[5]) == 'd') return 93;
	if (len == 6 && tolower(word[1]) == 's' && tolower(word[2]) == 's' && tolower(word[3]) == 'e' && tolower(word[4]) == 'r' && tolower(word[5]) == 't') return 115;
	return -1;
__42:
__62:
	if (len == 4 && tolower(word[1]) == 'y' && tolower(word[2]) == 't' && tolower(word[3]) == 'e') return 2;
	if (len == 5 && tolower(word[1]) == 'r' && tolower(word[2]) == 'e' && tolower(word[3]) == 'a' && tolower(word[4]) == 'k') return 56;
	if (len == 5 && tolower(word[1]) == 'y' && tolower(word[2]) == 'r' && tolower(word[3]) == 'e' && tolower(word[4]) == 'f') return 118;
	if (len == 6 && tolower(word[1]) == 'e' && tolower(word[2]) == 'g' && tolower(word[3]) == 'i' && tolower(word[4]) == 'n' && tolower(word[5]) == 's') return 179;
	if (len == 6 && tolower(word[1]) == 'i' && tolower(word[2]) == 'n' && tolower(word[3]) == 'a' && tolower(word[4]) == 'r' && tolower(word[5]) == 'y') return 95;
	if (len == 7 && tolower(word[1]) == 'o' && tolower(word[2]) == 'o' && tolower(word[3]) == 'l' && tolower(word[4]) == 'e' && tolower(word[5]) == 'a' && tolower(word[6]) == 'n') return 1;
	return -1;
__43:
__63:
	if (len == 4 && tolower(word[1]) == 'a' && tolower(word[2]) == 's' && tolower(word[3]) == 'e') return 54;
	if (len == 4 && tolower(word[1]) == 'o' && tolower(word[2]) == 'p' && tolower(word[3]) == 'y') return 104;
	if (len == 5 && tolower(word[1]) == 'a' && tolower(word[2]) == 't' && tolower(word[3]) == 'c' && tolower(word[4]) == 'h') return 65;
	if (len == 5 && tolower(word[1]) == 'h' && tolower(word[2]) == 'g' && tolower(word[3]) == 'r' && tolower(word[4]) == 'p') return 122;
	if (len == 5 && tolower(word[1]) == 'h' && tolower(word[2]) == 'm' && tolower(word[3]) == 'o' && tolower(word[4]) == 'd') return 120;
	if (len == 5 && tolower(word[1]) == 'h' && tolower(word[2]) == 'o' && tolower(word[3]) == 'w' && tolower(word[4]) == 'n') return 121;
	if (len == 5 && tolower(word[1]) == 'l' && tolower(word[2]) == 'a' && tolower(word[3]) == 's' && tolower(word[4]) == 's') return 13;
	if (len == 5 && tolower(word[1]) == 'l' && tolower(word[2]) == 'o' && tolower(word[3]) == 's' && tolower(word[4]) == 'e') return 91;
	if (len == 5 && tolower(word[1]) == 'o' && tolower(word[2]) == 'n' && tolower(word[3]) == 's' && tolower(word[4]) == 't') return 16;
	if (len == 6 && tolower(word[1]) == 'r' && tolower(word[2]) == 'e' && tolower(word[3]) == 'a' && tolower(word[4]) == 't' && tolower(word[5]) == 'e') return 94;
	if (len == 8 && tolower(word[1]) == 'o' && tolower(word[2]) == 'n' && tolower(word[3]) == 't' && tolower(word[4]) == 'i' && tolower(word[5]) == 'n' && tolower(word[6]) == 'u' && tolower(word[7]) == 'e') return 57;
	return -1;
__44:
__64:
	if (len == 2 && tolower(word[1]) == 'o') return 36;
	if (len == 3 && tolower(word[1]) == 'e' && tolower(word[2]) == 'c') return 106;
	if (len == 3 && tolower(word[1]) == 'i' && tolower(word[2]) == 'm') return 29;
	if (len == 3 && tolower(word[1]) == 'i' && tolower(word[2]) == 'v') return 172;
	if (len == 4 && tolower(word[1]) == 'a' && tolower(word[2]) == 't' && tolower(word[3]) == 'e') return 3;
	if (len == 5 && tolower(word[1]) == 'e' && tolower(word[2]) == 'b' && tolower(word[3]) == 'u' && tolower(word[4]) == 'g') return 114;
	if (len == 6 && tolower(word[1]) == 'o' && tolower(word[2]) == 'w' && tolower(word[3]) == 'n' && tolower(word[4]) == 't' && tolower(word[5]) == 'o') return 49;
	if (len == 7 && tolower(word[1]) == 'e' && tolower(word[2]) == 'f' && tolower(word[3]) == 'a' && tolower(word[4]) == 'u' && tolower(word[5]) == 'l' && tolower(word[6]) == 't') return 74;
	return -1;
__45:
__65:
	if (len == 3 && tolower(word[1]) == 'n' && tolower(word[2]) == 'd') return 46;
	if (len == 4 && tolower(word[1]) == 'a' && tolower(word[2]) == 'c' && tolower(word[3]) == 'h') return 72;
	if (len == 4 && tolower(word[1]) == 'l' && tolower(word[2]) == 's' && tolower(word[3]) == 'e') return 44;
	if (len == 4 && tolower(word[1]) == 'n' && tolower(word[2]) == 'd' && tolower(word[3]) == 's') return 181;
	if (len == 4 && tolower(word[1]) == 'n' && tolower(word[2]) == 'u' && tolower(word[3]) == 'm') return 80;
	if (len == 4 && tolower(word[1]) == 'x' && tolower(word[2]) == 'e' && tolower(word[3]) == 'c') return 98;
	if (len == 4 && tolower(word[1]) == 'x' && tolower(word[2]) == 'i' && tolower(word[3]) == 't') return 55;
	if (len == 5 && tolower(word[1]) == 'n' && tolower(word[2]) == 'd' && tolower(word[3]) == 'i' && tolower(word[4]) == 'f') return 45;
	if (len == 5 && tolower(word[1]) == 'r' && tolower(word[2]) == 'r' && tolower(word[3]) == 'o' && tolower(word[4]) == 'r') return 78;
	if (len == 5 && tolower(word[1]) == 'v' && tolower(word[2]) == 'e' && tolower(word[3]) == 'n' && tolower(word[4]) == 't') return 23;
	if (len == 6 && tolower(word[1]) == 'x' && tolower(word[2]) == 'p' && tolower(word[3]) == 'o' && tolower(word[4]) == 'r' && tolower(word[5]) == 't') return 26;
	if (len == 6 && tolower(word[1]) == 'x' && tolower(word[2]) == 't' && tolower(word[3]) == 'e' && tolower(word[4]) == 'r' && tolower(word[5]) == 'n') return 71;
	return -1;
__46:
__66:
	if (len == 3 && tolower(word[1]) == 'o' && tolower(word[2]) == 'r') return 47;
	if (len == 4 && tolower(word[1]) == 'a' && tolower(word[2]) == 's' && tolower(word[3]) == 't') return 20;
	if (len == 4 && tolower(word[1]) == 'r' && tolower(word[2]) == 'o' && tolower(word[3]) == 'm') return 50;
	if (len == 5 && tolower(word[1]) == 'a' && tolower(word[2]) == 'l' && tolower(word[3]) == 's' && tolower(word[4]) == 'e') return 68;
	if (len == 5 && tolower(word[1]) == 'l' && tolower(word[2]) == 'o' && tolower(word[3]) == 'a' && tolower(word[4]) == 't') return 5;
	if (len == 5 && tolower(word[1]) == 'l' && tolower(word[2]) == 'u' && tolower(word[3]) == 's' && tolower(word[4]) == 'h') return 97;
	if (len == 7 && tolower(word[1]) == 'i' && tolower(word[2]) == 'n' && tolower(word[3]) == 'a' && tolower(word[4]) == 'l' && tolower(word[5]) == 'l' && tolower(word[6]) == 'y') return 64;
	if (len == 8 && tolower(word[1]) == 'u' && tolower(word[2]) == 'n' && tolower(word[3]) == 'c' && tolower(word[4]) == 't' && tolower(word[5]) == 'i' && tolower(word[6]) == 'o' && tolower(word[7]) == 'n') return 14;
	return -1;
__47:
__67:
	if (len == 4 && tolower(word[1]) == 'o' && tolower(word[2]) == 't' && tolower(word[3]) == 'o') return 58;
	if (len == 5 && tolower(word[1]) == 'o' && tolower(word[2]) == 's' && tolower(word[3]) == 'u' && tolower(word[4]) == 'b') return 59;
	return -1;
__48:
__68:
	return -1;
__49:
__69:
	if (len == 2 && tolower(word[1]) == 'f') return 42;
	if (len == 2 && tolower(word[1]) == 'n') return 73;
	if (len == 2 && tolower(word[1]) == 's') return 175;
	if (len == 3 && tolower(word[1]) == 'n' && tolower(word[2]) == 'c') return 105;
	if (len == 5 && tolower(word[1]) == 'n' && tolower(word[2]) == 'p' && tolower(word[3]) == 'u' && tolower(word[4]) == 't') return 86;
	if (len == 7 && tolower(word[1]) == 'n' && tolower(word[2]) == 't' && tolower(word[3]) == 'e' && tolower(word[4]) == 'g' && tolower(word[5]) == 'e' && tolower(word[6]) == 'r') return 6;
	if (len == 8 && tolower(word[1]) == 'n' && tolower(word[2]) == 'h' && tolower(word[3]) == 'e' && tolower(word[4]) == 'r' && tolower(word[5]) == 'i' && tolower(word[6]) == 't' && tolower(word[7]) == 's') return 24;
	if (len == 10 && tolower(word[1]) == 'm' && tolower(word[2]) == 'p' && tolower(word[3]) == 'l' && tolower(word[4]) == 'e' && tolower(word[5]) == 'm' && tolower(word[6]) == 'e' && tolower(word[7]) == 'n' && tolower(word[8]) == 't' && tolower(word[9]) == 's') return 25;
	return -1;
__4A:
__6A:
	return -1;
__4B:
__6B:
	if (len == 4 && tolower(word[1]) == 'i' && tolower(word[2]) == 'l' && tolower(word[3]) == 'l') return 102;
	return -1;
__4C:
__6C:
	if (len == 3 && tolower(word[1]) == 'e' && tolower(word[2]) == 't') return 81;
	if (len == 3 && tolower(word[1]) == 's' && tolower(word[2]) == 'l') return 169;
	if (len == 3 && tolower(word[1]) == 's' && tolower(word[2]) == 'r') return 170;
	if (len == 4 && tolower(word[1]) == 'a' && tolower(word[2]) == 's' && tolower(word[3]) == 't') return 62;
	if (len == 4 && tolower(word[1]) == 'i' && tolower(word[2]) == 'k' && tolower(word[3]) == 'e') return 177;
	if (len == 4 && tolower(word[1]) == 'i' && tolower(word[2]) == 'n' && tolower(word[3]) == 'e') return 96;
	if (len == 4 && tolower(word[1]) == 'i' && tolower(word[2]) == 'n' && tolower(word[3]) == 'k') return 110;
	if (len == 4 && tolower(word[1]) == 'o' && tolower(word[2]) == 'c' && tolower(word[3]) == 'k') return 111;
	if (len == 4 && tolower(word[1]) == 'o' && tolower(word[2]) == 'n' && tolower(word[3]) == 'g') return 7;
	if (len == 4 && tolower(word[1]) == 'o' && tolower(word[2]) == 'o' && tolower(word[3]) == 'p') return 37;
	if (len == 7 && tolower(word[1]) == 'i' && tolower(word[2]) == 'b' && tolower(word[3]) == 'r' && tolower(word[4]) == 'a' && tolower(word[5]) == 'r' && tolower(word[6]) == 'y') return 113;
	return -1;
__4D:
__6D:
	if (len == 2 && tolower(word[1]) == 'e') return 61;
	if (len == 3 && tolower(word[1]) == 'o' && tolower(word[2]) == 'd') return 174;
	if (len == 4 && tolower(word[1]) == 'o' && tolower(word[2]) == 'v' && tolower(word[3]) == 'e') return 103;
	if (len == 5 && tolower(word[1]) == 'a' && tolower(word[2]) == 't' && tolower(word[3]) == 'c' && tolower(word[4]) == 'h') return 183;
	if (len == 5 && tolower(word[1]) == 'k' && tolower(word[2]) == 'd' && tolower(word[3]) == 'i' && tolower(word[4]) == 'r') return 107;
	if (len == 6 && tolower(word[1]) == 'e' && tolower(word[2]) == 'm' && tolower(word[3]) == 'o' && tolower(word[4]) == 'r' && tolower(word[5]) == 'y') return 119;
	return -1;
__4E:
__6E:
	if (len == 3 && tolower(word[1]) == 'e' && tolower(word[2]) == 'w') return 30;
	if (len == 3 && tolower(word[1]) == 'o' && tolower(word[2]) == 't') return 161;
	if (len == 4 && tolower(word[1]) == 'e' && tolower(word[2]) == 'x' && tolower(word[3]) == 't') return 52;
	if (len == 4 && tolower(word[1]) == 'u' && tolower(word[2]) == 'l' && tolower(word[3]) == 'l') return 70;
	return -1;
__4F:
__6F:
	if (len == 2 && tolower(word[1]) == 'f') return 28;
	if (len == 2 && tolower(word[1]) == 'n') return 60;
	if (len == 2 && tolower(word[1]) == 'r') return 160;
	if (len == 4 && tolower(word[1]) == 'p' && tolower(word[2]) == 'e' && tolower(word[3]) == 'n') return 90;
	if (len == 6 && tolower(word[1]) == 'b' && tolower(word[2]) == 'j' && tolower(word[3]) == 'e' && tolower(word[4]) == 'c' && tolower(word[5]) == 't') return 11;
	if (len == 6 && tolower(word[1]) == 'u' && tolower(word[2]) == 't' && tolower(word[3]) == 'p' && tolower(word[4]) == 'u' && tolower(word[5]) == 't') return 35;
	if (len == 8 && tolower(word[1]) == 'p' && tolower(word[2]) == 't' && tolower(word[3]) == 'i' && tolower(word[4]) == 'o' && tolower(word[5]) == 'n' && tolower(word[6]) == 'a' && tolower(word[7]) == 'l') return 34;
	return -1;
__50:
__70:
	if (len == 4 && tolower(word[1]) == 'e' && tolower(word[2]) == 'e' && tolower(word[3]) == 'k') return 88;
	if (len == 4 && tolower(word[1]) == 'i' && tolower(word[2]) == 'p' && tolower(word[3]) == 'e') return 116;
	if (len == 5 && tolower(word[1]) == 'r' && tolower(word[2]) == 'i' && tolower(word[3]) == 'n' && tolower(word[4]) == 't') return 85;
	if (len == 6 && tolower(word[1]) == 'u' && tolower(word[2]) == 'b' && tolower(word[3]) == 'l' && tolower(word[4]) == 'i' && tolower(word[5]) == 'c') return 18;
	if (len == 7 && tolower(word[1]) == 'o' && tolower(word[2]) == 'i' && tolower(word[3]) == 'n' && tolower(word[4]) == 't' && tolower(word[5]) == 'e' && tolower(word[6]) == 'r') return 12;
	if (len == 7 && tolower(word[1]) == 'r' && tolower(word[2]) == 'i' && tolower(word[3]) == 'v' && tolower(word[4]) == 'a' && tolower(word[5]) == 't' && tolower(word[6]) == 'e') return 17;
	if (len == 8 && tolower(word[1]) == 'r' && tolower(word[2]) == 'o' && tolower(word[3]) == 'p' && tolower(word[4]) == 'e' && tolower(word[5]) == 'r' && tolower(word[6]) == 't' && tolower(word[7]) == 'y') return 22;
	if (len == 9 && tolower(word[1]) == 'r' && tolower(word[2]) == 'o' && tolower(word[3]) == 'c' && tolower(word[4]) == 'e' && tolower(word[5]) == 'd' && tolower(word[6]) == 'u' && tolower(word[7]) == 'r' && tolower(word[8]) == 'e') return 31;
	return -1;
__51:
__71:
	if (len == 4 && tolower(word[1]) == 'u' && tolower(word[2]) == 'i' && tolower(word[3]) == 't') return 76;
	return -1;
__52:
__72:
	if (len == 3 && tolower(word[1]) == 'o' && tolower(word[2]) == 'l') return 167;
	if (len == 3 && tolower(word[1]) == 'o' && tolower(word[2]) == 'r') return 168;
	if (len == 4 && tolower(word[1]) == 'e' && tolower(word[2]) == 'a' && tolower(word[3]) == 'd') return 87;
	if (len == 5 && tolower(word[1]) == 'a' && tolower(word[2]) == 'i' && tolower(word[3]) == 's' && tolower(word[4]) == 'e') return 77;
	if (len == 5 && tolower(word[1]) == 'm' && tolower(word[2]) == 'd' && tolower(word[3]) == 'i' && tolower(word[4]) == 'r') return 108;
	if (len == 6 && tolower(word[1]) == 'e' && tolower(word[2]) == 'p' && tolower(word[3]) == 'e' && tolower(word[4]) == 'a' && tolower(word[5]) == 't') return 40;
	if (len == 6 && tolower(word[1]) == 'e' && tolower(word[2]) == 't' && tolower(word[3]) == 'u' && tolower(word[4]) == 'r' && tolower(word[5]) == 'n') return 33;
	if (len == 9 && tolower(word[1]) == 'a' && tolower(word[2]) == 'n' && tolower(word[3]) == 'd' && tolower(word[4]) == 'o' && tolower(word[5]) == 'm' && tolower(word[6]) == 'i' && tolower(word[7]) == 'z' && tolower(word[8]) == 'e') return 117;
	return -1;
__53:
__73:
	if (len == 3 && tolower(word[1]) == 'h' && tolower(word[2]) == 'l') return 163;
	if (len == 3 && tolower(word[1]) == 'h' && tolower(word[2]) == 'r') return 165;
	if (len == 3 && tolower(word[1]) == 'u' && tolower(word[2]) == 'b') return 32;
	if (len == 4 && tolower(word[1]) == 'e' && tolower(word[2]) == 'e' && tolower(word[3]) == 'k') return 92;
	if (len == 4 && tolower(word[1]) == 't' && tolower(word[2]) == 'e' && tolower(word[3]) == 'p') return 51;
	if (len == 4 && tolower(word[1]) == 't' && tolower(word[2]) == 'o' && tolower(word[3]) == 'p') return 75;
	if (len == 4 && tolower(word[1]) == 'w' && tolower(word[2]) == 'a' && tolower(word[3]) == 'p') return 69;
	if (len == 5 && tolower(word[1]) == 'h' && tolower(word[2]) == 'e' && tolower(word[3]) == 'l' && tolower(word[4]) == 'l') return 99;
	if (len == 5 && tolower(word[1]) == 'h' && tolower(word[2]) == 'o' && tolower(word[3]) == 'r' && tolower(word[4]) == 't') return 8;
	if (len == 5 && tolower(word[1]) == 'l' && tolower(word[2]) == 'e' && tolower(word[3]) == 'e' && tolower(word[4]) == 'p') return 101;
	if (len == 5 && tolower(word[1]) == 'u' && tolower(word[2]) == 'p' && tolower(word[3]) == 'e' && tolower(word[4]) == 'r') return 79;
	if (len == 6 && tolower(word[1]) == 'e' && tolower(word[2]) == 'l' && tolower(word[3]) == 'e' && tolower(word[4]) == 'c' && tolower(word[5]) == 't') return 53;
	if (len == 6 && tolower(word[1]) == 'i' && tolower(word[2]) == 'n' && tolower(word[3]) == 'g' && tolower(word[4]) == 'l' && tolower(word[5]) == 'e') return 4;
	if (len == 6 && tolower(word[1]) == 't' && tolower(word[2]) == 'a' && tolower(word[3]) == 't' && tolower(word[4]) == 'i' && tolower(word[5]) == 'c') return 19;
	if (len == 6 && tolower(word[1]) == 't' && tolower(word[2]) == 'r' && tolower(word[3]) == 'i' && tolower(word[4]) == 'n' && tolower(word[5]) == 'g') return 9;
	if (len == 6 && tolower(word[1]) == 't' && tolower(word[2]) == 'r' && tolower(word[3]) == 'u' && tolower(word[4]) == 'c' && tolower(word[5]) == 't') return 15;
	return -1;
__54:
__74:
	if (len == 2 && tolower(word[1]) == 'o') return 48;
	if (len == 3 && tolower(word[1]) == 'r' && tolower(word[2]) == 'y') return 63;
	if (len == 4 && tolower(word[1]) == 'h' && tolower(word[2]) == 'e' && tolower(word[3]) == 'n') return 43;
	if (len == 4 && tolower(word[1]) == 'r' && tolower(word[2]) == 'u' && tolower(word[3]) == 'e') return 67;
	return -1;
__55:
__75:
	if (len == 3 && tolower(word[1]) == 's' && tolower(word[2]) == 'e') return 84;
	if (len == 5 && tolower(word[1]) == 'n' && tolower(word[2]) == 't' && tolower(word[3]) == 'i' && tolower(word[4]) == 'l') return 39;
	if (len == 6 && tolower(word[1]) == 'n' && tolower(word[2]) == 'l' && tolower(word[3]) == 'o' && tolower(word[4]) == 'c' && tolower(word[5]) == 'k') return 112;
	if (len == 6 && tolower(word[1]) == 'n' && tolower(word[2]) == 's' && tolower(word[3]) == 'a' && tolower(word[4]) == 'f' && tolower(word[5]) == 'e') return 21;
	return -1;
__56:
__76:
	if (len == 7 && tolower(word[1]) == 'a' && tolower(word[2]) == 'r' && tolower(word[3]) == 'i' && tolower(word[4]) == 'a' && tolower(word[5]) == 'n' && tolower(word[6]) == 't') return 10;
	return -1;
__57:
__77:
	if (len == 4 && tolower(word[1]) == 'a' && tolower(word[2]) == 'i' && tolower(word[3]) == 't') return 100;
	if (len == 4 && tolower(word[1]) == 'e' && tolower(word[2]) == 'n' && tolower(word[3]) == 'd') return 41;
	if (len == 4 && tolower(word[1]) == 'i' && tolower(word[2]) == 't' && tolower(word[3]) == 'h') return 66;
	if (len == 5 && tolower(word[1]) == 'a' && tolower(word[2]) == 't' && tolower(word[3]) == 'c' && tolower(word[4]) == 'h') return 109;
	if (len == 5 && tolower(word[1]) == 'h' && tolower(word[2]) == 'i' && tolower(word[3]) == 'l' && tolower(word[4]) == 'e') return 38;
	if (len == 5 && tolower(word[1]) == 'r' && tolower(word[2]) == 'i' && tolower(word[3]) == 't' && tolower(word[4]) == 'e') return 89;
	return -1;
__58:
__78:
	if (len == 3 && tolower(word[1]) == 'o' && tolower(word[2]) == 'r') return 162;
	return -1;
__59:
__79:
	return -1;
__5A:
__7A:
	return -1;
__5B:
	return -1;
__5C:
	if (len == 2 && word[1] == '=') return 189;
	return -1;
__5D:
	return -1;
__5E:
	if (len == 2 && word[1] == '=') return 193;
	return -1;
__5F:
	return -1;
__60:
	return -1;
__7B:
	return -1;
__7C:
	return -1;
__7D:
	return -1;
__7E:
	return -1;
}

int RESERVED_find_subr(const char *word, int len)
{
	// No symbol longer than 11 characters in the table
	
	if (len > 11)
		return -1;
	
	// Now find it

	static void *jump[] = {
		&&__20, &&__21, &&__22, &&__23, &&__24, &&__25, &&__26, &&__27, 
		&&__28, &&__29, &&__2A, &&__2B, &&__2C, &&__2D, &&__2E, &&__2F, 
		&&__30, &&__31, &&__32, &&__33, &&__34, &&__35, &&__36, &&__37, 
		&&__38, &&__39, &&__3A, &&__3B, &&__3C, &&__3D, &&__3E, &&__3F, 
		&&__40, &&__41, &&__42, &&__43, &&__44, &&__45, &&__46, &&__47, 
		&&__48, &&__49, &&__4A, &&__4B, &&__4C, &&__4D, &&__4E, &&__4F, 
		&&__50, &&__51, &&__52, &&__53, &&__54, &&__55, &&__56, &&__57, 
		&&__58, &&__59, &&__5A, &&__5B, &&__5C, &&__5D, &&__5E, &&__5F, 
		&&__60, &&__61, &&__62, &&__63, &&__64, &&__65, &&__66, &&__67, 
		&&__68, &&__69, &&__6A, &&__6B, &&__6C, &&__6D, &&__6E, &&__6F, 
		&&__70, &&__71, &&__72, &&__73, &&__74, &&__75, &&__76, &&__77, 
		&&__78, &&__79, &&__7A, &&__7B, &&__7C, &&__7D, &&__7E, 
	};

	goto *jump[*word - 32];

__20:
	return -1;
__21:
	return -1;
__22:
	return -1;
__23:
	return -1;
__24:
	return -1;
__25:
	return -1;
__26:
	return -1;
__27:
	return -1;
__28:
	return -1;
__29:
	return -1;
__2A:
	return -1;
__2B:
	return -1;
__2C:
	return -1;
__2D:
	return -1;
__2E:
	if (len == 4 && tolower(word[1]) == 'u' && tolower(word[2]) == 's' && tolower(word[3]) == 'e') return 228;
	if (len == 5 && tolower(word[1]) == 'c' && tolower(word[2]) == 'o' && tolower(word[3]) == 'p' && tolower(word[4]) == 'y') return 201;
	if (len == 5 && tolower(word[1]) == 'e' && tolower(word[2]) == 'x' && tolower(word[3]) == 'e' && tolower(word[4]) == 'c') return 220;
	if (len == 5 && tolower(word[1]) == 'k' && tolower(word[2]) == 'i' && tolower(word[3]) == 'l' && tolower(word[4]) == 'l') return 194;
	if (len == 5 && tolower(word[1]) == 'l' && tolower(word[2]) == 'i' && tolower(word[3]) == 'n' && tolower(word[4]) == 'k') return 202;
	if (len == 5 && tolower(word[1]) == 'l' && tolower(word[2]) == 'o' && tolower(word[3]) == 'c' && tolower(word[4]) == 'k') return 185;
	if (len == 5 && tolower(word[1]) == 'm' && tolower(word[2]) == 'o' && tolower(word[3]) == 'v' && tolower(word[4]) == 'e') return 200;
	if (len == 5 && tolower(word[1]) == 'o' && tolower(word[2]) == 'p' && tolower(word[3]) == 'e' && tolower(word[4]) == 'n') return 173;
	if (len == 5 && tolower(word[1]) == 'p' && tolower(word[2]) == 'e' && tolower(word[3]) == 'e' && tolower(word[4]) == 'k') return 181;
	if (len == 5 && tolower(word[1]) == 'r' && tolower(word[2]) == 'e' && tolower(word[3]) == 'a' && tolower(word[4]) == 'd') return 179;
	if (len == 5 && tolower(word[1]) == 'w' && tolower(word[2]) == 'a' && tolower(word[3]) == 'i' && tolower(word[4]) == 't') return 170;
	if (len == 6 && tolower(word[1]) == 'a' && tolower(word[2]) == 'r' && tolower(word[3]) == 'r' && tolower(word[4]) == 'a' && tolower(word[5]) == 'y') return 91;
	if (len == 6 && tolower(word[1]) == 'c' && tolower(word[2]) == 'h' && tolower(word[3]) == 'g' && tolower(word[4]) == 'r' && tolower(word[5]) == 'p') return 205;
	if (len == 6 && tolower(word[1]) == 'c' && tolower(word[2]) == 'h' && tolower(word[3]) == 'm' && tolower(word[4]) == 'o' && tolower(word[5]) == 'd') return 203;
	if (len == 6 && tolower(word[1]) == 'c' && tolower(word[2]) == 'h' && tolower(word[3]) == 'o' && tolower(word[4]) == 'w' && tolower(word[5]) == 'n') return 204;
	if (len == 6 && tolower(word[1]) == 'c' && tolower(word[2]) == 'l' && tolower(word[3]) == 'o' && tolower(word[4]) == 's' && tolower(word[5]) == 'e') return 175;
	if (len == 6 && tolower(word[1]) == 'd' && tolower(word[2]) == 'e' && tolower(word[3]) == 'b' && tolower(word[4]) == 'u' && tolower(word[5]) == 'g') return 169;
	if (len == 6 && tolower(word[1]) == 'e' && tolower(word[2]) == 'r' && tolower(word[3]) == 'r' && tolower(word[4]) == 'o' && tolower(word[5]) == 'r') return 168;
	if (len == 6 && tolower(word[1]) == 'f' && tolower(word[2]) == 'l' && tolower(word[3]) == 'u' && tolower(word[4]) == 's' && tolower(word[5]) == 'h') return 184;
	if (len == 6 && tolower(word[1]) == 'i' && tolower(word[2]) == 'n' && tolower(word[3]) == 'p' && tolower(word[4]) == 'u' && tolower(word[5]) == 't') return 176;
	if (len == 6 && tolower(word[1]) == 'm' && tolower(word[2]) == 'k' && tolower(word[3]) == 'd' && tolower(word[4]) == 'i' && tolower(word[5]) == 'r') return 195;
	if (len == 6 && tolower(word[1]) == 'p' && tolower(word[2]) == 'r' && tolower(word[3]) == 'i' && tolower(word[4]) == 'n' && tolower(word[5]) == 't') return 178;
	if (len == 6 && tolower(word[1]) == 'r' && tolower(word[2]) == 'm' && tolower(word[3]) == 'd' && tolower(word[4]) == 'i' && tolower(word[5]) == 'r') return 196;
	if (len == 6 && tolower(word[1]) == 's' && tolower(word[2]) == 'h' && tolower(word[3]) == 'e' && tolower(word[4]) == 'l' && tolower(word[5]) == 'l') return 221;
	if (len == 6 && tolower(word[1]) == 's' && tolower(word[2]) == 'l' && tolower(word[3]) == 'e' && tolower(word[4]) == 'e' && tolower(word[5]) == 'p') return 227;
	if (len == 6 && tolower(word[1]) == 'w' && tolower(word[2]) == 'r' && tolower(word[3]) == 'i' && tolower(word[4]) == 't' && tolower(word[5]) == 'e') return 182;
	if (len == 7 && tolower(word[1]) == 'u' && tolower(word[2]) == 'n' && tolower(word[3]) == 'l' && tolower(word[4]) == 'o' && tolower(word[5]) == 'c' && tolower(word[6]) == 'k') return 186;
	if (len == 8 && tolower(word[1]) == 'e' && tolower(word[2]) == 'r' && tolower(word[3]) == 'r' && tolower(word[4]) == 'o' && tolower(word[5]) == 'r' && tolower(word[6]) == 't' && tolower(word[7]) == 'o') return 190;
	if (len == 9 && tolower(word[1]) == 'l' && tolower(word[2]) == 'o' && tolower(word[3]) == 'c' && tolower(word[4]) == 'k' && tolower(word[5]) == 'w' && tolower(word[6]) == 'a' && tolower(word[7]) == 'i' && tolower(word[8]) == 't') return 187;
	if (len == 9 && tolower(word[1]) == 'm' && tolower(word[2]) == 'o' && tolower(word[3]) == 'v' && tolower(word[4]) == 'e' && tolower(word[5]) == 'k' && tolower(word[6]) == 'i' && tolower(word[7]) == 'l' && tolower(word[8]) == 'l') return 206;
	if (len == 9 && tolower(word[1]) == 'o' && tolower(word[2]) == 'u' && tolower(word[3]) == 't' && tolower(word[4]) == 'p' && tolower(word[5]) == 'u' && tolower(word[6]) == 't' && tolower(word[7]) == 't' && tolower(word[8]) == 'o') return 189;
	if (len == 9 && tolower(word[1]) == 'w' && tolower(word[2]) == 'a' && tolower(word[3]) == 'i' && tolower(word[4]) == 't' && tolower(word[5]) == 'n' && tolower(word[6]) == 'e' && tolower(word[7]) == 'x' && tolower(word[8]) == 't') return 172;
	if (len == 10 && tolower(word[1]) == 'c' && tolower(word[2]) == 'h' && tolower(word[3]) == 'e' && tolower(word[4]) == 'c' && tolower(word[5]) == 'k' && tolower(word[6]) == 'e' && tolower(word[7]) == 'x' && tolower(word[8]) == 'e' && tolower(word[9]) == 'c') return 229;
	if (len == 10 && tolower(word[1]) == 'i' && tolower(word[2]) == 'n' && tolower(word[3]) == 'p' && tolower(word[4]) == 'u' && tolower(word[5]) == 't' && tolower(word[6]) == 'f' && tolower(word[7]) == 'r' && tolower(word[8]) == 'o' && tolower(word[9]) == 'm') return 188;
	if (len == 10 && tolower(word[1]) == 'l' && tolower(word[2]) == 'i' && tolower(word[3]) == 'n' && tolower(word[4]) == 'e' && tolower(word[5]) == 'i' && tolower(word[6]) == 'n' && tolower(word[7]) == 'p' && tolower(word[8]) == 'u' && tolower(word[9]) == 't') return 177;
	if (len == 10 && tolower(word[1]) == 'r' && tolower(word[2]) == 'a' && tolower(word[3]) == 'n' && tolower(word[4]) == 'd' && tolower(word[5]) == 'o' && tolower(word[6]) == 'm' && tolower(word[7]) == 'i' && tolower(word[8]) == 'z' && tolower(word[9]) == 'e') return 84;
	if (len == 10 && tolower(word[1]) == 'r' && tolower(word[2]) == 'e' && tolower(word[3]) == 'a' && tolower(word[4]) == 'd' && tolower(word[5]) == 'b' && tolower(word[6]) == 'y' && tolower(word[7]) == 't' && tolower(word[8]) == 'e' && tolower(word[9]) == 's') return 180;
	if (len == 10 && tolower(word[1]) == 'w' && tolower(word[2]) == 'a' && tolower(word[3]) == 'i' && tolower(word[4]) == 't' && tolower(word[5]) == 'd' && tolower(word[6]) == 'e' && tolower(word[7]) == 'l' && tolower(word[8]) == 'a' && tolower(word[9]) == 'y') return 171;
	if (len == 11 && tolower(word[1]) == 'c' && tolower(word[2]) == 'o' && tolower(word[3]) == 'l' && tolower(word[4]) == 'l' && tolower(word[5]) == 'e' && tolower(word[6]) == 'c' && tolower(word[7]) == 't' && tolower(word[8]) == 'i' && tolower(word[9]) == 'o' && tolower(word[10]) == 'n') return 232;
	if (len == 11 && tolower(word[1]) == 'o' && tolower(word[2]) == 'p' && tolower(word[3]) == 'e' && tolower(word[4]) == 'n' && tolower(word[5]) == 'm' && tolower(word[6]) == 'e' && tolower(word[7]) == 'm' && tolower(word[8]) == 'o' && tolower(word[9]) == 'r' && tolower(word[10]) == 'y') return 174;
	if (len == 11 && tolower(word[1]) == 'w' && tolower(word[2]) == 'r' && tolower(word[3]) == 'i' && tolower(word[4]) == 't' && tolower(word[5]) == 'e' && tolower(word[6]) == 'b' && tolower(word[7]) == 'y' && tolower(word[8]) == 't' && tolower(word[9]) == 'e' && tolower(word[10]) == 's') return 183;
	return -1;
__2F:
	return -1;
__30:
	return -1;
__31:
	return -1;
__32:
	return -1;
__33:
	return -1;
__34:
	return -1;
__35:
	return -1;
__36:
	return -1;
__37:
	return -1;
__38:
	return -1;
__39:
	return -1;
__3A:
	return -1;
__3B:
	return -1;
__3C:
	return -1;
__3D:
	return -1;
__3E:
	return -1;
__3F:
	return -1;
__40:
	return -1;
__41:
__61:
	if (len == 3 && tolower(word[1]) == 'b' && tolower(word[2]) == 's') return 45;
	if (len == 3 && tolower(word[1]) == 'c' && tolower(word[2]) == 's') return 60;
	if (len == 3 && tolower(word[1]) == 'n' && tolower(word[2]) == 'g') return 94;
	if (len == 3 && tolower(word[1]) == 's' && tolower(word[2]) == 'c') return 29;
	if (len == 3 && tolower(word[1]) == 's' && tolower(word[2]) == 'l') return 114;
	if (len == 3 && tolower(word[1]) == 's' && tolower(word[2]) == 'n') return 58;
	if (len == 3 && tolower(word[1]) == 's' && tolower(word[2]) == 'r') return 116;
	if (len == 3 && tolower(word[1]) == 't' && tolower(word[2]) == 'n') return 56;
	if (len == 4 && tolower(word[1]) == 'c' && tolower(word[2]) == 'o' && tolower(word[3]) == 's') return 61;
	if (len == 4 && tolower(word[1]) == 'c' && tolower(word[2]) == 's' && tolower(word[3]) == 'h') return 70;
	if (len == 4 && tolower(word[1]) == 's' && tolower(word[2]) == 'i' && tolower(word[3]) == 'n') return 59;
	if (len == 4 && tolower(word[1]) == 's' && tolower(word[2]) == 'n' && tolower(word[3]) == 'h') return 68;
	if (len == 4 && tolower(word[1]) == 't' && tolower(word[2]) == 'a' && tolower(word[3]) == 'n') return 57;
	if (len == 4 && tolower(word[1]) == 't' && tolower(word[2]) == 'n' && word[3] == '2') return 93;
	if (len == 4 && tolower(word[1]) == 't' && tolower(word[2]) == 'n' && tolower(word[3]) == 'h') return 72;
	if (len == 5 && tolower(word[1]) == 'c' && tolower(word[2]) == 'o' && tolower(word[3]) == 's' && tolower(word[4]) == 'h') return 71;
	if (len == 5 && tolower(word[1]) == 'l' && tolower(word[2]) == 'l' && tolower(word[3]) == 'o' && tolower(word[4]) == 'c') return 222;
	if (len == 5 && tolower(word[1]) == 's' && tolower(word[2]) == 'i' && tolower(word[3]) == 'n' && tolower(word[4]) == 'h') return 69;
	if (len == 5 && tolower(word[1]) == 't' && tolower(word[2]) == 'a' && tolower(word[3]) == 'n' && word[4] == '2') return 92;
	if (len == 5 && tolower(word[1]) == 't' && tolower(word[2]) == 'a' && tolower(word[3]) == 'n' && tolower(word[4]) == 'h') return 73;
	if (len == 6 && tolower(word[1]) == 'c' && tolower(word[2]) == 'c' && tolower(word[3]) == 'e' && tolower(word[4]) == 's' && tolower(word[5]) == 's') return 212;
	return -1;
__42:
__62:
	if (len == 3 && tolower(word[1]) == 'i' && tolower(word[2]) == 'n') return 145;
	if (len == 4 && tolower(word[1]) == 'c' && tolower(word[2]) == 'h' && tolower(word[3]) == 'g') return 112;
	if (len == 4 && tolower(word[1]) == 'c' && tolower(word[2]) == 'l' && tolower(word[3]) == 'r') return 109;
	if (len == 4 && tolower(word[1]) == 'i' && tolower(word[2]) == 'n' && word[3] == '$') return 144;
	if (len == 4 && tolower(word[1]) == 's' && tolower(word[2]) == 'e' && tolower(word[3]) == 't') return 110;
	if (len == 4 && tolower(word[1]) == 't' && tolower(word[2]) == 's' && tolower(word[3]) == 't') return 111;
	if (len == 5 && tolower(word[1]) == 'o' && tolower(word[2]) == 'o' && tolower(word[3]) == 'l' && word[4] == '@') return 275;
	if (len == 5 && tolower(word[1]) == 'y' && tolower(word[2]) == 't' && tolower(word[3]) == 'e' && word[4] == '@') return 277;
	if (len == 6 && tolower(word[1]) == 'a' && tolower(word[2]) == 's' && tolower(word[3]) == 'e' && word[4] == '6' && word[5] == '4') return 241;
	if (len == 7 && tolower(word[1]) == 'a' && tolower(word[2]) == 's' && tolower(word[3]) == 'e' && word[4] == '6' && word[5] == '4' && word[6] == '$') return 242;
	if (len == 8 && tolower(word[1]) == 'o' && tolower(word[2]) == 'o' && tolower(word[3]) == 'l' && tolower(word[4]) == 'e' && tolower(word[5]) == 'a' && tolower(word[6]) == 'n' && word[7] == '@') return 276;
	return -1;
__43:
__63:
	if (len == 3 && tolower(word[1]) == 'b' && tolower(word[2]) == 'r') return 77;
	if (len == 3 && tolower(word[1]) == 'h' && tolower(word[2]) == 'r') return 28;
	if (len == 3 && tolower(word[1]) == 'o' && tolower(word[2]) == 's') return 54;
	if (len == 4 && tolower(word[1]) == 'e' && tolower(word[2]) == 'i' && tolower(word[3]) == 'l') return 81;
	if (len == 4 && tolower(word[1]) == 'h' && tolower(word[2]) == 'r' && word[3] == '$') return 27;
	if (len == 4 && tolower(word[1]) == 'i' && tolower(word[2]) == 'n' && tolower(word[3]) == 't') return 134;
	if (len == 4 && tolower(word[1]) == 'o' && tolower(word[2]) == 'm' && tolower(word[3]) == 'p') return 38;
	if (len == 4 && tolower(word[1]) == 'o' && tolower(word[2]) == 'n' && tolower(word[3]) == 'v') return 39;
	if (len == 4 && tolower(word[1]) == 'o' && tolower(word[2]) == 's' && tolower(word[3]) == 'h') return 66;
	if (len == 4 && tolower(word[1]) == 's' && tolower(word[2]) == 't' && tolower(word[3]) == 'r') return 140;
	if (len == 5 && tolower(word[1]) == 'b' && tolower(word[2]) == 'o' && tolower(word[3]) == 'o' && tolower(word[4]) == 'l') return 130;
	if (len == 5 && tolower(word[1]) == 'b' && tolower(word[2]) == 'y' && tolower(word[3]) == 't' && tolower(word[4]) == 'e') return 132;
	if (len == 5 && tolower(word[1]) == 'd' && tolower(word[2]) == 'a' && tolower(word[3]) == 't' && tolower(word[4]) == 'e') return 139;
	if (len == 5 && tolower(word[1]) == 'l' && tolower(word[2]) == 'o' && tolower(word[3]) == 'n' && tolower(word[4]) == 'g') return 136;
	if (len == 5 && tolower(word[1]) == 'o' && tolower(word[2]) == 'n' && tolower(word[3]) == 'v' && word[4] == '$') return 40;
	if (len == 6 && tolower(word[1]) == 'f' && tolower(word[2]) == 'l' && tolower(word[3]) == 'o' && tolower(word[4]) == 'a' && tolower(word[5]) == 't') return 138;
	if (len == 6 && tolower(word[1]) == 'h' && tolower(word[2]) == 'o' && tolower(word[3]) == 'o' && tolower(word[4]) == 's' && tolower(word[5]) == 'e') return 90;
	if (len == 6 && tolower(word[1]) == 's' && tolower(word[2]) == 'h' && tolower(word[3]) == 'o' && tolower(word[4]) == 'r' && tolower(word[5]) == 't') return 133;
	if (len == 7 && tolower(word[1]) == 's' && tolower(word[2]) == 'i' && tolower(word[3]) == 'n' && tolower(word[4]) == 'g' && tolower(word[5]) == 'l' && tolower(word[6]) == 'e') return 137;
	if (len == 7 && tolower(word[1]) == 's' && tolower(word[2]) == 't' && tolower(word[3]) == 'r' && tolower(word[4]) == 'i' && tolower(word[5]) == 'n' && tolower(word[6]) == 'g') return 141;
	if (len == 8 && tolower(word[1]) == 'b' && tolower(word[2]) == 'o' && tolower(word[3]) == 'o' && tolower(word[4]) == 'l' && tolower(word[5]) == 'e' && tolower(word[6]) == 'a' && tolower(word[7]) == 'n') return 131;
	if (len == 8 && tolower(word[1]) == 'i' && tolower(word[2]) == 'n' && tolower(word[3]) == 't' && tolower(word[4]) == 'e' && tolower(word[5]) == 'g' && tolower(word[6]) == 'e' && tolower(word[7]) == 'r') return 135;
	if (len == 8 && tolower(word[1]) == 'p' && tolower(word[2]) == 'o' && tolower(word[3]) == 'i' && tolower(word[4]) == 'n' && tolower(word[5]) == 't' && tolower(word[6]) == 'e' && tolower(word[7]) == 'r') return 142;
	if (len == 8 && tolower(word[1]) == 'v' && tolower(word[2]) == 'a' && tolower(word[3]) == 'r' && tolower(word[4]) == 'i' && tolower(word[5]) == 'a' && tolower(word[6]) == 'n' && tolower(word[7]) == 't') return 143;
	return -1;
__44:
__64:
	if (len == 3 && tolower(word[1]) == 'a' && tolower(word[2]) == 'y') return 157;
	if (len == 3 && tolower(word[1]) == 'e' && tolower(word[2]) == 'g') return 62;
	if (len == 3 && tolower(word[1]) == 'i' && tolower(word[2]) == 'r') return 218;
	if (len == 4 && tolower(word[1]) == 'a' && tolower(word[2]) == 't' && tolower(word[3]) == 'e') return 163;
	if (len == 5 && tolower(word[1]) == 'a' && tolower(word[2]) == 't' && tolower(word[3]) == 'e' && word[4] == '@') return 284;
	if (len == 5 && tolower(word[1]) == 'c' && tolower(word[2]) == 'o' && tolower(word[3]) == 'n' && tolower(word[4]) == 'v') return 43;
	if (len == 5 && tolower(word[1]) == 'f' && tolower(word[2]) == 'r' && tolower(word[3]) == 'e' && tolower(word[4]) == 'e') return 214;
	if (len == 6 && tolower(word[1]) == 'c' && tolower(word[2]) == 'o' && tolower(word[3]) == 'n' && tolower(word[4]) == 'v' && word[5] == '$') return 44;
	if (len == 7 && tolower(word[1]) == 'a' && tolower(word[2]) == 't' && tolower(word[3]) == 'e' && tolower(word[4]) == 'a' && tolower(word[5]) == 'd' && tolower(word[6]) == 'd') return 165;
	if (len == 8 && tolower(word[1]) == 'a' && tolower(word[2]) == 't' && tolower(word[3]) == 'e' && tolower(word[4]) == 'd' && tolower(word[5]) == 'i' && tolower(word[6]) == 'f' && tolower(word[7]) == 'f') return 166;
	return -1;
__45:
__65:
	if (len == 3 && tolower(word[1]) == 'o' && tolower(word[2]) == 'f') return 191;
	if (len == 3 && tolower(word[1]) == 'x' && tolower(word[2]) == 'p') return 51;
	if (len == 4 && tolower(word[1]) == 'v' && tolower(word[2]) == 'a' && tolower(word[3]) == 'l') return 167;
	if (len == 4 && tolower(word[1]) == 'v' && tolower(word[2]) == 'e' && tolower(word[3]) == 'n') return 197;
	if (len == 4 && tolower(word[1]) == 'x' && tolower(word[2]) == 'p' && word[3] == '2') return 74;
	if (len == 4 && tolower(word[1]) == 'x' && tolower(word[2]) == 'p' && tolower(word[3]) == 'm') return 78;
	if (len == 5 && tolower(word[1]) == 'x' && tolower(word[2]) == 'i' && tolower(word[3]) == 's' && tolower(word[4]) == 't') return 211;
	if (len == 5 && tolower(word[1]) == 'x' && tolower(word[2]) == 'p' && word[3] == '1' && word[4] == '0') return 75;
	return -1;
__46:
__66:
	if (len == 3 && tolower(word[1]) == 'i' && tolower(word[2]) == 'x') return 47;
	if (len == 4 && tolower(word[1]) == 'r' && tolower(word[2]) == 'a' && tolower(word[3]) == 'c') return 49;
	if (len == 4 && tolower(word[1]) == 'r' && tolower(word[2]) == 'e' && tolower(word[3]) == 'e') return 223;
	if (len == 5 && tolower(word[1]) == 'l' && tolower(word[2]) == 'o' && tolower(word[3]) == 'o' && tolower(word[4]) == 'r') return 80;
	if (len == 6 && tolower(word[1]) == 'l' && tolower(word[2]) == 'o' && tolower(word[3]) == 'a' && tolower(word[4]) == 't' && word[5] == '@') return 283;
	if (len == 6 && tolower(word[1]) == 'o' && tolower(word[2]) == 'r' && tolower(word[3]) == 'm' && tolower(word[4]) == 'a' && tolower(word[5]) == 't') return 152;
	if (len == 7 && tolower(word[1]) == 'o' && tolower(word[2]) == 'r' && tolower(word[3]) == 'm' && tolower(word[4]) == 'a' && tolower(word[5]) == 't' && word[6] == '$') return 151;
	if (len == 7 && tolower(word[1]) == 'r' && tolower(word[2]) == 'o' && tolower(word[3]) == 'm' && tolower(word[4]) == 'u' && tolower(word[5]) == 'r' && tolower(word[6]) == 'l') return 251;
	if (len == 8 && tolower(word[1]) == 'r' && tolower(word[2]) == 'o' && tolower(word[3]) == 'm' && tolower(word[4]) == 'u' && tolower(word[5]) == 'r' && tolower(word[6]) == 'l' && word[7] == '$') return 252;
	if (len == 10 && tolower(word[1]) == 'r' && tolower(word[2]) == 'o' && tolower(word[3]) == 'm' && tolower(word[4]) == 'b' && tolower(word[5]) == 'a' && tolower(word[6]) == 's' && tolower(word[7]) == 'e' && word[8] == '6' && word[9] == '4') return 249;
	if (len == 11 && tolower(word[1]) == 'r' && tolower(word[2]) == 'o' && tolower(word[3]) == 'm' && tolower(word[4]) == 'b' && tolower(word[5]) == 'a' && tolower(word[6]) == 's' && tolower(word[7]) == 'e' && word[8] == '6' && word[9] == '4' && word[10] == '$') return 250;
	return -1;
__47:
__67:
	return -1;
__48:
__68:
	if (len == 3 && tolower(word[1]) == 'e' && tolower(word[2]) == 'x') return 147;
	if (len == 3 && tolower(word[1]) == 'y' && tolower(word[2]) == 'p') return 95;
	if (len == 4 && tolower(word[1]) == 'e' && tolower(word[2]) == 'x' && word[3] == '$') return 146;
	if (len == 4 && tolower(word[1]) == 'o' && tolower(word[2]) == 'u' && tolower(word[3]) == 'r') return 158;
	if (len == 4 && tolower(word[1]) == 't' && tolower(word[2]) == 'm' && tolower(word[3]) == 'l') return 239;
	if (len == 5 && tolower(word[1]) == 't' && tolower(word[2]) == 'm' && tolower(word[3]) == 'l' && word[4] == '$') return 240;
	return -1;
__49:
__69:
	if (len == 2 && tolower(word[1]) == 'f') return 88;
	if (len == 3 && tolower(word[1]) == 'i' && tolower(word[2]) == 'f') return 89;
	if (len == 3 && tolower(word[1]) == 'n' && tolower(word[2]) == 't') return 46;
	if (len == 4 && tolower(word[1]) == 'n' && tolower(word[2]) == 't' && word[3] == '@') return 279;
	if (len == 5 && tolower(word[1]) == 'n' && tolower(word[2]) == 's' && tolower(word[3]) == 't' && tolower(word[4]) == 'r') return 30;
	if (len == 5 && tolower(word[1]) == 's' && tolower(word[2]) == 'd' && tolower(word[3]) == 'i' && tolower(word[4]) == 'r') return 217;
	if (len == 5 && tolower(word[1]) == 's' && tolower(word[2]) == 'i' && tolower(word[3]) == 'n' && tolower(word[4]) == 'f') return 210;
	if (len == 5 && tolower(word[1]) == 's' && tolower(word[2]) == 'n' && tolower(word[3]) == 'a' && tolower(word[4]) == 'n') return 209;
	if (len == 6 && tolower(word[1]) == 's' && tolower(word[2]) == 'd' && tolower(word[3]) == 'a' && tolower(word[4]) == 't' && tolower(word[5]) == 'e') return 125;
	if (len == 6 && tolower(word[1]) == 's' && tolower(word[2]) == 'h' && tolower(word[3]) == 'e' && tolower(word[4]) == 'x' && tolower(word[5]) == 'a') return 104;
	if (len == 6 && tolower(word[1]) == 's' && tolower(word[2]) == 'l' && tolower(word[3]) == 'o' && tolower(word[4]) == 'n' && tolower(word[5]) == 'g') return 123;
	if (len == 6 && tolower(word[1]) == 's' && tolower(word[2]) == 'n' && tolower(word[3]) == 'u' && tolower(word[4]) == 'l' && tolower(word[5]) == 'l') return 127;
	if (len == 7 && tolower(word[1]) == 's' && tolower(word[2]) == 'a' && tolower(word[3]) == 'l' && tolower(word[4]) == 'n' && tolower(word[5]) == 'u' && tolower(word[6]) == 'm') return 108;
	if (len == 7 && tolower(word[1]) == 's' && tolower(word[2]) == 'a' && tolower(word[3]) == 's' && tolower(word[4]) == 'c' && tolower(word[5]) == 'i' && tolower(word[6]) == 'i') return 97;
	if (len == 7 && tolower(word[1]) == 's' && tolower(word[2]) == 'b' && tolower(word[3]) == 'l' && tolower(word[4]) == 'a' && tolower(word[5]) == 'n' && tolower(word[6]) == 'k') return 106;
	if (len == 7 && tolower(word[1]) == 's' && tolower(word[2]) == 'd' && tolower(word[3]) == 'i' && tolower(word[4]) == 'g' && tolower(word[5]) == 'i' && tolower(word[6]) == 't') return 103;
	if (len == 7 && tolower(word[1]) == 's' && tolower(word[2]) == 'f' && tolower(word[3]) == 'l' && tolower(word[4]) == 'o' && tolower(word[5]) == 'a' && tolower(word[6]) == 't') return 124;
	if (len == 7 && tolower(word[1]) == 's' && tolower(word[2]) == 'l' && tolower(word[3]) == 'c' && tolower(word[4]) == 'a' && tolower(word[5]) == 's' && tolower(word[6]) == 'e') return 99;
	if (len == 7 && tolower(word[1]) == 's' && tolower(word[2]) == 'l' && tolower(word[3]) == 'o' && tolower(word[4]) == 'w' && tolower(word[5]) == 'e' && tolower(word[6]) == 'r') return 100;
	if (len == 7 && tolower(word[1]) == 's' && tolower(word[2]) == 'p' && tolower(word[3]) == 'u' && tolower(word[4]) == 'n' && tolower(word[5]) == 'c' && tolower(word[6]) == 't') return 107;
	if (len == 7 && tolower(word[1]) == 's' && tolower(word[2]) == 's' && tolower(word[3]) == 'p' && tolower(word[4]) == 'a' && tolower(word[5]) == 'c' && tolower(word[6]) == 'e') return 105;
	if (len == 7 && tolower(word[1]) == 's' && tolower(word[2]) == 'u' && tolower(word[3]) == 'c' && tolower(word[4]) == 'a' && tolower(word[5]) == 's' && tolower(word[6]) == 'e') return 101;
	if (len == 7 && tolower(word[1]) == 's' && tolower(word[2]) == 'u' && tolower(word[3]) == 'p' && tolower(word[4]) == 'p' && tolower(word[5]) == 'e' && tolower(word[6]) == 'r') return 102;
	if (len == 8 && tolower(word[1]) == 'n' && tolower(word[2]) == 't' && tolower(word[3]) == 'e' && tolower(word[4]) == 'g' && tolower(word[5]) == 'e' && tolower(word[6]) == 'r' && word[7] == '@') return 280;
	if (len == 8 && tolower(word[1]) == 's' && tolower(word[2]) == 'l' && tolower(word[3]) == 'e' && tolower(word[4]) == 't' && tolower(word[5]) == 't' && tolower(word[6]) == 'e' && tolower(word[7]) == 'r') return 98;
	if (len == 8 && tolower(word[1]) == 's' && tolower(word[2]) == 'n' && tolower(word[3]) == 'u' && tolower(word[4]) == 'm' && tolower(word[5]) == 'b' && tolower(word[6]) == 'e' && tolower(word[7]) == 'r') return 126;
	if (len == 9 && tolower(word[1]) == 's' && tolower(word[2]) == 'b' && tolower(word[3]) == 'o' && tolower(word[4]) == 'o' && tolower(word[5]) == 'l' && tolower(word[6]) == 'e' && tolower(word[7]) == 'a' && tolower(word[8]) == 'n') return 121;
	if (len == 9 && tolower(word[1]) == 's' && tolower(word[2]) == 'i' && tolower(word[3]) == 'n' && tolower(word[4]) == 't' && tolower(word[5]) == 'e' && tolower(word[6]) == 'g' && tolower(word[7]) == 'e' && tolower(word[8]) == 'r') return 122;
	if (len == 9 && tolower(word[1]) == 's' && tolower(word[2]) == 'm' && tolower(word[3]) == 'i' && tolower(word[4]) == 's' && tolower(word[5]) == 's' && tolower(word[6]) == 'i' && tolower(word[7]) == 'n' && tolower(word[8]) == 'g') return 231;
	return -1;
__4A:
__6A:
	return -1;
__4B:
__6B:
	return -1;
__4C:
__6C:
	if (len == 3 && tolower(word[1]) == 'e' && tolower(word[2]) == 'n') return 6;
	if (len == 3 && tolower(word[1]) == 'o' && tolower(word[2]) == 'f') return 192;
	if (len == 3 && tolower(word[1]) == 'o' && tolower(word[2]) == 'g') return 50;
	if (len == 3 && tolower(word[1]) == 's' && tolower(word[2]) == 'l') return 119;
	if (len == 3 && tolower(word[1]) == 's' && tolower(word[2]) == 'r') return 120;
	if (len == 4 && tolower(word[1]) == 'e' && tolower(word[2]) == 'f' && tolower(word[3]) == 't') return 1;
	if (len == 4 && tolower(word[1]) == 'o' && tolower(word[2]) == 'g' && word[3] == '2') return 76;
	if (len == 4 && tolower(word[1]) == 'o' && tolower(word[2]) == 'g' && tolower(word[3]) == 'p') return 79;
	if (len == 5 && tolower(word[1]) == 'c' && tolower(word[2]) == 'a' && tolower(word[3]) == 's' && tolower(word[4]) == 'e') return 24;
	if (len == 5 && tolower(word[1]) == 'e' && tolower(word[2]) == 'f' && tolower(word[3]) == 't' && word[4] == '$') return 0;
	if (len == 5 && tolower(word[1]) == 'o' && tolower(word[2]) == 'g' && word[3] == '1' && word[4] == '0') return 64;
	if (len == 5 && tolower(word[1]) == 'o' && tolower(word[2]) == 'n' && tolower(word[3]) == 'g' && word[4] == '@') return 281;
	if (len == 5 && tolower(word[1]) == 'o' && tolower(word[2]) == 'w' && tolower(word[3]) == 'e' && tolower(word[4]) == 'r') return 22;
	if (len == 5 && tolower(word[1]) == 't' && tolower(word[2]) == 'r' && tolower(word[3]) == 'i' && tolower(word[4]) == 'm') return 14;
	if (len == 6 && tolower(word[1]) == 'c' && tolower(word[2]) == 'a' && tolower(word[3]) == 's' && tolower(word[4]) == 'e' && word[5] == '$') return 23;
	if (len == 6 && tolower(word[1]) == 'o' && tolower(word[2]) == 'w' && tolower(word[3]) == 'e' && tolower(word[4]) == 'r' && word[5] == '$') return 21;
	if (len == 6 && tolower(word[1]) == 't' && tolower(word[2]) == 'r' && tolower(word[3]) == 'i' && tolower(word[4]) == 'm' && word[5] == '$') return 13;
	return -1;
__4D:
__6D:
	if (len == 3 && tolower(word[1]) == 'a' && tolower(word[2]) == 'g') return 96;
	if (len == 3 && tolower(word[1]) == 'a' && tolower(word[2]) == 'x') return 87;
	if (len == 3 && tolower(word[1]) == 'i' && tolower(word[2]) == 'd') return 3;
	if (len == 3 && tolower(word[1]) == 'i' && tolower(word[2]) == 'n') return 86;
	if (len == 4 && tolower(word[1]) == 'i' && tolower(word[2]) == 'd' && word[3] == '$') return 2;
	if (len == 5 && tolower(word[1]) == 'k' && tolower(word[2]) == 'i' && tolower(word[3]) == 'n' && tolower(word[4]) == 't') return 261;
	if (len == 5 && tolower(word[1]) == 'o' && tolower(word[2]) == 'n' && tolower(word[3]) == 't' && tolower(word[4]) == 'h') return 156;
	if (len == 6 && tolower(word[1]) == 'i' && tolower(word[2]) == 'n' && tolower(word[3]) == 'u' && tolower(word[4]) == 't' && tolower(word[5]) == 'e') return 159;
	if (len == 6 && tolower(word[1]) == 'k' && tolower(word[2]) == 'b' && tolower(word[3]) == 'o' && tolower(word[4]) == 'o' && tolower(word[5]) == 'l') return 253;
	if (len == 6 && tolower(word[1]) == 'k' && tolower(word[2]) == 'b' && tolower(word[3]) == 'y' && tolower(word[4]) == 't' && tolower(word[5]) == 'e') return 257;
	if (len == 6 && tolower(word[1]) == 'k' && tolower(word[2]) == 'd' && tolower(word[3]) == 'a' && tolower(word[4]) == 't' && tolower(word[5]) == 'e') return 271;
	if (len == 6 && tolower(word[1]) == 'k' && tolower(word[2]) == 'i' && tolower(word[3]) == 'n' && tolower(word[4]) == 't' && word[5] == '$') return 262;
	if (len == 6 && tolower(word[1]) == 'k' && tolower(word[2]) == 'l' && tolower(word[3]) == 'o' && tolower(word[4]) == 'n' && tolower(word[5]) == 'g') return 265;
	if (len == 7 && tolower(word[1]) == 'k' && tolower(word[2]) == 'b' && tolower(word[3]) == 'o' && tolower(word[4]) == 'o' && tolower(word[5]) == 'l' && word[6] == '$') return 254;
	if (len == 7 && tolower(word[1]) == 'k' && tolower(word[2]) == 'b' && tolower(word[3]) == 'y' && tolower(word[4]) == 't' && tolower(word[5]) == 'e' && word[6] == '$') return 258;
	if (len == 7 && tolower(word[1]) == 'k' && tolower(word[2]) == 'd' && tolower(word[3]) == 'a' && tolower(word[4]) == 't' && tolower(word[5]) == 'e' && word[6] == '$') return 272;
	if (len == 7 && tolower(word[1]) == 'k' && tolower(word[2]) == 'f' && tolower(word[3]) == 'l' && tolower(word[4]) == 'o' && tolower(word[5]) == 'a' && tolower(word[6]) == 't') return 269;
	if (len == 7 && tolower(word[1]) == 'k' && tolower(word[2]) == 'l' && tolower(word[3]) == 'o' && tolower(word[4]) == 'n' && tolower(word[5]) == 'g' && word[6] == '$') return 266;
	if (len == 7 && tolower(word[1]) == 'k' && tolower(word[2]) == 's' && tolower(word[3]) == 'h' && tolower(word[4]) == 'o' && tolower(word[5]) == 'r' && tolower(word[6]) == 't') return 259;
	if (len == 8 && tolower(word[1]) == 'k' && tolower(word[2]) == 'f' && tolower(word[3]) == 'l' && tolower(word[4]) == 'o' && tolower(word[5]) == 'a' && tolower(word[6]) == 't' && word[7] == '$') return 270;
	if (len == 8 && tolower(word[1]) == 'k' && tolower(word[2]) == 's' && tolower(word[3]) == 'h' && tolower(word[4]) == 'o' && tolower(word[5]) == 'r' && tolower(word[6]) == 't' && word[7] == '$') return 260;
	if (len == 8 && tolower(word[1]) == 'k' && tolower(word[2]) == 's' && tolower(word[3]) == 'i' && tolower(word[4]) == 'n' && tolower(word[5]) == 'g' && tolower(word[6]) == 'l' && tolower(word[7]) == 'e') return 267;
	if (len == 9 && tolower(word[1]) == 'k' && tolower(word[2]) == 'b' && tolower(word[3]) == 'o' && tolower(word[4]) == 'o' && tolower(word[5]) == 'l' && tolower(word[6]) == 'e' && tolower(word[7]) == 'a' && tolower(word[8]) == 'n') return 255;
	if (len == 9 && tolower(word[1]) == 'k' && tolower(word[2]) == 'i' && tolower(word[3]) == 'n' && tolower(word[4]) == 't' && tolower(word[5]) == 'e' && tolower(word[6]) == 'g' && tolower(word[7]) == 'e' && tolower(word[8]) == 'r') return 263;
	if (len == 9 && tolower(word[1]) == 'k' && tolower(word[2]) == 'p' && tolower(word[3]) == 'o' && tolower(word[4]) == 'i' && tolower(word[5]) == 'n' && tolower(word[6]) == 't' && tolower(word[7]) == 'e' && tolower(word[8]) == 'r') return 273;
	if (len == 9 && tolower(word[1]) == 'k' && tolower(word[2]) == 's' && tolower(word[3]) == 'i' && tolower(word[4]) == 'n' && tolower(word[5]) == 'g' && tolower(word[6]) == 'l' && tolower(word[7]) == 'e' && word[8] == '$') return 268;
	if (len == 10 && tolower(word[1]) == 'k' && tolower(word[2]) == 'b' && tolower(word[3]) == 'o' && tolower(word[4]) == 'o' && tolower(word[5]) == 'l' && tolower(word[6]) == 'e' && tolower(word[7]) == 'a' && tolower(word[8]) == 'n' && word[9] == '$') return 256;
	if (len == 10 && tolower(word[1]) == 'k' && tolower(word[2]) == 'i' && tolower(word[3]) == 'n' && tolower(word[4]) == 't' && tolower(word[5]) == 'e' && tolower(word[6]) == 'g' && tolower(word[7]) == 'e' && tolower(word[8]) == 'r' && word[9] == '$') return 264;
	if (len == 10 && tolower(word[1]) == 'k' && tolower(word[2]) == 'p' && tolower(word[3]) == 'o' && tolower(word[4]) == 'i' && tolower(word[5]) == 'n' && tolower(word[6]) == 't' && tolower(word[7]) == 'e' && tolower(word[8]) == 'r' && word[9] == '$') return 274;
	return -1;
__4E:
__6E:
	if (len == 3 && tolower(word[1]) == 'o' && tolower(word[2]) == 'w') return 154;
	return -1;
__4F:
__6F:
	if (len == 3 && tolower(word[1]) == 'c' && tolower(word[2]) == 't') return 26;
	if (len == 3 && tolower(word[1]) == 'd' && tolower(word[2]) == 'd') return 198;
	if (len == 4 && tolower(word[1]) == 'c' && tolower(word[2]) == 't' && word[3] == '$') return 25;
	return -1;
__50:
__70:
	if (len == 2 && tolower(word[1]) == 'i') return 82;
	if (len == 8 && tolower(word[1]) == 'o' && tolower(word[2]) == 'i' && tolower(word[3]) == 'n' && tolower(word[4]) == 't' && tolower(word[5]) == 'e' && tolower(word[6]) == 'r' && word[7] == '@') return 285;
	return -1;
__51:
__71:
	if (len == 5 && tolower(word[1]) == 'u' && tolower(word[2]) == 'o' && tolower(word[3]) == 't' && tolower(word[4]) == 'e') return 235;
	if (len == 6 && tolower(word[1]) == 'u' && tolower(word[2]) == 'o' && tolower(word[3]) == 't' && tolower(word[4]) == 'e' && word[5] == '$') return 236;
	return -1;
__52:
__72:
	if (len == 3 && tolower(word[1]) == 'a' && tolower(word[2]) == 'd') return 63;
	if (len == 3 && tolower(word[1]) == 'n' && tolower(word[2]) == 'd') return 85;
	if (len == 3 && tolower(word[1]) == 'o' && tolower(word[2]) == 'l') return 117;
	if (len == 3 && tolower(word[1]) == 'o' && tolower(word[2]) == 'r') return 118;
	if (len == 4 && tolower(word[1]) == 'a' && tolower(word[2]) == 'n' && tolower(word[3]) == 'd') return 199;
	if (len == 4 && tolower(word[1]) == 'd' && tolower(word[2]) == 'i' && tolower(word[3]) == 'r') return 219;
	if (len == 5 && tolower(word[1]) == 'i' && tolower(word[2]) == 'g' && tolower(word[3]) == 'h' && tolower(word[4]) == 't') return 5;
	if (len == 5 && tolower(word[1]) == 'o' && tolower(word[2]) == 'u' && tolower(word[3]) == 'n' && tolower(word[4]) == 'd') return 83;
	if (len == 5 && tolower(word[1]) == 't' && tolower(word[2]) == 'r' && tolower(word[3]) == 'i' && tolower(word[4]) == 'm') return 16;
	if (len == 6 && tolower(word[1]) == 'i' && tolower(word[2]) == 'g' && tolower(word[3]) == 'h' && tolower(word[4]) == 't' && word[5] == '$') return 4;
	if (len == 6 && tolower(word[1]) == 'i' && tolower(word[2]) == 'n' && tolower(word[3]) == 's' && tolower(word[4]) == 't' && tolower(word[5]) == 'r') return 31;
	if (len == 6 && tolower(word[1]) == 't' && tolower(word[2]) == 'r' && tolower(word[3]) == 'i' && tolower(word[4]) == 'm' && word[5] == '$') return 15;
	if (len == 7 && tolower(word[1]) == 'e' && tolower(word[2]) == 'a' && tolower(word[3]) == 'l' && tolower(word[4]) == 'l' && tolower(word[5]) == 'o' && tolower(word[6]) == 'c') return 224;
	if (len == 7 && tolower(word[1]) == 'e' && tolower(word[2]) == 'p' && tolower(word[3]) == 'l' && tolower(word[4]) == 'a' && tolower(word[5]) == 'c' && tolower(word[6]) == 'e') return 35;
	if (len == 8 && tolower(word[1]) == 'e' && tolower(word[2]) == 'p' && tolower(word[3]) == 'l' && tolower(word[4]) == 'a' && tolower(word[5]) == 'c' && tolower(word[6]) == 'e' && word[7] == '$') return 34;
	return -1;
__53:
__73:
	if (len == 3 && tolower(word[1]) == 'g' && tolower(word[2]) == 'n') return 48;
	if (len == 3 && tolower(word[1]) == 'h' && tolower(word[2]) == 'l') return 113;
	if (len == 3 && tolower(word[1]) == 'h' && tolower(word[2]) == 'r') return 115;
	if (len == 3 && tolower(word[1]) == 'i' && tolower(word[2]) == 'n') return 53;
	if (len == 3 && tolower(word[1]) == 'q' && tolower(word[2]) == 'r') return 52;
	if (len == 3 && tolower(word[1]) == 't' && tolower(word[2]) == 'r') return 150;
	if (len == 4 && tolower(word[1]) == 'c' && tolower(word[2]) == 'a' && tolower(word[3]) == 'n') return 37;
	if (len == 4 && tolower(word[1]) == 'e' && tolower(word[2]) == 'e' && tolower(word[3]) == 'k') return 193;
	if (len == 4 && tolower(word[1]) == 'i' && tolower(word[2]) == 'n' && tolower(word[3]) == 'h') return 65;
	if (len == 4 && tolower(word[1]) == 't' && tolower(word[2]) == 'a' && tolower(word[3]) == 't') return 213;
	if (len == 4 && tolower(word[1]) == 't' && tolower(word[2]) == 'r' && word[3] == '$') return 149;
	if (len == 4 && tolower(word[1]) == 't' && tolower(word[2]) == 'r' && word[3] == '@') return 225;
	if (len == 4 && tolower(word[1]) == 'w' && tolower(word[2]) == 'a' && tolower(word[3]) == 'p') return 207;
	if (len == 5 && tolower(word[1]) == 'c' && tolower(word[2]) == 'o' && tolower(word[3]) == 'n' && tolower(word[4]) == 'v') return 41;
	if (len == 5 && tolower(word[1]) == 'h' && tolower(word[2]) == 'e' && tolower(word[3]) == 'l' && tolower(word[4]) == 'l') return 237;
	if (len == 5 && tolower(word[1]) == 'p' && tolower(word[2]) == 'a' && tolower(word[3]) == 'c' && tolower(word[4]) == 'e') return 8;
	if (len == 5 && tolower(word[1]) == 'p' && tolower(word[2]) == 'l' && tolower(word[3]) == 'i' && tolower(word[4]) == 't') return 36;
	if (len == 5 && tolower(word[1]) == 'u' && tolower(word[2]) == 'b' && tolower(word[3]) == 's' && tolower(word[4]) == 't') return 33;
	if (len == 5 && tolower(word[1]) == 'w' && tolower(word[2]) == 'a' && tolower(word[3]) == 'p' && word[4] == '$') return 208;
	if (len == 6 && tolower(word[1]) == 'c' && tolower(word[2]) == 'o' && tolower(word[3]) == 'n' && tolower(word[4]) == 'v' && word[5] == '$') return 42;
	if (len == 6 && tolower(word[1]) == 'e' && tolower(word[2]) == 'c' && tolower(word[3]) == 'o' && tolower(word[4]) == 'n' && tolower(word[5]) == 'd') return 160;
	if (len == 6 && tolower(word[1]) == 'h' && tolower(word[2]) == 'e' && tolower(word[3]) == 'l' && tolower(word[4]) == 'l' && word[5] == '$') return 238;
	if (len == 6 && tolower(word[1]) == 'h' && tolower(word[2]) == 'o' && tolower(word[3]) == 'r' && tolower(word[4]) == 't' && word[5] == '@') return 278;
	if (len == 6 && tolower(word[1]) == 'i' && tolower(word[2]) == 'z' && tolower(word[3]) == 'e' && tolower(word[4]) == 'o' && tolower(word[5]) == 'f') return 129;
	if (len == 6 && tolower(word[1]) == 'p' && tolower(word[2]) == 'a' && tolower(word[3]) == 'c' && tolower(word[4]) == 'e' && word[5] == '$') return 7;
	if (len == 6 && tolower(word[1]) == 't' && tolower(word[2]) == 'r' && tolower(word[3]) == 'i' && tolower(word[4]) == 'n' && tolower(word[5]) == 'g') return 10;
	if (len == 6 && tolower(word[1]) == 'u' && tolower(word[2]) == 'b' && tolower(word[3]) == 's' && tolower(word[4]) == 't' && word[5] == '$') return 32;
	if (len == 7 && tolower(word[1]) == 'i' && tolower(word[2]) == 'n' && tolower(word[3]) == 'g' && tolower(word[4]) == 'l' && tolower(word[5]) == 'e' && word[6] == '@') return 282;
	if (len == 7 && tolower(word[1]) == 't' && tolower(word[2]) == 'r' && tolower(word[3]) == 'i' && tolower(word[4]) == 'n' && tolower(word[5]) == 'g' && word[6] == '$') return 9;
	if (len == 7 && tolower(word[1]) == 't' && tolower(word[2]) == 'r' && tolower(word[3]) == 'i' && tolower(word[4]) == 'n' && tolower(word[5]) == 'g' && word[6] == '@') return 226;
	return -1;
__54:
__74:
	if (len == 2 && tolower(word[1]) == 'r') return 233;
	if (len == 3 && tolower(word[1]) == 'a' && tolower(word[2]) == 'n') return 55;
	if (len == 3 && tolower(word[1]) == 'r' && word[2] == '$') return 234;
	if (len == 4 && tolower(word[1]) == 'a' && tolower(word[2]) == 'n' && tolower(word[3]) == 'h') return 67;
	if (len == 4 && tolower(word[1]) == 'e' && tolower(word[2]) == 'm' && tolower(word[3]) == 'p') return 215;
	if (len == 4 && tolower(word[1]) == 'i' && tolower(word[2]) == 'm' && tolower(word[3]) == 'e') return 164;
	if (len == 4 && tolower(word[1]) == 'r' && tolower(word[2]) == 'i' && tolower(word[3]) == 'm') return 12;
	if (len == 5 && tolower(word[1]) == 'e' && tolower(word[2]) == 'm' && tolower(word[3]) == 'p' && word[4] == '$') return 216;
	if (len == 5 && tolower(word[1]) == 'i' && tolower(word[2]) == 'm' && tolower(word[3]) == 'e' && tolower(word[4]) == 'r') return 153;
	if (len == 5 && tolower(word[1]) == 'r' && tolower(word[2]) == 'i' && tolower(word[3]) == 'm' && word[4] == '$') return 11;
	if (len == 6 && tolower(word[1]) == 'y' && tolower(word[2]) == 'p' && tolower(word[3]) == 'e' && tolower(word[4]) == 'o' && tolower(word[5]) == 'f') return 128;
	return -1;
__55:
__75:
	if (len == 3 && tolower(word[1]) == 'r' && tolower(word[2]) == 'l') return 243;
	if (len == 4 && tolower(word[1]) == 'r' && tolower(word[2]) == 'l' && word[3] == '$') return 244;
	if (len == 5 && tolower(word[1]) == 'c' && tolower(word[2]) == 'a' && tolower(word[3]) == 's' && tolower(word[4]) == 'e') return 20;
	if (len == 5 && tolower(word[1]) == 'p' && tolower(word[2]) == 'p' && tolower(word[3]) == 'e' && tolower(word[4]) == 'r') return 18;
	if (len == 6 && tolower(word[1]) == 'c' && tolower(word[2]) == 'a' && tolower(word[3]) == 's' && tolower(word[4]) == 'e' && word[5] == '$') return 19;
	if (len == 6 && tolower(word[1]) == 'p' && tolower(word[2]) == 'p' && tolower(word[3]) == 'e' && tolower(word[4]) == 'r' && word[5] == '$') return 17;
	if (len == 7 && tolower(word[1]) == 'n' && tolower(word[2]) == 'q' && tolower(word[3]) == 'u' && tolower(word[4]) == 'o' && tolower(word[5]) == 't' && tolower(word[6]) == 'e') return 245;
	if (len == 8 && tolower(word[1]) == 'n' && tolower(word[2]) == 'b' && tolower(word[3]) == 'a' && tolower(word[4]) == 's' && tolower(word[5]) == 'e' && word[6] == '6' && word[7] == '4') return 247;
	if (len == 8 && tolower(word[1]) == 'n' && tolower(word[2]) == 'q' && tolower(word[3]) == 'u' && tolower(word[4]) == 'o' && tolower(word[5]) == 't' && tolower(word[6]) == 'e' && word[7] == '$') return 246;
	if (len == 9 && tolower(word[1]) == 'n' && tolower(word[2]) == 'b' && tolower(word[3]) == 'a' && tolower(word[4]) == 's' && tolower(word[5]) == 'e' && word[6] == '6' && word[7] == '4' && word[8] == '$') return 248;
	return -1;
__56:
__76:
	if (len == 3 && tolower(word[1]) == 'a' && tolower(word[2]) == 'l') return 148;
	if (len == 6 && tolower(word[1]) == 'a' && tolower(word[2]) == 'r' && tolower(word[3]) == 'p' && tolower(word[4]) == 't' && tolower(word[5]) == 'r') return 230;
	return -1;
__57:
__77:
	if (len == 4 && tolower(word[1]) == 'e' && tolower(word[2]) == 'e' && tolower(word[3]) == 'k') return 162;
	if (len == 7 && tolower(word[1]) == 'e' && tolower(word[2]) == 'e' && tolower(word[3]) == 'k' && tolower(word[4]) == 'd' && tolower(word[5]) == 'a' && tolower(word[6]) == 'y') return 161;
	return -1;
__58:
__78:
	return -1;
__59:
__79:
	if (len == 4 && tolower(word[1]) == 'e' && tolower(word[2]) == 'a' && tolower(word[3]) == 'r') return 155;
	return -1;
__5A:
__7A:
	return -1;
__5B:
	return -1;
__5C:
	return -1;
__5D:
	return -1;
__5E:
	return -1;
__5F:
	return -1;
__60:
	return -1;
__7B:
	return -1;
__7C:
	return -1;
__7D:
	return -1;
__7E:
	return -1;
}
