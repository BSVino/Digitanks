#include "crunch.h"

#include <assert.h>
#include <IL/il.h>
#include <IL/ilu.h>
#include <GL/glew.h>
#include <GL/glut.h>

#include <raytracer/raytracer.h>
#include <maths.h>

#if 0
#ifdef _DEBUG
#define NORMAL_DEBUG
#endif
#endif

#ifdef NORMAL_DEBUG
#include "ui/modelwindow.h"
#endif

CNormalGenerator::CNormalGenerator(CConversionScene* pScene, std::vector<CMaterial>* paoMaterials)
{
	m_pScene = pScene;
	m_paoMaterials = paoMaterials;

	m_aflHeightValues = NULL;
	m_aflHeightGeneratedValues = NULL;
	m_avecNormalValues = NULL;
	m_aiHeightReads = NULL;
	m_bPixelMask = NULL;

	SetSize(512, 512);

	m_pWorkListener = NULL;

	m_bIsGenerating = false;
	m_bDoneGenerating = false;
	m_bStopGenerating = false;
}

CNormalGenerator::~CNormalGenerator()
{
	free(m_bPixelMask);
	delete[] m_aflHeightValues;
	delete[] m_aflHeightGeneratedValues;
	delete[] m_avecNormalValues;
	delete[] m_aiHeightReads;
}

void CNormalGenerator::SetSize(size_t iWidth, size_t iHeight)
{
	m_iWidth = iWidth;
	m_iHeight = iHeight;

	if (m_aflHeightValues)
	{
		delete[] m_aflHeightValues;
		delete[] m_aflHeightGeneratedValues;
		delete[] m_avecNormalValues;
		delete[] m_aiHeightReads;
	}

	// Shadow volume result buffer.
	m_aflHeightValues = new float[iWidth*iHeight];
	m_aflHeightGeneratedValues = new float[iWidth*iHeight];
	m_avecNormalValues = new Vector[iWidth*iHeight];
	m_aiHeightReads = new size_t[iWidth*iHeight];

	// Big hack incoming!
	memset(&m_aflHeightValues[0], 0, iWidth*iHeight*sizeof(float));
	memset(&m_aflHeightGeneratedValues[0], 0, iWidth*iHeight*sizeof(float));
	memset(&m_avecNormalValues[0].x, 0, iWidth*iHeight*sizeof(Vector));
	memset(&m_aiHeightReads[0], 0, iWidth*iHeight*sizeof(size_t));

	if (m_bPixelMask)
		free(m_bPixelMask);
	m_bPixelMask = (bool*)malloc(m_iWidth*m_iHeight*sizeof(bool));
}

void CNormalGenerator::SetModels(const std::vector<CConversionMeshInstance*>& apHiRes, const std::vector<CConversionMeshInstance*>& apLoRes)
{
	m_apLoRes = apLoRes;
	m_apHiRes = apHiRes;
}

