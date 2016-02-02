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

  string pluginDir;

  vector<class Plugin*> masterOut;
  vector<class Plugin*> masterIn;

public:

  int scanDir();

#if 0
  int clonePlugin(const Plugin &src, int stackType, class Channel *ch);

  Plugin *addPlugin(const char *fname, int stackType, class Channel *ch=NULL);

  void processEvents(float *buffer, class Channel *ch);

  /* processStack
   * apply the fx list to the buffer. */

  void processStack(float *buffer, int stackType, class Channel *ch=NULL);

  /* processStackOffline
   * apply the fx list to a longer chunk of data */

  void processStackOffline(float *buffer, int stackType, class Channel *ch, int size);

  /* createVstMidiEvent
   * return a pointer to a new VstMidiEvent structure. */

  VstMidiEvent *createVstMidiEvent(uint32_t msg);

  vector <Plugin *> *getStack(int stackType, class Channel *ch=NULL);

  Plugin *getPluginById(int id, int stackType, class Channel *ch=NULL);

  Plugin *getPluginByIndex(int index, int stackType, class Channel *ch=NULL);

  int getPluginIndex(int id, int stackType, class Channel *ch=NULL);

  unsigned countPlugins(int stackType, class Channel *ch=NULL);

  void freeStack(int stackType, class Channel *ch=NULL);

  void freeAllStacks();

  void freePlugin(int id, int stackType, class Channel *ch=NULL);

  void swapPlugin(unsigned indexA, unsigned indexB, int stackType, class Channel *ch=NULL);

#endif
};
#endif

#endif // #ifdef WITH_VST
