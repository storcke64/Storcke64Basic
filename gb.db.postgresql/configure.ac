dnl ---- configure.ac for gb.db.postgresql driver

dnl ---- Initialization

m4_include([../version.m4])
AC_INIT([gambas3-gb-db-postgresql],[GB_VERSION],[GB_MAIL],[],[GB_URL])
AC_CONFIG_MACRO_DIR([m4])

GB_INIT(gb.db.postgresql)
LT_INIT

dnl ---- PostgreSQL driver

GB_COMPONENT(
  postgresql, POSTGRESQL, gb.db.postgresql, [src],
  [GB_FIND(libpq-fe.h, $prefix /usr/local/lib /usr/local /opt /usr/lib /usr, include/pgsql* pgsql*/include include/postgresql* postgresql*/include include)],
  [GB_FIND(libpq.$SHLIBEXT, $prefix /usr/local /opt /usr, lib pgsql*/lib postgresql*/lib)],
  [$C_LIB -lpq])
 
dnl ---- Create makefiles

AC_CONFIG_FILES([\
Makefile \
src/Makefile \
])
AC_OUTPUT

GB_PRINT_MESSAGES
