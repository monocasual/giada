/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2020 Giovanni A. Zuliani | Monocasual
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
#include "utils/log.h"
#include "utils/time.h"
#include "core/const.h"
#include "core/plugins/pluginManager.h"
#include "plugin.h"


using std::string;


namespace giada {
namespace m 
{
Plugin::Plugin(ID id, const std::string& UID)
: id            (id)
, valid         (false)
, onEditorResize(nullptr)
, m_plugin      (nullptr)
, m_UID         (UID)
{
}


/* -------------------------------------------------------------------------- */


Plugin::Plugin(ID id, std::unique_ptr<juce::AudioPluginInstance> plugin, double samplerate,
	int buffersize)
: id            (id)
, valid         (true)
, onEditorResize(nullptr)
, m_plugin      (std::move(plugin))
, m_bypass      (false)
{
	/* Initialize midiInParams vector, where midiInParams.size == number of 
	plugin parameters. All values are initially empty (0x0): they will be filled
	during MIDI learning process. */

	midiInParams = std::vector<uint32_t>(m_plugin->getParameters().size());

	m_buffer.setSize(G_MAX_IO_CHANS, buffersize);

	/* Try to set the main bus to the current number of channels. In the future
	this setup will be performed manually through a proper channel matrix. */

	juce::AudioProcessor::Bus* outBus = getMainBus(BusType::OUT);
	juce::AudioProcessor::Bus* inBus  = getMainBus(BusType::IN);
	if (outBus != nullptr) outBus->setNumberOfChannels(G_MAX_IO_CHANS);
	if (inBus != nullptr)  inBus->setNumberOfChannels(G_MAX_IO_CHANS);

	m_plugin->prepareToPlay(samplerate, buffersize);

	u::log::print("[Plugin] plugin initialized and ready. MIDI input params: %lu\n", 
		midiInParams.size());
}


/* -------------------------------------------------------------------------- */


Plugin::Plugin(const Plugin& o)
: id            (o.id)
, midiInParams  (o.midiInParams)
, valid         (o.valid)
, onEditorResize(o.onEditorResize)
, m_plugin      (std::move(pluginManager::makePlugin(o)->m_plugin))
, m_bypass      (o.m_bypass.load())
{
}


/* -------------------------------------------------------------------------- */


Plugin::~Plugin()
{
	if (!valid)
		return;

	juce::AudioProcessorEditor* e = m_plugin->getActiveEditor();
	if (e != nullptr)
		e->removeComponentListener(this);

	m_plugin->suspendProcessing(true);
	m_plugin->releaseResources();
}


/* -------------------------------------------------------------------------- */


void Plugin::componentMovedOrResized(juce::Component& c, bool moved, bool/* resized*/)
{
	if (moved)
		return;
	if (onEditorResize != nullptr)
		onEditorResize(c.getWidth(), c.getHeight());
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


juce::AudioProcessorEditor* Plugin::createEditor() const
{
	juce::AudioProcessorEditor* e = m_plugin->createEditorIfNeeded();
	if (e != nullptr)
		e->addComponentListener(const_cast<Plugin*>(this));
	return e;
}


/* -------------------------------------------------------------------------- */


string Plugin::getUniqueId() const
{
	if (!valid)
		return m_UID;
	return m_plugin->getPluginDescription().createIdentifierString().toStdString();
}


/* -------------------------------------------------------------------------- */


int Plugin::getNumParameters() const
{
	return valid ? m_plugin->getParameters().size() : 0;
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
	if (!valid)
		return "** invalid **";
	return m_plugin->getName().toStdString();
}


/* -------------------------------------------------------------------------- */


bool Plugin::isSuspended() const
{
	if (!valid)
		return false;
	return m_plugin->isSuspended();
}


/* -------------------------------------------------------------------------- */


bool Plugin::acceptsMidi() const
{
	if (!valid)
		return false;
	return m_plugin->acceptsMidi();
}


/* -------------------------------------------------------------------------- */


PluginState Plugin::getState() const
{
	juce::MemoryBlock data;
	m_plugin->getStateInformation(data);
	return PluginState(std::move(data));
}


/* -------------------------------------------------------------------------- */


bool Plugin::isBypassed() const { return m_bypass.load(); }
void Plugin::setBypass(bool b) { m_bypass.store(b); }


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


void Plugin::setState(PluginState state)
{
	m_plugin->setStateInformation(state.getData(), state.getSize());
}


/* -------------------------------------------------------------------------- */


int Plugin::getNumPrograms() const
{
	if (!valid)
		return 0;
	return m_plugin->getNumPrograms();
}


/* -------------------------------------------------------------------------- */


int Plugin::getCurrentProgram() const
{
	if (!valid)
		return 0;
	return m_plugin->getCurrentProgram();
}


/* -------------------------------------------------------------------------- */


void Plugin::setCurrentProgram(int index) const
{
	if (valid)
		m_plugin->setCurrentProgram(index);
}


/* -------------------------------------------------------------------------- */


bool Plugin::hasEditor() const
{
	if (!valid)
		return false;
	return m_plugin->hasEditor();
}


/* -------------------------------------------------------------------------- */


string Plugin::getProgramName(int index) const
{
	if (!valid)
		return {};
	return m_plugin->getProgramName(index).toStdString();
}


/* -------------------------------------------------------------------------- */


string Plugin::getParameterName(int index) const
{
	if (!valid)
		return {};
	const int labelSize = 64;
	return m_plugin->getParameters()[index]->getName(labelSize).toStdString();
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

}} // giada::m::


#endif
