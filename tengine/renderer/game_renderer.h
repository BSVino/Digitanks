#ifndef TENGINE_GAME_RENDERER_H
#define TENGINE_GAME_RENDERER_H

#include <tmap.h>

#include <renderer/renderer.h>
#include <textures/materialhandle.h>
#include <textures/texturehandle.h>

class CRenderBatch
{
public:
	const class CBaseEntity*	pEntity;
	class CModel*				pModel;
	Matrix4x4					mTransformation;
	bool						bWinding;
	Color						clrRender;
	size_t						iMaterial;
};

class CGameRenderer : public CRenderer
{
	DECLARE_CLASS(CGameRenderer, CRenderer);

	friend class CGameRenderingContext;

public:
					CGameRenderer(size_t iWidth, size_t iHeight);

public:
	virtual void	Render();
	virtual void	RenderEverything();

	virtual void	SetupFrame(class CRenderingContext* pContext);
	virtual void	DrawSkybox(class CRenderingContext* pContext);
	virtual void	ModifySkyboxContext(class CRenderingContext* c) {};
	virtual bool	ModifyShader(const class CBaseEntity* pEntity, class CRenderingContext* c) { return true; };
	virtual void	FinishRendering(class CRenderingContext* pContext);
	virtual void	FinishFrame(class CRenderingContext* pContext);
	virtual void    DrawWeaponViewModel();

	void			SetSkybox(const CTextureHandle& ft, const CTextureHandle& bk, const CTextureHandle& lf, const CTextureHandle& rt, const CTextureHandle& up, const CTextureHandle& dn);
	void			DisableSkybox();

	bool			ShouldBatchThisFrame() { return m_bBatchThisFrame; }
	void			BeginBatching();
	void			AddToBatch(class CModel* pModel, const class CBaseEntity* pEntity, const Matrix4x4& mTransformations, const Color& clrRender, bool bWinding);
	void			RenderBatches();
	bool			IsBatching() { return m_bBatching; };

	void			ClassifySceneAreaPosition(class CModel* pModel);
	void			FindSceneAreaPosition(class CModel* pModel);
	size_t			GetSceneAreaPosition(class CModel* pModel);

	bool			IsRenderingTransparent() const { return m_bRenderingTransparent; }
	void			SetRenderingTransparent(bool b) { m_bRenderingTransparent = b; }

	const class CBaseEntity*	GetRenderingEntity() { return m_pRendering; }

	virtual bool    ShouldCullByFrustum() const { return true; }
	virtual bool	ShouldRenderPhysicsDebug() const { return true; };
	virtual bool    ShouldRenderParticles() const { return true; }

	CTextureHandle  GetInvalidTexture() const { return m_hInvalidTexture; }
	CMaterialHandle GetInvalidMaterial() const { return m_hInvalidMaterial; }

protected:
	CTextureHandle	m_hSkyboxFT;
	CTextureHandle	m_hSkyboxLF;
	CTextureHandle	m_hSkyboxBK;
	CTextureHandle	m_hSkyboxRT;
	CTextureHandle	m_hSkyboxDN;
	CTextureHandle	m_hSkyboxUP;

	Vector2D		m_avecSkyboxTexCoords[6];
	Vector			m_avecSkyboxFT[6];
	Vector			m_avecSkyboxBK[6];
	Vector			m_avecSkyboxLF[6];
	Vector			m_avecSkyboxRT[6];
	Vector			m_avecSkyboxUP[6];
	Vector			m_avecSkyboxDN[6];

	tvector<CBaseEntity*>	m_apRenderOpaqueList;
	tvector<CBaseEntity*>	m_apRenderTransparentList;
	bool			m_bRenderingTransparent;

	bool			m_bBatchThisFrame;
	bool			m_bBatching;
	tmap<CMaterialHandle, tvector<CRenderBatch> > m_aBatches;

	const CBaseEntity*	m_pRendering;

	tmap<tstring, size_t> m_aiCurrentSceneAreas;

	CTextureHandle  m_hInvalidTexture;
	CMaterialHandle m_hInvalidMaterial;

private:
	static size_t	s_iTexturesLoaded;
};

#endif
