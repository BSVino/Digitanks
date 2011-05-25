#ifndef DT_PARTICLES_H
#define DT_PARTICLES_H

#include <EASTL/vector.h>
#include <vector.h>
#include <color.h>
#include <geometry.h>
#include <game/baseentity.h>

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

	float							m_flAlpha;
	float							m_flSpawnTime;
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

	void							Render(class CRenderingContext* c);

	void							FollowEntity(CBaseEntity* pFollow);
	void							SetInheritedVelocity(Vector vecInheritedVelocity);

	void							Stop();
	bool							IsStopped();

	size_t							GetNumParticles();

	void							SetColor(Color c);

	CParticleSystem*				GetSystem() { return m_pSystem; }

protected:
	CParticleSystem*				m_pSystem;
	eastl::vector<CSystemInstance*>	m_apChildren;

	Vector							m_vecOrigin;
	Vector							m_vecInheritedVelocity;

	EAngle							m_angAngles;

	bool							m_bStopped;

	eastl::vector<CParticle>		m_aParticles;
	size_t							m_iNumParticlesAlive;

	float							m_flLastEmission;
	int								m_iTotalEmitted;

	CEntityHandle<CBaseEntity>		m_hFollow;

	bool							m_bColorOverride;
	Color							m_clrOverride;
};

class CParticleSystem
{
public:
									CParticleSystem(eastl::string16 sName);

public:
	bool							IsLoaded() { return m_bLoaded; }
	void							Load();

	eastl::string16					GetName() { return m_sName; }

	void							SetTexture(eastl::string16 sTexture) { m_sTexture = sTexture; };
	void							SetTexture(size_t iTexture) { m_iTexture = iTexture; };

	eastl::string16					GetTextureName() { return m_sTexture; }
	inline size_t					GetTexture() { return m_iTexture; }

	void							SetModel(eastl::string16 sModel) { m_sModel = sModel; };
	void							SetModel(size_t iModel) { m_iModel = iModel; };

	eastl::string16					GetModelName() { return m_sModel; }
	inline size_t					GetModel() { return m_iModel; }

	bool							IsRenderable();

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

	void							AddChild(size_t iSystem);
	void							AddChild(CParticleSystem* pSystem);
	size_t							GetNumChildren() { return m_aiChildren.size(); };
	size_t							GetChild(size_t iChild) { return m_aiChildren[iChild]; };

protected:
	bool							m_bLoaded;

	eastl::string16					m_sName;

	eastl::string16					m_sTexture;
	size_t							m_iTexture;

	eastl::string16					m_sModel;
	size_t							m_iModel;

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

	eastl::vector<size_t>			m_aiChildren;
};

class CParticleSystemLibrary
{
public:
									CParticleSystemLibrary();
									~CParticleSystemLibrary();

public:
	static size_t					GetNumParticleSystems() { return Get()->m_apParticleSystems.size(); };

	size_t							AddParticleSystem(const eastl::string16& sName);
	size_t							FindParticleSystem(const eastl::string16& sName);
	void							LoadParticleSystem(size_t iSystem);
	CParticleSystem*				GetParticleSystem(size_t i);

public:
	static void						Simulate();
	static void						Render();

	static size_t					AddInstance(const eastl::string16& sName, Vector vecOrigin, EAngle angAngles=EAngle(0, 0, 0));
	static size_t					AddInstance(size_t iParticleSystem, Vector vecOrigin, EAngle angAngles=EAngle(0, 0, 0));
	static void						StopInstance(size_t iInstance);
	static void						StopInstances(const eastl::string16& sName);
	static void						RemoveInstance(size_t iInstance);
	static CSystemInstance*			GetInstance(size_t iInstance);

	static void						ClearInstances();

	static void						ReloadSystems();

	static CParticleSystemLibrary*	Get() { return s_pParticleSystemLibrary; };

private:
	static void						InitSystems();

protected:
	eastl::vector<CParticleSystem*>	m_apParticleSystems;
	eastl::map<size_t, CSystemInstance*>	m_apInstances;
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
	void							SetSystem(const eastl::string16& sSystem, Vector vecOrigin);
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
