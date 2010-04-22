#define ENDL "\n"

/*
	struct gl_LightSourceParameters {
		vec4 ambient; 
		vec4 diffuse; 
		vec4 specular; 
		vec4 position; 
		vec4 halfVector; 
		vec3 spotDirection; 
		float spotExponent; 
		float spotCutoff; // (range: [0.0,90.0], 180.0)
		float spotCosCutoff; // (range: [1.0,0.0],-1.0)
		float constantAttenuation; 
		float linearAttenuation; 
		float quadraticAttenuation;	
	};

	uniform gl_LightSourceParameters gl_LightSource[gl_MaxLights];

	struct gl_LightModelParameters {
		vec4 ambient; 
	};

	uniform gl_LightModelParameters gl_LightModel;

	struct gl_MaterialParameters {
		vec4 emission;   
		vec4 ambient;    
		vec4 diffuse;    
		vec4 specular;   
		float shininess; 
	};

	uniform gl_MaterialParameters gl_FrontMaterial;
	uniform gl_MaterialParameters gl_BackMaterial;
*/

const char* GetVSModelShader()
{
	return
		"varying vec3 vecVertexNormal;"
		"varying vec3 vecVertexTangent;"
		"varying vec3 vecVertexBitangent;"

		"varying vec3 vecLightDir;"
		"varying vec3 vecLightHalf;"
		"varying vec3 vecViewDir;"

		"varying vec4 vecAmbientLight;"
		"varying vec4 vecDiffuseLight;"
		"varying vec4 vecSpecularLight;"

		"attribute vec3 vecTangent;"
		"attribute vec3 vecBitangent;"

		"void main()"
		"{"
		"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"

		"	vecVertexNormal = normalize(gl_Normal);"
		"	vecVertexTangent = normalize(vecTangent);"
		"	vecVertexBitangent = normalize(vecBitangent);"

		"	vecLightDir = normalize(gl_LightSource[0].position.xyz);"
		"	vecLightHalf = gl_LightSource[0].halfVector.xyz;"
		"	vecViewDir = normalize(-vec3(gl_ModelViewMatrix * gl_Vertex));"

		"	vecAmbientLight = gl_FrontMaterial.emission + gl_FrontMaterial.ambient * gl_LightModel.ambient + gl_LightSource[0].ambient * gl_FrontMaterial.ambient;"
		"	vecDiffuseLight = gl_LightSource[0].diffuse * gl_FrontMaterial.diffuse;"
		"	vecSpecularLight = gl_LightSource[0].specular * gl_FrontMaterial.specular;"

		"	gl_TexCoord[0] = gl_MultiTexCoord0;"
		"}";
}

