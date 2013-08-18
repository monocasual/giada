/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * pluginHost
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2013 Giovanni A. Zuliani | Monocasual
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
 * ------------------------------------------------------------------ */


#ifdef WITH_VST


#include "pluginHost.h"
#include "conf.h"
#include "const.h"
#include "mixer.h"
#include "gd_mainWindow.h"
#include "channel.h"


extern Conf          G_Conf;
extern Mixer         G_Mixer;
extern PluginHost    G_PluginHost;
extern unsigned      G_beats;
extern gdMainWindow *mainWin;


PluginHost::PluginHost() {

	/* initially we fill vstTimeInfo with trash. Only when the plugin requests
	 * the opcode we load the right infos from G_Mixer. */

	vstTimeInfo.samplePos          = 0.0;
	vstTimeInfo.sampleRate         = G_Conf.samplerate;
	vstTimeInfo.nanoSeconds        = 0.0;
	vstTimeInfo.ppqPos             = 0.0;
	vstTimeInfo.tempo              = 120.0;
	vstTimeInfo.barStartPos        = 0.0;
	vstTimeInfo.cycleStartPos      = 0.0;
	vstTimeInfo.cycleEndPos        = 0.0;
	vstTimeInfo.timeSigNumerator   = 4;
	vstTimeInfo.timeSigDenominator = 4;
	vstTimeInfo.smpteOffset        = 0;
	vstTimeInfo.smpteFrameRate     = 1;
	vstTimeInfo.samplesToNextClock = 0;
	vstTimeInfo.flags              = 0;
}


/* ------------------------------------------------------------------ */


PluginHost::~PluginHost() {}


/* ------------------------------------------------------------------ */


int PluginHost::allocBuffers() {

	/** FIXME - ERROR CHECKING! */

	/* never, ever use G_Conf.buffersize to alloc these chunks of memory.
	 * If you use JACK, that value would be meaningless. Always refer to
	 * kernelAudio::realBufsize. */

	int bufSize = kernelAudio::realBufsize*sizeof(float);

	bufferI    = (float **) malloc(2 * sizeof(float*));
	bufferI[0] =  (float *) malloc(bufSize);
	bufferI[1] =  (float *) malloc(bufSize);

	bufferO    = (float **) malloc(2 * sizeof(float*));
	bufferO[0] =  (float *) malloc(bufSize);
	bufferO[1] =  (float *) malloc(bufSize);

	memset(bufferI[0], 0, bufSize);
	memset(bufferI[1], 0, bufSize);
	memset(bufferO[0], 0, bufSize);
	memset(bufferO[1], 0, bufSize);

	printf("[pluginHost] buffers allocated, buffersize = %d\n", 2*kernelAudio::realBufsize);

	//printOpcodes();

	return 1;
}


/* ------------------------------------------------------------------ */


VstIntPtr VSTCALLBACK PluginHost::HostCallback(AEffect *effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void *ptr, float opt) {
	return G_PluginHost.gHostCallback(effect, opcode, index, value, ptr, opt);
}


/* ------------------------------------------------------------------ */


