#ifndef _CF_PARALLELIZE_H
#define _CF_PARALLELIZE_H

#include <list>
#include <vector>
#include <pthread.h>

class CParallelizeThread
{
public:
	void							Process();

public:
	pthread_t						m_iThread;
	class CParallelizer*			m_pParallelizer;
	bool							m_bQuitWhenDone;
	bool							m_bDone;
};

typedef void (*JobCallback)(void*);

class CParallelizer
{
public:
	friend CParallelizeThread;

public:
									CParallelizer(JobCallback pfnCallback);
									~CParallelizer();

public:
	void							AddJob(void* pJobData, size_t iSize);
	void							FinishJobs();
	bool							AreAllJobsDone();

	void							Stop() { m_bStopped = true; };

	void							LockData();
	void							UnlockData();

	size_t							GetJobsTotal() { return m_iJobsGiven; };
	size_t							GetJobsRemaining() { return m_lpJobs.size(); };

private:
	void							DispatchJob(void* pJobData);

	std::vector<CParallelizeThread>	m_aThreads;
	std::list<void*>				m_lpJobs;
	pthread_mutex_t					m_iJobsMutex;
	size_t							m_iJobsGiven;
	pthread_mutex_t					m_iDataMutex;

	bool							m_bStopped;

	JobCallback						m_pfnCallback;
};

#endif