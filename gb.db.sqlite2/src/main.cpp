/***************************************************************************

	main.cpp

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

#define __MAIN_C

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <libgen.h> /* For basename function */
#include <pwd.h>    /* For passwd file functions */
#include <grp.h>    /* For group file functions */
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>

#include "sqlitedataset.h"

#include "main.h"

extern "C" {
GB_INTERFACE GB EXPORT;
DB_INTERFACE DB EXPORT;
} //end extern "C"

static char _buffer[32];

static int _print_query = FALSE;


/*****************************************************************************

	The driver interface

*****************************************************************************/

DECLARE_DRIVER(_driver, "sqlite2");


/* Internal function to convert a database type into a Gambas type */

static GB_TYPE conv_type(int type)
{
	switch(type)
	{
		case ft_Boolean:
			return GB_T_BOOLEAN;
		case ft_Short:
		case ft_UShort:
		case ft_Long:
		case ft_ULong:
			return GB_T_INTEGER;

		case ft_Float:
		case ft_Double:
			return GB_T_FLOAT;

		case ft_LongDouble:
			return GB_T_LONG;

		case ft_Date:
			return GB_T_DATE;

		case ft_String:
		case ft_WideString:
		case ft_Char:
		case ft_WChar:
			return GB_T_STRING;

		default:
			return GB_T_STRING;

	}
}


/* Internal function to convert a database value into a Gambas variant value */

static void conv_data(const char *data, GB_VARIANT_VALUE *val, fType type)
{
	GB_VALUE conv;
	GB_DATE_SERIAL date;
	double sec;

	switch (type)
	{
	case ft_Boolean:

			val->type = GB_T_BOOLEAN;
			if (data[0] == 't' || data[0] == 'T')
				val->value._boolean = -1;
			else
				val->value._boolean = atoi(data) ? -1 : 0;
			break;

		case ft_Short:
		case ft_UShort:
		case ft_Long:
		case ft_ULong:

			GB.NumberFromString(GB_NB_READ_INTEGER, data, strlen(data), &conv);

			val->type = GB_T_INTEGER;
			val->value._integer = conv._integer.value;

			break;

		case ft_LongDouble:

			GB.NumberFromString(GB_NB_READ_LONG, data, strlen(data), &conv);

			val->type = GB_T_LONG;
			val->value._long = conv._long.value;

			break;

		case ft_Float:
		case ft_Double:

			GB.NumberFromString(GB_NB_READ_FLOAT, data, strlen(data), &conv);

			val->type = GB_T_FLOAT;
			val->value._float = conv._float.value;

			break;

		case ft_Date:

			memset(&date, 0, sizeof(date));

			switch (strlen(data))
			{
				case 14:
					sscanf(data, "%4d%2d%2d%2d%2d%lf", &date.year, &date.month,
								&date.day, &date.hour, &date.min, &sec);
					date.sec = (short) sec;
					date.msec = (short) ((sec - date.sec) * 1000 + 0.5);
					break;
				case 12:
					sscanf(data, "%2d%2d%2d%2d%2d%lf", &date.year, &date.month,
								&date.day, &date.hour, &date.min, &sec);
					date.sec = (short) sec;
					date.msec = (short) ((sec - date.sec) * 1000 + 0.5);
					break;
				case 10:
					if (sscanf(data, "%4d-%2d-%2d", &date.year, &date.month,
										&date.day) != 3)
					{
						if (sscanf(data, "%4d/%2d/%2d", &date.year, &date.month,
											&date.day) != 3)
						{
							if (sscanf(data, "%4d:%2d:%lf", &date.hour, &date.min,
												&sec) == 3)
							{
								date.sec = (short) sec;
								date.msec = (short) ((sec - date.sec) * 1000 + 0.5);
							}
							else
							{
								sscanf(data, "%2d%2d%2d%2d%2d", &date.year,
											&date.month, &date.day, &date.hour, &date.min);
							}
						}
					}

					break;
				case 8:
					if (sscanf(data, "%4d%2d%2d", &date.year, &date.month,
										&date.day) != 3)
					{
						sscanf(data, "%2d/%2d/%2d", &date.year, &date.month,
									&date.day);
					}
					break;
				case 6:
					sscanf(data, "%2d%2d%2d", &date.year, &date.month, &date.day);
					break;
				case 4:
					sscanf(data, "%2d%2d", &date.year, &date.month);
					break;
				case 2:
					sscanf(data, "%2d", &date.year);
					break;
				default:
					sscanf(data, "%4d-%2d-%2d %2d:%2d:%lf", &date.year,
								&date.month, &date.day, &date.hour, &date.min, &sec);
					date.sec = (short) sec;
					date.msec = (short) ((sec - date.sec) * 1000 + 0.5);
			}
			if (date.year < 100)
				date.year += 1900;

			GB.MakeDate(&date, (GB_DATE *) & conv);

			val->type = GB_T_DATE;
			val->value._date.date = conv._date.value.date;
			val->value._date.time = conv._date.value.time;

			break;

		case ft_String:
		case ft_WideString:
		case ft_Char:
		case ft_WChar:
		default:
			
			val->type = GB_T_CSTRING;
			val->value._string = (char *)data;

			break;
	}

}

/* Internal function to substitute the table name into a query */

static char *query_param[3];

static void query_get_param(int index, char **str, int *len, char quote)
{
	if (index > 3)
		return;

	index--;
	*str = query_param[index];
	*len = strlen(*str);

	if (quote == '\'')
	{
		*str = DB.QuoteString(*str, *len, quote);
		*len = GB.StringLength(*str);
	}
}

/* Internal function to run a query */

static int do_query(DB_DATABASE *db, const char *error, Dataset **pres,
										const char *qtemp, int nsubst, ...)
{
	SqliteDatabase *conn = (SqliteDatabase *)db->handle;
	va_list args;
	int i;
	const char *query;
	Dataset *res = conn->CreateDataset();
	int ret;

	if (nsubst)
	{
		va_start(args, nsubst);
		if (nsubst > 3)
			nsubst = 3;
		for (i = 0; i < nsubst; i++)
			query_param[i] = va_arg(args, char *);

		query = DB.SubstString(qtemp, 0, query_get_param);
	}
	else
		query = qtemp;

	if (_print_query)
	{
		_print_query = FALSE;
	}

	DB.Debug("sqlite2","%p: %s", conn, query);

	if (strncasecmp("select",query,6) == 0){

		if(res->query(query)){
					ret = FALSE;
					if (pres){
						*pres = res;
					}
				}
				else {
					ret = TRUE;
					GB.Error(error, conn->getErrorMsg());
					}
	}
	else {
		if(res->exec(query)){
					ret = FALSE;
					if (pres){
						*pres = res;
					}
				}
				else {
					ret = TRUE;
					GB.Error(error, conn->getErrorMsg());
				}
	}

	if (!pres)
		res->close();
	
	if (ret)
		db->error = conn->lastError();
	else
		db->error = 0;
	
	return ret;
}

/* Internal function to check whether a file is a sqlite database file */
static bool is_database_file(const char *filename)
{
	/*                   SQLite databases start with the string:
	*                  ** This file contains an SQLite 2.1 database **
	*                                  */
	FILE* fp;
	bool res;
	char magic_text[48];

	fp = fopen(filename, "r");
	if (!fp)
		return false;

	res = fread(magic_text, 1, 47, fp) == 47;
	fclose(fp);

	if (!res)
		return false;

	magic_text[47] = '\0';

	if (strcmp(magic_text, "** This file contains an SQLite 2.1 database **"))
		return false;

	return true;
}

