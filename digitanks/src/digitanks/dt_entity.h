#pragma once

#include <tengine/game/entities/baseentitydata.h>

class CDTEntityData : public CBaseEntityData
{
public:
	CDTEntityData()
	{
		m_bDTEntity = false;
	}

public:
	bool m_bDTEntity;
};

