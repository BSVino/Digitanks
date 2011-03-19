#include "shaders.h"

#include <GL/glew.h>
#include <assert.h>
#include <time.h>

#include <platform.h>

CShaderLibrary* CShaderLibrary::s_pShaderLibrary = NULL;
static CShaderLibrary g_ShaderLibrary = CShaderLibrary();

CShaderLibrary::CShaderLibrary()
{
	s_pShaderLibrary = this;

	m_bCompiled = false;
}

CShaderLibrary::~CShaderLibrary()
{
	for (size_t i = 0; i < m_aShaders.size(); i++)
	{
		CShader* pShader = &m_aShaders[i];
		glDetachShader((GLuint)pShader->m_iProgram, (GLuint)pShader->m_iVShader);
		glDetachShader((GLuint)pShader->m_iProgram, (GLuint)pShader->m_iFShader);
		glDeleteProgram((GLuint)pShader->m_iProgram);
		glDeleteShader((GLuint)pShader->m_iVShader);
		glDeleteShader((GLuint)pShader->m_iFShader);
	}

	s_pShaderLibrary = NULL;
}

void CShaderLibrary::AddShaders()
{
	m_iTerrain = AddShader(GetVSTerrainShader(), GetFSTerrainShader());
	m_iModel = AddShader(GetVSModelShader(), GetFSModelShader());
	m_iProp = AddShader(GetVSPropShader(), GetFSPropShader());
	m_iScrollingTexture = AddShader(GetVSScrollingTextureShader(), GetFSScrollingTextureShader());

	m_iExplosion = AddShader(GetVSPassShader(), GetFSExplosionShader());
	m_iBlur = AddShader(GetVSPassShader(), GetFSBlurShader());
	m_iBrightPass = AddShader(GetVSPassShader(), GetFSBrightPassShader());
	m_iDarken = AddShader(GetVSPassShader(), GetFSDarkenShader());
	m_iStencil = AddShader(GetVSPassShader(), GetFSStencilShader());
	m_iCameraGuided = AddShader(GetVSPassShader(), GetFSCameraGuidedShader());
}

size_t CShaderLibrary::AddShader(const char* pszVS, const char* pszFS)
{
	if (m_bCompiled)
		return ~0;

	m_aShaders.push_back(CShader(pszVS, pszFS));
	return m_aShaders.size()-1;
}

void CShaderLibrary::CompileShaders()
{
	if (Get()->m_bCompiled)
		return;

	Get()->AddShaders();

	Get()->ClearLog();

	bool bShadersCompiled = true;
	for (size_t i = 0; i < Get()->m_aShaders.size(); i++)
		bShadersCompiled &= Get()->CompileShader(i);

	if (bShadersCompiled)
		Get()->m_bCompiled = true;
	else
		DestroyShaders();
}

void CShaderLibrary::DestroyShaders()
{
	for (size_t i = 0; i < Get()->m_aShaders.size(); i++)
		Get()->DestroyShader(i);
}

bool CShaderLibrary::CompileShader(size_t iShader)
{
	CShader* pShader = &m_aShaders[iShader];

	pShader->m_iVShader = glCreateShader(GL_VERTEX_SHADER);
	const char* pszStr = pShader->m_sVS.c_str();
	glShaderSource((GLuint)pShader->m_iVShader, 1, &pszStr, NULL);
	glCompileShader((GLuint)pShader->m_iVShader);

	int iLogLength = 0;
	char szLog[1024];
	glGetShaderInfoLog((GLuint)pShader->m_iVShader, 1024, &iLogLength, szLog);
	WriteLog(szLog, pszStr);

	int iVertexCompiled;
	glGetShaderiv((GLuint)pShader->m_iVShader, GL_COMPILE_STATUS, &iVertexCompiled);

	pShader->m_iFShader = glCreateShader(GL_FRAGMENT_SHADER);
	pszStr = pShader->m_sFS.c_str();
	glShaderSource((GLuint)pShader->m_iFShader, 1, &pszStr, NULL);
	glCompileShader((GLuint)pShader->m_iFShader);

	szLog[0] = '\0';
	iLogLength = 0;
	glGetShaderInfoLog((GLuint)pShader->m_iFShader, 1024, &iLogLength, szLog);
	WriteLog(szLog, pszStr);

	int iFragmentCompiled;
	glGetShaderiv((GLuint)pShader->m_iFShader, GL_COMPILE_STATUS, &iFragmentCompiled);

	pShader->m_iProgram = glCreateProgram();
	glAttachShader((GLuint)pShader->m_iProgram, (GLuint)pShader->m_iVShader);
	glAttachShader((GLuint)pShader->m_iProgram, (GLuint)pShader->m_iFShader);
	glLinkProgram((GLuint)pShader->m_iProgram);

	szLog[0] = '\0';
	iLogLength = 0;
	glGetProgramInfoLog((GLuint)pShader->m_iProgram, 1024, &iLogLength, szLog);
	WriteLog(szLog, "link");

	int iProgramLinked;
	glGetProgramiv((GLuint)pShader->m_iProgram, GL_LINK_STATUS, &iProgramLinked);

	assert(iVertexCompiled == GL_TRUE && iFragmentCompiled == GL_TRUE && iProgramLinked == GL_TRUE);
	if (iVertexCompiled == GL_TRUE && iFragmentCompiled == GL_TRUE && iProgramLinked == GL_TRUE)
		return true;
	else
		return false;
}

void CShaderLibrary::DestroyShader(size_t iShader)
{
	CShader* pShader = &m_aShaders[iShader];

	glDetachShader((GLuint)pShader->m_iProgram, (GLuint)pShader->m_iVShader);
	glDetachShader((GLuint)pShader->m_iProgram, (GLuint)pShader->m_iFShader);
	glDeleteShader((GLuint)pShader->m_iVShader);
	glDeleteShader((GLuint)pShader->m_iFShader);
	glDeleteProgram((GLuint)pShader->m_iProgram);
}

void CShaderLibrary::ClearLog()
{
	m_bLogNeedsClearing = true;
}

void CShaderLibrary::WriteLog(const char* pszLog, const char* pszShaderText)
{
	if (!pszLog || strlen(pszLog) == 0)
		return;

	eastl::string16 sFile = GetAppDataDirectory(L"Digitanks", L"shaders.txt");

	if (m_bLogNeedsClearing)
	{
		// Only clear it if we're actually going to write to it so we don't create the file.
		FILE* fp = _wfopen(sFile.c_str(), L"w");
		fclose(fp);
		m_bLogNeedsClearing = false;
	}

	char szText[100];
	strncpy(szText, pszShaderText, 99);
	szText[99] = '\0';

	FILE* fp = _wfopen(sFile.c_str(), L"a");
	fprintf(fp, "Shader compile output %d\n", time(NULL));
	fprintf(fp, "%s\n\n", pszLog);
	fprintf(fp, "%s...\n\n", szText);
	fclose(fp);
}

CShader* CShaderLibrary::GetShader(size_t i)
{
	if (i >= m_aShaders.size())
		return NULL;

	return &m_aShaders[i];
}

CShader::CShader(const char* pszVS, const char* pszFS)
{
	m_sVS = pszVS;
	m_sFS = pszFS;
	m_iVShader = 0;
	m_iFShader = 0;
	m_iProgram = 0;
}
