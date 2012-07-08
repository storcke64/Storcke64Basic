/***************************************************************************

  gambas.h

  (c) 2000-2012 Benoît Minisini <gambas@users.sourceforge.net>

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

#ifndef __GAMBAS_H
#define __GAMBAS_H

#include "config.h"
#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>

/* Gambas API Version */

#define GB_VERSION  1


/* Useful macros */

#ifndef CLEAR
#define CLEAR(s) (memset(s, 0, sizeof(*s)))
#endif

#ifndef offsetof
#define offsetof(_type, _arg) ((size_t)&(((_type *)0)->_arg))
#endif

/* The following symbols must be declared with EXPORT in a component:
   - GB
   - GB_INIT()
   - GB_EXIT()
   - GB_CLASSES
   - The component interface if present
*/

#ifdef EXPORT
#undef EXPORT
#endif
#ifdef HAVE_GCC_VISIBILITY
#define EXPORT __attribute__((visibility("default")))
#else
#define EXPORT
#endif

#if defined(__cplusplus)
	#define __null ((intptr_t)0)
#else
	#ifdef bool
		#undef bool
	#endif
	#define bool char
#endif

#ifndef PACKED
#define PACKED __attribute__((packed))
#endif

/* Gambas datatypes identifiers */

#define GB_T_VOID         0
#define GB_T_BOOLEAN      1
#define GB_T_BYTE         2
#define GB_T_SHORT        3
#define GB_T_INTEGER      4
#define GB_T_LONG         5
#define GB_T_SINGLE       6
#define GB_T_FLOAT        7
#define GB_T_DATE         8
#define GB_T_STRING       9
#define GB_T_CSTRING      10
#define GB_T_POINTER      11
#define GB_T_VARIANT      12
#define GB_T_NULL         15
#define GB_T_OBJECT       16


/* This type represents a Gambas datatype identifier */

typedef
	uintptr_t GB_TYPE;


/* This opaque type represents a Gambas class identifier */

typedef
	GB_TYPE GB_CLASS;


/* This structure represents the base of every Gambas object.
   It must be placed in the beginning of all object structure defined
   in a component.
*/

typedef
	struct {
		GB_CLASS klass;
		intptr_t ref;
		}
	GB_BASE;


/* Gambas STRING datatype definition */

typedef
  struct {
    GB_TYPE type;
    struct {
      char *addr;
      int start;
      int len;
      } value;
    #if __WORDSIZE == 64
		intptr_t _reserved;
		#endif
    }
  GB_STRING;


/* Gambas INTEGER datatype definition */

typedef
  struct {
    GB_TYPE type;
    int value;
    #if __WORDSIZE == 64
    int _pad;
    #endif
		intptr_t _reserved[2];
    }
  GB_INTEGER;


/* Gambas LONG datatype definition */

typedef
  struct {
    GB_TYPE type;
		#if __WORDSIZE == 32
		int _pad;
		#endif
    int64_t value;
    #if __WORDSIZE == 64
    intptr_t _reserved[2];
    #endif
    }
  GB_LONG;


/* Gambas POINTER datatype definition */

typedef
  struct {
    GB_TYPE type;
    intptr_t value;
    intptr_t _reserved[2];
    }
  GB_POINTER;


/* Gambas BOOLEAN datatype definition */

typedef
  struct {
    GB_TYPE type;
    int value;
    #if __WORDSIZE == 64
    int _pad;
    #endif
		intptr_t _reserved[2];
    }
  GB_BOOLEAN;


/* Gambas SINGLE datatype definition */

typedef
  struct {
    GB_TYPE type;
    float value;
    #if __WORDSIZE == 64
    float _pad;
    #endif
		intptr_t _reserved[2];
    }
  GB_SINGLE;


/* Gambas FLOAT datatype definition */

typedef
  struct {
    GB_TYPE type;
    #if __WORDSIZE == 32
		int _pad;
		#endif
    double value;
    #if __WORDSIZE == 64
    intptr_t _reserved[2];
    #endif
    }
  GB_FLOAT;


/* Gambas DATE datatype definition */

typedef
  struct {
    int date;
    int time;
    }
  GB_DATE_VALUE;

typedef
  struct {
    GB_TYPE type;
		GB_DATE_VALUE value;
    #if __WORDSIZE == 64
    intptr_t _reserved[2];
    #else
    int _reserved;
    #endif
    }
  GB_DATE;


