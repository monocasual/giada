# ------------------------------------------------------------------------------
# Preliminary setup
# ------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.12)

# CMAKE_OSX_DEPLOYMENT_TARGET should be set prior to the first project() or
# enable_language() command invocation because it may influence configuration
# of the toolchain and flags.
# Also, see https://stackoverflow.com/questions/34208360/cmake-seems-to-ignore-cmake-osx-deployment-target

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	set(CMAKE_OSX_DEPLOYMENT_TARGET "10.14" CACHE STRING "Minimum OS X deployment version")
endif()

# ------------------------------------------------------------------------------
# Project
# ------------------------------------------------------------------------------

project(giada LANGUAGES CXX)

# ------------------------------------------------------------------------------
# Lists definition
#
# SOURCES - contains the source files
# PREPROCESSOR_DEFS - preprocessor definitions
# INCLUDE_DIRS - include directories (e.g. -I)
# COMPILER_OPTIONS - additional flags for the compiler
# LIBRARIES - external dependencies to link
# COMPILER_FEATURES - e.g. C++17
# TARGET_PROPERTIES - additional properties for the target 'giada'.
# ------------------------------------------------------------------------------

list(APPEND SOURCES
	src/main.cpp
	src/mapper.cpp
	src/core/engine.cpp
	src/core/rendering/renderer.cpp
    src/core/rendering/sampleReactions.cpp
    src/core/rendering/sampleRendering.cpp
    src/core/rendering/midiReactions.cpp
	src/core/rendering/midiOutput.cpp
	src/core/rendering/midiRendering.cpp
	src/core/api/mainApi.cpp
	src/core/api/channelsApi.cpp
	src/core/api/sampleEditorApi.cpp
	src/core/api/actionEditorApi.cpp
	src/core/api/pluginsApi.cpp
	src/core/api/storageApi.cpp
	src/core/api/IOApi.cpp
	src/core/api/configApi.cpp
	src/core/worker.cpp
	src/core/eventDispatcher.cpp
	src/core/midiDispatcher.cpp
	src/core/midiMapper.cpp
	src/core/midiEvent.cpp
	src/core/quantizer.cpp
	src/core/confFactory.cpp
	src/core/patchFactory.cpp
	src/core/kernelAudio.cpp
	src/core/jackTransport.cpp
	src/core/sequencer.cpp
	src/core/metronome.cpp
	src/core/init.cpp
	src/core/wave.cpp
	src/core/waveFx.cpp
	src/core/kernelMidi.cpp
	src/core/patch.cpp
	src/core/actions/actionFactory.cpp
	src/core/actions/actionRecorder.cpp
	src/core/mixer.cpp
	src/core/jackSynchronizer.cpp
	src/core/midiSynchronizer.cpp
	src/core/waveFactory.cpp
	src/core/recorder.cpp
	src/core/midiLearnParam.cpp
	src/core/resampler.cpp
	src/core/waveReader.cpp
	src/core/plugins/pluginHost.cpp
	src/core/plugins/pluginManager.cpp
	src/core/plugins/plugin.cpp
	src/core/plugins/pluginState.cpp
	src/core/plugins/pluginFactory.cpp
	src/core/channels/channelManager.cpp
	src/core/channels/sampleAdvancer.cpp
	src/core/channels/samplePlayer.cpp
	src/core/channels/midiLightning.cpp
	src/core/channels/midiLearn.cpp
	src/core/channels/channel.cpp
	src/core/channels/sampleChannel.cpp
	src/core/channels/midiChannel.cpp
	src/core/channels/channelShared.cpp
	src/core/channels/channelFactory.cpp
	src/core/model/sequencer.cpp
	src/core/model/mixer.cpp
	src/core/model/model.cpp
	src/core/model/channels.cpp
	src/core/model/actions.cpp
	src/core/idManager.cpp
	src/glue/main.cpp
	src/glue/io.cpp
	src/glue/storage.cpp
	src/glue/channel.cpp
	src/glue/plugin.cpp
	src/glue/sampleEditor.cpp
	src/glue/actionEditor.cpp
	src/glue/config.cpp
	src/glue/layout.cpp
	src/gui/ui.cpp
	src/gui/model.cpp
	src/gui/dialogs/window.cpp
	src/gui/dispatcher.cpp
	src/gui/updater.cpp
	src/gui/langMapper.cpp
	src/gui/drawing.cpp
	src/gui/dialogs/progress.cpp
	src/gui/dialogs/keyGrabber.cpp
	src/gui/dialogs/about.cpp
	src/gui/dialogs/mainWindow.cpp
	src/gui/dialogs/beatsInput.cpp
	src/gui/dialogs/warnings.cpp
	src/gui/dialogs/bpmInput.cpp
	src/gui/dialogs/channelNameInput.cpp
	src/gui/dialogs/channelRouting.cpp
	src/gui/dialogs/config.cpp
	src/gui/dialogs/pluginList.cpp
	src/gui/dialogs/pluginWindow.cpp
	src/gui/dialogs/sampleEditor.cpp
	src/gui/dialogs/pluginWindowGUI.cpp
	src/gui/dialogs/pluginChooser.cpp
	src/gui/dialogs/missingAssets.cpp
	src/gui/dialogs/actionEditor/baseActionEditor.cpp
	src/gui/dialogs/actionEditor/sampleActionEditor.cpp
	src/gui/dialogs/actionEditor/midiActionEditor.cpp
	src/gui/dialogs/browser/browserBase.cpp
	src/gui/dialogs/browser/browserDir.cpp
	src/gui/dialogs/browser/browserLoad.cpp
	src/gui/dialogs/browser/browserSave.cpp
	src/gui/dialogs/midiIO/midiOutputBase.cpp
	src/gui/dialogs/midiIO/midiOutputSampleCh.cpp
	src/gui/dialogs/midiIO/midiOutputMidiCh.cpp
	src/gui/dialogs/midiIO/midiInputBase.cpp
	src/gui/dialogs/midiIO/midiInputChannel.cpp
	src/gui/dialogs/midiIO/midiInputMaster.cpp
	src/gui/elems/midiIO/midiLearner.cpp
	src/gui/elems/midiIO/midiLearnerPack.cpp
    src/gui/elems/fileBrowser.cpp
	src/gui/elems/soundMeter.cpp
	src/gui/elems/keyBinder.cpp
	src/gui/elems/plugin/pluginBrowser.cpp
	src/gui/elems/plugin/pluginParameter.cpp
	src/gui/elems/plugin/pluginElement.cpp
	src/gui/elems/sampleEditor/waveform.cpp
	src/gui/elems/sampleEditor/waveTools.cpp
    src/gui/elems/volumeTool.cpp
	src/gui/elems/panTool.cpp
	src/gui/elems/midiActivity.cpp
	src/gui/elems/sampleEditor/pitchTool.cpp
	src/gui/elems/sampleEditor/rangeTool.cpp
	src/gui/elems/sampleEditor/shiftTool.cpp
	src/gui/elems/actionEditor/baseActionEditor.cpp
	src/gui/elems/actionEditor/baseAction.cpp
	src/gui/elems/actionEditor/envelopeEditor.cpp
	src/gui/elems/actionEditor/velocityEditor.cpp
	src/gui/elems/actionEditor/envelopePoint.cpp
	src/gui/elems/actionEditor/pianoRoll.cpp
	src/gui/elems/actionEditor/pianoItem.cpp
	src/gui/elems/actionEditor/sampleActionEditor.cpp
	src/gui/elems/actionEditor/sampleAction.cpp
	src/gui/elems/actionEditor/gridTool.cpp
	src/gui/elems/actionEditor/splitScroll.cpp
	src/gui/elems/mainWindow/mainIO.cpp
	src/gui/elems/mainWindow/mainMenu.cpp
	src/gui/elems/mainWindow/mainTimer.cpp
	src/gui/elems/mainWindow/mainTransport.cpp
	src/gui/elems/mainWindow/sequencer.cpp
	src/gui/elems/mainWindow/keyboard/sampleChannelMode.cpp
	src/gui/elems/mainWindow/keyboard/channelButton.cpp
	src/gui/elems/mainWindow/keyboard/channelStatus.cpp
	src/gui/elems/mainWindow/keyboard/keyboard.cpp
	src/gui/elems/mainWindow/keyboard/column.cpp
	src/gui/elems/mainWindow/keyboard/sampleChannel.cpp
	src/gui/elems/mainWindow/keyboard/midiChannel.cpp
	src/gui/elems/mainWindow/keyboard/channel.cpp
	src/gui/elems/mainWindow/keyboard/sampleChannelButton.cpp
	src/gui/elems/mainWindow/keyboard/midiChannelButton.cpp
	src/gui/elems/config/tabMisc.cpp
	src/gui/elems/config/tabMidi.cpp
	src/gui/elems/config/tabAudio.cpp
	src/gui/elems/config/tabBehaviors.cpp
	src/gui/elems/config/tabPlugins.cpp
	src/gui/elems/config/tabBindings.cpp
	src/gui/elems/config/stringMenu.cpp
	src/gui/elems/basics/button.cpp
	src/gui/elems/basics/imageButton.cpp
	src/gui/elems/basics/textButton.cpp
	src/gui/elems/basics/scroll.cpp
	src/gui/elems/basics/pack.cpp
	src/gui/elems/basics/group.cpp
	src/gui/elems/basics/scrollPack.cpp
	src/gui/elems/basics/boxtypes.cpp
	src/gui/elems/basics/resizerBar.cpp
	src/gui/elems/basics/input.cpp
	src/gui/elems/basics/liquidScroll.cpp
	src/gui/elems/basics/choice.cpp
	src/gui/elems/basics/dial.cpp
	src/gui/elems/basics/box.cpp
	src/gui/elems/basics/slider.cpp
	src/gui/elems/basics/progress.cpp
	src/gui/elems/basics/check.cpp
	src/gui/elems/basics/split.cpp
	src/gui/elems/basics/browser.cpp
	src/gui/elems/basics/flex.cpp
	src/gui/elems/basics/tabs.cpp
	src/gui/elems/basics/menu.cpp
	src/utils/log.cpp
	src/utils/time.cpp
	src/utils/math.cpp
	src/utils/gui.cpp
	src/utils/fs.cpp
	src/utils/ver.cpp
	src/utils/string.cpp
	src/deps/rtaudio/RtAudio.cpp
	src/deps/mcl-audio-buffer/src/audioBuffer.cpp)

