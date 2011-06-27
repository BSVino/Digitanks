#include "shaders.h"

const char* CShaderLibrary::GetVSModelShader()
{
	return
		"void main()"
		"{"
		"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
		"	gl_TexCoord[0] = gl_MultiTexCoord0;"
		"	gl_FrontColor = gl_Color;"
		"}";
}

const char* CShaderLibrary::GetFSModelShader()
{
	return
		"uniform bool bDiffuse;"
		"uniform sampler2D iDiffuse;"

		"uniform float flAlpha;"

		"uniform bool bColorSwapInAlpha;"
		"uniform vec3 vecColorSwap;"

		"void main()"
		"{"
		"	vec4 vecDiffuse = gl_Color;"

		"	if (bDiffuse)"
		"		vecDiffuse *= texture2D(iDiffuse, gl_TexCoord[0].st);"

		"	if (bColorSwapInAlpha)"
		"		vecDiffuse = vec4(vecColorSwap * vecDiffuse.a + vecDiffuse.xyz * (1.0-vecDiffuse.a), 1.0);"

		"	gl_FragColor = vecDiffuse;"
		"	gl_FragColor.a *= flAlpha;"
		"}";
}

const char* CShaderLibrary::GetVSScrollingTextureShader()
{
	return
		"uniform float flTime;"
		"uniform float flSpeed;"

		"void main()"
		"{"
		"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
		"	gl_TexCoord[0] = gl_MultiTexCoord0;"
		"	gl_TexCoord[0].t -= mod(flTime*flSpeed, 1.0);"
		"	gl_FrontColor = gl_Color;"
		"}";
}

const char* CShaderLibrary::GetFSScrollingTextureShader()
{
	return
		"uniform sampler2D iTexture;"

		"uniform float flAlpha;"

		"void main()"
		"{"
		"	gl_FragColor = gl_Color * texture2D(iTexture, gl_TexCoord[0].st);"
		"	gl_FragColor.a *= flAlpha;"
		"}";
}