/* Internal function to locate database and return full qualified */
/* path. */
static char *find_database(const char *name, const char *hostName)
{
	char *dbhome = NULL;
	char *fullpath = NULL;
	char *path;

	/* Does Name includes fullpath */
	if (*name == '/')
	{
		if (is_database_file(name))
			return (char *)name;
	}

	/* Hostname contains home area */
	fullpath = GB.NewZeroString(hostName);
	fullpath = GB.AddChar(fullpath, '/');
	fullpath = GB.AddString(fullpath, name, 0);
	
	path = GB.FileName(fullpath, GB.StringLength(fullpath));
	GB.FreeString(&fullpath);
	
	if (is_database_file(path))
		return path;

	/* Check the GAMBAS_SQLITE_DBHOME setting */
	dbhome = getenv("GAMBAS_SQLITE_DBHOME");

	if (dbhome != NULL)
	{
		fullpath = GB.NewZeroString(dbhome);
		fullpath = GB.AddChar(fullpath, '/');
		fullpath = GB.AddString(fullpath, name, 0);

		path = GB.FileName(fullpath, GB.StringLength(fullpath));
		GB.FreeString(&fullpath);

		if (is_database_file(path))
			return path;
	}

	fullpath = GB.NewZeroString(GB.TempDir());
	fullpath = GB.AddString(fullpath, "/sqlite/", 0);
	fullpath = GB.AddString(fullpath, name, 0);
	GB.FreeStringLater(fullpath);

	if (is_database_file(fullpath))
		return fullpath;

	return NULL;
}


/* Internal function to return database home directory */
/* - GAMBAS_SQLITE_HOMEDB if set or temporary directory /tmp/gambas.%pid%/ */

static char *GetDatabaseHome()
{
	char *env = NULL;
	char *dbhome = NULL;
	
	GB.Alloc(POINTER(&dbhome), MAX_PATH);

	/* Check for Environment variable */

	env = getenv("GAMBAS_SQLITE_DBHOME");

	/* if not set then set to current working directory */
	if (env == NULL){
			/*
		if (getcwd(dbhome, MAX_PATH) == NULL){
				GB.Error("Unable to get databases: &1", "Can't find current directory");
				GB.Free((void **)&dbhome);
				return NULL;
		}
		*/

		sprintf(dbhome, "%s/sqlite", GB.TempDir());
	}
	else {
		strcpy(dbhome, env);
	}

	return dbhome;
}


// Internal function to walk a directory and list files
// Used by database_list

static int WalkDirectory(const char *dir, char ***databases)
{
	DIR *dp;
	struct dirent *entry;
	struct stat statbuf;
	char cwd[MAX_PATH];

	if ((dp = opendir(dir)) == NULL)
		return -1;

	if (getcwd(cwd, MAX_PATH) == NULL)
	{
		fprintf(stderr, "gb.db.sqlite2: warning: getcwd: %s\n", strerror(errno));
		return -1;
	}

	if (chdir(dir))
	{
		fprintf(stderr, "gb.db.sqlite2: warning: chdir: %s\n", strerror(errno));
		return -1;
	}

	while ((entry = readdir(dp)) != NULL) 
	{
		stat(entry->d_name, &statbuf);

		if (S_ISREG(statbuf.st_mode)) 
		{
			if (is_database_file(entry->d_name))
				*(char **)GB.Add(databases) = GB.NewZeroString(entry->d_name);
		}
	}

	// BM: you must call closedir()
	closedir(dp);
	
	if (chdir(cwd))
		fprintf(stderr, "gb.db.sqlite2: warning: chdir: %s\n", strerror(errno));
	
	return GB.Count(databases);
}

/* Internal function to check database version number */
int db_version()
{
	//Check db version
	int dbversion =0;
	unsigned int verMain, verMajor, verMinor;
	sscanf(sqlite_version,"%2u.%2u.%2u", &verMain, &verMajor, &verMinor);
	dbversion = ((verMain * 10000) + (verMajor * 100) + verMinor);
	return dbversion;
}

/*****************************************************************************

	get_quote()

	Returns the character used for quoting object names.

*****************************************************************************/

static const char *get_quote(void)
{
	return QUOTE_STRING;
}


/*****************************************************************************

	open_database()

	Connect to a database.

	<desc> points at a structure describing each connection parameter.
	<db> points at the DB_DATABASE structure that must be initialized.

	This function must return TRUE if the connection has failed.

	The name of the database can be NULL, meaning a default database.

	In Sqlite, there is no such thing as a host.  If this is set then check
	to see whether this is actually a path to a home area. NG 01/04/04

*****************************************************************************/

static int open_database(DB_DESC *desc, DB_DATABASE *db)
{
	SqliteDatabase *conn = new SqliteDatabase();
	char *name = NULL;
	char *db_fullpath = NULL;
	bool memory;

	/* connect by default to memory database */

	if (desc->name)
	{
		name = GB.NewZeroString(desc->name);
		memory = false;
	}
	else
	{
		name = GB.NewZeroString(":memory:");
		memory = true;
	}

	if (desc->host)
		conn->setHostName(desc->host);

	if (memory)
	{
		conn->setDatabase(name);
	}
	else if ((db_fullpath = find_database(name, conn->getHostName())) != NULL)
	{
		conn->setDatabase(db_fullpath);
	}
	else
	{
		GB.Error("Unable to locate database: &1", name);
		GB.FreeString(&name);
		delete conn;
		return TRUE;
	}

	GB.FreeString(&name);

	if ( conn->connect() != DB_CONNECTION_OK)
	{
		GB.Error("Cannot open database: &1", conn->getErrorMsg());
		conn->disconnect();
		delete conn;
		return TRUE;
	}

	/* Character set cannot be set for sqlite. A sqlite re-compile
	* is required.                                             */
	db->charset = GB.NewZeroString(strcmp(sqlite_encoding, "iso8859") == 0 ? "ISO-8859-1" : "UTF-8");

	/* set dbversion */
	db->version = db_version();

	/* flags */
	db->flags.no_table_type = TRUE;
	db->flags.no_serial = TRUE;
	db->flags.no_blob = TRUE;
	db->flags.no_nest = TRUE;
	db->flags.no_collation = TRUE;

	db->db_name_char = ".";
	
	db->handle = conn;
	return FALSE;
}


/*****************************************************************************

	close_database()

	Terminates the database connection.

	<handle> contains the database handle.

*****************************************************************************/

static void close_database(DB_DATABASE *db)
{
	SqliteDatabase *conn = (SqliteDatabase *)db->handle;

	if (conn)
	{
		conn->disconnect();
		delete conn;
	}
}


/*****************************************************************************

	get_collations()

	Return the available collations as a Gambas string array.

*****************************************************************************/

static GB_ARRAY get_collations(DB_DATABASE *db)
{
	return NULL;
}



/*****************************************************************************

	format_value()

	This function transforms a gambas value into a string value that can
	be inserted into a SQL query.

	<arg> points to the value.
	<add> is a callback called to insert the string into the query.

	This function must return TRUE if it translates the value, and FALSE if
	it does not.

	If the value is not translated, then a default translation is used.

*****************************************************************************/

