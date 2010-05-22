#include "shaders.h"

const char* CShaderLibrary::GetVSModelShader()
{
	return
		"void main()"
		"{"
		"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
		"	gl_FrontColor = gl_Color;"
		"}";
}

const char* CShaderLibrary::GetFSModelShader()
{
	return
		"uniform float flAlpha;"

		"void main()"
		"{"
		"	gl_FragColor = gl_Color;"
		"	gl_FragColor.a = flAlpha;"
		"}";
}
