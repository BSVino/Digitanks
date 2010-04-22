#include "ui/modelwindow.h"

#include <time.h>

#ifdef _WIN32
#include <sys/utime.h>
#endif

#include <license/license.h>

// Don't want to import winsock just for htonl and ntohl, so we're shortcutting it.
#ifdef _WIN32
#define htopl(A) ((((unsigned long)(A) & 0xff000000) >> 24) | \
	(((unsigned long)(A) & 0x00ff0000) >> 8) | \
	(((unsigned long)(A) & 0x0000ff00) << 8) | \
	(((unsigned long)(A) & 0x000000ff) << 24))
#define ptohl htopl
#else
#define htopl(A) (A)
#define ptohl(A) (A)
#endif

static unsigned long g_iSMAKTex = 0;
static char g_szSMAKTex[40];
static unsigned char g_iSMAKId[8];

extern void GetMACAddresses(unsigned char*& paiAddresses, size_t& iAddresses);

// It says it's the SMAK texture but it's actually our license file. Sneaky!
void CModelWindow::LoadSMAKTexture()
{
	FILE* fp = fopen("smak.png", "rb");

	// Can't license the product without our license file.
	if (!fp)
		return;

	unsigned char* paiAddresses;
	size_t iAddresses;
	GetMACAddresses(paiAddresses, iAddresses);

	bool bFoundTexture = false;

	fseek(fp, 0, SEEK_END);
	long iFileLength = ftell(fp);

	fseek(fp, 8, SEEK_SET);
	while (!feof(fp))
	{
		long iChunkStart = ftell(fp);

		if (iChunkStart >= iFileLength)
			break;

		unsigned long iLength = 0;
		fread(&iLength, sizeof(iLength), 1, fp);
		iLength = ptohl(iLength);

		char szName[4];
		fread(&szName[0], sizeof(szName), 1, fp);

		if (strncmp(szName, "dtAp", 4) == 0)
		{
			char szData[9];
			fread(&szData[0], sizeof(szData)-1, 1, fp);
			szData[8] = '\0';
			g_iSMAKTex = atoi(szData);

			unsigned char szData3[8];
			fread(&szData3[0], sizeof(szData3), 1, fp);

			if (g_iSMAKTex)
			{
				// Look for our nic card. If it's not on the list then we should generate a new one.
				for (size_t i = 0; i < iAddresses; i++)
				{
					if (memcmp(szData3, &paiAddresses[i*8], sizeof(szData3)) == 0)
					{
						memcpy(g_iSMAKId, szData3, sizeof(g_iSMAKId));
						bFoundTexture = true;
						break;
					}
				}
			}

			char szData2[40];
			fread(&szData2[0], sizeof(szData2), 1, fp);
			strncpy(g_szSMAKTex, szData2, sizeof(szData2));
			break;
		}
		else
			// Skip to the next section!
			fseek(fp, iLength+4, SEEK_CUR);
	}

	fclose(fp);

	if (!bFoundTexture)
		SetupSMAKTexture();
}

void CModelWindow::SetupSMAKTexture()
{
	size_t i;

	srand((unsigned int)time(NULL));
	char szCode[9];
	for (i = 0; i < 8; i++)
		szCode[i] = (rand() % 10) + '0';

	szCode[8] = '\0';

	g_iSMAKTex = atol(szCode);

	for (i = 0; i < 40; i++)
		g_szSMAKTex[i] = '\0';

	unsigned char* paiAddresses;
	size_t iAddresses;
	GetMACAddresses(paiAddresses, iAddresses);

	// Just use the first one.
	memcpy(g_iSMAKId, paiAddresses, sizeof(g_iSMAKId));

	SaveSMAKTexture();
}

