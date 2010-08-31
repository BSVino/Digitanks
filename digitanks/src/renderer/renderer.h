#ifndef DT_RENDERER_H
#define DT_RENDERER_H

#include <string>
#include <vector.h>
#include <matrix.h>
#include <color.h>

typedef enum
{
	BLEND_NONE = 0,
	BLEND_ALPHA,
	BLEND_ADDITIVE,
} blendtype_t;

class CRenderingContext
{
public:
				CRenderingContext(class CRenderer* pRenderer);
				~CRenderingContext();

public:
	void		Transform(const Matrix4x4& m);
	void		Translate(Vector vecTranslate);
	void		Rotate(float flAngle, Vector vecAxis);
	void		Scale(float flX, float flY, float flZ);
	void		SetBlend(blendtype_t eBlend);
	void		SetAlpha(float flAlpha) { m_flAlpha = flAlpha; };
	void		SetDepthMask(bool bDepthMask);
	void		SetBackCulling(bool bCull);
	void		SetColorSwap(Color clrSwap);

	float		GetAlpha() { return m_flAlpha; };

	void		RenderModel(size_t iModel, bool bNewCallList = false);
	void		RenderSceneNode(class CModel* pModel, class CConversionScene* pScene, class CConversionSceneNode* pNode, bool bNewCallList);
	void		RenderMeshInstance(class CModel* pModel, class CConversionScene* pScene, class CConversionMeshInstance* pMeshInstance, bool bNewCallList);

	void		RenderSphere();

	void		UseFrameBuffer(const class CFrameBuffer* pBuffer);
	void		UseProgram(size_t iProgram);
	void		SetUniform(const char* pszName, int iValue);
	void		SetUniform(const char* pszName, float flValue);
	void		SetUniform(const char* pszName, const Vector& vecValue);
	void		BindTexture(size_t iTexture);
	void		SetColor(Color c);
	void		BeginRenderTris();
	void		BeginRenderQuads();
	void		TexCoord(float s, float t);
	void		TexCoord(const Vector& v);
	void		Vertex(const Vector& v);
	void		EndRender();

protected:
	void		PushAttribs();

public:
	CRenderer*	m_pRenderer;

	bool		m_bMatrixTransformations;
	bool		m_bBoundTexture;
	bool		m_bFBO;
	size_t		m_iProgram;
	bool		m_bAttribs;

	bool		m_bColorSwap;
	Color		m_clrSwap;

	blendtype_t	m_eBlend;
	float		m_flAlpha;
};

class CRopeRenderer
{
public:
						CRopeRenderer(class CRenderer* pRenderer, size_t iTexture, Vector vecStart);

public:
	void				AddLink(Vector vecLink);
	void				Finish(Vector vecLink);

	void				SetWidth(float flWidth) { m_flWidth = flWidth; };
	void				SetColor(Color c) { m_clrRope = c; };
	void				SetTextureScale(float flTextureScale) { m_flTextureScale = flTextureScale; };
	void				SetTextureOffset(float flTextureOffset) { m_flTextureOffset = flTextureOffset; };

	void				SetForward(Vector vecForward);

protected:
	CRenderer*			m_pRenderer;
	CRenderingContext	m_oContext;

	size_t				m_iTexture;
	Vector				m_vecLastLink;
	bool				m_bFirstLink;

	float				m_flWidth;
	Color				m_clrRope;
	float				m_flTextureScale;
	float				m_flTextureOffset;

	bool				m_bUseForward;
	Vector				m_vecForward;
};

class CFrameBuffer
{
public:
				CFrameBuffer();

public:
	size_t		m_iWidth;
	size_t		m_iHeight;

	size_t		m_iMap;
	size_t		m_iDepth;
	size_t		m_iFB;
};

#define BLOOM_FILTERS 3

class CRenderer
{
public:
					CRenderer(size_t iWidth, size_t iHeight);

public:
	CFrameBuffer	CreateFrameBuffer(size_t iWidth, size_t iHeight, bool bDepth, bool bLinear);

	void			CreateNoise();

	virtual void	SetupFrame();
	void			DrawBackground();
	void			StartRendering();
	virtual void	FinishRendering();
	virtual void	RenderOffscreenBuffers() {};
	virtual void	RenderFullscreenBuffers() {};

	void			RenderMapFullscreen(size_t iMap);
	void			RenderMapToBuffer(size_t iMap, CFrameBuffer* pBuffer);

	void			SetCameraPosition(Vector vecCameraPosition) { m_vecCameraPosition = vecCameraPosition; };
	void			SetCameraTarget(Vector vecCameraTarget) { m_vecCameraTarget = vecCameraTarget; };

	Vector			GetCameraVector();
	void			GetCameraVectors(Vector* pvecForward, Vector* pvecRight, Vector* pvecUp);

	void			SetSize(int w, int h);

	Vector			ScreenPosition(Vector vecWorld);
	Vector			WorldPosition(Vector vecScreen);

	const CFrameBuffer*	GetSceneBuffer() { return &m_oSceneBuffer; }

public:
	static size_t	CreateCallList(size_t iModel);
	static size_t	LoadTextureIntoGL(std::wstring sFilename, bool bHUD = false);

protected:
	size_t			m_iWidth;
	size_t			m_iHeight;

	Vector			m_vecCameraPosition;
	Vector			m_vecCameraTarget;

	double			m_aiModelView[16];
	double			m_aiProjection[16];
	int				m_aiViewport[4];

	CFrameBuffer	m_oSceneBuffer;

	CFrameBuffer	m_oBloom1Buffers[BLOOM_FILTERS];
	CFrameBuffer	m_oBloom2Buffers[BLOOM_FILTERS];

	CFrameBuffer	m_oNoiseBuffer;
};

#endif