static int format_value(GB_VALUE * arg, DB_FORMAT_CALLBACK add)
{
	char *s;
	int i, l;
	GB_DATE_SERIAL *date;

	switch (arg->type)
	{
		case GB_T_BOOLEAN:
/*Note this is likely to go to a tinyint  */
			if (VALUE((GB_BOOLEAN *) arg))
				add("'1'", 3);
			else
				add("'0'", 3);
			return TRUE;

		case GB_T_STRING:
		case GB_T_CSTRING:

			s = VALUE((GB_STRING *)arg).addr + VALUE((GB_STRING *)arg).start;
			l = VALUE((GB_STRING *)arg).len;

			add("'", 1);

			for (i = 0; i < l; i++, s++)
			{
				add(s, 1);
				if (*s == '\'')
					add(s, 1);
			}

			add("'", 1);
			
			return TRUE;

		case GB_T_DATE:

			date = GB.SplitDate((GB_DATE *) arg);

			l = sprintf(_buffer, "'%04d-%02d-%02d %02d:%02d:%02d",
									date->year, date->month, date->day,
									date->hour, date->min, date->sec);

			add(_buffer, l);

			if (date->msec)
			{
				l = sprintf(_buffer, ".%03d", date->msec);
				add(_buffer, l);
			}

			add("'", 1);

			return TRUE;

		default:
			return FALSE;
	}
}


/*****************************************************************************

	format_blob()

	This function transforms a blob value into a string value that can
	be inserted into a SQL query.

	<blob> points to the DB_BLOB structure.
	<add> is a callback called to insert the string into the query.

*****************************************************************************/

static void format_blob(DB_BLOB *blob, DB_FORMAT_CALLBACK add)
{
}


/*****************************************************************************

	exec_query()

	Send a query to the server and gets the result.

	<handle> is the database handle, as returned by open_database()
	<query> is the query string.
	<result> will receive the result handle of the query.
	<err> is an error message used when the query failed.

	<result> can be NULL, when we don't care getting the result.

*****************************************************************************/

static int exec_query(DB_DATABASE *db, const char *query, DB_RESULT *result, const char *err)
{
	return do_query(db, err, (Dataset **)result, query, 0);
}


/*****************************************************************************

	get_last_insert_id()

	Return the value of the last serial field used in an INSERT statement

	<db> is the database handle, as returned by open_database()

*****************************************************************************/

static int64_t get_last_insert_id(DB_DATABASE *db)
{
	GB.Error("Unsupported feature");
	return -1;
}


/*****************************************************************************

	query_init()

	Initialize an info structure from a query result.

	<result> is the handle of the query result.
	<info> points to the info structure.
	<count> will receive the number of records returned by the query.

	This function must initialize the info->nfield field with the number of
	field in the query result.

*****************************************************************************/

static void query_init(DB_RESULT result, DB_INFO *info, int *count)
{
	Dataset *res = (Dataset *)result;

	if(res)
	{
				*count = res->num_rows();
				info->nfield = res->fieldCount();
	}
	else {
				*count = 0;
	info->nfield = 0;
	}
}


/*****************************************************************************

	query_release()

	Free the info structure filled by query_init() and the result handle.

	<result> is the handle of the query result.
	<info> points to the info structure.
	<invalid> tells if the associated connection has been closed.

*****************************************************************************/

static void query_release(DB_RESULT result, DB_INFO *info, bool invalid)
{
	((Dataset *)result)->close();
}


/*****************************************************************************

	query_fill()

	Fill a result buffer with the value of each field of a record.

	<db> is the database handle, as returned by open_database()
	<result> is the handle of the result.
	<pos> is the index of the record in the result.
	<buffer> points to an array having one element for each field in the
	result.
	<next> is a boolean telling if we want the next row.

	This function must return DB_OK, DB_ERROR or DB_NO_DATA
	
	This function must use GB.StoreVariant() to store the value in the
	buffer.

*****************************************************************************/

#if 0
static int query_fill(DB_DATABASE *db, DB_RESULT result, int pos, GB_VARIANT_VALUE *buffer, int next)
{
	Dataset *res = (Dataset *)result;
	int i;
	char *data;
	GB_VARIANT value;

	if (!next)
		res->seek(pos);/* move to record */
	else
		res->next();

	for ( i=0; i < res->fieldCount(); i++)
	{
			GB.NewZeroString( &data, res->fv(res->fieldName(i)).get_asString().data());

				value.type = GB_T_VARIANT;
				value.value._object.type = GB_T_NULL;

				//if (field->type != FIELD_TYPE_NULL)
				if (data){
						conv_data(data, &value.value, (fType) res->fieldType(i));
				}

	GB.FreeString(&data);
				GB.StoreVariant(&value, &buffer[i]);
	}

	return FALSE;
}
#endif

static int query_fill(DB_DATABASE *db, DB_RESULT result, int pos, GB_VARIANT_VALUE * buffer, int next)
{
	Dataset *res = (Dataset *) result;
	int i;
	const char *data;
	GB_VARIANT value;

	if (!next)
		res->seek(pos);
	else
		res->next();

	for (i = 0; i < res->fieldCount(); i++)
	{
		if (res->fv(i).get_isNull())
			data = NULL;
		else
			data = res->fv(i).get_asString().data();
			
		//fprintf(stderr, "query_fill: %d.%d %s\n", pos, i, data);

		value.type = GB_T_VARIANT;
		value.value.type = GB_T_NULL;

		//if (field->type != FIELD_TYPE_NULL)
		if (data)
			conv_data(data, &value.value, (fType) res->fieldType(i));

		//GB.FreeString(&data);
		GB.StoreVariant(&value, &buffer[i]);
	}

	return DB_OK;
}


/*****************************************************************************

	blob_read()

	Returns the value of a BLOB field.

	<result> is the handle of the result.
	<pos> is the index of the record in the result.
	<blob> points at a DB_BLOB structure that will receive a pointer to the
	data and its length.

	NOTE: this function is always called after query_fill() with the same
	value of <pos>.

*****************************************************************************/

static void blob_read(DB_RESULT result, int pos, int field, DB_BLOB *blob)
{
}


/*****************************************************************************

	field_name()

	Return the name of a field in a result from its index.

	<result> is the result handle.
	<field> is the field index.

*****************************************************************************/

static char *field_name(DB_RESULT result, int field)
{
	return (char *)(((Dataset *)result)->fieldName(field));
}


/*****************************************************************************

	field_index()

	Return the index of a field in a result from its name.

	<Result> is the result handle.
	<name> is the field name.
	<handle> can be ignored by this driver.

*****************************************************************************/

static int field_index(DB_RESULT result, const char *name, DB_DATABASE *db)
{
	char *fld;

	fld = strchr((char *)name, (int)FLD_SEP);
	if (fld){ //Includes table identity
		fld[0] = '.';
	}
	return (((Dataset *)result)->fieldIndex(name));
}


/*****************************************************************************

	field_type()

	Return the Gambas type of a field in a result from its index.

	<result> is the result handle.
	<field> is the field index.

*****************************************************************************/

static GB_TYPE field_type(DB_RESULT result, int field)
{
	return conv_type(((Dataset *)result)->fieldType(field));
}


/*****************************************************************************

	field_length()

	Return the length of a field in a result from its index.

	<result> is the result handle.
	<field> is the field index.

*****************************************************************************/

