#define ENDL "\n"

const char* GetVSFlattenedShadowMap()
{
	return
		"uniform sampler2D yadeya;" ENDL
		"varying vec4 vecShadowCoord;" ENDL
		"varying vec3 vecSurfaceNormal;" ENDL
		ENDL
		"void main()" ENDL
		"{" ENDL
		"	vecShadowCoord = gl_TextureMatrix[7] * gl_Vertex;" ENDL
		"	vecSurfaceNormal = gl_Normal;" ENDL
		"	gl_Position = gl_MultiTexCoord0;" ENDL
		"	gl_FrontColor = gl_Color;" ENDL
		"}" ENDL;
}

const char* GetFSFlattenedShadowMap()
{
	return
		"uniform sampler2D iShadowMap;" ENDL
		"uniform vec3 vecLightNormal;" ENDL
		"varying vec4 vecShadowCoord;" ENDL
		"varying vec3 vecSurfaceNormal;" ENDL
		ENDL
		"void main()" ENDL
		"{" ENDL
		"	float flShadow = 1.0;" ENDL
		ENDL
			// If the face is facing away from the light source, don't include it in the sample.
		"	if (dot(vecLightNormal, normalize(vecSurfaceNormal)) > 0.0)" ENDL
		"	{" ENDL
		"		gl_FragColor = vec4(1.0, 0.0, 0.0, 0.0);" ENDL
		"		return;" ENDL
		"	}" ENDL
		ENDL
		"	if (vecShadowCoord.w > 0.0)" ENDL
		"	{" ENDL
		"		vec4 vecShadowCoordinateWdivide = vecShadowCoord / vecShadowCoord.w;" ENDL
		"		float flDistanceFromLight = texture2D(iShadowMap, vecShadowCoordinateWdivide.st).z;" ENDL
//		"		vecShadowCoordinateWdivide.z -= 0.0005;" ENDL	// Reduce moire and self-shadowing
		"		flShadow = flDistanceFromLight < vecShadowCoordinateWdivide.z?0.0:1.0;" ENDL
		"	}" ENDL
		ENDL
		"	gl_FragColor = flShadow * gl_Color;" ENDL
		"}" ENDL;
}
