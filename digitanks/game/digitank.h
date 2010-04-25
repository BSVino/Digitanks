#ifndef DT_DIGITANK_H
#define DT_DIGITANK_H

#include "baseentity.h"

class CDigitank : public CBaseEntity
{
public:
					CDigitank();

public:
	float			GetTotalPower() { return m_flTotalPower; };
	float			GetAttackPower() { return m_flAttackPower/m_flTotalPower; };
	float			GetDefensePower() { return m_flDefensePower/m_flTotalPower; };
	float			GetMovementPower() { return m_flMovementPower/m_flTotalPower; };

protected:
	float			m_flTotalPower;
	float			m_flAttackPower;
	float			m_flDefensePower;
	float			m_flMovementPower;
};

#endif