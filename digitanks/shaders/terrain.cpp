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

		"uniform vec3 vecTankOrigin;"
		"uniform float flMoveDistance;"
		"uniform bool bMovement;"

		"varying vec4 vecFrontColor;"
		"varying vec3 vecPosition;"

		"void main()"
		"{"
		"	vec4 vecBaseColor = vecFrontColor;"

		"	float flXMod = mod(vecPosition.x, 10.0);"
		"	float flZMod = mod(vecPosition.z, 10.0);"
		"	if (flXMod < 0.1f || flZMod < 0.1f)"
		"		vecBaseColor = (vecBaseColor * 0.8) + vec4(0.75, 0.75, 0.75, 1.0)*0.2;"

		"	float flYMod = mod(vecPosition.y, 10.0);"
		"	if (flYMod < 0.1f)"
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

		"	gl_FragColor = vecBaseColor;"
		"}";
}
