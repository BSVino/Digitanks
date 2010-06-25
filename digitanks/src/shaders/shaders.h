#ifndef DT_SHADERS_H
#define DT_SHADERS_H

#include <vector>

class CShader
{
public:
							CShader(const char* pszVS, const char* pszFS);

public:
	std::string				m_sVS;
	std::string				m_sFS;
	size_t					m_iVShader;
	size_t					m_iFShader;
	size_t					m_iProgram;
};

class CShaderLibrary
{
public:
							CShaderLibrary();
							~CShaderLibrary();

public:
	size_t					GetNumShaders() { return m_aShaders.size(); };

	void					AddShaders();
	size_t					AddShader(const char* pszVS, const char* pszFS);
	CShader*				GetShader(size_t i);

public:
	static size_t			GetProgram(size_t iProgram) { return Get()->GetShader(iProgram)->m_iProgram; };

	static const char*		GetVSPassShader();

	static const char*		GetVSTerrainShader();
	static const char*		GetFSTerrainShader();
	static size_t			GetTerrainProgram() { return GetProgram(Get()->m_iTerrain); };

	static const char*		GetVSModelShader();
	static const char*		GetFSModelShader();
	static size_t			GetModelProgram() { return GetProgram(Get()->m_iModel); };

	static const char*		GetFSExplosionShader();
	static size_t			GetExplosionProgram() { return GetProgram(Get()->m_iExplosion); };

	static const char*		GetFSBlurShader();
	static size_t			GetBlurProgram() { return GetProgram(Get()->m_iBlur); };

	static const char*		GetFSBrightPassShader();
	static size_t			GetBrightPassProgram() { return GetProgram(Get()->m_iBrightPass); };

	static void				CompileShaders();

	static CShaderLibrary*	Get() { return s_pShaderLibrary; };

protected:
	void					CompileShader(size_t iShader);

	void					ClearLog();
	void					WriteLog(const char* pszLog, const char* pszShaderText);

protected:
	std::vector<CShader>	m_aShaders;
	bool					m_bCompiled;

	size_t					m_iTerrain;
	size_t					m_iModel;
	size_t					m_iExplosion;
	size_t					m_iBlur;
	size_t					m_iBrightPass;

	bool					m_bLogNeedsClearing;

private:
	static CShaderLibrary*	s_pShaderLibrary;
};

inline const char* CShaderLibrary::GetVSPassShader()
{
	return
		"void main()"
		"{"
		"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
		"	gl_TexCoord[0] = gl_MultiTexCoord0;"
		"	gl_FrontColor = gl_Color;"
		"}";
}

#define ENDL "\n"

#define REMAPVAL \
	"float RemapVal(float flInput, float flInLo, float flInHi, float flOutLo, float flOutHi)" \
	"{" \
	"	return (((flInput-flInLo) / (flInHi-flInLo)) * (flOutHi-flOutLo)) + flOutLo;" \
	"}" \

#define LENGTHSQR \
	"float LengthSqr(vec3 v)" \
	"{" \
	"	return v.x*v.x + v.y*v.y + v.z*v.z;" \
	"}" \
	"float LengthSqr(vec2 v)" \
	"{" \
	"	return v.x*v.x + v.y*v.y;" \
	"}" \

#define LERP \
	"float Lerp(float x, float flLerp)" \
	"{" \
		"if (flLerp == 0.5)" \
			"return x;" \
		"return pow(x, log(flLerp) * -1.4427);" \
	"}" \

// Depends on LENGTHSQR
#define DISTANCE_TO_SEGMENT_SQR \
	"float DistanceToLineSegmentSqr(vec3 p, vec3 v1, vec3 v2)" \
	"{" \
		"float flResult;" \
		"vec3 v = v2 - v1;" \
		"vec3 w = p - v1;" \
		"float c1 = dot(w, v);" \
		"if (c1 < 0.0)" \
		"	flResult = LengthSqr(v1-p);" \
		"else" \
		"{" \
		"	float c2 = dot(v, v);" \
		"	if (c2 < c1)" \
		"		flResult = LengthSqr(v2-p);" \
		"	else" \
		"	{" \
		"		float b = c1/c2;" \
		"		vec3 vb = v1 + v*b;" \
		"		flResult = LengthSqr(vb - p);" \
		"	}" \
		"}" \
		"return flResult;" \
	"}" \

#define ANGLE_DIFFERENCE \
	"float AngleDifference(float a, float b)" \
	"{" \
		"float flYawDifference = a - b;" \
		"if ( a > b )" \
		"	while ( flYawDifference >= 180.0 )" \
		"		flYawDifference -= 360.0;" \
		"else" \
		"	while ( flYawDifference <= -180.0 )" \
		"		flYawDifference += 360.0;" \
		"return flYawDifference;" \
	"}" \

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

#endif