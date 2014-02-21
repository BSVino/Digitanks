#ifndef TENGINE_GAME_RENDERINGCONTEXT_H
#define TENGINE_GAME_RENDERINGCONTEXT_H

#include <tstring.h>
#include <tengine_config.h>
#include <plane.h>
#include <matrix.h>
#include <color.h>

#include <renderer/renderingcontext.h>
#include <textures/texturehandle.h>

class CGameRenderingContext : public CRenderingContext
{
	DECLARE_CLASS(CGameRenderingContext, CRenderingContext);

public:
							CGameRenderingContext(class CGameRenderer* pRenderer, bool bInherit = false);	// Make bInherit true if you want to preserve and not clobber GL settings set previously

public:
	void					RenderModel(size_t iModel, const class CBaseEntity* pEntity = nullptr);
	void					RenderModel(class CModel* pModel, size_t iMaterial);

	void					RenderMaterialModel(const CMaterialHandle& hMaterial, const class CBaseEntity* pEntity = nullptr);

	void					RenderBillboard(const tstring& sTexture, float flRadius);

public:
	class CGameRenderer*	m_pRenderer;
};

#endif
