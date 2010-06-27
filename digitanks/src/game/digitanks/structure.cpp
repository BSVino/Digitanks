#include "structure.h"

#include <renderer/renderer.h>

REGISTER_ENTITY(CStructure);

CStructure::CStructure()
{
	m_bConstructing = false;
	m_iTurnsToConstruct = 0;
}

void CStructure::BeginConstruction(size_t iTurns)
{
	m_iTurnsToConstruct = iTurns;
	m_bConstructing = true;
}

void CStructure::StartTurn()
{
	if (IsConstructing())
	{
		if (--m_iTurnsToConstruct == 0)
			m_bConstructing = false;
	}
}

void CStructure::ModifyContext(class CRenderingContext* pContext)
{
	if (IsConstructing())
	{
		pContext->SetAlpha(0.5f);
		pContext->SetBlend(BLEND_ADDITIVE);
	}
}