VstIntPtr PluginHost::gHostCallback(AEffect *effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void *ptr, float opt) {

	/* warning: VST headers compiled with DECLARE_VST_DEPRECATED. */

	switch (opcode) {

		/* 0 - Called after a control has changed in the editor and when
		 * the associated parameter should be automated. Index contains the
		 * param, opt the value. Thanks, but we don't need it now. It will
		 * be useful when recording actions from VST (in the future). */

		case audioMasterAutomate:
			return 0;

		/* 1 - host version (2.4) */

		case audioMasterVersion:
			return kVstVersion;

		/* 3 - Give idle time to Host application, e.g. if plug-in editor is
		 * doing mouse tracking in a modal loop. This a is multithread app,
		 * we don't need it. */

		case audioMasterIdle:
			return 0;

		/* 6 - tells the host that the plugin is an instrument. Deprecated. */

		case DECLARE_VST_DEPRECATED(audioMasterWantMidi):
			return 0;

		/* 7 - time infos */

		case audioMasterGetTime:
			vstTimeInfo.samplePos          = G_Mixer.actualFrame;
			vstTimeInfo.sampleRate         = G_Conf.samplerate;
			vstTimeInfo.tempo              = G_Mixer.bpm;
			vstTimeInfo.timeSigNumerator   = G_Mixer.beats;
			vstTimeInfo.timeSigDenominator = G_Mixer.bars;
			vstTimeInfo.ppqPos             = (G_Mixer.actualFrame / (float) G_Conf.samplerate) * (float) G_Mixer.bpm / 60.0f;
			return (VstIntPtr) &vstTimeInfo;

		/* ? - requires a pointer to VstEvents. No vstEvents so far (v0.5.4) */

		case audioMasterProcessEvents:
			return 0;

		/* 13 - tells that numInputs/numOutputs are changed. Not supported and
		 * not needed. */

		case audioMasterIOChanged:
			return false;

		/* 14 - plugin needs idle calls (outside its editor window). Deprecated */

		case DECLARE_VST_DEPRECATED(audioMasterNeedIdle):
			return 0;

		/* 15 - requests to resize the editor window. w = index, h = value*/

		case audioMasterSizeWindow: {
			gWindow *window = NULL;
			for (unsigned i=0; i<masterOut.size && !window; i++)
				if (masterOut.at(i)->getPlugin() == effect)
					window = masterOut.at(i)->window;

			for (unsigned i=0; i<masterIn.size && !window; i++)
				if (masterIn.at(i)->getPlugin() == effect)
					window = masterIn.at(i)->window;

			for (unsigned i=0; i<G_Mixer.channels.size && !window; i++) {
				Channel *ch = G_Mixer.channels.at(i);
				for (unsigned j=0; j<ch->plugins.size && !window; j++)
					if (ch->plugins.at(j)->getPlugin() == effect)
						window = ch->plugins.at(j)->window;
			}

			if (window) {
				printf("[pluginHost] audioMasterSizeWindow: resizing window from plugin %p\n", (void*) effect);
				if (index == 1 || value == 1)
					puts("[pluginHost] warning: non-sense values!");
				else
					window->size((int)index, (int)value);
				return 1;
			}
			else {
				printf("[pluginHost] audioMasterSizeWindow: window from plugin %p not found\n", (void*) effect);
				return 0;
			}
		}

		/* 16 - sample rate */

		case audioMasterGetSampleRate:
			return G_Conf.samplerate;

		/* ?? - buffer size */

		case audioMasterGetBlockSize:
			return kernelAudio::realBufsize;

		case audioMasterGetInputLatency:
			printf("[pluginHost] requested opcode 'audioMasterGetInputLatency' (%d)\n", opcode);
			return 0;

		case audioMasterGetOutputLatency:
			printf("[pluginHost] requested opcode 'audioMasterGetOutputLatency' (%d)\n", opcode);
			return 0;

		/* 23 - wants to know what kind of process is that.
		 * kVstProcessLevelRealtime = currently in audio thread (where
		 * process is called). */

		case audioMasterGetCurrentProcessLevel:
			return kVstProcessLevelRealtime;

		/* 32 - vendor name */

		case audioMasterGetVendorString:
			strcpy((char*)ptr, "Monocasual");
			return 1;

		/* 32 - product name */

		case audioMasterGetProductString:
			strcpy((char*)ptr, "Giada");
			return 1;

		/* 33 - product version */

		case audioMasterGetVendorVersion:
			return (int) VERSIONE_FLOAT * 100;


		/* 37 - Plugin asks Host if it implements the feature text. */

		case audioMasterCanDo:
			printf("[pluginHost] audioMasterCanDo: %s\n", (char*)ptr);
			if (!strcmp((char*)ptr, "sizeWindow")       ||
					!strcmp((char*)ptr, "sendVstTimeInfo")  ||
					!strcmp((char*)ptr, "sendVstMidiEvent") ||
					!strcmp((char*)ptr, "sendVstMidiEventFlagIsRealtime"))
				return 1; // we can do all of that
			else
				return 0;

		/* 42 - Something has changed, update the host's 'multi-fx' display.
		 * Not supported right now, return 0. This opcode deals with the program
		 * changes, more infos http://www.asseca.com/vst-24-specs/amUpdateDisplay.html */

		case audioMasterUpdateDisplay:
			return 0;

		case audioMasterGetLanguage:
			return kVstLangEnglish;

		/* ?? */

		case audioMasterGetAutomationState:
			printf("[pluginHost] requested opcode 'audioMasterGetAutomationState' (%d)\n", opcode);
			return 0;

		/* 43 - It tells the Host that if it needs to, it has to record
		 * automation data for this control. In other words this opcode is fired
		 * when the user start to tweak a parameter with the mouse.
		 * Useful when the plugin actions will be recorded. */

		case audioMasterBeginEdit:
			return 0;

		/* 44 - no more interaction for the user, started with the previous
		 * opcode. */

		case audioMasterEndEdit:
			return 0;

		default:
			printf("[pluginHost] FIXME: host callback called with opcode %d\n", opcode);
			return 0;
	}
}


