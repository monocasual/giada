#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# prereq & init

AC_PREREQ(2.60)
AC_INIT([giada], [0.9], [info@giadamusic.com])
AC_CONFIG_SRCDIR([src/main.cpp])
AM_INIT_AUTOMAKE([giada], [0.9])

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

# Check for programs.

AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET

# ----------------------------------------------------------------------

# Check for libraries.

AC_CHECK_LIB(
	[pthread],
	[pthread_exit],
	[],
	[AC_MSG_ERROR([error: library 'pthread' not found!])]
)

# ----------------------------------------------------------------------

# Check for generic headers (fltk, rtaudio and libsndfile are static,
# we ask if headers are available)

AC_LANG_PUSH([C++])
AC_CHECK_HEADER(
	[FL/Fl.H],
	[],
	[AC_MSG_ERROR([library 'fltk' not found!])]
)
AC_LANG_POP

AC_LANG_PUSH([C++])
AC_CHECK_HEADER(
	[RtMidi.h],
	[],
	[AC_MSG_ERROR([library 'rtMidi' not found!])]
)
AC_LANG_POP

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
#	PKG_CHECK_MODULES(
#		SAMPLERATE,
#		samplerate >= 0.1.8,
#		[],
#		AC_MSG_ERROR([library 'libsamplerate' not found!])
#	)
	AC_LANG_PUSH([C++])
	AC_CHECK_HEADER(
		[samplerate.h],
		[],
		[AC_MSG_ERROR([library 'samplerate' not found!])]
	)
	AC_LANG_POP
fi


# ----------------------------------------------------------------------

# Check for linux header files.

if test "x$os" = "xlinux"; then

# TODO

#	AC_LANG_PUSH([C++])
#	AC_CHECK_HEADER(
#		[X11/extensions/Xext.h],
#		[],
#		[AC_MSG_ERROR([missing Xext.h, maybe you need to install the libxext-dev package?])]
#	)
#	AC_LANG_POP

#	AC_LANG_PUSH([C++])
#	AC_CHECK_HEADER(
#		[X11/Xft.h],
#		[],
#		[AC_MSG_ERROR([missing Xft.h, maybe you need to install the libxft-dev package?])]
#	)
#	AC_LANG_POP

	AC_LANG_PUSH([C++])
	AC_CHECK_HEADER(
		[X11/xpm.h],
		[],
		[AC_MSG_ERROR([missing xpm.h, maybe you need to install the libxpm-dev package?])]
	)
	AC_LANG_POP
fi

# ----------------------------------------------------------------------

# finalizing

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
