//------------------------------------------------------------------------
// Project     : VST SDK
// Version     : 3.6.5
//
// Category    : Interfaces
// Filename    : pluginterfaces/vst/ivstmessage.h
// Created by  : Steinberg, 04/2005
// Description : VST Message Interfaces
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
//----------------------------------------------------------------------------------

#pragma once

#include "pluginterfaces/vst/ivstattributes.h"

//------------------------------------------------------------------------
#include "pluginterfaces/base/falignpush.h"
//------------------------------------------------------------------------

//------------------------------------------------------------------------
namespace Steinberg {
namespace Vst {
//------------------------------------------------------------------------
/** Private Plug-in message.
\ingroup vstIHost vst300
- [host imp]
- [create via IHostApplication::createInstance]
- [released: 3.0.0]

Messages are sent from a VST-controller component to a VST-editor component and vice versa.
\see IAttributeList, IConnectionPoint, \ref vst3Communication */
//------------------------------------------------------------------------
class IMessage: public FUnknown
{
public:
//------------------------------------------------------------------------
	/** Returns the message ID (for example "TextMessage"). */
	virtual FIDString PLUGIN_API getMessageID () = 0;

	/** Sets a message ID (for example "TextMessage"). */
	virtual void PLUGIN_API setMessageID (FIDString id /*in*/) = 0;

	/** Returns the attribute list associated to the message. */
	virtual IAttributeList* PLUGIN_API getAttributes () = 0;

//------------------------------------------------------------------------
	static const FUID iid;
};

DECLARE_CLASS_IID (IMessage, 0x936F033B, 0xC6C047DB, 0xBB0882F8, 0x13C1E613)

//------------------------------------------------------------------------
/** Connect a component with another one.
\ingroup vstIPlug vst300
- [plug imp]
- [host imp]
- [released: 3.0.0]

This interface is used for the communication of separate components.
Note that some hosts will place a proxy object between the components so that they are not directly connected.

\see \ref vst3Communication*/
//------------------------------------------------------------------------
class IConnectionPoint: public FUnknown
{
public:
//------------------------------------------------------------------------
	/** Connects this instance with another connection point. */
	virtual tresult PLUGIN_API connect (IConnectionPoint* other) = 0;

	/** Disconnects a given connection point from this. */
	virtual tresult PLUGIN_API disconnect (IConnectionPoint* other) = 0;

	/** Called when a message has been sent from the connection point to this. */
	virtual tresult PLUGIN_API notify (IMessage* message) = 0;

//------------------------------------------------------------------------
	static const FUID iid;
};

DECLARE_CLASS_IID (IConnectionPoint, 0x70A4156F, 0x6E6E4026, 0x989148BF, 0xAA60D8D1)

//------------------------------------------------------------------------
} // namespace Vst
} // namespace Steinberg

//------------------------------------------------------------------------
#include "pluginterfaces/base/falignpop.h"
//------------------------------------------------------------------------
