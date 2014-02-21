#pragma once

#include <common.h>

#include "camera.h"

class CCharacter;

class CCharacterCamera : public CCamera
{
	REGISTER_ENTITY_CLASS(CCharacterCamera, CCamera);

public:
				CCharacterCamera();

public:
	virtual void			CameraThink();

	virtual TVector			GetThirdPersonCameraPosition();
	virtual TVector			GetThirdPersonCameraDirection();

	void					SetThirdPerson(bool bOn) { m_bThirdPerson = bOn; };
	bool					GetThirdPerson() { return m_bThirdPerson; };

	virtual float           GetCameraNear() const { return 0.05f; }

	void                    SetCharacter(CCharacter* pCharacter);
	CCharacter*             GetCharacter() const;

private:
	bool						m_bThirdPerson;
	CEntityHandle<CCharacter>	m_hCharacter;

	float					m_flBack;
	float					m_flUp;
	float					m_flSide;
};
