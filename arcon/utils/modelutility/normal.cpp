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

	m_avecNormalValues = NULL;
	m_avecNormalGeneratedValues = NULL;
	m_bPixelMask = NULL;

	SetSize(512, 512);

	m_pWorkListener = NULL;

	m_bIsGenerating = false;
	m_bDoneGenerating = false;
	m_bStopGenerating = false;

	m_iNormal2GLId = 0;
	m_aflTextureTexels = NULL;
	m_aflLowPassTexels = NULL;
	m_abLowPassMask = NULL;
	m_aflNormal2Texels = NULL;
	m_bNewNormal2Available = false;
	m_bNormal2Generated = false;

	m_pNormalParallelizer = NULL;
	m_pNormal2Parallelizer = NULL;
}

CNormalGenerator::~CNormalGenerator()
{
	free(m_bPixelMask);
	delete[] m_avecNormalValues;
	delete[] m_avecNormalGeneratedValues;

	if (m_iNormal2GLId)
		glDeleteTextures(1, &m_iNormal2GLId);

	if (m_aflNormal2Texels)
	{
		delete[] m_aflTextureTexels;
		delete[] m_aflLowPassTexels;
		delete[] m_abLowPassMask;
		delete[] m_aflNormal2Texels;
	}
}

void CNormalGenerator::SetSize(size_t iWidth, size_t iHeight)
{
	m_iWidth = iWidth;
	m_iHeight = iHeight;

	if (m_avecNormalValues)
	{
		delete[] m_avecNormalValues;
		delete[] m_avecNormalGeneratedValues;
	}

	// Shadow volume result buffer.
	m_avecNormalValues = new Vector[iWidth*iHeight];
	m_avecNormalGeneratedValues = new Vector[iWidth*iHeight];

	// Big hack incoming!
	memset(&m_avecNormalValues[0].x, 0, iWidth*iHeight*sizeof(Vector));
	memset(&m_avecNormalGeneratedValues[0], 0, iWidth*iHeight*sizeof(Vector));

	if (m_bPixelMask)
		free(m_bPixelMask);
	m_bPixelMask = (bool*)malloc(m_iWidth*m_iHeight*sizeof(bool));
}

void CNormalGenerator::SetModels(const std::vector<CConversionMeshInstance*>& apHiRes, const std::vector<CConversionMeshInstance*>& apLoRes)
{
	m_apLoRes = apLoRes;
	m_apHiRes = apHiRes;
}

typedef struct
{
	CNormalGenerator*			pGenerator;
	CConversionMeshInstance*	pMeshInstance;
	CConversionFace*			pFace;
	CConversionVertex*			pV1;
	CConversionVertex*			pV2;
	CConversionVertex*			pV3;
	size_t						x;
	size_t						y;
	raytrace::CRaytracer*		pTracer;
} normal_data_t;

void FindNormalAtTexel(void* pVoidData)
{
	normal_data_t* pJobData = (normal_data_t*)pVoidData;

	pJobData->pGenerator->FindNormalAtTexel(pJobData->pMeshInstance, pJobData->pFace, pJobData->pV1, pJobData->pV2, pJobData->pV3, pJobData->x, pJobData->y, pJobData->pTracer);
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

	memset(&m_bPixelMask[0], 0, m_iWidth*m_iHeight*sizeof(bool));

	m_bIsGenerating = true;
	m_bStopGenerating = false;
	m_bDoneGenerating = false;

	raytrace::CRaytracer* pTracer = NULL;

	pTracer = new raytrace::CRaytracer(m_pScene);

	for (size_t	m = 0; m < m_apHiRes.size(); m++)
		pTracer->AddMeshInstance(m_apHiRes[m]);

	pTracer->BuildTree();

	m_pNormalParallelizer = new CParallelizer((JobCallback)::FindNormalAtTexel);
	m_pNormalParallelizer->Start();

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

	RegenerateNormal2Texture();

	if (m_pWorkListener)
		m_pWorkListener->SetAction(L"Dispatching jobs", (size_t)(flTotalArea*m_iWidth*m_iHeight));

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

	m_pNormalParallelizer->FinishJobs();

	if (m_pWorkListener)
		m_pWorkListener->SetAction(L"Rendering", m_pNormalParallelizer->GetJobsTotal());

	while (true)
	{
		if (m_pNormalParallelizer->AreAllJobsDone())
			break;

		if (m_pWorkListener)
			m_pWorkListener->WorkProgress(m_pNormalParallelizer->GetJobsDone());

		if (m_bStopGenerating)
			break;
	}

	delete pTracer;

	delete m_pNormalParallelizer;

	Bleed();
	TexturizeValues(m_avecNormalValues);

	if (!m_bStopGenerating)
		m_bDoneGenerating = true;
	m_bIsGenerating = false;

	// One last call to let them know we're done.
	if (m_pWorkListener)
		m_pWorkListener->EndProgress();
}

