AUTOMAKE_OPTIONS = foreign

# make giada -------------------------------------------------------------------

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
src/core/pluginHost.h		               \
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
src/core/patch_DEPR_.h                 \
src/core/patch_DEPR_.cpp               \
src/core/patch.h                       \
src/core/patch.cpp                     \
src/core/recorder.h                    \
src/core/recorder.cpp                  \
src/core/mixer.h                       \
src/core/mixer.cpp                     \
src/core/dataStorageIni.h	             \
src/core/dataStorageIni.cpp            \
src/core/dataStorageJson.h	           \
src/core/dataStorageJson.cpp           \
src/glue/main.h                        \
src/glue/main.cpp                      \
src/glue/io.h                          \
src/glue/io.cpp                        \
src/glue/storage.h                     \
src/glue/storage.cpp                   \
src/glue/channel.h                     \
src/glue/channel.cpp                   \
src/glue/plugin.h                      \
src/glue/plugin.cpp                    \
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
src/gui/dialogs/gd_actionEditor.h	     \
src/gui/dialogs/gd_actionEditor.cpp    \
src/gui/dialogs/gd_pluginChooser.h     \
src/gui/dialogs/gd_pluginChooser.cpp   \
src/gui/dialogs/midiIO/midiOutputBase.h    \
src/gui/dialogs/midiIO/midiOutputBase.cpp  \
src/gui/dialogs/midiIO/midiOutputSampleCh.h    \
src/gui/dialogs/midiIO/midiOutputSampleCh.cpp    \
src/gui/dialogs/midiIO/midiOutputMidiCh.h  \
src/gui/dialogs/midiIO/midiOutputMidiCh.cpp  \
src/gui/dialogs/midiIO/midiInputBase.h     \
src/gui/dialogs/midiIO/midiInputBase.cpp   \
src/gui/dialogs/midiIO/midiInputChannel.h     \
src/gui/dialogs/midiIO/midiInputChannel.cpp     \
src/gui/dialogs/midiIO/midiInputMaster.h   \
src/gui/dialogs/midiIO/midiInputMaster.cpp   \
src/gui/elems/ge_column.h              \
src/gui/elems/ge_column.cpp            \
src/gui/elems/sampleChannel.h          \
src/gui/elems/sampleChannel.cpp        \
src/gui/elems/midiChannel.h            \
src/gui/elems/midiChannel.cpp          \
src/gui/elems/midiLearner.h            \
src/gui/elems/midiLearner.cpp          \
src/gui/elems/ge_mixed.h               \
src/gui/elems/ge_mixed.cpp             \
src/gui/elems/ge_waveform.h            \
src/gui/elems/ge_waveform.cpp          \
src/gui/elems/browser.h		             \
src/gui/elems/browser.cpp              \
src/gui/elems/baseActionEditor.h       \
src/gui/elems/baseActionEditor.cpp     \
src/gui/elems/envelopeEditor.h         \
src/gui/elems/envelopeEditor.cpp       \
src/gui/elems/pianoRoll.h              \
src/gui/elems/pianoRoll.cpp            \
src/gui/elems/noteEditor.h		         \
src/gui/elems/noteEditor.cpp           \
src/gui/elems/pianoItem.h              \
src/gui/elems/pianoItem.cpp            \
src/gui/elems/channel.h                \
src/gui/elems/channel.cpp              \
src/gui/elems/muteEditor.h             \
src/gui/elems/muteEditor.cpp           \
src/gui/elems/actionEditor.h           \
src/gui/elems/actionEditor.cpp         \
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
src/gui/elems/channelButton.h          \
src/gui/elems/channelButton.cpp        \
src/gui/elems/ge_pluginBrowser.h       \
src/gui/elems/ge_pluginBrowser.cpp     \
src/gui/elems/basics/widget.h     \
src/gui/elems/basics/widget.cpp     \
src/gui/elems/basics/scroll.h     \
src/gui/elems/basics/scroll.cpp     \
src/gui/elems/basics/boxtypes.h     \
src/gui/elems/basics/boxtypes.cpp     \
src/utils/log.h                        \
src/utils/log.cpp                      \
src/utils/gui.h                        \
src/utils/gui.cpp                      \
src/utils/gvector.h                    \
src/utils/fs.h                         \
src/utils/fs.cpp                       \
src/utils/string.h                     \
src/utils/string.cpp

