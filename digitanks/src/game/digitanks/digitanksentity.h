#ifndef DT_DIGITANKSENTITY_H
#define DT_DIGITANKSENTITY_H

#include <baseentity.h>

class CDigitanksEntity : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CDigitanksEntity, CBaseEntity);

public:
	virtual void					Spawn();

	virtual void					Think();

	virtual void					StartTurn() {};

	class CDigitanksTeam*			GetDigitanksTeam() const;

	virtual void					RenderVisibleArea();
	virtual float					GetVisibility(CDigitanksTeam* pTeam) const;
	virtual float					GetVisibility() const;

	virtual void					ModifyContext(class CRenderingContext* pContext);
	virtual void					OnRender();

	virtual void					UpdateInfo(std::string& sInfo) {};
	virtual const char*				GetName() { return "Entity"; };

	virtual float					VisibleRange() const { return 0; };
	virtual float					TotalHealth() const { return 10; };
};

#endif