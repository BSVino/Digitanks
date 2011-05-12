#ifndef COMMON_H
#define COMMON_H

#define DECLARE_CLASS( className, baseClassName ) \
	typedef baseClassName BaseClass; \
	typedef className ThisClass; \

#ifdef _DEBUG

#define TAssert(x) \
	{ \
		if (!(x)) \
			__asm { int 3 }; \
	} \

#else

#define TAssert(x) \
	{ \
	} \

#endif

#endif