void CNormalGenerator::Generate()
{
	if (m_pWorkListener)
	{
		m_pWorkListener->BeginProgress();
		m_pWorkListener->SetAction(L"Building tree", 0);
	}

#ifdef _DEBUG
	if (CModelWindow::Get())
		CModelWindow::Get()->ClearDebugLines();
#endif

	m_bIsGenerating = true;
	m_bStopGenerating = false;
	m_bDoneGenerating = false;

	raytrace::CRaytracer* pTracer = NULL;

	pTracer = new raytrace::CRaytracer(m_pScene);

	for (size_t	m = 0; m < m_apHiRes.size(); m++)
		pTracer->AddMeshInstance(m_apHiRes[m]);

	pTracer->BuildTree();

	float flTotalArea = 0;

	for (size_t m = 0; m < m_pScene->GetNumMeshes(); m++)
	{
		CConversionMesh* pMesh = m_pScene->GetMesh(m);
		for (size_t f = 0; f < pMesh->GetNumFaces(); f++)
		{
			CConversionFace* pFace = pMesh->GetFace(f);
			flTotalArea += pFace->GetUVArea();
		}
	}

	if (m_pWorkListener)
		m_pWorkListener->SetAction(L"Generating", (size_t)(flTotalArea*m_iWidth*m_iHeight));

	size_t iRendered = 0;

	for (size_t i = 0; i < m_apLoRes.size(); i++)
	{
		CConversionMeshInstance* pMeshInstance = m_apLoRes[i];

		if (!pMeshInstance->GetMesh()->GetNumUVs())
			continue;

		for (size_t f = 0; f < pMeshInstance->GetMesh()->GetNumFaces(); f++)
		{
			CConversionFace* pFace = pMeshInstance->GetMesh()->GetFace(f);

			if (pFace->m != ~0)
			{
				if (!pMeshInstance->GetMappedMaterial(pFace->m)->IsVisible())
					continue;

				CConversionMaterial* pMaterial = m_pScene->GetMaterial(pMeshInstance->GetMappedMaterial(pFace->m)->m_iMaterial);
				if (pMaterial && !pMaterial->IsVisible())
					continue;
			}

			std::vector<Vector> avecPoints;
			std::vector<size_t> aiPoints;
			for (size_t t = 0; t < pFace->GetNumVertices(); t++)
			{
				avecPoints.push_back(pMeshInstance->GetVertex(pFace->GetVertex(t)->v));
				aiPoints.push_back(t);
			}

			while (avecPoints.size() > 3)
			{
				size_t iEar = FindEar(avecPoints);
				size_t iLast = iEar==0?avecPoints.size()-1:iEar-1;
				size_t iNext = iEar==avecPoints.size()-1?0:iEar+1;
				GenerateTriangleByTexel(pMeshInstance, pFace, aiPoints[iLast], aiPoints[iEar], aiPoints[iNext], pTracer, iRendered);
				avecPoints.erase(avecPoints.begin()+iEar);
				aiPoints.erase(aiPoints.begin()+iEar);
				if (m_bStopGenerating)
					break;
			}
			GenerateTriangleByTexel(pMeshInstance, pFace, aiPoints[0], aiPoints[1], aiPoints[2], pTracer, iRendered);
			if (m_bStopGenerating)
				break;
		}
		if (m_bStopGenerating)
			break;
	}

	delete pTracer;

	Bleed();
	ScaleHeightValues(m_aflHeightValues);
	NormalizeHeightValues(m_aflHeightValues);

	if (!m_bStopGenerating)
		m_bDoneGenerating = true;
	m_bIsGenerating = false;

	// One last call to let them know we're done.
	if (m_pWorkListener)
		m_pWorkListener->EndProgress();
}

