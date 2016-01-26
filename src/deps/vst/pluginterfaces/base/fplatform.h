//-----------------------------------------------------------------------------
// Project     : SDK Core
//
// Category    : SDK Core Interfaces
// Filename    : pluginterfaces/base/fplatform.h
// Created by  : Steinberg, 01/2004
// Description : Detect platform and set define
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

#ifndef __fplatform__
#define __fplatform__

#define kLittleEndian 0
#define kBigEndian 1

#undef PLUGIN_API

#undef WINDOWS
#undef MAC

#if defined (_WIN32)						// WIN32 AND WIN64
	#define WINDOWS 1
	#define BYTEORDER kLittleEndian
	#define COM_COMPATIBLE 1
	#define PLUGIN_API __stdcall

	#define _CRT_SECURE_NO_WARNINGS

	#pragma warning (disable : 4244) // Conversion from 'type1' to 'type2', possible loss of data.
	#pragma warning (disable : 4250 ) // Inheritance via dominance is allowed
	#pragma warning (disable : 4996) // deprecated functions

	#pragma warning (3 : 4189) // local variable is initialized but not referenced
	#pragma warning (3 : 4238) // nonstandard extension used : class rvalue used as lvalue

	#if defined (_WIN64)					// WIN64 only
		#define PLATFORM_64 1
	#endif
	#ifndef WIN32
		#define WIN32	1
	#endif

	#ifdef __cplusplus
		#define SMTG_CPP11	__cplusplus >= 201103L || _MSC_VER >= 1600 || __INTEL_CXX11_MODE__
		#define SMTG_CPP11_STDLIBSUPPORT SMTG_CPP11
	#endif
#elif __UNIX__
	#if LINUX
		#define BYTEORDER kLittleEndian
	#else
		#define BYTEORDER kBigEndian
	#endif
	#define COM_COMPATIBLE 0
	#define PLUGIN_API
#elif __APPLE__                             // Mac and iOS
	#include <TargetConditionals.h>
	#define MAC 1
	#define PTHREADS 1
	#if !TARGET_OS_IPHONE
		#ifndef __CF_USE_FRAMEWORK_INCLUDES__
		#define __CF_USE_FRAMEWORK_INCLUDES__
		#endif
		#ifndef TARGET_API_MAC_CARBON
		#define TARGET_API_MAC_CARBON 1
		#endif
	#endif
	#if __LP64__
		#define PLATFORM_64 1
	#endif
	#if defined (__BIG_ENDIAN__)
		#define BYTEORDER kBigEndian
	#else
		#define BYTEORDER kLittleEndian
	#endif
	#define COM_COMPATIBLE 0
	#define PLUGIN_API

	#if !defined(__PLIST__) && !defined(SMTG_DISABLE_DEFAULT_DIAGNOSTICS)
		#ifdef __clang__
			#pragma GCC diagnostic ignored "-Wswitch-enum"
			#pragma GCC diagnostic ignored "-Wparentheses"
			#pragma GCC diagnostic ignored "-Wuninitialized"
			#if __clang_major__ >= 3
				#pragma GCC diagnostic ignored "-Wtautological-compare"
				#pragma GCC diagnostic ignored "-Wunused-value"
				#if __clang_major__ >= 4 || __clang_minor__ >= 1
					#pragma GCC diagnostic ignored "-Wswitch"
					#pragma GCC diagnostic ignored "-Wcomment"
				#endif
				#if __clang_major__ >= 5
					#pragma GCC diagnostic ignored "-Wunsequenced"
					#if __clang_minor__ >= 1
						#pragma GCC diagnostic ignored "-Wunused-const-variable"
					#endif
				#endif
			#endif
		#endif
	#endif
	#ifdef __cplusplus
		#include <cstddef>
		#define SMTG_CPP11 (__cplusplus >= 201103L || __INTEL_CXX11_MODE__)
		#define SMTG_CPP11_STDLIBSUPPORT (defined (_LIBCPP_VERSION) && SMTG_CPP11)
	#endif
#else
	#pragma error unknown platform

#endif

#endif
