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
		"vec3 vecNormal;"
		"vec4 vecAmbientLight;"
		"vec4 vecDiffuseLight;"
		"vec4 vecSpecularLight;"

		"void main()"
		"{"
		"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"

		"	vecNormal = normalize(gl_NormalMatrix * gl_Normal);"
		"	vec3 vecLightDir = normalize(vec3(gl_LightSource[0].position));"

		"	float flNormalDot = dot (vecNormal, vecLightDir);"
		"	float flNormalDotHalfVector = min (0.0, dot (vecNormal, vec3(gl_LightSource[0].halfVector)));"

		"	float flPowerFactor;"
		"	if (flNormalDot == 0.0)"
		"		flPowerFactor = 0.0;"
		"	else"
		"		flPowerFactor = pow (flNormalDotHalfVector, gl_FrontMaterial.shininess);"

		"	vecAmbientLight += gl_LightSource[0].ambient;"
		"	vecDiffuseLight += gl_LightSource[0].diffuse * flNormalDot;"
		"	vecSpecularLight += gl_LightSource[0].specular * flPowerFactor;"

		"	gl_TexCoord[0] = gl_MultiTexCoord0;"

		"	gl_FrontColor = gl_FrontMaterial.emission + gl_FrontMaterial.ambient * gl_LightModel.ambient +"
		"		vecAmbientLight * gl_FrontMaterial.ambient +"
		"		vecDiffuseLight * gl_FrontMaterial.diffuse +"
		"		vecSpecularLight * gl_FrontMaterial.specular;"
		"}";
}

const char* GetFSModelShader()
{
	return
		"uniform sampler2D iDiffuseTexture;"
		"uniform sampler2D iNormalMap;"
		"uniform sampler2D iAOMap;"
		"uniform sampler2D iCAOMap;"

		"void main()"
		"{"
		"	vec4 clrFront = gl_FrontColor;"

		"	vec4 clrDiffuseColor = texture2D(iDiffuseTexture, gl_TexCoord[0].xy);"
		"	vec4 vecNormal = texture2D(iNormalMap, gl_TexCoord[0].xy);"
		"	vec4 clrAO = texture2D(iAOMap, gl_TexCoord[0].xy);"
		"	vec4 clrCAO = texture2D(iCAOMap, gl_TexCoord[0].xy);"

		"	clrFront.r = 0.0;"
		"	vec4 clrBaseColor = clrFront * clrDiffuseColor * clrAO * clrCAO;"

		"	gl_FragColor = clrBaseColor;"

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
