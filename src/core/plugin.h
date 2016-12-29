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


#include "../deps/juce-config.h"


class Plugin
{
private:

  static int idGenerator;

  juce::AudioProcessorEditor *ui;     // gui
  juce::AudioPluginInstance  *plugin; // core

  int    id;
  bool   bypass;
  bool   status;

public:

  Plugin(juce::AudioPluginInstance *p, double samplerate, int buffersize);
  ~Plugin();

  void showEditor(void *parent);

  /* closeEditor
   * Shut down plugin GUI. */

  void closeEditor();

  /* isEditorOpen */

  bool isEditorOpen();

  /* getUniqueId
   * Return a string-based UID. */

  std::string getUniqueId();

  std::string getName();

  bool  hasEditor();
  int   getNumParameters();
  float getParameter(int index);
  void  setParameter(int index, float value);
  std::string getParameterName(int index);
  std::string getParameterText(int index);
  std::string getParameterLabel(int index);
  void  prepareToPlay(double samplerate, int buffersize);
  bool  isSuspended();
  void  process(juce::AudioBuffer<float> &b, juce::MidiBuffer &m);
  int   getNumPrograms();
  int   getCurrentProgram();
  void  setCurrentProgram(int index);
  std::string getProgramName(int index);

  int  getId()           { return id; }
  bool getStatus()       { return status; }
  bool isBypassed()      { return bypass; }
  int  getEditorW()      { return ui->getWidth(); }
  int  getEditorH()      { return ui->getHeight(); }
  void toggleBypass()    { bypass = !bypass; }
  void setStatus(int s)  { status = s; }
  void setBypass(bool b) { bypass = b; }

  /* midiInParams
  A list of midiIn hex values for parameter automation. */

  std::vector<int> midiInParams;
};

#endif

#endif // #ifdef WITH_VST
