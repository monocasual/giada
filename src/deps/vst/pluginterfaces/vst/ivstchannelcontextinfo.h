//------------------------------------------------------------------------
// Project     : VST SDK
// Version     : 3.6.5
//
// Category    : Interfaces
// Filename    : pluginterfaces/vst/ivstchannelcontextinfo.h
// Created by  : Steinberg, 02/2014
// Description : VST Channel Context Info Interface
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
namespace ChannelContext {

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

//------------------------------------------------------------------------
//------------------------------------------------------------------------
/** Channel Context Interface. 
\ingroup vstIHost vst365
- [plug imp]
- [extends IEditController]
- [released: 3.6.5]
- [optional]

Allows the host to inform the Plug-in about the context in which the Plug-in is instantiated,
mainly channel based info (color, name, ...). As soon as the Plug-in provides this IInfoListener
interface, the host will call setChannelContextInfos for each change occurring to this channel
(new name, new color, new indexation,...)

\note Example:
\verbatim
tresult PLUGIN_API MyPlugin::setChannelContextInfos (IAttributeList* list)
{
	if (list)
	{
		// optional we can ask for the Channel Name Length
		int64 length;
		if (list->getInt (ChannelContext::kChannelNameLengthKey, length) == kResultTrue)
		{
			...
		}
		
		// get the Channel Name where we, as Plug-in, are instantiated
		String128 name;
		if (list->getString (ChannelContext::kChannelNameKey, name, sizeof (name)) == kResultTrue)
		{
			...
		}

		// get the Channel UID
		if (list->getString (ChannelContext::kChannelUIDKey, name, sizeof (name)) == kResultTrue)
		{
			...
		}
		
		// get Channel Index
		int64 index;
		if (list->getInt (ChannelContext::kChannelIndexKey, index) == kResultTrue)
		{
			...
		}
		
		// get the Channel Color
		int64 color;
		if (list->getInt (ChannelContext::kChannelColorKey, color) == kResultTrue)
		{
			ColorSpec channelColor = (ColorSpec)color;
			...
		}

		// do not forget to call addRef () if you want to keep this list
	}
}
\endverbatim
\see \ref */
//------------------------------------------------------------------------
class IInfoListener: public FUnknown
{
public:
	/** Receive the channel context infos from host. */
	virtual tresult PLUGIN_API setChannelContextInfos (IAttributeList* list) = 0;

	static const FUID iid;
};

DECLARE_CLASS_IID (IInfoListener, 0x0F194781, 0x8D984ADA, 0xBBA0C1EF, 0xC011D8D0)


//------------------------------------------------------------------------
/** Values used for kChannelPluginLocationKey */
//------------------------------------------------------------------------
enum ChannelPluginLocation
{
	kPreVolumeFader = 0,
	kPostVolumeFader,
	kUsedAsPanner
};

//------------------------------------------------------------------------
//------------------------------------------------------------------------
// Colors		
typedef uint32 ColorSpec;	///< ARGB (Alpha-Red-Green-Blue)
typedef uint8 ColorComponent;

inline ColorComponent GetBlue (ColorSpec cs)	{return (ColorComponent)(cs & 0x000000FF); }
inline ColorComponent GetGreen (ColorSpec cs)	{return (ColorComponent)((cs >> 8) & 0x000000FF); }
inline ColorComponent GetRed (ColorSpec cs)		{return (ColorComponent)((cs >> 16) & 0x000000FF); }
inline ColorComponent GetAlpha (ColorSpec cs)	{return (ColorComponent)((cs >> 24) & 0x000000FF); }

//------------------------------------------------------------------------
/** Keys used as AttrID (Attribute ID) in the return IAttributeList of
 * IInfoListener::setChannelContextInfos  */
//------------------------------------------------------------------------
const CString kChannelUIDKey = "channel uid"; ///< string (TChar) [optional]
const CString kChannelUIDLengthKey = "channel uid length"; ///< integer (int64) [optional] number of characters in kChannelUIDKey

const CString kChannelNameKey = "channel name"; ///< string (TChar) [optional]
const CString kChannelNameLengthKey = "channel name length"; ///< integer (int64) [optional] number of characters in kChannelNameKey

const CString kChannelColorKey = "channel color"; ///< color (ColorSpec) [optional]

const CString kChannelIndexKey = "channel index"; ///< integer (int64) [optional] start with 1
const CString kChannelIndexNamespaceOrderKey = "channel index namespace order"; ///< integer (int64) [optional] start with 1
const CString kChannelIndexNamespaceKey = "channel index namespace"; ///< string (TChar) [optional]
const CString kChannelIndexNamespaceLengthKey =	"channel index namespace length"; ///< integer (int64) [optional] number of characters in
																				  ///kChannelIndexNamespaceKey

const CString kChannelImageKey = "channel image"; ///< PNG image representation as binary [optional]
const CString kChannelPluginLocationKey = "channel plugin location"; ///< integer (int64) [optional] see ChannelPluginLocation

//------------------------------------------------------------------------

} // namespace ChannelContext
} // namespace Vst
} // namespace Steinberg

//------------------------------------------------------------------------
#include "pluginterfaces/base/falignpop.h"
//------------------------------------------------------------------------
