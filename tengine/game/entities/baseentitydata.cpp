#include "baseentitydata.h"

#include "baseentity.h"

void CBaseEntityData::SetEntity(CBaseEntity* pEntity)
{
	m_pEntity = pEntity;
}

void CBaseEntityData::Think()
{
	return m_pEntity->Think();
}

const Matrix4x4 CBaseEntityData::GetRenderTransform() const
{
	return m_pEntity->GetRenderTransform();
}

const Vector CBaseEntityData::GetRenderOrigin() const
{
	return m_pEntity->GetRenderOrigin();
}
