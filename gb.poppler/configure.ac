dnl ---- configure.ac for gb.poppler
m4_include([../version.m4])
AC_INIT(gambas3-gb-poppler, GB_VERSION, GB_MAIL, [], GB_URL)
AC_CONFIG_MACRO_DIR([m4])
GB_INIT(gb.poppler)
AC_PROG_LIBTOOL
GB_COMPONENT_PKG_CONFIG(
  poppler,
  POPPLER,
  gb.poppler,
  [src],
  'poppler >= 0.20' poppler-cpp poppler-glib)
AC_OUTPUT( Makefile src/Makefile )
GB_PRINT_MESSAGES
