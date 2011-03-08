#ifndef DT_WRECKAGE_H
#define DT_WRECKAGE_H

#include "dt_common.h"
#include "digitanksentity.h"
#include <renderer/particles.h>

class CWreckage : public CDigitanksEntity
{
	REGISTER_ENTITY_CLASS(CWreckage, CDigitanksEntity);

public:
	virtual void	Precache();
	virtual void	Spawn();
	virtual void	Think();

	virtual void	Touching(CBaseEntity* pOther);

	virtual void	ModifyContext(class CRenderingContext* pContext, bool bTransparent);
	virtual void	OnRender(class CRenderingContext* pContext, bool bTransparent);

	void			FellIntoHole() { m_bFallingIntoHole = true; };

	void			SetTurretModel(size_t iTurret) { m_iTurretModel = iTurret; };
	void			SetColorSwap(Color clrSwap);

	void			SetOldTeam(CDigitanksTeam* pOldTeam) { m_hOldTeam = pOldTeam; };
	CDigitanksTeam*	GetOldTeam() { return m_hOldTeam; };

protected:
	bool			m_bFallingIntoHole;

	size_t			m_iTurretModel;

	Vector			m_vecColorSwap;
	Color			m_clrSwap;

	EAngle			m_angList;

	CParticleSystemInstanceHandle m_hBurnParticles;

	bool			m_bCrashed;

	CEntityHandle<CDigitanksTeam>	m_hOldTeam;
};

#endif
