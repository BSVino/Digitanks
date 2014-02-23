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

#ifndef LW_MATHS_H
#define LW_MATHS_H

// For min/max
#include <algorithm>

// Generic math functions
#include <math.h>
#include <string.h>

inline float Bias(float x, float flAmount)
{
	static float flLastAmount = -1;
	static float flLastExp = -1;

	if (flAmount == 0.5f)
		return x;

	if (flLastAmount != flAmount)
		flLastExp = log(flAmount) * -1.4427f;	// 1/log(0.5f)

	if (x < 0)
		return 0;

	return pow(x, flLastExp);
}

inline float Gain(float x, float flAmount)
{
	if(x < 0.5f)
		return Bias(2*x, flAmount)/2;
	else
		return 1-Bias(2-2*x, flAmount)/2;
}

template <class T>
inline const T LerpValue(const T& from, const T& to, float flLerp)
{
	return to * flLerp + from * (1-flLerp);
}

template <class T>
inline const T LerpValue(const T& from, const T& to, float flLerp, float flInLo, float flInHi)
{
	return (((flLerp-flInLo) / (flInHi-flInLo)) * (to-from)) + from;
}

template <class T>
inline const T LerpValueClamped(const T& from, const T& to, float flLerp, float flInLo, float flInHi)
{
	if (flLerp < flInLo)
		return from;

	if (flLerp > flInHi)
		return to;

	return (((flLerp-flInLo) / (flInHi-flInLo)) * (to-from)) + from;
}

inline float RemapVal(float flInput, float flInLo, float flInHi, float flOutLo, float flOutHi)
{
	return (((flInput-flInLo) / (flInHi-flInLo)) * (flOutHi-flOutLo)) + flOutLo;
}

inline double RemapVal(double flInput, double flInLo, double flInHi, double flOutLo, double flOutHi)
{
	return (((flInput-flInLo) / (flInHi-flInLo)) * (flOutHi-flOutLo)) + flOutLo;
}

template <class T>
inline T RemapVal(T flInput, T flInLo, T flInHi, T flOutLo, T flOutHi)
{
	return (((flInput-flInLo) / (flInHi-flInLo)) * (flOutHi-flOutLo)) + flOutLo;
}

template <class T>
inline void TSwap(T& l, T& r)
{
	T temp = l;
	l = r;
	r = temp;
}

inline float RemapValClamped(float flInput, float flInLo, float flInHi, float flOutLo, float flOutHi)
{
	float flReturn = RemapVal(flInput, flInLo, flInHi, flOutLo, flOutHi);

	if (flOutHi < flOutLo)
		TSwap(flOutHi, flOutLo);

	if (flReturn < flOutLo)
		return flOutLo;

	if (flReturn > flOutHi)
		return flOutHi;

	return flReturn;
}

inline double RemapValClamped(double flInput, double flInLo, double flInHi, double flOutLo, double flOutHi)
{
	double flReturn = RemapVal(flInput, flInLo, flInHi, flOutLo, flOutHi);

	if (flOutHi < flOutLo)
		TSwap(flOutHi, flOutLo);

	if (flReturn < flOutLo)
		return flOutLo;

	if (flReturn > flOutHi)
		return flOutHi;

	return flReturn;
}

template <class T>
inline T RemapValClamped(float flInput, float flInLo, float flInHi, const T& flOutLo, const T& flOutHi)
{
	float flLower = std::min(flInLo, flInHi);
	float flHigher = std::max(flInLo, flInHi);

	if (flInput < flLower)
		return flOutLo;

	if (flInput > flHigher)
		return flOutHi;

	return ((flOutHi-flOutLo) * ((flInput-flInLo) / (flInHi-flInLo))) + flOutLo;
}

template<typename T>
T Clamp(T flInput, T flMin, T flMax)
{
	return (flInput<flMin)?flMin:(flInput>flMax)?flMax:flInput;
}
 
inline float Blink(float flTime, float flLength)
{
	if (fmod(flTime, flLength) > flLength/2)
		return 1.0f;
	
	return 0.0f;
}

inline float Oscillate(float flTime, float flLength)
{
	return fabs(RemapVal((float)fmod(flTime, flLength), 0, flLength, -1, 1));
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

inline float AngleNormalize( float a )
{
	while ( a >= 180 )
		a -= 360;

	while ( a <= -180 )
		a += 360;

	return a;
}

inline float AngleDifference( float a, float b )
{
	if (a != a || b != b)
		return 0;

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
