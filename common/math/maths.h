#ifndef CF_MATHS_H
#define CF_MATHS_H

// Generic math functions
#include <math.h>

inline float Lerp(float x, float flLerp)
{
	static float flLastLerp = -1;
	static float flLastExp = -1;

	if (flLerp == 0.5f)
		return x;

	if (flLastLerp != flLerp)
		flLastExp = log(flLerp) * -1.4427f;	// 1/log(0.5f)

	return pow(x, flLastExp);
}

inline float SLerp(float x, float flLerp)
{
	if(x < 0.5f)
		return Lerp(2*x, flLerp)/2;
	else
		return 1-Lerp(2-2*x, flLerp)/2;
}

inline float RemapVal(float flInput, float flInLo, float flInHi, float flOutLo, float flOutHi)
{
	return (((flInput-flInLo) / (flInHi-flInLo)) * (flOutHi-flOutLo)) + flOutLo;
}

inline float RemapValClamped(float flInput, float flInLo, float flInHi, float flOutLo, float flOutHi)
{
	if (flInput < flInLo)
		return flOutLo;

	if (flInput > flInHi)
		return flOutHi;

	return RemapVal(flInput, flInLo, flInHi, flOutLo, flOutHi);
}

inline float Approach(float flGoal, float flInput, float flAmount)
{
	float flDifference = flGoal - flInput;

	if (flDifference > flAmount)
		return flInput + flAmount;
	else if (flDifference < -flAmount)
		return flInput -= flAmount;
	else
		return flGoal;
}

#endif