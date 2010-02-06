const char* GetVSFlattenedShadowMap()
{
	return
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
		"	float flLightDot = dot(vecLightNormal, normalize(vecSurfaceNormal));"

			// If the face is facing away from the light source, don't include it in the sample.
		"	if (flLightDot > 0.0)"
		"	{"
		"		gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);"
		"	}"
		"	else"
		"	{"
		"		float flShadow = 1.0;"
		"		if (vecShadowCoord.w > 0.0)"
		"		{"
		"			vec4 vecShadowCoordinateWdivide = vecShadowCoord / vecShadowCoord.w;"
		"			float flDistanceFromLight = texture2D(iShadowMap, vecShadowCoordinateWdivide.st).z;"

					// Reduce moire and self-shadowing
		"			vecShadowCoordinateWdivide.z -= 0.003 + 0.004*(1.0-flLightDot);"	// Use flLightDot to get further away from surfaces at high angles.

					// It's .99 because sometimes if every sample on a point is perfectly white it suffers integer overflow down the line and becomes black.
		"			flShadow = flDistanceFromLight < vecShadowCoordinateWdivide.z?0.0:0.99;"
		"		}"

		"		gl_FragColor = vec4(flShadow, flShadow, flShadow, 1.0);"
		"	}"
		"}";
}

const char* GetVSAOMap()
{
	return
		"varying vec2 vecUV;"

		"void main()"
		"{"
		"	gl_Position = ftransform();"
		"	gl_FrontColor = gl_Color;"
		"	vecUV = gl_MultiTexCoord0.st;"
		"}";
}

const char* GetFSAOMap()
{
	return
		"uniform sampler2D iAOMap;"
		"varying vec2 vecUV;"

		"void main()"
		"{"
		"	vec4 vecColor = texture2D(iAOMap, vecUV);"
		"	if (vecColor.a == 0.0)"	// No samples
		"		gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);"
		"	else"
		"		gl_FragColor = vec4(vecColor.x/vecColor.a, vecColor.y/vecColor.a, vecColor.z/vecColor.a, 1.0);"
		"}";
}
