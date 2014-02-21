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
	virtual void					ModifyContext(class CRenderingContext* pContext) const;
	virtual void					OnRender(class CGameRenderingContext* pContext) const;
	virtual bool					ShouldRender() const { return true; }

	virtual void					Think();

	virtual bool					IsTouching(CBaseEntity* pOther, Vector& vecPoint) const;
	void							Pickup(class CDigitank* pTank);
	DECLARE_ENTITY_OUTPUT(OnPickup);

	virtual tstring					GetEntityName() const { return "Your File"; };

	void							SetFile(const tstring& sFile);

protected:
	size_t							m_iImage;
	tstring							m_sFilename;

	float							m_flPickupTime;
};

#endif
