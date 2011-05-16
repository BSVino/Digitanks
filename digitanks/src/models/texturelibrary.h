#ifndef DT_TEXTURE_LIBRARY_H
#define DT_TEXTURE_LIBRARY_H

#include <EASTL/map.h>
#include <EASTL/string.h>

class CTexture
{
public:
	size_t					m_iGLID;
	size_t					m_iWidth;
	size_t					m_iHeight;
};

class CTextureLibrary
{
public:
							CTextureLibrary();
							~CTextureLibrary();

public:
	static size_t			GetNumTextures() { return Get()->m_aTextures.size(); }

	static const CTexture*	AddTexture(const eastl::string16& sTexture, int iClamp = 0);
	static size_t			AddTextureID(const eastl::string16& sTexture, int iClamp = 0);
	static const CTexture*	FindTexture(const eastl::string16& sTexture);

	static size_t			GetTextureGLID(const eastl::string16& sTexture);
	static size_t			GetTextureWidth(const eastl::string16& sTexture);
	static size_t			GetTextureHeight(const eastl::string16& sTexture);

	static size_t			GetNumTexturesLoaded() { return Get()->m_aTextures.size(); };

public:
	static CTextureLibrary*	Get() { return s_pTextureLibrary; };

protected:
	eastl::map<eastl::string16, CTexture>	m_aTextures;

private:
	static CTextureLibrary*	s_pTextureLibrary;
};

#endif