/* Gambas OBJECT datatype definition */

typedef
  struct {
    GB_TYPE type;
    void *value;
    intptr_t _reserved[2];
    }
  GB_OBJECT;


/* Gambas VARIANT datatype definition */

typedef
  struct {
    GB_TYPE type;
		union {
			char _boolean;
			unsigned char _byte;
			short _short;
			int _integer;
			int64_t _long;
			float _single;
			double _float;
			GB_DATE_VALUE _date;
			char *_string;
			intptr_t _pointer;
			void *_object;
			int64_t data;
			}
			value;
		}
	PACKED
  GB_VARIANT_VALUE;

typedef
  struct {
    GB_TYPE type;
		GB_VARIANT_VALUE value;
		#if __WORDSIZE == 64
		int64_t _pad;
		#endif
    }
	PACKED
  GB_VARIANT;


/* Gambas common value definition */

typedef
	union {
		GB_TYPE type;
		GB_BOOLEAN _boolean;
		GB_INTEGER _integer;
		GB_LONG _long;
		GB_SINGLE _single;
		GB_FLOAT _float;
		GB_DATE _date;
		GB_STRING _string;
		GB_POINTER _pointer;
		GB_OBJECT _object;
		GB_VARIANT _variant;
		}
	GB_VALUE;


/* Predefined errors constants */

#define GB_ERR_TYPE       ((char *)6)
#define GB_ERR_NSYMBOL    ((char *)11)
#define GB_ERR_NPROPERTY  ((char *)17)
#define GB_ERR_ARG        ((char *)20)
#define GB_ERR_BOUND      ((char *)21)
#define GB_ERR_ZERO       ((char *)26)


/* Gambas description start macro */

#define GB_DECLARE(name, size) \
	{ name, (intptr_t)GB_VERSION, (intptr_t)size }


/* Gambas description end macro */

#define GB_END_DECLARE  { (char *)0 }


/* Special description identifiers */

#define GB_VIRTUAL_CLASS_ID     ((char *)1)
#define GB_HOOK_CHECK_ID        ((char *)2)
#define GB_NOT_CREATABLE_ID     ((char *)3)
#define GB_AUTO_CREATABLE_ID    ((char *)4)
#define GB_INHERITS_ID          ((char *)5)


/* Description hook macros */

//#define GB_HOOK_NEW(hook)    { GB_HOOK_NEW_ID, (int)hook }
//#define GB_HOOK_FREE(hook)   { GB_HOOK_FREE_ID, (int)hook }
#define GB_HOOK_CHECK(hook)  { GB_HOOK_CHECK_ID, (intptr_t)hook }


/* Virtual class description macro */

#define GB_VIRTUAL_CLASS() { GB_VIRTUAL_CLASS_ID }, { GB_NOT_CREATABLE_ID }


/* Not creatable class macro */

#define GB_NOT_CREATABLE() { GB_NOT_CREATABLE_ID }


/* Auto creatable class macro */

#define GB_AUTO_CREATABLE() { GB_AUTO_CREATABLE_ID }


/* Symbol description prefixes */

#define GB_PROPERTY_ID          'p'
#define GB_METHOD_ID            'm'
#define GB_CONSTANT_ID          'C'
#define GB_EVENT_ID             ':'
#define GB_ENUM_ID              '#'
#define GB_STATIC_PROPERTY_ID   'P'
#define GB_STATIC_METHOD_ID     'M'


/* Symbol description macros */

#define GB_CONSTANT(symbol, type, value) \
	{ "C" symbol, (intptr_t)type, (intptr_t)value }

#define GB_PROPERTY(symbol, type, proc) \
	{ "p" symbol, (intptr_t)type, (intptr_t)proc }

#define GB_PROPERTY_READ(symbol, type, proc) \
	{ "r" symbol, (intptr_t)type, (intptr_t)proc }

#define GB_PROPERTY_SELF(symbol, type) \
	{ "r" symbol, (intptr_t)type, (intptr_t)(-1) }

#define GB_METHOD(symbol, type, exec, signature) \
	{ "m" symbol, (intptr_t)type, (intptr_t)exec, (intptr_t)signature }

#define GB_EVENT(symbol, type, signature, id) \
	{ "::" symbol, (intptr_t)type, (intptr_t)id, (intptr_t)signature }