static int field_length(DB_RESULT result, int field)
{
	int len;
	len = ((Dataset *)result)->fieldSize(field);
	GB_TYPE type = conv_type(((Dataset *)result)->fieldType(field));

	if (type != GB_T_STRING)
		return 0;
	else
		return len;
}


/*****************************************************************************

	begin_transaction()

	Begin a transaction.

	<handle> is the database handle.

	This function returns TRUE if the command has failed, and FALSE if
	everything was OK.

	In mysql commit/rollback can only be used with transaction safe tables (BDB,
	or InnoDB tables)

	ISAM, MyISAM and HEAP tables will commit straight away. The transaction
	methods are therefore ignored.

*****************************************************************************/

static int begin_transaction(DB_DATABASE *db)
{
	return do_query(db, "Unable to begin transaction: &1", NULL, "BEGIN", 0);
}


/*****************************************************************************

	commit_transaction()

	Commit a transaction.

	<handle> is the database handle.

	This function returns TRUE if the command has failed, and FALSE if
	everything was OK.

*****************************************************************************/

static int commit_transaction(DB_DATABASE *db)
{
	return do_query(db, "Unable to commit transaction: &1", NULL, "COMMIT", 0);
}


/*****************************************************************************

	rollback_transaction()

	Rolllback a transaction.

	<handle> is the database handle.

	This function returns TRUE if the command has failed, and FALSE if
	everything was OK.

*****************************************************************************/

static int rollback_transaction(DB_DATABASE *db)
{
	return do_query(db, "Unable to rollback transaction: &1", NULL, "ROLLBACK", 0);
}

/*****************************************************************************

	table_init()

	Initialize an info structure from table fields.

	<handle> is the database handle.
	<table> is the table name.
	<info> points at the info structure.

	This function must initialize the following info fields:
	- info->nfield must contain the number of fields in the table.
	- info->field is an array of DB_FIELD, one element for each field.

	This function returns TRUE if the command has failed, and FALSE if
	everything was OK.

*****************************************************************************/

static int field_info(DB_DATABASE *db, const char *table, const char *field, DB_FIELD *info);

static int table_init(DB_DATABASE *db, const char *table, DB_INFO *info)
{
	const char *qfield = "PRAGMA table_info('&1')";

	Dataset *res;
	int i, n;
	DB_FIELD *f;
	const char *field;

	/* Nom de la table */

	info->table = GB.NewZeroString(table);

	/* Liste des champs */

	if (do_query(db, "Unable to get table fields: &1", &res, qfield, 1, table))
											return TRUE;

	result_set* r = (result_set*) res->getExecRes();

	info->nfield = n = r->records.size();
	if (n == 0){
		res->close();
		return TRUE;
	}

	GB.Alloc(POINTER(&info->field), sizeof(DB_FIELD) * n);


	for (i = 0; i < n; i++)
	{
		f = &info->field[i];
		field = r->records[i][1].get_asString().data();

		if (field_info(db, table, field, f))
		{
			res->close();
			return TRUE;
		}

		f->name = GB.NewZeroString(field);
		/*f->length = 0;
		f->type = conv_type(GetFieldType((char *) r->records[i][2].get_asString().data(), (unsigned int *) &f->length));*/
	}

	res->close();

	return FALSE;
}

/*****************************************************************************

	table_index()

	Initialize an info structure from table primary index.

	<handle> is the database handle.
	<table> is the table name.
	<info> points at the info structure.

	This function must initialize the following info fields:
	- info->nindex must contain the number of fields in the primary index.
	- info->index is a int[] giving the index of each index field in
		info->fields.

	This function must be called after table_init().

	This function returns TRUE if the command has failed, and FALSE if
	everything was OK.

*****************************************************************************/

static int table_index(DB_DATABASE *db, const char *table, DB_INFO *info)
{
	const char *qindex1 = "PRAGMA index_list('&1')";
	const char *qindex2 = "PRAGMA index_info('&1')";

	Dataset *res;
	char *sql = NULL;
	int n = 0;

	/* Index primaire */

	if (do_query(db, "Unable to get primary index: &1", &res, qindex1, 1, table))
		return TRUE;

	result_set* r = (result_set*) res->getExecRes();
	if (( n = r->records.size()) <= 0){
		/* no indexes returned */
		/* sqlite will use a PRIMARY INDEX on ROWID */
		/* BM: without a true primary index, Edit() does not work */
		GB.Error("Table '&1' has no primary index", table);
		res->close();
		return TRUE;
	}

	for (int i = 0; i < n; i++)
	{
		if (strstr(r->records[i][1].get_asString().data(), "autoindex") != NULL){
			sql = GB.NewZeroString(r->records[i][1].get_asString().data());
			res->close();

			if (do_query(db, "Unable to get information on primary index: &1", &res, qindex2, 1, sql)){
					res->close();
					GB.FreeString(&sql);
					return TRUE;
			}
			GB.FreeString(&sql);

			r = (result_set*) res->getExecRes();
			info->nindex = r->records.size();
			GB.Alloc(POINTER(&info->index), sizeof(int) * info->nindex);

			for (i = 0; i < info->nindex; i++){
					info->index[i] = r->records[i][1].get_asInteger();
			}
			break;
		}
	}

	res->close();

	return FALSE;
}


/*****************************************************************************

	table_release()

	Free the info structure filled by table_init() and/or table_index()

	<handle> is the database handle.
	<info> points at the info structure.

*****************************************************************************/

static void table_release(DB_DATABASE *db, DB_INFO *info)
{
	/* All is done outside the driver */
}

/*****************************************************************************

	table_exist()

	Returns if a table exists

	<handle> is the database handle.
	<table> is the table name.

	This function returns TRUE if the table exists, and FALSE if not.

*****************************************************************************/

static int table_exist(DB_DATABASE *db, const char *table)
{

	const char *query = "select tbl_name from "
		"( select tbl_name from sqlite_master where type = 'table' union "
		"select tbl_name from sqlite_temp_master where type = 'table' ) "
		"where tbl_name = '&1'";

	if ( strcmp(table,"sqlite_master") == 0 || strcmp(table,"sqlite_temp_master") == 0){
		return TRUE;
	}

	Dataset *res;
	int exist;

	if (do_query(db, "Unable to check table: &1", &res, query, 1, table))
					return FALSE;

	exist = res->num_rows();
	res->close();
	return exist;
}

/*****************************************************************************

	table_list()

	Returns an array containing the name of each table in the database

	<handle> is the database handle.
	<tables> points to a variable that will receive the char* array.

	This function returns the number of tables, or -1 if the command has
	failed.

	Be careful: <tables> can be NULL, so that just the count is returned.

*****************************************************************************/

static int table_list(DB_DATABASE *db, char ***tables)
{
		const char *query =
		"select tbl_name from "
		"( select tbl_name from sqlite_master where type = 'table' union "
		"   select tbl_name from sqlite_temp_master where type = 'table')";

	Dataset *res;
	int rows;
	int i = 0;

	if (do_query(db, "Unable to get tables: &1", &res, query, 0))
				return -1;

	rows = res->num_rows();
	GB.NewArray(tables, sizeof(char *), rows + 2);//sqlite_master and sqlite_temp_master need to be
							//added to the list

	while ( !res->eof()){
		(*tables)[i] = GB.NewZeroString(res->fv("tbl_name").get_asString().data());
		res->next();
		i++;
	}


	res->close();

	(*tables)[i] = GB.NewZeroString("sqlite_master");
	(*tables)[++i] = GB.NewZeroString("sqlite_temp_master");
	return rows;
}

