//-----------------------------------------------------------------------------
// Project     : SDK Core
//
// Category    : SDK GUI Interfaces
// Filename    : pluginterfaces/gui/iplugview.h
// Created by  : Steinberg, 12/2007
// Description : Plug-in User Interface
//
//-----------------------------------------------------------------------------
// LICENSE
// (c) 2015, Steinberg Media Technologies GmbH, All Rights Reserved
//-----------------------------------------------------------------------------
// This Software Development Kit may not be distributed in parts or its entirety
// without prior written agreement by Steinberg Media Technologies GmbH.
// This SDK must not be used to re-engineer or manipulate any technology used
// in any Steinberg or Third-party application or software module,
// unless permitted by law.
// Neither the name of the Steinberg Media Technologies nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
//
// THIS SDK IS PROVIDED BY STEINBERG MEDIA TECHNOLOGIES GMBH "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL STEINBERG MEDIA TECHNOLOGIES GMBH BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//------------------------------------------------------------------------------

#ifndef __iplugview__
#define __iplugview__

#include "../base/funknown.h"

namespace Steinberg {

class IPlugFrame;

//------------------------------------------------------------------------
/*! \defgroup pluginGUI Graphical User Interface
*/

//------------------------------------------------------------------------
/**  Graphical rectangle structure. Used with IPlugView.
\ingroup pluginGUI
*/
//------------------------------------------------------------------------
struct ViewRect
{
//------------------------------------------------------------------------
	ViewRect (int32 l = 0, int32 t = 0, int32 r = 0, int32 b = 0)
	: left (l), top (t), right (r), bottom (b) {}

	int32 left;
	int32 top;
	int32 right;
	int32 bottom;
//------------------------------------------------------------------------
	int32 getWidth () const { return right - left; }
	int32 getHeight () const { return bottom - top; }
//------------------------------------------------------------------------
};

//------------------------------------------------------------------------
/**  \defgroup platformUIType Platform UI Types
\ingroup pluginGUI
List of Platform UI types for IPlugView. This list is used to match the GUI-System between
the host and a Plug-in in case that an OS provides multiple GUI-APIs.
*/
/*@{*/
/** The parent parameter in IPlugView::attached() is a HWND handle. You should attach a child window to it. */
const FIDString kPlatformTypeHWND = "HWND";			///< HWND handle. (Microsoft Windows)

/** The parent parameter in IPlugView::attached() is a WindowRef. You should attach a HIViewRef to the content view of the window. */
const FIDString kPlatformTypeHIView = "HIView";		///< HIViewRef. (Mac OS X)

/** The parent parameter in IPlugView::attached() is a NSView pointer. You should attach a NSView to it. */
const FIDString kPlatformTypeNSView = "NSView";		///< NSView pointer. (Mac OS X)

/** The parent parameter in IPlugView::attached() is a UIView pointer. You should attach an UIView to it. */
const FIDString kPlatformTypeUIView = "UIView";		///< UIView pointer. (iOS)
/*@}*/
//------------------------------------------------------------------------

//------------------------------------------------------------------------
/**  Plug-in definition of a view.
\ingroup pluginGUI
- [plug imp]

\par Sizing of a view
Usually the size of a Plug-in view is fixed. But both the host and the Plug-in can cause
a view to be resized:
\n
- <b> Host </b> : If IPlugView::canResize() returns kResultTrue the host will setup the window
  so that the user can resize it. While the user resizes the window IPlugView::checkSizeConstraint ()
  is called, allowing the Plug-in to change the size to a valid rect. The host then resizes the window
  to this rect and calls IPlugView::onSize().
  \n \n
- <b> Plug-in </b> : The Plug-in can call IPlugFrame::resizeView () and cause the host to resize the window.
  Afterwards IPlugView::onSize() is called.
.
\n
Please only resize the platform representation of the view when IPlugView::onSize() is called.

\par Keyboard handling
The Plug-in view receives keyboard events from the host. A view implementation must not handle keyboard
events by the means of platform callbacks, but let the host pass them to the view. The host depends on
a proper return value when IPlugView::onKeyDown is called, otherwise the Plug-in view may cause a disfunction of the
host's key command handling!

\see IPlugFrame, \ref platformUIType
*/
//------------------------------------------------------------------------
class IPlugView: public FUnknown
{
public:
//------------------------------------------------------------------------
	/** Is Platform UI Type supported
		\param type : IDString of \ref platformUIType */
	virtual tresult PLUGIN_API isPlatformTypeSupported (FIDString type) = 0;

