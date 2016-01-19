#ifndef __JUCE_APPCONFIG_H__
#define __JUCE_APPCONFIG_H__

#ifdef _WIN32
  #include <sys/types.h>
  #include <sys/time.h>
#endif

#define JUCE_MODULE_AVAILABLE_juce_audio_basics     1
#define JUCE_MODULE_AVAILABLE_juce_audio_processors 1
#define JUCE_MODULE_AVAILABLE_juce_core             1
#define JUCE_MODULE_AVAILABLE_juce_data_structures  1
#define JUCE_MODULE_AVAILABLE_juce_events           1
#define JUCE_MODULE_AVAILABLE_juce_graphics         1
#define JUCE_MODULE_AVAILABLE_juce_gui_basics       1
#define JUCE_MODULE_AVAILABLE_juce_gui_extra        1

#define JUCE_STANDALONE_APPLICATION 1
#define JUCE_PLUGINHOST_VST 1
#define JUCE_PLUGINHOST_VST3 0
#define JUCE_PLUGINHOST_AU 0
/*
#include "juce_audio_basics/juce_audio_basics.h"
#include "juce_audio_processors/juce_audio_processors.h"
#include "juce_core/juce_core.h"
#include "juce_data_structures/juce_data_structures.h"
#include "juce_events/juce_events.h"
#include "juce_graphics/juce_graphics.h"
#include "juce_gui_basics/juce_gui_basics.h"
#include "juce_gui_extra/juce_gui_extra.h"
*/
#endif