#define GB_STATIC_PROPERTY(symbol, type, proc) \
	{ "P" symbol, (intptr_t)type, (intptr_t)proc }

#define GB_STATIC_PROPERTY_READ(symbol, type, proc) \
	{ "R" symbol, (intptr_t)type, (intptr_t)proc }

#define GB_STATIC_PROPERTY_SELF(symbol, type) \
	{ "R" symbol, (intptr_t)type, (-1) }

#define GB_STATIC_METHOD(symbol, type, exec, signature) \
	{ "M" symbol, (intptr_t)type, (intptr_t)exec, (intptr_t)signature }

#define GB_STATIC_FAST_METHOD(symbol, type, exec, signature) \
	{ "M!" symbol, (intptr_t)type, (intptr_t)exec, (intptr_t)signature }

#define GB_INHERITS(symbol) \
	{ GB_INHERITS_ID, (intptr_t)symbol }
	
#define GB_INTERFACE(symbol, pointer) \
	{ "C_@" symbol, (intptr_t)"p", (intptr_t)pointer } 


/* Method implementation begin macro */

#define BEGIN_METHOD(_name, par) \
typedef \
	struct { \
		par; \
		} \
		_##_name; \
\
void _name(void *_object, void *_param) \
{ \
_##_name *_p = (_##_name *)_param;


/* Parameter-less Method implementation begin macro */

#define BEGIN_METHOD_VOID(_name) \
void _name(void *_object, void *_param) { \


/* Parameter access macro */

#define ARG(_name) (&(_p)->_name)


/* Testing if a argument is missing */

#define MISSING(_name) ((_p)->_name.type == GB_T_VOID)


/* Method implementation end macro */

#define END_METHOD }


/* Macro used for calling a parameter-less implementation method */

#define CALL_METHOD_VOID(_name) _name(_object, NULL)


/* Property implementation begin macro */

#define BEGIN_PROPERTY(_name) \
void _name(void *_object, void *_param) {


/* Macro indicating if the property implementation is called for reading or writing */

#define READ_PROPERTY  (_param == NULL)


/* Macro to get the value written to a property */

#define PROP(_type) ((_type *)_param)


/* Property implementation end macro */

#define END_PROPERTY }


/* Macros to get the value of an argument or a property */

#define VALUE(_arg) ((_arg)->value)
#define VARG(_p) VALUE(ARG(_p))
#define VPROP(_p) VALUE(PROP(_p))


/* Macros to get a string argument */

#define STRING(_arg) (VARG(_arg).addr + VARG(_arg).start)
#define LENGTH(_arg) (VARG(_arg).len)


/* Macros to get a string property */

#define PSTRING() (VPROP(GB_STRING).addr + VPROP(GB_STRING).start)
#define PLENGTH() (VPROP(GB_STRING).len)


/* Macro to get an optional argument */

#define VARGOPT(_arg, _default) (MISSING(_arg) ? (_default) : VARG(_arg))


/* Casting macro. Usable only in an implementation function */

#define OBJECT(type) ((type *)_object)


/* Macro for returning itself. Usable only in an implementation function */

#define RETURN_SELF()  GB.ReturnSelf(_object)


/* Macro for declaring a variable used for storing an event identifier */

#define DECLARE_EVENT(_event) static int _event


/* Macro to help accessing enumeration index. Use only in an enumeration method implementation */

#define ENUM(_type)  (*((_type *)GB.GetEnum()))


/* Structure used for describing a class */

typedef
	struct {
		const char *name;
		intptr_t val1;
		intptr_t val2;
		intptr_t val3;
		intptr_t val4;
		intptr_t val5;
		}
	GB_DESC;


/* Type of a method implementation function */

typedef
	void GB_METHOD_FUNC(void *, void *);


/* Type of a property implementation function */

typedef
	void GB_PROPERTY_FUNC(void *, void *);


/* Macro for declaring a method implementation function */

#define DECLARE_METHOD(_method) GB_METHOD_FUNC _method


/* Macro for declaring a property implementation function */

#define DECLARE_PROPERTY(_property) GB_PROPERTY_FUNC _property


/* Constants used with the GB.Hook() API function */

#define GB_HOOK_MAX 10

#define GB_HOOK_MAIN      1
#define GB_HOOK_LOOP      2
#define GB_HOOK_WAIT      3
#define GB_HOOK_TIMER     4
#define GB_HOOK_LANG      5
#define GB_HOOK_WATCH     6
#define GB_HOOK_POST      7
#define GB_HOOK_QUIT      8
#define GB_HOOK_ERROR     9
#define GB_HOOK_TIMEOUT   10

/* Macro for calling the previous hook */

#define CALL_HOOK_MAIN(_hook, _pargc, _pargv) do { if (_hook) { ((void (*)(int *, char ***))(_hook))((_pargc), (_pargv)); } } while (0);

/* Constants that represent interpreter signals catched by GB_SIGNAL function */

#define GB_SIGNAL_DEBUG_BREAK         1
#define GB_SIGNAL_DEBUG_CONTINUE      2
#define GB_SIGNAL_DEBUG_FORWARD				3


/* Constants used with the GB.Watch() API function */

#define GB_WATCH_NONE         0
#define GB_WATCH_READ         1
#define GB_WATCH_WRITE        2


/* Type of a generic callback */

typedef
	void (*GB_CALLBACK)();


/* Type of a watch callback function */

typedef
	void (*GB_WATCH_CALLBACK)(int, int, intptr_t);


/* Type of the GB.SubstString() callback */

typedef
	void (*GB_SUBST_CALLBACK)(int, char **, int *);


/* Type of the GB.SubstStringAdd() callback */

typedef
	void (*GB_SUBST_ADD_CALLBACK)(int);


/* Type of the GB.BrowseProject() callback */

typedef
	void (*GB_BROWSE_CALLBACK)(const char *, int64_t);


/* Type of a timer callback */

typedef
	int (*GB_TIMER_CALLBACK)(intptr_t);


/* Type of a signal callback */

typedef
	void (*GB_SIGNAL_CALLBACK)(int, intptr_t);


/* Type of a posted function */

typedef
	void (*GB_POST_FUNC)();


/* A structure for the components of a date */

typedef
	struct {
		short year;
		short month;
		short day;
		short hour;
		short min;
		short sec;
		short weekday;
		short msec;
		}
	GB_DATE_SERIAL;


/* Opaque type of a Gambas interpreted or native function */

typedef
	struct {
		void *object;
		void *desc;
		}
	GB_FUNCTION;

#define GB_FUNCTION_IS_VALID(_func) ((_func)->desc)


/* Opaque type of a Gambas Array */

typedef
	void *GB_ARRAY;


/* Opaque type of a Gambas Collection */

typedef
	void *GB_COLLECTION;


/* Opaque type of a Gambas Collection iterator */

typedef
	struct {
		void *iter1;
		void *iter2;
		}
	GB_COLLECTION_ITER;


/* opaque type of an hash table */

typedef
	void *GB_HASHTABLE;


/* hash table enumeration function */

typedef
	void (*GB_HASHTABLE_ENUM_FUNC)(void *);


/* opaque type for a Stream object */

struct GB_STREAM;

typedef
	struct {
		int (*open)(struct GB_STREAM *stream, const char *path, int mode, void *data);
		int (*close)(struct GB_STREAM *stream);
		int (*read)(struct GB_STREAM *stream, char *buffer, int len);
		int (*getchar)(struct GB_STREAM *stream, char *buffer);
		int (*write)(struct GB_STREAM *stream, char *buffer, int len);
		int (*seek)(struct GB_STREAM *stream, int64_t pos, int whence);
		int (*tell)(struct GB_STREAM *stream, int64_t *pos);
		int (*flush)(struct GB_STREAM *stream);
		int (*eof)(struct GB_STREAM *stream);
		int (*lof)(struct GB_STREAM *stream, int64_t *len);
		int (*handle)(struct GB_STREAM *stream);
		}
	GB_STREAM_DESC;

typedef
	struct {
		GB_STREAM_DESC *desc;
		int64_t _reserved;
		intptr_t _reserved2;
		intptr_t _reserved3;
		}
	GB_STREAM_BASE;

typedef
	struct GB_STREAM {
		GB_STREAM_DESC *desc;
		int64_t _reserved;
		intptr_t _reserved2;
		intptr_t _reserved3;
		void *tag;
		#if __WORDSIZE == 64
		int _free[4];
		#else
		int _free[5];
		#endif
		GB_VARIANT_VALUE _reserved4;
		}
	GB_STREAM;


/* Constants used by the GB.NumberFromString() API function */

#define GB_NB_READ_INTEGER    1
#define GB_NB_READ_LONG       2
#define GB_NB_READ_INT_LONG   3
#define GB_NB_READ_FLOAT      4
#define GB_NB_READ_ALL        7
#define GB_NB_READ_HEX_BIN    8
#define GB_NB_LOCAL           16


/* Constants used by the GB.Collection.New() and GB.HashTable.New() API function */

#define GB_COMP_BINARY      0
#define GB_COMP_NOCASE      1


/* Constant used by GB.ConvString to convert to 32 bits Unicode (that needs some special processing) */

#define GB_SC_UNICODE ((char *)-1)


/* Opaque type for a SubCollection object */

typedef
	void *GB_SUBCOLLECTION;


/* SubCollection description */

typedef
	struct {
		char *klass;
		void *(*get)(void *, const char *);
		int (*exist)(void *, const char *);
		void (*list)(void *, char ***);
		void (*release)(void *, void *);
	}
	GB_SUBCOLLECTION_DESC;


/* Timer object */

typedef
	struct {
		GB_BASE object;
		intptr_t id;
		intptr_t tag;
		int delay;
		#if __WORDSIZE == 64
		int _pad;
		#endif
		GB_TIMER_CALLBACK callback;
		}
	PACKED
	GB_TIMER;

/* Structure for GB.RaiseCatch handler */

typedef
	struct {
		void (*callback)(intptr_t);
		intptr_t data;
		void *old;
	}
	GB_RAISE_HANDLER;

/* A macro for preventing gcc from warning about breaks in the
   strict aliasing rules */
	
#define POINTER(_pointer) (void **)(void *)_pointer

/* For classes that implements arithmetic operators (e.g. complex numbers...) */

typedef
	struct {
		int (*equal)(void *, void *);
		int (*equalf)(void *, double);
		int (*comp)(void *, void *);
		int (*compf)(void *, double);
		void *(*add)(void *, void *);
		void *(*addf)(void *, double);
		void *(*sub)(void *, void *);
		void *(*subf)(void *, double);
		void *(*mul)(void *, void *);
		void *(*mulf)(void *, double);
		void *(*div)(void *, void *);
		void *(*divf)(void *, double);
		void *(*idivf)(void *, double);
		void *(*neg)(void *);
		void *(*pow)(void *, void *);
		void *(*powf)(void *, double);
		double (*abs)(void *);
		void *(*max)(void *, void *);
		void *(*min)(void *, void *);
	}
	PACKED
	GB_OPERATOR_DESC;


/* Gambas Application Programming Interface */

typedef
	struct {
		intptr_t version;

		bool (*GetInterface)(const char *, int, void *);

		void *(*Hook)(int, void *);

		struct {
			bool (*Load)(const char *);
			bool (*Exist)(const char *);
			char *(*Current)(void);
			bool (*GetInfo)(const char *, void **);
			void (*Signal)(int, void *);
		}
		Component;

		void (*Push)(int, ...);
		bool (*GetFunction)(GB_FUNCTION *, void *, const char *, const char *, const char *);
		GB_VALUE *(*Call)(GB_FUNCTION *, int, int);
		void *(*GetClassInterface)(GB_CLASS, const char *);
		void (*GetProperty)(void *, const char *);
		void (*SetProperty)(void *, const char *, ...);

		bool (*Loop)(int);
		void (*Post)(GB_CALLBACK, intptr_t);
		void (*Post2)(GB_CALLBACK, intptr_t, intptr_t);
		GB_TIMER *(*Every)(int, GB_TIMER_CALLBACK, intptr_t);
		bool (*Raise)(void *, int, int, ...);
		void (*RaiseBegin)(GB_RAISE_HANDLER *handler);
		void (*RaiseEnd)(GB_RAISE_HANDLER *handler);
		void (*RaiseLater)(void *, int);
		void (*CheckPost)(void);
		bool (*CanRaise)(void *, int);
		int (*GetEvent)(GB_CLASS, const char *);
		char *(*GetLastEventName)();
		void (*RaiseTimer)(void *);
		bool (*Stopped)(void);

		int (*NParam)(void);
		bool (*Conv)(GB_VALUE *, GB_TYPE);
		char *(*GetUnknown)(void);
		
		void (*Error)(const char *, ...);
		void (*Propagate)(void);

		GB_CLASS (*GetClass)(void *);
		char *(*GetClassName)(void *);
		bool (*ExistClass)(const char *);
		GB_CLASS (*FindClass)(const char *);
		bool (*ExistClassLocal)(const char *);
		GB_CLASS (*FindClassLocal)(const char *);
		bool (*Is)(void *, GB_CLASS);
		void (*Ref)(void *);
		void (*Unref)(void **);
		void (*UnrefKeep)(void **, int);
		void (*Detach)(void *);
		void (*Attach)(void *, void *, const char *);
		void *(*Parent)(void *);
		void *(*New)(GB_CLASS, char *, void *);
		void *(*AutoCreate)(GB_CLASS, int);
		bool (*CheckObject)(void *);

		void *(*GetEnum)();
		void (*StopEnum)();
		void (*ListEnum)(void *);
		bool (*NextEnum)();
		void (*StopAllEnum)(void *);

		void (*Return)(GB_TYPE, ...);
		void (*ReturnInteger)(int);
		void (*ReturnLong)(int64_t);
		void (*ReturnPointer)(void *);
		void (*ReturnBoolean)(int);
		void (*ReturnDate)(GB_DATE *);
		void (*ReturnObject)(void *);
		void (*ReturnNull)(void);
		void (*ReturnFloat)(double);
		void (*ReturnVariant)(GB_VARIANT_VALUE *);
		void (*ReturnConvVariant)();
		void (*ReturnPtr)(GB_TYPE, void *);
		void (*ReturnSelf)(void *);

		void (*ReturnString)(char *);
		void (*ReturnVoidString)(void);
		void (*ReturnConstString)(const char *, int);
		void (*ReturnConstZeroString)(const char *);
		void (*ReturnNewString)(const char *, int);
		void (*ReturnNewZeroString)(const char *);

		char *(*NewString)(const char *, int);
		char *(*NewZeroString)(const char *);
		char *(*TempString)(const char *, int);
		void (*FreeString)(char **);
		char *(*ExtendString)(char *, int);
		char *(*AddString)(char *, const char *, int);
		char *(*AddChar)(char *, char);
		int (*StringLength)(char *);
		char *(*ToZeroString)(GB_STRING *);
		bool (*MatchString)(const char *, int, const char *, int);
		bool (*NumberFromString)(int, const char *, int, GB_VALUE *);
		bool (*NumberToString)(int, double, const char *, char **, int *);
		char *(*Translate)(const char *);

		char *(*SubstString)(const char *, int, GB_SUBST_CALLBACK);
		char *(*SubstStringAdd)(const char *, int, GB_SUBST_ADD_CALLBACK);
		void (*SubstAddCallback)(const char *, int);
		bool (*ConvString)(char **, const char *, int, const char *, const char *);
		char *(*FileName)(char *, int);
		char *(*RealFileName)(char *, int);

		bool (*LoadFile)(const char *, int, char **, int *);
		void (*ReleaseFile)(char *, int);
		char *(*TempDir)(void);
		char *(*TempFile)(const char *);
		bool (*CopyFile)(const char *, const char *);
		void (*BrowseProject)(GB_BROWSE_CALLBACK);

		void (*Store)(GB_TYPE, GB_VALUE *, void *);
		void (*StoreString)(GB_STRING *, char **);
		void (*StoreObject)(GB_OBJECT *, void **);
		void (*StoreVariant)(GB_VARIANT *, void *);
		void (*ReadValue)(GB_VALUE *, void *, GB_TYPE);
		void (*BorrowValue)(GB_VALUE *);
		void (*ReleaseValue)(GB_VALUE *);

		GB_DATE_SERIAL *(*SplitDate)(GB_DATE *);
		bool (*MakeDate)(GB_DATE_SERIAL *, GB_DATE *);
		void (*MakeDateFromTime)(int, int, GB_DATE *);
		bool (*GetTime)(double *, int);

		void (*Watch)(int, int, void *, intptr_t);

		GB_VALUE *(*Eval)(void *, void *);

		void (*Alloc)(void **, int);
		void (*Free)(void **);
		void (*Realloc)(void **, int);

		void (*NewArray)(void *, int, int);
		void (*FreeArray)(void *);
		int (*Count)(void *);
		void *(*Add)(void *);
		void *(*Insert)(void *, int, int);
		void (*Remove)(void *, int, int);

		struct {
			void (*New)(GB_SUBCOLLECTION *, GB_SUBCOLLECTION_DESC *, void *);
			void (*Add)(void *, const char *, int, void *);
			void (*Remove)(void *, const char *, int);
			void *(*Get)(void *, const char *, int);
			void *(*Container)(void *);
			}
		SubCollection;

		int (*ToLower)(int);
		int (*ToUpper)(int);
		int (*StrCaseCmp)(const char *, const char *);
		int (*StrNCaseCmp)(const char *, const char *, int);

		struct {
			char *(*Name)(void);
			char *(*Title)(void);
			char *(*Version)(void);
			char *(*Path)(void);
			GB_CLASS (*StartupClass)(void);
			}
		Application;

		struct {
			char *(*Charset)(void);
			char *(*Language)(void);
			char *(*DomainName)(void);
			bool (*IsRightToLeft)(void);
			char *(*Path)(void);
			}
		System;

		struct {
			void (*New)(GB_ARRAY *, GB_TYPE, int);
			int (*Count)(GB_ARRAY);
			void *(*Add)(GB_ARRAY);
			void *(*Get)(GB_ARRAY, int);
			GB_TYPE (*Type)(GB_ARRAY);
			}
		Array;

		struct {
			void (*New)(GB_COLLECTION *, int);
			int (*Count)(GB_COLLECTION);
			bool (*Set)(GB_COLLECTION, const char *, int, GB_VARIANT *);
			bool (*Get)(GB_COLLECTION, const char *, int, GB_VARIANT *);
			bool (*Enum)(GB_COLLECTION, GB_COLLECTION_ITER *, GB_VARIANT *, char **key, int *len);
			}
		Collection;

		struct {
			void (*New)(GB_HASHTABLE *, int);
			void (*Free)(GB_HASHTABLE *);
			int (*Count)(GB_HASHTABLE);
			void (*Add)(GB_HASHTABLE, const char *, int, void *);
			void (*Remove)(GB_HASHTABLE, const char *, int);
			bool (*Get)(GB_HASHTABLE, const char *, int, void **);
			void (*Enum)(GB_HASHTABLE, GB_HASHTABLE_ENUM_FUNC);
			}
		HashTable;

		struct {
			GB_STREAM *(*Get)(void *object);
			void (*SetBytesRead)(GB_STREAM *stream, int length);
			void (*SetSwapping)(GB_STREAM *stream, int swap);
			bool (*Block)(GB_STREAM *stream, int block);
			int (*Read)(GB_STREAM *stream, void *addr, int len);
			int (*Write)(GB_STREAM *stream, void *addr, int len);
			}
		Stream;

		struct {
			void (*Start)(int length);
			char *(*End)();
			void (*Add)(const char *src, int len);
			}
		String;
		
		struct {
			char *(*GetCurrentPosition)();
			void (*EnterEventLoop)();
			void (*LeaveEventLoop)();
			}
		Debug;
		
		struct {
			GB_SIGNAL_CALLBACK *(*Register)(int signum, void (*func)(int, intptr_t), intptr_t data);
			void (*Unregister)(int signum, GB_SIGNAL_CALLBACK *cb);
		}
		Signal;
		
		}
	GB_INTERFACE;


/*

  Special methods that can be declared in a class

  _get        array reading operator
  _put        array writing operator
  _new        constructor
  _free       destructor
  _next       next iteration of an enumeration
  _call       called when the object or the class is used as a function
  _unknown    called when the name of the property or method is unknown

*/

/*

  Syntax of a method or event signature

  Gambas datatype      String representation

  BOOLEAN              b
  INTEGER              i
  LONG                 l
  FLOAT                f
  DATE                 d
  STRING               s
  VARIANT              v
  OBJECT               o
  POINTER              p
  Any class            ClassName;

*/

#ifndef NO_GAMBAS_CASE_REPLACEMENT

/* Replacements for case unsensitive comparisons.
   They ensure that case comparison does not use current locale,
   otherwise Turkish speakers will have problems!
*/

#include <string.h>
#include <ctype.h>

#ifdef tolower
#undef tolower
#endif
#ifdef toupper
#undef toupper
#endif
#ifdef strcasecmp
#undef strcasecmp
#endif
#ifdef strncasecmp
#undef strncasecmp
#endif

#define strcasecmp GB.StrCaseCmp
#define strncasecmp GB.StrNCaseCmp
#define toupper GB.ToUpper
#define tolower GB.ToLower

#endif

#endif
