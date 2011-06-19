#include "texturesheet.h"

#include <iostream>
#include <fstream>

#include <strutils.h>

#include <datamanager/dataserializer.h>
#include <renderer/renderer.h>
#include <models/texturelibrary.h>

CTextureSheet::CTextureSheet(tstring sFile)
{
	std::ifstream f(sFile.c_str());

	CData* pFile = new CData();
	CDataSerializer::Read(f, pFile);

	for (size_t i = 0; i < pFile->GetNumChildren(); i++)
	{
		CData* pChild = pFile->GetChild(i);
		if (pChild->GetKey() == "Texture")
		{
			tstring sTexture = convertstring<char, tchar>(pChild->GetValueString());
			const CTexture* pTex = CTextureLibrary::AddTexture(sTexture);
			m_iDefaultSheet = pTex->m_iGLID;
			m_iDefaultSheetWidth = pTex->m_iWidth;
			m_iDefaultSheetHeight = pTex->m_iHeight;
		}
		else if (pChild->GetKey() == "Area")
		{
			int x = 0;
			int y = 0;
			int w = 0;
			int h = 0;

			CData* pData = pChild->FindChild("x");
			if (pData)
				x = pData->GetValueInt();

			pData = pChild->FindChild("y");
			if (pData)
				y = pData->GetValueInt();

			pData = pChild->FindChild("w");
			if (pData)
				w = pData->GetValueInt();

			pData = pChild->FindChild("h");
			if (pData)
				h = pData->GetValueInt();

			m_aAreas[pChild->GetValueString()].m_rRect = Rect(x, y, w, h);

			m_aAreas[pChild->GetValueString()].m_iSheet = ~0;
			pData = pChild->FindChild("Texture");
			if (pData)
			{
				tstring sTexture = convertstring<char, tchar>(pData->GetValueString());
				const CTexture* pTex = CTextureLibrary::AddTexture(sTexture);
				m_aAreas[pChild->GetValueString()].m_iSheet = pTex->m_iGLID;
				m_aAreas[pChild->GetValueString()].m_iSheetWidth = pTex->m_iWidth;
				m_aAreas[pChild->GetValueString()].m_iSheetHeight = pTex->m_iHeight;
			}
		}
	}

	delete pFile;
}

CTextureSheet::~CTextureSheet()
{
}

const Rect& CTextureSheet::GetArea(const eastl::string& sArea) const
{
	eastl::map<eastl::string, CTextureArea>::const_iterator it = m_aAreas.find(sArea);

	if (it == m_aAreas.end())
	{
		static Rect empty(0,0,0,0);
		return empty;
	}

	return it->second.m_rRect;
}

size_t CTextureSheet::GetSheet(const eastl::string& sArea) const
{
	eastl::map<eastl::string, CTextureArea>::const_iterator it = m_aAreas.find(sArea);

	if (it == m_aAreas.end())
		return 0;

	size_t iSheet = it->second.m_iSheet;
	if (iSheet == ~0)
		return m_iDefaultSheet;

	return iSheet;
}

size_t CTextureSheet::GetSheetWidth(const eastl::string& sArea) const
{
	eastl::map<eastl::string, CTextureArea>::const_iterator it = m_aAreas.find(sArea);

	if (it == m_aAreas.end())
		return 0;

	size_t iSheet = it->second.m_iSheet;
	if (iSheet == ~0)
		return m_iDefaultSheetWidth;

	return it->second.m_iSheetWidth;
}

size_t CTextureSheet::GetSheetHeight(const eastl::string& sArea) const
{
	eastl::map<eastl::string, CTextureArea>::const_iterator it = m_aAreas.find(sArea);

	if (it == m_aAreas.end())
		return 0;

	size_t iSheet = it->second.m_iSheet;
	if (iSheet == ~0)
		return m_iDefaultSheetHeight;

	return it->second.m_iSheetHeight;
}
