/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * pluginHost
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2016 Giovanni A. Zuliani | Monocasual
 *
 * This file is part of Giada - Your Hardcore Loopmachine.
 *
 * Giada - Your Hardcore Loopmachine is free software: you can
 * redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * Giada - Your Hardcore Loopmachine is distributed in the hope that it
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Giada - Your Hardcore Loopmachine. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * -------------------------------------------------------------------------- */

#ifdef WITH_VST

#ifndef __PLUGIN_HOST_H__
#define __PLUGIN_HOST_H__


//#include "../deps/juce/AppConfig.h"
// TODO - can we move this stuff to a separate file?
#include "../deps/juce/juce_audio_basics/juce_audio_basics.h"
#include "../deps/juce/juce_audio_processors/juce_audio_processors.h"
#include "../deps/juce/juce_core/juce_core.h"
#include "../deps/juce/juce_data_structures/juce_data_structures.h"
#include "../deps/juce/juce_events/juce_events.h"
#include "../deps/juce/juce_graphics/juce_graphics.h"
#include "../deps/juce/juce_gui_basics/juce_gui_basics.h"
#include "../deps/juce/juce_gui_extra/juce_gui_extra.h"


using std::string;
using std::vector;


class PluginHost
{
private:

  juce::MessageManager *messageManager;

  /* pluginFormat
   * Plugin format manager. */

  juce::VSTPluginFormat pluginFormat;

  /* knownPuginList
   * List of known (i.e. scanned) plugins. */

  juce::KnownPluginList knownPluginList;

  vector<class Plugin*> masterOut;
  vector<class Plugin*> masterIn;

  /* Audio|MidiBuffer
   * Dynamic buffers. */

  juce::AudioBuffer<float> audioBuffer;

  int samplerate;
  int buffersize;

public:

  enum stackType {
		MASTER_OUT,
		MASTER_IN,
		CHANNEL
	};

  struct PluginInfo {
    string uid;
    string name;
    string category;
    string manufacturerName;
    string format;
    bool isInstrument;
  };

  void init(int bufSize, int frequency);

  /* scanDir
   * Parse plugin directory and store list in knownPluginList. */

  int scanDir(const string &path);

  /* (save|load)List
   * (Save|Load) knownPluginList (in|from) an XML file. */

  int saveList(const string &path);
  int loadList(const string &path);

  /* addPlugin
   * Add a new plugin to 'stackType' by unique id or by index in knownPluginList
   * vector. Requires:
   * fid - plugin unique file id (i.e. path to dynamic library)
   * stackType - which stack to add plugin to
   * mutex - Mixer.mutex_plugin
   * freq - current audio frequency
   * bufSize - buffer size
   * ch - if stackType == CHANNEL. */

  Plugin *addPlugin(const string &fid, int stackType, pthread_mutex_t *mutex,
    class Channel *ch=NULL);
  Plugin *addPlugin(int index, int stackType, pthread_mutex_t *mutex,
    class Channel *ch=NULL);

  /* countPlugins
   * Return size of 'stackType'. */

  unsigned countPlugins(int stackType, class Channel *ch=NULL);

  /* countAvailablePlugins
  * Return size of knownPluginList. */

  int countAvailablePlugins();

  /* getAvailablePluginInfo
  * Return the available plugin information (name, type, ...) from
  * knownPluginList at index 'index'. */

  PluginInfo getAvailablePluginInfo(int index);

  /* freeStack
   * free plugin stack of type 'stackType'. */

  void freeStack(int stackType, pthread_mutex_t *mutex, class Channel *ch=NULL);

  /* processStack
   * apply the fx list to the buffer. */

  void processStack(float *buffer, int stackType, class Channel *ch=NULL);

  /* getStack
  * Return a vector <Plugin *> given the stackType. If stackType == CHANNEL
  * a pointer to Channel is also required. */

  vector <Plugin *> *getStack(int stackType, class Channel *ch=NULL);

  /* getPluginByIndex */

  Plugin *getPluginByIndex(int index, int stackType, class Channel *ch=NULL);

  /* getPluginIndex */

  int getPluginIndex(int id, int stackType, class Channel *ch=NULL);

  /* swapPlugin */

  void swapPlugin(unsigned indexA, unsigned indexB, int stackType,
    pthread_mutex_t *mutex, class Channel *ch=NULL);

  /* freePlugin */

  void freePlugin(int id, int stackType, pthread_mutex_t *mutex,
    class Channel *ch=NULL);


  /* runDispatchLoop
   * Wake up plugins' GUI manager for N milliseconds. */

  void runDispatchLoop();

  /* processStackOffline
   * apply the fx list to a longer chunk of data */

  void processStackOffline(float *buffer, int stackType, class Channel *ch, int size);

  /* freeAllStacks
   * Free everything. */

  void freeAllStacks(vector <Channel*> *channels, pthread_mutex_t *mutex);

  /* clonePlugin */

  int clonePlugin(Plugin *src, int stackType, pthread_mutex_t *mutex,
    class Channel *ch);

#if 0

  void processEvents(float *buffer, class Channel *ch);

  /* createVstMidiEvent
   * return a pointer to a new VstMidiEvent structure. */

  VstMidiEvent *createVstMidiEvent(uint32_t msg);

  Plugin *getPluginById(int id, int stackType, class Channel *ch=NULL);
#endif
};
#endif

#endif // #ifdef WITH_VST
