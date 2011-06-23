#include "data.h"

#include <strutils.h>

CData::CData()
{
	m_pParent = NULL;
}

CData::CData(tstring sKey, tstring sValue)
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

CData* CData::AddChild(tstring sKey)
{
	return AddChild(sKey, _T(""));
}

CData* CData::AddChild(tstring sKey, tstring sValue)
{
	CData* pData = new CData(sKey, sValue);
	m_apChildren.push_back(pData);
	pData->m_pParent = this;
	return pData;
}

size_t CData::FindChildIndex(tstring sKey)
{
	for (size_t i = 0; i < m_apChildren.size(); i++)
		if (m_apChildren[i]->GetKey() == sKey)
			return i;

	return ~0;
}

CData* CData::FindChild(tstring sKey)
{
	size_t iIndex = FindChildIndex(sKey);

	if (iIndex == ~0)
		return NULL;

	return m_apChildren[iIndex];
}

bool CData::GetValueBool() const
{
	tstring sValue = GetValueTString();

	for( tstring::iterator p = sValue.begin(); p != sValue.end(); ++p )
		*p = toupper(*p);  // make string all caps

	if( sValue == tstring(_T("FALSE")) || sValue == tstring(_T("F")) ||
	    sValue == tstring(_T("NO")) || sValue == tstring(_T("N")) ||
	    sValue == tstring(_T("0")) || sValue == tstring(_T("NONE")) || sValue == tstring(_T("OFF")) )
		return false;

	return true;
}

int CData::GetValueInt() const
{
	return (int)stoi(GetValueTString().c_str());
}

size_t CData::GetValueUInt() const
{
	return (size_t)stoi(GetValueTString().c_str());
}

float CData::GetValueFloat() const
{
	return (float)stof(GetValueTString().c_str());
}

Vector2D CData::GetValueVector2D() const
{
	eastl::vector<tstring> asTokens;
	tstrtok(GetValueTString(), asTokens, _T(","));

	Vector2D vecResult;
	if (asTokens.size() > 0)
		vecResult.x = (float)stof(asTokens[0].c_str());
	if (asTokens.size() > 1)
		vecResult.y = (float)stof(asTokens[1].c_str());

	return vecResult;
}

EAngle CData::GetValueEAngle() const
{
	eastl::vector<tstring> asTokens;
	tstrtok(GetValueTString(), asTokens, _T(","));

	EAngle vecResult;
	if (asTokens.size() > 0)
		vecResult.p = (float)stof(asTokens[0].c_str());
	if (asTokens.size() > 1)
		vecResult.y = (float)stof(asTokens[1].c_str());
	if (asTokens.size() > 2)
		vecResult.r = (float)stof(asTokens[2].c_str());

	return vecResult;
}

void CData::SetValue(bool bValue)
{
	m_sValue = bValue?_T("true"):_T("false");
}

void CData::SetValue(int iValue)
{
	m_sValue = sprintf(_T("%d"), iValue);
}

void CData::SetValue(size_t iValue)
{
	m_sValue = sprintf(_T("%u"), iValue);
}

void CData::SetValue(float flValue)
{
	m_sValue = sprintf(_T("%f"), flValue);
}

void CData::SetValue(Vector2D vecValue)
{
	m_sValue = sprintf(_T("%f, %f"), vecValue.x, vecValue.y);
}

void CData::SetValue(EAngle angValue)
{
	m_sValue = sprintf(_T("%f, %f, %f"), angValue.p, angValue.y, angValue.r);
}
