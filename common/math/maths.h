#ifndef LW_MATHS_H
#define LW_MATHS_H

// Generic math functions
#include <math.h>
#include <string.h>

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

inline float Blink(float flTime, float flLength)
{
	if (fmod(flTime, flLength) > flLength/2)
		return 1.0f;
	
	return 0.0f;
}

inline float Oscillate(float flTime, float flLength)
{
	return fabs(RemapVal(fmod(flTime, flLength), 0, flLength, -1, 1));
}

// Strobe: Flicker("az", GetGameTime(), 0.1f)
// Blink: Flicker("aaaaaaz", GetGameTime(), 1.0f)
// Ramp: Flicker("abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcba", GetGameTime(), 1.0f)
inline float Flicker(const char* pszValues, float flTime, float flLength)
{
	if (!pszValues)
		return 0;

	int iValues = strlen(pszValues);
	if (iValues == 0)
		return 0;

	float flModTime = fmod(flTime, flLength);
	int iValue = (int)RemapValClamped(flModTime, 0, flLength, 0, (float)iValues);
	return RemapVal((float)pszValues[iValue], 'a', 'z', 0, 1);
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

inline float AngleDifference( float a, float b )
{
	float d = a - b;

	if ( a > b )
		while ( d >= 180 )
			d -= 360;
	else
		while ( d <= -180 )
			d += 360;

	return d;
}

inline float AngleApproach(float flGoal, float flInput, float flAmount)
{
	float flDifference = AngleDifference(flGoal, flInput);

	if (flDifference > flAmount)
		return flInput + flAmount;
	else if (flDifference < -flAmount)
		return flInput -= flAmount;
	else
		return flGoal;
}

#endif