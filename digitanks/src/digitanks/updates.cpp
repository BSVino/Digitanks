#include "updates.h"

#include <network/network.h>

#include "digitanksgame.h"

REGISTER_ENTITY(CUpdateGrid);

NETVAR_TABLE_BEGIN(CUpdateGrid);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CUpdateGrid);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYARRAY, CUpdateItem, m_aUpdates);

	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, int, m_iLowestX);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, int, m_iHighestX);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, int, m_iLowestY);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, int, m_iHighestY);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CUpdateGrid);
INPUTS_TABLE_END();

void CUpdateGrid::SetupStandardUpdates()
{
	int iCPU = UPDATE_GRID_SIZE/2;

	memset(&m_aUpdates[0][0], 0, sizeof(m_aUpdates));

	// CPU downloads
	m_aUpdates[iCPU][iCPU].m_eUpdateClass = UPDATECLASS_STRUCTURE;
	m_aUpdates[iCPU][iCPU].m_eStructure = STRUCTURE_CPU;
	m_aUpdates[iCPU][iCPU].m_flSize = 0;

	m_aUpdates[iCPU+1][iCPU].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU+1][iCPU].m_eStructure = STRUCTURE_CPU;
	m_aUpdates[iCPU+1][iCPU].m_eUpdateType = UPDATETYPE_PRODUCTION;
	m_aUpdates[iCPU+1][iCPU].m_flValue = 0.3f;
	m_aUpdates[iCPU+1][iCPU].m_flSize = 4;

	m_aUpdates[iCPU-1][iCPU].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-1][iCPU].m_eStructure = STRUCTURE_CPU;
	m_aUpdates[iCPU-1][iCPU].m_eUpdateType = UPDATETYPE_PRODUCTION;
	m_aUpdates[iCPU-1][iCPU].m_flValue = 0.3f;
	m_aUpdates[iCPU-1][iCPU].m_flSize = 4;

	m_aUpdates[iCPU][iCPU-1].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU][iCPU-1].m_eStructure = STRUCTURE_CPU;
	m_aUpdates[iCPU][iCPU-1].m_eUpdateType = UPDATETYPE_FLEETSUPPLY;
	m_aUpdates[iCPU][iCPU-1].m_flValue = 1;
	m_aUpdates[iCPU][iCPU-1].m_flSize = 4;

	m_aUpdates[iCPU-1][iCPU-1].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-1][iCPU-1].m_eStructure = STRUCTURE_CPU;
	m_aUpdates[iCPU-1][iCPU-1].m_eUpdateType = UPDATETYPE_FLEETSUPPLY;
	m_aUpdates[iCPU-1][iCPU-1].m_flValue = 1;
	m_aUpdates[iCPU-1][iCPU-1].m_flSize = 8;

	m_aUpdates[iCPU+1][iCPU-1].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU+1][iCPU-1].m_eStructure = STRUCTURE_CPU;
	m_aUpdates[iCPU+1][iCPU-1].m_eUpdateType = UPDATETYPE_PRODUCTION;
	m_aUpdates[iCPU+1][iCPU-1].m_flValue = 0.3f;
	m_aUpdates[iCPU+1][iCPU-1].m_flSize = 8;

	m_aUpdates[iCPU][iCPU-2].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU][iCPU-2].m_eStructure = STRUCTURE_CPU;
	m_aUpdates[iCPU][iCPU-2].m_eUpdateType = UPDATETYPE_BANDWIDTH;
	m_aUpdates[iCPU][iCPU-2].m_flValue = 1;
	m_aUpdates[iCPU][iCPU-2].m_flSize = 8;

	m_aUpdates[iCPU+1][iCPU-2].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU+1][iCPU-2].m_eStructure = STRUCTURE_CPU;
	m_aUpdates[iCPU+1][iCPU-2].m_eUpdateType = UPDATETYPE_FLEETSUPPLY;
	m_aUpdates[iCPU+1][iCPU-2].m_flValue = 1;
	m_aUpdates[iCPU+1][iCPU-2].m_flSize = 12;

	m_aUpdates[iCPU+1][iCPU-3].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU+1][iCPU-3].m_eStructure = STRUCTURE_CPU;
	m_aUpdates[iCPU+1][iCPU-3].m_eUpdateType = UPDATETYPE_BANDWIDTH;
	m_aUpdates[iCPU+1][iCPU-3].m_flValue = 2.0f;
	m_aUpdates[iCPU+1][iCPU-3].m_flSize = 15;

	m_aUpdates[iCPU+2][iCPU-2].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU+2][iCPU-2].m_eStructure = STRUCTURE_CPU;
	m_aUpdates[iCPU+2][iCPU-2].m_eUpdateType = UPDATETYPE_PRODUCTION;
	m_aUpdates[iCPU+2][iCPU-2].m_flValue = 0.6f;
	m_aUpdates[iCPU+2][iCPU-2].m_flSize = 15;

	m_aUpdates[iCPU+2][iCPU-3].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU+2][iCPU-3].m_eStructure = STRUCTURE_CPU;
	m_aUpdates[iCPU+2][iCPU-3].m_eUpdateType = UPDATETYPE_FLEETSUPPLY;
	m_aUpdates[iCPU+2][iCPU-3].m_flValue = 2;
	m_aUpdates[iCPU+2][iCPU-3].m_flSize = 18;

	m_aUpdates[iCPU+3][iCPU-3].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU+3][iCPU-3].m_eStructure = STRUCTURE_CPU;
	m_aUpdates[iCPU+3][iCPU-3].m_eUpdateType = UPDATETYPE_PRODUCTION;
	m_aUpdates[iCPU+3][iCPU-3].m_flValue = 1.0f;
	m_aUpdates[iCPU+3][iCPU-3].m_flSize = 18;

	m_aUpdates[iCPU+4][iCPU-3].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU+4][iCPU-3].m_eStructure = STRUCTURE_CPU;
	m_aUpdates[iCPU+4][iCPU-3].m_eUpdateType = UPDATETYPE_FLEETSUPPLY;
	m_aUpdates[iCPU+4][iCPU-3].m_flValue = 3;
	m_aUpdates[iCPU+4][iCPU-3].m_flSize = 21;

	m_aUpdates[iCPU+4][iCPU-4].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU+4][iCPU-4].m_eStructure = STRUCTURE_CPU;
	m_aUpdates[iCPU+4][iCPU-4].m_eUpdateType = UPDATETYPE_PRODUCTION;
	m_aUpdates[iCPU+4][iCPU-4].m_flValue = 1.5f;
	m_aUpdates[iCPU+4][iCPU-4].m_flSize = 24;

	m_aUpdates[iCPU+5][iCPU-4].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU+5][iCPU-4].m_eStructure = STRUCTURE_CPU;
	m_aUpdates[iCPU+5][iCPU-4].m_eUpdateType = UPDATETYPE_FLEETSUPPLY;
	m_aUpdates[iCPU+5][iCPU-4].m_flValue = 4.0f;
	m_aUpdates[iCPU+5][iCPU-4].m_flSize = 27;

	m_aUpdates[iCPU+5][iCPU-5].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU+5][iCPU-5].m_eStructure = STRUCTURE_CPU;
	m_aUpdates[iCPU+5][iCPU-5].m_eUpdateType = UPDATETYPE_BANDWIDTH;
	m_aUpdates[iCPU+5][iCPU-5].m_flValue = 4;
	m_aUpdates[iCPU+5][iCPU-5].m_flSize = 33;


	// Infantry downloads
	m_aUpdates[iCPU-1][iCPU-2].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-1][iCPU-2].m_eStructure = STRUCTURE_INFANTRYLOADER;
	m_aUpdates[iCPU-1][iCPU-2].m_eUpdateType = UPDATETYPE_TANKDEFENSE;
	m_aUpdates[iCPU-1][iCPU-2].m_flValue = 1;
	m_aUpdates[iCPU-1][iCPU-2].m_flSize = 18;

	m_aUpdates[iCPU-2][iCPU-2].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-2][iCPU-2].m_eStructure = STRUCTURE_INFANTRYLOADER;
	m_aUpdates[iCPU-2][iCPU-2].m_eUpdateType = UPDATETYPE_TANKHEALTH;
	m_aUpdates[iCPU-2][iCPU-2].m_flValue = 1;
	m_aUpdates[iCPU-2][iCPU-2].m_flSize = 20;

	m_aUpdates[iCPU-1][iCPU-3].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-1][iCPU-3].m_eStructure = STRUCTURE_INFANTRYLOADER;
	m_aUpdates[iCPU-1][iCPU-3].m_eUpdateType = UPDATETYPE_TANKATTACK;
	m_aUpdates[iCPU-1][iCPU-3].m_flValue = 1;
	m_aUpdates[iCPU-1][iCPU-3].m_flSize = 20;

	m_aUpdates[iCPU-3][iCPU-2].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-3][iCPU-2].m_eStructure = STRUCTURE_INFANTRYLOADER;
	m_aUpdates[iCPU-3][iCPU-2].m_eUpdateType = UPDATETYPE_TANKATTACK;
	m_aUpdates[iCPU-3][iCPU-2].m_flValue = 1;
	m_aUpdates[iCPU-3][iCPU-2].m_flSize = 22;

	m_aUpdates[iCPU-2][iCPU-3].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-2][iCPU-3].m_eStructure = STRUCTURE_INFANTRYLOADER;
	m_aUpdates[iCPU-2][iCPU-3].m_eUpdateType = UPDATETYPE_TANKMOVEMENT;
	m_aUpdates[iCPU-2][iCPU-3].m_flValue = 1;
	m_aUpdates[iCPU-2][iCPU-3].m_flSize = 22;

	m_aUpdates[iCPU-3][iCPU-3].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-3][iCPU-3].m_eStructure = STRUCTURE_INFANTRYLOADER;
	m_aUpdates[iCPU-3][iCPU-3].m_eUpdateType = UPDATETYPE_TANKHEALTH;
	m_aUpdates[iCPU-3][iCPU-3].m_flValue = 1;
	m_aUpdates[iCPU-3][iCPU-3].m_flSize = 24;

	m_aUpdates[iCPU-4][iCPU-3].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-4][iCPU-3].m_eStructure = STRUCTURE_INFANTRYLOADER;
	m_aUpdates[iCPU-4][iCPU-3].m_eUpdateType = UPDATETYPE_TANKMOVEMENT;
	m_aUpdates[iCPU-4][iCPU-3].m_flValue = 1;
	m_aUpdates[iCPU-4][iCPU-3].m_flSize = 26;

	m_aUpdates[iCPU-4][iCPU-4].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-4][iCPU-4].m_eStructure = STRUCTURE_INFANTRYLOADER;
	m_aUpdates[iCPU-4][iCPU-4].m_eUpdateType = UPDATETYPE_TANKATTACK;
	m_aUpdates[iCPU-4][iCPU-4].m_flValue = 2;
	m_aUpdates[iCPU-4][iCPU-4].m_flSize = 28;

	m_aUpdates[iCPU-5][iCPU-4].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-5][iCPU-4].m_eStructure = STRUCTURE_INFANTRYLOADER;
	m_aUpdates[iCPU-5][iCPU-4].m_eUpdateType = UPDATETYPE_TANKMOVEMENT;
	m_aUpdates[iCPU-5][iCPU-4].m_flValue = 2;
	m_aUpdates[iCPU-5][iCPU-4].m_flSize = 30;

	m_aUpdates[iCPU-5][iCPU-5].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-5][iCPU-5].m_eStructure = STRUCTURE_INFANTRYLOADER;
	m_aUpdates[iCPU-5][iCPU-5].m_eUpdateType = UPDATETYPE_TANKDEFENSE;
	m_aUpdates[iCPU-5][iCPU-5].m_flValue = 2;
	m_aUpdates[iCPU-5][iCPU-5].m_flSize = 32;


	// Tank downloads
	m_aUpdates[iCPU-3][iCPU-4].m_eUpdateClass = UPDATECLASS_STRUCTURE;
	m_aUpdates[iCPU-3][iCPU-4].m_eStructure = STRUCTURE_TANKLOADER;
	m_aUpdates[iCPU-3][iCPU-4].m_flSize = 40;

	m_aUpdates[iCPU-3][iCPU-5].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-3][iCPU-5].m_eStructure = STRUCTURE_TANKLOADER;
	m_aUpdates[iCPU-3][iCPU-5].m_eUpdateType = UPDATETYPE_TANKATTACK;
	m_aUpdates[iCPU-3][iCPU-5].m_flValue = 1;
	m_aUpdates[iCPU-3][iCPU-5].m_flSize = 24;

	m_aUpdates[iCPU-3][iCPU-6].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-3][iCPU-6].m_eStructure = STRUCTURE_TANKLOADER;
	m_aUpdates[iCPU-3][iCPU-6].m_eUpdateType = UPDATETYPE_TANKMOVEMENT;
	m_aUpdates[iCPU-3][iCPU-6].m_flValue = 1;
	m_aUpdates[iCPU-3][iCPU-6].m_flSize = 28;

	m_aUpdates[iCPU-2][iCPU-5].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-2][iCPU-5].m_eStructure = STRUCTURE_TANKLOADER;
	m_aUpdates[iCPU-2][iCPU-5].m_eUpdateType = UPDATETYPE_TANKDEFENSE;
	m_aUpdates[iCPU-2][iCPU-5].m_flValue = 1;
	m_aUpdates[iCPU-2][iCPU-5].m_flSize = 28;

	m_aUpdates[iCPU-4][iCPU-6].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-4][iCPU-6].m_eStructure = STRUCTURE_TANKLOADER;
	m_aUpdates[iCPU-4][iCPU-6].m_eUpdateType = UPDATETYPE_TANKDEFENSE;
	m_aUpdates[iCPU-4][iCPU-6].m_flValue = 1;
	m_aUpdates[iCPU-4][iCPU-6].m_flSize = 32;

	m_aUpdates[iCPU-2][iCPU-6].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-2][iCPU-6].m_eStructure = STRUCTURE_TANKLOADER;
	m_aUpdates[iCPU-2][iCPU-6].m_eUpdateType = UPDATETYPE_TANKHEALTH;
	m_aUpdates[iCPU-2][iCPU-6].m_flValue = 1;
	m_aUpdates[iCPU-2][iCPU-6].m_flSize = 32;

	m_aUpdates[iCPU-1][iCPU-5].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-1][iCPU-5].m_eStructure = STRUCTURE_TANKLOADER;
	m_aUpdates[iCPU-1][iCPU-5].m_eUpdateType = UPDATETYPE_TANKMOVEMENT;
	m_aUpdates[iCPU-1][iCPU-5].m_flValue = 1;
	m_aUpdates[iCPU-1][iCPU-5].m_flSize = 32;

	m_aUpdates[iCPU-1][iCPU-6].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-1][iCPU-6].m_eStructure = STRUCTURE_TANKLOADER;
	m_aUpdates[iCPU-1][iCPU-6].m_eUpdateType = UPDATETYPE_TANKATTACK;
	m_aUpdates[iCPU-1][iCPU-6].m_flValue = 1;
	m_aUpdates[iCPU-1][iCPU-6].m_flSize = 36;

	m_aUpdates[iCPU-4][iCPU-7].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-4][iCPU-7].m_eStructure = STRUCTURE_TANKLOADER;
	m_aUpdates[iCPU-4][iCPU-7].m_eUpdateType = UPDATETYPE_TANKHEALTH;
	m_aUpdates[iCPU-4][iCPU-7].m_flValue = 2;
	m_aUpdates[iCPU-4][iCPU-7].m_flSize = 36;

	m_aUpdates[iCPU-5][iCPU-7].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-5][iCPU-7].m_eStructure = STRUCTURE_TANKLOADER;
	m_aUpdates[iCPU-5][iCPU-7].m_eUpdateType = UPDATETYPE_TANKMOVEMENT;
	m_aUpdates[iCPU-5][iCPU-7].m_flValue = 2;
	m_aUpdates[iCPU-5][iCPU-7].m_flSize = 40;

	m_aUpdates[iCPU-5][iCPU-8].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-5][iCPU-8].m_eStructure = STRUCTURE_TANKLOADER;
	m_aUpdates[iCPU-5][iCPU-8].m_eUpdateType = UPDATETYPE_TANKATTACK;
	m_aUpdates[iCPU-5][iCPU-8].m_flValue = 2;
	m_aUpdates[iCPU-5][iCPU-8].m_flSize = 44;


	// Artillery downloads
	m_aUpdates[iCPU-1][iCPU-7].m_eUpdateClass = UPDATECLASS_STRUCTURE;
	m_aUpdates[iCPU-1][iCPU-7].m_eStructure = STRUCTURE_ARTILLERYLOADER;
	m_aUpdates[iCPU-1][iCPU-7].m_flSize = 50;

	m_aUpdates[iCPU-1][iCPU-8].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-1][iCPU-8].m_eStructure = STRUCTURE_ARTILLERYLOADER;
	m_aUpdates[iCPU-1][iCPU-8].m_eUpdateType = UPDATETYPE_TANKRANGE;
	m_aUpdates[iCPU-1][iCPU-8].m_flValue = 10;
	m_aUpdates[iCPU-1][iCPU-8].m_flSize = 36;

	m_aUpdates[iCPU-1][iCPU-9].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-1][iCPU-9].m_eStructure = STRUCTURE_ARTILLERYLOADER;
	m_aUpdates[iCPU-1][iCPU-9].m_eUpdateType = UPDATETYPE_TANKATTACK;
	m_aUpdates[iCPU-1][iCPU-9].m_flValue = 1;
	m_aUpdates[iCPU-1][iCPU-9].m_flSize = 40;

	m_aUpdates[iCPU-0][iCPU-8].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-0][iCPU-8].m_eStructure = STRUCTURE_ARTILLERYLOADER;
	m_aUpdates[iCPU-0][iCPU-8].m_eUpdateType = UPDATETYPE_TANKATTACK;
	m_aUpdates[iCPU-0][iCPU-8].m_flValue = 1;
	m_aUpdates[iCPU-0][iCPU-8].m_flSize = 40;

	m_aUpdates[iCPU-0][iCPU-9].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-0][iCPU-9].m_eStructure = STRUCTURE_ARTILLERYLOADER;
	m_aUpdates[iCPU-0][iCPU-9].m_eUpdateType = UPDATETYPE_TANKHEALTH;
	m_aUpdates[iCPU-0][iCPU-9].m_flValue = 1;
	m_aUpdates[iCPU-0][iCPU-9].m_flSize = 44;

	m_aUpdates[iCPU-2][iCPU-9].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-2][iCPU-9].m_eStructure = STRUCTURE_ARTILLERYLOADER;
	m_aUpdates[iCPU-2][iCPU-9].m_eUpdateType = UPDATETYPE_TANKHEALTH;
	m_aUpdates[iCPU-2][iCPU-9].m_flValue = 10;
	m_aUpdates[iCPU-2][iCPU-9].m_flSize = 44;

	m_aUpdates[iCPU+1][iCPU-8].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU+1][iCPU-8].m_eStructure = STRUCTURE_ARTILLERYLOADER;
	m_aUpdates[iCPU+1][iCPU-8].m_eUpdateType = UPDATETYPE_TANKMOVEMENT;
	m_aUpdates[iCPU+1][iCPU-8].m_flValue = 1;
	m_aUpdates[iCPU+1][iCPU-8].m_flSize = 44;

	m_aUpdates[iCPU+1][iCPU-9].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU+1][iCPU-9].m_eStructure = STRUCTURE_ARTILLERYLOADER;
	m_aUpdates[iCPU+1][iCPU-9].m_eUpdateType = UPDATETYPE_TANKRANGE;
	m_aUpdates[iCPU+1][iCPU-9].m_flValue = 10;
	m_aUpdates[iCPU+1][iCPU-9].m_flSize = 48;

	m_aUpdates[iCPU-2][iCPU-10].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU-2][iCPU-10].m_eStructure = STRUCTURE_ARTILLERYLOADER;
	m_aUpdates[iCPU-2][iCPU-10].m_eUpdateType = UPDATETYPE_TANKRANGE;
	m_aUpdates[iCPU-2][iCPU-10].m_flValue = 10;
	m_aUpdates[iCPU-2][iCPU-10].m_flSize = 52;

	m_aUpdates[iCPU+2][iCPU-9].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU+2][iCPU-9].m_eStructure = STRUCTURE_ARTILLERYLOADER;
	m_aUpdates[iCPU+2][iCPU-9].m_eUpdateType = UPDATETYPE_TANKATTACK;
	m_aUpdates[iCPU+2][iCPU-9].m_flValue = 1;
	m_aUpdates[iCPU+2][iCPU-9].m_flSize = 48;

	m_aUpdates[iCPU+2][iCPU-10].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU+2][iCPU-10].m_eStructure = STRUCTURE_ARTILLERYLOADER;
	m_aUpdates[iCPU+2][iCPU-10].m_eUpdateType = UPDATETYPE_TANKRANGE;
	m_aUpdates[iCPU+2][iCPU-10].m_flValue = 10;
	m_aUpdates[iCPU+2][iCPU-10].m_flSize = 56;


	// Buffer downloads
	m_aUpdates[iCPU+3][iCPU-4].m_eUpdateClass = UPDATECLASS_STRUCTURE;
	m_aUpdates[iCPU+3][iCPU-4].m_eStructure = STRUCTURE_BUFFER;
	m_aUpdates[iCPU+3][iCPU-4].m_flSize = 40;

	m_aUpdates[iCPU+3][iCPU-5].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU+3][iCPU-5].m_eStructure = STRUCTURE_BUFFER;
	m_aUpdates[iCPU+3][iCPU-5].m_eUpdateType = UPDATETYPE_SUPPORTENERGY;
	m_aUpdates[iCPU+3][iCPU-5].m_flValue = 1;
	m_aUpdates[iCPU+3][iCPU-5].m_flSize = 30;

	m_aUpdates[iCPU+3][iCPU-6].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU+3][iCPU-6].m_eStructure = STRUCTURE_BUFFER;
	m_aUpdates[iCPU+3][iCPU-6].m_eUpdateType = UPDATETYPE_SUPPORTRECHARGE;
	m_aUpdates[iCPU+3][iCPU-6].m_flValue = 1;
	m_aUpdates[iCPU+3][iCPU-6].m_flSize = 35;

	m_aUpdates[iCPU+2][iCPU-5].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU+2][iCPU-5].m_eStructure = STRUCTURE_BUFFER;
	m_aUpdates[iCPU+2][iCPU-5].m_eUpdateType = UPDATETYPE_BANDWIDTH;
	m_aUpdates[iCPU+2][iCPU-5].m_flValue = 1;
	m_aUpdates[iCPU+2][iCPU-5].m_flSize = 35;

	m_aUpdates[iCPU+4][iCPU-6].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU+4][iCPU-6].m_eStructure = STRUCTURE_BUFFER;
	m_aUpdates[iCPU+4][iCPU-6].m_eUpdateType = UPDATETYPE_BANDWIDTH;
	m_aUpdates[iCPU+4][iCPU-6].m_flValue = 1;
	m_aUpdates[iCPU+4][iCPU-6].m_flSize = 40;

	m_aUpdates[iCPU+2][iCPU-6].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU+2][iCPU-6].m_eStructure = STRUCTURE_BUFFER;
	m_aUpdates[iCPU+2][iCPU-6].m_eUpdateType = UPDATETYPE_FLEETSUPPLY;
	m_aUpdates[iCPU+2][iCPU-6].m_flValue = 1;
	m_aUpdates[iCPU+2][iCPU-6].m_flSize = 40;

	m_aUpdates[iCPU+1][iCPU-5].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU+1][iCPU-5].m_eStructure = STRUCTURE_BUFFER;
	m_aUpdates[iCPU+1][iCPU-5].m_eUpdateType = UPDATETYPE_SUPPORTRECHARGE;
	m_aUpdates[iCPU+1][iCPU-5].m_flValue = 1;
	m_aUpdates[iCPU+1][iCPU-5].m_flSize = 40;

	m_aUpdates[iCPU+1][iCPU-6].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU+1][iCPU-6].m_eStructure = STRUCTURE_BUFFER;
	m_aUpdates[iCPU+1][iCPU-6].m_eUpdateType = UPDATETYPE_SUPPORTENERGY;
	m_aUpdates[iCPU+1][iCPU-6].m_flValue = 1;
	m_aUpdates[iCPU+1][iCPU-6].m_flSize = 45;

	m_aUpdates[iCPU+4][iCPU-7].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU+4][iCPU-7].m_eStructure = STRUCTURE_BUFFER;
	m_aUpdates[iCPU+4][iCPU-7].m_eUpdateType = UPDATETYPE_FLEETSUPPLY;
	m_aUpdates[iCPU+4][iCPU-7].m_flValue = 1;
	m_aUpdates[iCPU+4][iCPU-7].m_flSize = 45;

	m_aUpdates[iCPU+5][iCPU-7].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU+5][iCPU-7].m_eStructure = STRUCTURE_BUFFER;
	m_aUpdates[iCPU+5][iCPU-7].m_eUpdateType = UPDATETYPE_SUPPORTRECHARGE;
	m_aUpdates[iCPU+5][iCPU-7].m_flValue = 2;
	m_aUpdates[iCPU+5][iCPU-7].m_flSize = 50;

	m_aUpdates[iCPU+5][iCPU-8].m_eUpdateClass = UPDATECLASS_STRUCTUREUPDATE;
	m_aUpdates[iCPU+5][iCPU-8].m_eStructure = STRUCTURE_BUFFER;
	m_aUpdates[iCPU+5][iCPU-8].m_eUpdateType = UPDATETYPE_SUPPORTENERGY;
	m_aUpdates[iCPU+5][iCPU-8].m_flValue = 2;
	m_aUpdates[iCPU+5][iCPU-8].m_flSize = 55;


	m_aUpdates[iCPU+3][iCPU-2].m_eUpdateClass = UPDATECLASS_STRUCTURE;
	m_aUpdates[iCPU+3][iCPU-2].m_eStructure = STRUCTURE_PSU;
	m_aUpdates[iCPU+3][iCPU-2].m_flSize = 45;


	m_aUpdates[iCPU+5][iCPU-6].m_eUpdateClass = UPDATECLASS_UNITSKILL;
	m_aUpdates[iCPU+5][iCPU-6].m_eStructure = UNIT_SCOUT;
	m_aUpdates[iCPU+5][iCPU-6].m_eUpdateType = UPDATETYPE_SKILL_CLOAK;
	m_aUpdates[iCPU+5][iCPU-6].m_flSize = 55;

	m_aUpdates[iCPU-3][iCPU-2].m_eUpdateClass = UPDATECLASS_UNITSKILL;
	m_aUpdates[iCPU-3][iCPU-2].m_eStructure = UNIT_INFANTRY;
	m_aUpdates[iCPU-3][iCPU-2].m_eUpdateType = UPDATETYPE_WEAPON_CHARGERAM;
	m_aUpdates[iCPU-3][iCPU-2].m_flSize = 35;

	m_aUpdates[iCPU+0][iCPU-5].m_eUpdateClass = UPDATECLASS_UNITSKILL;
	m_aUpdates[iCPU+0][iCPU-5].m_eStructure = UNIT_TANK;
	m_aUpdates[iCPU+0][iCPU-5].m_eUpdateType = UPDATETYPE_WEAPON_AOE;
	m_aUpdates[iCPU+0][iCPU-5].m_flSize = 55;

	m_aUpdates[iCPU-5][iCPU-6].m_eUpdateClass = UPDATECLASS_UNITSKILL;
	m_aUpdates[iCPU-5][iCPU-6].m_eStructure = UNIT_TANK;
	m_aUpdates[iCPU-5][iCPU-6].m_eUpdateType = UPDATETYPE_WEAPON_CLUSTER;
	m_aUpdates[iCPU-5][iCPU-6].m_flSize = 45;

	m_aUpdates[iCPU-5][iCPU-9].m_eUpdateClass = UPDATECLASS_UNITSKILL;
	m_aUpdates[iCPU-5][iCPU-9].m_eStructure = UNIT_TANK;
	m_aUpdates[iCPU-5][iCPU-9].m_eUpdateType = UPDATETYPE_WEAPON_ICBM;
	m_aUpdates[iCPU-5][iCPU-9].m_flSize = 65;

	m_aUpdates[iCPU+1][iCPU-7].m_eUpdateClass = UPDATECLASS_UNITSKILL;
	m_aUpdates[iCPU+1][iCPU-7].m_eStructure = UNIT_ARTILLERY;
	m_aUpdates[iCPU+1][iCPU-7].m_eUpdateType = UPDATETYPE_WEAPON_AOE;
	m_aUpdates[iCPU+1][iCPU-7].m_flSize = 65;

	m_aUpdates[iCPU-2][iCPU-11].m_eUpdateClass = UPDATECLASS_UNITSKILL;
	m_aUpdates[iCPU-2][iCPU-11].m_eStructure = UNIT_ARTILLERY;
	m_aUpdates[iCPU-2][iCPU-11].m_eUpdateType = UPDATETYPE_WEAPON_ICBM;
	m_aUpdates[iCPU-2][iCPU-11].m_flSize = 65;

	m_aUpdates[iCPU+2][iCPU-11].m_eUpdateClass = UPDATECLASS_UNITSKILL;
	m_aUpdates[iCPU+2][iCPU-11].m_eStructure = UNIT_ARTILLERY;
	m_aUpdates[iCPU+2][iCPU-11].m_eUpdateType = UPDATETYPE_WEAPON_DEVASTATOR;
	m_aUpdates[iCPU+2][iCPU-11].m_flSize = 80;


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

