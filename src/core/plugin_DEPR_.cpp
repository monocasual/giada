/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
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
#include "plugin_DEPR_.h"


int Plugin_DEPR_::id_generator = 0;


/* -------------------------------------------------------------------------- */


Plugin_DEPR_::Plugin_DEPR_()
: module    (NULL),
  entryPoint(NULL),
  plugin    (NULL),
  id        (id_generator++),
  program   (-1),
  bypass    (false),
  suspended (false)
{}


/* -------------------------------------------------------------------------- */


Plugin_DEPR_::~Plugin_DEPR_()
{
	unload();
}


/* -------------------------------------------------------------------------- */


int Plugin_DEPR_::unload()
{
	if (module == NULL)
		return 1;

#if defined(_WIN32)

	FreeLibrary((HMODULE)module); // FIXME - error checking
	return 1;

#elif defined(__linux__)

	return dlclose(module) == 0 ? 1 : 0;

#elif defined(__APPLE__)

	/* we must unload bundles but because bundles may be in use for other
	plug-in types it is important (and mandatory on certain plug-ins,
	e.g. Korg) to do a check on the retain count. */

	CFIndex retainCount = CFGetRetainCount(module);

	if (retainCount == 1) {
		gLog("[plugin] retainCount == 1, can unload dlyb\n");
		CFBundleUnloadExecutable(module);
		CFRelease(module);
	}
	else
		gLog("[plugin] retainCount > 1 (%d), leave dlyb alone\n", (int) retainCount);

	return 1;

#endif
}


/* -------------------------------------------------------------------------- */


int Plugin_DEPR_::load(const char *fname)
{
	strcpy(pathfile, fname);

#if defined(_WIN32)

	module = LoadLibrary(pathfile);

#elif defined(__linux__)

	module = dlopen(pathfile, RTLD_LAZY);

#elif defined(__APPLE__)

  /* creates the path to the bundle. In OSX vsts are stored inside the
   * so-called bundles, just a directory with '.vst' extension. Finally
   * we open the bundle with CFBundleCreate. */

  CFStringRef pathStr   = CFStringCreateWithCString(NULL, pathfile, kCFStringEncodingASCII);
  CFURLRef    bundleUrl = CFURLCreateWithFileSystemPath(kCFAllocatorDefault,	pathStr, kCFURLPOSIXPathStyle, true);
  if(bundleUrl == NULL) {
    gLog("[plugin] unable to create URL reference for plugin\n");
    status = 0;
    return 0;
  }
  module = CFBundleCreate(kCFAllocatorDefault, bundleUrl);

#endif

	if (module) {

	/* release (free) any old string */

#ifdef __APPLE__
		CFRelease(pathStr);
		CFRelease(bundleUrl);
#endif
		//strcpy(pathfile, fname); ???????????
		status = 1;
		return 1;
	}
	else {

#if defined(_WIN32)

		gLog("[plugin] unable to load %s, error: %d\n", fname, (int) GetLastError());

#elif defined(__linux__)

		gLog("[plugin] unable to load %s, error: %s\n", fname, dlerror());

#elif defined(__APPLE__)

    gLog("[plugin] unable to create bundle reference\n");
    CFRelease(pathStr);
    CFRelease(bundleUrl);

#endif
		status = 0;
		return 0;
	}
}


/* -------------------------------------------------------------------------- */


