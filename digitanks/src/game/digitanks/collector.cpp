#include "collector.h"

#include <sstream>

#include "digitanksteam.h"

#include <GL/glew.h>

void CCollector::Spawn()
{
	BaseClass::Spawn();

	SetModel(L"models/structures/psu.obj");
}

void CCollector::Precache()
{
	BaseClass::Precache();

	PrecacheModel(L"models/structures/psu.obj");
}

void CCollector::UpdateInfo(std::wstring& sInfo)
{
	std::wstringstream s;

	s << L"POWER SUPPLY UNIT\n";
	s << L"Resource collector\n \n";

	if (IsConstructing())
	{
		s << L"(Constructing)\n";
		s << L"Turns left: " << GetTurnsToConstruct() << "\n";
		sInfo = s.str();
		return;
	}

	s << L"Power supplied: " << (size_t)(m_hResource->GetProduction() * m_hSupplier->GetChildEfficiency()) << L"\n";
	s << L"Efficiency: " << (int)(m_hSupplier->GetChildEfficiency()*100) << L"%\n";

	sInfo = s.str();
}
