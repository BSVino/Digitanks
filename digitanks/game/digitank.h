#ifndef DT_DIGITANK_H
#define DT_DIGITANK_H

#include "baseentity.h"

class CDigitank : public CBaseEntity
{
public:
					CDigitank();

public:
	float			GetTotalPower() { return m_flTotalPower; };
	float			GetAttackPower();
	float			GetDefensePower();
	float			GetMovementPower();

	void			PreviewMove(Vector vecPreviewMove) { m_vecPreviewMove = vecPreviewMove; };

	void			SetDesiredMove();
	bool			HasDesiredMove() { return m_bDesiredMove; };
	Vector			GetDesiredMove() { return m_vecDesiredMove; };

	void			Move();

protected:
	float			m_flTotalPower;
	float			m_flAttackPower;
	float			m_flDefensePower;
	float			m_flMovementPower;

	Vector			m_vecPreviewMove;

	bool			m_bDesiredMove;
	Vector			m_vecDesiredMove;
};

#endif