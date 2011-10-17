#include "shaders.h"

const char* CShaderLibrary::GetFSBrightPassShader()
{
	return
		REMAPVAL

		"uniform sampler2D iSource;"
		"uniform float flBrightness;"
		"uniform float flScale;"

		"void main(void)"
		"{"
		"	vec4 vecFragmentColor = texture2D(iSource, gl_TexCoord[0].xy);"

		"	float flValue = vecFragmentColor.x;"
		"	if (vecFragmentColor.y > flValue)"
		"		flValue = vecFragmentColor.y;"
		"	if (vecFragmentColor.z > flValue)"
		"		flValue = vecFragmentColor.z;"

		"	if (flValue < flBrightness && flValue > flBrightness - 0.2)"
		"	{"
		"		float flStrength = RemapVal(flValue, flBrightness - 0.2, flBrightness, 0.0, 1.0);"
		"		vecFragmentColor = vecFragmentColor*flStrength;"
		"	}"
		"	else if (flValue < flBrightness - 0.2)"
		"		vecFragmentColor = vec4(0.0, 0.0, 0.0, 0.0);"

		"	gl_FragColor = vecFragmentColor*flScale;"
		"}";
}

const char* CShaderLibrary::GetFSDarkenShader()
{
	return
		"uniform sampler2D iDarkMap;"
		"uniform sampler2D iImage;"
		"uniform float flFactor;"

		"void main(void)"
		"{"
		"	vec4 vecDarkColor = texture2D(iDarkMap, gl_TexCoord[0].xy);"
		"	vec4 vecImageColor = texture2D(iImage, gl_TexCoord[0].xy);"

		"	vec4 vecFactor = ((vec4(1.0, 1.0, 1.0, 1.0) - vecDarkColor) * flFactor);"

		"	if (vecFactor.x < 1.0)"
		"		vecFactor.x = 1.0;"
		"	if (vecFactor.y < 1.0)"
		"		vecFactor.y = 1.0;"
		"	if (vecFactor.z < 1.0)"
		"		vecFactor.z = 1.0;"

		"	gl_FragColor = vecImageColor / vecFactor;"
		"}";
}

const char* CShaderLibrary::GetFSStencilShader()
{
	return
		"uniform sampler2D iStencilMap;"
		"uniform sampler2D iImage;"

		"void main(void)"
		"{"
		"	vec4 vecStencilColor = texture2D(iStencilMap, gl_TexCoord[0].xy);"
		"	vec4 vecImageColor = texture2D(iImage, gl_TexCoord[0].xy);"

		"	gl_FragColor = vecImageColor * vecStencilColor;"
		"}";
}
