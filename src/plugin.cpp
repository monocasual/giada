/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
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

#include "plugin.h"


int Plugin::id_generator = 0;


/* ------------------------------------------------------------------ */


Plugin::Plugin()
	: module(NULL),
	  entryPoint(NULL),
	  plugin(NULL),
	  id(id_generator++),
	  bypass(false),
	  suspended(false)
{}


/* ------------------------------------------------------------------ */


Plugin::~Plugin() {
	if (module)
		unload();
}


/* ------------------------------------------------------------------ */


int Plugin::unload() {
#if defined(_WIN32)
	FreeLibrary((HMODULE)module); // FIXME - error checking
	return 1;
#elif defined(__linux__)
	return dlclose(module) == 0 ? 1 : 0;
#elif defined(__APPLE__)
	CFBundleUnloadExecutable(module); // FIXME - error checking
	CFRelease(module);
	return 1;
#endif
}


/* ------------------------------------------------------------------ */


int Plugin::load(const char *fname) {

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
    puts("[plugin] unable to create URL reference for plugin");
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

		printf("[plugin] unable to load %s, error: %d\n", fname, (int) GetLastError());

#elif defined(__linux__)

		printf("[plugin] unable to load %s, error: %s\n", fname, dlerror());

#elif defined(__APPLE__)

    puts("[plugin] unable to create bundle reference");
    CFRelease(pathStr);
    CFRelease(bundleUrl);

#endif
		status = 0;
		return 0;
	}
}


/* ------------------------------------------------------------------ */


int Plugin::init(VstIntPtr VSTCALLBACK (*HostCallback) (AEffect* effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt)) {

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

	void *tmp;
	tmp = CFBundleGetFunctionPointerForName(module, CFSTR("VSTPluginMain"));
	if (!tmp)
		tmp = CFBundleGetFunctionPointerForName(module, CFSTR("main_macho"));
	if (!tmp)
		tmp = CFBundleGetFunctionPointerForName(module, CFSTR("main"));
	memcpy(&entryPoint, &tmp, sizeof(tmp));

#endif

	/* if entry point is found, add to plugin a pointer to hostCallback. Or
	 * in other words bind the callback to the plugin. */

	if (entryPoint) {
		puts("[plugin] entryPoint found");
		plugin = entryPoint(HostCallback);
		if (!plugin) {
			puts("[plugin] failed to create effect instance!");
			unload();
			return 0;
		}
	}
	else {
		puts("[plugin] entryPoint not found, unable to proceed");
		unload();
		return 0;
	}


	/* check the magicNumber */
	/** WARNING: on Windows one can load any DLL! Why!?! */

  if(plugin->magic == kEffectMagic) {
		puts("[plugin] magic number OK");
		return 1;
	}
	else {
    puts("[plugin] magic number is bad");
    return 0;
  }
}


/* ------------------------------------------------------------------ */


int Plugin::setup(int samplerate, int frames) {

  /* init plugin through the dispatcher with some basic infos */

  plugin->dispatcher(plugin, effOpen, 0, 0, 0, 0);
	plugin->dispatcher(plugin, effSetSampleRate, 0, 0, 0, samplerate);
	plugin->dispatcher(plugin, effSetBlockSize, 0, frames, 0, 0);

	/* check SDK compatibility */

	if (getSDKVersion() != kVstVersion)
		printf("[plugin] warning: different VST version (host: %d, plugin: %d)\n", kVstVersion, getSDKVersion());

	return 1;
}


/* ------------------------------------------------------------------ */


AEffect *Plugin::getPlugin() {
	return plugin;
}


/* ------------------------------------------------------------------ */


int Plugin::getId() {
	return id;
}

int Plugin::getSDKVersion() {
	return plugin->dispatcher(plugin, effGetVstVersion, 0, 0, 0, 0);
}

