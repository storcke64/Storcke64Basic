/***************************************************************************

  gb_overflow.h

  (c) Benoît Minisini <benoit.minisini@gambas-basic.org>

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

#ifndef __GB_OVERFLOW_H
#define __GB_OVERFLOW_H

#define DO_NOT_CHECK_OVERFLOW 0

#if !__has_builtin(__builtin_add_overflow)
#define __builtin_add_overflow(_a, _b, _c) (*(_c) = (_a) + (_b),0)
#endif

#if !__has_builtin(__builtin_sadd_overflow)
#define __builtin_sadd_overflow(_a, _b, _c) (*(_c) = (_a) + (_b),0)
#endif

#if !__has_builtin(__builtin_saddl_overflow)
#define __builtin_saddl_overflow(_a, _b, _c) (*(_c) = (_a) + (_b),0)
#endif

#if !__has_builtin(__builtin_sub_overflow)
#define __builtin_sub_overflow(_a, _b, _c) (*(_c) = (_a) - (_b),0)
#endif

#if !__has_builtin(__builtin_ssub_overflow)
#define __builtin_ssub_overflow(_a, _b, _c) (*(_c) = (_a) - (_b),0)
#endif

#if !__has_builtin(__builtin_ssubl_overflow)
#define __builtin_ssubl_overflow(_a, _b, _c) (*(_c) = (_a) - (_b),0)
#endif

#if !__has_builtin(__builtin_mul_overflow)
#define __builtin_mul_overflow(_a, _b, _c) (*(_c) = (_a) * (_b),0)
#endif

#if !__has_builtin(__builtin_smul_overflow)
#define __builtin_smul_overflow(_a, _b, _c) (*(_c) = (_a) * (_b),0)
#endif

#if !__has_builtin(__builtin_smull_overflow)
#define __builtin_smull_overflow(_a, _b, _c) (*(_c) = (_a) * (_b),0)
#endif

#endif /* __GB_OVERFLOW_H */