/* ------------------------------------------------------------------ */


int PluginHost::addPlugin(const char *fname, int stackType, Channel *ch) {

	Plugin *p    = new Plugin();
	bool success = true;

	gVector <Plugin *> *pStack;
	pStack = getStack(stackType, ch);

	if (!p->load(fname)) {
		//delete p;
		//return 0;
		success = false;
	}

	/* if the load failed we add a 'dead' plugin into the stack. This is
	 * useful to report a missing plugin. */

	if (!success) {
		pStack->add(p);
		return 0;
	}

	/* otherwise let's try to initialize it. */

	else {

		/* try to init the plugin. If fails, delete it and return error. */

		if (!p->init(&PluginHost::HostCallback)) {
			delete p;
			return 0;
		}

		/* plugin setup */

		p->setup(G_Conf.samplerate, kernelAudio::realBufsize);

		/* try to add the new plugin until succeed */

		int lockStatus;
		while (true) {
			lockStatus = pthread_mutex_trylock(&G_Mixer.mutex_plugins);
			if (lockStatus == 0) {
				pStack->add(p);
				pthread_mutex_unlock(&G_Mixer.mutex_plugins);
				break;
			}
		}

		char name[256]; p->getName(name);
		printf("[pluginHost] plugin id=%d loaded (%s), stack type=%d, stack size=%d\n", p->getId(), name, stackType, pStack->size);

		/* p->resume() is suggested. Who knows... */

		p->resume();

		return 1;
	}
}


/* ------------------------------------------------------------------ */


void PluginHost::processStack(float *buffer, int stackType, Channel *ch) {

	gVector <Plugin *> *pStack = getStack(stackType, ch);

	/* empty stack, stack not found or mixer not ready: do nothing */

	if (!G_Mixer.ready)
		return;
	if (pStack == NULL)
		return;
	if (pStack->size == 0)
		return;

	/* converting buffer from Giada to VST */

	for (unsigned i=0; i<kernelAudio::realBufsize; i++) {
		bufferI[0][i] = buffer[i*2];
		bufferI[1][i] = buffer[(i*2)+1];
	}

	/* hardcore processing. At the end we swap input and output, so that
	 * the N-th plugin will process the result of the plugin N-1. */

	for (unsigned i=0; i<pStack->size; i++) {
		if (pStack->at(i)->status != 1)
			continue;
		if (pStack->at(i)->suspended)
			continue;
		if (pStack->at(i)->bypass)
			continue;
		if (ch) {   // process events if it's a channel stack
			if (ch->type == CHANNEL_MIDI)
				pStack->at(i)->processEvents(((MidiChannel*)ch)->getVstEvents());
		}
		pStack->at(i)->processAudio(bufferI, bufferO, kernelAudio::realBufsize);
		bufferI = bufferO;
	}

	/* converting buffer from VST to Giada. A note for the future: if we
	 * overwrite (=) (as we do now) it's SEND, if we add (+) it's INSERT. */

	for (unsigned i=0; i<kernelAudio::realBufsize; i++) {
		buffer[i*2]     = bufferO[0][i];
		buffer[(i*2)+1] = bufferO[1][i];
	}
}


/* ------------------------------------------------------------------ */


