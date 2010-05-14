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

	size_t					AddShader(const char* pszVS, const char* pszFS);
	CShader*				GetShader(size_t i);

public:
	static size_t			GetProgram(size_t iProgram) { return Get()->GetShader(iProgram)->m_iProgram; };

	static const char*		GetVSTerrainShader();
	static const char*		GetFSTerrainShader();
	static size_t			GetTerrainProgram() { return GetProgram(Get()->m_iTerrain); };

	static void				CompileShaders();

	static CShaderLibrary*	Get() { return s_pShaderLibrary; };

protected:
	void					CompileShader(size_t iShader);

protected:
	std::vector<CShader>	m_aShaders;
	bool					m_bCompiled;

	size_t					m_iTerrain;

private:
	static CShaderLibrary*	s_pShaderLibrary;
};

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

#define LERP \
	"uniform float flLastLerp = -1;" \
	"uniform float flLastExp = -1;" \
	"float Lerp(float x, float flLerp)" \
	"{" \
		"if (flLerp == 0.5f)" \
			"return x;" \
		"if (flLastLerp != flLerp)" \
			"flLastExp = log(flLerp) * -1.4427f;" \
		"return pow(x, flLastExp);" \
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