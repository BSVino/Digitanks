#ifndef DT_TEXTURE_LIBRARY_H
#define DT_TEXTURE_LIBRARY_H

#include <EASTL/map.h>

#include <tstring.h>

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

	static const CTexture*	AddTexture(const tstring& sTexture, int iClamp = 0);
	static size_t			AddTextureID(const tstring& sTexture, int iClamp = 0);
	static const CTexture*	FindTexture(const tstring& sTexture);
	static size_t			FindTextureID(const tstring& sTexture);

	static size_t			GetTextureGLID(const tstring& sTexture);
	static size_t			GetTextureWidth(const tstring& sTexture);
	static size_t			GetTextureHeight(const tstring& sTexture);

	static size_t			GetNumTexturesLoaded() { return Get()->m_aTextures.size(); };

public:
	static CTextureLibrary*	Get() { return s_pTextureLibrary; };

protected:
	eastl::map<tstring, CTexture>	m_aTextures;

private:
	static CTextureLibrary*	s_pTextureLibrary;
};

#endif
