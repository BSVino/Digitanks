/*
Copyright (c) 2012, Lunar Workshop, Inc.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
3. All advertising materials mentioning features or use of this software must display the following acknowledgement:
   This product includes software developed by Lunar Workshop, Inc.
4. Neither the name of the Lunar Workshop nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY LUNAR WORKSHOP INC ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL LUNAR WORKSHOP BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef COMMON_H
#define COMMON_H

#define DECLARE_CLASS( className, baseClassName ) \
	typedef baseClassName BaseClass; \
	typedef className ThisClass; \

#ifdef __GNUC__

#if defined(__i386__) || defined(__x86_64__)
#define TDebugBreak() __asm__ __volatile__ ( "int $3\n\t" )
#else
#include <csignal>

#define TDebugBreak() \
	::raise(SIGTRAP); \

#endif

#else

#define TDebugBreak() \
	__debugbreak(); \

#endif

// tinker_platform.h
extern void DebugPrint(const char* pszText);

#ifdef _MSC_VER
#define PRAGMA_WARNING_PUSH __pragma(warning(push))
#define PRAGMA_WARNING_DISABLE(n) __pragma(warning(disable:n))
#define PRAGMA_WARNING_POP __pragma(warning(pop))
#else
#define PRAGMA_WARNING_PUSH
#define PRAGMA_WARNING_DISABLE(n)
#define PRAGMA_WARNING_POP
#endif

#ifdef _DEBUG

#define TAssert(x) \
do { \
	PRAGMA_WARNING_PUSH \
	PRAGMA_WARNING_DISABLE(4127) /* conditional expression is constant */ \
	if (!(x)) \
	PRAGMA_WARNING_POP \
	{ \
		TMsg("Assert failed: " #x "\n"); \
		TDebugBreak(); \
	} \
	PRAGMA_WARNING_PUSH \
	PRAGMA_WARNING_DISABLE(4127) /* conditional expression is constant */ \
} while (0) \
PRAGMA_WARNING_POP \

#define TAssertNoMsg(x) \
do { \
	PRAGMA_WARNING_PUSH \
	PRAGMA_WARNING_DISABLE(4127) /* conditional expression is constant */ \
	if (!(x)) \
	PRAGMA_WARNING_POP \
	{ \
		DebugPrint("Assert failed: " #x "\n"); \
		TDebugBreak(); \
	} \
	PRAGMA_WARNING_PUSH \
	PRAGMA_WARNING_DISABLE(4127) /* conditional expression is constant */ \
} while (0) \
PRAGMA_WARNING_POP \

#else

#if defined(_T_RELEASE_ASSERTS)
#define TAssert(x) \
{ \
	if (!(x)) \
		TMsg("Assert failed: " #x "\n"); \
} \

#define TAssertNoMsg(x) \
{ \
	if (!(x)) \
		DebugPrint("Assert failed: " #x "\n"); \
} \

#else

#define TAssert(x) {}
#define TAssertNoMsg(x) {}

#endif

#endif

#if defined(__ANDROID__)
// If you hit this, the code is either incomplete or untested.
#define TUnimplemented() do { \
	char s[1000]; \
	sprintf(s, "TUnimplemented file " __FILE__ " line %d\n", __LINE__); \
	DebugPrint(s); \
	TAssertNoMsg(false); \
	} while (0)
#else
// If you hit this, the code is either incomplete or untested.
#define TUnimplemented() TAssertNoMsg(false)
#endif

#ifdef _DEBUG

// Ruthlessly remove this if you see it.
#define TStubbed(x) \
	do { \
		static bool seen = false; \
		if (!seen) { \
			seen = true; \
			TMsg("STUBBED: " x "\n"); \
		} \
	} while (false); \

#else

// Code will not build in release mode until stubs are removed.
#define TStubbed(x) ERROR!

#endif

#if defined(__GNUC__) && !defined(__clang__)
#if __GNUC__ < 4 || __GNUC_MINOR__ < 6

const                        // this is a const object...
class {
public:
  template<class T>          // convertible to any type
    operator T*() const      // of null non-member
    { return 0; }            // pointer...
  template<class C, class T> // or any type of null
    operator T C::*() const  // member pointer...
    { return 0; }
private:
  void operator&() const;    // whose address can't be taken
} nullptr = {};              // and whose name is nullptr

#endif

// For std::shared_ptr
#include <memory>

#endif

#ifndef WITH_EASTL
#include <stdint.h>
#endif

#ifdef __ANDROID__
#define T_TOUCH_PLATFORM
#endif

#endif
