#include "updates.h"

#include "digitanksgame.h"

void CUpdateGrid::SetupStandardUpdates()
{
	int iCPU = UPDATE_GRID_SIZE/2;

	memset(&m_aUpdates[0][0], 0, sizeof(m_aUpdates));

	m_aUpdates[iCPU][iCPU].m_eUpdateClass = UPDATECLASS_STRUCTURE;
	m_aUpdates[iCPU][iCPU].m_eStructure = STRUCTURE_CPU;
	m_aUpdates[iCPU][iCPU].m_iSize = 0;

	m_aUpdates[iCPU-3][iCPU].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-3][iCPU].m_eStructure = STRUCTURE_CPU;
	m_aUpdates[iCPU-3][iCPU].m_eUpdateType = UPDATETYPE_BANDWIDTH;
	m_aUpdates[iCPU-3][iCPU].m_flValue = 2;
	m_aUpdates[iCPU-3][iCPU].m_iSize = 30;
	m_aUpdates[iCPU-3][iCPU].m_iProductionToInstall = 30;

	m_aUpdates[iCPU-2][iCPU].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-2][iCPU].m_eStructure = STRUCTURE_CPU;
	m_aUpdates[iCPU-2][iCPU].m_eUpdateType = UPDATETYPE_FLEETSUPPLY;
	m_aUpdates[iCPU-2][iCPU].m_flValue = 4;
	m_aUpdates[iCPU-2][iCPU].m_iSize = 12;
	m_aUpdates[iCPU-2][iCPU].m_iProductionToInstall = 15;

	m_aUpdates[iCPU-1][iCPU].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-1][iCPU].m_eStructure = STRUCTURE_CPU;
	m_aUpdates[iCPU-1][iCPU].m_eUpdateType = UPDATETYPE_PRODUCTION;
	m_aUpdates[iCPU-1][iCPU].m_flValue = 1;
	m_aUpdates[iCPU-1][iCPU].m_iSize = 6;
	m_aUpdates[iCPU-1][iCPU].m_iProductionToInstall = 10;

	m_aUpdates[iCPU+1][iCPU].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU+1][iCPU].m_eStructure = STRUCTURE_CPU;
	m_aUpdates[iCPU+1][iCPU].m_eUpdateType = UPDATETYPE_PRODUCTION;
	m_aUpdates[iCPU+1][iCPU].m_flValue = 1;
	m_aUpdates[iCPU+1][iCPU].m_iSize = 6;
	m_aUpdates[iCPU+1][iCPU].m_iProductionToInstall = 10;

	m_aUpdates[iCPU+2][iCPU].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU+2][iCPU].m_eStructure = STRUCTURE_CPU;
	m_aUpdates[iCPU+2][iCPU].m_eUpdateType = UPDATETYPE_FLEETSUPPLY;
	m_aUpdates[iCPU+2][iCPU].m_flValue = 3;
	m_aUpdates[iCPU+2][iCPU].m_iSize = 9;
	m_aUpdates[iCPU+2][iCPU].m_iProductionToInstall = 15;

	m_aUpdates[iCPU+3][iCPU].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU+3][iCPU].m_eStructure = STRUCTURE_CPU;
	m_aUpdates[iCPU+3][iCPU].m_eUpdateType = UPDATETYPE_BANDWIDTH;
	m_aUpdates[iCPU+3][iCPU].m_flValue = 2;
	m_aUpdates[iCPU+3][iCPU].m_iSize = 18;
	m_aUpdates[iCPU+3][iCPU].m_iProductionToInstall = 30;


	m_aUpdates[iCPU-2][iCPU-1].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-2][iCPU-1].m_eStructure = STRUCTURE_CPU;
	m_aUpdates[iCPU-2][iCPU-1].m_eUpdateType = UPDATETYPE_FLEETSUPPLY;
	m_aUpdates[iCPU-2][iCPU-1].m_flValue = 2;
	m_aUpdates[iCPU-2][iCPU-1].m_iSize = 12;
	m_aUpdates[iCPU-2][iCPU-1].m_iProductionToInstall = 20;

	m_aUpdates[iCPU-1][iCPU-1].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-1][iCPU-1].m_eStructure = STRUCTURE_CPU;
	m_aUpdates[iCPU-1][iCPU-1].m_eUpdateType = UPDATETYPE_PRODUCTION;
	m_aUpdates[iCPU-1][iCPU-1].m_flValue = 3;
	m_aUpdates[iCPU-1][iCPU-1].m_iSize = 9;
	m_aUpdates[iCPU-1][iCPU-1].m_iProductionToInstall = 15;

	m_aUpdates[iCPU][iCPU-1].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU][iCPU-1].m_eStructure = STRUCTURE_CPU;
	m_aUpdates[iCPU][iCPU-1].m_eUpdateType = UPDATETYPE_FLEETSUPPLY;
	m_aUpdates[iCPU][iCPU-1].m_flValue = 1;
	m_aUpdates[iCPU][iCPU-1].m_iSize = 6;
	m_aUpdates[iCPU][iCPU-1].m_iProductionToInstall = 10;

	m_aUpdates[iCPU+1][iCPU-1].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU+1][iCPU-1].m_eStructure = STRUCTURE_CPU;
	m_aUpdates[iCPU+1][iCPU-1].m_eUpdateType = UPDATETYPE_PRODUCTION;
	m_aUpdates[iCPU+1][iCPU-1].m_flValue = 3;
	m_aUpdates[iCPU+1][iCPU-1].m_iSize = 9;
	m_aUpdates[iCPU+1][iCPU-1].m_iProductionToInstall = 15;

	m_aUpdates[iCPU+2][iCPU-1].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU+2][iCPU-1].m_eStructure = STRUCTURE_CPU;
	m_aUpdates[iCPU+2][iCPU-1].m_eUpdateType = UPDATETYPE_FLEETSUPPLY;
	m_aUpdates[iCPU+2][iCPU-1].m_flValue = 2;
	m_aUpdates[iCPU+2][iCPU-1].m_iSize = 12;
	m_aUpdates[iCPU+2][iCPU-1].m_iProductionToInstall = 20;


	m_aUpdates[iCPU-1][iCPU-2].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-1][iCPU-2].m_eStructure = STRUCTURE_CPU;
	m_aUpdates[iCPU-1][iCPU-2].m_eUpdateType = UPDATETYPE_PRODUCTION;
	m_aUpdates[iCPU-1][iCPU-2].m_flValue = 2;
	m_aUpdates[iCPU-1][iCPU-2].m_iSize = 12;
	m_aUpdates[iCPU-1][iCPU-2].m_iProductionToInstall = 20;

	m_aUpdates[iCPU][iCPU-2].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU][iCPU-2].m_eStructure = STRUCTURE_CPU;
	m_aUpdates[iCPU][iCPU-2].m_eUpdateType = UPDATETYPE_BANDWIDTH;
	m_aUpdates[iCPU][iCPU-2].m_flValue = 1;
	m_aUpdates[iCPU][iCPU-2].m_iSize = 9;
	m_aUpdates[iCPU][iCPU-2].m_iProductionToInstall = 15;

	m_aUpdates[iCPU+1][iCPU-2].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU+1][iCPU-2].m_eStructure = STRUCTURE_CPU;
	m_aUpdates[iCPU+1][iCPU-2].m_eUpdateType = UPDATETYPE_PRODUCTION;
	m_aUpdates[iCPU+1][iCPU-2].m_flValue = 2;
	m_aUpdates[iCPU+1][iCPU-2].m_iSize = 12;
	m_aUpdates[iCPU+1][iCPU-2].m_iProductionToInstall = 20;


	m_aUpdates[iCPU][iCPU-3].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU][iCPU-3].m_eStructure = STRUCTURE_CPU;
	m_aUpdates[iCPU][iCPU-3].m_eUpdateType = UPDATETYPE_FLEETSUPPLY;
	m_aUpdates[iCPU][iCPU-3].m_flValue = 4;
	m_aUpdates[iCPU][iCPU-3].m_iSize = 12;
	m_aUpdates[iCPU][iCPU-3].m_iProductionToInstall = 20;


	m_aUpdates[iCPU+3][iCPU-1].m_eUpdateClass = UPDATECLASS_STRUCTURE;
	m_aUpdates[iCPU+3][iCPU-1].m_eStructure = STRUCTURE_PSU;
	m_aUpdates[iCPU+3][iCPU-1].m_iSize = 30;

	m_aUpdates[iCPU+2][iCPU-2].m_eUpdateClass = UPDATECLASS_STRUCTURE;
	m_aUpdates[iCPU+2][iCPU-2].m_eStructure = STRUCTURE_BUFFER;
	m_aUpdates[iCPU+2][iCPU-2].m_iSize = 20;

	m_aUpdates[iCPU][iCPU-4].m_eUpdateClass = UPDATECLASS_STRUCTURE;
	m_aUpdates[iCPU][iCPU-4].m_eStructure = STRUCTURE_ARTILLERYLOADER;
	m_aUpdates[iCPU][iCPU-4].m_iSize = 70;

	m_aUpdates[iCPU-2][iCPU-2].m_eUpdateClass = UPDATECLASS_STRUCTURE;
	m_aUpdates[iCPU-2][iCPU-2].m_eStructure = STRUCTURE_INFANTRYLOADER;
	m_aUpdates[iCPU-2][iCPU-2].m_iSize = 20;

	m_aUpdates[iCPU-3][iCPU-1].m_eUpdateClass = UPDATECLASS_STRUCTURE;
	m_aUpdates[iCPU-3][iCPU-1].m_eStructure = STRUCTURE_TANKLOADER;
	m_aUpdates[iCPU-3][iCPU-1].m_iSize = 50;


	m_aUpdates[iCPU+2][iCPU-3].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU+2][iCPU-3].m_eStructure = STRUCTURE_BUFFER;
	m_aUpdates[iCPU+2][iCPU-3].m_eUpdateType = UPDATETYPE_FLEETSUPPLY;
	m_aUpdates[iCPU+2][iCPU-3].m_flValue = 1;
	m_aUpdates[iCPU+2][iCPU-3].m_iSize = 18;
	m_aUpdates[iCPU+2][iCPU-3].m_iProductionToInstall = 35;

	m_aUpdates[iCPU+2][iCPU-4].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU+2][iCPU-4].m_eStructure = STRUCTURE_BUFFER;
	m_aUpdates[iCPU+2][iCPU-4].m_eUpdateType = UPDATETYPE_BANDWIDTH;
	m_aUpdates[iCPU+2][iCPU-4].m_flValue = 1;
	m_aUpdates[iCPU+2][iCPU-4].m_iSize = 21;
	m_aUpdates[iCPU+2][iCPU-4].m_iProductionToInstall = 40;

	m_aUpdates[iCPU+2][iCPU-5].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU+2][iCPU-5].m_eStructure = STRUCTURE_BUFFER;
	m_aUpdates[iCPU+2][iCPU-5].m_eUpdateType = UPDATETYPE_SUPPORTENERGY;
	m_aUpdates[iCPU+2][iCPU-5].m_flValue = 1;
	m_aUpdates[iCPU+2][iCPU-5].m_iSize = 24;
	m_aUpdates[iCPU+2][iCPU-5].m_iProductionToInstall = 45;

	m_aUpdates[iCPU+3][iCPU-4].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU+3][iCPU-4].m_eStructure = STRUCTURE_BUFFER;
	m_aUpdates[iCPU+3][iCPU-4].m_eUpdateType = UPDATETYPE_SUPPORTRECHARGE;
	m_aUpdates[iCPU+3][iCPU-4].m_flValue = 0.5f;
	m_aUpdates[iCPU+3][iCPU-4].m_iSize = 24;
	m_aUpdates[iCPU+3][iCPU-4].m_iProductionToInstall = 45;

	m_aUpdates[iCPU+3][iCPU-5].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU+3][iCPU-5].m_eStructure = STRUCTURE_BUFFER;
	m_aUpdates[iCPU+3][iCPU-5].m_eUpdateType = UPDATETYPE_FLEETSUPPLY;
	m_aUpdates[iCPU+3][iCPU-5].m_flValue = 2;
	m_aUpdates[iCPU+3][iCPU-5].m_iSize = 27;
	m_aUpdates[iCPU+3][iCPU-5].m_iProductionToInstall = 50;

	m_aUpdates[iCPU+4][iCPU-4].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU+4][iCPU-4].m_eStructure = STRUCTURE_BUFFER;
	m_aUpdates[iCPU+4][iCPU-4].m_eUpdateType = UPDATETYPE_SUPPORTENERGY;
	m_aUpdates[iCPU+4][iCPU-4].m_flValue = 2;
	m_aUpdates[iCPU+4][iCPU-4].m_iSize = 27;
	m_aUpdates[iCPU+4][iCPU-4].m_iProductionToInstall = 50;


	m_aUpdates[iCPU-4][iCPU-1].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-4][iCPU-1].m_eStructure = STRUCTURE_TANKLOADER;
	m_aUpdates[iCPU-4][iCPU-1].m_eUpdateType = UPDATETYPE_TANKATTACK;
	m_aUpdates[iCPU-4][iCPU-1].m_flValue = 1;
	m_aUpdates[iCPU-4][iCPU-1].m_iSize = 18;
	m_aUpdates[iCPU-4][iCPU-1].m_iProductionToInstall = 35;

	m_aUpdates[iCPU-5][iCPU-1].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-5][iCPU-1].m_eStructure = STRUCTURE_TANKLOADER;
	m_aUpdates[iCPU-5][iCPU-1].m_eUpdateType = UPDATETYPE_TANKMOVEMENT;
	m_aUpdates[iCPU-5][iCPU-1].m_flValue = 1;
	m_aUpdates[iCPU-5][iCPU-1].m_iSize = 21;
	m_aUpdates[iCPU-5][iCPU-1].m_iProductionToInstall = 40;

	m_aUpdates[iCPU-6][iCPU-1].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-6][iCPU-1].m_eStructure = STRUCTURE_TANKLOADER;
	m_aUpdates[iCPU-6][iCPU-1].m_eUpdateType = UPDATETYPE_TANKHEALTH;
	m_aUpdates[iCPU-6][iCPU-1].m_flValue = 1;
	m_aUpdates[iCPU-6][iCPU-1].m_iSize = 24;
	m_aUpdates[iCPU-6][iCPU-1].m_iProductionToInstall = 45;

	m_aUpdates[iCPU-5][iCPU-2].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-5][iCPU-2].m_eStructure = STRUCTURE_TANKLOADER;
	m_aUpdates[iCPU-5][iCPU-2].m_eUpdateType = UPDATETYPE_TANKDEFENSE;
	m_aUpdates[iCPU-5][iCPU-2].m_flValue = 1;
	m_aUpdates[iCPU-5][iCPU-2].m_iSize = 24;
	m_aUpdates[iCPU-5][iCPU-2].m_iProductionToInstall = 45;

	m_aUpdates[iCPU-6][iCPU-2].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-6][iCPU-2].m_eStructure = STRUCTURE_TANKLOADER;
	m_aUpdates[iCPU-6][iCPU-2].m_eUpdateType = UPDATETYPE_TANKATTACK;
	m_aUpdates[iCPU-6][iCPU-2].m_flValue = 1;
	m_aUpdates[iCPU-6][iCPU-2].m_iSize = 27;
	m_aUpdates[iCPU-6][iCPU-2].m_iProductionToInstall = 50;

	m_aUpdates[iCPU-5][iCPU-3].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-5][iCPU-3].m_eStructure = STRUCTURE_TANKLOADER;
	m_aUpdates[iCPU-5][iCPU-3].m_eUpdateType = UPDATETYPE_TANKMOVEMENT;
	m_aUpdates[iCPU-5][iCPU-3].m_flValue = 1;
	m_aUpdates[iCPU-5][iCPU-3].m_iSize = 27;
	m_aUpdates[iCPU-5][iCPU-3].m_iProductionToInstall = 50;


	m_aUpdates[iCPU-2][iCPU-3].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-2][iCPU-3].m_eStructure = STRUCTURE_INFANTRYLOADER;
	m_aUpdates[iCPU-2][iCPU-3].m_eUpdateType = UPDATETYPE_TANKDEFENSE;
	m_aUpdates[iCPU-2][iCPU-3].m_flValue = 1;
	m_aUpdates[iCPU-2][iCPU-3].m_iSize = 9;
	m_aUpdates[iCPU-2][iCPU-3].m_iProductionToInstall = 17;

	m_aUpdates[iCPU-2][iCPU-4].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-2][iCPU-4].m_eStructure = STRUCTURE_INFANTRYLOADER;
	m_aUpdates[iCPU-2][iCPU-4].m_eUpdateType = UPDATETYPE_TANKHEALTH;
	m_aUpdates[iCPU-2][iCPU-4].m_flValue = 1;
	m_aUpdates[iCPU-2][iCPU-4].m_iSize = 10;
	m_aUpdates[iCPU-2][iCPU-4].m_iProductionToInstall = 20;

	m_aUpdates[iCPU-2][iCPU-5].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-2][iCPU-5].m_eStructure = STRUCTURE_INFANTRYLOADER;
	m_aUpdates[iCPU-2][iCPU-5].m_eUpdateType = UPDATETYPE_TANKATTACK;
	m_aUpdates[iCPU-2][iCPU-5].m_flValue = 1;
	m_aUpdates[iCPU-2][iCPU-5].m_iSize = 12;
	m_aUpdates[iCPU-2][iCPU-5].m_iProductionToInstall = 22;

	m_aUpdates[iCPU-3][iCPU-4].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-3][iCPU-4].m_eStructure = STRUCTURE_INFANTRYLOADER;
	m_aUpdates[iCPU-3][iCPU-4].m_eUpdateType = UPDATETYPE_TANKMOVEMENT;
	m_aUpdates[iCPU-3][iCPU-4].m_flValue = 1;
	m_aUpdates[iCPU-3][iCPU-4].m_iSize = 12;
	m_aUpdates[iCPU-3][iCPU-4].m_iProductionToInstall = 22;

	m_aUpdates[iCPU-3][iCPU-5].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-3][iCPU-5].m_eStructure = STRUCTURE_INFANTRYLOADER;
	m_aUpdates[iCPU-3][iCPU-5].m_eUpdateType = UPDATETYPE_TANKDEFENSE;
	m_aUpdates[iCPU-3][iCPU-5].m_flValue = 1;
	m_aUpdates[iCPU-3][iCPU-5].m_iSize = 13;
	m_aUpdates[iCPU-3][iCPU-5].m_iProductionToInstall = 25;

	m_aUpdates[iCPU-4][iCPU-4].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-4][iCPU-4].m_eStructure = STRUCTURE_INFANTRYLOADER;
	m_aUpdates[iCPU-4][iCPU-4].m_eUpdateType = UPDATETYPE_TANKHEALTH;
	m_aUpdates[iCPU-4][iCPU-4].m_flValue = 1;
	m_aUpdates[iCPU-4][iCPU-4].m_iSize = 13;
	m_aUpdates[iCPU-4][iCPU-4].m_iProductionToInstall = 25;


	m_aUpdates[iCPU][iCPU-5].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU][iCPU-5].m_eStructure = STRUCTURE_ARTILLERYLOADER;
	m_aUpdates[iCPU][iCPU-5].m_eUpdateType = UPDATETYPE_TANKRANGE;
	m_aUpdates[iCPU][iCPU-5].m_flValue = 10;
	m_aUpdates[iCPU][iCPU-5].m_iSize = 18;
	m_aUpdates[iCPU][iCPU-5].m_iProductionToInstall = 35;

	m_aUpdates[iCPU][iCPU-6].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU][iCPU-6].m_eStructure = STRUCTURE_ARTILLERYLOADER;
	m_aUpdates[iCPU][iCPU-6].m_eUpdateType = UPDATETYPE_TANKATTACK;
	m_aUpdates[iCPU][iCPU-6].m_flValue = 1;
	m_aUpdates[iCPU][iCPU-6].m_iSize = 21;
	m_aUpdates[iCPU][iCPU-6].m_iProductionToInstall = 40;

	m_aUpdates[iCPU+1][iCPU-6].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU+1][iCPU-6].m_eStructure = STRUCTURE_ARTILLERYLOADER;
	m_aUpdates[iCPU+1][iCPU-6].m_eUpdateType = UPDATETYPE_TANKMOVEMENT;
	m_aUpdates[iCPU+1][iCPU-6].m_flValue = 1;
	m_aUpdates[iCPU+1][iCPU-6].m_iSize = 24;
	m_aUpdates[iCPU+1][iCPU-6].m_iProductionToInstall = 45;

	m_aUpdates[iCPU-1][iCPU-6].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-1][iCPU-6].m_eStructure = STRUCTURE_ARTILLERYLOADER;
	m_aUpdates[iCPU-1][iCPU-6].m_eUpdateType = UPDATETYPE_TANKHEALTH;
	m_aUpdates[iCPU-1][iCPU-6].m_flValue = 1;
	m_aUpdates[iCPU-1][iCPU-6].m_iSize = 24;
	m_aUpdates[iCPU-1][iCPU-6].m_iProductionToInstall = 45;

	m_aUpdates[iCPU+1][iCPU-7].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU+1][iCPU-7].m_eStructure = STRUCTURE_ARTILLERYLOADER;
	m_aUpdates[iCPU+1][iCPU-7].m_eUpdateType = UPDATETYPE_TANKRANGE;
	m_aUpdates[iCPU+1][iCPU-7].m_flValue = 1;
	m_aUpdates[iCPU+1][iCPU-7].m_iSize = 27;
	m_aUpdates[iCPU+1][iCPU-7].m_iProductionToInstall = 50;

	m_aUpdates[iCPU-1][iCPU-7].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-1][iCPU-7].m_eStructure = STRUCTURE_ARTILLERYLOADER;
	m_aUpdates[iCPU-1][iCPU-7].m_eUpdateType = UPDATETYPE_TANKATTACK;
	m_aUpdates[iCPU-1][iCPU-7].m_flValue = 1;
	m_aUpdates[iCPU-1][iCPU-7].m_iSize = 27;
	m_aUpdates[iCPU-1][iCPU-7].m_iProductionToInstall = 50;


	for (size_t i = 0; i < DigitanksGame()->GetNumTeams(); i++)
	{
		DigitanksGame()->GetDigitanksTeam(i)->DownloadUpdate(iCPU, iCPU, false);
		DigitanksGame()->GetDigitanksTeam(i)->DownloadComplete(false);
	}

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

		case STRUCTURE_BUFFER:
			return "Buffer";

		case STRUCTURE_PSU:
			return "Power Supply";

		case STRUCTURE_INFANTRYLOADER:
			return "Infantry Loader";

		case STRUCTURE_TANKLOADER:
			return "Main Battle Tank Loader";

		case STRUCTURE_ARTILLERYLOADER:
			return "Artillery Loader";

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

		case STRUCTURE_BUFFER:
			sResult = "Buffer ";
			break;

		case STRUCTURE_PSU:
			sResult = "Power Supply ";
			break;

		case STRUCTURE_INFANTRYLOADER:
			sResult = "Infantry Loader ";
			break;

		case STRUCTURE_TANKLOADER:
			sResult = "Tank Loader ";
			break;

		case STRUCTURE_ARTILLERYLOADER:
			sResult = "Artillery Loader ";
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

		case UPDATETYPE_SUPPORTENERGY:
			sResult.append("Support Energy Bonus");
			break;

		case UPDATETYPE_SUPPORTRECHARGE:
			sResult.append("Support Recharge Bonus");
			break;

		case UPDATETYPE_TANKATTACK:
			sResult.append("Attack Bonus");
			break;

		case UPDATETYPE_TANKDEFENSE:
			sResult.append("Defense Bonus");
			break;

		case UPDATETYPE_TANKMOVEMENT:
			sResult.append("Movement Bonus");
			break;

		case UPDATETYPE_TANKHEALTH:
			sResult.append("Health Bonus");
			break;

		case UPDATETYPE_TANKRANGE:
			sResult.append("Range Bonus");
			break;

		}
	}

	return sResult;
}
