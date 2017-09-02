/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2017 Giovanni A. Zuliani | Monocasual
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


#include <FL/Fl.H>
#include "../utils/log.h"
#include "../utils/time.h"
#include "const.h"
#include "plugin.h"


using std::string;
using namespace giada::u;


int Plugin::idGenerator = 1;


/* -------------------------------------------------------------------------- */


Plugin::Plugin(juce::AudioPluginInstance *plugin, double samplerate,
  int buffersize)
  : ui    (nullptr),
    plugin(plugin),
    id    (idGenerator++),
    bypass(false)
{
  /* Init midiInParams. All values are empty (0x0): they will be filled during
  midi learning process. */

  for (int i=0; i<plugin->getNumParameters(); i++)
    midiInParams.push_back(0x0);
  
  plugin->prepareToPlay(samplerate, buffersize);

  gu_log("[Plugin] plugin initialized and ready\n");
}


/* -------------------------------------------------------------------------- */


Plugin::~Plugin()
{
	closeEditor();
  plugin->suspendProcessing(true);
  plugin->releaseResources();
}


/* -------------------------------------------------------------------------- */


void Plugin::showEditor(void* parent)
{
  ui = plugin->createEditorIfNeeded();
  if (ui == nullptr) {
    gu_log("[Plugin::showEditor] unable to create editor!\n");
    return;
  }

  /* A silly workaround on X: it seems that calling addToDesktop too fast, i.e.
  before the X Window is fully ready screws up the plugin's event dispatcher. */

#ifdef G_OS_LINUX
  time::sleep(500);
#endif

  ui->setOpaque(true);
  ui->addToDesktop(0, parent);
}


/* -------------------------------------------------------------------------- */


bool Plugin::isEditorOpen()
{
  return ui != nullptr && ui->isVisible() && ui->isOnDesktop();
}


/* -------------------------------------------------------------------------- */


string Plugin::getUniqueId()
{
  return plugin->getPluginDescription().fileOrIdentifier.toStdString();
}


/* -------------------------------------------------------------------------- */


int Plugin::getNumParameters()
{
  return plugin->getNumParameters();
}


/* -------------------------------------------------------------------------- */


float Plugin::getParameter(int paramIndex)
{
  return plugin->getParameter(paramIndex);
}


/* -------------------------------------------------------------------------- */


void Plugin::setParameter(int paramIndex, float value)
{
  return plugin->setParameter(paramIndex, value);
}


/* -------------------------------------------------------------------------- */


void Plugin::prepareToPlay(double samplerate, int buffersize)
{
  plugin->prepareToPlay(samplerate, buffersize);
}


/* -------------------------------------------------------------------------- */


string Plugin::getName()
{
  return plugin->getName().toStdString();
}


/* -------------------------------------------------------------------------- */


bool Plugin::isSuspended()
{
  return plugin->isSuspended();
}


/* -------------------------------------------------------------------------- */


void Plugin::process(juce::AudioBuffer<float> &b, juce::MidiBuffer &m)
{
  plugin->processBlock(b, m);
}


/* -------------------------------------------------------------------------- */


int Plugin::getNumPrograms()
{
  return plugin->getNumPrograms();
}


/* -------------------------------------------------------------------------- */


int Plugin::getCurrentProgram()
{
  return plugin->getCurrentProgram();
}


/* -------------------------------------------------------------------------- */


void Plugin::setCurrentProgram(int index)
{
  plugin->setCurrentProgram(index);
}


/* -------------------------------------------------------------------------- */


bool Plugin::hasEditor()
{
  return plugin->hasEditor();
}


/* -------------------------------------------------------------------------- */


string Plugin::getProgramName(int index)
{
  return plugin->getProgramName(index).toStdString();
}


/* -------------------------------------------------------------------------- */


string Plugin::getParameterName(int index)
{
  return plugin->getParameterName(index).toStdString();
}


/* -------------------------------------------------------------------------- */


string Plugin::getParameterText(int index)
{
  return plugin->getParameterText(index).toStdString();
}


/* -------------------------------------------------------------------------- */


string Plugin::getParameterLabel(int index)
{
  return plugin->getParameterLabel(index).toStdString();
}


/* -------------------------------------------------------------------------- */


void Plugin::closeEditor()
{
  if (ui == nullptr)
    return;
  delete ui;
  ui = nullptr;
}

#endif