void CNormalGenerator::GenerateTriangleByTexel(CConversionMeshInstance* pMeshInstance, CConversionFace* pFace, size_t v1, size_t v2, size_t v3, raytrace::CRaytracer* pTracer, size_t& iRendered)
{
	CConversionVertex* pV1 = pFace->GetVertex(v1);
	CConversionVertex* pV2 = pFace->GetVertex(v2);
	CConversionVertex* pV3 = pFace->GetVertex(v3);

	CConversionMesh* pMesh = pMeshInstance->GetMesh();

	Vector vt1 = pMesh->GetUV(pV1->vt);
	Vector vt2 = pMesh->GetUV(pV2->vt);
	Vector vt3 = pMesh->GetUV(pV3->vt);

	Vector vecLoUV = vt1;
	Vector vecHiUV = vt1;

	if (vt2.x < vecLoUV.x)
		vecLoUV.x = vt2.x;
	if (vt3.x < vecLoUV.x)
		vecLoUV.x = vt3.x;
	if (vt2.x > vecHiUV.x)
		vecHiUV.x = vt2.x;
	if (vt3.x > vecHiUV.x)
		vecHiUV.x = vt3.x;

	if (vt2.y < vecLoUV.y)
		vecLoUV.y = vt2.y;
	if (vt3.y < vecLoUV.y)
		vecLoUV.y = vt3.y;
	if (vt2.y > vecHiUV.y)
		vecHiUV.y = vt2.y;
	if (vt3.y > vecHiUV.y)
		vecHiUV.y = vt3.y;

	size_t iLoX = (size_t)(vecLoUV.x * m_iWidth);
	size_t iLoY = (size_t)(vecLoUV.y * m_iHeight);
	size_t iHiX = (size_t)(vecHiUV.x * m_iWidth);
	size_t iHiY = (size_t)(vecHiUV.y * m_iHeight);

	for (size_t i = iLoX; i <= iHiX; i++)
	{
		for (size_t j = iLoY; j <= iHiY; j++)
		{
			float flU = ((float)i + 0.5f)/(float)m_iWidth;
			float flV = ((float)j + 0.5f)/(float)m_iHeight;

			bool bInside = PointInTriangle(Vector(flU,flV,0), vt1, vt2, vt3);

			if (!bInside)
				continue;

			Vector v1 = pMeshInstance->GetVertex(pV1->v);
			Vector v2 = pMeshInstance->GetVertex(pV2->v);
			Vector v3 = pMeshInstance->GetVertex(pV3->v);

			Vector vn1 = pMeshInstance->GetNormal(pV1->vn);
			Vector vn2 = pMeshInstance->GetNormal(pV2->vn);
			Vector vn3 = pMeshInstance->GetNormal(pV3->vn);

			// Find where the UV is in world space.

			// First build 2x2 a "matrix" of the UV values.
			float mta = vt2.x - vt1.x;
			float mtb = vt3.x - vt1.x;
			float mtc = vt2.y - vt1.y;
			float mtd = vt3.y - vt1.y;

			// Invert it.
			float d = mta*mtd - mtb*mtc;
			float mtia =  mtd / d;
			float mtib = -mtb / d;
			float mtic = -mtc / d;
			float mtid =  mta / d;

			// Now build a 2x3 "matrix" of the vertices.
			float mva = v2.x - v1.x;
			float mvb = v3.x - v1.x;
			float mvc = v2.y - v1.y;
			float mvd = v3.y - v1.y;
			float mve = v2.z - v1.z;
			float mvf = v3.z - v1.z;

			// Multiply them together.
			// [a b]   [a b]   [a b]
			// [c d] * [c d] = [c d]
			// [e f]           [e f]
			// Really wish I had a matrix math library about now!
			float mra = mva*mtia + mvb*mtic;
			float mrb = mva*mtib + mvb*mtid;
			float mrc = mvc*mtia + mvd*mtic;
			float mrd = mvc*mtib + mvd*mtid;
			float mre = mve*mtia + mvf*mtic;
			float mrf = mve*mtib + mvf*mtid;

			// These vectors should be the U and V axis in world space.
			Vector vecUAxis(mra, mrc, mre);
			Vector vecVAxis(mrb, mrd, mrf);

			Vector vecUVOrigin = v1 - vecUAxis * vt1.x - vecVAxis * vt1.y;

			Vector vecUVPosition = vecUVOrigin + vecUAxis * flU + vecVAxis * flV;

			Vector vecNormal;

			float wv1 = DistanceToLine(vecUVPosition, v2, v3) / DistanceToLine(v1, v2, v3);
			float wv2 = DistanceToLine(vecUVPosition, v1, v3) / DistanceToLine(v2, v1, v3);
			float wv3 = DistanceToLine(vecUVPosition, v1, v2) / DistanceToLine(v3, v1, v2);

			vecNormal = vn1 * wv1 + vn2 * wv2 + vn3 * wv3;

			size_t iTexel;
			Texel(i, j, iTexel, false);

			Vector vecHitFront;
			bool bHitFront = pTracer->Raytrace(Ray(vecUVPosition, vecNormal), &vecHitFront);

			Vector vecHitBack;
			bool bHitBack = pTracer->Raytrace(Ray(vecUVPosition, -vecNormal), &vecHitBack);

#ifdef NORMAL_DEBUG
			if (bHitFront && (vecUVPosition - vecHitFront).LengthSqr() > 0.001f)
				CModelWindow::Get()->AddDebugLine(vecUVPosition, vecHitFront);
			if (bHitBack && (vecUVPosition - vecHitBack).LengthSqr() > 0.001f)
				CModelWindow::Get()->AddDebugLine(vecUVPosition, vecHitBack);
#endif

			float flHit;
			if (bHitFront && !bHitBack)
				flHit = (vecUVPosition - vecHitFront).Length();
			else if (bHitBack && !bHitFront)
				flHit = -(vecUVPosition - vecHitBack).Length();
			else if (!bHitBack && !bHitFront)
				flHit = 0;
			else
			{
				float flHitFront = (vecUVPosition - vecHitFront).LengthSqr();
				float flHitBack = -(vecUVPosition - vecHitBack).LengthSqr();

				flHit = sqrt((fabs(flHitFront)<fabs(flHitBack))?flHitFront:flHitBack);
			}

			float flClosest = pTracer->Closest(vecUVPosition);

			if (flClosest >= 0 && fabs(flClosest) < fabs(flHit))
				flHit = flHit>0?flClosest:-flClosest;

			m_aflHeightValues[iTexel] += flHit;

			m_aiHeightReads[iTexel]++;
			m_bPixelMask[iTexel] = true;

			if (m_pWorkListener)
				m_pWorkListener->WorkProgress(++iRendered);

			if (m_bStopGenerating)
				break;
		}
		if (m_bStopGenerating)
			break;
	}
}

