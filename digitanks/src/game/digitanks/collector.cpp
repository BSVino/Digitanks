#include "collector.h"

#include <sstream>

#include "digitanksteam.h"

#include <GL/glew.h>
#include <GL/freeglut.h>

REGISTER_ENTITY(CCollector);

void CCollector::PreStartTurn()
{
	BaseClass::PreStartTurn();

	if (!IsConstructing() && GetTeam())
	{
		GetDigitanksTeam()->AddProduction((size_t)(m_hResource->GetProduction() * m_hSupplier->GetChildEfficiency()));
	}
}

void CCollector::OnRender()
{
	if (GetVisibility() == 0)
		return;

	glutSolidCube(8);
}

void CCollector::UpdateInfo(std::string& sInfo)
{
	std::stringstream s;

	s << "POWER SUPPLY UNIT\n";
	s << "Resource collector\n \n";

	if (IsConstructing())
	{
		s << "(Constructing)\n";
		s << "Turns left: " << GetTurnsToConstruct() << "\n";
		return;
	}

	s << "Production: " << (size_t)(m_hResource->GetProduction() * m_hSupplier->GetChildEfficiency()) << "\n";
	s << "Efficiency: " << (int)(m_hSupplier->GetChildEfficiency()*100) << "%\n";

	sInfo = s.str();
}
