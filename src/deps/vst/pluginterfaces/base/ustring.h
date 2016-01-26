//-----------------------------------------------------------------------------
// Project     : SDK Core
//
// Category    : Helpers
// Filename    : pluginterfaces/base/ustring.h
// Created by  : Steinberg, 12/2005
// Description : UTF-16 String class
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
//------------------------------------------------------------------------------

#ifndef __ustring__
#define __ustring__

#include "ftypes.h"

//------------------------------------------------------------------------
namespace Steinberg {

//------------------------------------------------------------------------
/** UTF-16 string class without buffer management. */
//------------------------------------------------------------------------
class UString
{
public:
//------------------------------------------------------------------------
	UString (char16* buffer, int32 size)
	: thisBuffer (buffer),
	  thisSize (size)
	{}

	int32 getSize () const			{ return thisSize; }	///< returns buffer size
	operator const char16* () const	{ return thisBuffer; }	///< cast to char16*

	/** Returns length of string (in code units). */
	int32 getLength () const;

	/** Copy from UTF-16 buffer. */
	UString& assign (const char16* src, int32 srcSize = -1);

	/** Append UTF-16 buffer. */
	UString& append (const char16* src, int32 srcSize = -1);

	/** Copy to UTF-16 buffer. */
	const UString& copyTo (char16* dst, int32 dstSize) const;

	/** Copy from ASCII string. */
	UString& fromAscii (const char* src, int32 srcSize = -1);
	UString& assign (const char* src, int32 srcSize = -1) { return fromAscii (src, srcSize); }

	/** Copy to ASCII string. */
	const UString& toAscii (char* dst, int32 dstSize) const;

	/** Scan integer from string. */
	bool scanInt (int64& value) const;

	/** Print integer to string. */
	bool printInt (int64 value);

	/** Scan float from string. */
	bool scanFloat (double& value) const;

	/** Print float to string. */
	bool printFloat (double value, int32 precision = 4);
//------------------------------------------------------------------------
protected:
	char16* thisBuffer;
	int32 thisSize;
};

//------------------------------------------------------------------------
/** UTF-16 string with fixed buffer size. */
//------------------------------------------------------------------------
template<int32 maxSize>
class UStringBuffer: public UString
{
public:
//------------------------------------------------------------------------
	UStringBuffer ()
	: UString (data, maxSize)
	{ data[0] = 0; }

	/** Construct from UTF-16 string. */
	UStringBuffer (const char16* src, int32 srcSize = -1)
	: UString (data, maxSize)
	{ data[0] = 0; if (src) assign (src, srcSize); }

	/** Construct from ASCII string. */
	UStringBuffer (const char* src, int32 srcSize = -1)
	: UString (data, maxSize)
	{ data[0] = 0; if (src) fromAscii (src, srcSize); }
//------------------------------------------------------------------------
protected:
	char16 data[maxSize];
};

//------------------------------------------------------------------------
typedef UStringBuffer<128> UString128;	///< 128 character UTF-16 string
typedef UStringBuffer<256> UString256;	///< 256 character UTF-16 string
} // namespace Steinberg

//------------------------------------------------------------------------
#define USTRING(asciiString)	Steinberg::UString256 (asciiString)
#define USTRINGSIZE(var)		(sizeof (var) / sizeof (Steinberg::char16))

//------------------------------------------------------------------------

#endif
