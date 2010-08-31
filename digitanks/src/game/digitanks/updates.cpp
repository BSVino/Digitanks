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
	m_aUpdates[iCPU-3][iCPU].m_iSize = 18;
	m_aUpdates[iCPU-3][iCPU].m_iProductionToInstall = 30;

	m_aUpdates[iCPU-2][iCPU].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-2][iCPU].m_eStructure = STRUCTURE_CPU;
	m_aUpdates[iCPU-2][iCPU].m_eUpdateType = UPDATETYPE_PRODUCTION;
	m_aUpdates[iCPU-2][iCPU].m_flValue = 2;
	m_aUpdates[iCPU-2][iCPU].m_iSize = 9;
	m_aUpdates[iCPU-2][iCPU].m_iProductionToInstall = 15;

	m_aUpdates[iCPU-1][iCPU].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-1][iCPU].m_eStructure = STRUCTURE_CPU;
	m_aUpdates[iCPU-1][iCPU].m_eUpdateType = UPDATETYPE_FLEETSUPPLY;
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
	m_aUpdates[iCPU+2][iCPU].m_flValue = 2;
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
	m_aUpdates[iCPU-2][iCPU-1].m_flValue = 3;
	m_aUpdates[iCPU-2][iCPU-1].m_iSize = 12;
	m_aUpdates[iCPU-2][iCPU-1].m_iProductionToInstall = 20;

	m_aUpdates[iCPU-1][iCPU-1].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-1][iCPU-1].m_eStructure = STRUCTURE_CPU;
	m_aUpdates[iCPU-1][iCPU-1].m_eUpdateType = UPDATETYPE_PRODUCTION;
	m_aUpdates[iCPU-1][iCPU-1].m_flValue = 2;
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
	m_aUpdates[iCPU+1][iCPU-1].m_eUpdateType = UPDATETYPE_FLEETSUPPLY;
	m_aUpdates[iCPU+1][iCPU-1].m_flValue = 2;
	m_aUpdates[iCPU+1][iCPU-1].m_iSize = 9;
	m_aUpdates[iCPU+1][iCPU-1].m_iProductionToInstall = 15;

	m_aUpdates[iCPU+2][iCPU-1].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU+2][iCPU-1].m_eStructure = STRUCTURE_CPU;
	m_aUpdates[iCPU+2][iCPU-1].m_eUpdateType = UPDATETYPE_PRODUCTION;
	m_aUpdates[iCPU+2][iCPU-1].m_flValue = 3;
	m_aUpdates[iCPU+2][iCPU-1].m_iSize = 12;
	m_aUpdates[iCPU+2][iCPU-1].m_iProductionToInstall = 20;


	m_aUpdates[iCPU-1][iCPU-2].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-1][iCPU-2].m_eStructure = STRUCTURE_CPU;
	m_aUpdates[iCPU-1][iCPU-2].m_eUpdateType = UPDATETYPE_PRODUCTION;
	m_aUpdates[iCPU-1][iCPU-2].m_flValue = 3;
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
	m_aUpdates[iCPU+1][iCPU-2].m_flValue = 3;
	m_aUpdates[iCPU+1][iCPU-2].m_iSize = 12;
	m_aUpdates[iCPU+1][iCPU-2].m_iProductionToInstall = 20;


	m_aUpdates[iCPU][iCPU-3].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU][iCPU-3].m_eStructure = STRUCTURE_CPU;
	m_aUpdates[iCPU][iCPU-3].m_eUpdateType = UPDATETYPE_FLEETSUPPLY;
	m_aUpdates[iCPU][iCPU-3].m_flValue = 3;
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
	m_aUpdates[iCPU+2][iCPU-4].m_iProductionToInstall = 80;

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

	m_aUpdates[iCPU+3][iCPU-6].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU+3][iCPU-6].m_eStructure = STRUCTURE_BUFFER;
	m_aUpdates[iCPU+3][iCPU-6].m_eUpdateType = UPDATETYPE_BANDWIDTH;
	m_aUpdates[iCPU+3][iCPU-6].m_flValue = 2;
	m_aUpdates[iCPU+3][iCPU-6].m_iSize = 30;
	m_aUpdates[iCPU+3][iCPU-6].m_iProductionToInstall = 110;

	m_aUpdates[iCPU+4][iCPU-5].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU+4][iCPU-5].m_eStructure = STRUCTURE_BUFFER;
	m_aUpdates[iCPU+4][iCPU-5].m_eUpdateType = UPDATETYPE_SUPPORTRECHARGE;
	m_aUpdates[iCPU+4][iCPU-5].m_flValue = 1;
	m_aUpdates[iCPU+4][iCPU-5].m_iSize = 30;
	m_aUpdates[iCPU+4][iCPU-5].m_iProductionToInstall = 55;

	m_aUpdates[iCPU+3][iCPU-7].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU+3][iCPU-7].m_eStructure = STRUCTURE_BUFFER;
	m_aUpdates[iCPU+3][iCPU-7].m_eUpdateType = UPDATETYPE_SUPPORTRECHARGE;
	m_aUpdates[iCPU+3][iCPU-7].m_flValue = 3;
	m_aUpdates[iCPU+3][iCPU-7].m_iSize = 33;
	m_aUpdates[iCPU+3][iCPU-7].m_iProductionToInstall = 60;

	m_aUpdates[iCPU+4][iCPU-6].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU+4][iCPU-6].m_eStructure = STRUCTURE_BUFFER;
	m_aUpdates[iCPU+4][iCPU-6].m_eUpdateType = UPDATETYPE_SUPPORTENERGY;
	m_aUpdates[iCPU+4][iCPU-6].m_flValue = 3;
	m_aUpdates[iCPU+4][iCPU-6].m_iSize = 33;
	m_aUpdates[iCPU+4][iCPU-6].m_iProductionToInstall = 60;

	m_aUpdates[iCPU+5][iCPU-5].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU+5][iCPU-5].m_eStructure = STRUCTURE_BUFFER;
	m_aUpdates[iCPU+5][iCPU-5].m_eUpdateType = UPDATETYPE_FLEETSUPPLY;
	m_aUpdates[iCPU+5][iCPU-5].m_flValue = 3;
	m_aUpdates[iCPU+5][iCPU-5].m_iSize = 33;
	m_aUpdates[iCPU+5][iCPU-5].m_iProductionToInstall = 60;


	m_aUpdates[iCPU-4][iCPU-1].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-4][iCPU-1].m_eStructure = STRUCTURE_TANKLOADER;
	m_aUpdates[iCPU-4][iCPU-1].m_eUpdateType = UPDATETYPE_TANKATTACK;
	m_aUpdates[iCPU-4][iCPU-1].m_flValue = 1;
	m_aUpdates[iCPU-4][iCPU-1].m_iSize = 36;
	m_aUpdates[iCPU-4][iCPU-1].m_iProductionToInstall = 35;

	m_aUpdates[iCPU-5][iCPU-1].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-5][iCPU-1].m_eStructure = STRUCTURE_TANKLOADER;
	m_aUpdates[iCPU-5][iCPU-1].m_eUpdateType = UPDATETYPE_TANKMOVEMENT;
	m_aUpdates[iCPU-5][iCPU-1].m_flValue = 1;
	m_aUpdates[iCPU-5][iCPU-1].m_iSize = 42;
	m_aUpdates[iCPU-5][iCPU-1].m_iProductionToInstall = 40;

	m_aUpdates[iCPU-6][iCPU-1].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-6][iCPU-1].m_eStructure = STRUCTURE_TANKLOADER;
	m_aUpdates[iCPU-6][iCPU-1].m_eUpdateType = UPDATETYPE_TANKHEALTH;
	m_aUpdates[iCPU-6][iCPU-1].m_flValue = 1;
	m_aUpdates[iCPU-6][iCPU-1].m_iSize = 48;
	m_aUpdates[iCPU-6][iCPU-1].m_iProductionToInstall = 45;

	m_aUpdates[iCPU-5][iCPU-2].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-5][iCPU-2].m_eStructure = STRUCTURE_TANKLOADER;
	m_aUpdates[iCPU-5][iCPU-2].m_eUpdateType = UPDATETYPE_TANKDEFENSE;
	m_aUpdates[iCPU-5][iCPU-2].m_flValue = 1;
	m_aUpdates[iCPU-5][iCPU-2].m_iSize = 48;
	m_aUpdates[iCPU-5][iCPU-2].m_iProductionToInstall = 45;

	m_aUpdates[iCPU-6][iCPU-2].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-6][iCPU-2].m_eStructure = STRUCTURE_TANKLOADER;
	m_aUpdates[iCPU-6][iCPU-2].m_eUpdateType = UPDATETYPE_TANKATTACK;
	m_aUpdates[iCPU-6][iCPU-2].m_flValue = 1;
	m_aUpdates[iCPU-6][iCPU-2].m_iSize = 54;
	m_aUpdates[iCPU-6][iCPU-2].m_iProductionToInstall = 50;

	m_aUpdates[iCPU-5][iCPU-3].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-5][iCPU-3].m_eStructure = STRUCTURE_TANKLOADER;
	m_aUpdates[iCPU-5][iCPU-3].m_eUpdateType = UPDATETYPE_TANKMOVEMENT;
	m_aUpdates[iCPU-5][iCPU-3].m_flValue = 1;
	m_aUpdates[iCPU-5][iCPU-3].m_iSize = 54;
	m_aUpdates[iCPU-5][iCPU-3].m_iProductionToInstall = 50;

	m_aUpdates[iCPU-7][iCPU-1].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-7][iCPU-1].m_eStructure = STRUCTURE_TANKLOADER;
	m_aUpdates[iCPU-7][iCPU-1].m_eUpdateType = UPDATETYPE_TANKMOVEMENT;
	m_aUpdates[iCPU-7][iCPU-1].m_flValue = 1;
	m_aUpdates[iCPU-7][iCPU-1].m_iSize = 54;
	m_aUpdates[iCPU-7][iCPU-1].m_iProductionToInstall = 50;

	m_aUpdates[iCPU-7][iCPU-2].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-7][iCPU-2].m_eStructure = STRUCTURE_TANKLOADER;
	m_aUpdates[iCPU-7][iCPU-2].m_eUpdateType = UPDATETYPE_TANKDEFENSE;
	m_aUpdates[iCPU-7][iCPU-2].m_flValue = 1;
	m_aUpdates[iCPU-7][iCPU-2].m_iSize = 60;
	m_aUpdates[iCPU-7][iCPU-2].m_iProductionToInstall = 55;

	m_aUpdates[iCPU-6][iCPU-3].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-6][iCPU-3].m_eStructure = STRUCTURE_TANKLOADER;
	m_aUpdates[iCPU-6][iCPU-3].m_eUpdateType = UPDATETYPE_TANKHEALTH;
	m_aUpdates[iCPU-6][iCPU-3].m_flValue = 1;
	m_aUpdates[iCPU-6][iCPU-3].m_iSize = 60;
	m_aUpdates[iCPU-6][iCPU-3].m_iProductionToInstall = 55;

	m_aUpdates[iCPU-7][iCPU-3].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-7][iCPU-3].m_eStructure = STRUCTURE_TANKLOADER;
	m_aUpdates[iCPU-7][iCPU-3].m_eUpdateType = UPDATETYPE_TANKATTACK;
	m_aUpdates[iCPU-7][iCPU-3].m_flValue = 1;
	m_aUpdates[iCPU-7][iCPU-3].m_iSize = 66;
	m_aUpdates[iCPU-7][iCPU-3].m_iProductionToInstall = 60;

	m_aUpdates[iCPU-6][iCPU-4].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-6][iCPU-4].m_eStructure = STRUCTURE_TANKLOADER;
	m_aUpdates[iCPU-6][iCPU-4].m_eUpdateType = UPDATETYPE_TANKDEFENSE;
	m_aUpdates[iCPU-6][iCPU-4].m_flValue = 1;
	m_aUpdates[iCPU-6][iCPU-4].m_iSize = 66;
	m_aUpdates[iCPU-6][iCPU-4].m_iProductionToInstall = 60;


	m_aUpdates[iCPU-2][iCPU-3].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-2][iCPU-3].m_eStructure = STRUCTURE_INFANTRYLOADER;
	m_aUpdates[iCPU-2][iCPU-3].m_eUpdateType = UPDATETYPE_TANKDEFENSE;
	m_aUpdates[iCPU-2][iCPU-3].m_flValue = 1;
	m_aUpdates[iCPU-2][iCPU-3].m_iSize = 18;
	m_aUpdates[iCPU-2][iCPU-3].m_iProductionToInstall = 17;

	m_aUpdates[iCPU-2][iCPU-4].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-2][iCPU-4].m_eStructure = STRUCTURE_INFANTRYLOADER;
	m_aUpdates[iCPU-2][iCPU-4].m_eUpdateType = UPDATETYPE_TANKHEALTH;
	m_aUpdates[iCPU-2][iCPU-4].m_flValue = 1;
	m_aUpdates[iCPU-2][iCPU-4].m_iSize = 20;
	m_aUpdates[iCPU-2][iCPU-4].m_iProductionToInstall = 20;

	m_aUpdates[iCPU-2][iCPU-5].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-2][iCPU-5].m_eStructure = STRUCTURE_INFANTRYLOADER;
	m_aUpdates[iCPU-2][iCPU-5].m_eUpdateType = UPDATETYPE_TANKATTACK;
	m_aUpdates[iCPU-2][iCPU-5].m_flValue = 1;
	m_aUpdates[iCPU-2][iCPU-5].m_iSize = 24;
	m_aUpdates[iCPU-2][iCPU-5].m_iProductionToInstall = 23;

	m_aUpdates[iCPU-3][iCPU-4].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-3][iCPU-4].m_eStructure = STRUCTURE_INFANTRYLOADER;
	m_aUpdates[iCPU-3][iCPU-4].m_eUpdateType = UPDATETYPE_TANKMOVEMENT;
	m_aUpdates[iCPU-3][iCPU-4].m_flValue = 1;
	m_aUpdates[iCPU-3][iCPU-4].m_iSize = 24;
	m_aUpdates[iCPU-3][iCPU-4].m_iProductionToInstall = 23;

	m_aUpdates[iCPU-3][iCPU-5].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-3][iCPU-5].m_eStructure = STRUCTURE_INFANTRYLOADER;
	m_aUpdates[iCPU-3][iCPU-5].m_eUpdateType = UPDATETYPE_TANKDEFENSE;
	m_aUpdates[iCPU-3][iCPU-5].m_flValue = 1;
	m_aUpdates[iCPU-3][iCPU-5].m_iSize = 26;
	m_aUpdates[iCPU-3][iCPU-5].m_iProductionToInstall = 26;

	m_aUpdates[iCPU-4][iCPU-4].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-4][iCPU-4].m_eStructure = STRUCTURE_INFANTRYLOADER;
	m_aUpdates[iCPU-4][iCPU-4].m_eUpdateType = UPDATETYPE_TANKHEALTH;
	m_aUpdates[iCPU-4][iCPU-4].m_flValue = 1;
	m_aUpdates[iCPU-4][iCPU-4].m_iSize = 26;
	m_aUpdates[iCPU-4][iCPU-4].m_iProductionToInstall = 26;

	m_aUpdates[iCPU-3][iCPU-6].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-3][iCPU-6].m_eStructure = STRUCTURE_INFANTRYLOADER;
	m_aUpdates[iCPU-3][iCPU-6].m_eUpdateType = UPDATETYPE_TANKMOVEMENT;
	m_aUpdates[iCPU-3][iCPU-6].m_flValue = 1;
	m_aUpdates[iCPU-3][iCPU-6].m_iSize = 28;
	m_aUpdates[iCPU-3][iCPU-6].m_iProductionToInstall = 29;

	m_aUpdates[iCPU-4][iCPU-5].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-4][iCPU-5].m_eStructure = STRUCTURE_INFANTRYLOADER;
	m_aUpdates[iCPU-4][iCPU-5].m_eUpdateType = UPDATETYPE_TANKATTACK;
	m_aUpdates[iCPU-4][iCPU-5].m_flValue = 1;
	m_aUpdates[iCPU-4][iCPU-5].m_iSize = 28;
	m_aUpdates[iCPU-4][iCPU-5].m_iProductionToInstall = 29;

	m_aUpdates[iCPU-5][iCPU-5].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-5][iCPU-5].m_eStructure = STRUCTURE_INFANTRYLOADER;
	m_aUpdates[iCPU-5][iCPU-5].m_eUpdateType = UPDATETYPE_TANKDEFENSE;
	m_aUpdates[iCPU-5][iCPU-5].m_flValue = 1;
	m_aUpdates[iCPU-5][iCPU-5].m_iSize = 30;
	m_aUpdates[iCPU-5][iCPU-5].m_iProductionToInstall = 32;

	m_aUpdates[iCPU-4][iCPU-6].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-4][iCPU-6].m_eStructure = STRUCTURE_INFANTRYLOADER;
	m_aUpdates[iCPU-4][iCPU-6].m_eUpdateType = UPDATETYPE_TANKHEALTH;
	m_aUpdates[iCPU-4][iCPU-6].m_flValue = 1;
	m_aUpdates[iCPU-4][iCPU-6].m_iSize = 30;
	m_aUpdates[iCPU-4][iCPU-6].m_iProductionToInstall = 32;

	m_aUpdates[iCPU-3][iCPU-7].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-3][iCPU-7].m_eStructure = STRUCTURE_INFANTRYLOADER;
	m_aUpdates[iCPU-3][iCPU-7].m_eUpdateType = UPDATETYPE_TANKATTACK;
	m_aUpdates[iCPU-3][iCPU-7].m_flValue = 1;
	m_aUpdates[iCPU-3][iCPU-7].m_iSize = 30;
	m_aUpdates[iCPU-3][iCPU-7].m_iProductionToInstall = 32;


	m_aUpdates[iCPU][iCPU-5].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU][iCPU-5].m_eStructure = STRUCTURE_ARTILLERYLOADER;
	m_aUpdates[iCPU][iCPU-5].m_eUpdateType = UPDATETYPE_TANKRANGE;
	m_aUpdates[iCPU][iCPU-5].m_flValue = 10;
	m_aUpdates[iCPU][iCPU-5].m_iSize = 36;
	m_aUpdates[iCPU][iCPU-5].m_iProductionToInstall = 35;

	m_aUpdates[iCPU][iCPU-6].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU][iCPU-6].m_eStructure = STRUCTURE_ARTILLERYLOADER;
	m_aUpdates[iCPU][iCPU-6].m_eUpdateType = UPDATETYPE_TANKATTACK;
	m_aUpdates[iCPU][iCPU-6].m_flValue = 1;
	m_aUpdates[iCPU][iCPU-6].m_iSize = 42;
	m_aUpdates[iCPU][iCPU-6].m_iProductionToInstall = 40;

	m_aUpdates[iCPU+1][iCPU-6].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU+1][iCPU-6].m_eStructure = STRUCTURE_ARTILLERYLOADER;
	m_aUpdates[iCPU+1][iCPU-6].m_eUpdateType = UPDATETYPE_TANKMOVEMENT;
	m_aUpdates[iCPU+1][iCPU-6].m_flValue = 1;
	m_aUpdates[iCPU+1][iCPU-6].m_iSize = 48;
	m_aUpdates[iCPU+1][iCPU-6].m_iProductionToInstall = 45;

	m_aUpdates[iCPU-1][iCPU-6].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-1][iCPU-6].m_eStructure = STRUCTURE_ARTILLERYLOADER;
	m_aUpdates[iCPU-1][iCPU-6].m_eUpdateType = UPDATETYPE_TANKHEALTH;
	m_aUpdates[iCPU-1][iCPU-6].m_flValue = 1;
	m_aUpdates[iCPU-1][iCPU-6].m_iSize = 48;
	m_aUpdates[iCPU-1][iCPU-6].m_iProductionToInstall = 45;

	m_aUpdates[iCPU][iCPU-7].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU][iCPU-7].m_eStructure = STRUCTURE_ARTILLERYLOADER;
	m_aUpdates[iCPU][iCPU-7].m_eUpdateType = UPDATETYPE_TANKMOVEMENT;
	m_aUpdates[iCPU][iCPU-7].m_flValue = 1;
	m_aUpdates[iCPU][iCPU-7].m_iSize = 48;
	m_aUpdates[iCPU][iCPU-7].m_iProductionToInstall = 45;

	m_aUpdates[iCPU+1][iCPU-7].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU+1][iCPU-7].m_eStructure = STRUCTURE_ARTILLERYLOADER;
	m_aUpdates[iCPU+1][iCPU-7].m_eUpdateType = UPDATETYPE_TANKRANGE;
	m_aUpdates[iCPU+1][iCPU-7].m_flValue = 1;
	m_aUpdates[iCPU+1][iCPU-7].m_iSize = 54;
	m_aUpdates[iCPU+1][iCPU-7].m_iProductionToInstall = 50;

	m_aUpdates[iCPU-1][iCPU-7].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-1][iCPU-7].m_eStructure = STRUCTURE_ARTILLERYLOADER;
	m_aUpdates[iCPU-1][iCPU-7].m_eUpdateType = UPDATETYPE_TANKATTACK;
	m_aUpdates[iCPU-1][iCPU-7].m_flValue = 1;
	m_aUpdates[iCPU-1][iCPU-7].m_iSize = 54;
	m_aUpdates[iCPU-1][iCPU-7].m_iProductionToInstall = 50;

	m_aUpdates[iCPU][iCPU-8].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU][iCPU-8].m_eStructure = STRUCTURE_ARTILLERYLOADER;
	m_aUpdates[iCPU][iCPU-8].m_eUpdateType = UPDATETYPE_TANKHEALTH;
	m_aUpdates[iCPU][iCPU-8].m_flValue = 1;
	m_aUpdates[iCPU][iCPU-8].m_iSize = 54;
	m_aUpdates[iCPU][iCPU-8].m_iProductionToInstall = 50;

	m_aUpdates[iCPU-1][iCPU-8].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-1][iCPU-8].m_eStructure = STRUCTURE_ARTILLERYLOADER;
	m_aUpdates[iCPU-1][iCPU-8].m_eUpdateType = UPDATETYPE_TANKRANGE;
	m_aUpdates[iCPU-1][iCPU-8].m_flValue = 1;
	m_aUpdates[iCPU-1][iCPU-8].m_iSize = 54;
	m_aUpdates[iCPU-1][iCPU-8].m_iProductionToInstall = 50;

	m_aUpdates[iCPU][iCPU-9].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU][iCPU-9].m_eStructure = STRUCTURE_ARTILLERYLOADER;
	m_aUpdates[iCPU][iCPU-9].m_eUpdateType = UPDATETYPE_TANKMOVEMENT;
	m_aUpdates[iCPU][iCPU-9].m_flValue = 1;
	m_aUpdates[iCPU][iCPU-9].m_iSize = 54;
	m_aUpdates[iCPU][iCPU-9].m_iProductionToInstall = 50;

	m_aUpdates[iCPU+1][iCPU-8].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU+1][iCPU-8].m_eStructure = STRUCTURE_ARTILLERYLOADER;
	m_aUpdates[iCPU+1][iCPU-8].m_eUpdateType = UPDATETYPE_TANKATTACK;
	m_aUpdates[iCPU+1][iCPU-8].m_flValue = 1;
	m_aUpdates[iCPU+1][iCPU-8].m_iSize = 54;
	m_aUpdates[iCPU+1][iCPU-8].m_iProductionToInstall = 50;


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