if WITH_VST
giada_SOURCES +=                                              \
src/deps/juce/modules/juce_audio_basics/juce_audio_basics.cpp         \
src/deps/juce/modules/juce_audio_processors/juce_audio_processors.cpp \
src/deps/juce/modules/juce_core/juce_core.cpp                         \
src/deps/juce/modules/juce_data_structures/juce_data_structures.cpp   \
src/deps/juce/modules/juce_events/juce_events.cpp                     \
src/deps/juce/modules/juce_graphics/juce_graphics.cpp                 \
src/deps/juce/modules/juce_gui_basics/juce_gui_basics.cpp             \
src/deps/juce/modules/juce_gui_extra/juce_gui_extra.cpp
endif

# Check for environment: WITH_VST, LINUX, WINDOWS and OSX are varibles defined
# via AM_CONDITIONAL inside configure.ac.
# Note: CPPFLAGS = C preprocessor flags, CXXFLAGS = C++ compiler flags.

giada_CXXFLAGS = -std=c++11 -Wall -Werror
giada_CPPFLAGS =

# TODO - these are flags for Linux only!
# Also, JUCE makes GCC complain if compiled with optimization set to -O2.
# Call configure script as follows:
#
# ./configure CXXFLAGS='-g -O1 -pedantic' --target=linux --enable-vst
#

if WITH_VST
giada_CPPFLAGS +=                          \
  -I./src/deps/juce/modules                \
  -I./src/deps/vst                         \
  -I/usr/include                           \
  -I/usr/include/freetype2                 \
  -DJUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1 \
  -DJUCE_STANDALONE_APPLICATION=1          \
  -DJUCE_PLUGINHOST_VST=1                  \
  -DJUCE_PLUGINHOST_VST3=0                 \
  -DJUCE_PLUGINHOST_AU=0
endif

if LINUX
giada_SOURCES += src/deps/rtaudio-mod/RtAudio.h src/deps/rtaudio-mod/RtAudio.cpp
# -Wno-error=vla: mute rtAudio error on variable length array
# -Wno-error=misleading-indentation: mute JUCE warnings on GCC6
giada_CXXFLAGS += -Wno-error=vla -Wno-error=misleading-indentation
giada_CPPFLAGS += -D__LINUX_ALSA__ -D__LINUX_PULSE__ -D__UNIX_JACK__
giada_LDADD = -lsndfile -lfltk -lXext -lX11 -lXft -lXpm -lm -ljack -lasound \
  -lpthread -ldl -lpulse-simple -lpulse -lsamplerate -lrtmidi -ljansson \
  -lfreetype
endif

if WINDOWS
giada_SOURCES +=                                      \
src/deps/rtaudio-mod/RtAudio.h                        \
src/deps/rtaudio-mod/RtAudio.cpp                      \
src/deps/rtaudio-mod/include/asio.h                   \
src/deps/rtaudio-mod/include/asio.cpp                 \
src/deps/rtaudio-mod/include/asiolist.h               \
src/deps/rtaudio-mod/include/asiolist.cpp             \
src/deps/rtaudio-mod/include/asiodrivers.h            \
src/deps/rtaudio-mod/include/asiodrivers.cpp          \
src/deps/rtaudio-mod/include/iasiothiscallresolver.h  \
src/deps/rtaudio-mod/include/iasiothiscallresolver.cpp
# -Wno-error=misleading-indentation: mute JUCE warnings on GCC6
# -Wno-error=unused-but-set-variable: silence ASIO errors
giada_CXXFLAGS +=                  \
-Wno-error=misleading-indentation  \
-Wno-error=unused-but-set-variable
giada_CPPFLAGS +=                \
-I./src/deps/rtaudio-mod/include \
-D__WINDOWS_ASIO__               \
-D__WINDOWS_WASAPI__             \
-D__WINDOWS_DS__
giada_LDADD = -ldsound -lwsock32 -lm -lfltk -lwininet -lgdi32 \
  -lshell32 -lvfw32 -lrpcrt4 -luuid -lcomctl32 -lole32 -lws2_32 -lsndfile \
  -lsamplerate -lrtmidi -lwinmm -lsetupapi -lksuser -lpthreadGC2 -ljansson \
  -limm32 -lglu32 -lshell32 -lversion -lopengl32 -loleaut32 -lshlwapi -lcomdlg32