void PluginHost::processStackOffline(float *buffer, int stackType, Channel *ch, int size) {

	/* call processStack on the entire size of the buffer. How many cycles?
	 * size / (kernelAudio::realBufsize*2) (ie. internal bufsize) */

	/** FIXME 1 - calling processStack is slow, due to its internal buffer
	 * conversions. We should also call processOffline from VST sdk */

	int index = 0;
	int step  = kernelAudio::realBufsize*2;

	while (index <= size) {
		int left = index+step-size;
		if (left < 0)
			processStack(&buffer[index], stackType, ch);

	/** FIXME 2 - we left out the last part of buffer, because size % step != 0.
	 * we should process the last chunk in a separate buffer, padded with 0 */

		//else
		//	printf("chunk of buffer left, size=%d\n", left);

		index+=step;
	}

}


/* ------------------------------------------------------------------ */


Plugin *PluginHost::getPluginById(int id, int stackType, Channel *ch) {
	gVector <Plugin *> *pStack = getStack(stackType, ch);
	for (unsigned i=0; i<pStack->size; i++) {
		if (pStack->at(i)->getId() == id)
			return pStack->at(i);
	}
	return NULL;
}


/* ------------------------------------------------------------------ */


Plugin *PluginHost::getPluginByIndex(int index, int stackType, Channel *ch) {
	gVector <Plugin *> *pStack = getStack(stackType, ch);
	if (pStack->size == 0)
		return NULL;
	if ((unsigned) index >= pStack->size)
		return NULL;
	return pStack->at(index);
}


/* ------------------------------------------------------------------ */


void PluginHost::freeStack(int stackType, Channel *ch) {

	gVector <Plugin *> *pStack;
	pStack = getStack(stackType, ch);

	if (pStack->size == 0)
		return;

	int lockStatus;
	while (true) {
		lockStatus = pthread_mutex_trylock(&G_Mixer.mutex_plugins);
		if (lockStatus == 0) {
			for (unsigned i=0; i<pStack->size; i++) {
				if (pStack->at(i)->status == 1) {  // only if plugin is ok
					pStack->at(i)->suspend();
					pStack->at(i)->close();
				}
				delete pStack->at(i);
			}
			pStack->clear();
			pthread_mutex_unlock(&G_Mixer.mutex_plugins);
			break;
		}
	}

}


/* ------------------------------------------------------------------ */


void PluginHost::freeAllStacks() {
	freeStack(PluginHost::MASTER_OUT);
	freeStack(PluginHost::MASTER_IN);
	for (unsigned i=0; i<G_Mixer.channels.size; i++)
		freeStack(PluginHost::CHANNEL, G_Mixer.channels.at(i));
}


/* ------------------------------------------------------------------ */


void PluginHost::freePlugin(int id, int stackType, Channel *ch) {

	gVector <Plugin *> *pStack;
	pStack = getStack(stackType, ch);

	/* try to delete the plugin until succeed. G_Mixer has priority. */

	for (unsigned i=0; i<pStack->size; i++)
		if (pStack->at(i)->getId() == id) {

			if (pStack->at(i)->status == 0) { // no frills if plugin is missing
				delete pStack->at(i);
				pStack->del(i);
				return;
			}
			else {
				int lockStatus;
				while (true) {
					lockStatus = pthread_mutex_trylock(&G_Mixer.mutex_plugins);
					if (lockStatus == 0) {
						pStack->at(i)->suspend();
						pStack->at(i)->close();
						delete pStack->at(i);
						pStack->del(i);
						pthread_mutex_unlock(&G_Mixer.mutex_plugins);
						printf("[pluginHost] plugin id=%d removed\n", id);
						return;
					}
					//else
						//puts("[pluginHost] waiting for mutex...");
				}
			}
		}
	printf("[pluginHost] plugin id=%d not found\n", id);
}


/* ------------------------------------------------------------------ */


void PluginHost::swapPlugin(unsigned indexA, unsigned indexB, int stackType, Channel *ch) {

	gVector <Plugin *> *pStack = getStack(stackType, ch);

	int lockStatus;
	while (true) {
		lockStatus = pthread_mutex_trylock(&G_Mixer.mutex_plugins);
		if (lockStatus == 0) {
			pStack->swap(indexA, indexB);
			pthread_mutex_unlock(&G_Mixer.mutex_plugins);
			printf("[pluginHost] plugin at index %d and %d swapped\n", indexA, indexB);
			return;
		}
		//else
			//puts("[pluginHost] waiting for mutex...");
	}
}


