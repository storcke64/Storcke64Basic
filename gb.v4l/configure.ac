dnl ---- configure.ac for gb.v4l

m4_include([../version.m4])
AC_INIT([gambas3-gb-v4l],[GB_VERSION],[GB_MAIL],[],[GB_URL])
AC_CONFIG_MACRO_DIR([m4])

GB_INIT(gb.v4l)
LT_INIT

GB_COMPONENT_PKG_CONFIG(
  v4lconvert, V4LCONVERT, gb.v4l, [src],
  libv4lconvert)

GB_COMPONENT(
  v4l, V4L, gb.v4l, [src],
  [GB_FIND(png.h jpeglib.h linux/videodev2.h, $prefix /usr/local/lib /usr/local /usr/lib /usr, include)],
  [GB_FIND(libpng.$SHLIBEXT libjpeg.$SHLIBEXT, $prefix /usr/local /usr, lib)],
  [$C_LIB -ljpeg -lpng])

AC_CONFIG_FILES([\
Makefile \
src/Makefile \
])
AC_OUTPUT

GB_PRINT_MESSAGES
