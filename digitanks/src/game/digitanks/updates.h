#ifndef DT_UPDATES_H
#define DT_UPDATES_H

#include "dt_common.h"
#include "structures/structure.h"

class CUpdateItem
{
public:
	eastl::string16	GetName();
	eastl::string16	GetInfo();
	eastl::string16	GetUnits();

public:
	updateclass_t	m_eUpdateClass;

	unittype_t		m_eStructure;

	updatetype_t	m_eUpdateType;
	float			m_flValue;

	size_t			m_iSize;
};

#define UPDATE_GRID_SIZE 30

class CUpdateGrid : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CUpdateGrid, CBaseEntity);

public:
	void			SetupStandardUpdates();

	virtual void	ClientUpdate(int iClient);
	void			UpdatesData(class CNetworkParameters* p);

public:
	CUpdateItem		m_aUpdates[UPDATE_GRID_SIZE][UPDATE_GRID_SIZE];

	int				m_iLowestX;
	int				m_iHighestX;
	int				m_iLowestY;
	int				m_iHighestY;
};

#endif
