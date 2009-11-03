#ifndef CF_MATHS_H
#define CF_MATHS_H

// Generic math functions
#include <math.h>

float g_flLastLerp = -1;
float g_flLastExp = -1;

float Lerp(float x, float flLerp)
{
	if (flLerp == 0.5f)
		return x;

	if (g_flLastLerp != flLerp)
		g_flLastExp = log(flLerp) * -1.4427f;	// 1/log(0.5f)

	return pow(x, g_flLastExp);
}

float SLerp(float x, float flLerp)
{
	if(x < 0.5f)
		return Lerp(2*x, flLerp)/2;
	else
		return 1-Lerp(2-2*x, flLerp)/2;
}

#endif