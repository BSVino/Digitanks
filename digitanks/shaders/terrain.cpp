#include "shaders.h"

const char* CShaderLibrary::GetVSTerrainShader()
{
	return
		"varying vec4 vecFrontColor;"
		"varying vec3 vecPosition;"

		"void main()"
		"{"
		"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
		"	vecPosition = gl_Vertex.xyz;"
		"	gl_FrontColor = vecFrontColor = gl_Color;"
		"}";
}

const char* CShaderLibrary::GetFSTerrainShader()
{
	return
		REMAPVAL
		LERP
		LENGTHSQR
		DISTANCE_TO_SEGMENT_SQR

		"uniform vec3 vecTankOrigin;"

		"uniform float flMoveDistance;"
		"uniform bool bMovement;"

		"uniform vec3 vecTurnPosition;"
		"uniform bool bTurnValid;"
		"uniform float flTankYaw;"
		"uniform float flTankMaxYaw;"
		"uniform bool bTurning;"

		"varying vec4 vecFrontColor;"
		"varying vec3 vecPosition;"

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

		"	if (bMovement)"
		"	{"
		"		float flTankDistanceSqr = LengthSqr(vecPosition - vecTankOrigin);"
		"		if (flTankDistanceSqr <= flMoveDistance*flMoveDistance)"
		"		{"
		"			float flMoveColorStrength = RemapVal(flTankDistanceSqr, 0.0, flMoveDistance*flMoveDistance, 0.0, 1.0);"
		"			flMoveColorStrength = Lerp(flMoveColorStrength, 0.2);"
		"			vecBaseColor = vecBaseColor * (1.0-flMoveColorStrength) + vec4(1.0, 1.0, 0.0, 1.0) * flMoveColorStrength;"
		"		}"
		"	}"

		"	if (bTurning)"
		"	{"

		"		float flInside = 3.0;"
		"		float flOutside = 10.0;"

				// Draw the turning indicator
		"		float flTankDistanceSqr = LengthSqr(vecPosition - vecTankOrigin);"
		"		if (flTankDistanceSqr > flInside*flInside && flTankDistanceSqr < flOutside*flOutside)"
		"		{"
		"			vec3 vecDirection = normalize(vecPosition - vecTankOrigin);"
		"			float flYaw = atan(vecDirection.z, vecDirection.x) * 180.0/3.14159;"

		"			float flYawDifference = flYaw - flTankYaw;"
		"			if ( flYaw > flTankYaw )"
		"				while ( flYawDifference >= 180.0 )"
		"					flYawDifference -= 360.0;"
		"			else"
		"				while ( flYawDifference <= -180.0 )"
		"					flYawDifference += 360.0;"

		"			if (abs(flYawDifference) < flTankMaxYaw)"
		"			{"
		"				float flMoveColorStrength = RemapVal(flTankDistanceSqr, flInside*flInside, flOutside*flOutside, 1.0, 0.0);"
		"				flMoveColorStrength = Lerp(flMoveColorStrength, 0.1);"
		"				vecBaseColor = vecBaseColor * (1.0-flMoveColorStrength) + vec4(1.0, 1.0, 0.0, 1.0) * flMoveColorStrength;"
		"			}"
		"		}"

		"		if (flTankDistanceSqr > flInside*flInside)"
		"		{"
		"			float flInner = 0.5;"
		"			float flOuter = 1.0;"

		"			float flTurnDistanceSqr = LengthSqr(vecPosition - vecTurnPosition);"

		"			vec4 vecTurnColor = vec4(1.0, 1.0, 0.0, 1.0);"
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

		"	gl_FragColor = vecBaseColor;"
		"}";
}
