/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_pluginWindowGUI
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

#include "gd_pluginWindowGUI.h"

#if defined(__APPLE__)
static pascal OSStatus windowHandler(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void* inUserData);
#endif


extern PluginHost G_PluginHost;


gdPluginWindowGUI::gdPluginWindowGUI(Plugin *pPlugin)
 : gWindow(450, 300), pPlugin(pPlugin)
{

	/* on OSX VST uses a Carbon window, completely different from this
	 * (type == Cocoa) */

#ifndef __APPLE__
	gu_setFavicon(this);
	set_non_modal();
	resize(x(), y(), pPlugin->getGuiWidth(), pPlugin->getGuiHeight());
	show();
#endif

#if defined(__APPLE__)
	Rect mRect = {0, 0, 300, 300};
	OSStatus err = CreateNewWindow(kDocumentWindowClass, kWindowCloseBoxAttribute | kWindowCompositingAttribute | kWindowAsyncDragAttribute | kWindowStandardHandlerAttribute, &mRect, &window);
	if (err != noErr)	{
		puts("[PluginWindow] Unable to create mac window!");
		return;
	}
	static EventTypeSpec eventTypes[] = {
		{ kEventClassWindow, kEventWindowClose }
	};
	InstallWindowEventHandler(window, windowHandler, GetEventTypeCount (eventTypes), eventTypes, window, NULL);
	pPlugin->openGui((void*)window);
	Rect bounds;
	GetWindowBounds(window, kWindowContentRgn, &bounds);
	bounds.right = bounds.left + pPlugin->getGuiWidth();
	bounds.bottom = bounds.top + pPlugin->getGuiHeight();
	SetWindowBounds(window, kWindowContentRgn, &bounds);
	RepositionWindow(window, NULL, kWindowCenterOnMainScreen);
	ShowWindow(window);
#else

	/* Fl::check(): Waits until "something happens" and then returns. It's
	 * mandatory on linux, otherwise X can't find 'this' window. */

	Fl::check();
	pPlugin->openGui((void*)fl_xid(this));
#endif

	char name[256];
	pPlugin->getProduct(name);
	copy_label(name);

	pPlugin->idle();
}



gdPluginWindowGUI::~gdPluginWindowGUI() {
	pPlugin->closeGui();
#if defined(__APPLE__)
	CFRelease(window);
#endif
}


#if defined(__APPLE__)
pascal OSStatus windowHandler (EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData) {
	OSStatus result = eventNotHandledErr;
	WindowRef window = (WindowRef) inUserData;
	UInt32 eventClass = GetEventClass (inEvent);
	UInt32 eventKind = GetEventKind (inEvent);

	switch (eventClass)	{
		case kEventClassWindow:	{
			switch (eventKind) {
				case kEventWindowClose:	{
					QuitAppModalLoopForWindow (window);
					break;
				}
			}
			break;
		}
	}
	return result;
}
#endif

#endif // #ifdef WITH_VST
