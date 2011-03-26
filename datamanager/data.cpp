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

size_t CData::FindChildIndex(eastl::string sKey)
{
	for (size_t i = 0; i < m_apChildren.size(); i++)
		if (m_apChildren[i]->GetKey() == sKey)
			return i;

	return ~0;
}

CData* CData::FindChild(eastl::string sKey)
{
	size_t iIndex = FindChildIndex(sKey);

	if (iIndex == ~0)
		return NULL;

	return m_apChildren[iIndex];
}

bool CData::GetValueBool() const
{
	eastl::string sValue = GetValueString();

	for( eastl::string::iterator p = sValue.begin(); p != sValue.end(); ++p )
		*p = toupper(*p);  // make string all caps

	if( sValue == eastl::string("FALSE") || sValue == eastl::string("F") ||
	    sValue == eastl::string("NO") || sValue == eastl::string("N") ||
	    sValue == eastl::string("0") || sValue == eastl::string("NONE") || sValue == eastl::string("OFF") )
		return false;

	return true;
}

int CData::GetValueInt() const
{
	return (int)atoi(GetValueString().c_str());
}

size_t CData::GetValueUInt() const
{
	return (size_t)atoi(GetValueString().c_str());
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

EAngle CData::GetValueEAngle() const
{
	eastl::vector<eastl::string> asTokens;
	strtok(GetValueString(), asTokens, ",");

	EAngle vecResult;
	if (asTokens.size() > 0)
		vecResult.p = (float)atof(asTokens[0].c_str());
	if (asTokens.size() > 1)
		vecResult.y = (float)atof(asTokens[1].c_str());
	if (asTokens.size() > 2)
		vecResult.r = (float)atof(asTokens[2].c_str());

	return vecResult;
}