/*****************************************************************************

	table_primary_key()

	Returns a string representing the primary key of a table.

	<handle> is the database handle.
	<table> is the table name.
	<key> points to a string that will receive the primary key.

	This function returns TRUE if the command has failed, and FALSE if
	everything was OK.

*****************************************************************************/

static int table_primary_key(DB_DATABASE *db, const char *table, char ***primary)
{
	const char *qindex1 = "PRAGMA index_list('&1')";
	const char *qindex2 = "PRAGMA index_info('&1')";

	Dataset *res;
	int i, n;
	char *sql;

	if (do_query(db, "Unable to get primary key: &1", &res, qindex1, 1, table))
		return TRUE;

	GB.NewArray(primary, sizeof(char *), 0);

	result_set* r = (result_set*) res->getExecRes();
	//if ((n = r->records.size()) < 1){ //No Indexes found
	//   GB.NewString((char **)GB.Add(primary), "ROWID", 0);
	//   res->close();
	//   return FALSE;
	//}

	n = r->records.size();
	for (i = 0; i < n; i++)
	{
		if (strstr(r->records[i][1].get_asString().data(), "autoindex")){
			sql = GB.NewZeroString(r->records[i][1].get_asString().data());
			res->close();

			if (do_query(db, "Unable to get primary key: &1", &res, qindex2, 1, sql)){
					res->close();
					GB.FreeString(&sql);
					return TRUE;
			}
			GB.FreeString(&sql);

			r = (result_set*) res->getExecRes();
			if ((n = r->records.size()) < 1 ){
								// No information returned for key
					res->close();
					return TRUE;
			}

			for (i = 0; i < n; i++)
				*(char **)GB.Add(primary) = GB.NewZeroString(r->records[i][2].get_asString().data());
			
			break;
		}
	}

	res->close();
/*
	if (GB.Count(*primary) == 0){
			GB.NewString((char **)GB.Add(primary), "ROWID", 0);
	}
*/
	return FALSE;
}

/*****************************************************************************

	table_is_system()

	Returns if a table is a system table.

	<handle> is the database handle.
	<table> is the table name.

	This function returns TRUE if the table is a system table, and FALSE if
	not.

	Note: There are only two tables in Sqlite used by the system.
				sqlite_master and sqlite_temp_master

*****************************************************************************/

static int table_is_system(DB_DATABASE *db, const char *table)
{
		if (strcmp(table,"sqlite_master") == 0 ||
			strcmp(table, "sqlite_temp_master") == 0)
			return TRUE;
		else
			return FALSE;
}

/*****************************************************************************

	table_type()
	Not Valid in Sqlite

	<handle> is the database handle.
	<table> is the table name.
*****************************************************************************/
static char *table_type(DB_DATABASE *db, const char *table, const char *type)
{
	if (type)
		GB.Error("SQLite does not have any table types");
	return NULL;
}

/*****************************************************************************

	table_delete()

	Deletes a table.

	<handle> is the database handle.
	<table> is the table name.

	This function returns TRUE if the command has failed, and FALSE if
	everything was OK.

*****************************************************************************/

static int table_delete(DB_DATABASE *db, const char *table)
{
	return
		do_query(db, "Unable to delete table: &1", NULL,
			"drop table '&1'", 1, table);
}

/*****************************************************************************

	table_create()

	Creates a table.

	<handle> is the database handle.
	<table> is the table name.
	<fields> points to a linked list of field descriptions.
	<key> is the primary key.

	This function returns TRUE if the command has failed, and FALSE if
	everything was OK.

*****************************************************************************/

static int table_create(DB_DATABASE *db, const char *table, DB_FIELD *fields, char **primary, const char *not_used)
{
	DB_FIELD *fp;
	int comma;
	const char *type;
	int i;

	DB.Query.Init();

	DB.Query.Add("CREATE TABLE ");
	DB.Query.Add(QUOTE_STRING);
	DB.Query.Add(table);
	DB.Query.Add(QUOTE_STRING);
	DB.Query.Add(" ( ");

	comma = FALSE;
	for (fp = fields; fp; fp = fp->next)
	{
		if (comma)
			DB.Query.Add(", ");
		else
			comma = TRUE;

		DB.Query.Add(QUOTE_STRING);
		DB.Query.Add(fp->name);
		DB.Query.Add(QUOTE_STRING);

		switch (fp->type)
		{
			case GB_T_BOOLEAN: type = "BOOL"; break;
			case GB_T_INTEGER: type = "INT4"; break;
			case GB_T_LONG: type = "BIGINT"; break;
			case GB_T_FLOAT: type = "FLOAT8"; break;
			case GB_T_DATE: type = "DATETIME"; break;
			case GB_T_STRING:

				if (fp->length <= 0)
					type = "TEXT";
				else
				{
					sprintf(_buffer, "VARCHAR(%d)", fp->length);
					type = _buffer;
				}

				break;

			default: type = "TEXT"; break;
		}

		DB.Query.Add(" ");
		DB.Query.Add(type);

		if (fp->def.type != GB_T_NULL)
		{
			DB.Query.Add(" NOT NULL DEFAULT ");
			DB.FormatVariant(&_driver, &fp->def, DB.Query.AddLength);
		}
		else if (DB.StringArray.Find(primary, fp->name) >= 0)
		{
			DB.Query.Add(" NOT NULL ");
		}
	}

	if (primary)
	{
		DB.Query.Add(", PRIMARY KEY (");

		for (i = 0; i < GB.Count(primary); i++)
		{
			if (i > 0)
				DB.Query.Add(",");

			DB.Query.Add(QUOTE_STRING);
			DB.Query.Add(primary[i]);
			DB.Query.Add(QUOTE_STRING);
		}

		DB.Query.Add(")");
	}

	DB.Query.Add(" )");

	return do_query(db, "Cannot create table: &1", NULL, DB.Query.Get(), 0);
}

/*****************************************************************************

	field_exist()

	Returns if a field exists in a given table

	<handle> is the database handle.
	<table> is the table name.
	<field> is the field name.

	This function returns TRUE if the field exists, and FALSE if not.

*****************************************************************************/

static int field_exist(DB_DATABASE *db, const char *table, const char *field)
{
	const char *query = "PRAGMA table_info('&1')";

	int i, n;
	Dataset *res;
	int exist = 0;

	if (do_query(db, "Unable to find field: &1.&2", &res, query, 2, table, field)){
		return FALSE;
	}

	result_set* r = (result_set*) res->getExecRes();

	n = r->records.size();

	for (i = 0; i < n; i++){
			if (strcmp(field, r->records[i][1].get_asString().data()) == 0)
				exist++;
	}

	res->close();
	return exist;
}

/*****************************************************************************

	field_list()

	Returns an array containing the name of each field in a given table

	<handle> is the database handle.
	<table> is the table name.
	<fields> points to a variable that will receive the char* array.

	This function returns the number of fields, or -1 if the command has
	failed.

	Be careful: <fields> can be NULL, so that just the count is returned.

*****************************************************************************/

static int field_list(DB_DATABASE *db, const char *table, char ***fields)
{
	const char *query = "PRAGMA table_info('&1')";

	int i, n;
	Dataset *res;

	if (do_query(db, "Unable to get fields: &1", &res, query, 1, table)){
			return -1;
	}

	result_set* r = (result_set*) res->getExecRes();

	n = r->records.size();

	if (fields) /* (BM) see the function commentary */
	{
		GB.NewArray(fields, sizeof(char *), n);

		for (i = 0; i < n; i++){
			(*fields)[i] = GB.NewZeroString(r->records[i][1].get_asString().data());
		}
	}

	res->close();
	return n;
}


