#ifndef DT_DIGITANKS_RENDERER_H
#define DT_DIGITANKS_RENDERER_H

#include <common.h>
#include <renderer/game_renderer.h>

#include "structures/structure.h"

class CDigitanksRenderer : public CGameRenderer
{
	DECLARE_CLASS(CDigitanksRenderer, CGameRenderer);

public:
					CDigitanksRenderer();

public:
	virtual void	Initialize();
	virtual void	SetupFrame(class CRenderingContext* pContext);
	virtual void	DrawBackground(class CRenderingContext* pContext) {};	// Skybox instead
	virtual void	StartRendering(class CRenderingContext* pContext);
	virtual void	DrawSkybox(class CRenderingContext* pContext);
	virtual void    OnDrawSkybox(class CRenderingContext* pContext);
	virtual void	FinishRendering(class CRenderingContext* pContext);
	virtual void	RenderPreviewModes();
	virtual void	RenderFogOfWar();
	virtual void	RenderAvailableAreas();
	virtual void	RenderOffscreenBuffers(class CRenderingContext* pContext);
	virtual void	RenderFullscreenBuffers(class CRenderingContext* pContext);

	const CFrameBuffer*	GetExplosionBuffer() { m_bExplosionsThisFrame = true; return &m_oExplosionBuffer; }
	const CFrameBuffer*	GetVisibility1Buffer() { return &m_oVisibility1Buffer; }
	const CFrameBuffer*	GetVisibility2Buffer() { return &m_oVisibility2Buffer; }
	const CFrameBuffer*	GetVisibilityMaskedBuffer() { return &m_oVisibilityMaskedBuffer; }
	const CFrameBuffer*	GetAvailableAreaBuffer() { return &m_oAvailableAreaBuffer; }

	virtual float	BloomBrightnessCutoff() const;
	virtual float	BloomScale() const;
	void			BloomPulse();

	void			ClearTendrilBatches();
	void			AddTendrilBatch(const class CSupplier* pSupplier);
	void			RenderTendrilBatches();

protected:
	CFrameBuffer	m_oExplosionBuffer;
	CFrameBuffer	m_oVisibility1Buffer;
	CFrameBuffer	m_oVisibility2Buffer;
	CFrameBuffer	m_oVisibilityMaskedBuffer;
	CFrameBuffer	m_oAvailableAreaBuffer;

#ifdef _DEBUG
	CFrameBuffer    m_oDebugBuffer;
#endif

	bool m_bExplosionsThisFrame;
	bool m_bAvailableAreasThisFrame;

	CTextureHandle  m_hNoise;
	CTextureHandle  m_hVignetting;

	size_t			m_iSkyboxFT;
	size_t			m_iSkyboxLF;
	size_t			m_iSkyboxBK;
	size_t			m_iSkyboxRT;
	size_t			m_iSkyboxDN;
	size_t			m_iSkyboxUP;

	size_t			m_iRing1;
	size_t			m_iRing2;
	size_t			m_iRing3;
	float			m_flRing1Yaw;
	float			m_flRing2Yaw;
	float			m_flRing3Yaw;

	size_t			m_iVortex;
	float			m_flVortexYaw;

	size_t			m_iDigiverse;

	size_t			m_iFloaters[15];

	double			m_flLastBloomPulse;

	tvector<CEntityHandle<CSupplier> > m_ahTendrilBatches;
};

#endif
