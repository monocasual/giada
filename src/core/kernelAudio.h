/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2023 Giovanni A. Zuliani | Monocasual Laboratories
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

#ifndef G_KERNELAUDIO_H
#define G_KERNELAUDIO_H

#include "core/model/model.h"
#include "core/weakAtomic.h"
#include "deps/rtaudio/RtAudio.h"
#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#ifdef WITH_AUDIO_JACK
#include "core/jackTransport.h"
#endif

namespace mcl
{
class AudioBuffer;
}

namespace giada::m::model
{
struct KernelAudio;
}

namespace giada::m
{
class KernelAudio final
{
public:
	struct Device
	{
		size_t                    index             = 0;
		bool                      probed            = false;
		std::string               name              = "";
		int                       maxOutputChannels = 0;
		int                       maxInputChannels  = 0;
		int                       maxDuplexChannels = 0;
		bool                      isDefaultOut      = false;
		bool                      isDefaultIn       = false;
		int                       channelsCount     = 0;
		int                       channelsStart     = 0;
		std::vector<unsigned int> sampleRates       = {};
	};

	struct StreamInfo
	{
		int          deviceOut        = G_DEFAULT_SOUNDDEV_OUT;
		int          channelsOutCount = 0;
		int          channelsOutStart = 0;
		int          deviceIn         = G_DEFAULT_SOUNDDEV_IN;
		int          channelsInCount  = 0;
		int          channelsInStart  = 0;
		unsigned int sampleRate       = G_DEFAULT_SAMPLERATE;
		unsigned int bufferSize       = G_DEFAULT_BUFSIZE;
	};

	KernelAudio(model::Model&);

	static void logCompiledAPIs();

	/* init
	Initializes the RtAudio object. Closes any existing and	running streams. */

	void init(RtAudio::Api api);

	bool openStream(const StreamInfo&);
	bool startStream();
	bool stopStream();
	void shutdown();

	bool                isReady() const;
	bool                isInputEnabled() const;
	bool                isLimitOutput() const;
	float               getRecTriggerLevel() const;
	Resampler::Quality  getResamplerQuality() const;
	unsigned int        getBufferSize() const;
	int                 getSampleRate() const;
	int                 getChannelsOutCount() const;
	int                 getChannelsInCount() const;
	bool                hasAPI(int API) const;
	RtAudio::Api        getAPI() const;
	std::vector<Device> getAvailableDevices() const;
	Device              getCurrentOutDevice() const;
	Device              getCurrentInDevice() const;
#ifdef WITH_AUDIO_JACK
	jack_client_t* getJackHandle() const;
#endif

	/* onAudioCallback
	Main callback invoked on each audio block. */

	std::function<int(mcl::AudioBuffer& out, const mcl::AudioBuffer& in)> onAudioCallback;

private:
	struct CallbackInfo
	{
		KernelAudio* kernelAudio      = nullptr;
		int          channelsOutCount = 0;
		int          channelsInCount  = 0;
	};

	static int audioCallback(void*, void*, unsigned, double, RtAudioStreamStatus, void*);

	Device fetchDevice(size_t deviceIndex) const;
	void   printDevices(const std::vector<Device>& devices) const;

#ifdef WITH_AUDIO_JACK
	JackTransport m_jackTransport;
#endif
	std::unique_ptr<RtAudio> m_rtAudio;
	CallbackInfo             m_callbackInfo;
	model::Model&            m_model;
};
} // namespace giada::m

#endif