/*****************************************************************************

	field_info()

	Get field description

	<handle> is the database handle.
	<table> is the table name.
	<field> is the field name.
	<info> points to a structure filled by the function.

	This function returns TRUE if the command has failed, and FALSE if
	everything was OK.

*****************************************************************************/

static int field_info(DB_DATABASE *db, const char *table, const char *field, DB_FIELD *info)
{
	const char *query = "PRAGMA table_info('&1')";

	Dataset *res;
	GB_VARIANT def;
	char *val;
	char *_fieldName = NULL;
	char *_fieldType = NULL;
	char *_defaultValue = NULL;
	bool _fieldNotNull = FALSE;
	int i, n;
	fType type;

	if (do_query(db, "Unable to get fields: &1", &res, query,1, table))
	{
		return TRUE;
	}

	result_set *r = (result_set*) res->getExecRes();
	if ((n = r->records.size()) == 0)
	{
		GB.Error("Unable to find field &1.&2", table, field);
		return TRUE;
	}

	for ( i =0; i < n; i++){
			_fieldName = (char *)r->records[i][1].get_asString().data();

			if (strcmp(_fieldName, field) == 0){
				_fieldType = (char *)r->records[i][2].get_asString().data();
				_fieldNotNull = (char *)r->records[i][3].get_asBool();
				_defaultValue = (char *)r->records[i][4].get_asString().data();
				break;
			}
	}

	if (i >= n)
	{
			GB.Error("Unable to find field &1.&2", table, field);
			return TRUE;
	}

	info->name = NULL;

	type = GetFieldType(_fieldType, (unsigned int *) &info->length);
	info->type = conv_type(type);

	info->def.type = GB_T_NULL;

	if ( _fieldNotNull)
	{
		def.type = GB_T_VARIANT;
		def.value.type = GB_T_NULL;

		val = _defaultValue;

		if (val && *val)
		{
			conv_data(val, &def.value, type);
			GB.StoreVariant(&def, &info->def);
		}
	}

	info->collation = NULL;

	res->close();
	return FALSE;
}

/*****************************************************************************

	index_exist()

	Returns if an index exists in a given table

	<handle> is the database handle.
	<table> is the table name.
	<field> is the index name.

	This function returns TRUE if the index exists, and FALSE if not.

*****************************************************************************/

static int index_exist(DB_DATABASE *db, const char *table, const char *index)
{
	const char *query = "select tbl_name from "
		"( select tbl_name from sqlite_master where type = 'index' and "
		" name = '&2' union "
		"select tbl_name from sqlite_temp_master where type = 'index' and "
		" name = '&2' ) "
		"where tbl_name = '&1'";

	Dataset *res;
	int exist;

	if (do_query(db, "Unable to check table: &1", &res, query, 2, table, index))
					return FALSE;

	exist = res->num_rows();
	res->close();
	return exist;
}

/*****************************************************************************

	index_list()

	Returns an array containing the name of each index in a given table

	<handle> is the database handle.
	<table> is the table name.
	<indexes> points to a variable that will receive the char* array.

	This function returns the number of indexes, or -1 if the command has
	failed.

	Be careful: <indexes> can be NULL, so that just the count is returned.

*****************************************************************************/

static int index_list(DB_DATABASE *db, const char *table, char ***indexes)
{

	const char *query =
	"select name from "
	"( select name from sqlite_master where type = 'index' and tbl_name = '&1' "
	" union select name from sqlite_temp_master where type = 'index' and "
	" tbl_name = '&1')";

	Dataset *res;
	int no_indexes, i = 0;

	if (do_query(db, "Unable to get tables: &1", &res, query, 1, table))
				return -1;

	no_indexes = res->num_rows();
	GB.NewArray(indexes, sizeof(char *), no_indexes);

	while ( !res->eof()){
										//(res->fv("tbl_name").get_asString().data());
		(*indexes)[i] = GB.NewZeroString(res->fv(res->fieldName(0)).get_asString().data());
		res->next();
		i++;
	}

	res->close();

	return no_indexes;
}

/*****************************************************************************

	index_info()

	Get index description

	<handle> is the database handle.
	<table> is the table name.
	<field> is the index name.
	<info> points to a structure filled by the function.

	This function returns TRUE if the command has failed, and FALSE if
	everything was OK.

*****************************************************************************/

static int index_info(DB_DATABASE *db, const char *table, const char *index, DB_INDEX *info)
{
	/* sqlite indexes are unique to the database, and not to the table */

	const char *qindex1 = "PRAGMA index_list('&1')";
	const char *qindex2 = "PRAGMA index_info('&1')";

	Dataset *res;
	int i, j, n;

	if (do_query(db, "Unable to get index info for table: &1", &res, qindex1, 1, table))
		return TRUE;

	result_set* r = (result_set*) res->getExecRes();
	if (( n = r->records.size()) == 0){
		/* no indexes found */
		res->close();
		GB.Error("Unable to find index &1.&2", table, index);
		return TRUE;
	}

	for (i = 0, j = 0; i < n; i++)
	{ /* Find the required index */
		if ( strcmp( index, r->records[i][1].get_asString().data()) == 0){
			j++;
			break;
		}
	}

	if ( j == 0)
	{
			GB.Error("Unable to find index &1.&2", table, index);
			return TRUE;
	}

	info->name = NULL;
	info->unique = strncmp("1",r->records[i][2].get_asString().data(),1) == 0 ? TRUE : FALSE;
	info->primary = strstr(r->records[i][1].get_asString().data(), "autoindex")
		!= NULL ? TRUE : FALSE;

	DB.Query.Init();

	if (do_query(db, "Unable to get index info for : &1", &res, qindex2, 1, index)){
		res->close();
		return TRUE;
	}

	r = (result_set*) res->getExecRes();
	n = r->records.size();
	i = 0;
	/* (BM) row can be null if we are seeking the last index */
	while ( i < n )
	{
		if (i > 0)
			DB.Query.Add(",");

		/* Get fields in key */
		DB.Query.Add(r->records[i][2].get_asString().data());
		i++;
	}

	res->close();
	info->fields = DB.Query.GetNew();

	return FALSE;
}

/*****************************************************************************

	index_delete()

	Deletes an index.

	<handle> is the database handle.
	<table> is the table name.
	<index> is the index name.

	This function returns TRUE if the command has failed, and FALSE if
	everything was OK.

*****************************************************************************/

static int index_delete(DB_DATABASE *db, const char *table, const char *index)
{
	return
		do_query(db, "Unable to delete index: &1", NULL,
			"drop index '&1'", 1, index);
}

/*****************************************************************************

	index_create()

	Creates an index.

	<handle> is the database handle.
	<table> is the table name.
	<index> is the index name.
	<info> points to a structure describing the index.

	This function returns TRUE if the command has failed, and FALSE if
	everything was OK.

*****************************************************************************/

