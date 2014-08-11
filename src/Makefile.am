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
const.h              gd_keyGrabber.h        glue.h               mixerHandler.cpp \
gd_about.cpp         gd_mainWindow.cpp      graphics.cpp         mixerHandler.h \
gd_about.h           gd_mainWindow.h        graphics.h           patch.cpp \
gd_beatsInput.cpp    gd_warnings.cpp        ge_mixed.cpp         patch.h \
gd_beatsInput.h      gd_warnings.h          ge_mixed.h           recorder.cpp \
gd_bpmInput.cpp      ge_waveform.cpp        gui_utils.cpp        recorder.h \
gd_bpmInput.h        ge_waveform.h          gui_utils.h          utils.cpp \
gd_browser.cpp       init.cpp               channel.h            utils.h \
gd_browser.h         init.h  				        gd_config.cpp        channel.cpp \
gg_keyboard.cpp      kernelAudio.cpp        wave.cpp  	         gd_config.h \
gg_keyboard.h        kernelAudio.h          waveFx.cpp 		       gd_editor.cpp \
gg_waveTools.cpp     main.cpp               waveFx.h				     gd_editor.h \
gg_waveTools.h       mixer.cpp              wave.h  				     gd_keyGrabber.cpp \
glue.cpp             mixer.h						    ge_browser.h		     ge_browser.cpp \
gd_devInfo.cpp		   gd_devInfo.h           plugin.h             plugin.cpp \
pluginHost.h			   pluginHost.cpp         gd_pluginList.h	     gd_pluginList.cpp \
gd_pluginWindow.h	   gd_pluginWindow.cpp    ge_window.h			     ge_window.cpp \
dataStorage.h			   dataStorage.cpp		    conf.h					     conf.cpp \
gd_actionEditor.h	   gd_actionEditor.cpp    ge_muteChannel.h     ge_muteChannel.cpp \
ge_actionChannel.h   ge_actionChannel.cpp   gd_pluginWindowGUI.h gd_pluginWindowGUI.cpp \
ge_actionWidget.h    ge_actionWidget.cpp    ge_envelopeChannel.h ge_envelopeChannel.cpp \
ge_pianoRoll.h		   ge_pianoRoll.cpp       kernelMidi.h         kernelMidi.cpp \
gd_midiOutputSetup.h gd_midiOutputSetup.cpp gd_midiGrabber.h     gd_midiGrabber.cpp \
sampleChannel.h      sampleChannel.cpp      midiChannel.cpp      midiChannel.h \
log.h                log.cpp



# Check for environment: these vars are defined via AM_CONDITIONAL
# inside configure.ac

if LINUX
giada_LDADD = -lsndfile -lfltk -lXext -lX11 -lXft -lXpm -lm  \
							-lrtaudio -ljack -lasound -lpthread -ldl \
							-lpulse-simple -lpulse -lsamplerate -lrtmidi
endif
if WINDOWS
giada_LDADD   = -lole32 -lrtaudio -ldsound -lwinmm -lwsock32 -lm \
								-lpthread -lfltk -lwininet -lgdi32 -lshell32 -lvfw32 \
  						  -lrpcrt4 -luuid -lcomctl32 -lole32 -lws2_32 -Lc:/mingw/lib \
  						  -lsndfile -lsamplerate -lrtmidi c:/mingw/lib/libwinmm.a \
  						  c:/mingw/lib/libsetupapi.a c:/mingw/lib/libksuser.a
giada_LDFLAGS = -mwindows -mno-cygwin -static
giada_SOURCES += resource.rc
endif
if OSX
giada_LDADD   = -lsndfile -lm -lpthread -lfltk -lrtmidi -lrtaudio \
								-lsamplerate
giada_LDFLAGS = -framework CoreAudio -framework Cocoa -framework Carbon \
                -framework CoreMIDI -framework CoreFoundation
endif


# used only under MinGW to compile the resource.rc file (program icon)

.rc.o:
	windres $^ -o $@
%.o : %.rc
	windres $^ -o $@



#~ compile libraries
#~ libs:
#~ if LINUX
#~ 	@cd rtaudio; echo "Building RtAudio for Linux..."; \
#~ 	./configure --with-jack --with-alsa --with-pulse; \
#~ 	make;
#~ endif
#~ if WINDOWS
#~ 	@cd rtaudio; echo "Building RtAudio for Windows..."; \
#~ 	./configure --with-asio --with-ds; \
#~ 	make;
#~ endif
#~ if OSX
#~ 	@cd rtaudio; echo "Building RtAudio for OS X..."; \
#~ 	./configure --with-core; \
#~ 	make;
#~ endif



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
