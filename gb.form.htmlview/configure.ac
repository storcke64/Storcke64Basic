dnl ---- configure.ac for gb.form.htmlview
m4_include([../version.m4])
AC_INIT(gambas3-gb-form-htmlview, GB_VERSION, GB_MAIL, [], GB_URL)
AC_CONFIG_MACRO_DIR([m4])
GB_INIT(gb.form.htmlview)
AC_PROG_LIBTOOL
GB_COMPONENT(
  form_htmlview,
  FORM_HTMLVIEW,
  gb.form.htmlview,
  [src],
  [GB_FIND(html.h gumbo.h, /usr/local/lib /usr/local /usr/lib /usr, include/litehtml litehtml/include litehtml*/include include/litehtml* include)],
  [GB_FIND(libgumbo.$SHLIBEXT liblitehtml.$SHLIBEXT, /usr/local /usr, lib)],
  [ -llitehtml -lgumbo],
  [ ])
AC_OUTPUT( Makefile src/Makefile )
GB_PRINT_MESSAGES
