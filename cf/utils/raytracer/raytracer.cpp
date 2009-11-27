#include "raytracer.h"

using namespace raytrace;

CRaytracer::CRaytracer(CConversionScene* pScene)
{
	m_pScene = pScene;
}

bool CRaytracer::Raytrace(const Ray& rayTrace, Vector* pvecHit)
{
	// Brute force method.
	Vector vecClosest;
	bool bFound = false;

	for (size_t m = 0; m < m_pScene->GetNumMeshes(); m++)
	{
		CConversionMesh* pMesh = m_pScene->GetMesh(m);
		for (size_t f = 0; f < pMesh->GetNumFaces(); f++)
		{
			CConversionFace* pFace = pMesh->GetFace(f);
			for (size_t t = 0; t < pFace->GetNumVertices()-2; t++)
			{
				CConversionVertex* pV1 = pFace->GetVertex(0);
				CConversionVertex* pV2 = pFace->GetVertex(t+1);
				CConversionVertex* pV3 = pFace->GetVertex(t+2);

				Vector v1 = pMesh->GetVertex(pV1->v);
				Vector v2 = pMesh->GetVertex(pV2->v);
				Vector v3 = pMesh->GetVertex(pV3->v);

				Vector vecHit;
				if (rayTrace.IntersectTriangle(v1, v2, v3, &vecHit))
				{
					if (!bFound || (vecHit - rayTrace.m_vecPos).LengthSqr() < (vecClosest - rayTrace.m_vecPos).LengthSqr())
					{
						bFound = true;
						vecClosest = vecHit;
					}
				}
			}
		}
	}

	if (bFound)
	{
		if (pvecHit)
			*pvecHit = vecClosest;
		return true;
	}

	return false;
}