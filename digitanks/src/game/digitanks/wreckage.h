#ifndef DT_WRECKAGE_H
#define DT_WRECKAGE_H

#include "dt_common.h"
#include "digitanksentity.h"
#include <renderer/particles.h>

class CWreckage : public CDigitanksEntity
{
	REGISTER_ENTITY_CLASS(CWreckage, CDigitanksEntity);

public:
	virtual void					Precache();
	virtual void					Spawn();
	virtual void					Think();

	virtual void					Touching(CBaseEntity* pOther);

	virtual void					ModifyContext(class CRenderingContext* pContext, bool bTransparent);
	virtual void					OnRender(class CRenderingContext* pContext, bool bTransparent);

	void							FellIntoHole() { m_bFallingIntoHole = true; };

	void							SetTurretModel(size_t iTurret) { m_iTurretModel = iTurret; };
	void							SetColorSwap(Color clrSwap);

	void							SetOldTeam(CDigitanksTeam* pOldTeam) { m_hOldTeam = pOldTeam; };
	CDigitanksTeam*					GetOldTeam() { return m_hOldTeam; };

	void							SetScale(float flScale) { m_flScale = flScale; }

protected:
	CNetworkedVariable<bool>		m_bFallingIntoHole;

	size_t							m_iTurretModel;

	CNetworkedVariable<float>		m_flScale;

	CNetworkedVector				m_vecColorSwap;
	Color							m_clrSwap;

	EAngle							m_angList;

	CParticleSystemInstanceHandle	m_hBurnParticles;

	CNetworkedVariable<bool>		m_bCrashed;

	CNetworkedHandle<CDigitanksTeam> m_hOldTeam;
};

class CDebris : public CDigitanksEntity
{
	REGISTER_ENTITY_CLASS(CDebris, CDigitanksEntity);

public:
	virtual void					Precache();
	virtual void					Spawn();
	virtual void					Think();

	virtual void					Touching(CBaseEntity* pOther);

protected:
	CParticleSystemInstanceHandle	m_hBurnParticles;
};

#endif