int Plugin_DEPR_::init(VstIntPtr VSTCALLBACK (*HostCallback) (AEffect* effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt))
{
#if defined(_WIN32)

	entryPoint = (vstPluginFuncPtr) GetProcAddress((HMODULE)module, "VSTPluginMain");
	if (!entryPoint)
		entryPoint = (vstPluginFuncPtr) GetProcAddress((HMODULE)module, "main");

#elif defined(__linux__)

	/* bad stuff here: main() is a function pointer, dlsym(module, "main")
	 * returns a pointer to an object (void*) which should be casted to
	 * a pointer to function (main(), precisely). Unfortunately the standard
	 * forbids the conversion from void* to function pointer. So we do a raw
	 * mem copy from tmp to entryPoint. */

	void *tmp;
	tmp = dlsym(module, "VSTPluginMain");
	if (!tmp)
		tmp = dlsym(module, "main");
	memcpy(&entryPoint, &tmp, sizeof(tmp));

#elif defined(__APPLE__)

	/* same also for Unix/OSX. */

	void *tmp = NULL;
	tmp = CFBundleGetFunctionPointerForName(module, CFSTR("VSTPluginMain"));

	if (!tmp) {
		gLog("[plugin] entryPoint 'VSTPluginMain' not found\n");
		tmp = CFBundleGetFunctionPointerForName(module, CFSTR("main_macho"));  // VST SDK < 2.4
	}
	if (!tmp) {
		gLog("[plugin] entryPoint 'main_macho' not found\n");
		tmp = CFBundleGetFunctionPointerForName(module, CFSTR("main"));
	}
	if (tmp)
		memcpy(&entryPoint, &tmp, sizeof(tmp));
	else
		gLog("[plugin] entryPoint 'main' not found\n");

#endif

	/* if entry point is found, add to plugin a pointer to hostCallback. Or
	 * in other words bind the callback to the plugin. */

	if (entryPoint) {
		gLog("[plugin] entryPoint found\n");
		plugin = entryPoint(HostCallback);
		if (!plugin) {
			gLog("[plugin] failed to create effect instance!\n");
			return 0;
		}
	}
	else {
		gLog("[plugin] entryPoint not found, unable to proceed\n");
		return 0;
	}


	/* check the magicNumber */
	/** WARNING: on Windows one can load any DLL! Why!?! */

  if(plugin->magic == kEffectMagic) {
		gLog("[plugin] magic number OK\n");
		return 1;
	}
	else {
    gLog("[plugin] magic number is bad\n");
    return 0;
  }
}


/* -------------------------------------------------------------------------- */


int Plugin_DEPR_::setup(int samplerate, int frames)
{
  /* init plugin through the dispatcher with some basic infos */

  plugin->dispatcher(plugin, effOpen, 0, 0, 0, 0);
	plugin->dispatcher(plugin, effSetSampleRate, 0, 0, 0, samplerate);
	plugin->dispatcher(plugin, effSetBlockSize, 0, frames, 0, 0);

	/* check SDK compatibility */

	if (getSDKVersion() != kVstVersion)
		gLog("[plugin] warning: different VST version (host: %d, plugin: %d)\n", kVstVersion, getSDKVersion());

	return 1;
}


/* -------------------------------------------------------------------------- */


AEffect *Plugin_DEPR_::getPlugin()
{
	return plugin;
}


/* -------------------------------------------------------------------------- */


int Plugin_DEPR_::getId() { return id; }


/* -------------------------------------------------------------------------- */


int Plugin_DEPR_::getSDKVersion()
{
	return plugin->dispatcher(plugin, effGetVstVersion, 0, 0, 0, 0);
}


/* -------------------------------------------------------------------------- */


void Plugin_DEPR_::getName(char *out)
{
	char tmp[128] = "\0";
	plugin->dispatcher(plugin, effGetEffectName, 0, 0, tmp, 0);
	tmp[kVstMaxEffectNameLen-1] = '\0';
	strncpy(out, tmp, kVstMaxEffectNameLen);
}


/* -------------------------------------------------------------------------- */


void Plugin_DEPR_::getVendor(char *out)
{
	char tmp[128] = "\0";
	plugin->dispatcher(plugin, effGetVendorString, 0, 0, tmp, 0);
	tmp[kVstMaxVendorStrLen-1] = '\0';
	strncpy(out, tmp, kVstMaxVendorStrLen);
}


/* -------------------------------------------------------------------------- */


void Plugin_DEPR_::getProduct(char *out)
{
	char tmp[128] = "\0";
	plugin->dispatcher(plugin, effGetProductString, 0, 0, tmp, 0);
	tmp[kVstMaxProductStrLen-1] = '\0';
	strncpy(out, tmp, kVstMaxProductStrLen);
}


/* -------------------------------------------------------------------------- */


int Plugin_DEPR_::getNumPrograms() { return plugin->numPrograms; }


/* -------------------------------------------------------------------------- */


int Plugin_DEPR_::setProgram(int index)
{
	plugin->dispatcher(plugin, effBeginSetProgram, 0, 0, 0, 0);
	plugin->dispatcher(plugin, effSetProgram, 0, index, 0, 0);
	gLog("[plugin] program changed, index %d\n", index);
	program = index;
	return plugin->dispatcher(plugin, effEndSetProgram, 0, 0, 0, 0);
}


/* -------------------------------------------------------------------------- */


int Plugin_DEPR_::getNumParams() const { return plugin->numParams; }


/* -------------------------------------------------------------------------- */


int Plugin_DEPR_::getNumInputs() { return plugin->numInputs; }


/* -------------------------------------------------------------------------- */


