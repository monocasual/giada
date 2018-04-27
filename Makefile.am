AUTOMAKE_OPTIONS = foreign

# Define conditional variables -------------------------------------------------
# WITH_VST, LINUX, WINDOWS and OSX are varibles defined via AM_CONDITIONAL 
# inside configure.ac.


cppFlags =
cxxFlags = -std=c++11 -Wall -Werror
ldAdd =
ldFlags =
sourcesExtra = 
sourcesMain = src/main.cpp 
sourcesCore =                            \
	src/core/const.h                       \
	src/core/types.h                       \
	src/core/channel.h                     \
	src/core/channel.cpp                   \
	src/core/sampleChannel.h               \
	src/core/sampleChannel.cpp             \
	src/core/midiDispatcher.h              \
	src/core/midiDispatcher.cpp            \
	src/core/midiChannel.h                 \
	src/core/midiChannel.cpp               \
	src/core/midiMapConf.h                 \
	src/core/midiMapConf.cpp               \
	src/core/midiEvent.h                   \
	src/core/midiEvent.cpp                 \
	src/core/audioBuffer.h                 \
	src/core/audioBuffer.cpp               \
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
	src/core/patch.h                       \
	src/core/patch.cpp                     \
	src/core/recorder.h                    \
	src/core/recorder.cpp                  \
	src/core/mixer.h                       \
	src/core/mixer.cpp                     \
	src/core/storager.h	                   \
	src/core/storager.cpp                  \
	src/core/clock.h                       \
	src/core/clock.cpp                     \
	src/core/waveManager.h                 \
	src/core/waveManager.cpp               \
	src/core/channelManager.h              \
	src/core/channelManager.cpp            \
	src/core/sampleChannelProc.h           \
	src/core/sampleChannelProc.cpp         \
	src/core/sampleChannelRec.h            \
	src/core/sampleChannelRec.cpp          \
	src/core/midiChannelProc.h             \
	src/core/midiChannelProc.cpp           \
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
	src/glue/transport.h                   \
	src/glue/transport.cpp                 \
	src/glue/recorder.h                    \
	src/glue/recorder.cpp                  \
	src/glue/sampleEditor.h                \
	src/glue/sampleEditor.cpp              \
	src/gui/dialogs/window.h			         \
	src/gui/dialogs/window.cpp             \
	src/gui/dialogs/gd_keyGrabber.h        \
	src/gui/dialogs/gd_keyGrabber.cpp      \
	src/gui/dialogs/about.h                \
	src/gui/dialogs/about.cpp              \
	src/gui/dialogs/gd_mainWindow.h        \
	src/gui/dialogs/gd_mainWindow.cpp      \
	src/gui/dialogs/beatsInput.h           \
	src/gui/dialogs/beatsInput.cpp         \
	src/gui/dialogs/gd_warnings.h          \
	src/gui/dialogs/gd_warnings.cpp        \
	src/gui/dialogs/bpmInput.h             \
	src/gui/dialogs/bpmInput.cpp           \
	src/gui/dialogs/channelNameInput.h     \
	src/gui/dialogs/channelNameInput.cpp   \
	src/gui/dialogs/gd_config.h      			 \
	src/gui/dialogs/gd_config.cpp          \
	src/gui/dialogs/gd_devInfo.h           \
	src/gui/dialogs/gd_devInfo.cpp		     \
	src/gui/dialogs/pluginList.h	         \
	src/gui/dialogs/pluginList.cpp         \
	src/gui/dialogs/pluginWindow.h	       \
	src/gui/dialogs/pluginWindow.cpp       \
	src/gui/dialogs/sampleEditor.h         \
	src/gui/dialogs/sampleEditor.cpp       \
	src/gui/dialogs/pluginWindowGUI.h      \
	src/gui/dialogs/pluginWindowGUI.cpp    \
	src/gui/dialogs/gd_actionEditor.h	     \
	src/gui/dialogs/gd_actionEditor.cpp    \
	src/gui/dialogs/pluginChooser.h        \
	src/gui/dialogs/pluginChooser.cpp      \
	src/gui/dialogs/browser/browserBase.h           \
	src/gui/dialogs/browser/browserBase.cpp         \
	src/gui/dialogs/browser/browserDir.h            \
	src/gui/dialogs/browser/browserDir.cpp          \
	src/gui/dialogs/browser/browserLoad.h           \
	src/gui/dialogs/browser/browserLoad.cpp         \
	src/gui/dialogs/browser/browserSave.h           \
	src/gui/dialogs/browser/browserSave.cpp         \
	src/gui/dialogs/midiIO/midiOutputBase.h          \
	src/gui/dialogs/midiIO/midiOutputBase.cpp        \
	src/gui/dialogs/midiIO/midiOutputSampleCh.h      \
	src/gui/dialogs/midiIO/midiOutputSampleCh.cpp    \
	src/gui/dialogs/midiIO/midiOutputMidiCh.h        \
	src/gui/dialogs/midiIO/midiOutputMidiCh.cpp      \
	src/gui/dialogs/midiIO/midiInputBase.h           \
	src/gui/dialogs/midiIO/midiInputBase.cpp         \
	src/gui/dialogs/midiIO/midiInputChannel.h        \
	src/gui/dialogs/midiIO/midiInputChannel.cpp      \
	src/gui/dialogs/midiIO/midiInputMaster.h         \
	src/gui/dialogs/midiIO/midiInputMaster.cpp       \
	src/gui/elems/midiLearner.h        \
	src/gui/elems/midiLearner.cpp      \
	src/gui/elems/browser.h		         \
	src/gui/elems/browser.cpp          \
	src/gui/elems/soundMeter.h		     \
	src/gui/elems/soundMeter.cpp       \
	src/gui/elems/plugin/pluginBrowser.h                \
	src/gui/elems/plugin/pluginBrowser.cpp              \
	src/gui/elems/plugin/pluginParameter.h              \
	src/gui/elems/plugin/pluginParameter.cpp            \
	src/gui/elems/sampleEditor/waveform.h               \
	src/gui/elems/sampleEditor/waveform.cpp             \
	src/gui/elems/sampleEditor/waveTools.h              \
	src/gui/elems/sampleEditor/waveTools.cpp            \
	src/gui/elems/sampleEditor/volumeTool.h             \
	src/gui/elems/sampleEditor/volumeTool.cpp           \
	src/gui/elems/sampleEditor/boostTool.h              \
	src/gui/elems/sampleEditor/boostTool.cpp            \
	src/gui/elems/sampleEditor/panTool.h                \
	src/gui/elems/sampleEditor/panTool.cpp              \
	src/gui/elems/sampleEditor/pitchTool.h              \
	src/gui/elems/sampleEditor/pitchTool.cpp            \
	src/gui/elems/sampleEditor/rangeTool.h              \
	src/gui/elems/sampleEditor/rangeTool.cpp            \
	src/gui/elems/sampleEditor/shiftTool.h              \
	src/gui/elems/sampleEditor/shiftTool.cpp            \
	src/gui/elems/actionEditor/baseActionEditor.h       \
	src/gui/elems/actionEditor/baseActionEditor.cpp     \
	src/gui/elems/actionEditor/envelopeEditor.h         \
	src/gui/elems/actionEditor/envelopeEditor.cpp       \
	src/gui/elems/actionEditor/pianoRoll.h              \
	src/gui/elems/actionEditor/pianoRoll.cpp            \
	src/gui/elems/actionEditor/noteEditor.h		          \
	src/gui/elems/actionEditor/noteEditor.cpp           \
	src/gui/elems/actionEditor/basePianoItem.h          \
	src/gui/elems/actionEditor/basePianoItem.cpp        \
	src/gui/elems/actionEditor/pianoItem.h              \
	src/gui/elems/actionEditor/pianoItem.cpp            \
	src/gui/elems/actionEditor/pianoItemOrphaned.h      \
	src/gui/elems/actionEditor/pianoItemOrphaned.cpp    \
	src/gui/elems/actionEditor/muteEditor.h             \
	src/gui/elems/actionEditor/muteEditor.cpp           \
	src/gui/elems/actionEditor/actionEditor.h           \
	src/gui/elems/actionEditor/actionEditor.cpp         \
	src/gui/elems/actionEditor/action.h                 \
	src/gui/elems/actionEditor/action.cpp               \
	src/gui/elems/actionEditor/gridTool.h               \
	src/gui/elems/actionEditor/gridTool.cpp             \
	src/gui/elems/mainWindow/mainIO.h          \
	src/gui/elems/mainWindow/mainIO.cpp        \
	src/gui/elems/mainWindow/mainMenu.h        \
	src/gui/elems/mainWindow/mainMenu.cpp      \
	src/gui/elems/mainWindow/mainTimer.h       \
	src/gui/elems/mainWindow/mainTimer.cpp     \
	src/gui/elems/mainWindow/mainTransport.h   \
	src/gui/elems/mainWindow/mainTransport.cpp \
	src/gui/elems/mainWindow/beatMeter.h       \
	src/gui/elems/mainWindow/beatMeter.cpp     \
	src/gui/elems/mainWindow/keyboard/channelMode.h           \
	src/gui/elems/mainWindow/keyboard/channelMode.cpp         \
	src/gui/elems/mainWindow/keyboard/channelButton.h         \
	src/gui/elems/mainWindow/keyboard/channelButton.cpp       \
	src/gui/elems/mainWindow/keyboard/channelStatus.h         \
	src/gui/elems/mainWindow/keyboard/channelStatus.cpp       \
	src/gui/elems/mainWindow/keyboard/keyboard.h              \
	src/gui/elems/mainWindow/keyboard/keyboard.cpp            \
	src/gui/elems/mainWindow/keyboard/column.h                \
	src/gui/elems/mainWindow/keyboard/column.cpp              \
	src/gui/elems/mainWindow/keyboard/sampleChannel.h         \
	src/gui/elems/mainWindow/keyboard/sampleChannel.cpp       \
	src/gui/elems/mainWindow/keyboard/midiChannel.h           \
	src/gui/elems/mainWindow/keyboard/midiChannel.cpp         \
	src/gui/elems/mainWindow/keyboard/channel.h               \
	src/gui/elems/mainWindow/keyboard/channel.cpp             \
	src/gui/elems/mainWindow/keyboard/sampleChannelButton.h   \
	src/gui/elems/mainWindow/keyboard/sampleChannelButton.cpp \
	src/gui/elems/mainWindow/keyboard/midiChannelButton.h     \
	src/gui/elems/mainWindow/keyboard/midiChannelButton.cpp   \
	src/gui/elems/config/tabMisc.h         \
	src/gui/elems/config/tabMisc.cpp       \
	src/gui/elems/config/tabMidi.h         \
	src/gui/elems/config/tabMidi.cpp       \
	src/gui/elems/config/tabAudio.h        \
	src/gui/elems/config/tabAudio.cpp      \
	src/gui/elems/config/tabBehaviors.h    \
	src/gui/elems/config/tabBehaviors.cpp  \
	src/gui/elems/config/tabPlugins.h      \
	src/gui/elems/config/tabPlugins.cpp    \
	src/gui/elems/basics/scroll.h          \
	src/gui/elems/basics/scroll.cpp        \
	src/gui/elems/basics/boxtypes.h        \
	src/gui/elems/basics/boxtypes.cpp      \
	src/gui/elems/basics/baseButton.h      \
	src/gui/elems/basics/baseButton.cpp    \
	src/gui/elems/basics/statusButton.h    \
	src/gui/elems/basics/statusButton.cpp  \
	src/gui/elems/basics/button.h          \
	src/gui/elems/basics/button.cpp        \
	src/gui/elems/basics/idButton.h        \
	src/gui/elems/basics/idButton.cpp      \
	src/gui/elems/basics/resizerBar.h      \
	src/gui/elems/basics/resizerBar.cpp    \
	src/gui/elems/basics/input.h           \
	src/gui/elems/basics/input.cpp         \
	src/gui/elems/basics/liquidScroll.h    \
	src/gui/elems/basics/liquidScroll.cpp  \
	src/gui/elems/basics/choice.h          \
	src/gui/elems/basics/choice.cpp        \
	src/gui/elems/basics/dial.h            \
	src/gui/elems/basics/dial.cpp          \
	src/gui/elems/basics/box.h             \
	src/gui/elems/basics/box.cpp           \
	src/gui/elems/basics/slider.h          \
	src/gui/elems/basics/slider.cpp        \
	src/gui/elems/basics/progress.h        \
	src/gui/elems/basics/progress.cpp      \
	src/gui/elems/basics/check.h           \
	src/gui/elems/basics/check.cpp         \
	src/gui/elems/basics/radio.h           \
	src/gui/elems/basics/radio.cpp         \
	src/utils/log.h                        \
	src/utils/log.cpp                      \
	src/utils/time.h                       \
	src/utils/time.cpp                     \
	src/utils/math.h                       \
	src/utils/math.cpp                     \
	src/utils/gui.h                        \
	src/utils/gui.cpp                      \
	src/utils/gvector.h                    \
	src/utils/fs.h                         \
	src/utils/fs.cpp                       \
	src/utils/ver.h                        \
	src/utils/ver.cpp                      \
	src/utils/string.h                     \
	src/utils/string.cpp                   \
	src/deps/rtaudio-mod/RtAudio.h         \
	src/deps/rtaudio-mod/RtAudio.cpp
