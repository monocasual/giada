#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# prereq & init

AC_PREREQ(2.60)
AC_INIT([giada], [0.16], [giadaloopmachine@gmail.com])
AC_CONFIG_SRCDIR([src/main.cpp])
AM_INIT_AUTOMAKE([subdir-objects])

# ------------------------------------------------------------------------------

# test the build environment. These vars are used in Makefile.am during
# the linking of the libraries.
# Usage: ./configure --target=[windows | linux | osx]

if test "$target" = ""; then
	AC_MSG_ERROR(["target OS not specified. Please run ./configure --target=<windows | linux | freebsd | osx>"])
fi

case "$target" in
  linux)
    os=linux
    ;;
  freebsd)
    os=freebsd
    ;;
  windows)
    os=windows
    ;;
  osx)
    os=osx
    ;;
  freebsd)
    os=freebsd
    ;;
  *)
    AC_MSG_ERROR(["Unrecognised target OS: $target"])
    ;;
esac
AM_CONDITIONAL(LINUX,   test "x$os" = "xlinux")
AM_CONDITIONAL(FREEBSD, test "x$os" = "xfreebsd")
AM_CONDITIONAL(WINDOWS, test "x$os" = "xwindows")
AM_CONDITIONAL(OSX,     test "x$os" = "xosx")
AM_CONDITIONAL(FREEBSD, test "x$os" = "xfreebsd")

# ------------------------------------------------------------------------------

# --enable-vst. VST compilation is disabled by default
#
# WITH_VST, if present, will be passed to gcc as -DWITH_VST
#
# AC_ARG_ENABLE (
# feature,                [--enable-] + [feature], eg --enable-vst
# help-string,
# [action-if-given],      == gcc ... -DWITH_VST
# [action-if-not-given])  not used here

AC_ARG_ENABLE(
	[vst],
	AS_HELP_STRING([--enable-vst], [enable vst support]),
  [AC_DEFINE(WITH_VST) AM_CONDITIONAL(WITH_VST, true)],
	[AM_CONDITIONAL(WITH_VST, false)]
)

# ------------------------------------------------------------------------------

# --enable-system-catch. If enabled, use the system-provided Catch. Use bundled
# version otherwise (default mode).

AC_ARG_ENABLE(
	[system-catch],
	AS_HELP_STRING([--enable-system-catch], [use system-provided Catch library]),
  [AC_DEFINE(WITH_SYSTEM_CATCH) AM_CONDITIONAL(WITH_SYSTEM_CATCH, true)],
	[AM_CONDITIONAL(WITH_SYSTEM_CATCH, false)]
)

# ------------------------------------------------------------------------------

# --debug. Enable debug compilation

AC_ARG_ENABLE(
	[debug],
	AS_HELP_STRING([--enable-debug], [enable debug mode (asserts, ...)]),
  [],
	[AC_DEFINE(NDEBUG)]
)

# ------------------------------------------------------------------------------

# test if files needed for Travis CI are present. If so, define a new macro
# RUN_TESTS_WITH_LOCAL_FILES used during the test suite

if test -f "giada-midimaps-master.zip" && test -f "dexed.tar.xz" ; then
  AC_DEFINE(RUN_TESTS_WITH_LOCAL_FILES)
fi

# ------------------------------------------------------------------------------

# Check for C++ compiler

AC_PROG_CXX

# Check for Objective-C++ compiler

AC_PROG_OBJCXX

# Check for C compiler (TODO - is that really needed?)

AC_PROG_CC

# Check for make

AC_PROG_MAKE_SET

# ------------------------------------------------------------------------------

# Check for libraries.

AC_CHECK_LIB(
	[pthread],
	[pthread_exit],
	[],
	[AC_MSG_ERROR([error: library 'pthread' not found!])]
)

# ------------------------------------------------------------------------------

# Check for generic headers (fltk, rtaudio and libsndfile are static,
# we ask if headers are available)

AC_LANG_PUSH([C++])
AC_CHECK_HEADER(
	[FL/Fl.H],
	[],
	[AC_MSG_ERROR([library 'fltk' not found!])]
)
AC_LANG_POP

if test "x$os" = "xosx"; then
	AC_LANG_PUSH([C++])
	AC_CHECK_HEADER(
		[RtMidi.h],
		[],
		[AC_MSG_ERROR([library 'rtMidi' not found!])]
	)
	AC_LANG_POP
else
	AC_LANG_PUSH([C++])
	AC_CHECK_HEADER(
		[rtmidi/RtMidi.h],
		[],
		[AC_MSG_ERROR([library 'rtMidi' not found!])]
	)
	AC_LANG_POP
fi


AC_LANG_PUSH([C++])
AC_CHECK_HEADER(
	[jansson.h],
	[],
	[AC_MSG_ERROR([library 'Jansson' not found!])]
)
AC_LANG_POP

AC_LANG_PUSH([C++])
AC_CHECK_HEADER(
	[sndfile.h],
	[],
	[AC_MSG_ERROR([library 'libsndfile' not found!])]
)
AC_LANG_POP

#~ AC_LANG_PUSH([C++])
#~ AC_CHECK_HEADER(
	#~ [RtAudio.h],
	#~ [],
	#~ [AC_MSG_ERROR([library 'RtAudio' not found!])]
#~ )
#~ AC_LANG_POP

AC_LANG_PUSH([C++])
AC_CHECK_HEADER(
	[samplerate.h],
	[],
	[AC_MSG_ERROR([library 'samplerate' not found!])]
)
AC_LANG_POP



# ------------------------------------------------------------------------------

# Check for linux header files.

if test "x$os" = "xlinux" || test "x$os" = "xfreebsd"; then

	AC_LANG_PUSH([C++])
	AC_CHECK_HEADER(
		[X11/xpm.h],
		[],
		[AC_MSG_ERROR([missing xpm.h, maybe you need to install the libxpm-dev package?])]
	)
	AC_LANG_POP
fi

# ------------------------------------------------------------------------------

# finalizing

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
