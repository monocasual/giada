//-----------------------------------------------------------------------------
// Project     : SDK Core
//
// Category    : SDK Core Interfaces
// Filename    : pluginterfaces/base/ierrorcontext.h
// Created by  : Steinberg, 02/2008
// Description : Error Context Interface
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

#ifndef __ierrorcontext__
#define __ierrorcontext__

#include "pluginterfaces/base/funknown.h"

namespace Steinberg {

class IString;

//------------------------------------------------------------------------
/** Interface for error handling. 
[plug imp] \n
[released: Sequel 2] */
//------------------------------------------------------------------------
class IErrorContext : public FUnknown
{
public:
//------------------------------------------------------------------------
	/** Tells the plug-in to not show any UI elements on errors. */
	virtual void PLUGIN_API disableErrorUI (bool state) = 0;    
	/** If an error happens and disableErrorUI was not set this should return kResultTrue if the plug-in already showed a message to the user what happened. */
	virtual tresult PLUGIN_API errorMessageShown () = 0;
	/** Fill message with error string. The host may show this to the user. */
	virtual tresult PLUGIN_API getErrorMessage (IString* message) = 0;

//------------------------------------------------------------------------
	static const FUID iid;
};
DECLARE_CLASS_IID (IErrorContext, 0x12BCD07B, 0x7C694336, 0xB7DA77C3, 0x444A0CD0)

} // namespace
#endif