#include "digitanksentity.h"

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <renderer/renderer.h>

#include "digitanksgame.h"

REGISTER_ENTITY(CDigitanksEntity);

CDigitanksTeam* CDigitanksEntity::GetDigitanksTeam()
{
	return dynamic_cast<CDigitanksTeam*>(BaseClass::GetTeam());
}

void CDigitanksEntity::RenderVisibleArea()
{
	if (VisibleRange() == 0)
		return;

	if ((CTeam*)DigitanksGame()->GetCurrentTeam() != GetTeam())
		return;

	CRenderingContext c(Game()->GetRenderer());
	c.Translate(GetOrigin());
	glutSolidSphere(VisibleRange(), 20, 10);
}

float CDigitanksEntity::GetVisibility() const
{
	if (!DigitanksGame()->ShouldRenderFogOfWar())
		return 1;

	CDigitanksTeam* pTeam = DigitanksGame()->GetCurrentTeam();

	return pTeam->GetEntityVisibility(GetHandle());
}

void CDigitanksEntity::ModifyContext(CRenderingContext* pContext)
{
	BaseClass::ModifyContext(pContext);

	CDigitanksTeam* pTeam = DigitanksGame()->GetCurrentTeam();

	float flVisibility = GetVisibility();
	if (flVisibility < 1)
	{
		pContext->SetAlpha(flVisibility);
		pContext->SetBlend(BLEND_ALPHA);
	}
}
