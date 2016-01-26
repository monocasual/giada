//-----------------------------------------------------------------------------
// Project     : SDK Core
//
// Category    : SDK Core Interfaces
// Filename    : pluginterfaces/base/conststringtable.cpp
// Created by  : Steinberg, 09/2007
// Description : constant unicode string table
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

#include "conststringtable.h"
#include <cstring>
#include <map>

namespace Steinberg
{

static std::map<const char8*, char16*>* stringMap;
static std::map<const char8, char16>* charMap;

static char16* generateUTF16 (const char8* str);

//----------------------------------------------------------------------------
ConstStringTable* ConstStringTable::instance ()
{
	static ConstStringTable stringTable;
	return &stringTable;
}

//----------------------------------------------------------------------------
const char16* ConstStringTable::getString (const char8* str) const
{
	std::map<const char8*, char16*>::iterator iter = stringMap->find (str);
	if (iter != stringMap->end ())
		return iter->second;
	char16* uStr = generateUTF16 (str);
	stringMap->insert (std::make_pair (str, uStr));
	return uStr;
}

//----------------------------------------------------------------------------
const char16 ConstStringTable::getString (const char8 str) const
{
	std::map<const char8, char16>::iterator iter = charMap->find (str);
	if (iter != charMap->end ())
		return iter->second;
	char16 uStr = 0;
#if BYTEORDER == kBigEndian
	char8* puStr = (char8*)&uStr;
	puStr[1] = str;
#else
	uStr = str;
#endif
	charMap->insert (std::make_pair (str, uStr));
	return uStr;
}

//----------------------------------------------------------------------------
ConstStringTable::ConstStringTable ()
{
	stringMap = new std::map<const char8*, char16*>;
	charMap = new std::map<const char8, char16>;
}

//----------------------------------------------------------------------------
ConstStringTable::~ConstStringTable ()
{
	// free out allocated strings
	{
		std::map<const char8*, char16*>::iterator iter = stringMap->begin ();
		while (iter != stringMap->end ())
		{
			delete [] iter->second;
			iter++;
		}
	} // delete iterator on map before deleting the map

	delete stringMap;
	delete charMap;
}

//----------------------------------------------------------------------------
char16* generateUTF16 (const char8* str)
{
	int32 len = (int32)strlen (str);
	char16* result = new char16[len+1];
	for (int32 i = 0; i < len; i++)
	{
	#if BYTEORDER == kBigEndian
		char8* pChr = (char8*)&result[i];
		pChr[0] = 0;
		pChr[1] = str[i];
	#else
		result[i] = str[i];
	#endif
	}
	result[len] = 0;
	return result;
}
}	// namespace Steinberg
