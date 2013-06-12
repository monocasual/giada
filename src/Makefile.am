AUTOMAKE_OPTIONS = foreign

# aeffect.h (header from VST SDK) uses 'long long' which is not supported
# in ISO C++ 1998 and -Werror flag breaks the compilation.
# This is a workaround, fixes needed.
if WINDOWS
AM_CXXFLAGS = -Wall -pedantic
else
AM_CXXFLAGS = -Wall -pedantic -Werror
endif

bin_PROGRAMS = giada

giada_SOURCES = \
const.h            gd_keyGrabber.h      glue.h               mixerHandler.cpp \
gd_about.cpp       gd_mainWindow.cpp    graphics.cpp         mixerHandler.h \
gd_about.h         gd_mainWindow.h      graphics.h           patch.cpp \
gd_beatsInput.cpp  gd_warnings.cpp      ge_mixed.cpp         patch.h \
gd_beatsInput.h    gd_warnings.h        ge_mixed.h           recorder.cpp \
gd_bpmInput.cpp    ge_waveform.cpp      gui_utils.cpp        recorder.h \
gd_bpmInput.h      ge_waveform.h        gui_utils.h          utils.cpp \
gd_browser.cpp     init.cpp             channel.h            utils.h \
gd_browser.h       init.h  				      gd_config.cpp \
gg_keyboard.cpp    kernelAudio.cpp      wave.cpp  	         gd_config.h \
gg_keyboard.h      kernelAudio.h        waveFx.cpp 		       gd_editor.cpp \
gg_waveTools.cpp   main.cpp             waveFx.h				     gd_editor.h \
gg_waveTools.h     mixer.cpp            wave.h  				     gd_keyGrabber.cpp \
glue.cpp           mixer.h						  ge_browser.h		     ge_browser.cpp \
gd_devInfo.cpp		 gd_devInfo.h         plugin.h             plugin.cpp \
pluginHost.h			 pluginHost.cpp       gd_pluginList.h	     gd_pluginList.cpp \
gd_pluginWindow.h	 gd_pluginWindow.cpp  ge_window.h			     ge_window.cpp \
dataStorage.h			 dataStorage.cpp		  conf.h					     conf.cpp \
gd_actionEditor.h	 gd_actionEditor.cpp  ge_muteChannel.h     ge_muteChannel.cpp \
ge_actionChannel.h ge_actionChannel.cpp gd_pluginWindowGUI.h gd_pluginWindowGUI.cpp \
ge_actionWidget.h  ge_actionWidget.cpp  ge_envelopeChannel.h ge_envelopeChannel.cpp \
ge_pianoRoll.h		 ge_pianoRoll.cpp     kernelMidi.h         kernelMidi.cpp


# Check for environment: these vars are defined via AM_CONDITIONAL
# inside configure.ac
if LINUX
giada_LDADD = -lsndfile -lfltk -lXext -lX11 -lXft -lXpm -lm -Lrtaudio \
							-l:librtaudio.a -ljack -lasound -lpthread -ldl \
							-lpulse-simple -lsamplerate
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


libs:
	cd RtAudio; echo "Building RtAudio..."; \
	./configure --with-jack --with-alsa --with-pulse; \
	make;



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
