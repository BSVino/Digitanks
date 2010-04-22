#ifndef _CF_MEMPOOL
#define _CF_MEMPOOL

#include <vector>

class CMemChunk
{
public:
	void*							m_pMem;
	size_t							m_iSize;
	CMemChunk*						m_pNext;
};

class CMemPool
{
public:
	static void*					Alloc(size_t iSize);
	static void						Free(void* pMem);

	// A potentially dangerous function to call if the memory is still being used.
	static void						ClearPools();

private:
									CMemPool();

public:
									~CMemPool();

private:
	void*							Reserve(void* pLocation, size_t iSize, CMemChunk* pAfter = NULL);

	void*							m_pMemPool;
	size_t							m_iMemPoolSize;
	size_t							m_iMemoryAllocated;
	CMemChunk*						m_pAllocMap;	// *Sorted* list of memory allocations.
	CMemChunk*						m_pAllocMapBack;

private:
	static CMemPool*				AddPool(size_t iSize);

	static size_t					s_iMemoryAllocated;
	static std::vector<CMemPool*>	s_apMemPools;
};

void* mempool_alloc(size_t iSize);
void mempool_free(void* pMem);

#endif