	/** The parent window of the view has been created, the
	    (platform) representation of the view should now be created as well.
		Note that the parent is owned by the caller and you are not allowed to alter it in any way other than adding your own views.
		\param parent : platform handle of the parent window or view
		\param type : \ref platformUIType which should be created */
	virtual tresult PLUGIN_API attached (void* parent, FIDString type) = 0;

	/** The parent window of the view is about to be destroyed.
		You have to remove all your own views from the parent window or view. */
	virtual tresult PLUGIN_API removed () = 0;

	/** Handling of mouse wheel. */
	virtual tresult PLUGIN_API onWheel (float distance) = 0;

	/** Handling of keyboard events : Key Down.
	    \param key : unicode code of key
		\param keyCode : virtual keycode for non ascii keys - see \ref VirtualKeyCodes in keycodes.h
		\param modifiers : any combination of modifiers - see \ref KeyModifier in keycodes.h
		\return kResultTrue if the key is handled, otherwise kResultFalse. \n
		        <b> Please note that kResultTrue must only be returned if the key has really been handled. </b>
		        Otherwise key command handling of the host might be blocked! */
	virtual tresult PLUGIN_API onKeyDown (char16 key, int16 keyCode, int16 modifiers) = 0;

	/** Handling of keyboard events : Key Up.
	    \param key : unicode code of key
		\param keyCode : virtual keycode for non ascii keys - see \ref VirtualKeyCodes in keycodes.h
		\param modifiers : any combination of KeyModifier - see \ref KeyModifier in keycodes.h
		\return kResultTrue if the key is handled, otherwise return kResultFalse. */
	virtual tresult PLUGIN_API onKeyUp (char16 key, int16 keyCode, int16 modifiers) = 0;

	/** Returns the size of the platform representation of the view. */
	virtual tresult PLUGIN_API getSize (ViewRect* size) = 0;

	/** Resizes the platform representation of the view to the given rect. */
	virtual tresult PLUGIN_API onSize (ViewRect* newSize) = 0;

	/** Focus changed message. */
	virtual tresult PLUGIN_API onFocus (TBool state) = 0;

	/** Sets IPlugFrame object to allow the Plug-in to inform the host about resizing. */
	virtual tresult PLUGIN_API setFrame (IPlugFrame* frame) = 0;

	/** Is view sizable by user. */
	virtual tresult PLUGIN_API canResize () = 0;

	/** On live resize this is called to check if the view can be resized to the given rect, if not adjust the rect to the allowed size. */
	virtual tresult PLUGIN_API checkSizeConstraint (ViewRect* rect) = 0;
//------------------------------------------------------------------------
	static const FUID iid;
};

DECLARE_CLASS_IID (IPlugView, 0x5BC32507, 0xD06049EA, 0xA6151B52, 0x2B755B29)

//------------------------------------------------------------------------
/** Callback interface passed to IPlugView.
\ingroup pluginGUI
- [host imp]

Enables a Plug-in to resize the view and cause the host to resize the window.
*/
//------------------------------------------------------------------------
class IPlugFrame: public FUnknown
{
public:
//------------------------------------------------------------------------
	/** Called to inform the host about the resize of a given view. */
	virtual tresult PLUGIN_API resizeView (IPlugView* view, ViewRect* newSize) = 0;
//------------------------------------------------------------------------
	static const FUID iid;
};

DECLARE_CLASS_IID (IPlugFrame, 0x367FAF01, 0xAFA94693, 0x8D4DA2A0, 0xED0882A3)
} // namespace Steinberg

#endif