/* ------------------------------------------------------------------ */


int PluginHost::getPluginIndex(int id, int stackType, Channel *ch) {

	gVector <Plugin *> *pStack = getStack(stackType, ch);

	for (unsigned i=0; i<pStack->size; i++)
		if (pStack->at(i)->getId() == id)
			return i;
	return -1;
}


/* ------------------------------------------------------------------ */


gVector <Plugin *> *PluginHost::getStack(int stackType, Channel *ch) {
	switch(stackType) {
		case MASTER_OUT:
			return &masterOut;
		case MASTER_IN:
			return &masterIn;
		case CHANNEL:
			return &ch->plugins;
		default:
			return NULL;
	}
}


/* ------------------------------------------------------------------ */


void PluginHost::addVstMidiEvent(VstMidiEvent *e, MidiChannel *ch) {
	if (ch->events.numEvents < MAX_VST_EVENTS) {
		ch->events.events[ch->events.numEvents] = (VstEvent*) e;
		ch->events.numEvents++;
		//printf("[pluginHost] VstMidiEvent added to channel %d, total = %d\n", ch->index, ch->events.numEvents);
	}
	else
		printf("[pluginHost] channel %d VstEvents = %d > MAX_VST_EVENTS, nothing to do\n", ch->index, ch->events.numEvents);
}


/* ------------------------------------------------------------------ */


void PluginHost::addVstMidiEvent(uint32_t msg, MidiChannel *ch) {
	addVstMidiEvent(createVstMidiEvent(msg), ch);
}


/* ------------------------------------------------------------------ */


VstMidiEvent *PluginHost::createVstMidiEvent(uint32_t msg) {

	VstMidiEvent *e = (VstMidiEvent*) malloc(sizeof(VstMidiEvent));

	/* type = two types of events: MIDI event and MIDI system exclusive
	 * (aka sysex, not implemented). */

	e->type     = kVstMidiType;
	e->byteSize = sizeof(VstMidiEvent);

	/* deltaFrames = sample frames related to the current block start
	 * sample position. */
	/** FIXME - use real values */

	e->deltaFrames = 0;

	/* flags = kVstMidiEventIsRealtime means that this event is played
	 * live (not in playback from a sequencer track). This allows the
	 * Plug-In to handle these flagged events with higher priority,
	 * especially when the Plug-In has a big latency */

	e->flags = kVstMidiEventIsRealtime;

	/* midiData = 1 to 3 MIDI bytes; midiData[3] is reserved (zero) */

	/** TODO - use kernelMidi::getB1,2,3() */

	e->midiData[0] = (msg >> 24) & 0xFF;  // note on/off + channel
	e->midiData[1] = (msg >> 16) & 0xFF;  // note number
	e->midiData[2] = (msg >> 8)  & 0xFF;  // velocity
	e->midiData[3] = 0;

	/* noteLength = (in sample frames) of entire note, if available,
	 * else 0 */

	e->noteLength = 0;

	/* noteOffset = offset (in sample frames) into note from note start
	 * if available, else 0 */

	e->noteOffset = 0;

	/* noteOffVelocity =  Note Off Velocity [0, 127]. */

	e->noteOffVelocity = 0;

	return e;
}


/* ------------------------------------------------------------------ */


void PluginHost::freeVstMidiEvents(MidiChannel *ch, bool init) {
	if (ch->events.numEvents == 0 && !init)
		return;
	memset(ch->events.events, 0, sizeof(VstEvent*) * MAX_VST_EVENTS);
	ch->events.numEvents = 0;
	ch->events.reserved  = 0;
	//printf("[PluginHost] Vst Midi Events freed\n");
}


/* ------------------------------------------------------------------ */


unsigned PluginHost::countPlugins(int stackType, Channel *ch) {
	gVector <Plugin *> *pStack = getStack(stackType, ch);
	return pStack->size;
}


#endif // #ifdef WITH_VST
