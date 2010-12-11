#include "data.h"

#include <strutils.h>

CData::CData()
{
	m_pParent = NULL;
}

CData::CData(eastl::string sKey, eastl::string sValue)
{
	m_pParent = NULL;
	m_sKey = sKey;
	m_sValue = sValue;
}

CData::~CData()
{
	for (size_t i = 0; i < m_apChildren.size(); i++)
	{
		CData* pData = m_apChildren[i];
		delete pData;
	}
}

CData* CData::AddChild(eastl::string sKey)
{
	return AddChild(sKey, "");
}

CData* CData::AddChild(eastl::string sKey, eastl::string sValue)
{
	CData* pData = new CData(sKey, sValue);
	m_apChildren.push_back(pData);
	pData->m_pParent = this;
	return pData;
}

float CData::GetValueFloat() const
{
	return (float)atof(GetValueString().c_str());
}

Vector2D CData::GetValueVector2D() const
{
	eastl::vector<eastl::string> asTokens;
	strtok(GetValueString(), asTokens, ",");

	Vector2D vecResult;
	if (asTokens.size() > 0)
		vecResult.x = (float)atof(asTokens[0].c_str());
	if (asTokens.size() > 1)
		vecResult.y = (float)atof(asTokens[1].c_str());

	return vecResult;
}
