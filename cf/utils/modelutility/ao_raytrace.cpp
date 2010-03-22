#include "crunch.h"

#include <list>
#include <pthread.h>
#include <time.h>

#include <raytracer/raytracer.h>
#include <platform.h>

pthread_mutex_t g_iDataMutex;

void CAOGenerator::RaytraceSceneFromPosition(raytrace::CRaytracer* pTracer, Vector vecUVPosition, Vector vecNormal, CConversionMeshInstance* pMeshInstance, CConversionFace* pFace, size_t iTexel)
{
	// Build rotation matrix
	Matrix4x4 m;
	m.SetOrientation(vecNormal);

	// Turn it sideways so that pitch 90 is up, for more uniform sampling
	Matrix4x4 m2;
	m2.SetRotation(EAngle(0, -90, 0));

	m *= m2;

	float flHits = 0;
	float flTotalHits = 0;

	for (size_t x = 0; x < m_iSamples/2; x++)
	{
		float flRandom = 0;
		if (m_bRandomize)
			flRandom = RemapVal((float)(rand()%10000), 0, 10000.0f, -0.5, 0.5);

		float flPitch = RemapVal(cos(RemapVal((float)x+flRandom, 0, (float)m_iSamples/2, 0, M_PI/2)), 0, 1, 90, 0);

		float flWeight = sin(flPitch * M_PI/180);

		for (size_t y = 0; y <= m_iSamples; y++)
		{
			flRandom = 0;
			if (m_bRandomize)
				flRandom = RemapVal((float)(rand()%10000), 0, 10000.0f, -0.5, 0.5);

			float flYaw = RemapVal((float)y+flRandom, 0, (float)m_iSamples, -180, 180);

			Vector vecDir = AngleVector(EAngle(flPitch, flYaw, 0));

			// Transform relative to the triangle's normal
			Vector vecRay = m * vecDir;

			//RenderSceneFromPosition(vecUVPosition, vecRay, pFace);

			flTotalHits += flWeight;

			raytrace::CTraceResult tr;
			if (pTracer->Raytrace(Ray(vecUVPosition + pFace->GetNormal()*0.01f, vecRay), &tr))
			{
				float flDistance = (tr.m_vecHit - vecUVPosition).Length();
				if (m_flRayFalloff < 0)
					flHits += flWeight;
				else
					flHits += flWeight * (1/pow(2, flDistance/m_flRayFalloff));
			}
			else if (m_bGroundOcclusion && vecRay.y < 0)
			{
				// The following math is basically a plane-ray intersection algorithm,
				// with shortcuts made for the assumption of an infinite plane facing straight up.

				Vector n = Vector(0,1,0);

				float a = -(vecUVPosition.y - pMeshInstance->m_pParent->m_oExtends.m_vecMins.y);
				float b = vecRay.y;

				float flDistance = a/b;

				if (flDistance < 1e-4f || m_flRayFalloff < 0)
					flHits += flWeight;
				else
					flHits += flWeight * (1/pow(2, flDistance/m_flRayFalloff));
			}
		}
	}

	// One last ray directly up, it is skipped in the above loop so it's not done 10 times.
	Vector vecDir = AngleVector(EAngle(90, 0, 0));

	// Transform relative to the triangle's normal
	Vector vecRay = m * vecDir;

	//RenderSceneFromPosition(vecUVPosition, vecRay, pFace);

	flTotalHits++;

	raytrace::CTraceResult tr;
	if (pTracer->Raytrace(Ray(vecUVPosition + pFace->GetNormal()*0.01f, vecRay), &tr))
	{
		float flDistance = (tr.m_vecHit - vecUVPosition).Length();
		if (m_flRayFalloff < 0)
			flHits += 1;
		else
			flHits += (1/pow(2, flDistance/m_flRayFalloff));
	}
	else if (m_bGroundOcclusion && vecRay.y < 0)
	{
		// The following math is basically a plane-ray intersection algorithm,
		// with shortcuts made for the assumption of an infinite plane facing straight up.

		Vector n = Vector(0,1,0);

		float a = -(vecUVPosition.y - pMeshInstance->m_pParent->m_oExtends.m_vecMins.y);
		float b = vecRay.y;

		float flDistance = a/b;

		if (flDistance < 1e-4f || m_flRayFalloff < 0)
			flHits += 1;
		else
			flHits += (1/pow(2, flDistance/m_flRayFalloff));
	}

	float flShadowValue = 1 - ((float)flHits / (float)flTotalHits);

	// Mutex may be dead, try to bail before.
	if (m_bStopGenerating)
		return;

	if (GetNumberOfProcessors() > 1)
	{
		// Keep all locking and unlocking in one branch to prevent processor prediction miss problems.
		// I saw it in some presentation somewhere.
		pthread_mutex_lock(&g_iDataMutex);
		m_avecShadowValues[iTexel] += Vector(flShadowValue, flShadowValue, flShadowValue);
		pthread_mutex_unlock(&g_iDataMutex);
	}
	else
		m_avecShadowValues[iTexel] += Vector(flShadowValue, flShadowValue, flShadowValue);
}

