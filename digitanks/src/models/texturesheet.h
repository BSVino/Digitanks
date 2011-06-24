#ifndef DT_TEXTURE_SHEET_H
#define DT_TEXTURE_SHEET_H

#include <EASTL/map.h>

#include <tstring.h>
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
							CTextureSheet(tstring sFile);
							~CTextureSheet();

public:
	const Rect&				GetArea(const tstring& sArea) const;
	size_t					GetSheet(const tstring& sArea) const;
	size_t					GetSheetWidth(const tstring& sArea) const;
	size_t					GetSheetHeight(const tstring& sArea) const;

protected:
	eastl::map<tstring, CTextureArea>	m_aAreas;
	size_t					m_iDefaultSheet;
	size_t					m_iDefaultSheetWidth;
	size_t					m_iDefaultSheetHeight;
};

#endif
