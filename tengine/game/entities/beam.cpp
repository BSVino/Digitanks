#include "beam.h"

#include <renderer/renderingcontext.h>
#include <renderer/game_renderer.h>

REGISTER_ENTITY(CBeam);

NETVAR_TABLE_BEGIN(CBeam);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CBeam);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, Vector, m_vecStart);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, Vector, m_vecEnd);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, Color, m_clrBeam);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CBeam);
INPUTS_TABLE_END();

const TVector CBeam::GetGlobalCenter() const
{
	return (m_vecStart + m_vecEnd)/2;
}

const TFloat CBeam::GetBoundingRadius() const
{
	return (m_vecStart - m_vecEnd).Length()/2;
}

void CBeam::PostRender() const
{
	if (!GameServer()->GetRenderer()->IsRenderingTransparent())
		return;

	CRenderingContext c(GameServer()->GetRenderer(), true);

	c.UseProgram("model");
	c.SetUniform("bDiffuse", false);
	c.BeginRenderDebugLines();
	c.Color(m_clrBeam);
	c.Vertex(m_vecStart);
	c.Vertex(m_vecEnd);
	c.EndRender();
}
