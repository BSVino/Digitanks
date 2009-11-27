#ifndef CF_RAYTRACE_H
#define CF_RAYTRACE_H

#include <modelconverter/convmesh.h>
#include <geometry.h>

namespace raytrace {

class CRaytracer
{
public:
								CRaytracer(CConversionScene* pScene);

	bool						Raytrace(const Ray& rayTrace, Vector* pvecHit = NULL);

protected:
	CConversionScene*			m_pScene;
};

};

#endif