#ifndef DT_TEXTURE_SHEET_H
#define DT_TEXTURE_SHEET_H

#include <EASTL/map.h>
#include <EASTL/string.h>
#include <geometry.h>

class CTextureArea
{
public:
	Rect					m_rRect;
	size_t					m_iSheet;
	size_t					m_iSheetWidth;
	size_t					m_iSheetHeight;
};

class CTextureSheet
{
public:
							CTextureSheet(eastl::string16 sFile);
							~CTextureSheet();

public:
	const Rect&				GetArea(const eastl::string& sArea) const;
	size_t					GetSheet(const eastl::string& sArea) const;
	size_t					GetSheetWidth(const eastl::string& sArea) const;
	size_t					GetSheetHeight(const eastl::string& sArea) const;

protected:
	eastl::map<eastl::string, CTextureArea>	m_aAreas;
	size_t					m_iDefaultSheet;
	size_t					m_iDefaultSheetWidth;
	size_t					m_iDefaultSheetHeight;
};

#endif