std::wstring CUpdateItem::GetName()
{
	std::wstring sResult;

	if (m_eUpdateClass == UPDATECLASS_STRUCTURE)
	{
		switch (m_eStructure)
		{
		case STRUCTURE_CPU:
			return L"CPU";

		case STRUCTURE_BUFFER:
			return L"Buffer";

		case STRUCTURE_PSU:
			return L"Power Supply";

		case STRUCTURE_INFANTRYLOADER:
			return L"Infantry Loader";

		case STRUCTURE_TANKLOADER:
			return L"Main Battle Tank Loader";

		case STRUCTURE_ARTILLERYLOADER:
			return L"Artillery Loader";

		default:
			return L"Structure";
		}
	}
	else if (m_eUpdateClass == UPDATECLASS_STRUCTUREUPDATE)
	{
		switch (m_eStructure)
		{
		case STRUCTURE_CPU:
			sResult = L"CPU ";
			break;

		case STRUCTURE_BUFFER:
			sResult = L"Buffer ";
			break;

		case STRUCTURE_PSU:
			sResult = L"Power Supply ";
			break;

		case STRUCTURE_INFANTRYLOADER:
			sResult = L"Infantry Loader ";
			break;

		case STRUCTURE_TANKLOADER:
			sResult = L"Tank Loader ";
			break;

		case STRUCTURE_ARTILLERYLOADER:
			sResult = L"Artillery Loader ";
			break;

		default:
			sResult = L"Structure ";
		}

		switch (m_eUpdateType)
		{
		case UPDATETYPE_PRODUCTION:
			sResult.append(L"Power");
			break;

		case UPDATETYPE_BANDWIDTH:
			sResult.append(L"Bandwidth");
			break;

		case UPDATETYPE_FLEETSUPPLY:
			sResult.append(L"Fleet Supply");
			break;

		case UPDATETYPE_SUPPORTENERGY:
			sResult.append(L"Support Energy Bonus");
			break;

		case UPDATETYPE_SUPPORTRECHARGE:
			sResult.append(L"Support Recharge Bonus");
			break;

		case UPDATETYPE_TANKATTACK:
			sResult.append(L"Attack Bonus");
			break;

		case UPDATETYPE_TANKDEFENSE:
			sResult.append(L"Defense Bonus");
			break;

		case UPDATETYPE_TANKMOVEMENT:
			sResult.append(L"Movement Bonus");
			break;

		case UPDATETYPE_TANKHEALTH:
			sResult.append(L"Health Bonus");
			break;

		case UPDATETYPE_TANKRANGE:
			sResult.append(L"Range Bonus");
			break;

		}
	}

	return sResult;
}

