#ifndef DT_INTRO_RENDERER_H
#define DT_INTRO_RENDERER_H

#include <renderer/renderer.h>
#include <common.h>

class CIntroRenderer : public CRenderer
{
	DECLARE_CLASS(CIntroRenderer, CRenderer);

public:
					CIntroRenderer();

public:
	virtual void	StartRendering();
	virtual void	FinishRendering();
};

#endif
