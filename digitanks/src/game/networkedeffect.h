#ifndef NETWORKEDEFFECT_H
#define NETWORKEDEFFECT_H

#include <EASTL/string.h>
#include <vector.h>

class CNetworkedEffect
{
public:
	static void AddInstance(const tstring& sName, Vector vecPosition, EAngle angDirection = EAngle());
};

#endif
