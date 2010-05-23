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