list(APPEND PREPROCESSOR_DEFS 
	__RTAUDIO_DUMMY__)
list(APPEND INCLUDE_DIRS
	${CMAKE_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/src)
list(APPEND COMPILER_OPTIONS)
list(APPEND LIBRARIES)
list(APPEND COMPILER_FEATURES cxx_std_20)
list(APPEND TARGET_PROPERTIES)

# ------------------------------------------------------------------------------
# Detect OS
# ------------------------------------------------------------------------------

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	set(OS_LINUX 1)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	set(OS_WINDOWS 1)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	set(OS_MACOS 1)
elseif(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
	set(OS_FREEBSD 1)
else()
	message(FATAL_ERROR "Unsupported platform '${CMAKE_SYSTEM_NAME}', quitting.")
endif()

# ------------------------------------------------------------------------------
# Compiler warnings
# ------------------------------------------------------------------------------

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	list(APPEND COMPILER_OPTIONS /W4 /bigobj /external:anglebrackets /external:W0)
else()
	list(APPEND COMPILER_OPTIONS 
		-Wall -Wextra -Wpedantic -Wunreachable-code -Wcast-align
		-Wno-implicit-fallthrough -Wno-ignored-qualifiers -Wredundant-decls
		-Woverloaded-virtual -Wreorder)
endif()

# ------------------------------------------------------------------------------
# Options
# ------------------------------------------------------------------------------

set(VST2_SDK_PATH "" CACHE STRING "Path to VST2 SDK")

option(WITH_VST2 "Enable VST2 support (requires path to VST2 SDK with -DVST2_SDK_PATH=...)." OFF)
option(WITH_VST3 "Enable VST3 support." OFF)
option(WITH_TESTS "Include the test suite." OFF)

if(DEFINED OS_LINUX)
	option(WITH_ALSA "Enable ALSA support (Linux only)." ON)
	option(WITH_PULSE "Enable PulseAudio support (Linux only)." ON)
	option(WITH_JACK "Enable JACK support (Linux only)." ON)
endif()

if(WITH_TESTS)
	list(APPEND PREPROCESSOR_DEFS 
		WITH_TESTS
		TEST_RESOURCES_DIR="${CMAKE_SOURCE_DIR}/tests/resources/")
endif()

if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
	list(APPEND PREPROCESSOR_DEFS NDEBUG)
endif()

# ------------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------------

# Threads (system)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
list(APPEND LIBRARIES Threads::Threads)

# pkg-config/pkgconf, required to find some external dependencies on some
# platforms
find_package(PkgConfig)

# RtMidi

find_package(RtMidi CONFIG)
if (RtMidi_FOUND)
	list(APPEND LIBRARIES RtMidi::rtmidi)
	message("RtMidi library found in " ${RtMidi_DIR})
elseif (PkgConfig_FOUND)
	pkg_check_modules(RtMidi IMPORTED_TARGET rtmidi)
	if (RtMidi_FOUND)
		list(APPEND LIBRARIES PkgConfig::RtMidi)
		message("RtMidi library found")
	endif()
endif()

if (NOT RtMidi_FOUND)
	# Fallback to find_library mode (in case rtmidi is too old). 
	find_library(LIBRARY_RTMIDI NAMES rtmidi)
	list(APPEND LIBRARIES ${LIBRARY_RTMIDI})

	if (NOT LIBRARY_RTMIDI)
		message(FATAL_ERROR "Can't find RtMidi, aborting.")
	endif()

	message("RtMidi library found in " ${RtMidi_DIR})
endif()

# RtMidi header path may vary across OSes, so a fix is needed.
# TODO - Find a way to avoid this additional step

find_path(LIBRARY_RTMIDI_INCLUDE_DIR RtMidi.h PATH_SUFFIXES rtmidi)
list(APPEND INCLUDE_DIRS ${LIBRARY_RTMIDI_INCLUDE_DIR})

# FLTK 

include (ExternalProject)
ExternalProject_Add(
    fltk
	SOURCE_DIR ${CMAKE_SOURCE_DIR}/src/deps/fltk
	INSTALL_COMMAND "" # Don't install
	CMAKE_ARGS 
		-DFLTK_BUILD_TEST=OFF 
		-DFLTK_BUILD_EXAMPLES=OFF 
		-DOPTION_BUILD_HTML_DOCUMENTATION=OFF 
		-DOPTION_BUILD_PDF_DOCUMENTATION=OFF
		-DOPTION_USE_SYSTEM_ZLIB=OFF
		-DOPTION_USE_WAYLAND=OFF
)
ExternalProject_Get_property(fltk BINARY_DIR)
ExternalProject_Get_property(fltk SOURCE_DIR)
list(APPEND INCLUDE_DIRS ${BINARY_DIR})
list(APPEND INCLUDE_DIRS ${SOURCE_DIR})
if(DEFINED OS_WINDOWS)
	list(APPEND LIBRARIES "${BINARY_DIR}/lib/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/fltk_images$<$<CONFIG:Debug>:d>.lib")
	list(APPEND LIBRARIES "${BINARY_DIR}/lib/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/fltk$<$<CONFIG:Debug>:d>.lib")
	list(APPEND LIBRARIES "${BINARY_DIR}/lib/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/fltk_z$<$<CONFIG:Debug>:d>.lib")
	list(APPEND LIBRARIES "${BINARY_DIR}/lib/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/fltk_gl$<$<CONFIG:Debug>:d>.lib")
	list(APPEND LIBRARIES gdiplus)
elseif (DEFINED OS_MACOS)
	list(APPEND LIBRARIES "${BINARY_DIR}/lib/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/libfltk_images.a")
	list(APPEND LIBRARIES "${BINARY_DIR}/lib/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/libfltk.a")
	list(APPEND LIBRARIES "${BINARY_DIR}/lib/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/libfltk_z.a")
else() # Linux and FreeBSD
	list(APPEND LIBRARIES "${BINARY_DIR}/lib/libfltk_images.a")
	list(APPEND LIBRARIES "${BINARY_DIR}/lib/libfltk.a")
	list(APPEND LIBRARIES "${BINARY_DIR}/lib/libfltk_z.a")
endif()

# Libsndfile

find_package(SndFile CONFIG)
if (SndFile_FOUND)
	list(APPEND LIBRARIES SndFile::sndfile)
	message("Libsndfile library found in " ${SndFile_DIR})
elseif(PkgConfig_FOUND)
	pkg_check_modules(SndFile IMPORTED_TARGET sndfile)
	if (SndFile_FOUND)
		list(APPEND LIBRARIES PkgConfig::SndFile)
		message("Libsndfile library found")
	endif()	
endif()

if (NOT SndFile_FOUND)
	# Fallback to find_library mode (in case libsndfile is too old). 
	find_library(LIBRARY_SNDFILE NAMES sndfile libsndfile libsndfile-1)
	
	if (NOT LIBRARY_SNDFILE)
		message(FATAL_ERROR "Can't find libsndfile, aborting.")
	endif()

	list(APPEND LIBRARIES ${LIBRARY_SNDFILE})
	message("Libsndfile library found in " ${LIBRARY_SNDFILE})

	# Find optional dependencies.

	find_library(LIBRARY_FLAC NAMES flac FLAC)
	find_library(LIBRARY_OGG NAMES ogg)
	find_library(LIBRARY_OPUS NAMES opus libopus)
	find_library(LIBRARY_VORBIS NAMES vorbis)
	find_library(LIBRARY_VORBISENC NAMES vorbisenc)

	if(LIBRARY_FLAC)
		list(APPEND LIBRARIES ${LIBRARY_FLAC})
	endif()
	if(LIBRARY_OGG)
		list(APPEND LIBRARIES ${LIBRARY_OGG})
	endif()
	if(LIBRARY_OPUS)
		list(APPEND LIBRARIES ${LIBRARY_OPUS})
	endif()
	if(LIBRARY_VORBIS)
		list(APPEND LIBRARIES ${LIBRARY_VORBIS})
	endif()
	if(LIBRARY_VORBISENC)
		list(APPEND LIBRARIES ${LIBRARY_VORBISENC})
	endif()	
endif()

# Libsamplerate

find_package(SampleRate CONFIG)
if (SampleRate_FOUND)
	list(APPEND LIBRARIES SampleRate::samplerate)
	message("Libsamplerate library found in " ${SampleRate_DIR})
else() 
	# Fallback to find_library mode (in case Libsamplerate is too old). 
	find_library(LIBRARY_SAMPLERATE 
	    NAMES samplerate libsamplerate libsamplerate-0 libsamplerate0 liblibsamplerate-0
		REQUIRED)
	list(APPEND LIBRARIES ${LIBRARY_SAMPLERATE})
	message("Libsamplerate library found in " ${LIBRARY_SAMPLERATE})
endif()

# fmt

find_package(fmt REQUIRED)
list(APPEND LIBRARIES fmt::fmt)

# nlohmann_json

find_package(nlohmann_json REQUIRED)
list(APPEND LIBRARIES nlohmann_json::nlohmann_json)

# Catch (if tests enabled)

if (WITH_TESTS)

	find_package(Catch2 CONFIG REQUIRED)
	list(APPEND LIBRARIES Catch2::Catch2)
	message("Catch2 library found in " ${Catch2_DIR})

endif()

# ------------------------------------------------------------------------------
# Conditional checks for different platforms.
# ------------------------------------------------------------------------------

if(DEFINED OS_LINUX)

	find_package(X11 REQUIRED)
	find_package(ALSA REQUIRED)
	find_library(LIBRARY_PULSE NAMES pulse REQUIRED)
	find_library(LIBRARY_PULSE_SIMPLE NAMES pulse-simple REQUIRED)
	find_library(LIBRARY_FONTCONFIG NAMES fontconfig REQUIRED)
	pkg_check_modules(JACK REQUIRED jack)
	list(APPEND LIBRARIES
		${X11_LIBRARIES} ${X11_Xrender_LIB} ${X11_Xft_LIB} ${X11_Xfixes_LIB}
		${X11_Xinerama_LIB} ${X11_Xcursor_LIB} ${X11_Xpm_LIB} ${LIBRARY_PULSE}
		${LIBRARY_PULSE_SIMPLE} ${LIBRARY_FONTCONFIG} ${JACK_LDFLAGS}
		${CMAKE_DL_LIBS} ${ALSA_LIBRARIES} pthread stdc++fs)

	if (WITH_ALSA)
		list(APPEND PREPROCESSOR_DEFS __LINUX_ALSA__)
	endif()
	if (WITH_PULSE)
		list(APPEND PREPROCESSOR_DEFS __LINUX_PULSE__)
	endif()
	if (WITH_JACK)
		list(APPEND PREPROCESSOR_DEFS WITH_AUDIO_JACK __UNIX_JACK__)
	endif()

elseif(DEFINED OS_WINDOWS)

	list(APPEND LIBRARIES dsound)

	list(APPEND SOURCES
		src/deps/rtaudio/include/asio.h
		src/deps/rtaudio/include/asio.cpp
		src/deps/rtaudio/include/asiosys.h
		src/deps/rtaudio/include/asiolist.h
		src/deps/rtaudio/include/asiolist.cpp
		src/deps/rtaudio/include/asiodrivers.h
		src/deps/rtaudio/include/asiodrivers.cpp
		src/deps/rtaudio/include/iasiothiscallresolver.h
		src/deps/rtaudio/include/iasiothiscallresolver.cpp
		src/ext/resource.rc)

	list(APPEND INCLUDE_DIRS
		src/deps/rtaudio/include)

	list(APPEND PREPROCESSOR_DEFS
		__WINDOWS_ASIO__
		__WINDOWS_WASAPI__
		__WINDOWS_DS__)

elseif(DEFINED OS_MACOS)

	find_library(CORE_AUDIO_LIBRARY CoreAudio REQUIRED)
	find_library(CORE_MIDI_LIBRARY CoreMIDI REQUIRED)
	find_library(COCOA_LIBRARY Cocoa REQUIRED)
	find_library(CARBON_LIBRARY Carbon REQUIRED)
	find_library(CORE_FOUNDATION_LIBRARY CoreFoundation REQUIRED)
	find_library(ACCELERATE_LIBRARY Accelerate REQUIRED)
	find_library(WEBKIT_LIBRARY WebKit REQUIRED)
	find_library(QUARZ_CORE_LIBRARY QuartzCore REQUIRED)
	find_library(IOKIT_LIBRARY IOKit REQUIRED)
	list(APPEND LIBRARIES
		${CORE_AUDIO_LIBRARY} ${CORE_MIDI_LIBRARY} ${COCOA_LIBRARY}
		${CARBON_LIBRARY} ${CORE_FOUNDATION_LIBRARY} ${ACCELERATE_LIBRARY}
		${WEBKIT_LIBRARY} ${QUARZ_CORE_LIBRARY} ${IOKIT_LIBRARY})

	list(APPEND SOURCES
		src/utils/cocoa.mm
		src/utils/cocoa.h)

	# TODO: why??
	list(APPEND INCLUDE_DIRS
		"/usr/local/include")

	list(APPEND PREPROCESSOR_DEFS 
		__MACOSX_CORE__)

elseif (DEFINED OS_FREEBSD)

	find_package(X11 REQUIRED)
	find_library(LIBRARY_PULSE NAMES pulse REQUIRED)
	find_library(LIBRARY_PULSE_SIMPLE NAMES pulse-simple REQUIRED)
	find_library(LIBRARY_FONTCONFIG NAMES fontconfig REQUIRED)
	find_library(LIBRARY_JACK NAMES jack REQUIRED)
	list(APPEND LIBRARIES
		${X11_LIBRARIES} ${X11_Xrender_LIB} ${X11_Xft_LIB} ${X11_Xfixes_LIB}
		${X11_Xinerama_LIB} ${X11_Xcursor_LIB} ${X11_Xpm_LIB} ${LIBRARY_PULSE}
		${LIBRARY_PULSE_SIMPLE} ${LIBRARY_FONTCONFIG} ${LIBRARY_JACK}
		${CMAKE_DL_LIBS} pthread)
	
	list(APPEND PREPROCESSOR_DEFS
		WITH_AUDIO_JACK
		__LINUX_PULSE__
		__UNIX_JACK__)

endif()

# ------------------------------------------------------------------------------
# Extra parameters for audio plug-ins support.
# ------------------------------------------------------------------------------

add_subdirectory(${CMAKE_SOURCE_DIR}/src/deps/juce)

list(APPEND LIBRARIES 
	juce::juce_audio_utils
	juce::juce_recommended_config_flags)

list(APPEND PREPROCESSOR_DEFS
	JUCE_DEBUG=$<BOOL:$<CONFIG:Debug>>
	JUCE_MODAL_LOOPS_PERMITTED=1
	JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1
	JUCE_STANDALONE_APPLICATION=1
	JUCE_PLUGINHOST_LV2=1
	JUCE_PLUGINHOST_AU=0
	JUCE_WEB_BROWSER=0
	JUCE_USE_CURL=0)

if(WITH_VST2 OR WITH_VST3)

	list(APPEND PREPROCESSOR_DEFS
		WITH_VST)

	if(WITH_VST2)
		if(VST2_SDK_PATH STREQUAL "")
			message(FATAL_ERROR "No VST2_SDK_PATH specified, required by WITH_VST2 option.")
		endif()

		list(APPEND PREPROCESSOR_DEFS
			WITH_VST2
			JUCE_PLUGINHOST_VST=1)
		
		list(APPEND INCLUDE_DIRS
			${VST2_SDK_PATH})
	endif()

	if(WITH_VST3)
		list(APPEND PREPROCESSOR_DEFS
			WITH_VST3
			JUCE_PLUGINHOST_VST3=1)
	endif()

endif()

# ------------------------------------------------------------------------------
# Finalize 'giada' target (main executable).
# ------------------------------------------------------------------------------

add_executable(giada)
add_dependencies(giada fltk) # Wait for fltk to be ready before building Giada
target_compile_features(giada PRIVATE ${COMPILER_FEATURES})
target_sources(giada PRIVATE ${SOURCES})
target_compile_definitions(giada PRIVATE ${PREPROCESSOR_DEFS})
target_include_directories(giada PRIVATE ${INCLUDE_DIRS})
target_link_libraries(giada PRIVATE ${LIBRARIES})
target_compile_options(giada PRIVATE ${COMPILER_OPTIONS})

# ------------------------------------------------------------------------------
# Install rules
# ------------------------------------------------------------------------------

if(DEFINED OS_LINUX)
	include(GNUInstallDirs)
	install(TARGETS giada DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR})
	install(FILES ${CMAKE_SOURCE_DIR}/extras/com.giadamusic.Giada.desktop DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications)
	install(FILES ${CMAKE_SOURCE_DIR}/extras/com.giadamusic.Giada.metainfo.xml DESTINATION ${CMAKE_INSTALL_PREFIX}/share/metainfo)
	install(FILES ${CMAKE_SOURCE_DIR}/extras/giada-logo.svg RENAME com.giadamusic.Giada.svg DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/scalable/apps)
endif()

# ------------------------------------------------------------------------------
# Extra
# ------------------------------------------------------------------------------

# TODO - move these into the 'if [OS]' conditionals (needs smarter list first)

if(DEFINED OS_WINDOWS)

	# Enable static linking of the MSVC runtime library on Windows
	
	set_target_properties(giada PROPERTIES
		MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

elseif(DEFINED OS_MACOS)

	# Enable hardened runtime:
	# https://developer.apple.com/documentation/security/hardened_runtime
	
	set_target_properties(giada PROPERTIES
		XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME YES)

endif()