typedef struct
{
	raytrace::CRaytracer*		pTracer;
	Vector						vecUVPosition;
	Vector						vecNormal;
	CConversionMeshInstance*	pMeshInstance;
	CConversionFace*			pFace;
	size_t						iTexel;
} thread_job_t;

typedef struct
{
	pthread_t					iThread;
	CAOGenerator*				pGenerator;
	bool						bQuitWhenDone;
	bool						bDone;
} thread_data_t;

std::vector<thread_data_t> g_aThreads;
std::list<thread_job_t> g_lJobs;
pthread_mutex_t g_iJobsMutex;
size_t g_iJobsGiven = 0;

void RaytraceThreadMain(void* pData)
{
	thread_data_t* pThread = (thread_data_t*)pData;
	while (true)
	{
		if (pThread->pGenerator->IsStopped())
		{
			pThread->bDone = true;
			pthread_exit(NULL);
			return;
		}

		pthread_mutex_lock(&g_iJobsMutex);
		if (!g_lJobs.size())
		{
			pthread_mutex_unlock(&g_iJobsMutex);

			if (pThread->bQuitWhenDone)
			{
				pThread->bDone = true;
				pthread_exit(NULL);
				return;
			}
			else
			{
				// Give the main thread a chance to render and whatnot.
				SleepMS(1);
				continue;
			}
		}

		// Copy so we can pop it.
		thread_job_t oThreadJob = g_lJobs.front();
		g_lJobs.pop_front();

		pthread_mutex_unlock(&g_iJobsMutex);

		pThread->pGenerator->RaytraceSceneFromPosition(oThreadJob.pTracer, oThreadJob.vecUVPosition, oThreadJob.vecNormal, oThreadJob.pMeshInstance, oThreadJob.pFace, oThreadJob.iTexel);

		// Give the main thread a chance to render and whatnot.
		SleepMS(1);
	}
}

void CAOGenerator::RaytraceSetupThreads()
{
	if (GetNumberOfProcessors() == 1)
		return;

	pthread_mutex_init(&g_iDataMutex, NULL);
	pthread_mutex_init(&g_iJobsMutex, NULL);

	g_iJobsGiven = 0;

	// Insert all first so that reallocations are all done before we pass the pointers to the threads.
	g_aThreads.insert(g_aThreads.begin(), GetNumberOfProcessors(), thread_data_t());

	for (size_t i = 0; i < GetNumberOfProcessors(); i++)
	{
		thread_data_t* pThread = &g_aThreads[i];

		pThread->pGenerator = this;
		pThread->bQuitWhenDone = false;
		pThread->bDone = false;

		pthread_create(&pThread->iThread, NULL, (void *(*) (void *))&RaytraceThreadMain, (void*)pThread);
	}
}

void CAOGenerator::RaytraceCleanupThreads()
{
	if (GetNumberOfProcessors() == 1)
		return;

	// If there are no threads then we've already cleaned up, don't do it twice.
	if (!g_aThreads.size())
		return;

	for (size_t i = 0; i < g_aThreads.size(); i++)
	{
		thread_data_t* pThread = &g_aThreads[i];

		pthread_detach(pThread->iThread);
	}

	pthread_mutex_destroy(&g_iDataMutex);
	pthread_mutex_destroy(&g_iJobsMutex);

	g_aThreads.clear();
	g_lJobs.clear();
}

void CAOGenerator::RaytraceSceneMultithreaded(raytrace::CRaytracer* pTracer, Vector vecUVPosition, Vector vecNormal, CConversionMeshInstance* pMeshInstance, CConversionFace* pFace, size_t iTexel)
{
	if (GetNumberOfProcessors() == 1)
	{
		RaytraceSceneFromPosition(pTracer, vecUVPosition, vecNormal, pMeshInstance, pFace, iTexel);
		return;
	}

	pthread_mutex_lock(&g_iJobsMutex);

	g_lJobs.push_back(thread_job_t());
	thread_job_t* pJob = &g_lJobs.back();
	pJob->pTracer = pTracer;
	pJob->vecUVPosition = vecUVPosition;
	pJob->vecNormal = vecNormal;
	pJob->pMeshInstance = pMeshInstance;
	pJob->pFace = pFace;
	pJob->iTexel = iTexel;

	pthread_mutex_unlock(&g_iJobsMutex);

	g_iJobsGiven++;
}

void CAOGenerator::RaytraceJoinThreads()
{
	if (GetNumberOfProcessors() == 1)
		return;

	// Doesn't really join the threads per se, just signals for them to quit and then waits for them to be done
	// while calling work progress updates so the user can see what's happening.

	for (size_t i = 0; i < g_aThreads.size(); i++)
		g_aThreads[i].bQuitWhenDone = true;

	if (m_pWorkListener)
		m_pWorkListener->SetAction(L"Rendering", g_iJobsGiven);

	while (true)
	{
		bool bDone = true;
		for (size_t t = 0; t < g_aThreads.size(); t++)
		{
			if (!g_aThreads[t].bDone)
				bDone = false;
		}

		if (bDone)
			return;

		if (m_pWorkListener)
			m_pWorkListener->WorkProgress(g_iJobsGiven - g_lJobs.size());

		if (m_bStopGenerating)
		{
			RaytraceCleanupThreads();
			return;
		}
	}
}
