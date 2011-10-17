#include "mtrand.h"

#define MT_SIZE 624
static size_t g_aiMT[MT_SIZE];
static size_t g_iMTI = 0;

// Mersenne Twister implementation from Wikipedia.
void mtsrand(size_t iSeed)
{
	g_aiMT[0] = iSeed;
	for (size_t i = 1; i < MT_SIZE; i++)
	{
		size_t inner1 = g_aiMT[i-1];
		size_t inner2 = (g_aiMT[i-1]>>30);
		size_t inner = inner1 ^ inner2;
		g_aiMT[i] = (0x6c078965 * inner) + i;
	}

	g_iMTI = 0;
}

size_t mtrand()
{
	if (g_iMTI == 0)
	{
		for (size_t i = 0; i < MT_SIZE; i++)
		{
			size_t y = (0x80000000&(g_aiMT[i])) + (0x7fffffff&(g_aiMT[(i+1) % MT_SIZE]));
			g_aiMT[i] = g_aiMT[(i + 397)%MT_SIZE] ^ (y>>1);
			if ((y%2) == 1)
				g_aiMT[i] = g_aiMT[i] ^ 0x9908b0df;
		}
	}

	size_t y = g_aiMT[g_iMTI];
	y = y ^ (y >> 11);
	y = y ^ ((y << 7) & (0x9d2c5680));
	y = y ^ ((y << 15) & (0xefc60000));
	y = y ^ (y >> 18);

	g_iMTI = (g_iMTI + 1) % MT_SIZE;

	return y;
}
