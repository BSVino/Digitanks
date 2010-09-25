#ifndef DT_PROPS_H
#define DT_PROPS_H

#include "digitanksentity.h"

class CStaticProp : public CDigitanksEntity
{
	REGISTER_ENTITY_CLASS(CStaticProp, CDigitanksEntity);

public:
	void			Precache();

	virtual void	ModifyContext(CRenderingContext* pContext);

	void			SetAdditive(bool bAdditive) { m_bAdditive = bAdditive; };
	void			SetDepthMask(bool bDepthMask) { m_bDepthMask = bDepthMask; };
	void			SetBackCulling(bool bBackCulling) { m_bBackCulling = bBackCulling; };

protected:
	bool			m_bAdditive;
	bool			m_bDepthMask;
	bool			m_bBackCulling;
};

#endif
