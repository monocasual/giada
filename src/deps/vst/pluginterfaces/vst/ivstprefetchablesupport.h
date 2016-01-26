//------------------------------------------------------------------------
// Project     : VST SDK
// Version     : 3.6.5
//
// Category    : Interfaces
// Filename    : pluginterfaces/vst/ivstprefetchablesupport.h
// Created by  : Steinberg, 02/2015
// Description : VST Prefetchable Support Interface
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

#include "pluginterfaces/vst/vsttypes.h"
#include "pluginterfaces/vst/ivstattributes.h"

//------------------------------------------------------------------------
#include "pluginterfaces/base/falignpush.h"
//------------------------------------------------------------------------

//------------------------------------------------------------------------
namespace Steinberg {
namespace Vst {

typedef uint32 PrefetchableSupport;
enum ePrefetchableSupport
{
	kIsNeverPrefetchable = 0,	///< every instance of the plug does not support prefetch processing
	kIsYetPrefetchable,			///< in the current state the plug support prefetch processing
	kIsNotYetPrefetchable,		///< in the current state the plug does not support prefetch processing
	kNumPrefetchableSupport
};

//------------------------------------------------------------------------
// IPrefetchableSupport Interface
//------------------------------------------------------------------------
/** Indicates that the Plug-in do not support Prefetch.
\ingroup vstIPlug vst365
- [plug imp]
- [extends IComponent]
- [released: 3.6.5]
- [optional]
*/
class IPrefetchableSupport : public FUnknown
{
public:
	//------------------------------------------------------------------------
	/** retrieve the current prefetch support. Use IComponentHandler::restartComponent (kPrefetchableSupportChanged)
		to inform the host that this support has changed. */
	virtual tresult PLUGIN_API getPrefetchableSupport (PrefetchableSupport& prefetchable /*out*/) = 0;

	//------------------------------------------------------------------------
	static const FUID iid;
};

DECLARE_CLASS_IID (IPrefetchableSupport, 0x8AE54FDA, 0xE93046B9, 0xA28555BC, 0xDC98E21E)

} // namespace Vst
} // namespace Steinberg

//------------------------------------------------------------------------
#include "pluginterfaces/base/falignpop.h"
//------------------------------------------------------------------------
