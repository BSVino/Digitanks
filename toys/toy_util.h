#pragma once

#include <tvector.h>
#include <tstring.h>
#include <vector.h>
#include <geometry.h>
#include <trs.h>

class CToy;

class CSceneArea
{
public:
	AABB					m_aabbArea;
	tvector<uint32_t>		m_aiNeighboringAreas;
	tvector<uint32_t>		m_aiVisibleAreas;
	tstring					m_sFileName;
};

// For reading and writing .toy files
class CToyUtil
{
public:
					CToyUtil();

public:
	tstring			GetGameDirectory() const { return m_sGameDirectory; }
	void			SetGameDirectory(const tstring& sGame) { m_sGameDirectory = sGame; }
	tstring			GetGameDirectoryFile(const tstring& sFile) const;

	tstring			GetScriptDirectory() const { return m_sScriptDirectory; }
	void			SetScriptDirectory(const tstring& sScript) { m_sScriptDirectory = sScript; }
	tstring			GetScriptDirectoryFile(const tstring& sFile) const;

	tstring			GetOutputDirectory() const { return m_sOutputDirectory; }
	void			SetOutputDirectory(const tstring& sOutput) { m_sOutputDirectory = sOutput; }

	tstring			GetOutputFile() const { return m_sOutputFile; }
	void			SetOutputFile(const tstring& sOutput) { m_sOutputFile = sOutput; }

	void			AddMaterial(const tstring& sMaterial, const tstring& sOriginalFile = "");
	size_t			GetNumMaterials() { return m_asMaterials.size(); }
	void            UseUV(bool bUse = true);
	void            UseNormals(bool bUse = true);
	void            AddVertex(size_t iMaterial, const Vector& vecPosition, const Vector2D& vecUV, const Matrix4x4& mNormals = Matrix4x4());
	size_t          GetNumVerts() const;
	bool            IsUsingUV() const { return m_bUseUV; }
	bool            IsUsingNormals() const { return m_bUseNormals; }

	size_t          GetVertexSizeInBytes() const;
	int             GetVertexUVOffsetInBytes() const;
	int             GetVertexNormalOffsetInBytes() const;
	int             GetVertexTangentOffsetInBytes() const;
	int             GetVertexBitangentOffsetInBytes() const;

	void			AddPhysTriangle(size_t v1, size_t v2, size_t v3);
	void			AddPhysVertex(Vector vecPosition);
	size_t			GetNumPhysIndices() { return m_aiPhysIndices.size(); }
	size_t			GetNumPhysVerts() { return m_avecPhysVerts.size(); }

	// Shapes
	void			AddPhysBox(const TRS& trsBox);
	size_t			GetNumPhysBoxes() { return m_atrsPhysBoxes.size(); }

	size_t			AddSceneArea(const tstring& sFileName);
	void			AddSceneAreaNeighbor(size_t iSceneArea, size_t iNeighbor);
	void			AddSceneAreaVisible(size_t iSceneArea, size_t iVisible);
	bool			IsVisibleFrom(size_t iSceneArea, size_t iVisible);
	size_t			GetNumSceneAreas() { return m_asSceneAreas.size(); }

	AABB			GetVisBounds() { return m_aabbVisBounds; };
	AABB			GetPhysBounds() { return m_aabbPhysBounds; };

	void			SetNeighborDistance(float flDistance) { m_flNeighborDistance = flDistance; };
	void			UseGlobalTransformations(bool bGlobal = true) { m_bUseLocalTransforms = !bGlobal; };
	void			UseLocalTransformations(bool bLocal = true) { m_bUseLocalTransforms = bLocal; };
	bool			IsUsingLocalTransformations() const { return m_bUseLocalTransforms; };

	void			AddVisibleNeighbors(size_t iArea, size_t iVisible);
	void			CalculateVisibleAreas();
	bool			Write(const tstring& sFileName);

	static bool		Read(const tstring& sFileName, CToy* pToy);

protected:
	tstring					m_sGameDirectory;
	tstring					m_sScriptDirectory;
	tstring					m_sOutputDirectory;
	tstring					m_sOutputFile;

	bool m_bUseUV;
	bool m_bUseNormals;

	tvector<tstring>			m_asMaterials;
	//tvector<tstring>			m_asCopyTextures;
	tvector<tvector<float> >	m_aaflData;
	AABB						m_aabbVisBounds;
	AABB						m_aabbPhysBounds;
	tvector<uint32_t>			m_aiPhysIndices;
	tvector<Vector>				m_avecPhysVerts;
	tvector<TRS>				m_atrsPhysBoxes;
	tvector<CSceneArea>			m_asSceneAreas;

	float					m_flNeighborDistance;	// How far can an area be considered a neighbor in the finding neighbors check?
	bool					m_bUseLocalTransforms;
};
