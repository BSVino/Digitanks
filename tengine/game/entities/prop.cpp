#include "prop.h"

#include <renderer/game_renderingcontext.h>

REGISTER_ENTITY(CProp);

NETVAR_TABLE_BEGIN(CProp);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN_EDITOR(CProp);
	SAVEDATA_EDITOR_VARIABLE("Visible");
	SAVEDATA_EDITOR_VARIABLE("RenderInverted");
	SAVEDATA_EDITOR_VARIABLE("DisableBackCulling");
	SAVEDATA_EDITOR_VARIABLE("Model");
	SAVEDATA_EDITOR_VARIABLE("Scale");
	SAVEDATA_OVERRIDE_DEFAULT(CSaveData::DATA_COPYTYPE, bool, m_bVisible, "Visible", true);
	SAVEDATA_DEFINE_HANDLE_DEFAULT(CSaveData::DATA_COPYTYPE, float, m_flThickness, "Thickness", 0);
	SAVEDATA_EDITOR_VARIABLE("Thickness");
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CProp);
INPUTS_TABLE_END();

void CProp::OnRender(class CGameRenderingContext* c) const
{
	BaseClass::OnRender(c);

	if (m_flThickness > 0 && ShouldRenderModel() && m_hMaterialModel.IsValid())
	{
		c->SetWinding(false);
		c->Translate(GetGlobalTransform().GetForwardVector() * m_flThickness);
		c->RenderMaterialModel(m_hMaterialModel, this);
	}
}
