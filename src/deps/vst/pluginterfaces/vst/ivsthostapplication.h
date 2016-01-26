//------------------------------------------------------------------------
// Project     : VST SDK
// Version     : 3.6.5
//
// Category    : Interfaces
// Filename    : pluginterfaces/vst/ivsthostapplication.h
// Created by  : Steinberg, 04/2006
// Description : VST Host Interface
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

#include "pluginterfaces/vst/ivstmessage.h"

//------------------------------------------------------------------------
namespace Steinberg {
namespace Vst {
//------------------------------------------------------------------------
/** Basic Host Callback Interface.
\ingroup vstIHost vst300
- [host imp]
- [passed as 'context' in to IPluginBase::initialize () ]

Basic VST host application interface. */
//------------------------------------------------------------------------
class IHostApplication: public FUnknown
{
public:
//------------------------------------------------------------------------
	/** Gets host application name. */
	virtual tresult PLUGIN_API getName (String128 name) = 0;

	/** Creates host object (e.g. Vst::IMessage). */
	virtual tresult PLUGIN_API createInstance (TUID cid, TUID _iid, void** obj) = 0;

//------------------------------------------------------------------------
	static const FUID iid;
};

DECLARE_CLASS_IID (IHostApplication, 0x58E595CC, 0xDB2D4969, 0x8B6AAF8C, 0x36A664E5)

//------------------------------------------------------------------------
inline IMessage* allocateMessage (IHostApplication* host)
{
	TUID iid;
	IMessage::iid.toTUID (iid);
	IMessage* m = 0;
	if (host->createInstance (iid, iid, (void**)&m) == kResultOk)
		return m;
	return 0;
}

//------------------------------------------------------------------------
/** VST 3 to VST 2 Wrapper Interface.
\ingroup vstIHost vst310
- [host imp]
- [passed as 'context' to IPluginBase::initialize () ]

Informs the Plug-in that a VST 3 to VST 2 wrapper is used between the Plug-in and the real host.
Implemented by the VST 2 Wrapper. */
//------------------------------------------------------------------------
class IVst3ToVst2Wrapper: public FUnknown
{
public:
	//------------------------------------------------------------------------
	static const FUID iid;
};

DECLARE_CLASS_IID (IVst3ToVst2Wrapper, 0x29633AEC, 0x1D1C47E2, 0xBB85B97B, 0xD36EAC61)

//------------------------------------------------------------------------
/** VST 3 to AU Wrapper Interface.
\ingroup vstIHost vst310
- [host imp]
- [passed as 'context' to IPluginBase::initialize () ]

Informs the Plug-in that a VST 3 to AU wrapper is used between the Plug-in and the real host.
Implemented by the AU Wrapper. */
//------------------------------------------------------------------------
class IVst3ToAUWrapper: public FUnknown
{
public:
	//------------------------------------------------------------------------
	static const FUID iid;
};

DECLARE_CLASS_IID (IVst3ToAUWrapper, 0xA3B8C6C5, 0xC0954688, 0xB0916F0B, 0xB697AA44)

//------------------------------------------------------------------------
} // namespace Vst
} // namespace Steinberg
