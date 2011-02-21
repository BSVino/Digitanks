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

const char* CShaderLibrary::GetFSCameraGuidedShader()
{
	return
		"uniform sampler2D iSource;"
		"uniform float flOffsetX;"
		"uniform float flOffsetY;"

		"void main(void)"
		"{"
		"	vec2 vecTC = gl_TexCoord[0].st;"

		"	vec4 vecColorSum;"
		"	vecColorSum  = texture2D(iSource, vecTC);"
		"	vecColorSum += texture2D(iSource, vecTC + vec2(flOffsetX, flOffsetY))/2.0;"
		"	vecColorSum += texture2D(iSource, vecTC + vec2(-flOffsetX, flOffsetY))/2.0;"
		"	vecColorSum += texture2D(iSource, vecTC + vec2(flOffsetX, -flOffsetY))/2.0;"
		"	vecColorSum += texture2D(iSource, vecTC + vec2(-flOffsetX, -flOffsetY))/2.0;"

		"	vecColorSum = vecColorSum/3.0;"

		"	float flHighest = vecColorSum.r;"
		"	if (vecColorSum.g > flHighest)"
		"		flHighest = vecColorSum.g;"
		"	if (vecColorSum.b > flHighest)"
		"		flHighest = vecColorSum.b;"

		"	gl_FragColor = vec4(flHighest, flHighest, flHighest, 1.0);"
		"}";
}
