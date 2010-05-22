#ifndef DT_RENDERER_H
#define DT_RENDERER_H

#include <vector.h>

class CRenderingContext
{
public:
				CRenderingContext();
				~CRenderingContext();

public:
	void		Translate(Vector vecTranslate);
	void		Rotate(float flAngle, Vector vecAxis);
	void		Scale(float flX, float flY, float flZ);
	void		SetAlpha(float flAlpha) { m_flAlpha = flAlpha; };

	void		RenderModel(size_t iModel);
	void		RenderSceneNode(class CConversionScene* pScene, class CConversionSceneNode* pNode);
	void		RenderMeshInstance(class CConversionScene* pScene, class CConversionMeshInstance* pMeshInstance);

public:
	bool		m_bMatrixTransformations;

	float		m_flAlpha;
};

class CRenderer
{
public:
				CRenderer(size_t iWidth, size_t iHeight);

public:
	void		SetupFrame();
	void		DrawBackground();
	void		StartRendering();
	void		FinishRendering();

	void		SetCameraPosition(Vector vecCameraPosition) { m_vecCameraPosition = vecCameraPosition; };
	void		SetCameraTarget(Vector vecCameraTarget) { m_vecCameraTarget = vecCameraTarget; };

	void		SetSize(int w, int h);

	Vector		ScreenPosition(Vector vecWorld);
	Vector		WorldPosition(Vector vecScreen);

protected:
	size_t		m_iWidth;
	size_t		m_iHeight;

	Vector		m_vecCameraPosition;
	Vector		m_vecCameraTarget;

	double		m_aiModelView[16];
	double		m_aiProjection[16];
	int			m_aiViewport[4];
};

#endif
