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
	virtual void	Initialize();
	virtual void	SetupFrame();
	virtual void	DrawBackground() {};	// Skybox instead
	virtual void	StartRendering();
	virtual void	RenderSkybox();
	virtual void	FinishRendering();
	virtual void	RenderPreviewModes();
	virtual void	RenderFogOfWar();
	virtual void	RenderAvailableAreas();
	virtual void	RenderOffscreenBuffers();
	virtual void	RenderFullscreenBuffers();

	void			RenderBloomPass(CFrameBuffer* apSources, CFrameBuffer* apTargets, bool bHorizontal);

	const CFrameBuffer*	GetExplosionBuffer() { return &m_oExplosionBuffer; }
	const CFrameBuffer*	GetVisibility1Buffer() { return &m_oVisibility1Buffer; }
	const CFrameBuffer*	GetVisibility2Buffer() { return &m_oVisibility2Buffer; }
	const CFrameBuffer*	GetVisibilityMaskedBuffer() { return &m_oVisibilityMaskedBuffer; }
	const CFrameBuffer*	GetAvailableAreaBuffer() { return &m_oAvailableAreaBuffer; }

	void			BloomPulse();

protected:
	CFrameBuffer	m_oExplosionBuffer;
	CFrameBuffer	m_oVisibility1Buffer;
	CFrameBuffer	m_oVisibility2Buffer;
	CFrameBuffer	m_oVisibilityMaskedBuffer;
	CFrameBuffer	m_oAvailableAreaBuffer;

	size_t			m_iVignetting;

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

	size_t			m_iDigiverse;
	size_t			m_iFloaters[15];

	float			m_flLastBloomPulse;
};

#endif
