const char* GetVSFlattenedShadowMap()
{
	return
		"uniform sampler2D yadeya;"
		"varying vec4 vecShadowCoord;"
		"varying vec3 vecSurfaceNormal;"

		"void main()"
		"{"
		"	vecShadowCoord = gl_TextureMatrix[7] * gl_Vertex;"
		"	vecSurfaceNormal = gl_Normal;"
		"	gl_Position = gl_MultiTexCoord0;"
		"	gl_FrontColor = gl_Color;"
		"}";
}

const char* GetFSFlattenedShadowMap()
{
	return
		"uniform sampler2D iShadowMap;"
		"uniform vec3 vecLightNormal;"
		"varying vec4 vecShadowCoord;"
		"varying vec3 vecSurfaceNormal;"

		"void main()"
		"{"
		"	float flShadow = 1.0;"
		"	float flLightDot = dot(vecLightNormal, normalize(vecSurfaceNormal));"

			// If the face is facing away from the light source, don't include it in the sample.
		"	if (flLightDot > 0.0)"
		"	{"
		"		gl_FragColor = vec4(1.0, 0.0, 0.0, 0.0);"
		"		return;"
		"	}"

		"	if (vecShadowCoord.w > 0.0)"
		"	{"
		"		vec4 vecShadowCoordinateWdivide = vecShadowCoord / vecShadowCoord.w;"
		"		float flDistanceFromLight = texture2D(iShadowMap, vecShadowCoordinateWdivide.st).z;"

				// Reduce moire and self-shadowing
		"		vecShadowCoordinateWdivide.z -= 0.003 + 0.004*(1-flLightDot);"	// Use flLightDot to get further away from surfaces at high angles.

		"		flShadow = flDistanceFromLight < vecShadowCoordinateWdivide.z?0.0:1.0;"
		"	}"

		"	gl_FragColor = flShadow * gl_Color;"
		"}";
}
