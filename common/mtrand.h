#ifndef _MTRAND_H
#define _MTRAND_H

#ifdef __GNUC__
#include <stddef.h>
#endif

void mtsrand(size_t iSeed);
size_t mtrand();

#include <maths.h>

inline float RandomFloat(float flLow, float flHigh)
{
	return RemapVal((float)(mtrand()%999999), 0, 999999, flLow, flHigh);
}

inline int RandomInt(int iLow, int iHigh)
{
	if (iLow == iHigh)
		return iLow;

	if (iLow > iHigh)
		return iLow;

	return mtrand()%(iHigh-iLow+1) + iLow;
}

#endif
