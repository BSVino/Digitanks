#include "shaders.h"

const char* CShaderLibrary::GetFSExplosionShader()
{
	return
		"uniform sampler2D iNoise;"
		"uniform sampler2D iExplosion;"

		"void main(void)"
		"{"
		"	vec4 vecNoise = texture2D(iNoise, gl_TexCoord[0].st);"
		"	vec4 vecExplosion = texture2D(iExplosion, gl_TexCoord[0].st);"

		"	if (vecNoise.r > vecExplosion.a)"
		"		vecExplosion.r = 0.0;"
		"	if (vecNoise.g > vecExplosion.a)"
		"		vecExplosion.g = 0.0;"
		"	if (vecNoise.b > vecExplosion.a)"
		"		vecExplosion.b = 0.0;"

		"	gl_FragColor = vecExplosion;"
		"}";
}