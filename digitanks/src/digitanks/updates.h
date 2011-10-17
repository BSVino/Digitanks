#ifndef DT_UPDATES_H
#define DT_UPDATES_H

#include "dt_common.h"
#include "structures/structure.h"

class CUpdateItem
{
public:
	tstring	GetName();
	tstring	GetInfo();
	tstring	GetUnits();

public:
	updateclass_t	m_eUpdateClass;

	unittype_t		m_eStructure;

	updatetype_t	m_eUpdateType;
	float			m_flValue;

	float			m_flSize;
};

#define UPDATE_GRID_SIZE 30

class CUpdateGrid : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CUpdateGrid, CBaseEntity);

public:
	void			SetupStandardUpdates();

	virtual void	ClientUpdate(int iClient);
	void			UpdatesData(class CNetworkParameters* p);

	void			FindUpdate(CUpdateItem* pItem, int& x, int& y);

public:
	CUpdateItem		m_aUpdates[UPDATE_GRID_SIZE][UPDATE_GRID_SIZE];

	int				m_iLowestX;
	int				m_iHighestX;
	int				m_iLowestY;
	int				m_iHighestY;
};

#endif
