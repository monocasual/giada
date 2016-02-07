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
#include "plugin.h"
#include "pluginHost.h"


using std::string;


int PluginHost::scanDir(const string &dirpath)
{
  gLog("[PluginHost::scanDir] plugin directory = '%s'\n", dirpath.c_str());

  juce::VSTPluginFormat format;
  juce::FileSearchPath path(dirpath);
  juce::PluginDirectoryScanner scanner(knownPluginList, format, path, false, juce::File::nonexistent);
  juce::String name;

  bool cont = true;
  while (cont) {
    cont = scanner.scanNextFile(false, name);
  }

  gLog("[PluginHost::scanDir] %d plugin(s) found\n", knownPluginList.getNumTypes());

  return knownPluginList.getNumTypes();
}


/* -------------------------------------------------------------------------- */


int PluginHost::saveList(const string &filepath)
{
  return knownPluginList.createXml()->writeToFile(juce::File(filepath), "");
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
  pthread_mutex_t *mutex, int freq, int bufSize, class Channel *ch)
{
  /* Get the proper stack to add the plugin to */

  vector <Plugin *> *pStack;
	pStack = getStack(stackType, ch);

  /* Initialize plugin */

  juce::PluginDescription *pd = knownPluginList.getTypeForFile(fid);
  if (!pd) {
    gLog("[PluginHost::addPlugin] no plugin found with fid=%s!\n", fid.c_str());
    return NULL;
  }

  Plugin *p = (Plugin *) pluginFormat.createInstanceFromDescription(*pd, freq, bufSize);
  if (!p) {
    gLog("[PluginHost::addPlugin] unable to create instance with fid=%s!\n", fid.c_str());
    return NULL;
  }

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
    fid.c_str(), p->getName().toRawUTF8(), stackType, pStack->size());

  return p;
}


/* -------------------------------------------------------------------------- */


Plugin *PluginHost::addPlugin(int index, int stackType, pthread_mutex_t *mutex,
  int freq, int bufSize, class Channel *ch)
{
  juce::PluginDescription *pd = knownPluginList.getType(index);
  if (pd) {
    gLog("[PluginHost::addPlugin] plugin found, uid=%s, name=%s...\n",
      pd->fileOrIdentifier.toRawUTF8(), pd->name.toRawUTF8());
    return addPlugin(pd->fileOrIdentifier.toRawUTF8(), stackType, mutex, freq, bufSize, ch);
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

#endif // #ifdef WITH_VST