std::wstring CUpdateItem::GetInfo()
{
	std::wstring sResult;

	if (m_eUpdateClass == UPDATECLASS_STRUCTURE)
	{
		switch (m_eStructure)
		{
		case STRUCTURE_CPU:
			return L"This program allows the player to construct a CPU. The CPU is the life and brains of your operation. It allows you to build structures and units.";

		case STRUCTURE_BUFFER:
			return L"This program allows the player to construct Buffers, which expand your territory and provide support to your tanks. They can be upgraded to provide additional fleet supply points and bandwidth as well.";

		case STRUCTURE_PSU:
			return L"This program allows the player to construct Power Supplies to mine the valuable Electronodes that provide your base with more Power. Then you can further your nefarious plans to construct units and erect structures.";

		case STRUCTURE_INFANTRYLOADER:
			return L"This program allows the player to construct Infantry Loaders, which provide the Mechanized Infantry unit, an cheap but essential defensive unit that can be fortified to defend your base.";

		case STRUCTURE_TANKLOADER:
			return L"This program allows the player to construct the Main Battle Tank Loader which produces Main Battle Tanks, the primary tool of defeating your enemies. These units are more expensive to build.";

		case STRUCTURE_ARTILLERYLOADER:
			return L"This program allows the player to construct Artillery Loaders. These provide the Artillery unit, which can support your tanks in their attacks by pulverizing the enemy from afar. Artillery do double damage to shields, but only half damage to structures and tank hulls.";

		default:
			return L"Somehow you have found a way to construct an edifice which shouldn't exist. Please be careful not to open a wormhole while you're doing it.";
		}
	}
	else if (m_eUpdateClass == UPDATECLASS_STRUCTUREUPDATE)
	{
		switch (m_eUpdateType)
		{
		case UPDATETYPE_PRODUCTION:
			return L"This update increases the amount of power that you generate per turn. Power helps you build structures and units faster. This update must be installed before use.";

		case UPDATETYPE_BANDWIDTH:
			return L"This update increases the amount of bandwidth available to you. Increased bandwidth helps you download updates and programs faster. This update must be installed before use.";

		case UPDATETYPE_FLEETSUPPLY:
			return L"This update increases your maximum fleet capacity, so that you can build more tanks. This update must be installed before use.";

		case UPDATETYPE_SUPPORTENERGY:
			return L"With this update installed, units supported by this buffer will receive bonuses to Attack and Defense energy.";

		case UPDATETYPE_SUPPORTRECHARGE:
			return L"When this update is installed, units supported by this buffer will recharge more health and shields each turn.";

		case UPDATETYPE_TANKATTACK:
			return L"This update once installed gives every tank that is produced from this loader an automatic increase to its Attack Energy.";

		case UPDATETYPE_TANKDEFENSE:
			return L"This update once installed gives every tank that is produced from this loader an automatic increase to its Defense Energy.";

		case UPDATETYPE_TANKMOVEMENT:
			return L"This update once installed gives every tank that is produced from this loader an automatic increase to its Movement Energy.";

		case UPDATETYPE_TANKHEALTH:
			return L"This update once installed gives every tank that is produced from this loader an automatic increase to its health.";

		case UPDATETYPE_TANKRANGE:
			return L"This update once installed gives every tank that is produced from this loader an automatic increase to its maximum firing range.";
		}
	}

	return L"If you can figure out what hell this thing does, please feel free to let me know.";
}