static int index_create(DB_DATABASE *db, const char *table, const char *index, DB_INDEX *info)
{
	DB.Query.Init();

	DB.Query.Add("CREATE ");
	if (info->unique)
		DB.Query.Add("UNIQUE ");
	DB.Query.Add("INDEX ");
	DB.Query.Add(QUOTE_STRING);
	DB.Query.Add(index);
	DB.Query.Add(QUOTE_STRING);
	DB.Query.Add(" ON ");
	DB.Query.Add(QUOTE_STRING);
	DB.Query.Add(table);
	DB.Query.Add(QUOTE_STRING);
	DB.Query.Add(" ( ");
	DB.Query.Add(info->fields);
	DB.Query.Add(" )");

	return do_query(db, "Cannot create index: &1", NULL, DB.Query.Get(), 0);
}

/*****************************************************************************
*
*   database_exist()
*
*   Returns if a database exists
*
*   <handle> is any database handle.
*   <name> is the database name.
*
*   This function returns TRUE if the database exists, and FALSE if not.
*   SQLite: Databases are just files, so we need to ceck to see whether
*   the file exists and is a sqlite file.
******************************************************************************/
static int database_exist(DB_DATABASE *db, const char *name)
{
	SqliteDatabase *conn = (SqliteDatabase *)db->handle;
	char *fullpath = NULL;

	if (strcmp(name,":memory:") == 0)
		return TRUE; //Database is loaded in memory only

	if ((fullpath = find_database(name, conn->getHostName())) != NULL)
		return TRUE;
	else
		return FALSE;
}

/*****************************************************************************
*
*   database_list()
*
*   Returns an array containing the name of each database
*
*   <handle> is any database handle.
*   <databases> points to a variable that will receive the char* array.
*
*   This function returns the number of databases, or -1 if the command has
*   failed.
*
*   Be careful: <databases> can be NULL, so that just the count is returned.
*
*   Sqlite databases are files. Using we will only list
*   files within a designated directory. Propose that all
*   areas are walked through.
******************************************************************************/

static int database_list(DB_DATABASE *db, char ***databases)
{
	char *dbhome;

	SqliteDatabase *conn = (SqliteDatabase *)db->handle;
	GB.NewArray(databases, sizeof(char *), 0);

	/* Hostname contains home area */
	dbhome = (char *)conn->getHostName();
	if (dbhome && *dbhome)
	{
		WalkDirectory( dbhome, databases );
	}
	else
	{
		/* Checks GAMBAS_SQLITE_DBHOME if set, or Current Working Directory */
		/* Might have to come back and seperate */
		dbhome = GetDatabaseHome();
		if (dbhome){
				//GB.Error("Unable to get databases: &1", "Can't find current directory");
				WalkDirectory( dbhome, databases );
				GB.Free(POINTER(&dbhome));
		}
	}

	return GB.Count(databases);
}

/*****************************************************************************
*
*   database_is_system()
*
*   Returns if a database is a system database.
*
*   <handle> is any database handle.
*   <name> is the database name.
*
*   This function returns TRUE if the database is a system database, and
*   FALSE if not.
*
*   NOTE: Sqlite doesn't have such a thing.
*
******************************************************************************/

static int database_is_system(DB_DATABASE *db, const char *name)
{
			return FALSE;
}

/*****************************************************************************
*
*   database_delete()
*
*   Deletes a database.
*
*   <handle> is the database handle.
*   <name> is the database name.
*
*   This function returns TRUE if the command has failed, and FALSE if
*   everything was OK.
*
******************************************************************************/

static int database_delete(DB_DATABASE *db, const char *name)
{
	char *fullpath = NULL;
	SqliteDatabase *conn = (SqliteDatabase *)db->handle;

	if((fullpath = find_database(name, conn->getHostName())) == NULL){
		GB.Error("Cannot Find  database: &1", name);
		return TRUE;
	}

	if( remove(fullpath) != 0){
			GB.Error("Unable to delete database  &1", fullpath);
			return TRUE;
	}

	return FALSE;
}

/*****************************************************************************
*
*   database_create()
*
*   Creates a database.
*
*   <handle> is the database handle.
*   <name> is the database name.
*
*   This function returns TRUE if the command has failed, and FALSE if
*   everything was OK.
*
*   SQLite automatically creates a database on connect if the file
*   does not exist.
******************************************************************************/

static int database_create(DB_DATABASE *db, const char *name)
{
	SqliteDatabase *conn = (SqliteDatabase *)db->handle;
	SqliteDatabase conn2;
	char *fullpath = NULL;
	//char *homepath = NULL;
	//DIR *dp;
	char *dir;
	const char *host;

	/* Does Name include fullpath */
	/* BM: do not use basename(), it can modify its argument */
	if (name && name[0] == '/')
	{
		fullpath = GB.NewZeroString(name);
		goto _CREATE_DATABASE;
	}

	/* Hostname contains home area */
	//if ((dp = opendir(conn->getHostName())))
	/* BM: Why use opendir(), and why forget to call closedir() ? */
	host = conn->getHostName();
	if (host && host[0])
	{
		fullpath = GB.NewZeroString(host);
	}
	else
	{
		dir = GetDatabaseHome();
		mkdir(dir, S_IRWXU);
		fullpath = GB.NewZeroString(dir);
		GB.Free(POINTER(&dir));
	}

	if (fullpath[strlen(fullpath) - 1] != '/')
		fullpath = GB.AddChar(fullpath, '/');

	fullpath = GB.AddString(fullpath, name, 0);

_CREATE_DATABASE:

	conn2.setDatabase(fullpath);

	GB.FreeString(&fullpath);
	if ( conn2.connect() != DB_CONNECTION_OK){
		GB.Error("Cannot create database: &1", conn2.getErrorMsg());
		conn2.disconnect();
		return TRUE;
	}

	//Create and remove a table to initialize database
	db->handle = &conn2;
	if (!do_query(db, "Unable to initialise database", NULL, "CREATE TABLE GAMBAS (FIELD1 TEXT)", 0))
		do_query(db, NULL, NULL, "DROP TABLE GAMBAS",0);

	conn2.disconnect();
	db->handle = conn;
	
	return FALSE;
}


/*****************************************************************************
*
*  user_exist()
*
*  Returns if a user exists.
*
*  <handle> is any database handle.
*  <name> is the user name.
*
*  This function returns TRUE if the user exists, and FALSE if not.
*  Sqlite does not have different  users.  Access is controlled by
*  access rightd on the file.
*  We can check that the user exists on the machine and has access to
*  database file!
*  [Currently only checks against /etc/passwd.
*   Does not check against /etc/shadow or pam.
*
******************************************************************************/

