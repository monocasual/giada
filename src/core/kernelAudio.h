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
		std::vector<unsigned int> sampleRates       = {};
	};

	struct CallbackInfo
	{
		KernelAudio* kernelAudio;
		bool         ready;
		bool         withJack;
		void*        outBuf;
		void*        inBuf;
		int          bufferSize;
		int          sampleRate;
		int          channelsOutCount;
		int          channelsInCount;
	};

	KernelAudio(model::Model&);

	static void logCompiledAPIs();

	/* init
	Initializes the RtAudio object. Closes any existing and	running streams. */

	void init();

	int  openStream();
	bool startStream();
	int  stopStream();
	int  shutdown();

	bool                isReady() const;
	bool                isInputEnabled() const;
	unsigned int        getBufferSize() const;
	int                 getSampleRate() const;
	int                 getChannelsOutCount() const;
	int                 getChannelsInCount() const;
	bool                hasAPI(int API) const;
	RtAudio::Api        getAPI() const;
	std::vector<Device> getAvailableDevices() const;
#ifdef WITH_AUDIO_JACK
	jack_client_t* getJackHandle() const;
#endif

	/* onAudioCallback
	Main callback invoked on each audio block. */

	std::function<int(CallbackInfo)> onAudioCallback;

private:
	static int audioCallback(void*, void*, unsigned, double, RtAudioStreamStatus, void*);

	Device fetchDevice(size_t deviceIndex) const;
	void   printDevices(const std::vector<Device>& devices) const;

#ifdef WITH_AUDIO_JACK
	JackTransport m_jackTransport;
#endif
	std::vector<Device>      m_devices;
	std::unique_ptr<RtAudio> m_rtAudio;
	CallbackInfo             m_callbackInfo;

	model::Model& m_model;
};
} // namespace giada::m

#endif
