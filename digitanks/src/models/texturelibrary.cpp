#include "texturelibrary.h"

#include <renderer/renderer.h>

CTextureLibrary* CTextureLibrary::s_pTextureLibrary = NULL;
static CTextureLibrary g_TextureLibrary = CTextureLibrary();

CTextureLibrary::CTextureLibrary()
{
	s_pTextureLibrary = this;
}

CTextureLibrary::~CTextureLibrary()
{
	for (eastl::map<tstring, CTexture>::iterator it = m_aTextures.begin(); it != m_aTextures.end(); it++)
		CRenderer::UnloadTextureFromGL(it->second.m_iGLID);

	s_pTextureLibrary = NULL;
}

const CTexture* CTextureLibrary::AddTexture(const tstring& sTexture, int iClamp)
{
	if (!sTexture.length())
		return NULL;

	const CTexture* pTexture = FindTexture(sTexture);
	if (pTexture != NULL)
		return pTexture;

	size_t iILID = CRenderer::LoadTextureData(sTexture);
	if (iILID == 0)
		return NULL;

	CTexture oTex;
	
	oTex.m_iGLID = CRenderer::LoadTextureIntoGL(iILID, iClamp);
	oTex.m_iWidth = CRenderer::GetTextureWidth(iILID);
	oTex.m_iHeight = CRenderer::GetTextureHeight(iILID);
	CRenderer::UnloadTextureData(iILID);

	Get()->m_aTextures[sTexture] = oTex;

	return &Get()->m_aTextures[sTexture];
}

size_t CTextureLibrary::AddTextureID(const tstring& sTexture, int iClamp)
{
	const CTexture* pTex = AddTexture(sTexture, iClamp);

	if (!pTex)
		return 0;

	return pTex->m_iGLID;
}

const CTexture* CTextureLibrary::FindTexture(const tstring& sTexture)
{
	eastl::map<tstring, CTexture>::iterator it = Get()->m_aTextures.find(sTexture);
	if (it == Get()->m_aTextures.end())
		return NULL;

	return &it->second;
}

size_t CTextureLibrary::FindTextureID(const tstring& sTexture)
{
	const CTexture* pTex = FindTexture(sTexture);

	if (!pTex)
		return 0;

	return pTex->m_iGLID;
}

size_t CTextureLibrary::GetTextureGLID(const tstring& sTexture)
{
	eastl::map<tstring, CTexture>::iterator it = Get()->m_aTextures.find(sTexture);
	if (it == Get()->m_aTextures.end())
		return ~0;

	return it->second.m_iGLID;
}

size_t CTextureLibrary::GetTextureWidth(const tstring& sTexture)
{
	eastl::map<tstring, CTexture>::iterator it = Get()->m_aTextures.find(sTexture);
	if (it == Get()->m_aTextures.end())
		return 0;

	return it->second.m_iWidth;
}

size_t CTextureLibrary::GetTextureHeight(const tstring& sTexture)
{
	eastl::map<tstring, CTexture>::iterator it = Get()->m_aTextures.find(sTexture);
	if (it == Get()->m_aTextures.end())
		return 0;

	return it->second.m_iHeight;
}
