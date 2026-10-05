/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2026 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "src/core/model/channel.h"
#include "src/gui/const.h"
#include <cassert>
#if G_DEBUG_MODE
#include "src/core/wave.h"
#include "src/utils/string.h"
#include <fmt/core.h>
#endif

namespace giada::m::model
{
bool Channel::Parameters::isPlaying() const
{
	ChannelStatus s = m_playStatus.load();
	return s == ChannelStatus::PLAY || s == ChannelStatus::ENDING;
}

/* -------------------------------------------------------------------------- */

bool Channel::Parameters::isReadingActions() const
{
	const ChannelStatus status = m_recStatus.load();
	const bool          read   = m_readActions.load();
	return read && (status == ChannelStatus::PLAY || status == ChannelStatus::ENDING);
}

/* -------------------------------------------------------------------------- */

bool Channel::Parameters::isActive() const
{
	ChannelStatus s = m_playStatus.load();
	return s == ChannelStatus::PLAY || s == ChannelStatus::WAIT || s == ChannelStatus::ENDING;
}

/* -------------------------------------------------------------------------- */

Frame         Channel::Parameters::getTracker() const { return m_tracker.load(); }
ChannelStatus Channel::Parameters::getPlayStatus() const { return m_playStatus.load(); }
ChannelStatus Channel::Parameters::getRecStatus() const { return m_recStatus.load(); }
bool          Channel::Parameters::getReadActions() const { return m_readActions.load(); }
float         Channel::Parameters::getVolumeInternal() const { return m_volumeInternal.load(); }

/* -------------------------------------------------------------------------- */

void Channel::Parameters::setTracker(Frame f) const { m_tracker.store(f); }
void Channel::Parameters::setPlayStatus(ChannelStatus s) const { m_playStatus.store(s); };
void Channel::Parameters::setRecStatus(ChannelStatus s) const { m_recStatus.store(s); };
void Channel::Parameters::setReadActions(bool v) const { m_readActions.store(v); };
void Channel::Parameters::setVolumeInternal(float v) const { m_volumeInternal.store(v); };

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

Channel::Rendering::Rendering(ChannelType type, int sampleRate, int bufferSize, Resampler::Quality quality)
: m_audioBuffer(bufferSize, G_MAX_IO_CHANS)
{
	if (type == ChannelType::SAMPLE || type == ChannelType::PREVIEW)
	{
		m_quantizer.emplace();
		m_resampler.emplace(quality);
		m_stretcher.emplace(sampleRate);
	}
}

/* -------------------------------------------------------------------------- */

void Channel::Rendering::pushMidiEvent(MidiEvent e) const
{
	m_midiQueue.enqueue(std::move(e));
}

/* -------------------------------------------------------------------------- */

const mcl::AudioBuffer& Channel::Rendering::getAudioBuffer() const { return m_audioBuffer; }

/* -------------------------------------------------------------------------- */

rendering::RenderInfo::Mode Channel::Rendering::getPendingRenderMode() const
{
	return m_pendingRenderMode.exchange(rendering::RenderInfo::Mode::NONE);
}

void Channel::Rendering::setPendingRenderMode(rendering::RenderInfo::Mode m) const
{
	m_pendingRenderMode.store(m);
}

/* -------------------------------------------------------------------------- */

void Channel::Rendering::copyToAudioBufferFrom(
    const mcl::AudioBuffer& src,
    Frame                   frames,
    Frame                   srcStart,
    Frame                   offset) const
{
	assert(offset + frames <= m_audioBuffer.countFrames());
	assert(srcStart + frames <= src.countFrames());

	m_audioBuffer.setAll(src, frames, srcStart, offset);
}

/* -------------------------------------------------------------------------- */

void Channel::Rendering::sumToAudioBufferFrom(
    const mcl::AudioBuffer& src,
    const Pan&              pan,
    float                   gain) const
{
	m_audioBuffer.sumAll(src, pan.get(), gain);
}

/* -------------------------------------------------------------------------- */

Resampler::Result Channel::Rendering::resampleToAudioBufferFrom(
    const mcl::AudioBuffer& src,
    Frame                   srcStart,
    Frame                   srcEnd,
    Frame                   offset,
    float                   ratio) const
{
	assert(m_resampler);
	assert(srcStart <= srcEnd && srcEnd <= src.countFrames());
	assert(offset <= m_audioBuffer.countFrames());

	return m_resampler->process(src, srcStart, srcEnd, m_audioBuffer, offset, ratio);
}

/* -------------------------------------------------------------------------- */

Stretcher::Result Channel::Rendering::stretchToAudioBufferFrom(
    const mcl::AudioBuffer& src,
    Frame                   srcStart,
    Frame                   srcEnd,
    Frame                   offset,
    double                  timeRatio,
    double                  pitchRatio) const
{
	assert(m_stretcher);
	assert(srcStart <= srcEnd && srcEnd <= src.countFrames());
	assert(offset <= m_audioBuffer.countFrames());

	return m_stretcher->process(src, srcStart, srcEnd, m_audioBuffer, offset, timeRatio, pitchRatio);
}

/* -------------------------------------------------------------------------- */

void Channel::Rendering::clearAudioBuffer(Frame offset) const
{
	m_audioBuffer.clear(offset);
}

/* -------------------------------------------------------------------------- */

void Channel::Rendering::notifyLastChunk() const
{
	assert(m_stretcher && m_resampler);
	m_resampler->last();
	m_stretcher->last();
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

Channel::Channel(ChannelType type, ID id)
: id(id)
, type(type)
, volume(G_DEFAULT_VOL)
, pan(G_DEFAULT_PAN)
, armed(false)
, key(0)
, height(G_GUI_UNIT)
, sendToMaster(true)
, m_mute(false)
, m_solo(false)
{
	switch (type)
	{
	case ChannelType::SAMPLE:
	case ChannelType::PREVIEW:
		sampleChannel.emplace();
		break;

	case ChannelType::MIDI:
		midiChannel.emplace();
		break;

	default:
		break;
	}
}

/* -------------------------------------------------------------------------- */

Channel::Channel(const Patch::Channel& p, float samplerateRatio, const SceneArray<Sample>& samples, std::vector<ID> plugins)
: id(p.id)
, type(p.type)
, volume(p.volume)
, pan(p.pan)
, armed(p.armed)
, key(p.key)
, height(p.height)
, plugins(plugins)
, sendToMaster(p.sendToMaster)
, extraOutputs(p.extraOutputs)
, midiInput(p)
, midiLightning(p)
, m_mute(p.mute)
, m_solo(p.solo)
, m_names(p.names)
{
	// shared->readActions.store(p.readActions);
	// shared->recStatus.store(p.readActions ? ChannelStatus::PLAY : ChannelStatus::OFF);

	switch (type)
	{
	case ChannelType::SAMPLE:
	case ChannelType::PREVIEW:
		sampleChannel.emplace(p, samples, samplerateRatio);
		break;

	case ChannelType::MIDI:
		midiChannel.emplace(p);
		break;

	default:
		break;
	}
}

/* -------------------------------------------------------------------------- */

bool Channel::operator==(const Channel& other) const
{
	return id == other.id;
}

/* -------------------------------------------------------------------------- */

bool Channel::isInternal() const
{
	return type == ChannelType::MASTER || type == ChannelType::PREVIEW;
}

bool Channel::isMuted() const
{
	/* Internals can't be muted. */
	return !isInternal() && m_mute;
}

bool Channel::isSoloed() const
{
	if (isInternal())
		return false;
	return m_solo;
}

bool Channel::isAudible(bool mixerHasSolos) const
{
	if (isInternal())
		return true;
	if (isMuted())
		return false;
	return !mixerHasSolos || (mixerHasSolos && isSoloed());
}

bool Channel::canInputRec(Scene scene) const
{
	if (type != ChannelType::SAMPLE)
		return false;

	bool hasWave     = sampleChannel->hasWave(scene);
	bool isProtected = sampleChannel->overdubProtection;
	bool canOverdub  = !hasWave || (hasWave && !isProtected);

	return armed && canOverdub;
}

bool Channel::canActionRec(Scene scene) const
{
	return hasWave(scene) && !sampleChannel->isAnyLoopMode();
}

bool Channel::hasWave(Scene scene) const
{
	return sampleChannel && sampleChannel->hasWave(scene);
}

/* -------------------------------------------------------------------------- */

std::string                    Channel::getName(Scene scene) const { return m_names[scene.getIndex()]; }
const SceneArray<std::string>& Channel::getNames() const { return m_names; };

/* -------------------------------------------------------------------------- */

#if G_DEBUG_MODE
std::string Channel::debug() const
{
	std::string out = fmt::format("ID={} type={}", id.getValue(), u::string::toString(type));

	if (type == ChannelType::SAMPLE || type == ChannelType::PREVIEW)
	{
		out += fmt::format(" mode={}", u::string::toString(sampleChannel->mode));
		out += "\n\twaves:\n";
		for (const Sample& s : sampleChannel->getSamples())
			if (s.wave != nullptr)
				out += fmt::format("\t\tID={} ({}), range=[{}, {})\n", s.wave->id.getValue(), (void*)s.wave, s.range.getA(), s.range.getB());
	}

	return out;
}
#endif

/* -------------------------------------------------------------------------- */

bool Channel::canReceiveAudio() const
{
	return type == ChannelType::SAMPLE && armed && sampleChannel->inputMonitor;
}

/* -------------------------------------------------------------------------- */

bool Channel::canSendMidi() const
{
	return type == ChannelType::MIDI && !isMuted() && midiChannel->outputEnabled;
}

/* -------------------------------------------------------------------------- */

void Channel::setMute(bool v)
{
	m_mute = v;
}

void Channel::setSolo(bool v)
{
	m_solo = v;
}

/* -------------------------------------------------------------------------- */

void Channel::setName(const std::string& name, Scene scene) { m_names[scene.getIndex()] = name; }

/* -------------------------------------------------------------------------- */

void Channel::loadSample(const Sample& s, Scene scene)
{
	assert(sampleChannel);

	sampleChannel->loadSample(s, scene);
	// shared->tracker.store(0);
	// shared->playStatus.store(sampleChannel->hasWaves() ? ChannelStatus::OFF : ChannelStatus::EMPTY);
}

/* -------------------------------------------------------------------------- */

void Channel::setWave(Wave* w, Scene scene, float samplerateRatio)
{
	assert(sampleChannel);

	sampleChannel->setWave(w, scene, samplerateRatio);
}

/* -------------------------------------------------------------------------- */

void Channel::kickIn(Frame f)
{
	// shared->tracker.store(f);
	// shared->playStatus.store(ChannelStatus::PLAY);
}
} // namespace giada::m::model