#include "shaders.h"

const char* CShaderLibrary::GetVSTerrainShader()
{
	return
		LENGTHSQR

		"varying vec4 vecFrontColor;"
		"varying vec3 vecPosition;"

		"uniform int iCraterMarks;"
		"uniform vec3 avecCraterMarks[10];"
		"varying vec3 avecClosestCrater;"

		"uniform vec3 avecAimTargets[5];"
		"uniform float aflAimTargetRadius[5];"
		"uniform int iAimTargets;"
		"varying vec2 vecAimTarget1;"
		"varying vec2 vecAimTarget2;"
		"varying vec2 vecAimTarget3;"
		"varying vec2 vecAimTarget4;"
		"varying vec2 vecAimTarget5;"
		"varying float flAimTargetRadius1;"
		"varying float flAimTargetRadius2;"
		"varying float flAimTargetRadius3;"
		"varying float flAimTargetRadius4;"
		"varying float flAimTargetRadius5;"

		"void main()"
		"{"
		"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
		"	vecPosition = gl_Vertex.xyz;"
		"	gl_FrontColor = vecFrontColor = gl_Color;"
		"	float flRadius = 6.0;"
		"	avecClosestCrater = vec3(0,0,0);"
		"	for (int i = 0; i < iCraterMarks; i++)"
		"	{"
		"		float flDistanceToMarkSqr = LengthSqr(vecPosition - avecCraterMarks[i]);"
		"		if (i == 0 || flDistanceToMarkSqr < LengthSqr(vecPosition - avecClosestCrater))"
		"			avecClosestCrater = avecCraterMarks[i];"
		"	}"
		"	flAimTargetRadius1 = aflAimTargetRadius[0];"
		"	flAimTargetRadius2 = aflAimTargetRadius[1];"
		"	flAimTargetRadius3 = aflAimTargetRadius[2];"
		"	flAimTargetRadius4 = aflAimTargetRadius[3];"
		"	flAimTargetRadius5 = aflAimTargetRadius[4];"
		"	vecAimTarget1 = vec2(avecAimTargets[0].x, avecAimTargets[0].z);"
		"	vecAimTarget2 = vec2(avecAimTargets[1].x, avecAimTargets[1].z);"
		"	vecAimTarget3 = vec2(avecAimTargets[2].x, avecAimTargets[2].z);"
		"	vecAimTarget4 = vec2(avecAimTargets[3].x, avecAimTargets[3].z);"
		"	vecAimTarget5 = vec2(avecAimTargets[4].x, avecAimTargets[4].z);"
		"}";
}