void Plugin::getName(char *out) {
	char tmp[256];
	plugin->dispatcher(plugin, effGetEffectName, 0, 0, tmp, 0);
	strlen(tmp) == 0 ? strcpy(out, " ") : strcpy(out, tmp);
}

void Plugin::getVendor(char *out) {
	plugin->dispatcher(plugin, effGetVendorString, 0, 0, out, 0);
}

void Plugin::getProduct(char *out) {
	char tmp[256];
	plugin->dispatcher(plugin, effGetProductString, 0, 0, tmp, 0);
	strlen(tmp) == 0 ? strcpy(out, " ") : strcpy(out, tmp);
}

int Plugin::getNumPrograms() {
	return plugin->numPrograms;
}

int Plugin::setProgram(int index) {
	plugin->dispatcher(plugin, effBeginSetProgram, 0, 0, 0, 0);
	plugin->dispatcher(plugin, effSetProgram, 0, index, 0, 0);
	printf("[plugin] program changed, index %d\n", index);
	return plugin->dispatcher(plugin, effEndSetProgram, 0, 0, 0, 0);
}

int Plugin::getNumParams() {
	return plugin->numParams;
}

int Plugin::getNumInputs() {
	return plugin->numInputs;
}

int Plugin::getNumOutputs() {
	return plugin->numOutputs;
}

void Plugin::getProgramName(int index, char *out) {
	char tmp[kVstMaxProgNameLen];
	plugin->dispatcher(plugin, effGetProgramNameIndexed, index, 0, tmp, 0);
	strncpy(out, tmp, kVstMaxProgNameLen);
}

void Plugin::getParamName(int index, char *out) {
	plugin->dispatcher(plugin, effGetParamName, index, 0, out, 0);
}

void Plugin::getParamLabel(int index, char *out) {
	char tmp[64];
	plugin->dispatcher(plugin, effGetParamLabel, index, 0, tmp, 0);
	tmp[strlen(tmp)] = '\0';
	strncpy(out, tmp, 64);
}

void Plugin::getParamDisplay(int index, char *out) {
	char tmp[64];
	plugin->dispatcher(plugin, effGetParamDisplay, index, 0, tmp, 0);
	tmp[strlen(tmp)] = '\0';
	strncpy(out, tmp, 64);
}

float Plugin::getParam(int index) {
	return plugin->getParameter(plugin, index);
}

void Plugin::setParam(int index, float value) {
	plugin->setParameter(plugin, index, value);
}

bool Plugin::hasGui() {
	return plugin->flags & effFlagsHasEditor;
}

void Plugin::openGui(void *w) {
	long val = 0;
#ifdef __linux__
  val = (long) w;
#endif
	plugin->dispatcher(plugin, effEditOpen, 0, val, w, 0);
}

void Plugin::closeGui() {
	plugin->dispatcher(plugin, effEditClose, 0, 0, 0, 0);
}

int Plugin::getGuiWidth() {
	ERect *pErect = NULL;
	plugin->dispatcher(plugin, effEditGetRect, 0, 0, &pErect, 0);
	return pErect->top + pErect->right;
}

int Plugin::getGuiHeight() {
	ERect *pErect = NULL;
	plugin->dispatcher(plugin, effEditGetRect, 0, 0, &pErect, 0);
	return pErect->top + pErect->bottom;
}

void Plugin::idle() {
	plugin->dispatcher(plugin, effEditIdle, 0, 0, NULL, 0);
}

void Plugin::processAudio(float **in, float **out, long frames) {
	plugin->processReplacing(plugin, in, out, frames);
}

void Plugin::resume() {
	plugin->dispatcher(plugin, effMainsChanged, 0, 1, 0, 0);
	suspended = false;
}

void Plugin::suspend() {
	plugin->dispatcher(plugin, effMainsChanged, 0, 0, 0, 0);
	suspended = true;
}

void Plugin::close() {
	plugin->dispatcher(plugin, effClose, 0, 0, 0, 0);
}


#endif