void CNormalGenerator::GenerateTriangleByTexel(CConversionMeshInstance* pMeshInstance, CConversionFace* pFace, size_t v1, size_t v2, size_t v3, raytrace::CRaytracer* pTracer, size_t& iRendered)
{
	normal_data_t oJob;

	CConversionVertex* pV1 = pFace->GetVertex(v1);
	CConversionVertex* pV2 = pFace->GetVertex(v2);
	CConversionVertex* pV3 = pFace->GetVertex(v3);

	CConversionMesh* pMesh = pMeshInstance->GetMesh();

	oJob.pMeshInstance = pMeshInstance;
	oJob.pFace = pFace;
	oJob.pV1 = pV1;
	oJob.pV2 = pV2;
	oJob.pV3 = pV3;
	oJob.pTracer = pTracer;
	oJob.pGenerator = this;

	Vector vu1 = pMesh->GetUV(pV1->vu);
	Vector vu2 = pMesh->GetUV(pV2->vu);
	Vector vu3 = pMesh->GetUV(pV3->vu);

	Vector vecLoUV = vu1;
	Vector vecHiUV = vu1;

	if (vu2.x < vecLoUV.x)
		vecLoUV.x = vu2.x;
	if (vu3.x < vecLoUV.x)
		vecLoUV.x = vu3.x;
	if (vu2.x > vecHiUV.x)
		vecHiUV.x = vu2.x;
	if (vu3.x > vecHiUV.x)
		vecHiUV.x = vu3.x;

	if (vu2.y < vecLoUV.y)
		vecLoUV.y = vu2.y;
	if (vu3.y < vecLoUV.y)
		vecLoUV.y = vu3.y;
	if (vu2.y > vecHiUV.y)
		vecHiUV.y = vu2.y;
	if (vu3.y > vecHiUV.y)
		vecHiUV.y = vu3.y;

	size_t iLoX = (size_t)(vecLoUV.x * m_iWidth);
	size_t iLoY = (size_t)(vecLoUV.y * m_iHeight);
	size_t iHiX = (size_t)(vecHiUV.x * m_iWidth);
	size_t iHiY = (size_t)(vecHiUV.y * m_iHeight);

	for (size_t i = iLoX; i <= iHiX; i++)
	{
		for (size_t j = iLoY; j <= iHiY; j++)
		{
			oJob.x = i;
			oJob.y = j;

			m_pNormalParallelizer->AddJob(&oJob, sizeof(oJob));

			if (m_pWorkListener)
				m_pWorkListener->WorkProgress(++iRendered);

			if (m_bStopGenerating)
				break;
		}
		if (m_bStopGenerating)
			break;
	}
}