static int user_exist(DB_DATABASE *db, const char *name)
{
	struct stat dbbuf;
	struct passwd *fileowner, *user; // /etc/passwd structure
	struct group *Group;   // /etc/group structure
	char **Member;
	const char *Databasefile;
	bool in_memory;

	SqliteDatabase *conn = (SqliteDatabase *)db->handle;

	if (( Databasefile = conn->getDatabase()) == NULL){
		GB.Error("User_exist:&1", "Unable to get databasename");
		return FALSE;
	}

	in_memory = strcmp(Databasefile, ":memory:") == 0;

	/* Is username in passwd file */
	if ((user = getpwnam(name)) == NULL){
		return FALSE;
	}

	if (in_memory)
		return (user->pw_uid == getuid());

	if (stat(Databasefile, &dbbuf) != 0)
	{
		GB.Error("User_exist: Unable to get status of &1", Databasefile);
		return FALSE;
	}

	/* Now what are the database file permissions */
	/* The order of tests is important */

	if (( fileowner = getpwuid(dbbuf.st_uid)) != NULL){
		if ( fileowner->pw_uid == user->pw_uid ){
			/* User is owner */
					if (( dbbuf.st_mode & S_IRUSR ) || (dbbuf.st_mode & S_IWUSR)){
		return TRUE;
		}
		else {
			return FALSE;
		}
		}
		if ( fileowner->pw_gid == user->pw_gid ){
					if (( dbbuf.st_mode & S_IRGRP ) || (dbbuf.st_mode & S_IWGRP)){
			/* User has access to the file via primary group access.*/
		return TRUE;
		}
		else {
			return FALSE;
		}
		}

	}

	/* Check whether user is in the same group */

	Group = getgrgid(dbbuf.st_gid);
	Member = Group->gr_mem;
	while (Member && *Member){
				if (strcmp(*Member, name) == 0){
					//User is a member of the group
			if (( dbbuf.st_mode & S_IRGRP ) || (dbbuf.st_mode & S_IWGRP)){
					return TRUE;
			}
		else {
						return FALSE;
		}
	}
					Member++;
	}

	if (( dbbuf.st_mode & S_IROTH ) || (dbbuf.st_mode & S_IWOTH)){
		/* Any user can access this file */
		return TRUE;
	}

	return FALSE;

}

/*****************************************************************************
*
*   user_list()
*
*   Returns an array containing the name of each user.
*
*   <handle> is the database handle.
*   <users> points to a variable that will receive the char* array.
*
*   This function returns the number of users, or -1 if the command has
*   failed.
*
*   Be careful: <users> can be NULL, so that just the count is returned.
*   Sqlite does not have users.
*
******************************************************************************/


static int user_list(DB_DATABASE *db, char ***users)
{
	//Should we use GB.HashTable.New etc. to ensure
	//duplicates are not reported back, then transfer
	//the valid elements to the Array.
	//Need to check order of rights. e.g. User overides
	//group and other, group overides other.
	//That is if the user is a member of a group that
	//is given no rights, but other has all rights,
	//they should not be able to access
	char *Databasefile;
	struct stat buf;
	struct passwd *user; // /etc/passwd structure
	struct group *Group;   // /etc/group structure
	char **Member;
	int Count = 0;
	bool in_memory;

	SqliteDatabase *conn = (SqliteDatabase *)db->handle;

	if (( Databasefile = (char *)conn->getDatabase()) == NULL){
		GB.Error("Unable to get databasename");
		return -1;
	}

	in_memory = strcmp(Databasefile, ":memory:") == 0;

	if (in_memory)
	{
		buf.st_mode = S_IWUSR | S_IRUSR;
		buf.st_uid = getuid();
	}
	else if (stat(Databasefile, &buf) != 0)
	{
		GB.Error("Unable to get status of &1", Databasefile);
		return -1;
	}

	if (users)
		GB.NewArray(users, sizeof(char *), 0);

	if (!in_memory)
	{

		/* If file has other access then any user could use it */
		if (( buf.st_mode & S_IROTH ) || (buf.st_mode & S_IWOTH)){
			/* Any user can access this file */

			while (( user = getpwent()) != NULL ){
						if (users){
								*(char **)GB.Add(users) = GB.NewZeroString(user->pw_name);
						}
						else {
								Count++;
						}
			}

			if (users){
					return GB.Count(users);
			}
			else {
					return Count;
			}
		}

		/* If group access then add all users in the group */
		if (( buf.st_mode & S_IRGRP ) || (buf.st_mode & S_IWGRP)){

					Group = getgrgid(buf.st_gid);
					Member = Group->gr_mem;
					while (Member && *Member){
							if (users){
									*(char **)GB.Add(users) = GB.NewZeroString(*Member);
							}
							else {
									Count++;
							}
							Member++;
					}
		}

	}

	/* Don't forget the owner if that has not already been added */
	if (( buf.st_mode & S_IRUSR ) || (buf.st_mode & S_IWUSR)){
		if (( user = getpwuid(buf.st_uid)) != NULL){
				if (users){
					*(char **)GB.Add(users) = GB.NewZeroString(user->pw_name);
	}
	else {
		Count++;
	}
		}
	}

	if (users)
		return GB.Count(users);
	else
		return Count;
}

/*****************************************************************************
*
*   user_info()
*
*   Get user description
*
*   <handle> is the database handle.
*   <name> is the user name.
*   <info> points to a structure filled by the function.
*
*   This function returns TRUE if the command has failed, and FALSE if
*   everything was OK.
*
*   Sqlite privileges are just file privileges. We will return Admin
*   rights where privilege allows Write. There is no password.
*
******************************************************************************/

static int user_info(DB_DATABASE *db, const char *name, DB_USER *info )
{
	char *Databasefile;
	//struct stat buf;
	struct passwd *user; // /etc/passwd structure
	bool in_memory;
	//struct group *Group;   // /etc/group structure
	//char **Member;
	//int Count;

	SqliteDatabase *conn = (SqliteDatabase *)db->handle;

	/* Is username in passwd file */
	if ((user = getpwnam(name)) == NULL){
		GB.Error("User_info: Invalid user &1", name);
		return TRUE;
	}

	if (( Databasefile = (char *)conn->getDatabase()) == NULL){
		GB.Error("User_info: &1", "Unable to get databasename");
		return TRUE;
	}

	in_memory = strcmp(Databasefile, ":memory:") == 0;

	if (in_memory)
		info->admin = true;
	else
		info->admin = access(Databasefile, W_OK);

	/* If file has other access then any user could use it */
	//Need to look at order of checks
	info->name = NULL;

// if (row[3]) //password does not exist for sqlite
//     GB.NewString(&info->password, row[3], 0); //password is encrypted in mysql

	return FALSE;
}

/*****************************************************************************

	user_delete()

	Deletes a user.

	<handle> is any database handle.
	<name> is the user name.

	This function returns TRUE if the command has failed, and FALSE if
	everything was OK.

	Sqlite users are operated by the O/S
*****************************************************************************/

static int user_delete(DB_DATABASE *db, const char *name)
{
	GB.Error("SQLite users do not exist.");
	return TRUE;
}

/*****************************************************************************
*
*   user_create()
*
*     Creates a user.
*
*     <handle> is the database handle.
*     <name> is the user name.
*     <info> points to a structure describing the user.
*
*     This function returns TRUE if the command has failed, and FALSE if
*           everything was OK.
*
*
*   Sqlite: No user create
******************************************************************************/

static int user_create(DB_DATABASE *db, const char *name, DB_USER *info)
{
	GB.Error("SQLite users do not exist.");
	return TRUE;
}

/*****************************************************************************
*
*   user_set_password()
*
*   Change the user password.
*
*   <handle> is the database handle.
*   <name> is the user name.
*   <password> is the new password
*
*   This function returns TRUE if the command has failed, and FALSE if
*   everything was OK.
*
*   Sqlite : No user passwords.
******************************************************************************/

static int user_set_password(DB_DATABASE *db, const char *name, const char *password)
{
	GB.Error("SQLite users do not exist.");
	return TRUE;
}


/*****************************************************************************

	The component entry and exit functions.

*****************************************************************************/

extern "C" {
int EXPORT GB_INIT(void)
{
	GB.GetInterface("gb.db", DB_INTERFACE_VERSION, &DB);
	DB.Register(&_driver);

	return 0;
}

void EXPORT GB_EXIT()
{
}

} //extern "C"
