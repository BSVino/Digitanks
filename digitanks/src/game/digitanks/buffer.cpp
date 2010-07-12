#include "buffer.h"

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <sstream>

#include <game/team.h>

REGISTER_ENTITY(CBuffer);

void CBuffer::Spawn()
{
	BaseClass::Spawn();

	SetModel(L"models/structures/buffer.obj");
}

void CBuffer::Precache()
{
	BaseClass::Precache();

	PrecacheModel(L"models/structures/buffer.obj", false);
}

void CBuffer::UpdateInfo(std::string& sInfo)
{
	std::stringstream s;

	s << "BUFFER INFO\n";
	s << "Network extender\n \n";

	if (IsConstructing())
	{
		s << "(Constructing)\n";
		s << "Production left: " << GetProductionRemaining() << "\n";
		s << "Turns left: " << GetTurnsToConstruct() << "\n";
		sInfo = s.str();
		return;
	}

	s << "Strength: " << m_iDataStrength << "\n";
	s << "Growth: " << (int)GetDataFlowRate() << "\n";
	s << "Size: " << (int)GetDataFlowRadius() << "\n";
	s << "Efficiency: " << (int)(GetChildEfficiency()*100) << "%\n";

	sInfo = s.str();
}
