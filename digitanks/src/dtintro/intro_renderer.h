#ifndef DT_INTRO_RENDERER_H
#define DT_INTRO_RENDERER_H

#include <renderer/game_renderer.h>
#include <common.h>

class CIntroRenderer : public CGameRenderer
{
	DECLARE_CLASS(CIntroRenderer, CGameRenderer);

public:
					CIntroRenderer();

public:
	virtual float	BloomBrightnessCutoff() const { return 0.6f; }
	virtual void	Initialize();
	virtual void	StartRendering(class CRenderingContext* pContext);

	void			RenderBackdrop();

	void			ZoomIntoHole();

protected:
	CMaterialHandle	m_hBackdrop;
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

	double			m_flZoomIntoHole;
};

#endif