int Plugin_DEPR_::getNumOutputs() {	return plugin->numOutputs; }


/* -------------------------------------------------------------------------- */


void Plugin_DEPR_::getProgramName(int index, char *out)
{
	char tmp[128] = "\0";
	plugin->dispatcher(plugin, effGetProgramNameIndexed, index, 0, tmp, 0);
	tmp[kVstMaxProgNameLen-1] = '\0';
	strncpy(out, tmp, kVstMaxProgNameLen);
}


/* -------------------------------------------------------------------------- */


void Plugin_DEPR_::getParamName(int index, char *out)
{
	char tmp[128] = "\0";
	plugin->dispatcher(plugin, effGetParamName, index, 0, tmp, 0);
	tmp[kVstMaxParamStrLen-1] = '\0';
	strncpy(out, tmp, kVstMaxParamStrLen);
}


/* -------------------------------------------------------------------------- */


void Plugin_DEPR_::getParamLabel(int index, char *out)
{
	char tmp[128] = "\0";
	plugin->dispatcher(plugin, effGetParamLabel, index, 0, tmp, 0);
	tmp[kVstMaxParamStrLen-1] = '\0';
	strncpy(out, tmp, kVstMaxParamStrLen);
}


/* -------------------------------------------------------------------------- */


void Plugin_DEPR_::getParamDisplay(int index, char *out)
{
	char tmp[128] = "\0";
	plugin->dispatcher(plugin, effGetParamDisplay, index, 0, tmp, 0);
	tmp[kVstMaxParamStrLen-1] = '\0';
	strncpy(out, tmp, kVstMaxParamStrLen);
}


/* -------------------------------------------------------------------------- */


float Plugin_DEPR_::getParam(int index) const
{
	return plugin->getParameter(plugin, index);
}


/* -------------------------------------------------------------------------- */


void Plugin_DEPR_::setParam(int index, float value)
{
	plugin->setParameter(plugin, index, value);
}


/* -------------------------------------------------------------------------- */


bool Plugin_DEPR_::hasGui()
{
	return plugin->flags & effFlagsHasEditor;
}


/* -------------------------------------------------------------------------- */


void Plugin_DEPR_::openGui(void *w)
{
	long val = 0;
#ifdef __linux__
  val = (long) w;
#endif
	plugin->dispatcher(plugin, effEditOpen, 0, val, w, 0);
}


/* -------------------------------------------------------------------------- */


void Plugin_DEPR_::closeGui()
{
	plugin->dispatcher(plugin, effEditClose, 0, 0, 0, 0);
}


/* -------------------------------------------------------------------------- */


int Plugin_DEPR_::getGuiWidth()
{
	ERect *pErect = NULL;
	plugin->dispatcher(plugin, effEditGetRect, 0, 0, &pErect, 0);
	return pErect->top + pErect->right;
}


/* -------------------------------------------------------------------------- */


int Plugin_DEPR_::getGuiHeight()
{
	ERect *pErect = NULL;
	plugin->dispatcher(plugin, effEditGetRect, 0, 0, &pErect, 0);
	return pErect->top + pErect->bottom;
}


/* -------------------------------------------------------------------------- */


void Plugin_DEPR_::idle()
{
	plugin->dispatcher(plugin, effEditIdle, 0, 0, NULL, 0);
}


/* -------------------------------------------------------------------------- */


void Plugin_DEPR_::processAudio(float **in, float **out, long frames)
{
	plugin->processReplacing(plugin, in, out, frames);
}


/* -------------------------------------------------------------------------- */


void Plugin_DEPR_::processEvents(VstEvents *events)
{
	plugin->dispatcher(plugin, effProcessEvents, 0, 0, events, 0.0);
}


/* -------------------------------------------------------------------------- */


void Plugin_DEPR_::resume()
{
	plugin->dispatcher(plugin, effMainsChanged, 0, 1, 0, 0);
	suspended = false;
}


/* -------------------------------------------------------------------------- */


void Plugin_DEPR_::suspend()
{
	plugin->dispatcher(plugin, effMainsChanged, 0, 0, 0, 0);
	suspended = true;
}


/* -------------------------------------------------------------------------- */


void Plugin_DEPR_::close()
{
	plugin->dispatcher(plugin, effClose, 0, 0, 0, 0);
}


/* -------------------------------------------------------------------------- */


void Plugin_DEPR_::getRect(ERect **out)
{
	plugin->dispatcher(plugin, effEditGetRect, 0, 0, out, 0);
}


#endif
