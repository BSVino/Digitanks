#ifndef DT_DIGITANKSENTITY_H
#define DT_DIGITANKSENTITY_H

#include <baseentity.h>

class CDigitanksEntity : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CDigitanksEntity, CBaseEntity);

public:
	virtual void					PreStartTurn() {};
	virtual void					StartTurn() {};
	virtual void					PostStartTurn() {};

	virtual void					RenderVisibleArea();
	virtual float					GetVisibility() const;

	virtual void					ModifyContext(class CRenderingContext* pContext);

	virtual float					VisibleRange() const { return 0; };
};

#endif