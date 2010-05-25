#ifndef DT_PARTICLES_H

#include <vector>
#include <vector.h>
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

	float							m_flAlpha;
	float							m_flSpawnTime;
	float							m_flRadius;
	float							m_flBillboardYaw;
};

class CSystemInstance
{
public:
									CSystemInstance(class CParticleSystem* pSystem, Vector vecOrigin);

public:
	void							Simulate();
	void							SpawnParticle();

	void							Render();

	void							FollowEntity(CBaseEntity* pFollow);

	void							Stop() { m_bStopped = true; };
	bool							IsStopped() { return m_bStopped; };

	size_t							GetNumParticles() { return m_iNumParticlesAlive; };

protected:
	CParticleSystem*				m_pSystem;
	Vector							m_vecOrigin;
	Vector							m_vecInheritedVelocity;

	bool							m_bStopped;

	std::vector<CParticle>			m_aParticles;
	size_t							m_iNumParticlesAlive;

	float							m_flLastEmission;

	CEntityHandle<CBaseEntity>		m_hFollow;
};

class CParticleSystem
{
public:
									CParticleSystem(std::wstring sName);

public:
	bool							IsLoaded() { return m_bLoaded; }
	void							Load();

	const wchar_t*					GetName() { return m_sName.c_str(); }

	void							SetTexture(std::wstring sTexture) { m_sTexture = sTexture; };
	void							SetTexture(size_t iTexture) { m_iTexture = iTexture; };

	const wchar_t*					GetTextureName() { return m_sTexture.c_str(); }
	inline size_t					GetTexture() { return m_iTexture; }

	void							SetLifeTime(float flLifeTime) { m_flLifeTime = flLifeTime; }
	inline float					GetLifeTime() { return m_flLifeTime; }

	void							SetEmissionRate(float flEmissionRate) { m_flEmissionRate = flEmissionRate; }
	inline float					GetEmissionRate() { return m_flEmissionRate; }

	void							SetAlpha(float flAlpha) { m_flAlpha = flAlpha; }
	inline float					GetAlpha() { return m_flAlpha; }

	void							SetRadius(float flRadius) { m_flStartRadius = m_flEndRadius = flRadius; }

	void							SetStartRadius(float flStartRadius) { m_flStartRadius = flStartRadius; }
	inline float					GetStartRadius() { return m_flStartRadius; }

	void							SetEndRadius(float flEndRadius) { m_flEndRadius = flEndRadius; }
	inline float					GetEndRadius() { return m_flEndRadius; }

	void							SetFadeOut(float flFadeOut) { m_flFadeOut = flFadeOut; }
	inline float					GetFadeOut() { return m_flFadeOut; }

	void							SetInheritedVelocity(float flInheritedVelocity) { m_flInheritedVelocity = flInheritedVelocity; }
	inline float					GetInheritedVelocity() { return m_flInheritedVelocity; }

	void							SetRandomVelocity(const AABB& oRandomVelocity) { m_oRandomVelocity = oRandomVelocity; }
	inline AABB						GetRandomVelocity() { return m_oRandomVelocity; }

	void							SetRandomBillboardYaw(bool bYaw) { m_bRandomBillboardYaw = bYaw; }
	bool							GetRandomBillboardYaw() { return m_bRandomBillboardYaw; }

protected:
	bool							m_bLoaded;

	std::wstring					m_sName;

	std::wstring					m_sTexture;
	size_t							m_iTexture;

	float							m_flLifeTime;
	float							m_flEmissionRate;
	float							m_flAlpha;
	float							m_flStartRadius;
	float							m_flEndRadius;
	float							m_flFadeOut;
	float							m_flInheritedVelocity;
	AABB							m_oRandomVelocity;
	bool							m_bRandomBillboardYaw;
};

class CParticleSystemLibrary
{
public:
									CParticleSystemLibrary();
									~CParticleSystemLibrary();

public:
	size_t							GetNumParticleSystems() { return m_apParticleSystems.size(); };

	size_t							AddParticleSystem(const std::wstring& sName);
	size_t							FindParticleSystem(const std::wstring& sName);
	void							LoadParticleSystem(size_t iSystem);
	CParticleSystem*				GetParticleSystem(size_t i);

public:
	static void						Simulate();
	static void						Render();

	static size_t					AddInstance(size_t iParticleSystem, Vector vecOrigin);
	static void						StopInstance(size_t iInstance);
	static void						RemoveInstance(size_t iInstance);
	static CSystemInstance*			GetInstance(size_t iInstance);

	static CParticleSystemLibrary*	Get() { return s_pParticleSystemLibrary; };

private:
	static void						InitSystems();

protected:
	std::vector<CParticleSystem*>	m_apParticleSystems;
	std::map<size_t, CSystemInstance*>	m_apInstances;
	size_t							m_iSystemInstanceIndex;

private:
	static CParticleSystemLibrary*	s_pParticleSystemLibrary;
};

#endif
