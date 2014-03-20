/*
Copyright (c) 2012, Lunar Workshop, Inc.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
3. All advertising materials mentioning features or use of this software must display the following acknowledgement:
   This product includes software developed by Lunar Workshop, Inc.
4. Neither the name of the Lunar Workshop nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY LUNAR WORKSHOP INC ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL LUNAR WORKSHOP BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "texturesheet.h"

#include <iostream>
#include <fstream>

#include <strutils.h>

#include <datamanager/dataserializer.h>
#include <renderer/renderer.h>
#include <textures/materiallibrary.h>

CTextureSheet::CTextureSheet(tstring sFile)
{
	FILE* fp = tfopen_asset(sFile, "r");

	TAssertNoMsg(fp);

	CData* pFile = new CData();
	CDataSerializer::Read(fp, pFile);

	for (size_t i = 0; i < pFile->GetNumChildren(); i++)
	{
		CData* pChild = pFile->GetChild(i);
		if (pChild->GetKey() == "Material")
		{
			tstring sTexture = pChild->GetValueString();
			m_hDefaultSheet = CMaterialLibrary::AddMaterial(sTexture);
			TAssertNoMsg(m_hDefaultSheet.IsValid());
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

			m_aAreas[pChild->GetValueString()].m_hSheet.Reset();
			pData = pChild->FindChild("Material");
			if (pData)
			{
				tstring sTexture = pData->GetValueString();
				m_aAreas[pChild->GetValueString()].m_hSheet = CMaterialLibrary::AddMaterial(sTexture);
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
	tmap<tstring, CTextureArea>::const_iterator it = m_aAreas.find(sArea);

	if (it == m_aAreas.end())
	{
		static Rect empty(0,0,0,0);
		return empty;
	}

	return it->second.m_rRect;
}

CMaterialHandle CTextureSheet::GetSheet(const tstring& sArea) const
{
	tmap<tstring, CTextureArea>::const_iterator it = m_aAreas.find(sArea);

	if (it == m_aAreas.end())
		return CMaterialHandle();

	const CMaterialHandle& hSheet = it->second.m_hSheet;
	if (!hSheet.IsValid())
		return m_hDefaultSheet;

	return hSheet;
}

size_t CTextureSheet::GetSheetWidth(const tstring& sArea) const
{
	tmap<tstring, CTextureArea>::const_iterator it = m_aAreas.find(sArea);

	if (it == m_aAreas.end())
		return 0;

	const CMaterialHandle& hSheet = it->second.m_hSheet;
	if (!hSheet.IsValid())
	{
		if (!m_hDefaultSheet)
			return 0;

		TAssertNoMsg(m_hDefaultSheet->m_ahTextures.size());
		return m_hDefaultSheet->m_ahTextures[0]->m_iWidth;
	}

	TAssertNoMsg(it->second.m_hSheet->m_ahTextures.size());
	return it->second.m_hSheet->m_ahTextures[0]->m_iWidth;
}

size_t CTextureSheet::GetSheetHeight(const tstring& sArea) const
{
	tmap<tstring, CTextureArea>::const_iterator it = m_aAreas.find(sArea);

	if (it == m_aAreas.end())
		return 0;

	const CMaterialHandle& hSheet = it->second.m_hSheet;
	if (!hSheet.IsValid())
	{
		if (!m_hDefaultSheet)
			return 0;

		TAssertNoMsg(m_hDefaultSheet->m_ahTextures.size());
		if (!m_hDefaultSheet->m_ahTextures.size())
			return 0;

		TAssertNoMsg(m_hDefaultSheet->m_ahTextures.size());
		if (!m_hDefaultSheet->m_ahTextures[0])
			return 0;

		return m_hDefaultSheet->m_ahTextures[0]->m_iHeight;
	}

	TAssertNoMsg(it->second.m_hSheet->m_ahTextures.size());
	if (!it->second.m_hSheet->m_ahTextures.size())
		return 0;

	TAssertNoMsg(it->second.m_hSheet->m_ahTextures.size());
	if (!it->second.m_hSheet->m_ahTextures[0])
		return 0;

	return it->second.m_hSheet->m_ahTextures[0]->m_iHeight;
}
