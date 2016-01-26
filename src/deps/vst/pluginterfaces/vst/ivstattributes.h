//------------------------------------------------------------------------
// Project     : VST SDK
// Version     : 3.6.5
//
// Category    : Interfaces
// Filename    : pluginterfaces/vst/ivstattributes.h
// Created by  : Steinberg, 05/2006
// Description : VST Attribute Interfaces
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

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/vsttypes.h"

//------------------------------------------------------------------------
#include "pluginterfaces/base/falignpush.h"
//------------------------------------------------------------------------

//------------------------------------------------------------------------
namespace Steinberg {
namespace Vst {
//------------------------------------------------------------------------
/** Attribute list used in IMessage and IStreamAttributes.
\ingroup vstIHost vst300
- [host imp]
- [released: 3.0.0]

An attribute list associates values with a key (id: some predefined keys could be found in \ref presetAttributes). */
//------------------------------------------------------------------------
class IAttributeList: public FUnknown
{
public:
//------------------------------------------------------------------------
	typedef const char* AttrID;

	/** Sets integer value. */
	virtual tresult PLUGIN_API setInt (AttrID id, int64 value) = 0;

	/** Gets integer value. */
	virtual tresult PLUGIN_API getInt (AttrID id, int64& value) = 0;

	/** Sets float value. */
	virtual tresult PLUGIN_API setFloat (AttrID id, double value) = 0;

	/** Gets float value. */
	virtual tresult PLUGIN_API getFloat (AttrID id, double& value) = 0;

	/** Sets string value (UTF16). */
	virtual tresult PLUGIN_API setString (AttrID id, const TChar* string) = 0;

	/** Gets string value (UTF16). Note that Size is in Byte, not the string Length! (Do not forget to multiply the length by sizeof (TChar)!) */
	virtual tresult PLUGIN_API getString (AttrID id, TChar* string, uint32 size) = 0;

	/** Sets binary data. */
	virtual tresult PLUGIN_API setBinary (AttrID id, const void* data, uint32 size) = 0;

	/** Gets binary data. */
	virtual tresult PLUGIN_API getBinary (AttrID id, const void*& data, uint32& size) = 0;
//------------------------------------------------------------------------
	static const FUID iid;
};

DECLARE_CLASS_IID (IAttributeList, 0x1E5F0AEB, 0xCC7F4533, 0xA2544011, 0x38AD5EE4)

//------------------------------------------------------------------------
/**  Meta attributes of a stream.
\ingroup vstIHost  vst360
- [host imp]
- [extends IBStream]
- [released: 3.6.0]

\code
...
#include "pluginterfaces/base/ustring.h"
#include "pluginterfaces/vst/vstpresetkeys.h"
...

tresult PLUGIN_API MyPlugin::setState (IBStream* state)
{
	FUnknownPtr<IStreamAttributes> stream (state);
	if (stream)
	{
		IAttributeList* list = stream->getAttributes ();
		if (list)
		{
			// get the current type (project/Default..) of this state
			String128 string;
			if (list->getString (PresetAttributes::kStateType, string, 128 * sizeof (TChar)) == kResultTrue)
			{
				UString128 tmp (string);
				char ascii[128];
				tmp.toAscii (ascii, 128);
				if (!strncmp (ascii, StateType::kProject, strlen (StateType::kProject)))
				{
					// we are in project loading context...
				}
			}

			// get the full file path of this state
			TChar fullPath[1024];
			if (list->getString (PresetAttributes::kFilePathStringType, fullPath, 1024 * sizeof (TChar)) == kResultTrue)
			{
				// here we have the full path ...
			}
		}
	}

	//...read the state here.....
	return kResultTrue;
}
\endcode
Interface to access preset meta information from stream, used for example in setState in order to inform the plug-in about 
the current context in which this preset loading occurs (Project context or Preset load (see \ref StateType)) 
or used to get the full file path of the loaded preset (if available). */
//------------------------------------------------------------------------
class IStreamAttributes: public FUnknown
{
public:
	//------------------------------------------------------------------------
	/** Gets filename (without file extension) of the stream. */
	virtual tresult PLUGIN_API getFileName (String128 name) = 0;

	/** Gets meta information list. */
	virtual IAttributeList* PLUGIN_API getAttributes () = 0;
	//------------------------------------------------------------------------
	static const FUID iid;
};

DECLARE_CLASS_IID (IStreamAttributes, 0xD6CE2FFC, 0xEFAF4B8C, 0x9E74F1BB, 0x12DA44B4)


//------------------------------------------------------------------------
} // namespace Vst
} // namespace Steinberg

//------------------------------------------------------------------------
#include "pluginterfaces/base/falignpop.h"
//------------------------------------------------------------------------
