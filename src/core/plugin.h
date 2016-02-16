/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * plugin
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

#ifndef __PLUGIN_H__
#define __PLUGIN_H__


#include "../deps/juce/juce_audio_basics/juce_audio_basics.h"
#include "../deps/juce/juce_audio_processors/juce_audio_processors.h"
#include "../deps/juce/juce_core/juce_core.h"
#include "../deps/juce/juce_data_structures/juce_data_structures.h"
#include "../deps/juce/juce_events/juce_events.h"
#include "../deps/juce/juce_graphics/juce_graphics.h"
#include "../deps/juce/juce_gui_basics/juce_gui_basics.h"
#include "../deps/juce/juce_gui_extra/juce_gui_extra.h"


//class Plugin : public juce::AudioProcessor
class Plugin : public juce::AudioPluginInstance
{
private:

  juce::AudioProcessorEditor *ui;     // gui
  juce::AudioProcessor       *proc;   // core

  static int idGenerator;
  int    id;
  bool   bypass;
  bool   status;

public:

  Plugin();

  /* initEditor
   * Prepare plugin GUI. 'parent' is a void pointer to the parent window that
   * will contain it. */

  void initEditor(void *parent);

  /* closeEditor
   * Shut down plugin GUI. */

  void closeEditor();

  /* isEditorOpen */
  
  bool isEditorOpen();

  inline int  getId() { return id; }
  inline bool getStatus() { return status; }
  inline bool isBypassed() { return bypass; }
  inline int  getEditorW() { return ui->getWidth(); }
  inline int  getEditorH() { return ui->getHeight(); }

  inline void toggleBypass() { bypass = !bypass; }
  inline void setStatus(int s) { status = s; }

};

#endif

#endif // #ifdef WITH_VST
