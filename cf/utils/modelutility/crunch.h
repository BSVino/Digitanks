#ifndef CF_CRUNCH_H
#define CF_CRUNCH_H

#include <modelconverter/convmesh.h>
#include "ui/modelwindow.h"

#include <GL/glut.h>

class CAOGenerator
{
public:
							CAOGenerator(CConversionScene* pScene, std::vector<CMaterial>* paoMaterials);
							~CAOGenerator();

	void					SetSize(size_t iWidth, size_t iHeight);
	void					SetUseTexture(bool bUseTexture) { m_bUseTexture = bUseTexture; };

	void					Generate();
	Vector					RenderSceneFromPosition(Vector vecPosition, Vector vecDirection, CConversionFace* pFace);
	void					DebugRenderSceneLookAtPosition(Vector vecPosition, Vector vecDirection, CConversionFace* pRenderFace);

	size_t					GenerateTexture();
	void					SaveToFile(const char* pszFilename);

protected:
	CConversionScene*		m_pScene;
	std::vector<CMaterial>*	m_paoMaterials;

	size_t					m_iWidth;
	size_t					m_iHeight;
	bool					m_bUseTexture;

	size_t					m_iViewportSize;
	size_t					m_iPixelDepth;
	GLfloat*				m_pPixels;

	GLuint					m_iSceneList;

	Vector*					m_avecShadowValues;
	size_t*					m_aiShadowReads;
};

#endif