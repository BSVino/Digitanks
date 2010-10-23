#ifndef DT_PROPS_H
#define DT_PROPS_H

#include "digitanksentity.h"
#include "color.h"

class CStaticProp : public CDigitanksEntity
{
	REGISTER_ENTITY_CLASS(CStaticProp, CDigitanksEntity);

public:
	void			Precache();
	virtual void	Spawn();

	virtual bool	UsesRaytracedCollision() { return true; }

	virtual void	ModifyContext(CRenderingContext* pContext);

	void			SetAdditive(bool bAdditive) { m_bAdditive = bAdditive; };
	void			SetDepthMask(bool bDepthMask) { m_bDepthMask = bDepthMask; };
	void			SetBackCulling(bool bBackCulling) { m_bBackCulling = bBackCulling; };

	void			SetColorSwap(Color clrSwap) { m_bSwap = true; m_clrSwap = clrSwap; }

protected:
	CNetworkedVariable<bool>	m_bAdditive;
	CNetworkedVariable<bool>	m_bDepthMask;
	CNetworkedVariable<bool>	m_bBackCulling;

	CNetworkedVariable<bool>	m_bSwap;
	CNetworkedVariable<Color>	m_clrSwap;
};

#endif
