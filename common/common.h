#ifndef COMMON_H
#define COMMON_H

#define DECLARE_CLASS( className, baseClassName ) \
	typedef baseClassName BaseClass; \
	typedef className ThisClass; \

#define TMsg printf

#ifdef __GNUC__

#include <csignal>

#define TDebugBreak() \
	::raise(SIGTRAP); \

#else

#define TDebugBreak() \
	__debugbreak(); \

#endif

#ifdef _DEBUG

#define TAssert(x) \
{ \
	if (!(x)) \
	{ \
		TMsg("Assert failed: " #x "\n"); \
		TDebugBreak(); \
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
