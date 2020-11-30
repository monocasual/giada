cmake_minimum_required(VERSION 3.12)

# CMAKE_OSX_DEPLOYMENT_TARGET should be set prior to the first project() or 
# enable_language() command invocation because it may influence configuration 
# of the toolchain and flags. A
# Also, see https://stackoverflow.com/questions/34208360/cmake-seems-to-ignore-cmake-osx-deployment-target

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	set(CMAKE_OSX_DEPLOYMENT_TARGET "10.14" CACHE STRING "Minimum OS X deployment version")
endif()

project(giada LANGUAGES CXX)

add_executable(giada)

# Define SOURCES list, containing the source files.

list(APPEND SOURCES
	src/main.cpp
	src/core/const.h                        
	src/core/queue.h                        
	src/core/ringBuffer.h                   
	src/core/types.h                        
	src/core/range.h                        
	src/core/action.h                       
	src/core/midiDispatcher.h               
	src/core/midiDispatcher.cpp             
	src/core/midiMapConf.h                  
	src/core/midiMapConf.cpp                
	src/core/midiEvent.h                    
	src/core/midiEvent.cpp                  
	src/core/audioBuffer.h                  
	src/core/audioBuffer.cpp                
	src/core/quantizer.h                    
	src/core/quantizer.cpp                  
	src/core/conf.h                         
	src/core/conf.cpp                       
	src/core/kernelAudio.h                  
	src/core/kernelAudio.cpp                                 
	src/core/mixerHandler.h                 
	src/core/mixerHandler.cpp               
	src/core/sequencer.h                    
	src/core/sequencer.cpp                  
	src/core/init.h                         
	src/core/init.cpp                       
	src/core/wave.h                         
	src/core/wave.cpp                       
	src/core/waveFx.h                       
	src/core/waveFx.cpp                     
	src/core/kernelMidi.h                   
	src/core/kernelMidi.cpp                 
	src/core/graphics.h                     
	src/core/graphics.cpp                   
	src/core/patch.h                        
	src/core/patch.cpp                      
	src/core/recorderHandler.h              
	src/core/recorderHandler.cpp            
	src/core/recorder.h                     
	src/core/recorder.cpp                   
	src/core/mixer.h                        
	src/core/mixer.cpp                      
	src/core/clock.h                        
	src/core/clock.cpp                      
	src/core/waveManager.h                  
	src/core/waveManager.cpp                
	src/core/recManager.h                   
	src/core/recManager.cpp
	src/core/midiLearnParam.cpp
	src/core/plugins/pluginHost.h
	src/core/plugins/pluginHost.cpp
	src/core/plugins/pluginManager.h
	src/core/plugins/pluginManager.cpp
	src/core/plugins/plugin.h
	src/core/plugins/plugin.cpp
	src/core/plugins/pluginState.h
	src/core/plugins/pluginState.cpp
	src/core/channels/state.h               
	src/core/channels/state.cpp             
	src/core/channels/sampleActionRecorder.h      
	src/core/channels/sampleActionRecorder.cpp    
	src/core/channels/midiActionRecorder.h      
	src/core/channels/midiActionRecorder.cpp    
	src/core/channels/waveReader.h          
	src/core/channels/waveReader.cpp        
	src/core/channels/midiController.h      
	src/core/channels/midiController.cpp    
	src/core/channels/sampleController.h    
	src/core/channels/sampleController.cpp  
	src/core/channels/samplePlayer.h        
	src/core/channels/samplePlayer.cpp      
	src/core/channels/audioReceiver.h       
	src/core/channels/audioReceiver.cpp     
	src/core/channels/midiLighter.h         
	src/core/channels/midiLighter.cpp       
	src/core/channels/midiLearner.h         
	src/core/channels/midiLearner.cpp       
	src/core/channels/midiSender.h          
	src/core/channels/midiSender.cpp        
	src/core/channels/midiReceiver.h        
	src/core/channels/midiReceiver.cpp      
	src/core/channels/channel.h             
	src/core/channels/channel.cpp           
	src/core/channels/channelManager.h      
	src/core/channels/channelManager.cpp    
	src/core/model/model.h                  
	src/core/model/model.cpp                
	src/core/model/storage.h                
	src/core/model/storage.cpp              
	src/core/idManager.h                    
	src/core/idManager.cpp                  
	src/glue/events.h                       
	src/glue/events.cpp                     
	src/glue/main.h                         
	src/glue/main.cpp                       
	src/glue/io.h                           
	src/glue/io.cpp                         
	src/glue/storage.h                      
	src/glue/storage.cpp                    
	src/glue/channel.h                      
	src/glue/channel.cpp                    
	src/glue/plugin.h                       
	src/glue/plugin.cpp                     
	src/glue/recorder.h                     
	src/glue/recorder.cpp                   
	src/glue/sampleEditor.h                 
	src/glue/sampleEditor.cpp               
	src/glue/actionEditor.h                 
	src/glue/actionEditor.cpp               
	src/gui/dialogs/window.h                
	src/gui/dialogs/window.cpp              
	src/gui/dispatcher.h                    
	src/gui/dispatcher.cpp                  
	src/gui/updater.h                       
	src/gui/updater.cpp                     
	src/gui/model.h                         
	src/gui/model.cpp                       
	src/gui/dialogs/keyGrabber.h            
	src/gui/dialogs/keyGrabber.cpp          
	src/gui/dialogs/about.h                 
	src/gui/dialogs/about.cpp               
	src/gui/dialogs/mainWindow.h            
	src/gui/dialogs/mainWindow.cpp          
	src/gui/dialogs/beatsInput.h            
	src/gui/dialogs/beatsInput.cpp          
	src/gui/dialogs/warnings.h              
	src/gui/dialogs/warnings.cpp            
	src/gui/dialogs/bpmInput.h              
	src/gui/dialogs/bpmInput.cpp            
	src/gui/dialogs/channelNameInput.h      
	src/gui/dialogs/channelNameInput.cpp    
	src/gui/dialogs/config.h                
	src/gui/dialogs/config.cpp              
	src/gui/dialogs/devInfo.h               
	src/gui/dialogs/devInfo.cpp             
	src/gui/dialogs/pluginList.h            
	src/gui/dialogs/pluginList.cpp          
	src/gui/dialogs/pluginWindow.h	        
	src/gui/dialogs/pluginWindow.cpp        
	src/gui/dialogs/sampleEditor.h          
	src/gui/dialogs/sampleEditor.cpp        
	src/gui/dialogs/pluginWindowGUI.h       
	src/gui/dialogs/pluginWindowGUI.cpp     
	src/gui/dialogs/pluginChooser.h         
	src/gui/dialogs/pluginChooser.cpp       
	src/gui/dialogs/actionEditor/baseActionEditor.h     
	src/gui/dialogs/actionEditor/baseActionEditor.cpp   
	src/gui/dialogs/actionEditor/sampleActionEditor.h   
	src/gui/dialogs/actionEditor/sampleActionEditor.cpp 
	src/gui/dialogs/actionEditor/midiActionEditor.h     
	src/gui/dialogs/actionEditor/midiActionEditor.cpp   
	src/gui/dialogs/browser/browserBase.h            
	src/gui/dialogs/browser/browserBase.cpp          
	src/gui/dialogs/browser/browserDir.h             
	src/gui/dialogs/browser/browserDir.cpp           
	src/gui/dialogs/browser/browserLoad.h            
	src/gui/dialogs/browser/browserLoad.cpp          
	src/gui/dialogs/browser/browserSave.h            
	src/gui/dialogs/browser/browserSave.cpp          
	src/gui/dialogs/midiIO/midiOutputBase.h          
	src/gui/dialogs/midiIO/midiOutputBase.cpp        
	src/gui/dialogs/midiIO/midiOutputSampleCh.h      
	src/gui/dialogs/midiIO/midiOutputSampleCh.cpp    
	src/gui/dialogs/midiIO/midiOutputMidiCh.h        
	src/gui/dialogs/midiIO/midiOutputMidiCh.cpp      
	src/gui/dialogs/midiIO/midiInputBase.h           
	src/gui/dialogs/midiIO/midiInputBase.cpp         
	src/gui/dialogs/midiIO/midiInputChannel.h        
	src/gui/dialogs/midiIO/midiInputChannel.cpp      
	src/gui/dialogs/midiIO/midiInputMaster.h         
	src/gui/dialogs/midiIO/midiInputMaster.cpp       
	src/gui/elems/midiIO/midiLearner.h               
	src/gui/elems/midiIO/midiLearner.cpp             
	src/gui/elems/midiIO/midiLearnerPack.h           
	src/gui/elems/midiIO/midiLearnerPack.cpp         
	src/gui/elems/browser.h		                     
	src/gui/elems/browser.cpp                        
	src/gui/elems/soundMeter.h		                 
	src/gui/elems/soundMeter.cpp                     
	src/gui/elems/plugin/pluginBrowser.h                
	src/gui/elems/plugin/pluginBrowser.cpp              
	src/gui/elems/plugin/pluginParameter.h              
	src/gui/elems/plugin/pluginParameter.cpp            
	src/gui/elems/plugin/pluginElement.h                
	src/gui/elems/plugin/pluginElement.cpp              
	src/gui/elems/sampleEditor/waveform.h               
	src/gui/elems/sampleEditor/waveform.cpp             
	src/gui/elems/sampleEditor/waveTools.h              
	src/gui/elems/sampleEditor/waveTools.cpp            
	src/gui/elems/sampleEditor/volumeTool.h             
	src/gui/elems/sampleEditor/volumeTool.cpp           
	src/gui/elems/sampleEditor/boostTool.h              
	src/gui/elems/sampleEditor/boostTool.cpp            
	src/gui/elems/sampleEditor/panTool.h                
	src/gui/elems/sampleEditor/panTool.cpp              
	src/gui/elems/sampleEditor/pitchTool.h              
	src/gui/elems/sampleEditor/pitchTool.cpp            
	src/gui/elems/sampleEditor/rangeTool.h              
	src/gui/elems/sampleEditor/rangeTool.cpp            
	src/gui/elems/sampleEditor/shiftTool.h              
	src/gui/elems/sampleEditor/shiftTool.cpp            
	src/gui/elems/actionEditor/baseActionEditor.h       
	src/gui/elems/actionEditor/baseActionEditor.cpp     
	src/gui/elems/actionEditor/baseAction.h             
	src/gui/elems/actionEditor/baseAction.cpp           
	src/gui/elems/actionEditor/envelopeEditor.h         
	src/gui/elems/actionEditor/envelopeEditor.cpp       
	src/gui/elems/actionEditor/velocityEditor.h         
	src/gui/elems/actionEditor/velocityEditor.cpp       
	src/gui/elems/actionEditor/envelopePoint.h          
	src/gui/elems/actionEditor/envelopePoint.cpp        
	src/gui/elems/actionEditor/pianoRoll.h              
	src/gui/elems/actionEditor/pianoRoll.cpp            
	src/gui/elems/actionEditor/noteEditor.h		        
	src/gui/elems/actionEditor/noteEditor.cpp           
	src/gui/elems/actionEditor/pianoItem.h              
	src/gui/elems/actionEditor/pianoItem.cpp            
	src/gui/elems/actionEditor/sampleActionEditor.h           
	src/gui/elems/actionEditor/sampleActionEditor.cpp         
	src/gui/elems/actionEditor/sampleAction.h                 
	src/gui/elems/actionEditor/sampleAction.cpp               
	src/gui/elems/actionEditor/gridTool.h               
	src/gui/elems/actionEditor/gridTool.cpp             
	src/gui/elems/mainWindow/mainIO.h          
	src/gui/elems/mainWindow/mainIO.cpp        
	src/gui/elems/mainWindow/mainMenu.h        
	src/gui/elems/mainWindow/mainMenu.cpp      
	src/gui/elems/mainWindow/mainTimer.h       
	src/gui/elems/mainWindow/mainTimer.cpp     
	src/gui/elems/mainWindow/mainTransport.h   
	src/gui/elems/mainWindow/mainTransport.cpp 
	src/gui/elems/mainWindow/beatMeter.h       
	src/gui/elems/mainWindow/beatMeter.cpp     
	src/gui/elems/mainWindow/keyboard/channelMode.h           
	src/gui/elems/mainWindow/keyboard/channelMode.cpp         
	src/gui/elems/mainWindow/keyboard/channelButton.h         
	src/gui/elems/mainWindow/keyboard/channelButton.cpp       
	src/gui/elems/mainWindow/keyboard/channelStatus.h         
	src/gui/elems/mainWindow/keyboard/channelStatus.cpp       
	src/gui/elems/mainWindow/keyboard/keyboard.h              
	src/gui/elems/mainWindow/keyboard/keyboard.cpp            
	src/gui/elems/mainWindow/keyboard/column.h                
	src/gui/elems/mainWindow/keyboard/column.cpp              
	src/gui/elems/mainWindow/keyboard/sampleChannel.h         
	src/gui/elems/mainWindow/keyboard/sampleChannel.cpp       
	src/gui/elems/mainWindow/keyboard/midiChannel.h           
	src/gui/elems/mainWindow/keyboard/midiChannel.cpp         
	src/gui/elems/mainWindow/keyboard/channel.h               
	src/gui/elems/mainWindow/keyboard/channel.cpp             
	src/gui/elems/mainWindow/keyboard/sampleChannelButton.h   
	src/gui/elems/mainWindow/keyboard/sampleChannelButton.cpp 
	src/gui/elems/mainWindow/keyboard/midiChannelButton.h     
	src/gui/elems/mainWindow/keyboard/midiChannelButton.cpp   
	src/gui/elems/config/tabMisc.h         
	src/gui/elems/config/tabMisc.cpp       
	src/gui/elems/config/tabMidi.h         
	src/gui/elems/config/tabMidi.cpp       
	src/gui/elems/config/tabAudio.h        
	src/gui/elems/config/tabAudio.cpp      
	src/gui/elems/config/tabBehaviors.h    
	src/gui/elems/config/tabBehaviors.cpp  
	src/gui/elems/config/tabPlugins.h      
	src/gui/elems/config/tabPlugins.cpp    
	src/gui/elems/basics/scroll.h          
	src/gui/elems/basics/scroll.cpp        
	src/gui/elems/basics/pack.h            
	src/gui/elems/basics/pack.cpp          
	src/gui/elems/basics/group.h           
	src/gui/elems/basics/group.cpp         
	src/gui/elems/basics/scrollPack.h      
	src/gui/elems/basics/scrollPack.cpp    
	src/gui/elems/basics/boxtypes.h        
	src/gui/elems/basics/boxtypes.cpp      
	src/gui/elems/basics/statusButton.h    
	src/gui/elems/basics/statusButton.cpp  
	src/gui/elems/basics/button.h          
	src/gui/elems/basics/button.cpp        
	src/gui/elems/basics/resizerBar.h      
	src/gui/elems/basics/resizerBar.cpp    
	src/gui/elems/basics/input.h           
	src/gui/elems/basics/input.cpp         
	src/gui/elems/basics/liquidScroll.h    
	src/gui/elems/basics/liquidScroll.cpp  
	src/gui/elems/basics/choice.h          
	src/gui/elems/basics/choice.cpp        
	src/gui/elems/basics/dial.h            
	src/gui/elems/basics/dial.cpp          
	src/gui/elems/basics/box.h             
	src/gui/elems/basics/box.cpp           
	src/gui/elems/basics/slider.h          
	src/gui/elems/basics/slider.cpp        
	src/gui/elems/basics/progress.h        
	src/gui/elems/basics/progress.cpp      
	src/gui/elems/basics/check.h           
	src/gui/elems/basics/check.cpp         
	src/gui/elems/basics/radio.h           
	src/gui/elems/basics/radio.cpp         
	src/utils/log.h                        
	src/utils/log.cpp                      
	src/utils/time.h                       
	src/utils/time.cpp                     
	src/utils/math.h                       
	src/utils/math.cpp                     
	src/utils/gui.h                        
	src/utils/gui.cpp                                         
	src/utils/fs.h                         
	src/utils/fs.cpp                       
	src/utils/vector.h                     
	src/utils/ver.h                        
	src/utils/ver.cpp                      
	src/utils/string.h                     
	src/utils/string.cpp                   
	src/deps/rtaudio/RtAudio.h             
	src/deps/rtaudio/RtAudio.cpp)

