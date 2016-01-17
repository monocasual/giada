/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_pluginWindowGUI
 *
 * ---------------------------------------------------------------------
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
 * ------------------------------------------------------------------ */


#ifdef WITH_VST


#include "../../utils/log.h"
#include "../../utils/gui_utils.h"
#include "../../core/pluginHost_DEPR_.h"
#include "../elems/ge_mixed.h"
#include "gd_pluginWindowGUI_DEPR_.h"


extern PluginHost G_PluginHost;


gdPluginWindowGUI::gdPluginWindowGUI(Plugin *pPlugin)
 : gWindow(450, 300), pPlugin(pPlugin)
{

  /* some effects like to have us get their rect before opening them */

  ERect *rect;
	pPlugin->getRect(&rect);

	gu_setFavicon(this);
	set_non_modal();
	resize(x(), y(), pPlugin->getGuiWidth(), pPlugin->getGuiWidth());
	show();

  gLog("[gdPluginWindowGUI] open window, w=%d h=%d\n",
      pPlugin->getGuiWidth(), pPlugin->getGuiWidth());

	/* Fl::check(): Waits until "something happens" and then returns. It's
	 * mandatory on linux, otherwise X can't find 'this' window. */

#ifndef __APPLE__
	Fl::check();
#endif

	pPlugin->openGui((void*)fl_xid(this));

	char name[256];
	pPlugin->getProduct(name);
	copy_label(name);

	/* add a pointer to this window to plugin */

	pPlugin->window = this;

	pPlugin->idle();
}


/* ------------------------------------------------------------------ */


gdPluginWindowGUI::~gdPluginWindowGUI() {
	pPlugin->closeGui();
}


/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */


#if defined(__APPLE__)


pascal OSStatus gdPluginWindowGUImac::windowHandler(EventHandlerCallRef ehc, EventRef e, void *data) {
	return ((gdPluginWindowGUImac*)data)->__wh(ehc, e);
}


/* ------------------------------------------------------------------ */


pascal OSStatus gdPluginWindowGUImac::__wh(EventHandlerCallRef inHandlerCallRef, EventRef inEvent) {
	OSStatus result   = eventNotHandledErr;     // let the Carbon Event Manager close the window
	UInt32 eventClass = GetEventClass(inEvent);
	UInt32 eventKind  = GetEventKind(inEvent);

	switch (eventClass)	{
		case kEventClassWindow:	{
			switch (eventKind) {
				case kEventWindowClose:	{
					gLog("[pluginWindowMac] <<< CALLBACK >>> kEventWindowClose for gWindow=%p, window=%p\n", (void*)this, (void*)carbonWindow);
					show();
					break;
				}
				case kEventWindowClosed: {
					gLog("[pluginWindowMac] <<< CALLBACK >>> kEventWindowClosed for gWindow=%p, window=%p\n", (void*)this, (void*)carbonWindow);
					open = false;
					result = noErr;
					break;
				}
			}
			break;
		}
	}
	return result;
}


/* ------------------------------------------------------------------ */


gdPluginWindowGUImac::gdPluginWindowGUImac(Plugin *pPlugin)
 : gWindow(450, 300), pPlugin(pPlugin), carbonWindow(NULL)
{

  /* some effects like to have us get their rect before opening them */

  ERect *rect;
	pPlugin->getRect(&rect);

	/* window initialization */

	Rect wRect;

	wRect.top    = rect->top;
	wRect.left   = rect->left;
	wRect.bottom = rect->bottom;
	wRect.right  = rect->right;

  int winclass = kDocumentWindowClass;
  int winattr  = kWindowStandardHandlerAttribute |
                 kWindowCloseBoxAttribute        |
                 kWindowCompositingAttribute     |
                 kWindowAsyncDragAttribute;

  // winattr &= GetAvailableWindowAttributes(winclass);	// make sure that the window will open

  OSStatus status = CreateNewWindow(winclass, winattr, &wRect, &carbonWindow);
	if (status != noErr)	{
		gLog("[pluginWindowMac] Unable to create window! Status=%d\n", (int) status);
		return;
	}
	else
		gLog("[pluginWindowMac] created window=%p\n", (void*)carbonWindow);

	/* install event handler, called when window is closed */

	static EventTypeSpec eventTypes[] = {
		{ kEventClassWindow, kEventWindowClose },
		{ kEventClassWindow, kEventWindowClosed }
	};
	InstallWindowEventHandler(carbonWindow, windowHandler, GetEventTypeCount(eventTypes), eventTypes, this, NULL);

	/* open window, center it, show it and start the handler */

	pPlugin->openGui((void*)carbonWindow);
	RepositionWindow(carbonWindow, NULL, kWindowCenterOnMainScreen);
	ShowWindow(carbonWindow);
	open = true;
}



/* ------------------------------------------------------------------ */


gdPluginWindowGUImac::~gdPluginWindowGUImac() {
	gLog("[pluginWindowMac] [[[ destructor ]]] gWindow=%p deleted, window=%p deleted\n", (void*)this, (void*)carbonWindow);
	pPlugin->closeGui();
	if (open)
		DisposeWindow(carbonWindow);
}

#endif

#endif // #ifdef WITH_VST
