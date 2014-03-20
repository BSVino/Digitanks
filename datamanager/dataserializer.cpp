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

#include "dataserializer.h"

#include <strutils.h>

#include "data.h"

void CDataSerializer::Read(FILE* fp, CData* pData)
{
	if (!fp)
		return;

	if (!pData)
		return;

	CData* pCurrentData = pData;
	CData* pLastData = NULL;

	fseek(fp, 0, SEEK_END);
	int iSize = ftell(fp);
	rewind(fp);

	tstring sFile;

	sFile.resize(iSize);

	int iRead = fread((void*)sFile.data(), 1, iSize, fp);
	TAssertNoMsg(iRead == iSize);

	// Just in case.
	sFile.append("\0");

	tvector<tstring> asTokens;
	tstrtok(sFile, asTokens, "\r\n");

	for (size_t i = 0; i < asTokens.size(); i++)
	{
		tstring sLine = asTokens[i];

		size_t iComment = sLine.find("//");
		if (iComment != tstring::npos)
			sLine = sLine.substr(0, iComment);

		sLine = trim(sLine);

		if (sLine.length() == 0)
			continue;

		if (sLine[0] == '{')
		{
			pCurrentData = pLastData;
			continue;
		}

		if (sLine[0] == '}')
		{
			pCurrentData = pCurrentData->GetParent();
			continue;
		}

		tvector<tstring> asTokens;
		explode(sLine, asTokens, ":");

		if (asTokens.size() == 1)
			pLastData = pCurrentData->AddChild(trim(sLine));
		else if (asTokens.size() >= 2)
			pLastData = pCurrentData->AddChild(trim(asTokens[0]), trim(sLine.substr(sLine.find(':') + 1)));
	}
}

static void SaveData(FILE* fp, CData* pData, size_t iLevel)
{
	tstring sTabs;
	for (size_t i = 0; i < iLevel; i++)
		sTabs += "\t";

	for (size_t i = 0; i < pData->GetNumChildren(); i++)
	{
		CData* pChild = pData->GetChild(i);

		if (pChild->GetValueString().length())
			fputs((sTabs + pChild->GetKey() + ": " + pChild->GetValueString() + "\n").c_str(), fp);
		else
			fputs((sTabs + pChild->GetKey() + "\n").c_str(), fp);

		if (pChild->GetNumChildren())
		{
			fputs((sTabs + "{\n").c_str(), fp);
			SaveData(fp, pChild, iLevel + 1);
			fputs((sTabs + "}\n").c_str(), fp);
		}
	}
}

void CDataSerializer::Save(FILE* fp, CData* pData)
{
	if (!fp)
		return;

	if (!pData)
		return;

	SaveData(fp, pData, 0);
}
