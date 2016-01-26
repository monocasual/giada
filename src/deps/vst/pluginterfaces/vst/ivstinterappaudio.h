//------------------------------------------------------------------------
// Project     : VST SDK
// Version     : 3.6.5
//
// Category    : Interfaces
// Filename    : pluginterfaces/vst/ivstinterappaudio.h
// Created by  : Steinberg, 08/2013
// Description : VST InterAppAudio Interfaces
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

//------------------------------------------------------------------------
namespace Steinberg {
struct ViewRect;
namespace Vst {
struct Event;
class IInterAppAudioPresetManager;

//------------------------------------------------------------------------
/** Inter-App Audio host Interface.
\ingroup vstIHost vst360
- [host imp]
- [passed as 'context' to IPluginBase::initialize () ]
- [released: 3.6.0]

Implemented by the InterAppAudio Wrapper. */
//------------------------------------------------------------------------
class IInterAppAudioHost: public FUnknown
{
public:
	/** get the size of the screen
	 *	@param size size of the screen
	 *	@param scale scale of the screen
	 *	@return kResultTrue on success
	 */
	virtual tresult PLUGIN_API getScreenSize (ViewRect* size, float* scale) = 0;

	/** get status of connection
	 *	@return kResultTrue if an Inter-App Audio connection is established
	 */
	virtual tresult PLUGIN_API connectedToHost () = 0;

	/** switch to the host.
	 *	@return kResultTrue on success
	 */
	virtual tresult PLUGIN_API switchToHost () = 0;

	/** send a remote control event to the host
	 *	@param event event type, see AudioUnitRemoteControlEvent in the iOS SDK documentation for possible types
	 *	@return kResultTrue on success
	 */
	virtual tresult PLUGIN_API sendRemoteControlEvent (uint32 event) = 0;

	/** ask for the host icon.
	 *	@param icon pointer to a CGImageRef
	 *	@return kResultTrue on success
	 */
	virtual tresult PLUGIN_API getHostIcon (void** icon) = 0;

	/** schedule an event from the user interface thread
	 *	@param event the event to schedule
	 *	@return kResultTrue on success
	 */
	virtual tresult PLUGIN_API scheduleEventFromUI (Event& event) = 0;

	/** get the preset manager
	 *	@param cid class ID to use by the preset manager
	 *	@return the preset manager. Needs to be released by called.
	 */
	virtual IInterAppAudioPresetManager* PLUGIN_API createPresetManager (const TUID& cid) = 0;

	/** show the settings view
	 *	currently includes MIDI settings and Tempo setting
	 *	@return kResultTrue on success
	 */
	virtual tresult PLUGIN_API showSettingsView () = 0;

	//------------------------------------------------------------------------
	static const FUID iid;
};

DECLARE_CLASS_IID (IInterAppAudioHost, 0x0CE5743D, 0x68DF415E, 0xAE285BD4, 0xE2CDC8FD)

//------------------------------------------------------------------------
/** Extended IEditController interface for Inter-App Audio connection state change notifications
\ingroup vstIPlug vst360
- [plug imp]
- [extends IEditController]
- [released: 3.6.0]
*/
//------------------------------------------------------------------------
class IInterAppAudioConnectionNotification : public FUnknown
{
public:
	/** called when the Inter-App Audio connection state changes
	 *	@param newState true if an Inter-App Audio connection is established, otherwise false
	*/
	virtual void PLUGIN_API onInterAppAudioConnectionStateChange (TBool newState) = 0;

	//------------------------------------------------------------------------	
	static const FUID iid;
};

DECLARE_CLASS_IID (IInterAppAudioConnectionNotification, 0x6020C72D, 0x5FC24AA1, 0xB0950DB5, 0xD7D6D5CF)

//------------------------------------------------------------------------
/** Extended IEditController interface for Inter-App Audio Preset Management
\ingroup vstIPlug vst360
- [plug imp]
- [extends IEditController]
- [released: 3.6.0]
*/
//------------------------------------------------------------------------
class IInterAppAudioPresetManager : public FUnknown
{
public:
	/** TODO */
	virtual tresult PLUGIN_API runLoadPresetBrowser () = 0;
	/** TODO */
	virtual tresult PLUGIN_API runSavePresetBrowser () = 0;
	/** TODO */
	virtual tresult PLUGIN_API loadNextPreset () = 0;
	/** TODO */
	virtual tresult PLUGIN_API loadPreviousPreset () = 0;

	//------------------------------------------------------------------------	
	static const FUID iid;
};

DECLARE_CLASS_IID (IInterAppAudioPresetManager, 0xADE6FCC4, 0x46C94E1D, 0xB3B49A80, 0xC93FEFDD)

//------------------------------------------------------------------------
} // namespace Vst
} // namespace Steinberg
