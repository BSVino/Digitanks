#include "shaders.h"
#include "dt_shaders.h"

const char* CShaderLibrary::GetVSPropShader()
{
	return
		"varying vec3 vecPosition;"

		"void main()"
		"{"
		"	vecPosition = vec3(gl_ModelViewMatrix * gl_Vertex);"
		"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
		"	gl_TexCoord[0] = gl_MultiTexCoord0;"
		"	gl_FrontColor = gl_Color;"
		"}";
}

const char* CShaderLibrary::GetFSPropShader()
{
	return
		REMAPVAL
		LERP
		LENGTHSQR

		"uniform bool bDiffuse;"
		"uniform sampler2D iDiffuse;"

		"uniform float flAlpha;"

		"uniform bool bColorSwapInAlpha;"
		"uniform vec3 vecColorSwap;"

		"uniform vec3 vecTankOrigin;"
		"uniform float flMoveDistance;"
		"uniform bool bMovement;"

		"varying vec3 vecPosition;"

		"void main()"
		"{"
		"	vec4 vecBaseColor = gl_Color;"

		"	if (bDiffuse)"
		"		vecBaseColor *= texture2D(iDiffuse, gl_TexCoord[0].st);"

		"	if (bColorSwapInAlpha)"
		"		vecBaseColor = vec4(vecColorSwap * vecBaseColor.a + vecBaseColor.xyz * (1.0-vecBaseColor.a), 1.0);"

		"	gl_FragColor = vecBaseColor;"
		"	gl_FragColor.a *= flAlpha;"
		"}";
}
