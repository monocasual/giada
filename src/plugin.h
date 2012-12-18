/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * plugin
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2012 Giovanni A. Zuliani | Monocasual
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

#ifndef __PLUGIN_H
#define __PLUGIN_H

#include <cstdio>

/* before including aeffetx(x).h we must define __cdecl, otherwise VST
 * headers can't be compiled correctly. In windows __cdecl is already
 * defined. */

#ifdef __GNUC__
	#ifndef _WIN32
		#define __cdecl
	#endif
#endif

#include "vst/aeffectx.h"

#if defined(_WIN32)
	#include <windows.h>
#elif defined(__linux__)
	#include <dlfcn.h>
	#include <X11/Xlib.h>
#elif defined(__APPLE__)
	#include <CoreFoundation/CFBundle.h>
#endif

#include <limits.h>  // PATH_MAX


// Plugin's entry point
typedef AEffect* (*vstPluginFuncPtr)(audioMasterCallback host);


class Plugin {

private:

#if defined(_WIN32) || defined(__linux__)
	void             *module;     // dll, so, ...
#elif defined(__APPLE__)
	CFBundleRef       module;			// OSX bundle
#endif

	vstPluginFuncPtr  entryPoint; // VST entry point
	AEffect          *plugin;     // real plugin

	/* each plugin has an unique ID */

	static int id_generator;
	int        id;

	/* unload
	 * free plugin from memory. Calls dlclose and similars. */

	int unload();

public:
	Plugin();
	~Plugin();

	int  load(const char *fname);
	int  init(VstIntPtr VSTCALLBACK (*HostCallback)(AEffect*, VstInt32, VstInt32, VstIntPtr, void*, float));
	int  setup(int samplerate, int frames);

	/* get[Item].
	 * Wrappers called by host when it wants info from the plugin. */

	int   getId();
	int   getSDKVersion();
	void  getName   (char *out);
	void  getVendor (char *out);
	void  getProduct(char *out);
	int   getNumPrograms();
	int   getNumParams();
	int   getNumInputs();
	int   getNumOutputs();
	void  getProgramName(int index, char *out);
	void  getParamName(int index, char *out);
	void  getParamLabel(int index, char *out);   // parameter's value(0, -39, ...)
	void  getParamDisplay(int index, char *out); // parameter's unit measurement (dB, Pan, ...)
	float getParam(int index);
	void  setParam(int index, float value);

	bool  hasGui();
	void  openGui(void *w);
	void  closeGui();
	int   getGuiWidth();
	int   getGuiHeight();
	void  idle();

	void  processAudio(float **in, float **out, long frames);
	void  resume();
	void  suspend();
	void  close();

	/* there's a specific opcode for the bypass, but we don't trust the
	 * plugin's developers. */

	bool bypass;

	/* the status of the plugin:
	 * 1: ok
	 * 0: missing (file not found) */

	int status;

	char pathfile[PATH_MAX]; // full path filename
};

#endif

#endif // #ifdef WITH_VST
