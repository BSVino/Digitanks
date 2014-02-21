#ifndef DT_PARTICLES_H
#define DT_PARTICLES_H

#include <tvector.h>
#include <vector.h>
#include <color.h>
#include <geometry.h>
#include <game/entities/baseentity.h>
#include <renderer/renderer.h>

class CParticle
{
public:
									CParticle();

public:
	void 							Reset();

public:
	bool							m_bActive;

	Vector							m_vecOrigin;
	Vector							m_vecVelocity;

	EAngle							m_angAngles;
	EAngle							m_angAngleVelocity;

	float							m_flAlpha;
	double							m_flSpawnTime;
	float							m_flRadius;
	float							m_flBillboardYaw;
};

class CSystemInstance
{
public:
									CSystemInstance(class CParticleSystem* pSystem, Vector vecOrigin, EAngle angAngles);
									~CSystemInstance();

public:
	void							SetOrigin(Vector vecOrigin) { m_vecOrigin = vecOrigin; }

	void							Simulate();
	void							SpawnParticle();

	void							Render(class CGameRenderingContext* c);

	void							FollowEntity(CBaseEntity* pFollow);
	void							SetInheritedVelocity(Vector vecInheritedVelocity);

	void							Stop();
	bool							IsStopped();

	size_t							GetNumParticles();

	void							SetColor(Color c);

	CParticleSystem*				GetSystem() { return m_pSystem; }

protected:
	CParticleSystem*				m_pSystem;
	tvector<CSystemInstance*>		m_apChildren;

	Vector							m_vecOrigin;
	Vector							m_vecInheritedVelocity;

	EAngle							m_angAngles;

	bool							m_bStopped;

	tvector<CParticle>				m_aParticles;
	size_t							m_iNumParticlesAlive;

	double							m_flLastEmission;
	int								m_iTotalEmitted;

	CEntityHandle<CBaseEntity>		m_hFollow;

	bool							m_bColorOverride;
	Color							m_clrOverride;
};

class CParticleSystem
{
public:
									CParticleSystem(tstring sName);
									~CParticleSystem();

public:
	bool							IsLoaded() { return m_bLoaded; }
	void							Load();
	void							Unload();

	tstring							GetName() { return m_sName; }

	void							SetMaterialName(tstring sMaterial) { m_sMaterial = sMaterial; };
	void							SetMaterial(const CMaterialHandle& hMaterial) { m_hMaterial = hMaterial; };

	tstring							GetMaterialName() { return m_sMaterial; }
	inline const CMaterialHandle&	GetMaterial() { return m_hMaterial; }

	void							SetModel(tstring sModel) { m_sModel = sModel; };
	void							SetModel(size_t iModel) { m_iModel = iModel; };

	tstring							GetModelName() { return m_sModel; }
	inline size_t					GetModel() { return m_iModel; }

	bool							IsRenderable();

	void							SetBlend(blendtype_t eBlend) { m_eBlend = eBlend; }
	inline blendtype_t				GetBlend() { return m_eBlend; }

	void							SetLifeTime(float flLifeTime) { m_flLifeTime = flLifeTime; }
	inline float					GetLifeTime() { return m_flLifeTime; }

	void							SetEmissionRate(float flEmissionRate) { m_flEmissionRate = flEmissionRate; }
	inline float					GetEmissionRate() { return m_flEmissionRate; }

	void							SetEmissionMax(int iEmissionMax) { m_iEmissionMax = iEmissionMax; }
	inline int						GetEmissionMax() { return m_iEmissionMax; }

	void							SetEmissionMaxDistance(float flDistance) { m_flEmissionMaxDistance = flDistance; }
	float							GetEmissionMaxDistance() { return m_flEmissionMaxDistance; }

	void							SetAlpha(float flAlpha) { m_flAlpha = flAlpha; }
	inline float					GetAlpha() { return m_flAlpha; }

	void							SetColor(const Color& clrColor) { m_clrColor = clrColor; }
	inline Color					GetColor() { return m_clrColor; }

	void							SetRadius(float flRadius) { m_flStartRadius = m_flEndRadius = flRadius; }

	void							SetStartRadius(float flStartRadius) { m_flStartRadius = flStartRadius; }
	inline float					GetStartRadius() { return m_flStartRadius; }

	void							SetEndRadius(float flEndRadius) { m_flEndRadius = flEndRadius; }
	inline float					GetEndRadius() { return m_flEndRadius; }

	void							SetFadeIn(float flFadeIn) { m_flFadeIn = flFadeIn; }
	inline float					GetFadeIn() { return m_flFadeIn; }

	void							SetFadeOut(float flFadeOut) { m_flFadeOut = flFadeOut; }
	inline float					GetFadeOut() { return m_flFadeOut; }

	void							SetSpawnOffset(const Vector& vecSpawnOffset) { m_vecSpawnOffset = vecSpawnOffset; }
	inline Vector					GetSpawnOffset() { return m_vecSpawnOffset; }

	void							SetInheritedVelocity(float flInheritedVelocity) { m_flInheritedVelocity = flInheritedVelocity; }
	inline float					GetInheritedVelocity() { return m_flInheritedVelocity; }

