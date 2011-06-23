#include "texturesheet.h"

#include <iostream>
#include <fstream>

#include <strutils.h>

#include <datamanager/dataserializer.h>
#include <renderer/renderer.h>
#include <models/texturelibrary.h>

CTextureSheet::CTextureSheet(tstring sFile)
{
	std::basic_ifstream<tchar> f(convertstring<tchar, char>(sFile).c_str());

	CData* pFile = new CData();
	CDataSerializer::Read(f, pFile);

	for (size_t i = 0; i < pFile->GetNumChildren(); i++)
	{
		CData* pChild = pFile->GetChild(i);
		if (pChild->GetKey() == _T("Texture"))
		{
			tstring sTexture = pChild->GetValueTString();
			const CTexture* pTex = CTextureLibrary::AddTexture(sTexture);
			m_iDefaultSheet = pTex->m_iGLID;
			m_iDefaultSheetWidth = pTex->m_iWidth;
			m_iDefaultSheetHeight = pTex->m_iHeight;
		}
		else if (pChild->GetKey() == _T("Area"))
		{
			int x = 0;
			int y = 0;
			int w = 0;
			int h = 0;

			CData* pData = pChild->FindChild(_T("x"));
			if (pData)
				x = pData->GetValueInt();

			pData = pChild->FindChild(_T("y"));
			if (pData)
				y = pData->GetValueInt();

			pData = pChild->FindChild(_T("w"));
			if (pData)
				w = pData->GetValueInt();

			pData = pChild->FindChild(_T("h"));
			if (pData)
				h = pData->GetValueInt();

			m_aAreas[pChild->GetValueTString()].m_rRect = Rect(x, y, w, h);

			m_aAreas[pChild->GetValueTString()].m_iSheet = ~0;
			pData = pChild->FindChild(_T("Texture"));
			if (pData)
			{
				tstring sTexture = pData->GetValueTString();
				const CTexture* pTex = CTextureLibrary::AddTexture(sTexture);
				m_aAreas[pChild->GetValueTString()].m_iSheet = pTex->m_iGLID;
				m_aAreas[pChild->GetValueTString()].m_iSheetWidth = pTex->m_iWidth;
				m_aAreas[pChild->GetValueTString()].m_iSheetHeight = pTex->m_iHeight;
			}
		}
	}

	delete pFile;
}

CTextureSheet::~CTextureSheet()
{
}

const Rect& CTextureSheet::GetArea(const tstring& sArea) const
{
	eastl::map<tstring, CTextureArea>::const_iterator it = m_aAreas.find(sArea);

	if (it == m_aAreas.end())
	{
		static Rect empty(0,0,0,0);
		return empty;
	}

	return it->second.m_rRect;
}

size_t CTextureSheet::GetSheet(const tstring& sArea) const
{
	eastl::map<tstring, CTextureArea>::const_iterator it = m_aAreas.find(sArea);

	if (it == m_aAreas.end())
		return 0;

	size_t iSheet = it->second.m_iSheet;
	if (iSheet == ~0)
		return m_iDefaultSheet;

	return iSheet;
}

size_t CTextureSheet::GetSheetWidth(const tstring& sArea) const
{
	eastl::map<tstring, CTextureArea>::const_iterator it = m_aAreas.find(sArea);

	if (it == m_aAreas.end())
		return 0;

	size_t iSheet = it->second.m_iSheet;
	if (iSheet == ~0)
		return m_iDefaultSheetWidth;

	return it->second.m_iSheetWidth;
}

size_t CTextureSheet::GetSheetHeight(const tstring& sArea) const
{
	eastl::map<tstring, CTextureArea>::const_iterator it = m_aAreas.find(sArea);

	if (it == m_aAreas.end())
		return 0;

	size_t iSheet = it->second.m_iSheet;
	if (iSheet == ~0)
		return m_iDefaultSheetHeight;

	return it->second.m_iSheetHeight;
}

const Rect& CTextureSheet::GetArea(const eastl::string& sArea) const
{
	return GetArea(convertstring<char, tchar>(sArea));
}

size_t CTextureSheet::GetSheet(const eastl::string& sArea) const
{
	return GetSheet(convertstring<char, tchar>(sArea));
}

size_t CTextureSheet::GetSheetWidth(const eastl::string& sArea) const
{
	return GetSheetWidth(convertstring<char, tchar>(sArea));
}

size_t CTextureSheet::GetSheetHeight(const eastl::string& sArea) const
{
	return GetSheetHeight(convertstring<char, tchar>(sArea));
}
