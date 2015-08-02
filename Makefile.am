AUTOMAKE_OPTIONS = foreign

AM_CXXFLAGS = -Wall -pedantic -Werror

bin_PROGRAMS = giada

giada_SOURCES =                        \
src/main.cpp                           \
src/core/const.h                       \
src/core/channel.h                     \
src/core/channel.cpp                   \
src/core/sampleChannel.h               \
src/core/sampleChannel.cpp             \
src/core/midiChannel.h                 \
src/core/midiChannel.cpp               \
src/core/midiMapConf.h                 \
src/core/midiMapConf.cpp               \
src/core/conf.h                        \
src/core/conf.cpp                      \
src/core/kernelAudio.h                 \
src/core/kernelAudio.cpp               \
src/core/pluginHost.h			             \
src/core/pluginHost.cpp                \
src/core/mixerHandler.h                \
src/core/mixerHandler.cpp              \
src/core/init.h                        \
src/core/init.cpp                      \
src/core/plugin.h                      \
src/core/plugin.cpp                    \
src/core/wave.h                        \
src/core/wave.cpp                      \
src/core/waveFx.h                      \
src/core/waveFx.cpp                    \
src/core/kernelMidi.h                  \
src/core/kernelMidi.cpp                \
src/core/graphics.h                    \
src/core/graphics.cpp                  \
src/core/patch.h                       \
src/core/patch.cpp                     \
src/core/recorder.h                    \
src/core/recorder.cpp                  \
src/core/mixer.h                       \
src/core/mixer.cpp                     \
src/core/dataStorage.h	               \
src/core/dataStorage.cpp               \
src/glue/glue.h                        \
src/glue/glue.cpp                      \
src/gui/dialogs/gd_keyGrabber.h        \
src/gui/dialogs/gd_keyGrabber.cpp      \
src/gui/dialogs/gd_about.h             \
src/gui/dialogs/gd_about.cpp           \
src/gui/dialogs/gd_mainWindow.h        \
src/gui/dialogs/gd_mainWindow.cpp      \
src/gui/dialogs/gd_beatsInput.h        \
src/gui/dialogs/gd_beatsInput.cpp      \
src/gui/dialogs/gd_warnings.h          \
src/gui/dialogs/gd_warnings.cpp        \
src/gui/dialogs/gd_bpmInput.h          \
src/gui/dialogs/gd_bpmInput.cpp        \
src/gui/dialogs/gd_browser.h           \
src/gui/dialogs/gd_browser.cpp         \
src/gui/dialogs/gd_config.h      			 \
src/gui/dialogs/gd_config.cpp          \
src/gui/dialogs/gd_devInfo.h           \
src/gui/dialogs/gd_devInfo.cpp		     \
src/gui/dialogs/gd_pluginList.h	       \
src/gui/dialogs/gd_pluginList.cpp      \
src/gui/dialogs/gd_pluginWindow.h	     \
src/gui/dialogs/gd_pluginWindow.cpp    \
src/gui/dialogs/gd_editor.h            \
src/gui/dialogs/gd_editor.cpp          \
src/gui/dialogs/gd_pluginWindowGUI.h   \
src/gui/dialogs/gd_pluginWindowGUI.cpp \
src/gui/dialogs/gd_midiOutput.h        \
src/gui/dialogs/gd_midiOutput.cpp      \
src/gui/dialogs/gd_midiInput.h         \
src/gui/dialogs/gd_midiInput.cpp       \
src/gui/dialogs/gd_actionEditor.h	     \
src/gui/dialogs/gd_actionEditor.cpp    \
src/gui/elems/ge_column.h              \
src/gui/elems/ge_column.cpp            \
src/gui/elems/ge_sampleChannel.h       \
src/gui/elems/ge_sampleChannel.cpp     \
src/gui/elems/ge_midiChannel.h         \
src/gui/elems/ge_midiChannel.cpp       \
src/gui/elems/ge_midiIoTools.h         \
src/gui/elems/ge_midiIoTools.cpp       \
src/gui/elems/ge_mixed.h               \
src/gui/elems/ge_mixed.cpp             \
src/gui/elems/ge_waveform.h            \
src/gui/elems/ge_waveform.cpp          \
src/gui/elems/ge_browser.h		         \
src/gui/elems/ge_browser.cpp           \
src/gui/elems/ge_actionWidget.h        \
src/gui/elems/ge_actionWidget.cpp      \
src/gui/elems/ge_envelopeChannel.h     \
src/gui/elems/ge_envelopeChannel.cpp   \
src/gui/elems/ge_pianoRoll.h		       \
src/gui/elems/ge_pianoRoll.cpp         \
src/gui/elems/ge_channel.h             \
src/gui/elems/ge_channel.cpp           \
src/gui/elems/ge_muteChannel.h         \
src/gui/elems/ge_muteChannel.cpp       \
src/gui/elems/ge_actionChannel.h       \
src/gui/elems/ge_actionChannel.cpp     \
src/gui/elems/ge_window.h			         \
src/gui/elems/ge_window.cpp            \
src/gui/elems/ge_status.h              \
src/gui/elems/ge_status.cpp            \
src/gui/elems/ge_keyboard.h            \
src/gui/elems/ge_keyboard.cpp          \
src/gui/elems/ge_waveTools.h           \
src/gui/elems/ge_waveTools.cpp         \
src/gui/elems/ge_modeBox.h             \
src/gui/elems/ge_modeBox.cpp           \
src/gui/elems/ge_controller.h          \
src/gui/elems/ge_controller.cpp        \
src/utils/log.h                        \
src/utils/log.cpp                      \
src/utils/gui_utils.h                  \
src/utils/gui_utils.cpp                \
src/utils/utils.h                      \
src/utils/utils.cpp

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