	void							SetRandomVelocity(const AABB& oRandomVelocity) { m_oRandomVelocity = oRandomVelocity; }
	inline AABB						GetRandomVelocity() { return m_oRandomVelocity; }

	void							SetGravity(const Vector& vecGravity) { m_vecGravity = vecGravity; }
	inline Vector					GetGravity() { return m_vecGravity; }

	void							SetDrag(float flDrag) { m_flDrag = flDrag; }
	inline float					GetDrag() { return m_flDrag; }

	void							SetRandomBillboardYaw(bool bYaw) { m_bRandomBillboardYaw = bYaw; }
	bool							GetRandomBillboardYaw() { return m_bRandomBillboardYaw; }

	void							SetRandomModelYaw(bool bYaw) { m_bRandomModelYaw = bYaw; }
	bool							GetRandomModelYaw() { return m_bRandomModelYaw; }

	void							SetRandomModelRoll(bool bRoll) { m_bRandomModelRoll = bRoll; }
	bool							GetRandomModelRoll() { return m_bRandomModelRoll; }

	void							SetRandomAngleVelocity(bool b) { m_bRandomAngleVelocity = b; }
	bool							GetRandomAngleVelocity() { return m_bRandomAngleVelocity; }

	void							AddChild(size_t iSystem);
	void							AddChild(CParticleSystem* pSystem);
	size_t							GetNumChildren() { return m_aiChildren.size(); };
	size_t							GetChild(size_t iChild) { return m_aiChildren[iChild]; };

	void							SetReferences(size_t i) { m_iReferences = i; }
	size_t							GetReferences() { return m_iReferences; }

protected:
	size_t							m_iReferences;

	bool							m_bLoaded;

	tstring							m_sName;

	tstring							m_sMaterial;
	CMaterialHandle					m_hMaterial;

	tstring							m_sModel;
	size_t							m_iModel;

	blendtype_t						m_eBlend;

	float							m_flLifeTime;
	float							m_flEmissionRate;
	int								m_iEmissionMax;
	float							m_flEmissionMaxDistance;
	float							m_flAlpha;
	Color							m_clrColor;
	float							m_flStartRadius;
	float							m_flEndRadius;
	float							m_flFadeIn;
	float							m_flFadeOut;
	Vector							m_vecSpawnOffset;
	float							m_flInheritedVelocity;
	AABB							m_oRandomVelocity;
	Vector							m_vecGravity;
	float							m_flDrag;
	bool							m_bRandomBillboardYaw;
	bool							m_bRandomModelYaw;
	bool							m_bRandomModelRoll;
	bool							m_bRandomAngleVelocity;

	tvector<size_t>					m_aiChildren;
};

class CParticleSystemLibrary
{
	friend class CParticleSystem;

public:
									CParticleSystemLibrary();
									~CParticleSystemLibrary();

public:
	static size_t					GetNumParticleSystems() { return Get()->m_apParticleSystems.size(); };
	static size_t					GetNumParticleSystemsLoaded() { return Get()->m_iParticleSystemsLoaded; };

	size_t							AddParticleSystem(const tstring& sName);
	size_t							FindParticleSystem(const tstring& sName);
	void							LoadParticleSystem(size_t iSystem);
	CParticleSystem*				GetParticleSystem(size_t i);

public:
	static void						Simulate();
	static void						Render();

	static size_t					AddInstance(const tstring& sName, Vector vecOrigin, EAngle angAngles=EAngle(0, 0, 0));
	static size_t					AddInstance(size_t iParticleSystem, Vector vecOrigin, EAngle angAngles=EAngle(0, 0, 0));
	static void						StopInstance(size_t iInstance);
	static void						StopInstances(const tstring& sName);
	static void						RemoveInstance(size_t iInstance);
	static CSystemInstance*			GetInstance(size_t iInstance);

	static void						ClearInstances();

	static void						ReloadSystems();

	static void						ResetReferenceCounts();
	static void						ClearUnreferenced();

	static CParticleSystemLibrary*	Get() { return s_pParticleSystemLibrary; };

private:
	static void						InitSystems();

protected:
	tvector<CParticleSystem*>		m_apParticleSystems;
	size_t							m_iParticleSystemsLoaded;
	tmap<size_t, CSystemInstance*>	m_apInstances;
	size_t							m_iSystemInstanceIndex;

private:
	static CParticleSystemLibrary*	s_pParticleSystemLibrary;
};

class CParticleSystemInstanceHandle
{
public:
	CParticleSystemInstanceHandle();
	~CParticleSystemInstanceHandle();

public:
	void							SetSystem(const tstring& sSystem, Vector vecOrigin);
	void							SetSystem(size_t iSystem, Vector vecOrigin);

	void							FollowEntity(CBaseEntity* pFollow) { m_hFollow = pFollow; }

	void							SetActive(bool bActive);
	bool							IsActive() { return m_iInstance != ~0; };

	size_t							GetInstance() { return m_iInstance; }

protected:
	size_t							m_iSystem;
	size_t							m_iInstance;
	CEntityHandle<CBaseEntity>		m_hFollow;
	Vector							m_vecOrigin;
};

#endif