void CNormalGenerator::FindNormalAtTexel(CConversionMeshInstance* pMeshInstance, CConversionFace* pFace, CConversionVertex* pV1, CConversionVertex* pV2, CConversionVertex* pV3, size_t i, size_t j, raytrace::CRaytracer* pTracer)
{
	CConversionMesh* pMesh = pMeshInstance->GetMesh();

	Vector vu1 = pMesh->GetUV(pV1->vu);
	Vector vu2 = pMesh->GetUV(pV2->vu);
	Vector vu3 = pMesh->GetUV(pV3->vu);

	float flU = ((float)i + 0.5f)/(float)m_iWidth;
	float flV = ((float)j + 0.5f)/(float)m_iHeight;

	bool bInside = PointInTriangle(Vector(flU,flV,0), vu1, vu2, vu3);

	if (!bInside)
		return;

	Vector v1 = pMeshInstance->GetVertex(pV1->v);
	Vector v2 = pMeshInstance->GetVertex(pV2->v);
	Vector v3 = pMeshInstance->GetVertex(pV3->v);

	// Find where the UV is in world space.

	// First build 2x2 a "matrix" of the UV values.
	float mta = vu2.x - vu1.x;
	float mtb = vu3.x - vu1.x;
	float mtc = vu2.y - vu1.y;
	float mtd = vu3.y - vu1.y;

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

	Vector vecUVOrigin = v1 - vecUAxis * vu1.x - vecVAxis * vu1.y;

	Vector vecUVPosition = vecUVOrigin + vecUAxis * flU + vecVAxis * flV;

	Vector vecNormal = pFace->GetNormal(vecUVPosition, pMeshInstance);

	size_t iTexel;
	Texel(i, j, iTexel, false);

	// Maybe use a closest-poly check here to eliminate the need for some raytracing?

	raytrace::CTraceResult trFront;
	bool bHitFront = pTracer->Raytrace(Ray(vecUVPosition, vecNormal), &trFront);

	raytrace::CTraceResult trBack;
	bool bHitBack = pTracer->Raytrace(Ray(vecUVPosition, -vecNormal), &trBack);

#ifdef NORMAL_DEBUG
	if (bHitFront && (vecUVPosition - trFront.m_vecHit).LengthSqr() > 0.001f)
		CModelWindow::Get()->AddDebugLine(vecUVPosition, trFront.m_vecHit);
	if (bHitBack && (vecUVPosition - trBack.m_vecHit).LengthSqr() > 0.001f)
		CModelWindow::Get()->AddDebugLine(vecUVPosition, trBack.m_vecHit);
#endif

	Vector vecHitNormal;
	if (bHitFront && !bHitBack)
		vecHitNormal = trFront.m_pFace->GetNormal(trFront.m_vecHit, trFront.m_pMeshInstance);
	else if (bHitBack && !bHitFront)
		vecHitNormal = trBack.m_pFace->GetNormal(trBack.m_vecHit, trBack.m_pMeshInstance);
	else if (!bHitBack && !bHitFront)
		vecHitNormal = vecNormal;
	else
	{
		float flHitFront = (vecUVPosition - trFront.m_vecHit).LengthSqr();
		float flHitBack = (vecUVPosition - trBack.m_vecHit).LengthSqr();

		if (flHitFront < flHitBack)
			vecHitNormal = trFront.m_pFace->GetNormal(trFront.m_vecHit, trFront.m_pMeshInstance);
		else
			vecHitNormal = trBack.m_pFace->GetNormal(trBack.m_vecHit, trBack.m_pMeshInstance);
	}

#ifdef NORMAL_DEBUG
//	CModelWindow::Get()->AddDebugLine(vecUVPosition, vecUVPosition+vecHitNormal);
	if (bHitFront && (vecUVPosition - trFront.m_vecHit).LengthSqr() > 0.001f)
		CModelWindow::Get()->AddDebugLine(trFront.m_vecHit, trFront.m_vecHit+vecHitNormal);
	if (bHitBack && (vecUVPosition - trBack.m_vecHit).LengthSqr() > 0.001f)
		CModelWindow::Get()->AddDebugLine(trBack.m_vecHit, trBack.m_vecHit+vecHitNormal);
#endif

	// Build rotation matrix
	Matrix4x4 mObjectToTangent;

	Vector t = pFace->GetBaseVector(vecUVPosition, 0, pMeshInstance);
	Vector b = pFace->GetBaseVector(vecUVPosition, 1, pMeshInstance);
	Vector n = pFace->GetBaseVector(vecUVPosition, 2, pMeshInstance);

	mObjectToTangent.SetColumn(0, t);
	mObjectToTangent.SetColumn(1, b);
	mObjectToTangent.SetColumn(2, n);
	mObjectToTangent.InvertTR();

	Vector vecTangentNormal = mObjectToTangent*vecHitNormal;

	m_pNormalParallelizer->LockData();

	m_avecNormalValues[iTexel] += vecTangentNormal;

	m_bPixelMask[iTexel] = true;

	m_pNormalParallelizer->UnlockData();
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
			Vector vecTotal(0,0,0);
			size_t iTexel;

			// If the texel has the mask on then it already has a value so skip it.
			if (Texel(w, h, iTexel, true))
				continue;

			bool bTotal = false;

			if (Texel(w-1, h-1, iTexel))
			{
				vecTotal += m_avecNormalValues[iTexel];
				bTotal = true;
			}

			if (Texel(w-1, h, iTexel))
			{
				vecTotal += m_avecNormalValues[iTexel];
				bTotal = true;
			}

			if (Texel(w-1, h+1, iTexel))
			{
				vecTotal += m_avecNormalValues[iTexel];
				bTotal = true;
			}

			if (Texel(w, h+1, iTexel))
			{
				vecTotal += m_avecNormalValues[iTexel];
				bTotal = true;
			}

			if (Texel(w+1, h+1, iTexel))
			{
				vecTotal += m_avecNormalValues[iTexel];
				bTotal = true;
			}

			if (Texel(w+1, h, iTexel))
			{
				vecTotal += m_avecNormalValues[iTexel];
				bTotal = true;
			}

			if (Texel(w+1, h-1, iTexel))
			{
				vecTotal += m_avecNormalValues[iTexel];
				bTotal = true;
			}

			if (Texel(w, h-1, iTexel))
			{
				vecTotal += m_avecNormalValues[iTexel];
				bTotal = true;
			}

			Texel(w, h, iTexel, false);

			if (bTotal)
			{
				m_avecNormalValues[iTexel] = vecTotal;
				m_avecNormalValues[iTexel].Normalize();
				abPixelMask[iTexel] = true;
			}
		}
	}

	for (size_t p = 0; p < m_iWidth*m_iHeight; p++)
		m_bPixelMask[p] |= abPixelMask[p];

	free(abPixelMask);
}

