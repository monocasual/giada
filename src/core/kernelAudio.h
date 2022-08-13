/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2022 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "core/conf.h"
#include "deps/rtaudio/RtAudio.h"
#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#ifdef WITH_AUDIO_JACK
#include "core/jackTransport.h"
#endif

namespace giada::m
{
class KernelAudio final
{
public:
	struct Device
	{
		size_t           index             = 0;
		bool             probed            = false;
		std::string      name              = "";
		int              maxOutputChannels = 0;
		int              maxInputChannels  = 0;
		int              maxDuplexChannels = 0;
		bool             isDefaultOut      = false;
		bool             isDefaultIn       = false;
		std::vector<int> sampleRates       = {};
	};

	struct CallbackInfo
	{
		KernelAudio* kernelAudio;
		bool         ready;
		bool         withJack;
		void*        outBuf;
		void*        inBuf;
		int          bufferSize;
		int          channelsOutCount;
		int          channelsInCount;
	};

	KernelAudio();

	static void logCompiledAPIs();

	int  openDevice(const Conf::Data& conf);
	void closeDevice();
	int  startStream();
	int  stopStream();

	bool                       isReady() const;
	bool                       isInputEnabled() const;
	int                        getBufferSize() const;
	int                        getSampleRate() const;
	int                        getChannelsOutCount() const;
	int                        getChannelsInCount() const;
	bool                       hasAPI(int API) const;
	RtAudio::Api               getAPI() const;
	Device                     getDevice(const char* name) const;
	const std::vector<Device>& getDevices() const;
#ifdef WITH_AUDIO_JACK
	jack_client_t* getJackHandle() const;
#endif

	/* onAudioCallback
	Main callback invoked on each audio block. */

	std::function<int(CallbackInfo)> onAudioCallback;

private:
	static int audioCallback(void*, void*, unsigned, double, RtAudioStreamStatus, void*);

	Device              fetchDevice(size_t deviceIndex) const;
	std::vector<Device> fetchDevices() const;
	void                printDevices(const std::vector<Device>& devices) const;

#ifdef WITH_AUDIO_JACK
	JackTransport m_jackTransport;
#endif
	std::vector<Device>      m_devices;
	std::unique_ptr<RtAudio> m_rtAudio;
	CallbackInfo             m_callbackInfo;
	bool                     m_ready;
	bool                     m_inputEnabled;
	unsigned                 m_realBufferSize; // Real buffer size from the soundcard
	int                      m_realSampleRate; // Sample rate might differ if JACK in use
	int                      m_channelsOutCount;
	int                      m_channelsInCount;
	RtAudio::Api             m_api;
};
} // namespace giada::m

#endif
