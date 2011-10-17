#include <stdio.h>
#include <string.h>

#include "../modelconverter.h"
#include "strutils.h"

void CModelConverter::WriteSMDs(const tstring& sFilename)
{
	for (size_t i = 0; i < m_pScene->GetNumMeshes(); i++)
		WriteSMD(i, sFilename);
}

void CModelConverter::WriteSMD(size_t iMesh, const tstring& sFilename)
{
	CConversionMesh* pMesh = m_pScene->GetMesh(iMesh);

	tstring sFile;
	if (sFilename.length())
	{
		sFile.append(sFilename);
		sFile.append(_T("_"));
	}
	sFile.append(pMesh->GetBoneName(0));

	sFile = GetFilename(sFile.c_str());

	sFile.append(_T(".smd"));

	FILE* fp = tfopen(sFile, _T("w"));

	// SMD file format: http://developer.valvesoftware.com/wiki/SMD

	// Header section
	fprintf(fp, "version 1\n\n");

	// Nodes section
	fprintf(fp, "nodes\n");
	// Only bothering with one node, we're only doing static props with this code for now.
	fprintf(fp, "0 \"%s\" -1\n", convertstring<tchar, char>(pMesh->GetBoneName(0)).c_str());
	fprintf(fp, "end\n\n");

	// Skeleton section
	fprintf(fp, "skeleton\n");
	fprintf(fp, "time 0\n");
	fprintf(fp, "0 0.000000 0.000000 0.000000 1.570796 0.000000 0.0000001\n");
	fprintf(fp, "end\n\n");
	
	fprintf(fp, "triangles\n");
	for (size_t i = 0; i < pMesh->GetNumFaces(); i++)
	{
		CConversionFace* pFace = pMesh->GetFace(i);

		if (!pFace->GetNumVertices())
		{
			printf("WARNING! Found a face with no vertices.\n");
			continue;
		}

		CConversionVertex* pV1 = pFace->GetVertex(0);

		for (size_t j = 0; j < pFace->GetNumVertices()-2; j++)
		{
			CConversionVertex* pV2 = pFace->GetVertex(j+1);
			CConversionVertex* pV3 = pFace->GetVertex(j+2);

			Vector v1 = pMesh->GetVertex(pV1->v);
			Vector v2 = pMesh->GetVertex(pV2->v);
			Vector v3 = pMesh->GetVertex(pV3->v);

			Vector n1 = pMesh->GetNormal(pV1->vn);
			Vector n2 = pMesh->GetNormal(pV2->vn);
			Vector n3 = pMesh->GetNormal(pV3->vn);

			Vector uv1 = pMesh->GetUV(pV1->vu);
			Vector uv2 = pMesh->GetUV(pV2->vu);
			Vector uv3 = pMesh->GetUV(pV3->vu);

			// Material name
			size_t iMaterial = pFace->m;

			if (iMaterial == ((size_t)~0) || !m_pScene->GetMaterial(iMaterial))
			{
				printf("ERROR! Can't find a material for a triangle.\n");
				fprintf(fp, "error\n");
			}
			else
				fprintf(fp, "%s\n", convertstring<tchar, char>(m_pScene->GetMaterial(iMaterial)->GetName()).c_str());

			// <int|Parent bone> <float|PosX PosY PosZ> <normal|NormX NormY NormZ> <normal|U V>
			// Studio coordinates are not the same as game coordinates. Studio (x, y, z) is game (x, -z, y) and vice versa.
			fprintf(fp, "0 \t %f %f %f \t %f %f %f \t %f %f\n", v1.x, -v1.z, v1.y, n1.x, -n1.z, n1.y, uv1.x, uv1.y);
			fprintf(fp, "0 \t %f %f %f \t %f %f %f \t %f %f\n", v2.x, -v2.z, v2.y, n2.x, -n2.z, n2.y, uv2.x, uv2.y);
			fprintf(fp, "0 \t %f %f %f \t %f %f %f \t %f %f\n", v3.x, -v3.z, v3.y, n3.x, -n3.z, n3.y, uv3.x, uv3.y);
		}
	}
	fprintf(fp, "end\n");

	fclose(fp);
}