void CNormalGenerator::TexturizeValues(Vector* avecTexture)
{
	for (size_t x = 0; x < m_iWidth; x++)
	{
		for (size_t y = 0; y < m_iHeight; y++)
		{
			size_t iTexel;

			Texel(x, y, iTexel, false);

			avecTexture[iTexel] = m_avecNormalValues[iTexel].Normalized()*0.99f/2 + Vector(0.5f, 0.5f, 0.5f);
		}
	}
}

size_t CNormalGenerator::GenerateTexture(bool bInMedias)
{
	Vector* avecNormalValues = m_avecNormalValues;

	if (bInMedias)
	{
		// Use this temporary buffer so we don't clobber the original.
		avecNormalValues = m_avecNormalGeneratedValues;
		TexturizeValues(avecNormalValues);
	}

	GLuint iGLId;
	glGenTextures(1, &iGLId);
	glBindTexture(GL_TEXTURE_2D, iGLId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, (GLint)m_iWidth, (GLint)m_iHeight, GL_RGB, GL_FLOAT, &avecNormalValues[0].x);

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

	if (m_aflNormal2Texels)
	{
		if (DoneGenerating())
		{
			size_t iTotalWidth = m_iWidth > m_iNormal2Width ? m_iWidth : m_iNormal2Width;
			size_t iTotalHeight = m_iHeight > m_iNormal2Height ? m_iHeight : m_iNormal2Height;

			Vector* avecResizedNormals = new Vector[iTotalWidth*iTotalHeight];
			Vector* avecResizedNormals2 = new Vector[iTotalWidth*iTotalHeight];

			ILuint iNormalId;
			ilGenImages(1, &iNormalId);
			ilBindImage(iNormalId);
			ilTexImage((ILint)m_iWidth, (ILint)m_iHeight, 1, 3, IL_RGB, IL_FLOAT, &m_avecNormalValues[0].x);
			iluImageParameter(ILU_FILTER, ILU_BILINEAR);
			iluScale((ILint)iTotalWidth, (ILint)iTotalHeight, 1);
			ilCopyPixels(0, 0, 0, (ILint)iTotalWidth, (ILint)iTotalHeight, 3, IL_RGB, IL_FLOAT, &avecResizedNormals[0].x);
			ilDeleteImage(iNormalId);

			ILuint iNormal2Id;
			ilGenImages(1, &iNormal2Id);
			ilBindImage(iNormal2Id);
			ilTexImage((ILint)m_iNormal2Width, (ILint)m_iNormal2Height, 1, 3, IL_RGB, IL_FLOAT, &m_aflNormal2Texels[0]);
			iluImageParameter(ILU_FILTER, ILU_BILINEAR);
			iluScale((ILint)iTotalWidth, (ILint)iTotalHeight, 1);
			ilCopyPixels(0, 0, 0, (ILint)iTotalWidth, (ILint)iTotalHeight, 3, IL_RGB, IL_FLOAT, &avecResizedNormals2[0].x);
			ilDeleteImage(iNormal2Id);

			Vector* avecMergedNormalValues = new Vector[iTotalWidth*iTotalHeight];

			for (size_t i = 0; i < iTotalWidth; i++)
			{
				for (size_t j = 0; j < iTotalHeight; j++)
				{
					size_t iTexel;
					Texel(i, j, iTexel, iTotalWidth, iTotalHeight, false);
					Vector vecNormal = (avecResizedNormals[iTexel]*2 - Vector(1.0f, 1.0f, 1.0f));
					Vector vecNormal2 = (avecResizedNormals2[iTexel]*2 - Vector(1.0f, 1.0f, 1.0f));

					Vector vecBitangent = vecNormal.Cross(Vector(1, 0, 0)).Normalized();
					Vector vecTangent = vecBitangent.Cross(vecNormal).Normalized();

					Matrix4x4 mTBN;
					mTBN.SetColumn(0, vecTangent);
					mTBN.SetColumn(1, vecBitangent);
					mTBN.SetColumn(2, vecNormal);

					avecMergedNormalValues[iTexel] = (mTBN * vecNormal2)*0.99f/2 + Vector(0.5f, 0.5f, 0.5f);
				}
			}

			delete[] avecResizedNormals;
			delete[] avecResizedNormals2;

			ilTexImage((ILint)iTotalWidth, (ILint)iTotalHeight, 1, 3, IL_RGB, IL_FLOAT, &avecMergedNormalValues[0].x);

			delete[] avecMergedNormalValues;
		}
		else
			ilTexImage((ILint)m_iNormal2Width, (ILint)m_iNormal2Height, 1, 3, IL_RGB, IL_FLOAT, &m_aflNormal2Texels[0]);
	}
	else
		ilTexImage((ILint)m_iWidth, (ILint)m_iHeight, 1, 3, IL_RGB, IL_FLOAT, &m_avecNormalValues[0].x);

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

bool CNormalGenerator::Texel(size_t w, size_t h, size_t& iTexel, size_t tw, size_t th, bool* abMask)
{
	if (w < 0 || h < 0 || w >= tw || h >= th)
		return false;

	iTexel = th*h + w;

	assert(iTexel >= 0 && iTexel < tw * th);

	if (abMask && !abMask[iTexel])
		return false;

	return true;
}

bool CNormalGenerator::Texel(size_t w, size_t h, size_t& iTexel, bool bUseMask)
{
	return Texel(w, h, iTexel, m_iWidth, m_iHeight, bUseMask?m_bPixelMask:NULL);
}

typedef struct
{
	CNormalGenerator*	pGenerator;
	size_t				x;
	size_t				y;
} normal2_data_t;

void NormalizeHeightValue(void* pVoidData)
{
	normal2_data_t* pJobData = (normal2_data_t*)pVoidData;

	pJobData->pGenerator->NormalizeHeightValue(pJobData->x, pJobData->y);
}

void CNormalGenerator::NormalizeHeightValue(size_t x, size_t y)
{
	if (!m_aflTextureTexels)
		return;

	float flHiScale = ((m_iNormal2Width+m_iNormal2Height)/2.0f)/200.0f;
	float flLoScale = ((m_iNormal2Width+m_iNormal2Height)/2.0f)/50.0f;

	size_t iTexel;
	Texel(x, y, iTexel, m_iNormal2Width, m_iNormal2Height, false);

	std::vector<Vector> avecHeights;

	float flHeight = (m_aflTextureTexels[iTexel*3]+m_aflTextureTexels[iTexel*3+1]+m_aflTextureTexels[iTexel*3+2])/3 * flHiScale;
	float flLoPass = GetLowPassValue(x, y) * flLoScale;

	Vector vecCenter((float)x, (float)y, flHeight*m_flNormalTextureHiDepth+flLoPass*m_flNormalTextureLoDepth);
	Vector vecNormal(0,0,0);

	if (Texel(x+1, y, iTexel, m_iNormal2Width, m_iNormal2Height, false))
	{
		flHeight = (m_aflTextureTexels[iTexel*3]+m_aflTextureTexels[iTexel*3+1]+m_aflTextureTexels[iTexel*3+2])/3 * flHiScale;
		flLoPass = GetLowPassValue(x+1, y) * flLoScale;
		Vector vecNeighbor(x+1.0f, (float)y, flHeight*m_flNormalTextureHiDepth+flLoPass*m_flNormalTextureLoDepth);
		vecNormal += (vecNeighbor-vecCenter).Normalized().Cross(Vector(0, 1, 0));
	}

	if (Texel(x-1, y, iTexel, m_iNormal2Width, m_iNormal2Height, false))
	{
		flHeight = (m_aflTextureTexels[iTexel*3]+m_aflTextureTexels[iTexel*3+1]+m_aflTextureTexels[iTexel*3+2])/3 * flHiScale;
		flLoPass = GetLowPassValue(x-1, y) * flLoScale;
		Vector vecNeighbor(x-1.0f, (float)y, flHeight*m_flNormalTextureHiDepth+flLoPass*m_flNormalTextureLoDepth);
		vecNormal += (vecNeighbor-vecCenter).Normalized().Cross(Vector(0, -1, 0));
	}

	if (Texel(x, y+1, iTexel, m_iNormal2Width, m_iNormal2Height, false))
	{
		flHeight = (m_aflTextureTexels[iTexel*3]+m_aflTextureTexels[iTexel*3+1]+m_aflTextureTexels[iTexel*3+2])/3 * flHiScale;
		flLoPass = GetLowPassValue(x, y+1) * flLoScale;
		Vector vecNeighbor((float)x, y+1.0f, flHeight*m_flNormalTextureHiDepth+flLoPass*m_flNormalTextureLoDepth);
		vecNormal += (vecNeighbor-vecCenter).Normalized().Cross(Vector(-1, 0, 0));
	}

	if (Texel(x, y-1, iTexel, m_iNormal2Width, m_iNormal2Height, false))
	{
		flHeight = (m_aflTextureTexels[iTexel*3]+m_aflTextureTexels[iTexel*3+1]+m_aflTextureTexels[iTexel*3+2])/3 * flHiScale;
		flLoPass = GetLowPassValue(x, y-1) * flLoScale;
		Vector vecNeighbor((float)x, y-1.0f, flHeight*m_flNormalTextureHiDepth+flLoPass*m_flNormalTextureLoDepth);
		vecNormal += (vecNeighbor-vecCenter).Normalized().Cross(Vector(1, 0, 0));
	}

	vecNormal.Normalize();

	for (size_t i = 0; i < 3; i++)
		vecNormal[i] = RemapVal(vecNormal[i], -1.0f, 1.0f, 0.0f, 0.99f);	// Don't use 1.0 because of integer overflow.

	// Don't need to lock the data because we're guaranteed never to access the same texel twice due to the generation method.
	m_aflNormal2Texels[iTexel*3] = vecNormal.x;
	m_aflNormal2Texels[iTexel*3+1] = vecNormal.y;
	m_aflNormal2Texels[iTexel*3+2] = vecNormal.z;
}

float CNormalGenerator::GetLowPassValue(size_t x, size_t y)
{
	if (x < 0)
		x = 0;

	if (y < 0)
		y = 0;

	if (x >= m_iNormal2Width)
		x = m_iNormal2Width-1;

	if (y >= m_iNormal2Height)
		y = m_iNormal2Height-1;

	size_t iTexel;
	Texel(x, y, iTexel, m_iNormal2Width, m_iNormal2Height);

	bool bGenerate = false;

	m_pNormal2Parallelizer->LockData();
	if (!m_abLowPassMask[iTexel])
		bGenerate = true;
	m_pNormal2Parallelizer->UnlockData();

	if (bGenerate)
	{
		// Generate the low pass value from a fast gaussian filter.

		const float flWeightTable[11][11] = {
			{ 0.004f, 0.004f, 0.005f, 0.005f, 0.005f, 0.005f, 0.005f, 0.005f, 0.005f, 0.004f, 0.004f },
			{ 0.004f, 0.005f, 0.005f, 0.006f, 0.007f, 0.007f, 0.007f, 0.006f, 0.005f, 0.005f, 0.004f },
			{ 0.005f, 0.005f, 0.006f, 0.008f, 0.009f, 0.009f, 0.009f, 0.008f, 0.006f, 0.005f, 0.005f },
			{ 0.005f, 0.006f, 0.008f, 0.010f, 0.012f, 0.014f, 0.012f, 0.010f, 0.008f, 0.006f, 0.005f },
			{ 0.005f, 0.007f, 0.009f, 0.012f, 0.019f, 0.027f, 0.019f, 0.012f, 0.009f, 0.007f, 0.005f },
			{ 0.005f, 0.007f, 0.009f, 0.014f, 0.027f, 0.054f, 0.027f, 0.014f, 0.009f, 0.007f, 0.005f },
			{ 0.005f, 0.007f, 0.009f, 0.012f, 0.019f, 0.027f, 0.019f, 0.012f, 0.009f, 0.007f, 0.005f },
			{ 0.005f, 0.006f, 0.008f, 0.010f, 0.012f, 0.014f, 0.012f, 0.010f, 0.008f, 0.006f, 0.005f },
			{ 0.005f, 0.005f, 0.006f, 0.008f, 0.009f, 0.009f, 0.009f, 0.008f, 0.006f, 0.005f, 0.005f },
			{ 0.004f, 0.005f, 0.005f, 0.006f, 0.007f, 0.007f, 0.007f, 0.006f, 0.005f, 0.005f, 0.004f },
			{ 0.004f, 0.004f, 0.005f, 0.005f, 0.005f, 0.005f, 0.005f, 0.005f, 0.005f, 0.004f, 0.004f },
		};

		float flHeight = 0;
		float flTotalHeight = 0;

		for (int i = -5; i <= 5; i++)
		{
			for (int j = -5; j <= 5; j++)
			{
				size_t iTexel2;
				// *2 is my sneaky hack to make it a radius 10 distribution.
				if (Texel(x+i*2, y+j*2, iTexel2, m_iNormal2Width, m_iNormal2Height))
				{
					flHeight += (m_aflTextureTexels[iTexel2*3]+m_aflTextureTexels[iTexel2*3+1]+m_aflTextureTexels[iTexel2*3+2])/3 * flWeightTable[i+5][j+5];
					flTotalHeight += flWeightTable[i+5][j+5];
				}
			}
		}

		m_pNormal2Parallelizer->LockData();
		m_aflLowPassTexels[iTexel] = flHeight/flTotalHeight;
		m_abLowPassMask[iTexel] = true;
		m_pNormal2Parallelizer->UnlockData();
	}

	return m_aflLowPassTexels[iTexel];
}

void CNormalGenerator::SetNormalTexture(bool bNormalTexture)
{
	// Options:
	// Depth
	// Lo pass
	// Mid pass
	// Hi pass

	if (m_iNormal2GLId)
		glDeleteTextures(1, &m_iNormal2GLId);
	m_iNormal2GLId = 0;

	// Don't let the listeners know yet, we want to generate the new one first so there is no lapse in displaying.
//	m_bNewNormal2Available = true;

	if (!bNormalTexture)
	{
		if (m_pNormal2Parallelizer)
		{
			delete m_pNormal2Parallelizer;
			m_pNormal2Parallelizer = NULL;
		}

		if (m_aflNormal2Texels)
		{
			delete[] m_aflTextureTexels;
			delete[] m_aflLowPassTexels;
			delete[] m_abLowPassMask;
			delete[] m_aflNormal2Texels;
		}
		m_aflTextureTexels = NULL;
		m_aflLowPassTexels = NULL;
		m_abLowPassMask = NULL;
		m_aflNormal2Texels = NULL;

		m_bNewNormal2Available = true;
		return;
	}

	m_bNormal2Generated = false;

	if (m_pNormal2Parallelizer && m_aflNormal2Texels)
	{
		m_pNormal2Parallelizer->RestartJobs();
		return;
	}

	if (m_pNormal2Parallelizer)
		delete m_pNormal2Parallelizer;
	m_pNormal2Parallelizer = new CParallelizer((JobCallback)::NormalizeHeightValue);

	for (size_t iMesh = 0; iMesh < m_apLoRes.size(); iMesh++)
	{
		CConversionMeshInstance* pMeshInstance = m_apLoRes[iMesh];

		for (size_t iMaterialStub = 0; iMaterialStub < pMeshInstance->GetMesh()->GetNumMaterialStubs(); iMaterialStub++)
		{
			size_t iMaterial = pMeshInstance->GetMappedMaterial(iMaterialStub)->m_iMaterial;

			// Materials not loaded yet?
			if (!m_paoMaterials->size())
				continue;

			CMaterial* pMaterial = &(*m_paoMaterials)[iMaterial];

			if (!pMaterial->m_iBase)
				continue;

			glBindTexture(GL_TEXTURE_2D, (GLuint)pMaterial->m_iBase);

			GLint iWidth, iHeight;
			glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &iWidth);
			glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &iHeight);

			m_aflTextureTexels = new float[iWidth*iHeight*3];
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, &m_aflTextureTexels[0]);

			m_iNormal2Width = iWidth;
			m_iNormal2Height = iHeight;

			if (!m_aflLowPassTexels)
				m_aflLowPassTexels = new float[iWidth*iHeight];
			if (!m_abLowPassMask)
				m_abLowPassMask = new bool[iWidth*iHeight];

			memset(m_abLowPassMask, 0, sizeof(bool)*iWidth*iHeight);

			if (!m_aflNormal2Texels)
				m_aflNormal2Texels = new float[iWidth*iHeight*3];

			NormalizeHeightValues(iWidth, iHeight, m_aflTextureTexels, m_aflNormal2Texels);

			break;
		}
	}
}

