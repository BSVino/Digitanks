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

	virtual bool					ModifyShader(class CRenderingContext* pContext) const;
	virtual void					OnRender(class CGameRenderingContext* pContext) const;

	void							FellIntoHole() { m_bFallingIntoHole = true; };

	void							SetTurretModel(size_t iTurret) { m_iTurretModel = iTurret; };
	void							SetColorSwap(Color clrSwap);

	void							SetOldPlayer(CDigitanksPlayer* hOldPlayer) { m_hOldPlayer = hOldPlayer; };
	CDigitanksPlayer*				GetOldPlayer() { return m_hOldPlayer; };

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

	CNetworkedHandle<CDigitanksPlayer> m_hOldPlayer;
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