const char* GetFSModelShader()
{
	return
		"varying vec3 vecVertexNormal;"
		"varying vec3 vecVertexTangent;"
		"varying vec3 vecVertexBitangent;"

		"varying vec3 vecLightDir;"
		"varying vec3 vecLightHalf;"
		"varying vec3 vecViewDir;"

		"varying vec4 vecAmbientLight;"
		"varying vec4 vecDiffuseLight;"
		"varying vec4 vecSpecularLight;"

		"uniform sampler2D iDiffuseTexture;"
		"uniform sampler2D iNormalMap;"
		"uniform sampler2D iNormal2Map;"
		"uniform sampler2D iAOMap;"
		"uniform sampler2D iCAOMap;"

		"uniform bool bLighting;"
		"uniform bool bDiffuseTexture;"
		"uniform bool bNormalMap;"
		"uniform bool bNormal2Map;"
		"uniform bool bAOMap;"
		"uniform bool bCAOMap;"

		"void main()"
		"{"
		"	vec4 clrDiffuseColor = vec4(1.0, 1.0, 1.0, 1.0);"
		"	vec3 vecTangentNormal = vec3(0.0, 0.0, 0.0);"
		"	vec3 vecTranslatedNormal;"
		"	vec4 clrAO = vec4(1.0, 1.0, 1.0, 1.0);"
		"	vec4 clrCAO = vec4(1.0, 1.0, 1.0, 1.0);"
		"	vec4 clrLight;"

		"	if (bDiffuseTexture)"
		"		clrDiffuseColor = texture2D(iDiffuseTexture, gl_TexCoord[0].xy);"

		"	if (!bNormalMap && !bNormal2Map)"
		"		vecTangentNormal = vec3(0.0, 0.0, 1.0);"

		"	if (bNormalMap)"
		"		vecTangentNormal = normalize(texture2D(iNormalMap, gl_TexCoord[0].xy).xyz * 2.0 - 1.0);"

		"	vec3 vecNormal2;"
		"	if (bNormal2Map)"
		"	{"
		"		vecNormal2 = normalize(texture2D(iNormal2Map, gl_TexCoord[0].xy).xyz * 2.0 - 1.0);"
		"		if (bNormalMap)"
		"		{"
					// Transform the normal to the tangent space of the first normal map's normal
		"			vec3 vecSmoothBitangent = normalize(cross(vecTangentNormal, vec3(1, 0, 0)));"
		"			vec3 vecSmoothTangent = normalize(cross(vecSmoothBitangent, vecTangentNormal));"
		"			mat3 mTBN = mat3(vecSmoothTangent, vecSmoothBitangent, vecTangentNormal);"
		"			vecTangentNormal = mTBN * vecNormal2;"
		"		}"
		"		else"
		"		{"
		"			vecTangentNormal = vecNormal2;"
		"		}"
		"	}"

		"	if (bAOMap)"
		"		clrAO = texture2D(iAOMap, gl_TexCoord[0].xy);"

		"	if (bCAOMap)"
		"		clrCAO = texture2D(iCAOMap, gl_TexCoord[0].xy);"

		"	if (bLighting)"
		"	{"
		"		if (bNormalMap || bNormal2Map)"
		"		{"
					// If we are in normal map mode, vecVertexNormal is really part of the inverse TBN matrix
		"			mat3 mTBN = mat3(normalize(vecVertexTangent), normalize(vecVertexBitangent), normalize(vecVertexNormal));"
		"			vecTranslatedNormal = normalize(gl_NormalMatrix * (mTBN * vecTangentNormal));"
		"		}"
		"		else"
					// If we are not in normal map mode, vecVertexNormal is just a normal normal
		"			vecTranslatedNormal = normalize(gl_NormalMatrix * vecVertexNormal);"

		"		float flLightStrength = dot(vecTranslatedNormal, vecLightDir);"
		"		if (flLightStrength < 0.0) flLightStrength = 0.0;"
		"		if (flLightStrength > 1.0) flLightStrength = 1.0;"

		"		float flNormalDotHalfVector = max(0.0, dot(vecTranslatedNormal, normalize(vecLightHalf)));"

		"		float flPowerFactor = 0.0;"
		"		if (flLightStrength > 0.0)"
		"			flPowerFactor = pow(flNormalDotHalfVector, gl_FrontMaterial.shininess);"

		"		clrLight = vecAmbientLight +"
		"			vecDiffuseLight * flLightStrength +"
		"			vecSpecularLight * flPowerFactor;"
		"	}"
		"	else"
		"	{"
		"		if (bNormalMap || bNormal2Map)"
		"		{"
		"			mat3 mTBN = mat3(normalize(vecVertexTangent), normalize(vecVertexBitangent), normalize(vecVertexNormal));"
		"			vecTranslatedNormal = normalize(gl_NormalMatrix * (mTBN * vecTangentNormal));"
		"		}"
		"		else"
		"			vecTranslatedNormal = normalize(gl_NormalMatrix * vecVertexNormal);"

		"		float flDot = dot(vecTranslatedNormal, vec3(0, 1, 0));"
		"		clrLight = vec4(1, 1, 1, 1) * (flDot * 0.5) + vec4(0.45, 0.45, 0.45, 0.45);"
		"		clrLight = clrLight * gl_FrontMaterial.diffuse;"
		"	}"

		"	gl_FragColor = clrLight * clrDiffuseColor * clrAO * clrCAO;"

		"	gl_FragColor.a = clrDiffuseColor.a;"
		"}";
}

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
		"uniform bool bOccludeAll;"
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
		"		if (bOccludeAll)"
		"			flShadow = 0.0;"
		"		else if (vecShadowCoord.w > 0.0)"
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
