#include "level.h"

#include <datamanager/data.h>

void CLevel::ReadFromData(const CData* pData)
{
	for (size_t i = 0; i < pData->GetNumChildren(); i++)
	{
		CData* pChildData = pData->GetChild(i);

		OnReadData(pChildData);
	}
}

void CLevel::OnReadData(const CData* pData)
{
	if (pData->GetKey() == "Name")
		m_sName = pData->GetValueString();
}
