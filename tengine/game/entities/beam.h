#pragma once

#include <tengine/game/entities/baseentity.h>

class CBeam : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CBeam, CBaseEntity);

public:
	// There are used in frustum culling so we override them with our own stuff
	virtual const TVector	GetGlobalCenter() const;
	virtual const TFloat	GetBoundingRadius() const;

	virtual bool			ShouldRender() const { return true; };
	virtual void			PostRender() const;

	Vector					GetStart() const { return m_vecStart; }
	void					SetStart(const Vector& vecStart) { m_vecStart = vecStart; }
	Vector					GetEnd() const { return m_vecEnd; }
	void					SetEnd(const Vector& vecEnd) { m_vecEnd = vecEnd; }
	Color					GetColor() const { return m_clrBeam; }
	void					SetColor(const Color& clrBeam) { m_clrBeam = clrBeam; }

protected:
	Vector					m_vecStart;
	Vector					m_vecEnd;
	Color					m_clrBeam;
};
