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

	void			RenderBackdrop();

protected:
	size_t			m_iBackdrop;
	float			m_flLayer1Speed;
	float			m_flLayer1Alpha;
	float			m_flLayer2Speed;
	float			m_flLayer2Alpha;
	float			m_flLayer3Speed;
	float			m_flLayer3Alpha;
	float			m_flLayer4Speed;
	float			m_flLayer4Alpha;
	float			m_flLayer5Speed;
	float			m_flLayer5Alpha;
};

#endif