sourcesTests =                 \
	tests/main.cpp               \
	tests/conf.cpp               \
	tests/wave.cpp               \
	tests/waveManager.cpp        \
	tests/patch.cpp              \
	tests/midiMapConf.cpp        \
	tests/pluginHost.cpp         \
	tests/utils.cpp              \
	tests/recorder.cpp           \
	tests/waveFx.cpp             \
	tests/audioBuffer.cpp        \
	tests/sampleChannelProc.cpp

if WITH_VST

sourcesExtra += \
	src/deps/juce/modules/juce_audio_basics/juce_audio_basics.cpp         \
	src/deps/juce/modules/juce_audio_processors/juce_audio_processors.cpp \
	src/deps/juce/modules/juce_core/juce_core.cpp                         \
	src/deps/juce/modules/juce_data_structures/juce_data_structures.cpp   \
	src/deps/juce/modules/juce_events/juce_events.cpp                     \
	src/deps/juce/modules/juce_graphics/juce_graphics.cpp                 \
	src/deps/juce/modules/juce_gui_basics/juce_gui_basics.cpp             \
	src/deps/juce/modules/juce_gui_extra/juce_gui_extra.cpp

cppFlags += \
  -I$(top_srcdir)/src/deps/juce/modules      \
  -I$(top_srcdir)/src/deps/vst               \
  -I/usr/include                             \
  -I/usr/include/freetype2                   \
  -DJUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1   \
  -DJUCE_STANDALONE_APPLICATION=1            \
  -DJUCE_PLUGINHOST_VST=1                    \
  -DJUCE_PLUGINHOST_VST3=0                   \
  -DJUCE_PLUGINHOST_AU=0                     \
  -DJUCE_WEB_BROWSER=0 

