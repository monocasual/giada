AUTOMAKE_OPTIONS = foreign

AM_CXXFLAGS = -Wall -pedantic -Werror

# aeffect.h (header from VST SDK) uses 'long long' which is not supported
# in ISO C++ 1998 and -Werror flag breaks the compilation.
# This is a workaround, fixes needed.
if WINDOWS
AM_CXXFLAGS = -Wall -pedantic
endif

bin_PROGRAMS = giada

giada_SOURCES = *.h *.cpp

# Check for environment: these vars are defined via AM_CONDITIONAL
# inside configure.ac
if LINUX
giada_LDADD = -lsndfile -lfltk -lXext -lX11 -lXft -lXpm -lm -lrtaudio  \
						  -ljack -lasound -lpthread -ldl -lpulse-simple -lsamplerate
endif
if WINDOWS
giada_LDADD   = -lole32 -lrtaudio -ldsound -lwinmm -lwsock32 -lm       \
  						  -lpthread -lfltk -lwininet -lgdi32 -lshell32 -lvfw32   \
  						  -lrpcrt4 -luuid -lcomctl32 -lole32 -lws2_32            \
  						  -Lc:/mingw/lib -lsndfile -lsamplerate
giada_LDFLAGS = -mwindows -mno-cygwin -static
giada_SOURCES += resource.rc
endif
if OSX
giada_LDADD   = /usr/local/lib/libsndfile.a -lrtaudio -lm -lpthread -lfltk
giada_LDFLAGS = -framework CoreAudio -framework Cocoa -framework Carbon \
                -lsamplerate
endif


# used only under MinGW to compile the resource.rc file (program icon)
.rc.o:
	windres $^ -o $@
%.o : %.rc
	windres $^ -o $@


# rename the binaries
if LINUX
rename:
	mv giada giada_lin
endif
if WINDOWS
rename:
	mv giada giada_win.exe
endif
if OSX
rename:
	mv giada giada_osx
endif
