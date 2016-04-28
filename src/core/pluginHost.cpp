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


#include "../utils/log.h"
#include "../utils/utils.h"
#include "mixer.h"
#include "channel.h"
#include "midiChannel.h"
#include "plugin.h"
#include "pluginHost.h"


using std::string;


PluginHost::~PluginHost()
{
  messageManager->deleteInstance();
}


/* -------------------------------------------------------------------------- */


void PluginHost::init(int _buffersize, int _samplerate)
{
  gLog("[PluginHost::init] initialize with buffersize=%d, samplerate=%d\n",
    _buffersize, _samplerate);

  messageManager = juce::MessageManager::getInstance();
  audioBuffer.setSize(2, _buffersize);
  samplerate = _samplerate;
  buffersize = _buffersize;
  missingPlugins = false;
  //unknownPluginList.empty();
  loadList(gGetHomePath() + gGetSlash() + "plugins.xml");
}


/* -------------------------------------------------------------------------- */


int PluginHost::scanDir(const string &dirpath, void (*callback)(float progress, void *p),
    void *p)
{
  gLog("[PluginHost::scanDir] requested directory: '%s'\n", dirpath.c_str());
  gLog("[PluginHost::scanDir] current plugins: %d\n", knownPluginList.getNumTypes());

  knownPluginList.clear();   // clear up previous plugins

  juce::VSTPluginFormat format;
  juce::FileSearchPath path(dirpath);
  juce::PluginDirectoryScanner scanner(knownPluginList, format, path,
      true, juce::File::nonexistent); // true: recursive

  bool cont = true;
  juce::String name;
  while (cont) {
    gLog("[PluginHost::scanDir]   scanning '%s'\n", name.toRawUTF8());
    cont = scanner.scanNextFile(false, name);
    if (callback)
      callback(scanner.getProgress(), p);
  }

  gLog("[PluginHost::scanDir] %d plugin(s) found\n", knownPluginList.getNumTypes());
  return knownPluginList.getNumTypes();
}


/* -------------------------------------------------------------------------- */


int PluginHost::saveList(const string &filepath)
{
  int out = knownPluginList.createXml()->writeToFile(juce::File(filepath), "");
  if (!out)
    gLog("[PluginHost::saveList] unable to save plugin list to %s\n", filepath.c_str());
  return out;
}


/* -------------------------------------------------------------------------- */


int PluginHost::loadList(const string &filepath)
{
  juce::XmlElement *elem = juce::XmlDocument::parse(juce::File(filepath));
  if (elem) {
    knownPluginList.recreateFromXml(*elem);
    delete elem;
    return 1;
  }
  return 0;
}


/* -------------------------------------------------------------------------- */


Plugin *PluginHost::addPlugin(const string &fid, int stackType,
  pthread_mutex_t *mutex, class Channel *ch)
{
  /* Get the proper stack to add the plugin to */

  vector <Plugin *> *pStack;
	pStack = getStack(stackType, ch);

  /* Initialize plugin */

  juce::PluginDescription *pd = knownPluginList.getTypeForFile(fid);
  if (!pd) {
    gLog("[PluginHost::addPlugin] no plugin found with fid=%s!\n", fid.c_str());
    missingPlugins = true;
    unknownPluginList.push_back(fid);
    return NULL;
  }

  Plugin *p = (Plugin *) pluginFormat.createInstanceFromDescription(*pd, samplerate, buffersize);
  if (!p) {
    gLog("[PluginHost::addPlugin] unable to create instance with fid=%s!\n", fid.c_str());
    missingPlugins = true;
    return NULL;
  }

  p->setStatus(1);
  p->setId();
  p->prepareToPlay(samplerate, buffersize);

  gLog("[PluginHost::addPlugin] plugin instance with fid=%s created\n", fid.c_str());

  /* Try to inject the plugin as soon as possible. */

  int lockStatus;
  while (true) {
    lockStatus = pthread_mutex_trylock(mutex);
    if (lockStatus == 0) {
      pStack->push_back(p);
      pthread_mutex_unlock(mutex);
      break;
    }
  }

  gLog("[PluginHost::addPlugin] plugin id=%s loaded (%s), stack type=%d, stack size=%d\n",
    fid.c_str(), p->getName().toStdString().c_str(), stackType, pStack->size());

  return p;
}


/* -------------------------------------------------------------------------- */


