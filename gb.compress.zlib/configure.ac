dnl ---- Initialization

m4_include([../version.m4])
AC_INIT([gambas3-gb-compress-zlib],[GB_VERSION],[GB_MAIL],[],[GB_URL])
AC_CONFIG_MACRO_DIR([m4])

GB_INIT(gb.compress.zlib)
LT_INIT

dnl ---- zlib compression driver

GB_COMPONENT_PKG_CONFIG(
  zlib, ZLIB, gb.compress.zlib, [src],
  zlib)

dnl ---- Create makefiles

AC_CONFIG_FILES([\
Makefile \
src/Makefile \
])
AC_OUTPUT

GB_PRINT_MESSAGES
