#ifndef COMMON_H
#define COMMON_H

#define DECLARE_CLASS( className, baseClassName ) \
	typedef baseClassName BaseClass; \
	typedef className ThisClass; \

extern void DebugBreak();

#define TMsg printf;

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
	{ \
		TMsg("Assert failed: " #x "\n"); \
		DebugBreak(); \
	} \
} \

#else

#define TAssert(x) \
{ \
	if (!(x)) \
		printf("Assert failed: " #x "\n"); \
} \

#endif

#endif