void CUpdateGrid::ClientUpdate(int iClient)
{
	CNetworkParameters p;
	p.ui1 = GetHandle();
	p.i2 = m_iLowestX;
	p.i3 = m_iHighestX;
	p.i4 = m_iLowestY;
	p.i5 = m_iHighestY;

	p.CreateExtraData(sizeof(m_aUpdates));

	memcpy(p.m_pExtraData, &m_aUpdates[0][0], sizeof(m_aUpdates));

	GameNetwork()->CallFunctionParameters(NETWORK_TOCLIENTS, "UpdatesData", &p);
}

void CUpdateGrid::UpdatesData(CNetworkParameters* p)
{
	m_iLowestX = p->i2;
	m_iHighestX = p->i3;
	m_iLowestY = p->i4;
	m_iHighestY = p->i5;

	memcpy(&m_aUpdates[0][0], p->m_pExtraData, sizeof(m_aUpdates));
}

void CUpdateGrid::FindUpdate(CUpdateItem* pItem, int& x, int& y)
{
	for (int i = 0; i < UPDATE_GRID_SIZE; i++)
	{
		for (int j = 0; j < UPDATE_GRID_SIZE; j++)
		{
			if (pItem == &m_aUpdates[i][j])
			{
				x = i;
				y = j;
				return;
			}
		}
	}

	TAssert(!"Can't find an update.");
}