void CNormalGenerator::RegenerateNormal2Texture()
{
	if (!m_aflNormal2Texels)
		return;

	if (m_iNormal2GLId)
		glDeleteTextures(1, &m_iNormal2GLId);

	GLuint iGLId;
	glGenTextures(1, &iGLId);
	glBindTexture(GL_TEXTURE_2D, iGLId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, (GLint)m_iNormal2Width, (GLint)m_iNormal2Height, GL_RGB, GL_FLOAT, &m_aflNormal2Texels[0]);

	m_iNormal2GLId = iGLId;

	m_bNewNormal2Available = true;
	m_bNormal2Generated = true;
}

void CNormalGenerator::NormalizeHeightValues(size_t w, size_t h, const float* aflTexture, float* aflNormals)
{
	normal2_data_t oJob;
	oJob.pGenerator = this;

	for (size_t x = 0; x < w; x++)
	{
		for (size_t y = 0; y < h; y++)
		{
			oJob.x = x;
			oJob.y = y;
			m_pNormal2Parallelizer->AddJob(&oJob, sizeof(oJob));
		}
	}

	m_pNormal2Parallelizer->Start();
}

bool CNormalGenerator::IsNewNormal2Available()
{
	if (m_pNormal2Parallelizer)
	{
		if (m_pNormal2Parallelizer->AreAllJobsDone() && !m_bNormal2Generated)
		{
			RegenerateNormal2Texture();

			m_bNewNormal2Available = true;
		}
	}

	return m_bNewNormal2Available;
}

bool CNormalGenerator::IsGeneratingNewNormal2()
{
	if (!m_pNormal2Parallelizer)
		return false;

	if (m_pNormal2Parallelizer->AreAllJobsDone())
		return false;

	return true;
}

float CNormalGenerator::GetNormal2GenerationProgress()
{
	if (!m_pNormal2Parallelizer)
		return 0;

	if (m_pNormal2Parallelizer->GetJobsTotal() == 0)
		return 0;

	return (float)m_pNormal2Parallelizer->GetJobsDone() / (float)m_pNormal2Parallelizer->GetJobsTotal();
}

size_t CNormalGenerator::GetNormalMap2()
{
	size_t iNormal2 = m_iNormal2GLId;
	m_iNormal2GLId = 0;
	m_bNewNormal2Available = false;
	return iNormal2;
}
