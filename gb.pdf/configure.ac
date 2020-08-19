dnl ---- configure.ac for gb.pdf

m4_include([../version.m4])
AC_INIT(gambas3-gb-pdf, GB_VERSION, GB_MAIL, [], GB_URL)
AC_CONFIG_MACRO_DIR([m4])

GB_INIT(gb.pdf)
AC_PROG_LIBTOOL

GB_COMPONENT_PKG_CONFIG(
  poppler, POPPLER, gb.pdf, [src],
  'poppler >= 0.20'
)

if test "$have_poppler" = "yes"; then
  pkg-config --atleast-version=0.58.0 poppler
  AC_DEFINE_UNQUOTED(POPPLER_VERSION_0_58, $((1-$?)), Poppler version >= 0.58)
  pkg-config --atleast-version=0.64.0 poppler
  AC_DEFINE_UNQUOTED(POPPLER_VERSION_0_64, $((1-$?)), Poppler version >= 0.64)
  pkg-config --atleast-version=0.71.0 poppler
  AC_DEFINE_UNQUOTED(POPPLER_VERSION_0_71, $((1-$?)), Poppler version >= 0.71)
  pkg-config --atleast-version=0.72.0 poppler
  AC_DEFINE_UNQUOTED(POPPLER_VERSION_0_72, $((1-$?)), Poppler version >= 0.72)
  pkg-config --atleast-version=0.76.0 poppler
  AC_DEFINE_UNQUOTED(POPPLER_VERSION_0_76, $((1-$?)), Poppler version >= 0.76)
  pkg-config --atleast-version=0.83.0 poppler
  AC_DEFINE_UNQUOTED(POPPLER_VERSION_0_83, $((1-$?)), Poppler version >= 0.83)
  pkg-config --atleast-version=0.85.0 poppler
  AC_DEFINE_UNQUOTED(POPPLER_VERSION_0_85, $((1-$?)), Poppler version >= 0.85)
  pkg-config --atleast-version=0.86.0 poppler
  AC_DEFINE_UNQUOTED(POPPLER_VERSION_0_86, $((1-$?)), Poppler version >= 0.86)
fi

AC_OUTPUT( \
Makefile \
src/Makefile \
)

GB_PRINT_MESSAGES
