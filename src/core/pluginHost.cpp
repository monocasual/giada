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
#include "../utils/log.h"
#include "../utils/fs.h"
#include "../utils/string.h"
#include "const.h"
#include "channel.h"
#include "plugin.h"
#include "pluginHost.h"


using std::vector;
using std::string;


namespace giada {
namespace m {
namespace pluginHost
{
namespace
{
juce::MessageManager* messageManager;

/* pluginFormat
 * Plugin format manager. */

juce::VSTPluginFormat pluginFormat;

/* knownPuginList
 * List of known (i.e. scanned) plugins. */

juce::KnownPluginList knownPluginList;

/* unknownPluginList
 * List of unrecognized plugins found in a patch. */

vector<string> unknownPluginList;

vector<Plugin*> masterOut;
vector<Plugin*> masterIn;

/* Audio|MidiBuffer
 * Dynamic buffers. */

juce::AudioBuffer<float> audioBuffer;

int samplerate;
int buffersize;

/* missingPlugins
 * If some plugins from any stack are missing. */

bool missingPlugins;

void splitPluginDescription(const string& descr, vector<string>& out)
{
	// input:  VST-mda-Ambience-18fae2d2-6d646141  string
	// output: [2-------------] [1-----] [0-----]  vector.size() == 3
	
	string chunk = "";
	int count = 2;
	for (int i=descr.length()-1; i >= 0; i--) {
		if (descr[i] == '-' && count != 0) {
			out.push_back(chunk);
			count--;
			chunk = "";
		}
		else
			chunk += descr[i];
	}
	out.push_back(chunk);
}


/* findPluginDescription
Browses the list of known plug-ins until plug-in with id == 'id' is found.
Unfortunately knownPluginList.getTypeForIdentifierString(id) doesn't work for
VSTs: their ID is based on the plug-in file location. E.g.:

	/home/vst/mdaAmbience.so      -> VST-mdaAmbience-18fae2d2-6d646141
	/home/vst-test/mdaAmbience.so -> VST-mdaAmbience-b328b2f6-6d646141

The following function simply drops the first hash code during comparison. */

const juce::PluginDescription* findPluginDescription(const string& id)
{
	vector<string> idParts;
	splitPluginDescription(id, idParts);

	for (const juce::PluginDescription* pd : knownPluginList) {
		vector<string> tmpIdParts;
		splitPluginDescription(pd->createIdentifierString().toStdString(), tmpIdParts);
		if (idParts[0] == tmpIdParts[0] && idParts[2] == tmpIdParts[2])
			return pd;
	}
	return nullptr;
}
}; // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


pthread_mutex_t mutex_midi;


/* -------------------------------------------------------------------------- */


void close()
{
	messageManager->deleteInstance();
	pthread_mutex_destroy(&mutex_midi);
}


/* -------------------------------------------------------------------------- */


void init(int buffersize_, int samplerate_)
{
	messageManager = juce::MessageManager::getInstance();
	audioBuffer.setSize(G_MAX_IO_CHANS, buffersize_);
	samplerate = samplerate_;
	buffersize = buffersize_;
	missingPlugins = false;
	//unknownPluginList.empty();
	loadList(gu_getHomePath() + G_SLASH + "plugins.xml");

	pthread_mutex_init(&mutex_midi, nullptr);

	gu_log("[pluginHost::init] initialized with buffersize=%d, samplerate=%d\n",
	buffersize, samplerate);
}


/* -------------------------------------------------------------------------- */


int scanDirs(const string& dirs, const std::function<void(float)>& cb)
{
	gu_log("[pluginHost::scanDir] requested directories: '%s'\n", dirs.c_str());
	gu_log("[pluginHost::scanDir] current plugins: %d\n", knownPluginList.getNumTypes());

	knownPluginList.clear();   // clear up previous plugins

	vector<string> dirVec;
	gu_split(dirs, ";", &dirVec);

	juce::VSTPluginFormat format;
	juce::FileSearchPath searchPath;
	for (const string& dir : dirVec)
		searchPath.add(juce::File(dir));

	juce::PluginDirectoryScanner scanner(knownPluginList, format, searchPath, 
		true, juce::File()); // true: recursive

	juce::String name;
	while (scanner.scanNextFile(false, name)) {
		gu_log("[pluginHost::scanDir]   scanning '%s'\n", name.toRawUTF8());
		cb(scanner.getProgress());
	}

	gu_log("[pluginHost::scanDir] %d plugin(s) found\n", knownPluginList.getNumTypes());
	return knownPluginList.getNumTypes();
}


/* -------------------------------------------------------------------------- */


int saveList(const string& filepath)
{
	int out = knownPluginList.createXml()->writeToFile(juce::File(filepath), "");
	if (!out)
		gu_log("[pluginHost::saveList] unable to save plugin list to %s\n", filepath.c_str());
	return out;
}


/* -------------------------------------------------------------------------- */


int loadList(const string& filepath)
{
	juce::XmlElement* elem = juce::XmlDocument::parse(juce::File(filepath));
	if (elem) {
		knownPluginList.recreateFromXml(*elem);
		delete elem;
		return 1;
	}
	return 0;
}


/* -------------------------------------------------------------------------- */


Plugin* addPlugin(const string& fid, int stackType, pthread_mutex_t* mutex, 
	Channel* ch)
{
	vector<Plugin*>* pStack = getStack(stackType, ch);

	/* Initialize plugin. The default mode uses getTypeForIdentifierString, 
	falling back to  getTypeForFile (deprecated) for old patches (< 0.14.4). */

	const juce::PluginDescription* pd = findPluginDescription(fid);
	if (pd == nullptr) {
		gu_log("[pluginHost::addPlugin] no plugin found with fid=%s! Trying with "
			"deprecated mode...\n", fid.c_str());
		pd = knownPluginList.getTypeForFile(fid);
		if (pd == nullptr) {
			gu_log("[pluginHost::addPlugin] still nothing to do, returning unknown plugin\n");
			missingPlugins = true;
			unknownPluginList.push_back(fid);
			return nullptr;
		}
	}

	juce::AudioPluginInstance* pi = pluginFormat.createInstanceFromDescription(*pd, samplerate, buffersize);
	if (!pi) {
		gu_log("[pluginHost::addPlugin] unable to create instance with fid=%s!\n", fid.c_str());
		missingPlugins = true;
		return nullptr;
	}
	gu_log("[pluginHost::addPlugin] plugin instance with fid=%s created\n", fid.c_str());

	Plugin* p = new Plugin(pi, samplerate, buffersize);

	/* Try to inject the plugin as soon as possible. */

	while (true) {
		if (pthread_mutex_trylock(mutex) != 0)
			continue;
		pStack->push_back(p);
		pthread_mutex_unlock(mutex);
		break;
	}

	gu_log("[pluginHost::addPlugin] plugin id=%s loaded (%s), stack type=%d, stack size=%d\n",
		fid.c_str(), p->getName().c_str(), stackType, pStack->size());

	return p;
}


/* -------------------------------------------------------------------------- */


Plugin* addPlugin(int index, int stackType, pthread_mutex_t* mutex,
	Channel* ch)
{
	juce::PluginDescription* pd = knownPluginList.getType(index);
	if (pd) {
		gu_log("[pluginHost::addPlugin] plugin found, uid=%s, name=%s...\n",
			pd->createIdentifierString().toRawUTF8(), pd->name.toRawUTF8());
		return addPlugin(pd->createIdentifierString().toStdString(), stackType, mutex, ch);
	}
	gu_log("[pluginHost::addPlugin] no plugins found at index=%d!\n", index);
	return nullptr;
}


/* -------------------------------------------------------------------------- */


vector<Plugin*>* getStack(int stackType, Channel* ch)
{
	switch(stackType) {
		case MASTER_OUT:
			return &masterOut;
		case MASTER_IN:
			return &masterIn;
		case CHANNEL:
			return &ch->plugins;
		default:
			return nullptr;
	}
}


/* -------------------------------------------------------------------------- */


unsigned countPlugins(int stackType, Channel* ch)
{
	vector<Plugin*>* pStack = getStack(stackType, ch);
	return pStack->size();
}


/* -------------------------------------------------------------------------- */


int countAvailablePlugins()
{
	return knownPluginList.getNumTypes();
}


/* -------------------------------------------------------------------------- */


unsigned countUnknownPlugins()
{
	return unknownPluginList.size();
}


/* -------------------------------------------------------------------------- */


pluginHost::PluginInfo getAvailablePluginInfo(int i)
{
	juce::PluginDescription* pd = knownPluginList.getType(i);
	PluginInfo pi;
	pi.uid              = pd->fileOrIdentifier.toStdString();
	pi.name             = pd->name.toStdString();
	pi.category         = pd->category.toStdString();
	pi.manufacturerName = pd->manufacturerName.toStdString();
	pi.format           = pd->pluginFormatName.toStdString();
	pi.isInstrument     = pd->isInstrument;
	return pi;
}


/* -------------------------------------------------------------------------- */


bool hasMissingPlugins()
{
	return missingPlugins;
};


/* -------------------------------------------------------------------------- */


string getUnknownPluginInfo(int i)
{
	return unknownPluginList.at(i);
}


/* -------------------------------------------------------------------------- */


void freeStack(int stackType, pthread_mutex_t* mutex, Channel* ch)
{
	vector<Plugin*>* pStack = getStack(stackType, ch);

	if (pStack->size() == 0)
		return;

	while (true) {
		if (pthread_mutex_trylock(mutex) != 0)
			continue;
		for (unsigned i=0; i<pStack->size(); i++)
			delete pStack->at(i);
		pStack->clear();
		pthread_mutex_unlock(mutex);
		break;
	}
	gu_log("[pluginHost::freeStack] stack type=%d freed\n", stackType);
}


/* -------------------------------------------------------------------------- */


void processStack(AudioBuffer& outBuf, int stackType, Channel* ch)
{
	vector<Plugin*>* pStack = getStack(stackType, ch);

	/* Empty stack, stack not found or mixer not ready: do nothing. */

	if (pStack == nullptr || pStack->size() == 0)
		return;

	assert(outBuf.countFrames() == audioBuffer.getNumSamples());

	/* MIDI channels must not process the current buffer: give them an empty one. 
	Sample channels and Master in/out want audio data instead: let's convert the 
	internal buffer from Giada to Juce. */

	if (ch != nullptr && ch->type == ChannelType::MIDI) 
		audioBuffer.clear();
	else
		for (int i=0; i<outBuf.countFrames(); i++)
			for (int j=0; j<outBuf.countChannels(); j++)
				audioBuffer.setSample(j, i, outBuf[i][j]);

	/* Hardcore processing. At the end we swap input and output, so that he N-th
	plugin will process the result of the plugin N-1. Part of this loop must be
	guarded by mutexes, i.e. the MIDI process part. You definitely don't want
	a situation like the following one:
		this::processStack()
		[a new midi event comes in from kernelMidi thread]
		channel::clearMidiBuffer()
	The midi event in between would be surely lost, deleted by the last call to
	channel::clearMidiBuffer()! */

	if (ch != nullptr)
		pthread_mutex_lock(&mutex_midi);

	for (const Plugin* plugin : *pStack) {
		if (plugin->isSuspended() || plugin->isBypassed())
			continue;

		/* If this is a Channel (ch != nullptr) and the current plugin is an 
		instrument (i.e. accepts MIDI), don't let it fill the current audio buffer: 
		create a new temporary one instead and then merge the result into the main
		one when done. This way each plug-in generates its own audio data and we can
		play more than one plug-in instrument in the same stack, driven by the same
		set of MIDI events. */

		if (ch != nullptr && plugin->acceptsMidi()) {
			juce::AudioBuffer<float> tmp(audioBuffer.getNumChannels(), buffersize);
			plugin->process(tmp, ch->getPluginMidiEvents());
			for (int i=0; i<audioBuffer.getNumSamples(); i++)
				for (int j=0; j<audioBuffer.getNumChannels(); j++)
					audioBuffer.addSample(j, i, tmp.getSample(j, i));	
		}
		else
			plugin->process(audioBuffer, juce::MidiBuffer()); // Empty MIDI buffer
	}

	if (ch != nullptr) {
		ch->clearMidiBuffer();
		pthread_mutex_unlock(&mutex_midi);
	}

	/* Converting buffer from Juce to Giada. A note for the future: if we 
	overwrite (=) (as we do now) it's SEND, if we add (+) it's INSERT. */

	for (int i=0; i<outBuf.countFrames(); i++)
		for (int j=0; j<outBuf.countChannels(); j++)	
			outBuf[i][j] = audioBuffer.getSample(j, i);
}


/* -------------------------------------------------------------------------- */


Plugin* getPluginByIndex(int index, int stackType, Channel* ch)
{
	vector<Plugin*>* pStack = getStack(stackType, ch);
	if (pStack->size() == 0)
		return nullptr;
	if ((unsigned) index >= pStack->size())
		return nullptr;
	return pStack->at(index);
}


/* -------------------------------------------------------------------------- */


int getPluginIndex(int id, int stackType, Channel* ch)
{
	vector<Plugin*>* pStack = getStack(stackType, ch);
	for (unsigned i=0; i<pStack->size(); i++)
		if (pStack->at(i)->getId() == id)
			return i;
	return -1;
}


/* -------------------------------------------------------------------------- */


void swapPlugin(unsigned indexA, unsigned indexB, int stackType,
	pthread_mutex_t* mutex, Channel* ch)
{
	vector<Plugin*>* pStack = getStack(stackType, ch);
	while (true) {
		if (pthread_mutex_trylock(mutex) != 0)
			continue;
		std::swap(pStack->at(indexA), pStack->at(indexB));
		pthread_mutex_unlock(mutex);
		gu_log("[pluginHost::swapPlugin] plugin at index %d and %d swapped\n", indexA, indexB);
		return;
	}
}


/* -------------------------------------------------------------------------- */


int freePlugin(int id, int stackType, pthread_mutex_t* mutex, Channel* ch)
{
	vector<Plugin*>* pStack = getStack(stackType, ch);
	for (unsigned i=0; i<pStack->size(); i++) {
		Plugin *pPlugin = pStack->at(i);
		if (pPlugin->getId() != id)
			continue;
		while (true) {
			if (pthread_mutex_trylock(mutex) != 0)
				continue;
			delete pPlugin;
			pStack->erase(pStack->begin() + i);
			pthread_mutex_unlock(mutex);
			gu_log("[pluginHost::freePlugin] plugin id=%d removed\n", id);
			return i;
		}
	}
	gu_log("[pluginHost::freePlugin] plugin id=%d not found\n", id);
	return -1;
}


/* -------------------------------------------------------------------------- */


void runDispatchLoop()
{
	messageManager->runDispatchLoopUntil(10);
	//gu_log("[pluginHost::runDispatchLoop] %d, hasStopMessageBeenSent=%d\n", r, messageManager->hasStopMessageBeenSent());
}


/* -------------------------------------------------------------------------- */


void freeAllStacks(vector<Channel*>* channels, pthread_mutex_t* mutex)
{
	freeStack(pluginHost::MASTER_OUT, mutex);
	freeStack(pluginHost::MASTER_IN, mutex);
	for (unsigned i=0; i<channels->size(); i++)
		freeStack(pluginHost::CHANNEL, mutex, channels->at(i));
	missingPlugins = false;
	unknownPluginList.clear();
}


/* -------------------------------------------------------------------------- */


int clonePlugin(Plugin* src, int stackType, pthread_mutex_t* mutex,
	Channel* ch)
{
	Plugin* p = addPlugin(src->getUniqueId(), stackType, mutex, ch);
	if (!p) {
		gu_log("[pluginHost::clonePlugin] unable to add new plugin to stack!\n");
		return 0;
	}

	for (int k=0; k<src->getNumParameters(); k++)
		p->setParameter(k, src->getParameter(k));

	return 1;
}


/* -------------------------------------------------------------------------- */


bool doesPluginExist(const string& fid)
{
	return pluginFormat.doesPluginStillExist(*knownPluginList.getTypeForFile(fid));
}


/* -------------------------------------------------------------------------- */


void sortPlugins(int method)
{
	switch (method) {
		case sortMethod::NAME:
			knownPluginList.sort(juce::KnownPluginList::SortMethod::sortAlphabetically, true);
			break;
		case sortMethod::CATEGORY:
			knownPluginList.sort(juce::KnownPluginList::SortMethod::sortByCategory, true);
			break;
		case sortMethod::MANUFACTURER:
			knownPluginList.sort(juce::KnownPluginList::SortMethod::sortByManufacturer, true);
			break;
		case sortMethod::FORMAT:
			knownPluginList.sort(juce::KnownPluginList::SortMethod::sortByFormat, true);
			break;
	}
}


/* -------------------------------------------------------------------------- */


void forEachPlugin(int stackType, const Channel* ch, std::function<void(const Plugin* p)> f)
{
	/* TODO - Remove const is ugly. This is a temporary workaround until all
	PluginHost functions params will be const-correct. */
	vector<Plugin*>* stack = getStack(stackType, const_cast<Channel*>(ch));
	for (const Plugin* p : *stack)
		f(p);
}


}}}; // giada::m::pluginHost::


#endif // #ifdef WITH_VST
