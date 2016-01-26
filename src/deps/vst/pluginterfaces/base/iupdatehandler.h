//------------------------------------------------------------------------
// Project     : Steinberg Module Architecture SDK
//
// Category    : Basic Host Service Interfaces
// Filename    : pluginterfaces/base/iupdatehandler.h
// Created by  : Steinberg, 01/2004
// Description : Update handling
//
//-----------------------------------------------------------------------------
// LICENSE
// (c) 2014, Steinberg Media Technologies GmbH, All Rights Reserved
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

#ifndef __iupdatehandler__
#define __iupdatehandler__

#include "pluginterfaces/base/funknown.h"

namespace Steinberg {

class IDependent;

//------------------------------------------------------------------------
/** Host implements dependency handling for plugins.
[host imp] \n
[get this interface from IHostClasses] \n
[released N3.1] \n

- Install/Remove change notifications
- Trigger updates when an object has changed

Can be used between host-objects and the Plug-In or 
inside the Plug-In to handle internal updates!

\see IDependent
\ingroup frameworkHostClasses
*/
//------------------------------------------------------------------------
class IUpdateHandler: public FUnknown
{
public:
//------------------------------------------------------------------------
	/** Install update notification for given object. It is essential to
	    remove all dependencies again using 'removeDependent'! Dependencies
		are not removed automatically when the 'object' is released! 
	\param object : interface to object that sends change notifications 
	\param dependent : interface through which the update is passed */
	virtual tresult PLUGIN_API addDependent (FUnknown* object, IDependent* dependent) = 0;
	
	/** Remove a previously installed dependency.*/
	virtual tresult PLUGIN_API removeDependent (FUnknown* object, IDependent* dependent) = 0;

	/** Inform all dependents, that object has changed. 
	\param object is the object that has changed
	\param message is a value of enum IDependent::ChangeMessage, usually  IDependent::kChanged - can be
	                 a private message as well (only known to sender and dependent)*/
	virtual	tresult PLUGIN_API triggerUpdates (FUnknown* object, int32 message) = 0;

	/** Same as triggerUpdates, but delivered in idle (usefull to collect updates).*/
	virtual	tresult PLUGIN_API deferUpdates (FUnknown* object, int32 message) = 0;
	static const FUID iid;
};

DECLARE_CLASS_IID (IUpdateHandler, 0xF5246D56, 0x86544d60, 0xB026AFB5, 0x7B697B37)

//------------------------------------------------------------------------
/**  A dependent will get notified about changes of a model.
[plug imp]
- notify changes of a model

\see IUpdateHandler
\ingroup frameworkHostClasses
*/
//------------------------------------------------------------------------
class IDependent: public FUnknown
{
public:
//------------------------------------------------------------------------
	/** Inform the dependent, that the passed FUnknown has changed. */
	virtual void PLUGIN_API update (FUnknown* changedUnknown, int32 message) = 0;

	enum ChangeMessage 
	{
		kWillChange,
		kChanged,
		kDestroyed,
		kWillDestroy,

		kStdChangeMessageLast = kWillDestroy
	};
	//------------------------------------------------------------------------
	static const FUID iid;
};

DECLARE_CLASS_IID (IDependent, 0xF52B7AAE, 0xDE72416d, 0x8AF18ACE, 0x9DD7BD5E)

}

#endif