const char* CShaderLibrary::GetFSTerrainShader()
{
	return
		REMAPVAL
		LERP
		LENGTHSQR
		DISTANCE_TO_SEGMENT_SQR
		ANGLE_DIFFERENCE

		"uniform vec3 vecTankOrigin;"

		"uniform float flMoveDistance;"
		"uniform bool bMovement;"

		"uniform vec3 vecTurnPosition;"
		"uniform bool bTurnValid;"
		"uniform float flTankYaw;"
		"uniform float flTankMaxYaw;"
		"uniform bool bTurning;"

		"uniform vec3 vecTankPreviewOrigin;"
		"uniform float flTankMaxRange;"
		"uniform float flTankEffRange;"
		"uniform float flTankMinRange;"
		"uniform float flTankFiringCone;"
		"uniform bool bShowRanges;"
		"uniform bool bFocusRanges;"

		"uniform int iCraterMarks;"
		"varying vec3 avecClosestCrater;"

		"uniform int iFocusTarget;"
		"uniform int iAimTargets;"
		"varying vec2 vecAimTarget1;"
		"varying vec2 vecAimTarget2;"
		"varying vec2 vecAimTarget3;"
		"varying vec2 vecAimTarget4;"
		"varying vec2 vecAimTarget5;"
		"varying float flAimTargetRadius1;"
		"varying float flAimTargetRadius2;"
		"varying float flAimTargetRadius3;"
		"varying float flAimTargetRadius4;"
		"varying float flAimTargetRadius5;"

		"varying vec4 vecFrontColor;"
		"varying vec3 vecPosition;"

		"float GetAimTargetStrength(float flDistanceSqr, float flRadius, bool bFocusTarget)"
		"{"
		"	float flColorStrength = 0.3;"
		"	if (bFocusTarget)"
		"		flColorStrength = 0.7;"

		"	float flTargetColorStrength = RemapVal(flDistanceSqr, 0.0, flRadius*flRadius, 0.0, 1.0);"

		"	if (flRadius > 2.0 && flRadius < 10.0)"
		"		flColorStrength = RemapVal(flRadius, 2.0, 10.0, flColorStrength, flColorStrength/4.0);"

		"	return Lerp(flTargetColorStrength, 0.8) * flColorStrength;"
		"}"

		"void main()"
		"{"
		"	vec4 vecBaseColor = vecFrontColor;"

		"	float flXMod = mod(vecPosition.x, 10.0);"
		"	float flZMod = mod(vecPosition.z, 10.0);"
		"	if (flXMod < 0.1 || flZMod < 0.1)"
		"		vecBaseColor = (vecBaseColor * 0.8) + vec4(0.75, 0.75, 0.75, 1.0)*0.2;"

		"	float flYMod = mod(vecPosition.y, 10.0);"
		"	if (flYMod < 0.1)"
		"		vecBaseColor = (vecBaseColor * 0.8) + vec4(0.5, 0.5, 0.5, 1.0)*0.2;"

		"	if (iCraterMarks > 0)"
		"	{"
		"		float flRadius = 6.0;"
		"		float flDistanceToMarkSqr = LengthSqr(vecPosition - avecClosestCrater);"
		"		if (flDistanceToMarkSqr < flRadius*flRadius)"
		"		{"
		"			float flStrength = RemapVal(flDistanceToMarkSqr, 0.0, flRadius*flRadius, 0.2, 0.0);"
		"			vecBaseColor = vecBaseColor + vec4(vec3(1.0, 1.0, 1.0) * (mod(flDistanceToMarkSqr*3.0, 2.0)-1.0), 1.0) * flStrength;"
		"		}"
		"	}"

		"	if (bMovement)"
		"	{"
		"		float flTankDistanceSqr = LengthSqr(vecPosition - vecTankOrigin);"
		"		if (flTankDistanceSqr <= flMoveDistance*flMoveDistance)"
		"		{"
		"			float flMoveColorStrength = RemapVal(flTankDistanceSqr, 0.0, flMoveDistance*flMoveDistance, 0.0, 1.0);"
		"			flMoveColorStrength = Lerp(flMoveColorStrength, 0.2);"
		"			vecBaseColor = vecBaseColor * (1.0-flMoveColorStrength) + vec4(0.8, 0.8, 0.0, 1.0) * flMoveColorStrength;"
		"		}"
		"	}"

		"	if (bTurning)"
		"	{"
		"		float flInside = 4.0;"
		"		float flOutside = 10.0;"

				// Draw the turning indicator
		"		float flTankDistanceSqr = LengthSqr(vecPosition - vecTankOrigin);"
		"		if (flTankDistanceSqr > flInside*flInside && flTankDistanceSqr < flOutside*flOutside)"
		"		{"
		"			vec3 vecDirection = normalize(vecPosition - vecTankOrigin);"
		"			float flYaw = atan(vecDirection.z, vecDirection.x) * 180.0/3.14159;"

		"			float flYawDifference = AngleDifference(flYaw, flTankYaw);"

		"			if (abs(flYawDifference) < flTankMaxYaw)"
		"			{"
		"				float flMoveColorStrength = RemapVal(flTankDistanceSqr, flInside*flInside, flOutside*flOutside, 1.0, 0.0);"
		"				flMoveColorStrength = Lerp(flMoveColorStrength, 0.1);"
		"				vecBaseColor = vecBaseColor * (1.0-flMoveColorStrength) + vec4(0.8, 0.8, 0.0, 1.0) * flMoveColorStrength;"
		"			}"
		"		}"

		"		if (flTankDistanceSqr > flInside*flInside)"
		"		{"
		"			float flInner = 0.5;"
		"			float flOuter = 1.0;"

		"			float flTurnDistanceSqr = LengthSqr(vecPosition - vecTurnPosition);"

		"			vec4 vecTurnColor = vec4(0.8, 0.8, 0.0, 1.0);"
		"			if (!bTurnValid)"
		"				vecTurnColor = vec4(0.2, 0.2, 0.0, 1.0);"

					// Draw a dot and a line to it that represents the direction we are looking
		"			if (flTurnDistanceSqr <= flInner*flInner)"
		"				vecBaseColor = vecTurnColor;"
		"			else if (flTurnDistanceSqr <= flOuter*flOuter)"
		"			{"
		"				float flStrength = RemapVal(flTurnDistanceSqr, flInner*flInner, flOuter*flOuter, 1.0, 0.0);"
		"				vecBaseColor = vecBaseColor * (1.0-flStrength) + vecTurnColor * flStrength;"
		"			}"

		"			float flLine = 0.2;"
		"			vec3 vecPositionY = vecPosition;"
		"			vec3 vecTurnPositionY = vecTurnPosition;"
		"			vec3 vecTankOriginY = vecTankOrigin;"
		"			vecPositionY.y = vecTurnPositionY.y = vecTankOriginY.y = 0.0;"
		"			float flDistanceToLineSqr = DistanceToLineSegmentSqr(vecPositionY, vecTurnPositionY, vecTankOriginY);"
		"			if (flDistanceToLineSqr < flLine*flLine)"
		"			{"
		"				float flStrength = RemapVal(flDistanceToLineSqr, 0.0, flLine*flLine, 1.0, 0.0);"
		"				vecBaseColor = vecBaseColor * (1.0-flStrength) + vecTurnColor * flStrength;"
		"			}"
		"		}"
		"	}"

		"	if (bShowRanges)"
		"	{"
		"		float flColorStrength = 0.7;"
		"		if (!bFocusRanges)"
		"			flColorStrength = 0.3;"

		"		vec3 vecDirection = vecPosition - vecTankPreviewOrigin;"
		"		float flPreviewDistanceSqr = LengthSqr(vecDirection);"
		"		vecDirection = normalize(vecDirection);"

		"		float flYaw = atan(vecDirection.z, vecDirection.x) * 180.0/3.14159;"
		"		float flYawDifference = AngleDifference(flYaw, flTankYaw);"

		"		if (abs(flYawDifference) < flTankFiringCone)"
		"		{"
		"			if (flPreviewDistanceSqr <= flTankEffRange*flTankEffRange && flPreviewDistanceSqr >= flTankMinRange*flTankMinRange)"
		"			{"
		"				float flMoveColorStrength = RemapVal(flPreviewDistanceSqr, flTankMinRange*flTankMinRange, flTankEffRange*flTankEffRange, 0.0, 1.0);"
		"				flMoveColorStrength = Lerp(flMoveColorStrength, 0.8) * flColorStrength;"
		"				vecBaseColor = vecBaseColor * (1.0-flMoveColorStrength) + vec4(0.0, 0.8, 0.0, 1.0) * flMoveColorStrength;"
		"			}"
		"			if (flPreviewDistanceSqr <= flTankMaxRange*flTankMaxRange && flPreviewDistanceSqr >= flTankEffRange*flTankEffRange)"
		"			{"
		"				float flMoveColorStrength = RemapVal(flPreviewDistanceSqr, flTankMaxRange*flTankMaxRange, flTankEffRange*flTankEffRange, 0.0, 1.0);"
		"				flMoveColorStrength = Lerp(flMoveColorStrength, 0.1) * flColorStrength;"
		"				vecBaseColor = vecBaseColor * (1.0-flMoveColorStrength) + vec4(0.0, 0.8, 0.0, 1.0) * flMoveColorStrength;"
		"			}"

		"			float flTankMaxRangeInside = flTankMaxRange-3.0;"
		"			if (flPreviewDistanceSqr <= flTankMaxRange*flTankMaxRange && flPreviewDistanceSqr >= flTankMaxRangeInside*flTankMaxRangeInside)"
		"			{"
		"				float flMoveColorStrength = RemapVal(flPreviewDistanceSqr, flTankMaxRangeInside*flTankMaxRangeInside, flTankMaxRange*flTankMaxRange, 0.0, 1.0);"
		"				flMoveColorStrength = Lerp(flMoveColorStrength, 0.2) * flColorStrength;"
		"				vecBaseColor = vecBaseColor * (1.0-flMoveColorStrength) + vec4(0.8, 0.0, 0.0, 1.0) * flMoveColorStrength;"
		"			}"
		"		}"
		"	}"

		"	vec4 vecAimTargetColor = vec4(0.8, 0.0, 0.0, 1.0);"

		"	vec2 vecPosition2D = vec2(vecPosition.x, vecPosition.z);"

		"	if (iAimTargets > 0)"
		"	{"
		"		float flDistanceToAimSqr1 = LengthSqr(vecPosition2D - vecAimTarget1);"
		"		if (flDistanceToAimSqr1 < flAimTargetRadius1*flAimTargetRadius1)"
		"		{"
		"			float flTargetColorStrength = GetAimTargetStrength(flDistanceToAimSqr1, flAimTargetRadius1, iFocusTarget == 0);"
		"			vecBaseColor = vecBaseColor * (1.0-flTargetColorStrength) + vec4(0.8, 0.0, 0.0, 1.0) * flTargetColorStrength;"
		"		}"
		"	}"

		"	if (iAimTargets > 1)"
		"	{"
		"		float flDistanceToAimSqr2 = LengthSqr(vecPosition2D - vecAimTarget2);"
		"		if (flDistanceToAimSqr2 < flAimTargetRadius2*flAimTargetRadius2)"
		"		{"
		"			float flTargetColorStrength = GetAimTargetStrength(flDistanceToAimSqr2, flAimTargetRadius2, iFocusTarget == 1);"
		"			vecBaseColor = vecBaseColor * (1.0-flTargetColorStrength) + vec4(0.8, 0.0, 0.0, 1.0) * flTargetColorStrength;"
		"		}"
		"	}"

		"	if (iAimTargets > 2)"
		"	{"
		"		float flDistanceToAimSqr3 = LengthSqr(vecPosition2D - vecAimTarget3);"
		"		if (flDistanceToAimSqr3 < flAimTargetRadius3*flAimTargetRadius3)"
		"		{"
		"			float flTargetColorStrength = GetAimTargetStrength(flDistanceToAimSqr3, flAimTargetRadius3, iFocusTarget == 2);"
		"			vecBaseColor = vecBaseColor * (1.0-flTargetColorStrength) + vec4(0.8, 0.0, 0.0, 1.0) * flTargetColorStrength;"
		"		}"
		"	}"

		"	if (iAimTargets > 3)"
		"	{"
		"		float flDistanceToAimSqr4 = LengthSqr(vecPosition2D - vecAimTarget4);"
		"		if (flDistanceToAimSqr4 < flAimTargetRadius4*flAimTargetRadius4)"
		"		{"
		"			float flTargetColorStrength = GetAimTargetStrength(flDistanceToAimSqr4, flAimTargetRadius4, iFocusTarget == 3);"
		"			vecBaseColor = vecBaseColor * (1.0-flTargetColorStrength) + vec4(0.8, 0.0, 0.0, 1.0) * flTargetColorStrength;"
		"		}"
		"	}"

		"	if (iAimTargets > 4)"
		"	{"
		"		float flDistanceToAimSqr5 = LengthSqr(vecPosition2D - vecAimTarget5);"
		"		if (flDistanceToAimSqr5 < flAimTargetRadius5*flAimTargetRadius5)"
		"		{"
		"			float flTargetColorStrength = GetAimTargetStrength(flDistanceToAimSqr5, flAimTargetRadius5, iFocusTarget == 4);"
		"			vecBaseColor = vecBaseColor * (1.0-flTargetColorStrength) + vec4(0.8, 0.0, 0.0, 1.0) * flTargetColorStrength;"
		"		}"
		"	}"

		"	gl_FragColor = vecBaseColor;"
		"}";
}