tstring CUpdateItem::GetName()
{
	tstring sResult;

	if (m_eUpdateClass == UPDATECLASS_STRUCTURE)
	{
		switch (m_eStructure)
		{
		case STRUCTURE_CPU:
			return "CPU";

		case STRUCTURE_BUFFER:
			return "Macro-Buffer";

		case STRUCTURE_PSU:
			return "Power Supply";

		case STRUCTURE_INFANTRYLOADER:
			return "Resistor Factory";

		case STRUCTURE_TANKLOADER:
			return "Digitank Factory";

		case STRUCTURE_ARTILLERYLOADER:
			return "Artillery Factory";

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
			sResult = "Macro-Buffer ";
			break;

		case STRUCTURE_PSU:
			sResult = "Power Supply ";
			break;

		case STRUCTURE_INFANTRYLOADER:
			sResult = "Resistor Factory ";
			break;

		case STRUCTURE_TANKLOADER:
			sResult = "Tank Factory ";
			break;

		case STRUCTURE_ARTILLERYLOADER:
			sResult = "Artillery Factory ";
			break;

		default:
			sResult = "Structure ";
		}

		switch (m_eUpdateType)
		{
		case UPDATETYPE_PRODUCTION:
			sResult += "Power";
			break;

		case UPDATETYPE_BANDWIDTH:
			sResult += "Bandwidth";
			break;

		case UPDATETYPE_FLEETSUPPLY:
			sResult += "Fleet Supply";
			break;

		case UPDATETYPE_SUPPORTENERGY:
			sResult += "Support Energy Bonus";
			break;

		case UPDATETYPE_SUPPORTRECHARGE:
			sResult += "Support Recharge Bonus";
			break;

		case UPDATETYPE_TANKATTACK:
			sResult += "Attack Bonus";
			break;

		case UPDATETYPE_TANKDEFENSE:
			sResult += "Shield Energy Bonus";
			break;

		case UPDATETYPE_TANKMOVEMENT:
			sResult += "Movement Bonus";
			break;

		case UPDATETYPE_TANKHEALTH:
			sResult += "Health Bonus";
			break;

		case UPDATETYPE_TANKRANGE:
			sResult += "Range Bonus";
			break;

		}
	}
	else if (m_eUpdateClass == UPDATECLASS_UNITSKILL)
	{
		switch (m_eStructure)
		{
		case UNIT_SCOUT:
			sResult = "Rogue ";
			break;

		case UNIT_INFANTRY:
			sResult = "Resistor ";
			break;

		case UNIT_TANK:
		default:
			sResult = "Digitank ";
			break;

		case UNIT_ARTILLERY:
			sResult = "Artillery ";
			break;
		}

		switch (m_eUpdateType)
		{
		case UPDATETYPE_SKILL_CLOAK:
			sResult += "Cloaking Device";
			break;

		case UPDATETYPE_WEAPON_CHARGERAM:
			sResult += "Charging Ram";
			break;

		case UPDATETYPE_WEAPON_AOE:
			sResult += "Plasma Charge";
			break;

		case UPDATETYPE_WEAPON_CLUSTER:
			sResult += "Cluster Bomb";
			break;

		case UPDATETYPE_WEAPON_ICBM:
			sResult += "WAN Bomb";
			break;

		case UPDATETYPE_WEAPON_DEVASTATOR:
			sResult += "Devastator";
			break;
		}
	}
	
	return sResult;
}

