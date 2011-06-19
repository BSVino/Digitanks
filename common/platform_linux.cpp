#include <platform.h>

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <dirent.h>
#include <netinet/in.h>
#include <linux/if.h>
#include <X11/Xlib.h>

#include <EASTL/string.h>

#include <strutils.h>

void GetMACAddresses(unsigned char*& paiAddresses, size_t& iAddresses)
{
	static unsigned char aiAddresses[16][8];

	struct ifreq ifr;
	struct ifreq *IFR;
	struct ifconf ifc;
   	char buf[1024];
  	int s, i;

	iAddresses = 0;

	s = socket(AF_INET, SOCK_DGRAM, 0);
   	if (s == -1)
  		return;

	ifc.ifc_len = sizeof(buf);
   	ifc.ifc_buf = buf;
  	ioctl(s, SIOCGIFCONF, &ifc);

 	IFR = ifc.ifc_req;
	for (i = ifc.ifc_len / sizeof(struct ifreq); --i >= 0; IFR++)
	{
		if (iAddresses >= 16)
			break;

   		strcpy(ifr.ifr_name, IFR->ifr_name);
  		if (ioctl(s, SIOCGIFFLAGS, &ifr) != 0)
			continue;
 
		if (ifr.ifr_flags & IFF_LOOPBACK)
			continue;

  		if (ioctl(s, SIOCGIFHWADDR, &ifr) != 0)
			continue;

		aiAddresses[iAddresses][6] = 0;
		aiAddresses[iAddresses][7] = 0;
  		bcopy( ifr.ifr_hwaddr.sa_data, aiAddresses[iAddresses++], 6);
	}

	close(s);
}

void GetScreenSize(int& iWidth, int& iHeight)
{
	Display* pdsp = NULL;
	Window wid = 0;
	XWindowAttributes xwAttr;

	pdsp = XOpenDisplay( NULL );
	if ( !pdsp )
		return;

	wid = DefaultRootWindow( pdsp );
	if ( 0 > wid )
		return;
 
	Status ret = XGetWindowAttributes( pdsp, wid, &xwAttr );
	iWidth = xwAttr.width;
	iHeight = xwAttr.height;

	XCloseDisplay( pdsp );
}

size_t GetNumberOfProcessors()
{
	return sysconf(_SC_NPROCESSORS_ONLN);
}

void SleepMS(size_t iMS)
{
	usleep(iMS);
}

void OpenBrowser(const eastl::string16& sURL)
{
	system(convertstring<char16_t, char>(eastl::string16("firefox ") + sURL).c_str());
}

void CreateMinidump(void* pInfo, wchar_t* pszDirectory)
{
}

wchar_t* OpenFileDialog(const wchar_t* pszFileTypes, const wchar_t* pszDirectory)
{
}

wchar_t* SaveFileDialog(const wchar_t* pszFileTypes, const wchar_t* pszDirectory)
{
}

eastl::string GetClipboard()
{
}

void SetClipboard(const eastl::string& sBuf)
{
}

eastl::string16 GetAppDataDirectory(const eastl::string16& sDirectory, const eastl::string16& sFile)
{
	char* pszVar = getenv("HOME");

	eastl::string16 sSuffix;
	sSuffix.append(sDirectory).append(L"\\").append(sFile);

	eastl::string16 sReturn(convertstring<char, char16_t>(pszVar));

	mkdir(convertstring<char16_t, char>(eastl::string16(sReturn).append(L"\\").append(sDirectory)).c_str(), 0777);

	sReturn.append(L"\\").append(sSuffix);
	return sReturn;
}

eastl::vector<eastl::string16> ListDirectory(eastl::string16 sDirectory, bool bDirectories)
{
	eastl::vector<eastl::string16> asResult;

	struct dirent *dp;

	DIR *dir = opendir(convertstring<char16_t, char>(sDirectory).c_str());
	while ((dp=readdir(dir)) != NULL)
	{
		if (!bDirectories && (dp->d_type == DT_DIR))
			continue;

		asResult.push_back(convertstring<char, char16_t>(dp->d_name));
	}
	closedir(dir);

	return asResult;
}

bool IsFile(eastl::string16 sPath)
{
	struct stat stFileInfo;
	bool blnReturn;
	int intStat;

	// Attempt to get the file attributes
	intStat = stat(convertstring<char16_t, char>(sPath).c_str(), &stFileInfo);
	if(intStat == 0 && S_ISREG(stFileInfo.st_mode))
		return true;
	else
		return false;
}

bool IsDirectory(eastl::string16 sPath)
{
	struct stat stFileInfo;
	bool blnReturn;
	int intStat;

	// Attempt to get the file attributes
	intStat = stat(convertstring<char16_t, char>(sPath).c_str(), &stFileInfo);
	if(intStat == 0 && S_ISDIR(stFileInfo.st_mode))
		return true;
	else
		return false;
}

void DebugPrint(eastl::string16 sText)
{
	wprintf(sText.c_str());
}

void Exec(eastl::string sLine)
{
	system(sLine.c_str());
}