endif

if !WITH_SYSTEM_CATCH

cppFlags += -I$(top_srcdir)/tests/catch2/single_include

endif

if WINDOWS

sourcesExtra += \
	src/deps/rtaudio-mod/include/asio.h                    \
	src/deps/rtaudio-mod/include/asio.cpp                  \
	src/deps/rtaudio-mod/include/asiolist.h                \
	src/deps/rtaudio-mod/include/asiolist.cpp              \
	src/deps/rtaudio-mod/include/asiodrivers.h             \
	src/deps/rtaudio-mod/include/asiodrivers.cpp           \
	src/deps/rtaudio-mod/include/iasiothiscallresolver.h   \
	src/deps/rtaudio-mod/include/iasiothiscallresolver.cpp \
	resource.rc

cppFlags += \
	-I$(top_srcdir)/src/deps/rtaudio-mod/include \
	-D__WINDOWS_ASIO__                           \
	-D__WINDOWS_WASAPI__                         \
	-D__WINDOWS_DS__

cxxFlags += -Wno-error

ldAdd += -ldsound -lwsock32 -lm -lfltk -lwininet -lgdi32 -lshell32 -lvfw32 \
  -lrpcrt4 -luuid -lcomctl32 -lole32 -lws2_32 -lsndfile -lsamplerate -lrtmidi \
  -lwinmm -lsetupapi -lksuser -ljansson -limm32 -lglu32 -lshell32 -lversion \
  -lopengl32 -loleaut32 -lshlwapi -lcomdlg32 -lflac -lvorbis -logg -lvorbisenc

