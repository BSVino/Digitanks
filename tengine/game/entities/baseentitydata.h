#ifndef TINKER_BASE_ENTITY_DATA_H
#define TINKER_BASE_ENTITY_DATA_H

#include <vector.h>
#include <matrix.h>

class CBaseEntityData
{
public:
	void						SetEntity(class CBaseEntity* pEntity);

	virtual void				Think();

	virtual const Matrix4x4     GetRenderTransform() const;
	virtual const Vector        GetRenderOrigin() const;

public:
	class CBaseEntity*			m_pEntity;
};

#endif
