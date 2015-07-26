AUTOMAKE_OPTIONS = foreign

AM_CXXFLAGS = -Wall -pedantic -Werror

bin_PROGRAMS = giada

giada_SOURCES =              \
src/core/channel.h           \
src/core/channel.cpp         \
src/core/conf.h              \
src/core/conf.cpp            \
src/core/kernelAudio.h       \
src/core/kernelAudio.cpp     \
src/const.h              src/gd_keyGrabber.h        src/glue.h               src/mixerHandler.cpp \
src/gd_about.cpp         src/gd_mainWindow.cpp      src/graphics.cpp         src/mixerHandler.h \
src/gd_about.h           src/gd_mainWindow.h        src/graphics.h           src/patch.cpp \
src/gd_beatsInput.cpp    src/gd_warnings.cpp        src/ge_mixed.cpp         src/patch.h \
src/gd_beatsInput.h      src/gd_warnings.h          src/ge_mixed.h           src/recorder.cpp \
src/gd_bpmInput.cpp      src/ge_waveform.cpp        src/gui_utils.cpp        src/recorder.h \
src/gd_bpmInput.h        src/ge_waveform.h          src/gui_utils.h          src/utils.cpp \
src/gd_browser.cpp       src/init.cpp               src/utils.h \
src/gd_browser.h         src/init.h  				        src/gd_config.cpp         \
src/gg_keyboard.cpp              src/wave.cpp  	         src/gd_config.h \
src/gg_keyboard.h                 src/waveFx.cpp 		       src/gd_editor.cpp \
src/gg_waveTools.cpp     src/main.cpp               src/waveFx.h				     src/gd_editor.h \
src/gg_waveTools.h       src/mixer.cpp              src/wave.h  				     src/gd_keyGrabber.cpp \
src/glue.cpp             src/mixer.h						    src/ge_browser.h		     src/ge_browser.cpp \
src/gd_devInfo.cpp		   src/gd_devInfo.h           src/plugin.h             src/plugin.cpp \
src/pluginHost.h			   src/pluginHost.cpp         src/gd_pluginList.h	     src/gd_pluginList.cpp \
src/gd_pluginWindow.h	   src/gd_pluginWindow.cpp    src/ge_window.h			     src/ge_window.cpp \
src/dataStorage.h			   src/dataStorage.cpp		     \
src/gd_actionEditor.h	   src/gd_actionEditor.cpp    src/ge_muteChannel.h     src/ge_muteChannel.cpp \
src/ge_actionChannel.h   src/ge_actionChannel.cpp   src/gd_pluginWindowGUI.h src/gd_pluginWindowGUI.cpp \
src/ge_actionWidget.h    src/ge_actionWidget.cpp    src/ge_envelopeChannel.h src/ge_envelopeChannel.cpp \
src/ge_pianoRoll.h		   src/ge_pianoRoll.cpp       src/kernelMidi.h         src/kernelMidi.cpp \
src/gd_midiOutput.h      src/gd_midiOutput.cpp      src/gd_midiInput.h       src/gd_midiInput.cpp \
src/sampleChannel.h      src/sampleChannel.cpp      src/midiChannel.cpp      src/midiChannel.h \
src/midiMapConf.cpp      src/midiMapConf.h          src/ge_channel.h         src/ge_channel.cpp \
src/log.h                src/log.cpp                src/ge_column.h          src/ge_column.cpp \
src/ge_sampleChannel.h   src/ge_sampleChannel.cpp   src/ge_midiChannel.h     src/ge_midiChannel.cpp \
src/ge_midiIoTools.h     src/ge_midiIoTools.cpp



# Check for environment: these vars are defined via AM_CONDITIONAL
# inside configure.ac

if LINUX
giada_LDADD = -lsndfile -lfltk -lXext -lX11 -lXft -lXpm -lm \
							src/deps/rtaudio-mod/librtaudio.a -ljack -lasound -lpthread -ldl \
							-lpulse-simple -lpulse -lsamplerate -lrtmidi
#giada_LDFLAGS = -DWITH_VST
endif
if WINDOWS
giada_LDADD   = -lrtaudio -ldsound -lwsock32 -lm -lfltk -lwininet -lgdi32 \
                -lshell32 -lvfw32 -lrpcrt4 -luuid -lcomctl32 -lole32 -lws2_32 \
                -lsndfile -lsamplerate -lrtmidi -lwinmm -lsetupapi -lksuser \
                -lpthreadGC2
giada_LDFLAGS = -mwindows -static
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



#compile libraries

#libs:
#if LINUX
#	@cd rtaudio-mod; echo "Building RtAudio for Linux..."; \
#	./configure --with-jack --with-alsa --with-pulse; \
#	make;
#endif
#if WINDOWS
#	echo "Building RtAudio for Windows: nothing to do"
#endif
#if OSX
#	@cd rtaudio-mod; echo "Building RtAudio for OS X..."; \
#	./configure --with-core; \
#	make;
#endif

src/deps/rtaudio-mod/librtaudio.a:
	@cd src/deps/rtaudio-mod; echo "Building RtAudio for Linux..."; \
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
