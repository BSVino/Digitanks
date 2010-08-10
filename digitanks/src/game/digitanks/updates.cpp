#include "updates.h"

#include "digitanksgame.h"

void CUpdateGrid::SetupStandardUpdates()
{
	int iCPU = UPDATE_GRID_SIZE/2;

	memset(&m_aUpdates[0][0], 0, sizeof(m_aUpdates));

	for (size_t i = 0; i < DigitanksGame()->GetNumTeams(); i++)
	{
		DigitanksGame()->GetDigitanksTeam(i)->DownloadUpdate(iCPU, iCPU, false);
		DigitanksGame()->GetDigitanksTeam(i)->DownloadComplete();
	}

	m_aUpdates[iCPU][iCPU].m_eUpdateClass = UPDATECLASS_STRUCTURE;
	m_aUpdates[iCPU][iCPU].m_eStructure = STRUCTURE_CPU;
	m_aUpdates[iCPU][iCPU].m_iSize = 0;

	m_aUpdates[iCPU-3][iCPU].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-3][iCPU].m_eStructure = STRUCTURE_CPU;
	m_aUpdates[iCPU-3][iCPU].m_eUpdateType = UPDATETYPE_BANDWIDTH;
	m_aUpdates[iCPU-3][iCPU].m_iValue = 2;
	m_aUpdates[iCPU-3][iCPU].m_iSize = 30;
	m_aUpdates[iCPU-3][iCPU].m_iProductionToInstall = 60;

	m_aUpdates[iCPU-2][iCPU].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-2][iCPU].m_eStructure = STRUCTURE_CPU;
	m_aUpdates[iCPU-2][iCPU].m_eUpdateType = UPDATETYPE_FLEETSUPPLY;
	m_aUpdates[iCPU-2][iCPU].m_iValue = 4;
	m_aUpdates[iCPU-2][iCPU].m_iSize = 20;
	m_aUpdates[iCPU-2][iCPU].m_iProductionToInstall = 40;

	m_aUpdates[iCPU-1][iCPU].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-1][iCPU].m_eStructure = STRUCTURE_CPU;
	m_aUpdates[iCPU-1][iCPU].m_eUpdateType = UPDATETYPE_PRODUCTION;
	m_aUpdates[iCPU-1][iCPU].m_iValue = 2;
	m_aUpdates[iCPU-1][iCPU].m_iSize = 10;
	m_aUpdates[iCPU-1][iCPU].m_iProductionToInstall = 20;

	m_aUpdates[iCPU+1][iCPU].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU+1][iCPU].m_eStructure = STRUCTURE_CPU;
	m_aUpdates[iCPU+1][iCPU].m_eUpdateType = UPDATETYPE_PRODUCTION;
	m_aUpdates[iCPU+1][iCPU].m_iValue = 2;
	m_aUpdates[iCPU+1][iCPU].m_iSize = 10;
	m_aUpdates[iCPU+1][iCPU].m_iProductionToInstall = 20;

	m_aUpdates[iCPU+2][iCPU].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU+2][iCPU].m_eStructure = STRUCTURE_CPU;
	m_aUpdates[iCPU+2][iCPU].m_eUpdateType = UPDATETYPE_FLEETSUPPLY;
	m_aUpdates[iCPU+2][iCPU].m_iValue = 3;
	m_aUpdates[iCPU+2][iCPU].m_iSize = 15;
	m_aUpdates[iCPU+2][iCPU].m_iProductionToInstall = 30;

	m_aUpdates[iCPU+3][iCPU].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU+3][iCPU].m_eStructure = STRUCTURE_CPU;
	m_aUpdates[iCPU+3][iCPU].m_eUpdateType = UPDATETYPE_BANDWIDTH;
	m_aUpdates[iCPU+3][iCPU].m_iValue = 2;
	m_aUpdates[iCPU+3][iCPU].m_iSize = 30;
	m_aUpdates[iCPU+3][iCPU].m_iProductionToInstall = 60;


	m_aUpdates[iCPU-2][iCPU-1].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-2][iCPU-1].m_eStructure = STRUCTURE_CPU;
	m_aUpdates[iCPU-2][iCPU-1].m_eUpdateType = UPDATETYPE_PRODUCTION;
	m_aUpdates[iCPU-2][iCPU-1].m_iValue = 4;
	m_aUpdates[iCPU-2][iCPU-1].m_iSize = 20;
	m_aUpdates[iCPU-2][iCPU-1].m_iProductionToInstall = 40;

	m_aUpdates[iCPU-1][iCPU-1].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-1][iCPU-1].m_eStructure = STRUCTURE_CPU;
	m_aUpdates[iCPU-1][iCPU-1].m_eUpdateType = UPDATETYPE_FLEETSUPPLY;
	m_aUpdates[iCPU-1][iCPU-1].m_iValue = 3;
	m_aUpdates[iCPU-1][iCPU-1].m_iSize = 15;
	m_aUpdates[iCPU-1][iCPU-1].m_iProductionToInstall = 30;

	m_aUpdates[iCPU][iCPU-1].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU][iCPU-1].m_eStructure = STRUCTURE_CPU;
	m_aUpdates[iCPU][iCPU-1].m_eUpdateType = UPDATETYPE_PRODUCTION;
	m_aUpdates[iCPU][iCPU-1].m_iValue = 2;
	m_aUpdates[iCPU][iCPU-1].m_iSize = 10;
	m_aUpdates[iCPU][iCPU-1].m_iProductionToInstall = 20;

	m_aUpdates[iCPU+1][iCPU-1].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU+1][iCPU-1].m_eStructure = STRUCTURE_CPU;
	m_aUpdates[iCPU+1][iCPU-1].m_eUpdateType = UPDATETYPE_FLEETSUPPLY;
	m_aUpdates[iCPU+1][iCPU-1].m_iValue = 3;
	m_aUpdates[iCPU+1][iCPU-1].m_iSize = 15;
	m_aUpdates[iCPU+1][iCPU-1].m_iProductionToInstall = 30;

	m_aUpdates[iCPU+2][iCPU-1].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU+2][iCPU-1].m_eStructure = STRUCTURE_CPU;
	m_aUpdates[iCPU+2][iCPU-1].m_eUpdateType = UPDATETYPE_PRODUCTION;
	m_aUpdates[iCPU+2][iCPU-1].m_iValue = 4;
	m_aUpdates[iCPU+2][iCPU-1].m_iSize = 20;
	m_aUpdates[iCPU+2][iCPU-1].m_iProductionToInstall = 40;


	m_aUpdates[iCPU-1][iCPU-2].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-1][iCPU-2].m_eStructure = STRUCTURE_CPU;
	m_aUpdates[iCPU-1][iCPU-2].m_eUpdateType = UPDATETYPE_FLEETSUPPLY;
	m_aUpdates[iCPU-1][iCPU-2].m_iValue = 4;
	m_aUpdates[iCPU-1][iCPU-2].m_iSize = 20;
	m_aUpdates[iCPU-1][iCPU-2].m_iProductionToInstall = 40;

	m_aUpdates[iCPU][iCPU-2].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU][iCPU-2].m_eStructure = STRUCTURE_CPU;
	m_aUpdates[iCPU][iCPU-2].m_eUpdateType = UPDATETYPE_BANDWIDTH;
	m_aUpdates[iCPU][iCPU-2].m_iValue = 1;
	m_aUpdates[iCPU][iCPU-2].m_iSize = 15;
	m_aUpdates[iCPU][iCPU-2].m_iProductionToInstall = 30;

	m_aUpdates[iCPU+1][iCPU-2].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU+1][iCPU-2].m_eStructure = STRUCTURE_CPU;
	m_aUpdates[iCPU+1][iCPU-2].m_eUpdateType = UPDATETYPE_FLEETSUPPLY;
	m_aUpdates[iCPU+1][iCPU-2].m_iValue = 4;
	m_aUpdates[iCPU+1][iCPU-2].m_iSize = 20;
	m_aUpdates[iCPU+1][iCPU-2].m_iProductionToInstall = 40;


	m_aUpdates[iCPU][iCPU-3].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU][iCPU-3].m_eStructure = STRUCTURE_CPU;
	m_aUpdates[iCPU][iCPU-3].m_eUpdateType = UPDATETYPE_FLEETSUPPLY;
	m_aUpdates[iCPU][iCPU-3].m_iValue = 4;
	m_aUpdates[iCPU][iCPU-3].m_iSize = 20;
	m_aUpdates[iCPU][iCPU-3].m_iProductionToInstall = 40;


	m_iLowestX = UPDATE_GRID_SIZE;
	m_iHighestX = 0;
	m_iLowestY = UPDATE_GRID_SIZE;
	m_iHighestY = 0;
	for (int i = 0; i < UPDATE_GRID_SIZE; i++)
	{
		for (int j = 0; j < UPDATE_GRID_SIZE; j++)
		{
			if (m_aUpdates[i][j].m_eUpdateClass == UPDATECLASS_EMPTY)
				continue;

			if (i < m_iLowestX)
				m_iLowestX = i;

			if (i > m_iHighestX)
				m_iHighestX = i;

			if (j < m_iLowestY)
				m_iLowestY = j;

			if (j > m_iHighestY)
				m_iHighestY = j;
		}
	}
}

std::string CUpdateItem::GetName()
{
	std::string sResult;

	if (m_eUpdateClass == UPDATECLASS_STRUCTURE)
	{
		switch (m_eStructure)
		{
		case STRUCTURE_CPU:
			return "CPU";

		default:
			return "Structure";
		}
	}
	else if (m_eUpdateClass == UPDATECLASS_STRUCTUREUPDATE)
	{
		switch (m_eStructure)
		{
		case STRUCTURE_CPU:
			sResult = "CPU ";
			break;

		default:
			sResult = "Structure ";
		}

		switch (m_eUpdateType)
		{
		case UPDATETYPE_PRODUCTION:
			sResult.append("Production");
			break;

		case UPDATETYPE_BANDWIDTH:
			sResult.append("Bandwidth");
			break;

		case UPDATETYPE_FLEETSUPPLY:
			sResult.append("Fleet Supply");
			break;
		}
	}

	return sResult;
}
