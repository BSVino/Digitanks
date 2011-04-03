#ifndef DT_TEXTURE_LIBRARY_H
#define DT_TEXTURE_LIBRARY_H

#include <EASTL/map.h>
#include <EASTL/string.h>

class CTextureLibrary
{
public:
							CTextureLibrary();
							~CTextureLibrary();

public:
	static size_t			GetNumTextures() { return Get()->m_aiTextures.size(); }

	static size_t			AddTexture(const eastl::string16& sTexture, int iClamp = 0);
	static size_t			FindTexture(const eastl::string16& sTexture);

	static size_t			GetNumTexturesLoaded() { return Get()->m_aiTextures.size(); };

public:
	static CTextureLibrary*	Get() { return s_pTextureLibrary; };

protected:
	eastl::map<eastl::string16, size_t>	m_aiTextures;

private:
	static CTextureLibrary*	s_pTextureLibrary;
};

#endif
