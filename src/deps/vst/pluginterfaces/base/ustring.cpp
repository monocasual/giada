//-----------------------------------------------------------------------------
// Project     : SDK Core
//
// Category    : Helpers
// Filename    : pluginterfaces/base/ustring.cpp
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

#include "ustring.h"

#if WINDOWS
#include <stdio.h>
#pragma warning (disable : 4996)

#elif MAC
#include <CoreFoundation/CoreFoundation.h>

#endif

//------------------------------------------------------------------------
namespace Steinberg {

//------------------------------------------------------------------------
/** Copy strings of different character width. */
//------------------------------------------------------------------------
template <class TDstChar, class TSrcChar>
void StringCopy (TDstChar* dst, int32 dstSize, const TSrcChar* src, int32 srcSize = -1)
{
	int32 count = dstSize;
	if (srcSize >= 0 && srcSize < dstSize)
		count = srcSize;
	for (int32 i = 0; i < count; i++)
	{
		dst[i] = (TDstChar)src[i];
		if (src[i] == 0)
			break;
	}
	dst[dstSize - 1] = 0;
}

//------------------------------------------------------------------------
/** Find length of null-terminated string. */
//------------------------------------------------------------------------
template <class TSrcChar>
int32 StringLength (const TSrcChar* src, int32 srcSize = -1)
{
	if (srcSize == 0)
		return 0;
	int32 length = 0;
	while (src[length])
	{
		length++;
		if (srcSize > 0 && length >= srcSize)
			break;
	}
	return length;
}

//------------------------------------------------------------------------
// UString
//------------------------------------------------------------------------
int32 UString::getLength () const
{
	return StringLength<char16> (thisBuffer, thisSize);
}

//------------------------------------------------------------------------
UString& UString::assign (const char16* src, int32 srcSize)
{
	StringCopy<char16, char16> (thisBuffer, thisSize, src, srcSize);
	return *this;
}

//------------------------------------------------------------------------
UString& UString::append (const char16* src, int32 srcSize)
{
	int32 length = getLength ();
	StringCopy<char16, char16> (thisBuffer + length, thisSize - length, src, srcSize);
	return *this;
}

//------------------------------------------------------------------------
const UString& UString::copyTo (char16* dst, int32 dstSize) const
{
	StringCopy<char16, char16> (dst, dstSize, thisBuffer, thisSize);
	return *this;
}

//------------------------------------------------------------------------
UString& UString::fromAscii (const char* src, int32 srcSize)
{
	StringCopy<char16, char> (thisBuffer, thisSize, src, srcSize);
	return *this;
}

//------------------------------------------------------------------------
const UString& UString::toAscii (char* dst, int32 dstSize) const
{
	StringCopy<char, char16> (dst, dstSize, thisBuffer, thisSize);
	return *this;
}

//------------------------------------------------------------------------
bool UString::scanFloat (double& value) const
{
#if WINDOWS
	return swscanf ((const wchar_t*)thisBuffer, L"%lf", &value) == 1;

#elif TARGET_API_MAC_CARBON
	CFStringRef cfStr = CFStringCreateWithBytes (0, (const UInt8 *)thisBuffer, getLength () * 2, kCFStringEncodingUTF16, false);
	if (cfStr)
	{
		value = CFStringGetDoubleValue (cfStr);
		CFRelease (cfStr);
		return true;
	}
	return false;

#else
	// implement me!
	return false;
#endif
}

//------------------------------------------------------------------------
bool UString::printFloat (double value, int32 precision)
{
#if WINDOWS
	return swprintf ((wchar_t*)thisBuffer, L"%.*lf", precision, value) == 1;
#elif MAC
	bool result = false;
	CFStringRef cfStr = CFStringCreateWithFormat (0, 0, CFSTR("%.*lf"), precision, value);
	if (cfStr)
	{
		memset (thisBuffer, 0, thisSize);
		CFRange range = {0, CFStringGetLength (cfStr)};
		CFStringGetBytes (cfStr, range, kCFStringEncodingUTF16, 0, false, (UInt8*)thisBuffer, thisSize, 0);
		CFRelease (cfStr);
	}
	return result;
#else
	// implement me!
	return false;
#endif
}

//------------------------------------------------------------------------
bool UString::scanInt (int64& value) const
{
#if WINDOWS
	return swscanf ((const wchar_t*)thisBuffer, L"%I64d", &value) == 1;

#elif MAC
	CFStringRef cfStr = CFStringCreateWithBytes (0, (const UInt8 *)thisBuffer, getLength () * 2, kCFStringEncodingUTF16, false);
	if (cfStr)
	{
		value = CFStringGetIntValue (cfStr);
		CFRelease (cfStr);
		return true;
	}
	return false;

#else
	// implement me!
	return false;
#endif
}

//------------------------------------------------------------------------
bool UString::printInt (int64 value)
{
#if WINDOWS
	return swprintf ((wchar_t*)thisBuffer, L"%I64d", value) == 1;

#elif MAC
	CFStringRef cfStr = CFStringCreateWithFormat (0, 0, CFSTR("%lld"), value);
	if (cfStr)
	{
		memset (thisBuffer, 0, thisSize);
		CFRange range = {0, CFStringGetLength (cfStr)};
		CFStringGetBytes (cfStr, range, kCFStringEncodingUTF16, 0, false, (UInt8*)thisBuffer, thisSize, 0);
		CFRelease (cfStr);
		return true;
	}
	return false;
#else
	// implement me!
	return false;
#endif
}
} // namespace Steinberg
