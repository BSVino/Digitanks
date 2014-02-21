#ifndef DT_PROPS_H
#define DT_PROPS_H

#include <color.h>

#include <digitanksentity.h>

class CStaticProp : public CDigitanksEntity
{
	REGISTER_ENTITY_CLASS(CStaticProp, CDigitanksEntity);

public:
	void			Precache();
	virtual void	Spawn();

	virtual bool	GetsConcealmentBonus() const { return false; };
	virtual bool	UsesRaytracedCollision() { return m_bUseRaytracedCollision; }
	void			SetUsesRaytracedCollision(bool bUse) { m_bUseRaytracedCollision = bUse; }

	virtual void	ModifyContext(CRenderingContext* pContext) const;
	virtual bool	ShouldRenderModel() const { return false; };
	virtual void	OnRender(class CGameRenderingContext* pContext) const;

	void			SetAdditive(bool bAdditive) { m_bAdditive = bAdditive; };
	void			SetDepthMask(bool bDepthMask) { m_bDepthMask = bDepthMask; };
	void			SetBackCulling(bool bBackCulling) { m_bBackCulling = bBackCulling; };
	virtual bool	IsRammable() const { return false; }

	void			SetColorSwap(Color clrSwap) { m_bSwap = true; m_clrSwap = clrSwap; }

protected:
	CNetworkedVariable<bool>	m_bAdditive;
	CNetworkedVariable<bool>	m_bDepthMask;
	CNetworkedVariable<bool>	m_bBackCulling;

	CNetworkedVariable<bool>	m_bSwap;
	CNetworkedVariable<Color>	m_clrSwap;

	CNetworkedVariable<bool>	m_bUseRaytracedCollision;
};

#endif
