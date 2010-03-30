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
	bool							m_bQuit;
};

class CParallelizeJob
{
public:
									CParallelizeJob() : m_pJobData(NULL), m_iExecuted(0) {};

public:
	void*							m_pJobData;
	size_t							m_iExecuted;
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
	bool							AreAllJobsQuit();

	void							Start() { m_bStopped = false; };
	void							Stop() { m_bStopped = true; };
	void							RestartJobs();

	void							LockData();
	void							UnlockData();

	size_t							GetJobsTotal() { return m_iJobsGiven; };
	size_t							GetJobsDone() { return m_iJobsDone; };	// GetJobsDone: What I like to say

private:
	void							DispatchJob(void* pJobData);

	std::vector<CParallelizeThread>	m_aThreads;
	std::vector<CParallelizeJob>	m_aJobs;
	size_t							m_iLastExecuted;
	size_t							m_iLastAssigned;
	pthread_mutex_t					m_iJobsMutex;
	size_t							m_iJobsGiven;
	size_t							m_iJobsDone;	// JobsDone: What I like to hear
	pthread_mutex_t					m_iDataMutex;
	size_t							m_iExecutions;

	bool							m_bStopped;
	bool							m_bShuttingDown;

	JobCallback						m_pfnCallback;
};

#endif