giada_LDFLAGS = -mwindows -static
giada_SOURCES += resource.rc
endif

if OSX
# for 32 bit compilation:
# export CXXFLAGS="-m32"
# export LDFLAGS="-m32"
# -ObjC++: Juce requires to build some Objective C code
# -Wno-unknown-pragmas: shut up Juce even more
giada_SOURCES += src/utils/cocoa.mm src/utils/cocoa.h
giada_CXXFLAGS += -ObjC++ -Wno-unknown-pragmas
giada_LDADD = -lsndfile -lm -lpthread -lfltk -lrtmidi -lrtaudio \
	-lsamplerate -ljansson
giada_LDFLAGS = -framework CoreAudio -framework Cocoa -framework Carbon \
  -framework CoreMIDI -framework CoreFoundation -framework Accelerate \
  -framework WebKit -framework QuartzCore -framework IOKit
endif

# used only under MinGW to compile the resource.rc file (program icon)

resource.o:
	windres src/ext/resource.rc -o resource.o

# make test --------------------------------------------------------------------

TESTS = giada_tests
check_PROGRAMS = giada_tests
giada_tests_SOURCES =        \
tests/main.cpp               \
tests/conf.cpp               \
tests/wave.cpp               \
tests/patch.cpp              \
tests/midiMapConf.cpp        \
tests/pluginHost.cpp         \
tests/utils.cpp              \
src/core/conf.cpp            \
src/core/wave.cpp            \
src/core/midiMapConf.cpp     \
src/core/patch.cpp           \
src/core/plugin.cpp          \
src/core/dataStorageIni.cpp  \
src/core/dataStorageJson.cpp \
src/utils/fs.cpp             \
src/utils/string.cpp         \
src/utils/log.cpp

if WITH_VST
giada_tests_SOURCES +=                                        \
src/deps/juce/modules/juce_audio_basics/juce_audio_basics.cpp         \
src/deps/juce/modules/juce_audio_processors/juce_audio_processors.cpp \
src/deps/juce/modules/juce_core/juce_core.cpp                         \
src/deps/juce/modules/juce_data_structures/juce_data_structures.cpp   \
src/deps/juce/modules/juce_events/juce_events.cpp                     \
src/deps/juce/modules/juce_graphics/juce_graphics.cpp                 \
src/deps/juce/modules/juce_gui_basics/juce_gui_basics.cpp             \
src/deps/juce/modules/juce_gui_extra/juce_gui_extra.cpp
endif

giada_tests_LDADD = -ljansson -lsndfile -lsamplerate -lfltk -lXext -lX11 -lXft \
  -lXpm -lm -ljack -lasound -lpthread -ldl -lpulse-simple -lpulse -lrtmidi \
  -lfreetype

giada_tests_CXXFLAGS = -std=c++11

if WITH_VST
giada_tests_CPPFLAGS =                     \
  -I./src/deps/juce/modules                \
  -I./src/deps/vst                         \
  -I/usr/include                           \
  -I/usr/include/freetype2                 \
  -DJUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1 \
  -DJUCE_STANDALONE_APPLICATION=1          \
  -DJUCE_PLUGINHOST_VST=1                  \
  -DJUCE_PLUGINHOST_VST3=0                 \
  -DJUCE_PLUGINHOST_AU=0
endif

# make rename ------------------------------------------------------------------

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
