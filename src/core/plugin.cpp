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


#include <cassert>
#include <FL/Fl.H>
#include "../utils/log.h"
#include "../utils/time.h"
#include "const.h"
#include "plugin.h"


using std::string;


namespace giada {
namespace m 
{
int Plugin::m_idGenerator = 1;


/* -------------------------------------------------------------------------- */


Plugin::Plugin(juce::AudioPluginInstance* plugin, double samplerate, int buffersize)
: m_ui    (nullptr),
  m_plugin(plugin),
  m_id    (m_idGenerator++),
  m_bypass(false)
{
	using namespace juce;
	
	/* Init midiInParams. All values are empty (0x0): they will be filled during
	midi learning process. */

	const OwnedArray<AudioProcessorParameter>& params = m_plugin->getParameters();
	for (int i=0; i<params.size(); i++)
		midiInParams.push_back(0x0);
	
	m_buffer.setSize(G_MAX_IO_CHANS, buffersize);

	/* Try to set the main bus to the current number of channels. In the future
	this setup will be performed manually through a proper channel matrix. */

	juce::AudioProcessor::Bus* outBus = getMainBus(BusType::OUT);
	juce::AudioProcessor::Bus* inBus  = getMainBus(BusType::IN);
	if (outBus != nullptr) outBus->setNumberOfChannels(G_MAX_IO_CHANS);
	if (inBus != nullptr)  inBus->setNumberOfChannels(G_MAX_IO_CHANS);

	m_plugin->prepareToPlay(samplerate, buffersize);

	gu_log("[Plugin] plugin initialized and ready. MIDI input params: %lu\n", 
		midiInParams.size());
}


/* -------------------------------------------------------------------------- */


Plugin::~Plugin()
{
	closeEditor();
	m_plugin->suspendProcessing(true);
	m_plugin->releaseResources();
}


/* -------------------------------------------------------------------------- */


juce::AudioProcessor::Bus* Plugin::getMainBus(BusType b) const
{
	const bool isInput = static_cast<bool>(b);
	for (int i=0; i<m_plugin->getBusCount(isInput); i++)
		if (m_plugin->getBus(isInput, i)->isMain())
			return m_plugin->getBus(isInput, i); 
	return nullptr;
}


int Plugin::countMainOutChannels() const
{
	juce::AudioProcessor::Bus* b = getMainBus(BusType::OUT);
	assert(b != nullptr);
	return b->getNumberOfChannels();
}


/* -------------------------------------------------------------------------- */


void Plugin::showEditor(void* parent)
{
	m_ui = m_plugin->createEditorIfNeeded();
	if (m_ui == nullptr) {
		gu_log("[Plugin::showEditor] unable to create editor!\n");
		return;
	}
	m_ui->setOpaque(true);
	m_ui->addToDesktop(0, parent);
}


/* -------------------------------------------------------------------------- */


bool Plugin::isEditorOpen() const
{
	return m_ui != nullptr && m_ui->isVisible() && m_ui->isOnDesktop();
}


/* -------------------------------------------------------------------------- */


string Plugin::getUniqueId() const
{
	return m_plugin->getPluginDescription().createIdentifierString().toStdString();
}


/* -------------------------------------------------------------------------- */


int Plugin::getNumParameters() const
{
	return m_plugin->getParameters().size();
}


/* -------------------------------------------------------------------------- */


float Plugin::getParameter(int paramIndex) const
{
	return m_plugin->getParameters()[paramIndex]->getValue();
}


/* -------------------------------------------------------------------------- */


void Plugin::setParameter(int paramIndex, float value) const
{
	m_plugin->getParameters()[paramIndex]->setValue(value);
}


/* -------------------------------------------------------------------------- */


string Plugin::getName() const
{
	return m_plugin->getName().toStdString();
}


/* -------------------------------------------------------------------------- */


bool Plugin::isSuspended() const
{
	return m_plugin->isSuspended();
}


/* -------------------------------------------------------------------------- */


bool Plugin::acceptsMidi() const
{
	return m_plugin->acceptsMidi();
}


/* -------------------------------------------------------------------------- */


bool Plugin::isBypassed() const { return m_bypass; }
void Plugin::toggleBypass() { m_bypass = !m_bypass; }
void Plugin::setBypass(bool b) { m_bypass = b; }


/* -------------------------------------------------------------------------- */


int Plugin::getId() const { return m_id; }


/* -------------------------------------------------------------------------- */


int Plugin::getEditorW() const { assert(m_ui != nullptr); return m_ui->getWidth(); }
int Plugin::getEditorH() const { assert(m_ui != nullptr); return m_ui->getHeight(); }


/* -------------------------------------------------------------------------- */


void Plugin::process(juce::AudioBuffer<float>& out, juce::MidiBuffer m)
{
	/* If this is not an instrument (i.e. doesn't accept MIDI), copy the 
	incoming buffer data into the temporary one. This way FXes will process
	existing audio data. Conversely, if the plug-in is an instrument, it 
	generates its own audio data inside a clean m_buffer and we can play more 
	than one plug-in instrument in the same stack, driven by the same set of 
	MIDI events. */

	const bool isInstrument = m_plugin->acceptsMidi();

	if (!isInstrument)
		m_buffer = out;
	else
		m_buffer.clear();

	m_plugin->processBlock(m_buffer, m);

	/* The local buffer is now filled. Let's try to fill the 'out' one as well
	by taking into account the bus layout - many plug-ins might have mono output
	and we have a stereo buffer to fill. */

	for (int i=0, j=0; i<out.getNumChannels(); i++) {
		if (isInstrument)
			out.addFrom(i, 0, m_buffer, j, 0, m_buffer.getNumSamples());
		else
			out.copyFrom(i, 0, m_buffer, j, 0, m_buffer.getNumSamples());
		if (i < countMainOutChannels() - 1)
			j++;
	}
}


/* -------------------------------------------------------------------------- */


int Plugin::getNumPrograms() const
{
	return m_plugin->getNumPrograms();
}


/* -------------------------------------------------------------------------- */


int Plugin::getCurrentProgram() const
{
	return m_plugin->getCurrentProgram();
}


/* -------------------------------------------------------------------------- */


void Plugin::setCurrentProgram(int index) const
{
	m_plugin->setCurrentProgram(index);
}


/* -------------------------------------------------------------------------- */


bool Plugin::hasEditor() const
{
	return m_plugin->hasEditor();
}


/* -------------------------------------------------------------------------- */


string Plugin::getProgramName(int index) const
{
	return m_plugin->getProgramName(index).toStdString();
}


/* -------------------------------------------------------------------------- */


string Plugin::getParameterName(int index) const
{
	return m_plugin->getParameters()[index]->getName(MAX_LABEL_SIZE).toStdString();
}


/* -------------------------------------------------------------------------- */


string Plugin::getParameterText(int index) const
{
	return m_plugin->getParameters()[index]->getCurrentValueAsText().toStdString();
}


/* -------------------------------------------------------------------------- */


string Plugin::getParameterLabel(int index) const
{
	return m_plugin->getParameters()[index]->getLabel().toStdString();
}


/* -------------------------------------------------------------------------- */


void Plugin::closeEditor()
{
	delete m_ui;
	m_ui = nullptr;
}

}} // giada::m::


#endif
