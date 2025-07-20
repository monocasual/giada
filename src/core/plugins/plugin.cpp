/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2025 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "src/core/plugins/plugin.h"
#include "src/utils/log.h"
#include "src/utils/time.h"
#include <FL/Fl.H>
#include <cassert>
#include <memory>

#if G_OS_WINDOWS
#undef IN
#undef OUT
#endif

namespace giada::m
{
Plugin::Plugin(ID id, const std::string& UID)
: id(id)
, valid(false)
, onEditorResize(nullptr)
, m_plugin(nullptr)
, m_UID(UID)
, m_hasEditor(false)
{
}

/* -------------------------------------------------------------------------- */

Plugin::Plugin(ID id, std::unique_ptr<juce::AudioPluginInstance> plugin,
    std::unique_ptr<PluginHost::Info> playHead, double samplerate, int buffersize)
: id(id)
, valid(true)
, onEditorResize(nullptr)
, m_plugin(std::move(plugin))
, m_playHead(std::move(playHead))
, m_bypass(false)
, m_hasEditor(m_plugin->hasEditor())
{
	/* (1) Initialize midiInParams vector, where midiInParams.size == number of
	plugin parameters. All values are initially empty (0x0): they will be filled
	during MIDI learning process. */

	for (int i = 0; i < m_plugin->getParameters().size(); i++)
		midiInParams.emplace_back(0x0, i);

	m_buffer.setSize(G_MAX_IO_CHANS, buffersize);

	/* Try to set the main bus to the current number of channels. In the future
	this setup will be performed manually through a proper channel matrix. */

	juce::AudioProcessor::Bus* outBus = getMainBus(BusType::OUT);
	juce::AudioProcessor::Bus* inBus  = getMainBus(BusType::IN);
	if (outBus != nullptr)
		outBus->setNumberOfChannels(G_MAX_IO_CHANS);
	if (inBus != nullptr)
		inBus->setNumberOfChannels(G_MAX_IO_CHANS);

	/* Set pointer to PlayHead, used to pass Giada information (bpm, time, ...)
	to the plug-in. */

	m_plugin->setPlayHead(m_playHead.get());

	m_plugin->prepareToPlay(samplerate, buffersize);

	u::log::print("[Plugin] plugin initialized and ready. MIDI input params: {}\n",
	    midiInParams.size());
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

void Plugin::componentMovedOrResized(juce::Component& c, bool moved, bool /* resized*/)
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
	for (int i = 0; i < m_plugin->getBusCount(isInput); i++)
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

std::string Plugin::getUniqueId() const
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

std::string Plugin::getName() const
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

bool Plugin::isInstrument() const
{
	if (!valid)
		return false;
	return m_plugin->acceptsMidi() && m_plugin->getTotalNumInputChannels() == 0;
}

/* -------------------------------------------------------------------------- */

PluginState Plugin::getState() const
{
	if (!valid)
		return {};
	juce::MemoryBlock data;
	m_plugin->getStateInformation(data);
	return PluginState(std::move(data));
}

/* -------------------------------------------------------------------------- */

bool Plugin::isBypassed() const { return m_bypass.load(); }
void Plugin::setBypass(bool b) { m_bypass.store(b); }

/* -------------------------------------------------------------------------- */

const Plugin::Buffer& Plugin::process(const Plugin::Buffer& out, juce::MidiBuffer m)
{
	/* Copy the incoming buffer data into the temporary one. This way FXes will
	process	existing audio data on the private buffer. This is needed later on
	when merging it back into the incoming buffer. */

	m_buffer = out;
	m_plugin->processBlock(m_buffer, m);
	return m_buffer;
}

/* -------------------------------------------------------------------------- */

void Plugin::setState(PluginState state)
{
	m_plugin->setStateInformation(state.getData(), static_cast<int>(state.getSize()));
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
	return m_hasEditor;
}

/* -------------------------------------------------------------------------- */

std::string Plugin::getProgramName(int index) const
{
	if (!valid)
		return {};
	return m_plugin->getProgramName(index).toStdString();
}

/* -------------------------------------------------------------------------- */

std::string Plugin::getParameterName(int index) const
{
	if (!valid)
		return {};
	const int labelSize = 64;
	return m_plugin->getParameters()[index]->getName(labelSize).toStdString();
}

/* -------------------------------------------------------------------------- */

std::string Plugin::getParameterText(int index) const
{
	return m_plugin->getParameters()[index]->getCurrentValueAsText().toStdString();
}

/* -------------------------------------------------------------------------- */

std::string Plugin::getParameterLabel(int index) const
{
	return m_plugin->getParameters()[index]->getLabel().toStdString();
}
} // namespace giada::m