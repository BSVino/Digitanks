#ifndef DT_USERFILE_H
#define DT_USERFILE_H

#include "../digitanksentity.h"

// A file of the user's that the user has to rescue
class CUserFile : public CDigitanksEntity
{
	REGISTER_ENTITY_CLASS(CUserFile, CDigitanksEntity);

public:
	virtual void					Spawn();

	virtual EAngle					GetRenderAngles() const;

	virtual bool					IsTouching(CBaseEntity* pOther, Vector& vecPoint) const;
	void							Pickup(class CDigitank* pTank);
	DECLARE_ENTITY_OUTPUT(OnPickup);

	virtual eastl::string16			GetEntityName() { return L"Your File"; };

protected:
};

#endif
