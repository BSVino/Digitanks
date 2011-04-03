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
	for (eastl::map<eastl::string16, size_t>::iterator it = m_aiTextures.begin(); it != m_aiTextures.end(); it++)
		CRenderer::UnloadTextureFromGL(it->second);

	s_pTextureLibrary = NULL;
}

size_t CTextureLibrary::AddTexture(const eastl::string16& sTexture, int iClamp)
{
	if (!sTexture.length())
		return 0;

	size_t iTexture = FindTexture(sTexture);
	if (iTexture != ~0)
		return iTexture;

	iTexture = CRenderer::LoadTextureIntoGL(sTexture, iClamp);

	Get()->m_aiTextures[sTexture] = iTexture;

	return iTexture;
}

size_t CTextureLibrary::FindTexture(const eastl::string16& sTexture)
{
	eastl::map<eastl::string16, size_t>::iterator it = Get()->m_aiTextures.find(sTexture);
	if (it == Get()->m_aiTextures.end())
		return ~0;

	return it->second;
}
