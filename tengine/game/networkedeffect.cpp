#include "networkedeffect.h"

#include <network/network.h>
#include <network/commands.h>
#include <tinker/application.h>
#include <renderer/particles.h>

// A networked particle effect. Call it on the server and it will show up on all clients.
SERVER_GAME_COMMAND(NetworkedEffect)
{
	if (pCmd->GetNumArguments() < 7)
	{
		TMsg("NetworkedEffect with less than 7 parameters.\n");
		return;
	}

	tstring sName(pCmd->Arg(0));
	Vector vecPosition(pCmd->ArgAsFloat(1), pCmd->ArgAsFloat(2), pCmd->ArgAsFloat(3));
	EAngle angDirection(pCmd->ArgAsFloat(4), pCmd->ArgAsFloat(5), pCmd->ArgAsFloat(6));

	CParticleSystemLibrary::AddInstance(sName, vecPosition, angDirection);
}

void CNetworkedEffect::AddInstance(const tstring& sName, Vector vecPosition, EAngle angDirection)
{
	::NetworkedEffect.RunCommand(sName + tsprintf(" %f %f %f %f %f %f", vecPosition.x, vecPosition.y, vecPosition.z, angDirection.p, angDirection.y, angDirection.r));
}
