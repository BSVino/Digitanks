#include "props.h"

#include <renderer/renderer.h>

void CStaticProp::Precache()
{
	PrecacheModel(L"models/props/prop01.obj", true);
}

void CStaticProp::ModifyContext(CRenderingContext* pContext)
{
	if (m_bAdditive)
		pContext->SetBlend(BLEND_ADDITIVE);

	pContext->SetDepthMask(m_bDepthMask);
	pContext->SetBackCulling(m_bBackCulling);

	BaseClass::ModifyContext(pContext);
}
