//-----------------------------------------------------------------------------
// Project     : SDK Core
//
// Category    : SDK Core Interfaces
// Filename    : pluginterfaces/base/funknown.h
// Created by  : Steinberg, 01/2004
// Description : Basic Interface
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

#pragma once

#include "pluginterfaces/base/fplatform.h"
#if SMTG_CPP11_STDLIBSUPPORT
#include <memory>
#endif

//------------------------------------------------------------------------
namespace Steinberg {

//------------------------------------------------------------------------
// IPtr
//------------------------------------------------------------------------
/**	IPtr - Smart pointer template class.
 \ingroup pluginBase

 - can be used as an I* pointer
 - handles refCount of the interface
 - Usage example:
 \code
	IPtr<IPath> path (sharedPath);
	if (path)
		path->ascend ();
 \endcode
 */
//------------------------------------------------------------------------
template <class I>
class IPtr
{
public:
//------------------------------------------------------------------------
	inline IPtr (I* ptr, bool addRef = true);
	inline IPtr (const IPtr&);

	template <class T>
	inline IPtr (const IPtr<T>& other)
	: ptr (other.get ())
	{
		if (ptr)
			ptr->addRef ();
	}

	inline IPtr ();
	inline ~IPtr ();

	inline I* operator= (I* ptr);

	inline IPtr& operator= (const IPtr& other);

	template <class T>
	inline IPtr& operator= (const IPtr<T>& other)
	{
		operator= (other.get ());
		return *this;
	}

	inline operator I* () const { return ptr; } // act as I*
	inline I* operator-> () const { return ptr; } // act as I*

	inline I* get () const { return ptr; }

#if SMTG_CPP11_STDLIBSUPPORT
	inline IPtr (IPtr<I>&& movePtr) : ptr (nullptr) { *this = std::move (movePtr); }
	inline IPtr& operator= (IPtr<I>&& movePtr)
	{
		if (ptr)
			ptr->release ();
		ptr = movePtr.ptr;
		movePtr.ptr = nullptr;
		return *this;
	}
#endif
//------------------------------------------------------------------------
protected:
	I* ptr;
};

//------------------------------------------------------------------------
template <class I>
inline IPtr<I>::IPtr (I* _ptr, bool addRef)
: ptr (_ptr)
{
	if (ptr && addRef)
		ptr->addRef ();
}

//------------------------------------------------------------------------
template <class I>
inline IPtr<I>::IPtr (const IPtr<I>& other)
: ptr (other.ptr)
{
	if (ptr)
		ptr->addRef ();
}

//------------------------------------------------------------------------
template <class I>
inline IPtr<I>::IPtr ()
: ptr (0)
{
}

//------------------------------------------------------------------------
template <class I>
inline IPtr<I>::~IPtr ()
{
	if (ptr)
		ptr->release ();
}

//------------------------------------------------------------------------
template <class I>
inline I* IPtr<I>::operator= (I* _ptr)
{
	if (_ptr != ptr)
	{
		if (ptr)
			ptr->release ();
		ptr = _ptr;
		if (ptr)
			ptr->addRef ();
	}
	return ptr;
}

//------------------------------------------------------------------------
template <class I>
inline IPtr<I>& IPtr<I>::operator= (const IPtr<I>& _ptr)
{
	operator= (_ptr.ptr);
	return *this;
}

//------------------------------------------------------------------------
/** OPtr - "owning" smart pointer used for newly created FObjects.
 \ingroup pluginBase

 FUnknown implementations are supposed to have a refCount of 1 right after creation.
 So using an IPtr on newly created objects would lead to a leak.
 Instead the OPtr can be used in this case. \n
 Example:
 \code
 OPtr<IPath> path = FHostCreate (IPath, hostClasses);
 // no release is needed...
 \endcode
 The assignment operator takes ownership of a new object and releases the old.
 So its safe to write:
 \code
 OPtr<IPath> path = FHostCreate (IPath, hostClasses);
 path = FHostCreate (IPath, hostClasses);
 path = 0;
 \endcode
 This is the difference to using an IPtr with addRef=false.
 \code
 // DONT DO THIS:
 IPtr<IPath> path (FHostCreate (IPath, hostClasses), false);
 path = FHostCreate (IPath, hostClasses);
 path = 0;
 \endcode
 This will lead to a leak!
 */
//------------------------------------------------------------------------
template <class I>
class OPtr : public IPtr<I>
{
public:
//------------------------------------------------------------------------
	inline OPtr (I* p) : IPtr<I> (p, false) {}
	inline OPtr (const IPtr<I>& p) : IPtr<I> (p) {}
	inline OPtr (const OPtr<I>& p) : IPtr<I> (p) {}
	inline OPtr () {}
	inline I* operator= (I* _ptr)
	{
		if (_ptr != this->ptr)
		{
			if (this->ptr)
				this->ptr->release ();
			this->ptr = _ptr;
		}
		return this->ptr;
	}
};

//------------------------------------------------------------------------
/** Assigning newly created object to an IPtr.
 Example:
 \code
 IPtr<IPath> path = owned (FHostCreate (IPath, hostClasses));
 \endcode
 which is a slightly shorter form of writing:
 \code
 IPtr<IPath> path = OPtr<IPath> (FHostCreate (IPath, hostClasses));
 \endcode
 */
template <class I>
IPtr<I> owned (I* p)
{
	return IPtr<I> (p, false);
}

//------------------------------------------------------------------------
} // Steinberg
