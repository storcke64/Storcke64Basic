dnl ---- configure.ac for gb.db.mysql driver

m4_include([../version.m4])
AC_INIT(gambas3-gb-db-mysql, GB_VERSION, GB_MAIL, [], GB_URL)
AC_CONFIG_MACRO_DIR([m4])

GB_INIT(gb.db.mysql)
AC_PROG_LIBTOOL
dnl LT_INIT

GB_COMPONENT(
  mysql, MYSQL, gb.db.mysql, [src],
  [GB_FIND(mysql.h, $prefix /opt/local /opt /usr/local/lib /usr/local /usr/lib /usr, include mysql*/include)],
  [GB_FIND(libmysqlclient.$SHLIBEXT, $prefix /opt/local /opt /usr/local /usr, lib mysql*/lib)],
  [$C_LIB -lmysqlclient -lz])

gb_save_cppflags=$CPPFLAGS
CPPFLAGS=$MYSQL_INC

AC_CACHE_CHECK(for SSL mode option,
  gb_cv_db_mysql_ssl_mode,
  [AC_TRY_COMPILE(
    [
      #include <mysql.h>
    ],
    [
      int flag = SSL_MODE_DISABLED;
    ],
    gb_cv_db_mysql_ssl_mode=yes, gb_cv_db_mysql_ssl_mode=no
    )])
  
  if test $gb_cv_db_mysql_ssl_mode = yes; then
    AC_DEFINE(HAVE_MYSQL_SSL_MODE_DISABLED, 1, [Define if you have SSL_MODE_DISABLED constant.])
  fi

CPPFLAGS=$gb_save_cppflags
  

AC_OUTPUT( \
Makefile \
src/Makefile \
)

GB_PRINT_MESSAGES
