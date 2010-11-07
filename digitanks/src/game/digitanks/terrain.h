#ifndef DT_TERRAIN_H
#define DT_TERRAIN_H

#include "baseentity.h"
#include "color.h"

#define TERRAIN_SIZE 200
#define TERRAIN_GEN_SECTORS 10
#define TERRAIN_SECTOR_SIZE (TERRAIN_SIZE/TERRAIN_GEN_SECTORS)

class CTerrain : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CTerrain, CBaseEntity);

public:
							CTerrain();
							~CTerrain();

public:
	virtual void			Spawn();

	virtual float			GetBoundingRadius() const { return sqrt(GetMapSize()*GetMapSize() + GetMapSize()*GetMapSize()); };

	void					GenerateTerrain();
	void					GenerateCollision();
	void					GenerateTerrainCallLists();
	void					GenerateCallLists();

	virtual bool			ShouldRender() const { return true; };
	virtual void			OnRender();

	float					GetRealHeight(int x, int y);
	float					GetHeight(float x, float y);
	Vector					SetPointHeight(Vector& vecPoint);
	float					GetMapSize() const;
	float					ArrayToWorldSpace(int i);
	int						WorldToArraySpace(float f);

	void					TakeDamage(CBaseEntity* pAttacker, CBaseEntity* pInflictor, float flDamage, bool bDirectHit);

	Color					GetPrimaryTerrainColor();

	void					UpdateTerrainData();
	void					TerrainData(class CNetworkParameters* p);
	void					ResyncClientTerrainData(int iClient);

	virtual void			ClientEnterGame();

protected:
	float					m_aflHeights[TERRAIN_SIZE][TERRAIN_SIZE];

	bool					m_bHeightsInitialized;
	float					m_flHighest;
	float					m_flLowest;

	size_t					m_iCallList;

	Vector					m_avecTerrainColors[4];

	eastl::vector<Vector>	m_avecCraterMarks;

	bool					m_abTerrainNeedsRegenerate[TERRAIN_GEN_SECTORS][TERRAIN_GEN_SECTORS];
};

#endif
