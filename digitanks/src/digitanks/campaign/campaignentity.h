#ifndef DT_CAMPAIGNENTITY_H
#define DT_CAMPAIGNENTITY_H

#include <game/entities/baseentity.h>

// A file of the user's that the user has to rescue
class CCampaignEntity : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CCampaignEntity, CBaseEntity);

public:
	virtual void		Spawn();

	virtual bool		OnUnserialize(std::istream& i);

protected:
	size_t				m_iLevel;
};

#endif
