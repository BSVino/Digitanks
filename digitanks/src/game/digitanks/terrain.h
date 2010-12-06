#ifndef DT_TERRAIN_H
#define DT_TERRAIN_H

#include "baseentity.h"
#include "color.h"

#define TERRAIN_SIZE 200
#define TERRAIN_CHUNKS 10
#define TERRAIN_CHUNK_SIZE (TERRAIN_SIZE/TERRAIN_CHUNKS)

#define TERRAIN_CHUNK_TEXTURE_SIZE 256
#define TERRAIN_CHUNK_TEXELS_PER (TERRAIN_CHUNK_TEXTURE_SIZE/TERRAIN_CHUNK_SIZE)

class CTerrainChunk
{
public:
	friend class CTerrain;

public:
									CTerrainChunk();
	virtual							~CTerrainChunk();

protected:
	float							m_aflHeights[TERRAIN_CHUNK_SIZE][TERRAIN_CHUNK_SIZE];
	class raytrace::CRaytracer*		m_pTracer;

	// A bit field
	unsigned char					m_aiSpecialData[TERRAIN_CHUNK_SIZE][TERRAIN_CHUNK_SIZE];

	Color							m_aclrTexture[TERRAIN_CHUNK_TEXTURE_SIZE][TERRAIN_CHUNK_TEXTURE_SIZE];
	size_t							m_iChunkTexture;

	size_t							m_iCallList;

	bool							m_bNeedsRegenerate;
};

class CTerrain : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CTerrain, CBaseEntity);

public:
							CTerrain();
	virtual					~CTerrain();

public:
	virtual void			Spawn();

	virtual float			GetBoundingRadius() const { return sqrt(GetMapSize()*GetMapSize() + GetMapSize()*GetMapSize()); };

	void					GenerateTerrain(float flHeight = 50);
	void					GenerateCollision();
	void					GenerateTerrainCallLists();
	void					GenerateTerrainCallList(int x, int y);
	void					GenerateCallLists();

	virtual bool			ShouldRender() const { return true; };
	virtual void			OnRender();
	void					RenderWithShaders();
	void					RenderWithoutShaders();

	void					GetChunk(float x, float y, int& i, int& j);
	CTerrainChunk*			GetChunk(int x, int y);
	CTerrainChunk*			GetChunk(float x, float y);

	float					GetRealHeight(int x, int y);
	void					SetRealHeight(int x, int y, float h);
	float					GetHeight(float x, float y);
	Vector					SetPointHeight(Vector& vecPoint);
	float					GetMapSize() const;
	float					ArrayToWorldSpace(int i);
	int						WorldToArraySpace(float f);
	int						ArrayToChunkSpace(int i, int& iIndex);
	int						ChunkToArraySpace(int iChunk, int i);
	float					ChunkToWorldSpace(int iChunk, int i);
	int						WorldToChunkSpace(float f, int& iIndex);
	bool					IsPointOnMap(Vector vecPoint);
	bool					IsPointOverLava(Vector vecPoint);
	bool					IsPointOverHole(Vector vecPoint);

	typedef enum
	{
		TB_LAVA = 0,
		TB_HOLE = 1,
		// uses m_aiSpecialData which is unsigned char so max 8 of these.
	} terrainbit_t;
	void					SetBit(int x, int y, terrainbit_t b, bool v);
	bool					GetBit(int x, int y, terrainbit_t b);

	Vector					GetNormalAtPoint(Vector vecPoint);

	virtual bool			Collide(const Vector& v1, const Vector& v2, Vector& vecPoint);
	void					TakeDamage(CBaseEntity* pAttacker, CBaseEntity* pInflictor, damagetype_t eDamageType, float flDamage, bool bDirectHit);

	Color					GetPrimaryTerrainColor();

	void					UpdateTerrainData();
	void					TerrainData(class CNetworkParameters* p);
	void					ResyncClientTerrainData(int iClient);

	virtual void			OnSerialize(std::ostream& o);
	virtual bool			OnUnserialize(std::istream& i);

	virtual void			ClientEnterGame();

	float					LavaHeight() { return 0.2f; }

protected:
	bool					m_bHeightsInitialized;
	float					m_flHighest;
	float					m_flLowest;

	Vector					m_vecTerrainColor;
	Vector					m_avecQuadMods[4];

	CTerrainChunk			m_aTerrainChunks[TERRAIN_CHUNKS][TERRAIN_CHUNKS];
};

#endif