# Define PREPROCESSOR_DEFS lists, containing preprocessor definitions.

list(APPEND PREPROCESSOR_DEFS)

# Define INCLUDE_DIRS, containing include directories (e.g. -I).

list(APPEND INCLUDE_DIRS 
	${CMAKE_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/src)

# Define COMPILER_OPTIONS lists, additional flags for the compiler.

list(APPEND COMPILER_OPTIONS)

# Define LIBRARIES lists, ie. external dependencies to link.

list(APPEND LIBRARIES)

# Set C++17.

target_compile_features(giada PRIVATE cxx_std_17)
	
# Enable compiler warnings. 

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	list(APPEND COMPILER_OPTIONS /W4)
else()
	list(APPEND COMPILER_OPTIONS -Wall -Wextra -Wpedantic)
endif()

# Set CMake options.

option(WITH_VST2 "Enable VST2 support." OFF)
option(WITH_VST3 "Enable VST3 support." OFF)
option(WITH_TESTS "Include the test suite." OFF)
option(WITH_SYSTEM_CATCH "Use system-wide Catch (not the submodule dep) (only if WITH_TESTS=ON)." OFF)

if(WITH_TESTS)
	list(APPEND PREPROCESSOR_DEFS WITH_TESTS)
	if(NOT WITH_SYSTEM_CATCH)
		list(APPEND INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/tests/catch2/single_include/catch2)
	endif()
endif()

if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
	list(APPEND PREPROCESSOR_DEFS NDEBUG)	
endif()

# Find required dependencies. Some don't support the find_package() command: 
# just fallback to find_library().

set(FLTK_SKIP_FLUID TRUE) # Don't search for FLTK's fluid
set(FLTK_SKIP_OPENGL TRUE) # Don't search for FLTK's OpenGL
find_package(Threads REQUIRED)
find_package(FLTK REQUIRED)
find_library(LIBRARY_RTMIDI NAMES rtmidi REQUIRED)
find_library(LIBRARY_SNDFILE NAMES sndfile libsndfile libsndfile-1 REQUIRED)
find_library(LIBRARY_SAMPLERATE NAMES samplerate libsamplerate libsamplerate-0 REQUIRED)

# Find optional dependencies.

find_library(LIBRARY_FLAC NAMES flac FLAC)
find_library(LIBRARY_OGG NAMES ogg)
find_library(LIBRARY_OPUS NAMES opus libopus)
find_library(LIBRARY_VORBIS NAMES vorbis)
find_library(LIBRARY_VORBISENC NAMES vorbisenc)

list(APPEND LIBRARIES 
	${Threads_LIBRARY} ${FLTK_LIBRARY} ${LIBRARY_RTMIDI} ${LIBRARY_SNDFILE} 
	${LIBRARY_SAMPLERATE})
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

# RtMidi header path may vary accross OSes, so a fix is needed. 
# TODO - Find a way to avoid this additional step

find_path(LIBRARY_RTMIDI_INCLUDE_DIR RtMidi.h PATH_SUFFIXES rtmidi)
list(APPEND INCLUDE_DIRS ${LIBRARY_RTMIDI_INCLUDE_DIR})
 
# Conditional checks for different platforms.

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")

	find_package(X11 REQUIRED)
	find_package(ALSA REQUIRED)
	find_library(LIBRARY_PULSE NAMES pulse REQUIRED)
	find_library(LIBRARY_PULSE_SIMPLE NAMES pulse-simple REQUIRED)
	find_library(LIBRARY_FONTCONFIG NAMES fontconfig REQUIRED)
	find_library(LIBRARY_JACK NAMES jack REQUIRED)
	list(APPEND LIBRARIES 
		${X11_LIBRARIES} ${X11_Xrender_LIB} ${X11_Xft_LIB} ${X11_Xfixes_LIB}
		${X11_Xinerama_LIB} ${X11_Xcursor_LIB} ${X11_Xpm_LIB} ${LIBRARY_PULSE} 
		${LIBRARY_PULSE_SIMPLE} ${LIBRARY_FONTCONFIG} ${LIBRARY_JACK}
		${CMAKE_DL_LIBS} ${ALSA_LIBRARIES} pthread stdc++fs)

	list(APPEND PREPROCESSOR_DEFS
		__LINUX_ALSA__
		__LINUX_PULSE__
		__UNIX_JACK__)

elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")

	# Enable static linking of the MSVC runtime library. 
	
	set_target_properties(giada PROPERTIES 
		MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

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

	list(APPEND PREPROCESSOR_DEFS
		__WINDOWS_ASIO__
		__WINDOWS_WASAPI__
		__WINDOWS_DS__)

	list(APPEND INCLUDE_DIRS
		src/deps/rtaudio/include)

elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")

	list(APPEND COMPILER_OPTIONS -mmacosx-version-min=10.15)

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

	list(APPEND PREPROCESSOR_DEFS 
		__MACOSX_CORE__)
	
	# TODO: why??
	list(APPEND INCLUDE_DIRS
		"/usr/local/include")
	
elseif (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")

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
		__LINUX_PULSE__
		__UNIX_JACK__)

endif()

# Define extra parameters if compiled with VST.

if(WITH_VST2 OR WITH_VST3)

	list(APPEND SOURCES
		src/deps/juce/modules/juce_audio_basics/juce_audio_basics.cpp
		src/deps/juce/modules/juce_audio_processors/juce_audio_processors.cpp
		src/deps/juce/modules/juce_core/juce_core.cpp
		src/deps/juce/modules/juce_data_structures/juce_data_structures.cpp
		src/deps/juce/modules/juce_events/juce_events.cpp
		src/deps/juce/modules/juce_graphics/juce_graphics.cpp
		src/deps/juce/modules/juce_gui_basics/juce_gui_basics.cpp
		src/deps/juce/modules/juce_gui_extra/juce_gui_extra.cpp)
	
	list(APPEND INCLUDE_DIRS
		${CMAKE_SOURCE_DIR}/src/deps/juce/modules 
		${CMAKE_SOURCE_DIR}/src/deps/vst3sdk)
	
	if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
		find_package(Freetype REQUIRED)
		list(APPEND LIBRARIES ${FREETYPE_LIBRARIES})
		list(APPEND INCLUDE_DIRS ${FREETYPE_INCLUDE_DIRS})
	endif()
	
	list(APPEND PREPROCESSOR_DEFS
		WITH_VST
		JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1
		JUCE_MODULE_AVAILABLE_juce_gui_basics=1
		JUCE_STANDALONE_APPLICATION=1		
		JUCE_PLUGINHOST_AU=0
		JUCE_WEB_BROWSER=0
		JUCE_USE_CURL=0)
	
	if(WITH_VST2)
		list(APPEND PREPROCESSOR_DEFS 
			WITH_VST2
			JUCE_PLUGINHOST_VST=1)
	endif()
	if(WITH_VST3)
		list(APPEND PREPROCESSOR_DEFS 
			WITH_VST3
			JUCE_PLUGINHOST_VST3=1)
	endif()

endif()

# Finalize 'giada' target (main executable).

target_sources(giada PRIVATE ${SOURCES})
target_compile_definitions(giada PRIVATE ${PREPROCESSOR_DEFS})
target_include_directories(giada PRIVATE ${INCLUDE_DIRS})
target_link_libraries(giada PRIVATE ${LIBRARIES})
target_compile_options(giada PRIVATE ${COMPILER_OPTIONS})