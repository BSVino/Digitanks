#define ENDL "\n"

const char* GetVSFlattenedShadowMap()
{
	return
		"varying vec4 vecShadowCoord;" ENDL
		ENDL
		"void main()" ENDL
		"{" ENDL
		"	vecShadowCoord = gl_TextureMatrix[7] * gl_Vertex;" ENDL
		"	gl_Position = gl_MultiTexCoord0;" ENDL
		"	gl_FrontColor = gl_Color;" ENDL
		"}" ENDL;
}

const char* GetFSFlattenedShadowMap()
{
	return
		"uniform sampler2D iShadowMap;" ENDL
		"varying vec4 vecShadowCoord;" ENDL
		ENDL
		"void main()" ENDL
		"{" ENDL
		ENDL
		"	float flShadow = 1.0;" ENDL
		"	if (vecShadowCoord.w > 0.0)" ENDL
		"	{" ENDL
		"		vec4 vecShadowCoordinateWdivide = vecShadowCoord / vecShadowCoord.w;" ENDL
		"		float flDistanceFromLight = texture2D(iShadowMap, vecShadowCoordinateWdivide.st).z;" ENDL
		"		vecShadowCoordinateWdivide.z -= 0.0005;" ENDL	// Reduce moire and self-shadowing
		"		flShadow = flDistanceFromLight < vecShadowCoordinateWdivide.z?0.0:1.0;" ENDL
		"	}" ENDL
		ENDL
		"	gl_FragColor = flShadow * gl_Color;" ENDL
		"}" ENDL;
}