tstring CUpdateItem::GetInfo()
{
	if (m_eUpdateClass == UPDATECLASS_STRUCTURE)
	{
		switch (m_eStructure)
		{
		case STRUCTURE_CPU:
			return "This program allows the player to construct a CPU. The CPU is the life and brains of your operation. It allows you to build structures and units.";

		case STRUCTURE_BUFFER:
			return "This program allows the player to construct Macro-Buffers, which expand your territory and provide support to your tanks. Downloads are also available for them to provide additional fleet supply points and bandwidth as well.";

		case STRUCTURE_PSU:
			return "This program allows the player to construct Power Supplies to mine the valuable Electronodes that provide your base with more Power. Then you can further your nefarious plans to construct units and erect structures.";

		case STRUCTURE_INFANTRYLOADER:
			return "This program allows the player to construct Resistor Factories, which provide the Resistor unit, an cheap but essential defensive unit that can be fortified to defend your base.";

		case STRUCTURE_TANKLOADER:
			return "This program allows the player to construct the Digitank Factory which produces Digitanks, the primary tool of defeating your enemies. These units are more expensive to build.";

		case STRUCTURE_ARTILLERYLOADER:
			return "This program allows the player to construct Artillery Factories. These provide the Artillery unit, which can support your tanks in their attacks by pulverizing the enemy from afar. Artillery do double damage to shields, but only half damage to structures and tank hulls.";

		default:
			return "Somehow you have found a way to construct an edifice which shouldn't exist. Please be careful not to open a wormhole while you're doing it.";
		}
	}
	else if (m_eUpdateClass == UPDATECLASS_STRUCTUREUPDATE)
	{
		switch (m_eUpdateType)
		{
		case UPDATETYPE_PRODUCTION:
			return "This update increases the amount of power that you generate per turn. Power helps you build structures and units faster.";

		case UPDATETYPE_BANDWIDTH:
			return "This update increases the amount of bandwidth available to you. Increased bandwidth helps you download updates and programs faster.";

		case UPDATETYPE_FLEETSUPPLY:
			return "This update increases your maximum fleet capacity, so that you can build more tanks.";

		case UPDATETYPE_SUPPORTENERGY:
			return "This update improves unit support energy, units supported by all buffers will receive bonuses to Attack and Shield energy.";

		case UPDATETYPE_SUPPORTRECHARGE:
			return "This update improves unit health and shield recharge, units supported by all buffers will recharge more health and shields each turn.";

		case UPDATETYPE_TANKATTACK:
			return "This gives every tank produced an automatic increase to its Attack Energy.";

		case UPDATETYPE_TANKDEFENSE:
			return "This gives every tank produced an automatic increase to its Shield Energy.";

		case UPDATETYPE_TANKMOVEMENT:
			return "This gives every tank produced an automatic increase to its Movement Energy.";

		case UPDATETYPE_TANKHEALTH:
			return "This gives every tank produced an automatic increase to its health.";

		case UPDATETYPE_TANKRANGE:
			return "This gives every tank produced an automatic increase to its maximum firing range.";
		}
	}
	else if (m_eUpdateClass == UPDATECLASS_UNITSKILL)
	{
		switch (m_eUpdateType)
		{
		case UPDATETYPE_SKILL_CLOAK:
			return "Once downloaded, every Rogue will receive a cloaking device.\n \nThe Rogue will be invisible to enemy units unless it gets too close.";

		case UPDATETYPE_WEAPON_CHARGERAM:
			return "Once downloaded, every Infantry will receive a Charging R.A.M. attack, which he can use to ram enemy units.";

		case UPDATETYPE_WEAPON_AOE:
			if (m_eStructure == UNIT_TANK)
				return "Once downloaded, every Digitank will receive a Plasma Charge weapon, which does double damage to shields.";
			else
				return "Once downloaded, every Artillery will receive a Plasma Charge weapon, which does damage over a much larger area.";

		case UPDATETYPE_WEAPON_CLUSTER:
			return "Once downloaded, every Digitank will receive a Cluster Bomb, which splits into multiple smaller fragments after the initial explosion.";

		case UPDATETYPE_WEAPON_ICBM:
			if (m_eStructure == UNIT_TANK)
				return "Once downloaded, every Digitank will receive an Wide Area Network Bomb, which splits into multiple smaller fragments in the air before impact.";
			else
				return "Once downloaded, every Artillery will receive an Wide Area Network Bomb, which splits into multiple smaller fragments in the air before impact.";

		case UPDATETYPE_WEAPON_DEVASTATOR:
			return "Once downloaded, every Artillery will receive The Devastator. The Devastator is the ultimate weapon of destruction.";
		}
	}

	return "If you can figure out what hell this thing does, please let me know.";
}

tstring CUpdateItem::GetUnits()
{
	switch (m_eUpdateType)
	{
	case UPDATETYPE_PRODUCTION:
		return "power";

	case UPDATETYPE_BANDWIDTH:
		return "mb per turn";

	case UPDATETYPE_FLEETSUPPLY:
		return "fleet supply";

	case UPDATETYPE_SUPPORTENERGY:
		return "energy";

	case UPDATETYPE_SUPPORTRECHARGE:
		return "recharge";

	case UPDATETYPE_TANKATTACK:
		return "attack energy";

	case UPDATETYPE_TANKDEFENSE:
		return "shield energy";

	case UPDATETYPE_TANKMOVEMENT:
		return "units";

	case UPDATETYPE_TANKHEALTH:
		return "hull points";

	case UPDATETYPE_TANKRANGE:
		return "units";
	}

	return "";
}
