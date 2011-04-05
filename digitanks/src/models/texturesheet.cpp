#include "texturesheet.h"

#include <iostream>
#include <fstream>

#include <strutils.h>

#include <datamanager/dataserializer.h>
#include <renderer/renderer.h>
#include <models/texturelibrary.h>

CTextureSheet::CTextureSheet(eastl::string16 sFile)
{
	std::ifstream f(sFile.c_str());

	CData* pFile = new CData();
	CDataSerializer::Read(f, pFile);

	for (size_t i = 0; i < pFile->GetNumChildren(); i++)
	{
		CData* pChild = pFile->GetChild(i);
		if (pChild->GetKey() == "Texture")
		{
			m_iSheet = CTextureLibrary::AddTexture(convertstring<char, char16_t>(pChild->GetValueString()));

			size_t iTextureData = CRenderer::LoadTextureData(convertstring<char, char16_t>(pChild->GetValueString()));
			m_iSheetWidth = CRenderer::GetTextureWidth(iTextureData);
			m_iSheetHeight = CRenderer::GetTextureHeight(iTextureData);
			CRenderer::UnloadTextureData(iTextureData);
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

			m_aAreas[pChild->GetValueString()] = Rect(x, y, w, h);
		}
	}

	delete pFile;
}

CTextureSheet::~CTextureSheet()
{
}

const Rect& CTextureSheet::GetArea(const eastl::string& sArea) const
{
	eastl::map<eastl::string, Rect>::const_iterator it = m_aAreas.find(sArea);

	if (it == m_aAreas.end())
	{
		static Rect empty(0,0,0,0);
		return empty;
	}

	return it->second;
}
