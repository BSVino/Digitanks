#include "parallelize.h"

#include <time.h>
#include <platform.h>
#include <mempool.h>

void ThreadMain(void* pData)
{
	CParallelizeThread* pThread = (CParallelizeThread*)pData;
	pThread->Process();
}

void CParallelizeThread::Process()
{
	while (true)
	{
		if (m_pParallelizer->m_bStopped)
		{
			m_bDone = true;
			pthread_exit(NULL);
			return;
		}

		pthread_mutex_lock(&m_pParallelizer->m_iJobsMutex);
		if (!m_pParallelizer->m_lpJobs.size())
		{
			pthread_mutex_unlock(&m_pParallelizer->m_iJobsMutex);

			if (m_bQuitWhenDone)
			{
				m_bDone = true;
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

		void* pJobData = m_pParallelizer->m_lpJobs.front();
		m_pParallelizer->m_lpJobs.pop_front();

		pthread_mutex_unlock(&m_pParallelizer->m_iJobsMutex);

		m_pParallelizer->DispatchJob(pJobData);

		pthread_mutex_lock(&m_pParallelizer->m_iJobsMutex);
		mempool_free(pJobData);
		pthread_mutex_unlock(&m_pParallelizer->m_iJobsMutex);
	}
}

CParallelizer::CParallelizer(JobCallback pfnCallback)
{
	pthread_mutex_init(&m_iDataMutex, NULL);
	pthread_mutex_init(&m_iJobsMutex, NULL);

	m_iJobsGiven = 0;

	// Insert all first so that reallocations are all done before we pass the pointers to the threads.
	m_aThreads.insert(m_aThreads.begin(), GetNumberOfProcessors(), CParallelizeThread());

	for (size_t i = 0; i < GetNumberOfProcessors(); i++)
	{
		CParallelizeThread* pThread = &m_aThreads[i];

		pThread->m_pParallelizer = this;
		pThread->m_bQuitWhenDone = false;
		pThread->m_bDone = false;

		pthread_create(&pThread->m_iThread, NULL, (void *(*) (void *))&ThreadMain, (void*)pThread);
	}

	m_bStopped = false;

	m_pfnCallback = pfnCallback;
}

CParallelizer::~CParallelizer()
{
	// If there are no threads then we've already cleaned up, don't do it twice.
	if (!m_aThreads.size())
		return;

	m_bStopped = true;

	while (!AreAllJobsDone());

	while (m_lpJobs.size())
	{
		free(m_lpJobs.front());
		m_lpJobs.pop_front();
	}

	pthread_mutex_destroy(&m_iDataMutex);
	pthread_mutex_destroy(&m_iJobsMutex);

	m_aThreads.clear();
	m_lpJobs.clear();
}

void CParallelizer::AddJob(void* pJobData, size_t iSize)
{
	pthread_mutex_lock(&m_iJobsMutex);
	void* pJobDataCopy = mempool_alloc(iSize);
	memcpy(pJobDataCopy, pJobData, iSize);

	m_lpJobs.push_back(pJobDataCopy);
	pthread_mutex_unlock(&m_iJobsMutex);

	m_iJobsGiven++;
}

void CParallelizer::FinishJobs()
{
	for (size_t i = 0; i < m_aThreads.size(); i++)
		m_aThreads[i].m_bQuitWhenDone = true;
}

bool CParallelizer::AreAllJobsDone()
{
	for (size_t t = 0; t < m_aThreads.size(); t++)
	{
		if (!m_aThreads[t].m_bDone)
		{
			// Give the job a chance to finish.
			SleepMS(1);
			return false;
		}
	}
	return true;
}

void CParallelizer::LockData()
{
	pthread_mutex_lock(&m_iDataMutex);
}

void CParallelizer::UnlockData()
{
	pthread_mutex_unlock(&m_iDataMutex);
}

void CParallelizer::DispatchJob(void* pJobData)
{
	m_pfnCallback(pJobData);
}
