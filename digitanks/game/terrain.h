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

	float					GetHeight(float x, float y);
	float					GetMapSize();

	bool					Collide(const Ray& rayTrace, Vector& vecPoint);
	bool					Collide(const Vector& s1, const Vector& s2, Vector& vecPoint);

protected:
	float					m_aflHeights[TERRAIN_SIZE][TERRAIN_SIZE];

	float					m_flHighest;
	float					m_flLowest;

	size_t					m_iCallList;

	raytrace::CRaytracer*	m_pTracer;
};

#endif
