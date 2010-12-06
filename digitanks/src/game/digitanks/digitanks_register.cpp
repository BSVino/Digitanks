#include "digitanksgame.h"
#include "structures/buffer.h"
#include "structures/collector.h"
#include "structures/cpu.h"
#include "structures/loader.h"
#include "units/standardtank.h"
#include "units/artillery.h"
#include "units/maintank.h"
#include "units/mechinf.h"
#include "units/scout.h"
#include "digitanksteam.h"
#include "powerup.h"
#include "weapons/baseweapon.h"
#include "weapons/projectile.h"
#include "weapons/specialshells.h"
#include "weapons/cameraguided.h"
#include "weapons/laser.h"
#include "weapons/missiledefense.h"
#include "terrain.h"
#include "updates.h"
#include "structures/props.h"
#include "menumarcher.h"

#include "registered_entities.h"

REGISTER_ENTITY(CDigitanksGame);
REGISTER_ENTITY(CDigitanksEntity);
REGISTER_ENTITY(CMiniBuffer);
REGISTER_ENTITY(CBuffer);
REGISTER_ENTITY(CBattery);
REGISTER_ENTITY(CCollector);
REGISTER_ENTITY(CCPU);
REGISTER_ENTITY(CLoader);
REGISTER_ENTITY(CResource);
REGISTER_ENTITY(CStructure);
REGISTER_ENTITY(CSupplier);
REGISTER_ENTITY(CArtillery);
REGISTER_ENTITY(CDigitank);
REGISTER_ENTITY(CStandardTank);
REGISTER_ENTITY(CMainBattleTank);
REGISTER_ENTITY(CMechInfantry);
REGISTER_ENTITY(CScout);
REGISTER_ENTITY(CDigitanksTeam);
REGISTER_ENTITY(CPowerup);
REGISTER_ENTITY(CBaseWeapon);
REGISTER_ENTITY(CProjectile);
REGISTER_ENTITY(CSmallShell);
REGISTER_ENTITY(CMediumShell);
REGISTER_ENTITY(CLargeShell);
REGISTER_ENTITY(CAOEShell);
REGISTER_ENTITY(CEMP);
REGISTER_ENTITY(CICBM);
REGISTER_ENTITY(CGrenade);
REGISTER_ENTITY(CDaisyChain);
REGISTER_ENTITY(CClusterBomb);
REGISTER_ENTITY(CEarthshaker);
REGISTER_ENTITY(CTractorBomb);
REGISTER_ENTITY(CSploogeShell);
REGISTER_ENTITY(CArtilleryShell);
REGISTER_ENTITY(CInfantryFlak);
REGISTER_ENTITY(CTorpedo);
REGISTER_ENTITY(CAirstrikeShell);
REGISTER_ENTITY(CCameraGuidedMissile);
REGISTER_ENTITY(CLaser);
REGISTER_ENTITY(CMissileDefense);
REGISTER_ENTITY(CSelectable);
REGISTER_ENTITY(CSupplyLine);
REGISTER_ENTITY(CTerrain);
REGISTER_ENTITY(CUpdateGrid);
REGISTER_ENTITY(CStaticProp);
REGISTER_ENTITY(CMenuMarcher);
REGISTER_ENTITY(CFireworks);
