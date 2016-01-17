/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * pluginHost
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

#ifndef __PLUGIN_HOST_
#define __PLUGIN_HOST_

#include <vector>
#include "../utils/utils.h"
#include "../gui/elems/ge_window.h"
#include "plugin_DEPR_.h"
#include "init.h"
#include "const.h"


using std::vector;


class PluginHost
{
private:

	/* VSTs have a different buffer model:
	 *
	 * buffer[0] = channel left
	 * buffer[1] = channel right
	 * buffer[0][....] = all signals from left chan
	 * buffer[1][....] = all signals from right chan */

	float **bufferI;
	float **bufferO;

	/* VST struct containing infos on tempo (bpm, freq, smtpe, ...). */

	VstTimeInfo vstTimeInfo;

public:

	/* stack types. Use them together with getStack() in order to geta
	 * pointer to the right stack. */

	enum stackType {
		MASTER_OUT,
		MASTER_IN,
		CHANNEL
	};

	/* stack of Plugins */

	vector <Plugin *> masterOut;
	vector <Plugin *> masterIn;

	PluginHost();
	~PluginHost();

	int clonePlugin(const Plugin &src, int stackType, class Channel *ch);

	int allocBuffers();

	/* The plugin can ask the host if it supports a given capability,
	 * which is done through the HostCallback() function.
	 *
	 * Why static? This is a callback attached to each plugin in the stack
	 * and C++ callback functions need to be static when declared in class.
	 *
	 * OPCODE LIST:
	 * base version: vstsdk2.4/pluginterfaces/aeffect.h (vst 1.x)
	 * enhanced v. : vstsdk2.4/pluginterfaces/effectx.h (vst 2.x) */

	static VstIntPtr VSTCALLBACK HostCallback(AEffect *effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void *ptr, float opt);
	VstIntPtr gHostCallback(AEffect *effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void *ptr, float opt);

	/* addPlugin
	 * Add a new plugin to the stack. If the operation goes well, returns a pointer
	 * to the newly inserted plugin. */

	Plugin *addPlugin(const char *fname, int stackType, class Channel *ch=NULL);

	void processEvents(float *buffer, class Channel *ch);

	/* processStack
	 * apply the fx list to the buffer. */

	void processStack(float *buffer, int stackType, class Channel *ch=NULL);

	/* processStackOffline
	 * apply the fx list to a longer chunk of data */

	void processStackOffline(float *buffer, int stackType, class Channel *ch, int size);

	/* createVstMidiEvent
	 * return a pointer to a new VstMidiEvent structure. */

	VstMidiEvent *createVstMidiEvent(uint32_t msg);

	vector <Plugin *> *getStack(int stackType, class Channel *ch=NULL);

	Plugin *getPluginById(int id, int stackType, class Channel *ch=NULL);

	Plugin *getPluginByIndex(int index, int stackType, class Channel *ch=NULL);

	int getPluginIndex(int id, int stackType, class Channel *ch=NULL);

	unsigned countPlugins(int stackType, class Channel *ch=NULL);

	void freeStack(int stackType, class Channel *ch=NULL);

	void freeAllStacks();

	void freePlugin(int id, int stackType, class Channel *ch=NULL);

	void swapPlugin(unsigned indexA, unsigned indexB, int stackType, class Channel *ch=NULL);
};
#endif

#endif // #ifdef WITH_VST
