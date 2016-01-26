//-----------------------------------------------------------------------------
// Project     : SDK Core
//
// Category    : SDK Core Interfaces
// Filename    : pluginterfaces/base/keycodes.h
// Created by  : Steinberg, 01/2004
// Description : Key Code Definitions
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
//-----------------------------------------------------------------------------

#ifndef __plugkeycodes__
#define __plugkeycodes__

#include "pluginterfaces/base/ftypes.h"

namespace Steinberg {
//------------------------------------------------------------------------------
/** Virtual Key Codes.
OS-independent enumeration of virtual keycodes.
*/
//------------------------------------------------------------------------------
enum VirtualKeyCodes
{
	KEY_BACK = 1,
	KEY_TAB,
	KEY_CLEAR,
	KEY_RETURN,
	KEY_PAUSE,
	KEY_ESCAPE,
	KEY_SPACE,
	KEY_NEXT,
	KEY_END,
	KEY_HOME,

	KEY_LEFT,
	KEY_UP,
	KEY_RIGHT,
	KEY_DOWN,
	KEY_PAGEUP,
	KEY_PAGEDOWN,

	KEY_SELECT,
	KEY_PRINT,
	KEY_ENTER,
	KEY_SNAPSHOT,
	KEY_INSERT,
	KEY_DELETE,
	KEY_HELP,
	KEY_NUMPAD0,
	KEY_NUMPAD1,
	KEY_NUMPAD2,
	KEY_NUMPAD3,
	KEY_NUMPAD4,
	KEY_NUMPAD5,
	KEY_NUMPAD6,
	KEY_NUMPAD7,
	KEY_NUMPAD8,
	KEY_NUMPAD9,
	KEY_MULTIPLY,
	KEY_ADD,
	KEY_SEPARATOR,
	KEY_SUBTRACT,
	KEY_DECIMAL,
	KEY_DIVIDE,
	KEY_F1,
	KEY_F2,
	KEY_F3,
	KEY_F4,
	KEY_F5,
	KEY_F6,
	KEY_F7,
	KEY_F8,
	KEY_F9,
	KEY_F10,
	KEY_F11,
	KEY_F12,
	KEY_NUMLOCK,
	KEY_SCROLL,

	KEY_SHIFT,
	KEY_CONTROL,
	KEY_ALT,

	KEY_EQUALS,				// only occurs on a Mac
	KEY_CONTEXTMENU,		// Windows only

	// multimedia keys
	KEY_MEDIA_PLAY,
	KEY_MEDIA_STOP,
	KEY_MEDIA_PREV,
	KEY_MEDIA_NEXT,
	KEY_VOLUME_UP,
	KEY_VOLUME_DOWN,

	KEY_F13,
	KEY_F14,
	KEY_F15,
	KEY_F16,
	KEY_F17,
	KEY_F18,
	KEY_F19,

	VKEY_FIRST_CODE = KEY_BACK,
	VKEY_LAST_CODE = KEY_F19,

	VKEY_FIRST_ASCII = 128
	/*
	 KEY_0 - KEY_9 are the same as ASCII '0' - '9' (0x30 - 0x39) + FIRST_ASCII
	 KEY_A - KEY_Z are the same as ASCII 'A' - 'Z' (0x41 - 0x5A) + FIRST_ASCII
	*/
};

//------------------------------------------------------------------------------
inline tchar VirtualKeyCodeToChar (uint8 vKey)
{
	if (vKey >= VKEY_FIRST_ASCII)
		return (tchar)(vKey - VKEY_FIRST_ASCII + 0x30);
	else if (vKey == KEY_SPACE)
		return ' ';
	return 0;
}

//------------------------------------------------------------------------------
inline uint8 CharToVirtualKeyCode (tchar character)
{
	if ((character >= 0x30 && character <= 0x39) || (character >= 0x41 && character <= 0x5A))
		return (uint8)(character - 0x30 + VKEY_FIRST_ASCII);
	if (character == ' ')
		return (uint8)KEY_SPACE;
	return 0;
}

//------------------------------------------------------------------------------
enum KeyModifier
{
	kShiftKey     = 1 << 0, ///< same on both PC and Mac
	kAlternateKey = 1 << 1, ///< same on both PC and Mac
	kCommandKey   = 1 << 2, ///< windows ctrl key; mac cmd key (apple button)
	kControlKey   = 1 << 3  ///< windows: not assigned, mac: ctrl key
};
}

#endif	// __plugkeycodes__
