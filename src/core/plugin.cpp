/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2018 Giovanni A. Zuliani | Monocasual
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


Plugin::Plugin(juce::AudioPluginInstance* plugin, double samplerate,
	int buffersize)
	: ui    (nullptr),
		plugin(plugin),
		id    (idGenerator++),
		bypass(false)
{
	using namespace juce;

	/* Init midiInParams. All values are empty (0x0): they will be filled during
	midi learning process. */

	const OwnedArray<AudioProcessorParameter>& params = plugin->getParameters();
	for (int i=0; i<params.size(); i++)
		midiInParams.push_back(0x0);
	
	plugin->prepareToPlay(samplerate, buffersize);

	gu_log("[Plugin] plugin initialized and ready. MIDI input params: %lu\n",
		midiInParams.size());
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

#if defined(G_OS_LINUX) || defined(G_OS_FREEBSD)
	time::sleep(500);
#endif

	ui->setOpaque(true);
	ui->addToDesktop(0, parent);
}


/* -------------------------------------------------------------------------- */


bool Plugin::isEditorOpen() const
{
	return ui != nullptr && ui->isVisible() && ui->isOnDesktop();
}


/* -------------------------------------------------------------------------- */


string Plugin::getUniqueId() const
{
	return plugin->getPluginDescription().createIdentifierString().toStdString();
}


/* -------------------------------------------------------------------------- */


int Plugin::getNumParameters() const
{
	return plugin->getParameters().size();
}


/* -------------------------------------------------------------------------- */


float Plugin::getParameter(int paramIndex) const
{
	return plugin->getParameters()[paramIndex]->getValue();
}


/* -------------------------------------------------------------------------- */


void Plugin::setParameter(int paramIndex, float value) const
{
	plugin->getParameters()[paramIndex]->setValue(value);
}


/* -------------------------------------------------------------------------- */


void Plugin::prepareToPlay(double samplerate, int buffersize) const
{
	plugin->prepareToPlay(samplerate, buffersize);
}


/* -------------------------------------------------------------------------- */


string Plugin::getName() const
{
	return plugin->getName().toStdString();
}


/* -------------------------------------------------------------------------- */


bool Plugin::isSuspended() const
{
	return plugin->isSuspended();
}


/* -------------------------------------------------------------------------- */


bool Plugin::acceptsMidi() const
{
	return plugin->acceptsMidi();
}


/* -------------------------------------------------------------------------- */


bool Plugin::isBypassed() const { return bypass; }
void Plugin::toggleBypass() { bypass = !bypass; }
void Plugin::setBypass(bool b) { bypass = b; }


/* -------------------------------------------------------------------------- */


int Plugin::getId() const { return id; }


/* -------------------------------------------------------------------------- */


int Plugin::getEditorW() const { return ui->getWidth(); }
int Plugin::getEditorH() const { return ui->getHeight(); }


/* -------------------------------------------------------------------------- */


void Plugin::process(juce::AudioBuffer<float>& b, juce::MidiBuffer m) const
{
	plugin->processBlock(b, m);
}


/* -------------------------------------------------------------------------- */


int Plugin::getNumPrograms() const
{
	return plugin->getNumPrograms();
}


/* -------------------------------------------------------------------------- */


int Plugin::getCurrentProgram() const
{
	return plugin->getCurrentProgram();
}


/* -------------------------------------------------------------------------- */


void Plugin::setCurrentProgram(int index) const
{
	plugin->setCurrentProgram(index);
}


/* -------------------------------------------------------------------------- */


bool Plugin::hasEditor() const
{
	return plugin->hasEditor();
}


/* -------------------------------------------------------------------------- */


string Plugin::getProgramName(int index) const
{
	return plugin->getProgramName(index).toStdString();
}


/* -------------------------------------------------------------------------- */


string Plugin::getParameterName(int index) const
{
	return plugin->getParameters()[index]->getName(MAX_LABEL_SIZE).toStdString();
}


/* -------------------------------------------------------------------------- */


string Plugin::getParameterText(int index) const
{
	return plugin->getParameters()[index]->getText(index, MAX_LABEL_SIZE).toStdString();
}


/* -------------------------------------------------------------------------- */


string Plugin::getParameterLabel(int index) const
{
	return plugin->getParameters()[index]->getLabel().toStdString();
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
