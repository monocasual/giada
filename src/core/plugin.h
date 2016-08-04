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


#include "../deps/juce/config.h"


using std::string;


class Plugin : public juce::AudioPluginInstance
{
private:

  juce::AudioProcessorEditor *ui;     // gui

  static int idGenerator;
  int    id;
  bool   bypass;
  bool   status;

public:

  void init();

  void showEditor(void *parent);

  /* closeEditor
   * Shut down plugin GUI. */

  void closeEditor();

  /* isEditorOpen */

  bool isEditorOpen();

  /* getUniqueId
   * Return a string-based UID. */

  string getUniqueId();

  int  getId()           { return id; }
  bool getStatus()       { return status; }
  bool isBypassed()      { return bypass; }
  int  getEditorW()      { return ui->getWidth(); }
  int  getEditorH()      { return ui->getHeight(); }
  void toggleBypass()    { bypass = !bypass; }
  void setStatus(int s)  { status = s; }
  void setBypass(bool b) { bypass = b; }
};

#endif

#endif // #ifdef WITH_VST