void CModelWindow::SaveSMAKTexture()
{
	FILE* fp = fopen("smak.png", "rb+");

	// Can't license the product without our license file.
	if (!fp)
		return;

	// Generate the product code.
	unsigned char szCode[56];

	memset(szCode, 0, sizeof(szCode));

	sprintf((char*)szCode, "%.8d", g_iSMAKTex);

	size_t i;
	for (i = 0; i < 8; i++)
		szCode[i+8] = g_iSMAKId[i];

	for (i = 0; i < 40; i++)
		szCode[i+16] = g_szSMAKTex[i];

	static unsigned char szChunkName[4] = { 'd', 't', 'A', 'p' };	// death to all pirates
	static unsigned char szEndChunk[12] = { 0, 0, 0, 0, 'I', 'E', 'N', 'D', 0xae, 0x42, 0x60, 0x82 };

	fseek(fp, 0, SEEK_END);
	long iFileLength = ftell(fp);

	fseek(fp, 8, SEEK_SET);
	while (!feof(fp))
	{
		long iChunkStart = ftell(fp);

		if (iChunkStart >= iFileLength)
			break;

		unsigned long iLength = 0;
		fread(&iLength, sizeof(iLength), 1, fp);
		iLength = ptohl(iLength);

		char szName[4];
		fread(&szName[0], sizeof(szName), 1, fp);

		if (strncmp(szName, "dtAp", 4) == 0 || strncmp(szName, "IEND", 4) == 0)
		{
			fseek(fp, iChunkStart, SEEK_SET);

			// Write our license management chunk.
			unsigned int iChunkSize = htopl(sizeof(szCode));
			fwrite(&iChunkSize, sizeof(iChunkSize), 1, fp);
			fwrite(szChunkName, sizeof(szChunkName), 1, fp);
			fwrite(szCode, sizeof(szCode), 1, fp);
			unsigned long iCRC = PNG_CRC(szCode, sizeof(szCode));
			fwrite(&iCRC, sizeof(iCRC), 1, fp);

			// Write the PNG end chunk.
			fwrite(szEndChunk, sizeof(szEndChunk), 1, fp);
			break;
		}
		else
			// Skip to the next section!
			fseek(fp, iLength+4, SEEK_CUR);
	}

	fclose(fp);

#ifdef _WIN32
	// Make the modification time be always the same so people hopefully don't catch on that the file is updated by the application.
	struct _utimbuf ut;
	ut.actime = ut.modtime = 1260997705; // Dec 16, 2009, the day I wrote this code
	_utime("smak.png", &ut);
#endif
}

bool CModelWindow::GetSMAKTexture()
{
	std::string sResult;
	if (!GenerateKey(GetSMAKTextureCode(), sResult))
		return false;

	return memcmp(sResult.c_str(), g_szSMAKTex, 40) == 0;
}

void CModelWindow::SetSMAKTexture(const char* pszTex)
{
	strncpy(g_szSMAKTex, pszTex, 40);

	if (GetSMAKTexture())
		SaveSMAKTexture();
}

std::string CModelWindow::GetSMAKTextureCode()
{
	char szCode[40];
	sprintf(szCode, "%.8d-%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x", g_iSMAKTex, g_iSMAKId[0], g_iSMAKId[1], g_iSMAKId[2], g_iSMAKId[3], g_iSMAKId[4], g_iSMAKId[5], g_iSMAKId[6], g_iSMAKId[7]);

	return std::string(szCode);
}

/* Table of CRCs of all 8-bit messages. */
static unsigned long g_aiCRCTable[256];

/* Flag: has the table been computed? Initially false. */
static bool g_bCRCComputed = 0;

/* Make the table for a fast CRC. */
void PNG_CreateCRCTable()
{
	unsigned long c;
	int n, k;

	for (n = 0; n < 256; n++)
	{
		c = (unsigned long) n;
		for (k = 0; k < 8; k++)
		{
			if (c & 1)
				c = 0xedb88320L ^ (c >> 1);
			else
				c = c >> 1;
		}
		g_aiCRCTable[n] = c;
	}
	g_bCRCComputed = true;
}

/* Update a running CRC with the bytes buf[0..len-1]--the CRC
should be initialized to all 1's, and the transmitted value
is the 1's complement of the final running CRC (see the
crc() routine below)). */

static unsigned long PNG_UpdateCRC(unsigned long crc, unsigned char *buf, int len)
{
	unsigned long c = crc;
	int n;

	if (!g_bCRCComputed)
		PNG_CreateCRCTable();

	for (n = 0; n < len; n++)
		c = g_aiCRCTable[(c ^ buf[n]) & 0xff] ^ (c >> 8);

	return c;
}

/* Return the CRC of the bytes buf[0..len-1]. */
unsigned long CModelWindow::PNG_CRC(unsigned char *buf, int len)
{
	return PNG_UpdateCRC(0xffffffffL, buf, len) ^ 0xffffffffL;
}
