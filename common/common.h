#ifndef COMMON_H
#define COMMON_H

#define DECLARE_CLASS( className, baseClassName ) \
	typedef baseClassName BaseClass; \
	typedef className ThisClass; \

extern void DebugBreak();

#ifdef __GNUC__

#include <csignal>

#define DebugBreak() \
	::raise(SIGTRAP); \

#else

#define DebugBreak() \
	__debugbreak(); \

#endif

#ifdef _DEBUG

#define TAssert(x) \
{ \
	if (!(x)) \
		DebugBreak(); \
} \

#else

#define TAssert(x) \
	{ \
	} \

#endif

#endif
