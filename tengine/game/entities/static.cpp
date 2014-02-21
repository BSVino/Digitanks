#include "static.h"

#include <physics/physics.h>
#include <textures/materiallibrary.h>

REGISTER_ENTITY(CStatic);

NETVAR_TABLE_BEGIN(CStatic);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN_EDITOR(CStatic);
	SAVEDATA_EDITOR_VARIABLE("Model");
	SAVEDATA_EDITOR_VARIABLE("DisableBackCulling");
	SAVEDATA_EDITOR_VARIABLE("Scale");
	SAVEDATA_OVERRIDE_DEFAULT(CSaveData::DATA_COPYTYPE, bool, m_bVisible, "Visible", true);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CStatic);
INPUTS_TABLE_END();

void CStatic::OnSetModel()
{
	BaseClass::OnSetModel();

	// In case the model has changed.
	if (IsInPhysics())
		RemoveFromPhysics();

	if (GetModelID() == ~0 && !GetMaterialModel().IsValid())
		return;

	AddToPhysics(CT_STATIC_MESH);
}

const AABB CStatic::GetPhysBoundingBox() const
{
	if (GetMaterialModel().IsValid() && GetMaterialModel()->m_ahTextures.size())
	{
		CTextureHandle hBaseTexture = GetMaterialModel()->m_ahTextures[0];

		float flBoxUpMax = 0.5f * ((float)hBaseTexture->m_iHeight/GetMaterialModel()->m_iTexelsPerMeter);
		float flBoxRightMax = 0.5f * ((float)hBaseTexture->m_iWidth/GetMaterialModel()->m_iTexelsPerMeter);

		AABB aabbPhysBox;
		aabbPhysBox.m_vecMaxs.x = 0.5f;
		aabbPhysBox.m_vecMins.x = -0.5f;
		aabbPhysBox.m_vecMaxs.y = flBoxUpMax;
		aabbPhysBox.m_vecMins.y = -flBoxUpMax;
		aabbPhysBox.m_vecMaxs.z = flBoxRightMax;
		aabbPhysBox.m_vecMins.z = -flBoxRightMax;

		return aabbPhysBox;
	}

	return BaseClass::GetPhysBoundingBox();
}
