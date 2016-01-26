//-----------------------------------------------------------------------------
// Project     : SDK Core
//
// Category    : SDK Core Interfaces
// Filename    : pluginterfaces/base/istringresult.h
// Created by  : Steinberg, 01/2005
// Description : Strings Interface
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

#ifndef __istringresult__
#define __istringresult__

#include "pluginterfaces/base/funknown.h"

namespace Steinberg {

//------------------------------------------------------------------------
/** Interface to return an ascii string of variable size. 
    In order to manage memory allocation and deallocation properly, 
	this interface is used to transfer a string as result parameter of
	a method requires a string of unknown size. 
[host imp] or [plug imp] \n
[released: SX 4] */
//------------------------------------------------------------------------
class IStringResult : public FUnknown
{
public:
//------------------------------------------------------------------------
	virtual void PLUGIN_API setText (const char8* text) = 0;    

//------------------------------------------------------------------------
	static const FUID iid;
};

DECLARE_CLASS_IID (IStringResult, 0x550798BC, 0x872049DB, 0x84920A15, 0x3B50B7A8)


//------------------------------------------------------------------------
/** Interface to a string of variable size and encoding. 
[host imp] or [plug imp] \n
[released: ] */
//------------------------------------------------------------------------
class IString : public FUnknown
{
public:
//------------------------------------------------------------------------
	/** Assign ASCII string */
	virtual void PLUGIN_API setText8 (const char8* text) = 0;    
	/** Assign unicode string */
	virtual void PLUGIN_API setText16 (const char16* text) = 0;

	/** Return ASCII string. If the string is unicode so far, it will be converted.
	    So you need to be careful, because the conversion can result in data loss. 
		It is save though to call getText8 if isWideString() returns false */
	virtual const char8* PLUGIN_API getText8 () = 0;   
	/** Return unicode string. If the string is ASCII so far, it will be converted. */
	virtual const char16* PLUGIN_API getText16 () = 0;    

	/** !Do not use this method! Early implementations take the given pointer as 
	     internal string and this will cause problems because 'free' will be used to delete the passed memory.
		 Later implementations will redirect 'take' to setText8 and setText16 */
	virtual void PLUGIN_API take (void* s, bool isWide) = 0;

	/** Returns true if the string is in unicode format, returns false if the string is ASCII */
	virtual bool PLUGIN_API isWideString () const = 0;

//------------------------------------------------------------------------
	static const FUID iid;
};

DECLARE_CLASS_IID (IString, 0xF99DB7A3, 0x0FC14821, 0x800B0CF9, 0x8E348EDF)

} // namespace
#endif