# Generate a GUI application (-mwindows), make the build static (-static).
ldFlags += -mwindows -static

endif

if LINUX

# Add preprocessor flags to enable ALSA, Pulse and JACK in RtAudio.
cppFlags += -D__LINUX_ALSA__ -D__LINUX_PULSE__ -D__UNIX_JACK__

# Don't stop on JUCE's unused functions.
cxxFlags += -Wno-error=unused-function

ldAdd += -lsndfile -lfltk -lXext -lX11 -lXft -lXpm -lm -ljack -lasound \
  -lpthread -ldl -lpulse-simple -lpulse -lsamplerate -lrtmidi -ljansson \
  -lfreetype

endif

if OSX

sourcesExtra += src/utils/cocoa.mm src/utils/cocoa.h

# Add preprocessor flags to enable CoreAudio in RtAudio.
cppFlags += -D__MACOSX_CORE__

# -ObjC++: Juce requires to build some Objective C code
cxxFlags += -ObjC++ -Wno-auto-var-id

ldAdd += -lsndfile -lfltk -lrtmidi -lsamplerate -ljansson -lm -lpthread \
  -lFLAC -logg -lvorbis -lvorbisenc

ldFlags += -framework CoreAudio -framework Cocoa -framework Carbon \
  -framework CoreMIDI -framework CoreFoundation -framework Accelerate \
  -framework WebKit -framework QuartzCore -framework IOKit

endif

# make giada -------------------------------------------------------------------

bin_PROGRAMS = giada

giada_SOURCES = $(sourcesCore)
giada_SOURCES += $(sourcesMain)
giada_SOURCES += $(sourcesExtra)
giada_CPPFLAGS = $(cppFlags)
giada_CXXFLAGS = $(cxxFlags)
giada_LDADD = $(ldAdd)
giada_LDFLAGS = $(ldFlags)

# Used only under MinGW to compile the resource.rc file (program icon)
resource.o:
	windres src/ext/resource.rc -o resource.o

# make check -------------------------------------------------------------------

TESTS = giada_tests

check_PROGRAMS = giada_tests
giada_tests_SOURCES = $(sourcesCore)
giada_tests_SOURCES += $(sourcesExtra)
giada_tests_SOURCES += $(sourcesTests)
giada_tests_CPPFLAGS = $(cppFlags) 
giada_tests_CPPFLAGS += -DTESTS
giada_tests_CXXFLAGS = $(cxxFlags)
giada_tests_LDADD = $(ldAdd)
giada_tests_LDFLAGS = $(ldFlags)

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
