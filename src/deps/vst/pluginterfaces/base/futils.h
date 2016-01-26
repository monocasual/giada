//-----------------------------------------------------------------------------
// Project     : SDK Core
//
// Category    : SDK Core Interfaces
// Filename    : pluginterfaces/base/futils.h
// Created by  : Steinberg, 01/2004
// Description : Basic utilities
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

#ifndef __futils__
#define __futils__

namespace Steinberg
{
	//----------------------------------------------------------------------------
	// min/max/etc. template functions
	template <class T> 
	inline const T& Min (const T& a, const T& b)
	{
		return b < a ? b : a;
	}

	template <class T> 
	inline const T& Max (const T& a, const T& b)
	{
		return a < b ? b : a;
	}

	template <class T> 
	inline T Abs (const T& value)
	{
		return (value >= (T)0) ? value : -value;
	}

	template <class T> 
	inline T Sign (const T& value)
	{
		return (value == (T)0) ? 0 : ((value >= (T)0) ? 1 : -1);
	}

	template <class T> 
	inline T Bound (T minval, T maxval, T x)
	{
		if (x < minval)
			return minval;
		else if (x > maxval)
			return maxval;
		return x;
	}

	template <class T> 
	void Swap (T& t1, T& t2)
	{
		T tmp = t1;
		t1 = t2;
		t2 = tmp;
	}

	template <class T> 
	bool IsApproximateEqual (T t1, T t2, T epsilon)
	{
		if (t1 == t2)
			return true;
		T diff = t1 - t2;
		if (diff < 0.0)
			diff = -diff;
		if (diff < epsilon)
			return true;
		return false;
	}
}

#endif
