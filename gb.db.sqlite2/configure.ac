dnl ---- configure.ac for gb.db.sqlite2 driver

dnl ---- Initialization

m4_include([../version.m4])
AC_INIT([gambas3-gb-db-sqlite2],[GB_VERSION],[GB_MAIL],[],[GB_URL])
AC_CONFIG_MACRO_DIR([m4])

GB_INIT(gb.db.sqlite2)
LT_INIT

dnl ---- SQLite driver

GB_COMPONENT_PKG_CONFIG(
  sqlite2, SQLITE2, gb.db.sqlite2, [src],
  sqlite)

dnl ---- Create makefiles

AC_CONFIG_FILES([\
Makefile \
src/Makefile \
])
AC_OUTPUT

GB_PRINT_MESSAGES