Plugin *PluginHost::addPlugin(int index, int stackType, pthread_mutex_t *mutex,
  class Channel *ch)
{
  juce::PluginDescription *pd = knownPluginList.getType(index);
  if (pd) {
    gLog("[PluginHost::addPlugin] plugin found, uid=%s, name=%s...\n",
      pd->fileOrIdentifier.toStdString().c_str(), pd->name.toStdString().c_str());
    return addPlugin(pd->fileOrIdentifier.toStdString(), stackType, mutex, ch);
  }
  else {
    gLog("[PluginHost::addPlugin] no plugins found at index=%d!\n", index);
    return NULL;
  }
}


/* -------------------------------------------------------------------------- */


vector <Plugin *> *PluginHost::getStack(int stackType, Channel *ch)
{
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


/* -------------------------------------------------------------------------- */


unsigned PluginHost::countPlugins(int stackType, Channel *ch)
{
	vector <Plugin *> *pStack = getStack(stackType, ch);
	return pStack->size();
}


/* -------------------------------------------------------------------------- */


int PluginHost::countAvailablePlugins()
{
  return knownPluginList.getNumTypes();
}


/* -------------------------------------------------------------------------- */


unsigned PluginHost::countUnknownPlugins()
{
  return unknownPluginList.size();
}


/* -------------------------------------------------------------------------- */


PluginHost::PluginInfo PluginHost::getAvailablePluginInfo(int i)
{
  juce::PluginDescription *pd = knownPluginList.getType(i);
  PluginInfo pi;
  pi.uid = pd->fileOrIdentifier.toStdString();
  pi.name = pd->name.toStdString();
  pi.category = pd->category.toStdString();
  pi.manufacturerName = pd->manufacturerName.toStdString();
  pi.format = pd->pluginFormatName.toStdString();
  pi.isInstrument = pd->isInstrument;
/*
  if (!p) {
    gLog("[PluginHost::getAvailablePlugin] unable to create plugin instance!\n");
    return NULL;
  }
  */
  return pi;
}


/* -------------------------------------------------------------------------- */


string PluginHost::getUnknownPluginInfo(int i)
{
  return unknownPluginList.at(i);
}


/* -------------------------------------------------------------------------- */


void PluginHost::freeStack(int stackType, pthread_mutex_t *mutex, Channel *ch)
{
	vector <Plugin *> *pStack;
	pStack = getStack(stackType, ch);

	if (pStack->size() == 0)
		return;

	int lockStatus;
	while (true) {
		lockStatus = pthread_mutex_trylock(mutex);
		if (lockStatus == 0) {
			for (unsigned i=0; i<pStack->size(); i++) {
        Plugin *pPlugin = pStack->at(i);
        if (pPlugin->getStatus() != 0) { // take care if plugin is in good status
          pPlugin->suspendProcessing(true);
          pPlugin->releaseResources();
        }
				delete pPlugin;
			}
			pStack->clear();
			pthread_mutex_unlock(mutex);
			break;
		}
	}
  gLog("[PluginHost::freeStack] stack type=%d freed\n", stackType);
}


/* -------------------------------------------------------------------------- */


void PluginHost::processStack(float *buffer, int stackType, Channel *ch)
{
	vector <Plugin *> *pStack = getStack(stackType, ch);

	/* empty stack, stack not found or mixer not ready: do nothing */

	if (pStack == NULL || pStack->size() == 0)
		return;

	/* converting buffer from Giada to Juce */

	for (int i=0; i<buffersize; i++) {
    audioBuffer.setSample(0, i, buffer[i*2]);
    audioBuffer.setSample(1, i, buffer[(i*2)+1]);
	}

	/* hardcore processing. At the end we swap input and output, so that
	 * the N-th plugin will process the result of the plugin N-1. */

	for (unsigned i=0; i<pStack->size(); i++) {
    Plugin *plugin = pStack->at(i);
		if (plugin->getStatus() != 1 || plugin->isSuspended() || plugin->isBypassed())
			continue;
    juce::MidiBuffer midiBuffer;
    if (ch) // ch might be null if stackType is MASTER_IN or MASTER_OUT
      midiBuffer = ch->getPluginMidiEvents();
		plugin->processBlock(audioBuffer, midiBuffer);
  }

	/* converting buffer from Juce to Giada. A note for the future: if we
	 * overwrite (=) (as we do now) it's SEND, if we add (+) it's INSERT. */

	for (int i=0; i<buffersize; i++) {
		buffer[i*2]     = audioBuffer.getSample(0, i);
		buffer[(i*2)+1] = audioBuffer.getSample(1, i);
	}
}


/* -------------------------------------------------------------------------- */


Plugin *PluginHost::getPluginByIndex(int index, int stackType, Channel *ch)
{
	vector <Plugin *> *pStack = getStack(stackType, ch);
	if (pStack->size() == 0)
		return NULL;
	if ((unsigned) index >= pStack->size())
		return NULL;
	return pStack->at(index);
}


/* -------------------------------------------------------------------------- */


int PluginHost::getPluginIndex(int id, int stackType, Channel *ch)
{
	vector <Plugin *> *pStack = getStack(stackType, ch);
	for (unsigned i=0; i<pStack->size(); i++)
		if (pStack->at(i)->getId() == id)
			return i;
	return -1;
}


/* -------------------------------------------------------------------------- */


void PluginHost::swapPlugin(unsigned indexA, unsigned indexB, int stackType,
  pthread_mutex_t *mutex, Channel *ch)
{
	vector <Plugin *> *pStack = getStack(stackType, ch);
	int lockStatus;
	while (true) {
		lockStatus = pthread_mutex_trylock(mutex);
		if (lockStatus == 0) {
			std::swap(pStack->at(indexA), pStack->at(indexB));
			pthread_mutex_unlock(mutex);
			gLog("[pluginHost::swapPlugin] plugin at index %d and %d swapped\n", indexA, indexB);
			return;
		}
		//else
			//gLog("[pluginHost] waiting for mutex...\n");
	}
}


/* -------------------------------------------------------------------------- */


void PluginHost::freePlugin(int id, int stackType, pthread_mutex_t *mutex,
  Channel *ch)
{
	vector <Plugin *> *pStack = getStack(stackType, ch);

	/* try to delete the plugin until succeed. G_Mixer has priority. */

	for (unsigned i=0; i<pStack->size(); i++) {
    Plugin *pPlugin = pStack->at(i);
		if (pPlugin->getId() == id) {
			if (pPlugin->getStatus() == 0) { // no frills if plugin is missing
				delete pPlugin;
				pStack->erase(pStack->begin() + i);
        gLog("[pluginHost::freePlugin] plugin id=%d removed with no frills, since it had status=0\n", id);
				return;
			}
			else {
				int lockStatus;
				while (true) {
					lockStatus = pthread_mutex_trylock(mutex);
					if (lockStatus == 0) {
            pPlugin->suspendProcessing(true);
            pPlugin->releaseResources();
						delete pPlugin;
						pStack->erase(pStack->begin() + i);
						pthread_mutex_unlock(mutex);
						gLog("[pluginHost::freePlugin] plugin id=%d removed\n", id);
						return;
					}
					//else
						//gLog("[pluginHost] waiting for mutex...\n");
				}
			}
		}
  }
	gLog("[pluginHost::freePlugin] plugin id=%d not found\n", id);
}


/* -------------------------------------------------------------------------- */


void PluginHost::runDispatchLoop()
{
  messageManager->runDispatchLoopUntil(10);
  //gLog("[PluginHost::runDispatchLoop] %d, hasStopMessageBeenSent=%d\n", r, messageManager->hasStopMessageBeenSent());
}


/* -------------------------------------------------------------------------- */


void PluginHost::freeAllStacks(vector <Channel*> *channels, pthread_mutex_t *mutex)
{
	freeStack(PluginHost::MASTER_OUT, mutex);
	freeStack(PluginHost::MASTER_IN, mutex);
	for (unsigned i=0; i<channels->size(); i++)
		freeStack(PluginHost::CHANNEL, mutex, channels->at(i));
  missingPlugins = false;
  unknownPluginList.clear();
}


/* -------------------------------------------------------------------------- */


int PluginHost::clonePlugin(Plugin *src, int stackType, pthread_mutex_t *mutex,
  Channel *ch)
{
  juce::PluginDescription pd = src->getPluginDescription();
	Plugin *p = addPlugin(pd.fileOrIdentifier.toStdString(), stackType, mutex, ch);
	if (!p) {
		gLog("[PluginHost::clonePlugin] unable to add new plugin to stack!\n");
		return 0;
	}
	for (int k=0; k<src->getNumParameters(); k++) {
		p->setParameter(k, src->getParameter(k));
	}
	return 1;
}


/* -------------------------------------------------------------------------- */


bool PluginHost::doesPluginExist(const string &fid)
{
  return pluginFormat.doesPluginStillExist(*knownPluginList.getTypeForFile(fid));
}


/* -------------------------------------------------------------------------- */


void PluginHost::processStackOffline(float *buffer, int stackType, Channel *ch, int size)
{
#if 0
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
		//	gLog("chunk of buffer left, size=%d\n", left);

		index+=step;
	}
#endif
}


#endif // #ifdef WITH_VST
