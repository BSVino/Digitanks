#include <string>
#include <vector>
#include <string.h>

#include <strutils.h>
#include <platform.h>

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

bool GenerateKey(std::string sProductCode, std::string& sKey)
{
	if (!sProductCode.size())
		return false;

	std::vector<std::string> sTokens;
	strtok(sProductCode, sTokens, "-");

	int iProductCode = atoi(sTokens[0].c_str());

	unsigned char aiHostID[8];
	memset(aiHostID, 0, sizeof(aiHostID));

	char szOctet[3];
	szOctet[2] = '\0';
	const char* pszOctets = sTokens[1].c_str();
	for (size_t i = 0; i < 8; i++)
	{
		szOctet[0] = pszOctets[(i*2)];
		szOctet[1] = pszOctets[(i*2)+1];
		int iID;
		sscanf(szOctet, "%x", &iID);
		aiHostID[i] = (unsigned char)iID;
	}

	if (!iProductCode)
		return false;

	unsigned char szTexture[40] =
	{
		0xb3, 0x5c, 0x5a, 0xdd, 0x83, 0xdf, 0xba, 0xd3, 0xf6, 0x99,
		0x86, 0xd9, 0xb7, 0x9d, 0x1e, 0xf1, 0xec, 0x13, 0x76, 0x00,
		0x2b, 0xb8, 0x69, 0x16, 0x5a, 0x51, 0x9c, 0x5d, 0xdc, 0x14,
		0x34, 0x21, 0x20, 0xf0, 0x94, 0x5b, 0x14, 0xfd, 0x53, 0xdd
	};

	size_t i;

#ifdef CF_LICENSE_GENERATOR
	bool bFoundId = true;
#else
	unsigned char* paiAddresses;
	size_t iAddresses;
	GetMACAddresses(paiAddresses, iAddresses);

	bool bFoundId = false;
	for (i = 0; i < iAddresses; i++)
	{
		if (memcmp(&aiHostID[0], &paiAddresses[i*8], sizeof(aiHostID)) == 0)
		{
			bFoundId = true;
			break;
		}
	}
#endif

	unsigned int iIdSum = 0;

	// Anybody with no ethernet card falls back to just the product code.
	// Technically it means all of those people can share a code/key but
	// everybody has nic cards these days so who cares?
	if (bFoundId)
	{
		for (i = 0; i < 8; i++)
			iIdSum += aiHostID[i];
	}

	mtsrand(iProductCode + iIdSum);

	unsigned char szResult[41];

	for (i = 0; i < 40; i++)
	{
		szResult[i] = (szTexture[i] ^ (unsigned char)mtrand())%36;

		if (szResult[i] < 10)
			szResult[i] += '0';
		else
			szResult[i] += 'A' - 10;

		if (szResult[i] == '1')
			szResult[i] = 'R';

		else if (szResult[i] == 'l')
			szResult[i] = 'T';

		else if (szResult[i] == 'I')
			szResult[i] = '7';

		else if (szResult[i] == '0')
			szResult[i] = '4';

		else if (szResult[i] == 'O')
			szResult[i] = 'Z';
	}

	szResult[40] = '\0';

	sKey = std::string((char*)szResult);

	return true;
}
