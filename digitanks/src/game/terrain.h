#ifndef DT_TERRAIN_H
#define DT_TERRAIN_H

#include <raytracer/raytracer.h>
#include "baseentity.h"

#define TERRAIN_SIZE 200

class CTerrain : public CBaseEntity
{
public:
							CTerrain();
							~CTerrain();

public:
	void					GenerateCallLists();

	virtual void			OnRender();

	float					GetRealHeight(int x, int y);
	float					GetHeight(float x, float y);
	void					SetPointHeight(Vector& vecPoint);
	float					GetMapSize();
	float					ArrayToWorldSpace(int i);
	int						WorldToArraySpace(float f);

	bool					Collide(const Ray& rayTrace, Vector& vecPoint);
	bool					Collide(const Vector& s1, const Vector& s2, Vector& vecPoint);

protected:
	float					m_aflHeights[TERRAIN_SIZE][TERRAIN_SIZE];

	float					m_flHighest;
	float					m_flLowest;

	size_t					m_iCallList;

	raytrace::CRaytracer*	m_pTracer;

	Vector					m_avecTerrainColors[4];
};

#endif
