/*
Copyright (c) 2012, Lunar Workshop, Inc.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
3. All advertising materials mentioning features or use of this software must display the following acknowledgement:
   This product includes software developed by Lunar Workshop, Inc.
4. Neither the name of the Lunar Workshop nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY LUNAR WORKSHOP INC ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL LUNAR WORKSHOP BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef DT_SHADERS_H
#define DT_SHADERS_H

#include <tvector.h>
#include <tmap.h>

#include <tstring.h>
#include <vector.h>
#include <matrix.h>

#define MAX_TEXTURE_CHANNELS 2

typedef enum
{
	UT_NONE = 0,
	UT_FLOAT,
	UT_VECTOR2D,
	UT_VECTOR3D,
	UT_VECTOR4D,
	UT_INT,
	UT_BOOL,
	UT_MATRIX4X4,
	UT_SAMPLER,
} uniform_type_t;

class CShaderDefault
{
public:
	CShaderDefault(const char* pszUniform, uniform_type_t eType, float flValue)
	{
		TAssertNoMsg(eType == UT_FLOAT);
		m_pszUniform = pszUniform;
		m_eType = eType;
		*(float*)&m_oData[0] = flValue;
	}

	CShaderDefault(const char* pszUniform, Vector2D vecValue)
	{
		m_pszUniform = pszUniform;
		m_eType = UT_VECTOR2D;
		*(Vector2D*)&m_oData[0] = vecValue;
	}

	CShaderDefault(const char* pszUniform, Vector vecValue)
	{
		m_pszUniform = pszUniform;
		m_eType = UT_VECTOR3D;
		*(Vector*)&m_oData[0] = vecValue;
	}

	CShaderDefault(const char* pszUniform, Vector4D vecValue)
	{
		m_pszUniform = pszUniform;
		m_eType = UT_VECTOR4D;
		*(Vector4D*)&m_oData[0] = vecValue;
	}

	CShaderDefault(const char* pszUniform, uniform_type_t eType, bool vecValue)
	{
		TAssertNoMsg(eType == UT_BOOL);
		m_pszUniform = pszUniform;
		m_eType = eType;
		*(bool*)&m_oData[0] = vecValue;
	}

	CShaderDefault(const char* pszUniform, Matrix4x4 mValue)
	{
		m_pszUniform = pszUniform;
		m_eType = UT_MATRIX4X4;
		*(Matrix4x4*)&m_oData[0] = mValue;
	}

	CShaderDefault(const char* pszUniform, uniform_type_t eType, int iValue)
	{
		TAssertNoMsg(eType == UT_INT || eType == UT_SAMPLER);
		m_pszUniform = pszUniform;
		m_eType = eType;
		*(int*)&m_oData[0] = iValue;
	}

public:
	float     GetFloat()     { TAssertNoMsg(m_eType == UT_FLOAT);     return *(float*)&m_oData[0]; }
	Vector2D  GetVector2D()  { TAssertNoMsg(m_eType == UT_VECTOR2D);  return *(Vector2D*)&m_oData[0]; }
	Vector    GetVector3D()  { TAssertNoMsg(m_eType == UT_VECTOR3D);  return *(Vector*)&m_oData[0]; }
	Vector4D  GetVector4D()  { TAssertNoMsg(m_eType == UT_VECTOR4D);  return *(Vector4D*)&m_oData[0]; }
	Matrix4x4 GetMatrix4x4() { TAssertNoMsg(m_eType == UT_MATRIX4X4); return *(Matrix4x4*)&m_oData[0]; }
	bool      GetBool()      { TAssertNoMsg(m_eType == UT_BOOL);      return *(bool*)&m_oData[0]; }
	int       GetInt()       { TAssertNoMsg(m_eType == UT_INT);       return *(int*)&m_oData[0]; }
	int       GetSampler()   { TAssertNoMsg(m_eType == UT_SAMPLER);   return *(int*)&m_oData[0]; }

public:
	const char*    m_pszUniform;
	uniform_type_t m_eType;
	char           m_oData[sizeof(Matrix4x4)];
};

class CShader
{
public:
							CShader(const tstring& sName, const tstring& sVertexFile, const tstring& sFragmentFile);

public:
	bool					Compile();
	void					Destroy();

	tstring					FindType(const tstring& sName) const;

	size_t					FindTextureByUniform(const tstring& sUniform) const;

public:
	tstring					m_sName;
	tstring					m_sVertexFile;
	tstring					m_sFragmentFile;
	size_t					m_iVShader;
	size_t					m_iFShader;
	size_t					m_iProgram;

	size_t					m_iPositionAttribute;
	size_t					m_iNormalAttribute;
	size_t					m_iTangentAttribute;
	size_t					m_iBitangentAttribute;
	size_t					m_aiTexCoordAttributes[MAX_TEXTURE_CHANNELS];
	size_t					m_iColorAttribute;

	class CParameter
	{
	public:
		tstring				m_sName;

		class CUniform
		{
		public:
			tstring			m_sName;
			union
			{
				float		m_flValue;
				int			m_iValue;
				bool		m_bValue;
			};
			Vector2D		m_vec2Value;
			Vector			m_vecValue;
			Vector4D		m_vec4Value;
			tstring			m_sValue;
			bool			m_bTexture;
		};

		tvector<CUniform>	m_aActions;

		tstring				m_sBlend;
	};

	class CUniform
	{
	public:
		tstring					m_sUniformType;
		CParameter::CUniform*	m_pDefault;
	};

	tmap<tstring, CParameter>			m_aParameters;	// What the shader.txt has.
	tmap<tstring, CUniform>				m_asUniforms;	// What the hardware has. Values are types.
	tmap<tstring, CParameter::CUniform>	m_aDefaults;	// Defaults for each uniform as specified by shader .txt (not GLSL)
	tvector<tstring>					m_asTextures;	// List of textures for purposes of assigning to channels and whatnot.

	tvector<CShaderDefault> m_aDefaultsBuffer;
};

class CShaderLibrary
{
	friend class CShader;

public:
							CShaderLibrary();
							~CShaderLibrary();

public:
	static void             Initialize();
	void                    InitializeNonStatic();
	static void             Destroy();
	void                    DestroyNonStatic();

	static size_t			GetNumShaders() { return Get()->m_aShaders.size(); };

	static CShader*			GetShader(const tstring& sName);
	static CShader*			GetShader(size_t i);

	static void				AddShader(const tstring& sFile);

	static size_t			GetProgram(const tstring& sName);

	static void				CompileShaders(int iSamples = -1);
	static void				DestroyShaders();

	static bool				IsCompiled() { return Get()->m_bCompiled; };

	static tstring			GetShaderHeader() { return Get()->m_sHeader; }
	static tstring			GetShaderFunctions() { return Get()->m_sFunctions; }

	static CShaderLibrary*	Get() { return s_pShaderLibrary; };

protected:
	void					ClearLog();
	void					WriteLog(const tstring& sFile, const char* pszLog, const char* pszShaderText);

protected:
	tmap<tstring, size_t>	m_aShaderNames;
	tvector<CShader>		m_aShaders;
	bool					m_bCompiled;

	bool					m_bLogNeedsClearing;

	int						m_iSamples;

	tstring					m_sHeader;
	tstring					m_sFunctions;
	tstring					m_sMain;

private:
	static CShaderLibrary*	s_pShaderLibrary;
};

#endif
