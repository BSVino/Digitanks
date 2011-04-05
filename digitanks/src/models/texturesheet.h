#ifndef DT_TEXTURE_SHEET_H
#define DT_TEXTURE_SHEET_H

#include <EASTL/map.h>
#include <EASTL/string.h>
#include <geometry.h>

class CTextureSheet
{
public:
							CTextureSheet(eastl::string16 sFile);
							~CTextureSheet();

public:
	const Rect&				GetArea(const eastl::string& sArea) const;
	size_t					GetSheet() const { return m_iSheet; };
	size_t					GetSheetWidth() const { return m_iSheetWidth; };
	size_t					GetSheetHeight() const { return m_iSheetHeight; };

protected:
	eastl::map<eastl::string, Rect>	m_aAreas;
	size_t					m_iSheet;
	size_t					m_iSheetWidth;
	size_t					m_iSheetHeight;
};

#endif