void CNormalGenerator::Bleed()
{
	bool* abPixelMask = (bool*)malloc(m_iWidth*m_iHeight*sizeof(bool));

	if (m_pWorkListener)
		m_pWorkListener->SetAction(L"Bleeding edges", 0);

	// This is for pixels that have been set this frame.
	memset(&abPixelMask[0], 0, m_iWidth*m_iHeight*sizeof(bool));

	for (size_t w = 0; w < m_iWidth; w++)
	{
		for (size_t h = 0; h < m_iHeight; h++)
		{
			float flTotal = 0;
			size_t iTotal = 0;
			size_t iTexel;

			// If the texel has the mask on then it already has a value so skip it.
			if (Texel(w, h, iTexel, true))
				continue;

			if (Texel(w-1, h-1, iTexel))
			{
				flTotal += m_aflHeightValues[iTexel];
				iTotal++;
			}

			if (Texel(w-1, h, iTexel))
			{
				flTotal += m_aflHeightValues[iTexel];
				iTotal++;
			}

			if (Texel(w-1, h+1, iTexel))
			{
				flTotal += m_aflHeightValues[iTexel];
				iTotal++;
			}

			if (Texel(w, h+1, iTexel))
			{
				flTotal += m_aflHeightValues[iTexel];
				iTotal++;
			}

			if (Texel(w+1, h+1, iTexel))
			{
				flTotal += m_aflHeightValues[iTexel];
				iTotal++;
			}

			if (Texel(w+1, h, iTexel))
			{
				flTotal += m_aflHeightValues[iTexel];
				iTotal++;
			}

			if (Texel(w+1, h-1, iTexel))
			{
				flTotal += m_aflHeightValues[iTexel];
				iTotal++;
			}

			if (Texel(w, h-1, iTexel))
			{
				flTotal += m_aflHeightValues[iTexel];
				iTotal++;
			}

			Texel(w, h, iTexel, false);

			if (iTotal)
			{
				flTotal /= (float)iTotal;
				m_aflHeightValues[iTexel] = flTotal/(float)iTotal;
				abPixelMask[iTexel] = true;
			}
		}
	}

	for (size_t p = 0; p < m_iWidth*m_iHeight; p++)
		m_bPixelMask[p] |= abPixelMask[p];

	free(abPixelMask);
}

void CNormalGenerator::ScaleHeightValues(float* aflHeightValues)
{
	size_t i;
	bool bFirst = true;
	float flLowestValue;
	float flHighestValue;

	for (i = 0; i < m_iWidth*m_iHeight; i++)
	{
		if (bFirst && m_aiHeightReads[i])
		{
			flHighestValue = flLowestValue = m_aflHeightValues[i]/m_aiHeightReads[i];
			bFirst = false;
		}
		else if (m_aiHeightReads[i])
		{
			float flValue = m_aflHeightValues[i]/m_aiHeightReads[i];
			if (flValue < flLowestValue)
				flLowestValue = flValue;
			if (flValue > flHighestValue)
				flHighestValue = flValue;
		}
	}

	if (!bFirst)
	{
		// Use a simple scale instead of remapping [lo, hi] -> [0, 1] so that 0.5 stays our centerline.
		float flScale = (fabs(flLowestValue) > fabs(flHighestValue))?fabs(flLowestValue):fabs(flHighestValue);
		for (i = 0; i < m_iWidth*m_iHeight; i++)
		{
			if (m_aiHeightReads[i] && aflHeightValues[i] != 0)
				aflHeightValues[i] = RemapVal(m_aflHeightValues[i], -flScale, flScale, 0.0f, 1.0f);
			else
				aflHeightValues[i] = 0.5f;
		}
	}
}

