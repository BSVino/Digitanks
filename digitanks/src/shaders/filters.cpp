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

		"	float flAverage = (vecFragmentColor.x + vecFragmentColor.y + vecFragmentColor.z)/3.0;"
		"	if (flAverage < flBrightness && flAverage > flBrightness - 0.1)"
		"	{"
		"		float flStrength = RemapVal(flAverage, flBrightness - 0.1, flBrightness, 0.0, 1.0);"
		"		vecFragmentColor = vecFragmentColor*flStrength;"
		"	}"
		"	else if (flAverage < flBrightness - 0.1)"
		"		vecFragmentColor = vec4(0.0, 0.0, 0.0, 0.0);"

		"	gl_FragColor = vecFragmentColor*flScale;"
		"}";
}
