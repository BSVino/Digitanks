#ifndef DT_DIGITANKS_RENDERER_H
#define DT_DIGITANKS_RENDERER_H

#include <renderer/renderer.h>
#include <common.h>

class CDigitanksRenderer : public CRenderer
{
	DECLARE_CLASS(CDigitanksRenderer, CRenderer);

public:
					CDigitanksRenderer();

public:
	virtual void	SetupFrame();
	virtual void	FinishRendering();
	virtual void	RenderFogOfWar();
	virtual void	RenderOffscreenBuffers();
	virtual void	RenderFullscreenBuffers();

	void			RenderBloomPass(CFrameBuffer* apSources, CFrameBuffer* apTargets, bool bHorizontal);

	const CFrameBuffer*	GetExplosionBuffer() { return &m_oExplosionBuffer; }
	const CFrameBuffer*	GetVisibility1Buffer() { return &m_oVisibility1Buffer; }
	const CFrameBuffer*	GetVisibility2Buffer() { return &m_oVisibility2Buffer; }
	const CFrameBuffer*	GetVisibilityMaskedBuffer() { return &m_oVisibilityMaskedBuffer; }

protected:
	CFrameBuffer	m_oExplosionBuffer;
	CFrameBuffer	m_oVisibility1Buffer;
	CFrameBuffer	m_oVisibility2Buffer;
	CFrameBuffer	m_oVisibilityMaskedBuffer;
};

#endif
