#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# prereq & init
AC_PREREQ(2.60)
AC_INIT([giada], [0.7], [info@giadamusic.com])
AC_CONFIG_SRCDIR([src/main.cpp])
AM_INIT_AUTOMAKE([giada], [0.7])

# ----------------------------------------------------------------------

# test the build environment. These vars are used in Makefile.am during
# the linking of the libraries.
# Usage: ./configure --target=[windows | linux | osx]
if test "$target" = ""; then
	AC_MSG_ERROR(["target OS not specified. Please run ./configure --target=<windows | linux | osx>"])
fi

case "$target" in
  linux)
    os=linux
    ;;
  windows)
    os=windows
    ;;
  osx)
    os=osx
    ;;
  *)
    AC_MSG_ERROR(["Unrecognised target OS: $target"])
    ;;
esac
AM_CONDITIONAL(LINUX,   test "x$os" = "xlinux")
AM_CONDITIONAL(WINDOWS, test "x$os" = "xwindows")
AM_CONDITIONAL(OSX,     test "x$os" = "xosx")

# ----------------------------------------------------------------------

# --enable-vst. VST compilation is disabled by default
#
# WITH_VST will be passed to gcc as -DWITH_VST
#
# AC_ARG_ENABLE (
# feature,                [--enable-] + [feature], eg --enable-vst
# help-string,
# [action-if-given],      == gcc ... -DWITH_VST
# [action-if-not-given])  not used here

AC_ARG_ENABLE(
	vst,
	AS_HELP_STRING([--enable-vst], [enable vst support]),
  [AC_DEFINE(WITH_VST)]
)

# ----------------------------------------------------------------------

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET

# ----------------------------------------------------------------------

# Checks for libraries.
AC_CHECK_LIB(
	[pthread],
	[pthread_exit],
	[],
	[AC_MSG_ERROR([error: library 'pthread' not found!])]
)

# ----------------------------------------------------------------------

# Checks for headers (fltk, rtaudio and libsndfile are static, we ask if
# headers are available)
AC_LANG_PUSH([C++])
AC_CHECK_HEADER(
	[FL/Fl.H],
	[],
	[AC_MSG_ERROR([library 'fltk' not found!])]
)
AC_LANG_POP

#AC_LANG_PUSH([C++])
#AC_CHECK_HEADER(
#	[RtAudio.h],
#	[],
#	[AC_MSG_ERROR([library 'rtAudio' not found!])]
#)
#AC_LANG_POP

AC_LANG_PUSH([C++])
AC_CHECK_HEADER(
	[sndfile.h],
	[],
	[AC_MSG_ERROR([library 'libsndfile' not found!])]
)
AC_LANG_POP

# brutal and temporary hack for OS X: don't use pkg-config

if test "x$os" = "xosx"; then
	AC_LANG_PUSH([C++])
	AC_CHECK_HEADER(
		[samplerate.h],
		[],
		[AC_MSG_ERROR([library 'samplerate' not found!])]
	)
	AC_LANG_POP
else
	PKG_CHECK_MODULES(
		SAMPLERATE,
		samplerate >= 0.1.8,
		[],
		AC_MSG_ERROR([library 'libsamplerate' not found!])
	)
fi


# ----------------------------------------------------------------------

# Checks for header files.
#AC_CHECK_HEADERS([stdint.h stdlib.h string.h])

# ----------------------------------------------------------------------

# Checks for typedefs, structures, and compiler characteristics.
#AC_HEADER_STDBOOL
#AC_C_INLINE
#AC_TYPE_INT16_T
#AC_TYPE_INT32_T
#AC_TYPE_UINT32_T
#AC_TYPE_UINT8_T

# ----------------------------------------------------------------------

# Checks for library functions.
#AC_FUNC_MALLOC
#AC_FUNC_REALLOC
#AC_CHECK_FUNCS([mkdir pow strstr])

# ----------------------------------------------------------------------

# finalizing
AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
