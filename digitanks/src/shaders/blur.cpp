#include "shaders.h"

const char* CShaderLibrary::GetFSBlurShader()
{
	return
		"uniform sampler2D iSource;"
		"uniform float aflCoefficients[3];"
		"uniform float flOffsetX;"
		"uniform float flOffsetY;"

		"void main(void)"
		"{"
		"	vec2 vecTC = gl_TexCoord[0].st;"
		"	vec2 vecOffset = vec2(flOffsetX, flOffsetY);"

		"	vec4 vecColorSum;"
		"	vecColorSum  = aflCoefficients[0] * texture2D(iSource, vecTC - vecOffset);"
		"	vecColorSum += aflCoefficients[1] * texture2D(iSource, vecTC);"
		"	vecColorSum += aflCoefficients[2] * texture2D(iSource, vecTC + vecOffset);"

		"	gl_FragColor = vecColorSum;"
		"}";
}