void CNormalGenerator::NormalizeHeightValues(float* aflHeightValues)
{
	float flScale = ((m_iWidth+m_iHeight)/2.0f)/10.0f;

	for (size_t x = 0; x < m_iWidth; x++)
	{
		for (size_t y = 0; y < m_iHeight; y++)
		{
			std::vector<Vector> avecHeights;
			size_t iTexel;

			Texel(x, y, iTexel, false);
			Vector vecCenter((float)x, (float)y, aflHeightValues[iTexel]*flScale);

			Vector& vecNormal = m_avecNormalValues[iTexel];
			vecNormal = Vector(0,0,0);

			if (Texel(x+1, y, iTexel, false))
			{
				Vector vecNeighbor(x+1.0f, (float)y, aflHeightValues[iTexel]*flScale);
				vecNormal += (vecNeighbor-vecCenter).Normalized().Cross(Vector(0, 1, 0));
			}

			if (Texel(x-1, y, iTexel, false))
			{
				Vector vecNeighbor(x-1.0f, (float)y, aflHeightValues[iTexel]*flScale);
				vecNormal += (vecNeighbor-vecCenter).Normalized().Cross(Vector(0, -1, 0));
			}

			if (Texel(x, y+1, iTexel, false))
			{
				Vector vecNeighbor((float)x, y+1.0f, aflHeightValues[iTexel]*flScale);
				vecNormal += (vecNeighbor-vecCenter).Normalized().Cross(Vector(-1, 0, 0));
			}

			if (Texel(x, y-1, iTexel, false))
			{
				Vector vecNeighbor((float)x, y-1.0f, aflHeightValues[iTexel]*flScale);
				vecNormal += (vecNeighbor-vecCenter).Normalized().Cross(Vector(1, 0, 0));
			}

			vecNormal.Normalize();

			for (size_t i = 0; i < 3; i++)
				vecNormal[i] = RemapVal(vecNormal[i], -1.0f, 1.0f, 0.0f, 0.99f);	// Don't use 1.0 because of integer overflow.
		}
	}
}

size_t CNormalGenerator::GenerateTexture(bool bInMedias)
{
	float* aflHeightValues = m_aflHeightValues;

	if (bInMedias)
	{
		// Use this temporary buffer so we don't clobber the original.
		aflHeightValues = m_aflHeightGeneratedValues;
		ScaleHeightValues(aflHeightValues);
		NormalizeHeightValues(aflHeightValues);
	}

	GLuint iGLId;
	glGenTextures(1, &iGLId);
	glBindTexture(GL_TEXTURE_2D, iGLId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, (GLint)m_iWidth, (GLint)m_iHeight, GL_RGB, GL_FLOAT, &m_avecNormalValues[0].x);

	return iGLId;
}

void CNormalGenerator::SaveToFile(const wchar_t *pszFilename)
{
	if (!pszFilename)
		return;

	ilEnable(IL_FILE_OVERWRITE);

	ILuint iDevILId;
	ilGenImages(1, &iDevILId);
	ilBindImage(iDevILId);

	// WHAT A HACK!
	ilTexImage((ILint)m_iWidth, (ILint)m_iHeight, 1, 3, IL_RGB, IL_FLOAT, NULL);

	ilSetData(&m_avecNormalValues[0].x);

	// Formats like PNG and VTF don't work unless it's in integer format.
	ilConvertImage(IL_RGB, IL_UNSIGNED_INT);

	if (!CModelWindow::GetSMAKTexture() && (m_iWidth > 128 || m_iHeight > 128))
	{
		iluImageParameter(ILU_FILTER, ILU_BILINEAR);
		iluScale(128, 128, 1);
	}

	ilSaveImage(pszFilename);

	ilDeleteImages(1,&iDevILId);
}

bool CNormalGenerator::Texel(size_t w, size_t h, size_t& iTexel, bool bUseMask)
{
	if (w < 0 || h < 0 || w >= m_iWidth || h >= m_iHeight)
		return false;

	iTexel = m_iHeight*h + w;

	assert(iTexel >= 0 && iTexel < m_iWidth * m_iHeight);

	if (bUseMask && !m_bPixelMask[iTexel])
		return false;

	return true;
}
