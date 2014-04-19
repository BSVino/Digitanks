/*
Copyright (c) 2012, Lunar Workshop, Inc.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
3. All advertising materials mentioning features or use of this software must display the following acknowledgement:
   This product includes software developed by Lunar Workshop, Inc.
4. Neither the name of the Lunar Workshop nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY LUNAR WORKSHOP INC ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL LUNAR WORKSHOP BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <tinker_platform.h>

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <dirent.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <linux/if.h>
#include <android/log.h>
#include <android/asset_manager_jni.h>
#include <jni.h>
#include <SDL.h>
#include <asm-generic/errno-base.h>

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

size_t GetNumberOfProcessors()
{
	return sysconf(_SC_NPROCESSORS_ONLN);
}

void SleepMS(size_t iMS)
{
	usleep(iMS);
}

void OpenBrowser(const tstring& sURL)
{
	int iSystem = system((tstring("firefox ") + sURL).c_str());
}

void OpenExplorer(const tstring& sDirectory)
{
	int iSystem = system((tstring("gnome-open ") + sDirectory).c_str());
}

void Alert(const tstring& sMessage)
{
	fputs(sMessage.c_str(), stderr);
}

void CreateMinidump(void* pInfo, tchar* pszDirectory)
{
}

tstring GetClipboard()
{
	TUnimplemented();
	return tstring();
}

void SetClipboard(const tstring& sBuf)
{
	TUnimplemented();
}

static jobject global_asset_manager = 0;
static AAssetManager* g_pAssetManager = 0;

void InitializeAssetManager()
{
	if (!global_asset_manager)
	{
		JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();

		jobject activity = (jobject)SDL_AndroidGetActivity();

		jclass activity_class = env->GetObjectClass(activity);

		jmethodID activity_class_getAssets = env->GetMethodID(activity_class, "getAssets", "()Landroid/content/res/AssetManager;");
		jobject asset_manager = env->CallObjectMethod(activity, activity_class_getAssets); // activity.getAssets();
		global_asset_manager = env->NewGlobalRef(asset_manager);

		g_pAssetManager = AAssetManager_fromJava(env, global_asset_manager);
	}
}

tvector<tstring> ListAndroidAssetsDirectory(const tstring& sDirectory, bool bDirectories)
{
	JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
	jobject activity = (jobject)SDL_AndroidGetActivity();

	jclass activity_class = env->GetObjectClass(activity);
	jmethodID activity_class_assetOpenDir = env->GetMethodID(activity_class, "assetOpenDir", "(Ljava/lang/String;)Z");
	jstring assetOpenDir_sDirectory = env->NewStringUTF(sDirectory.c_str());
	jboolean assetOpenDir_success = env->CallBooleanMethod(activity, activity_class_assetOpenDir, assetOpenDir_sDirectory); // activity.assetOpenDir(sDirectory);
	env->DeleteLocalRef(assetOpenDir_sDirectory);

	if (!assetOpenDir_success)
		return tvector<tstring>();

	tvector<tstring> asResult;
	const char* pszDir;

	jmethodID activity_class_assetGetNext = env->GetMethodID(activity_class, "assetGetNext", "()Ljava/lang/String;");
	jmethodID activity_class_assetIsDirectory = env->GetMethodID(activity_class, "assetIsDirectory", "(Ljava/lang/String;)Z");

	jstring dir;

	while ((dir = (jstring)env->CallObjectMethod(activity, activity_class_assetGetNext)) != NULL)
	{
		const char* pszDir = env->GetStringUTFChars(dir, nullptr);
		tstring sDir = pszDir;
		env->ReleaseStringUTFChars(dir, pszDir);

		tstring sFullDir = sDirectory + "/" + sDir;

		if (!bDirectories)
		{
			jstring assetIsDirectory_sFile = env->NewStringUTF(sFullDir.c_str());
			jboolean assetIsDirectory_result = env->CallBooleanMethod(activity, activity_class_assetIsDirectory, assetIsDirectory_sFile); // activity.assetIsDirectory(file);
			bool bIsDirectory = assetIsDirectory_result;
			env->DeleteLocalRef(assetIsDirectory_sFile);

			if (bIsDirectory)
				continue;
		}

		asResult.push_back(sDir);
	}

	return asResult;
}

tvector<tstring> ListDirectory(const tstring& sFullDirectory, bool bDirectories)
{
	tstring sDirectory = sFullDirectory;

	if (sDirectory.startswith("$ASSETS/"))
		return ListAndroidAssetsDirectory(sDirectory.substr(8), bDirectories);

	tvector<tstring> asResult;

	struct dirent *dp;

	DIR *dir = opendir((sDirectory).c_str());

	if (!dir)
		return asResult;

	while ((dp = readdir(dir)) != NULL)
	{
		if (!bDirectories && (dp->d_type == DT_DIR))
			continue;

		tstring sName = dp->d_name;
		if (sName == ".")
			continue;

		if (sName == "..")
			continue;

		asResult.push_back(sName);
	}
	closedir(dir);

	return asResult;
}

static int android_read(void* asset, char* buf, int size) {
	return AAsset_read((AAsset*)asset, buf, size);
}

static int android_write(void* asset, const char* buf, int size) {
	return EACCES; // can't provide write access to the apk
}

static fpos_t android_seek(void* asset, fpos_t offset, int whence) {
	return AAsset_seek((AAsset*)asset, offset, whence);
}

static int android_close(void* asset) {
	AAsset_close((AAsset*)asset);
	return 0;
}

FILE* Tinker_Android_tfopen(const tstring& sFile, const tstring& sMode)
{
	if (sMode[0] == 'w')
		return nullptr;

	InitializeAssetManager();

	AAsset* pAsset = AAssetManager_open(g_pAssetManager, sFile.c_str(), AASSET_MODE_STREAMING);
	if (!pAsset)
		return nullptr;

	return funopen(pAsset, android_read, android_write, android_seek, android_close);
}

bool IsFile(const tstring& sPath)
{
	InitializeAssetManager();

	AAsset* pAsset = AAssetManager_open(g_pAssetManager, sPath.c_str(), AASSET_MODE_STREAMING);
	if (pAsset)
	{
		AAsset_close(pAsset);
		return true;
	}

	struct stat stFileInfo;
	bool blnReturn;
	int intStat;

	// Attempt to get the file attributes
	intStat = stat(sPath.c_str(), &stFileInfo);
	if(intStat == 0 && S_ISREG(stFileInfo.st_mode))
		return true;
	else
		return false;
}

bool IsDirectory(const tstring& sPath)
{
	// Check first to see if it's in the assets folder.
	JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
	jobject activity = (jobject)SDL_AndroidGetActivity();
	jclass activity_class = env->GetObjectClass(activity);
	jmethodID activity_class_assetIsDirectory = env->GetMethodID(activity_class, "assetIsDirectory", "(Ljava/lang/String;)Z");

	jstring assetIsDirectory_sFile = env->NewStringUTF(sPath.c_str());
	jboolean assetIsDirectory_result = env->CallBooleanMethod(activity, activity_class_assetIsDirectory, assetIsDirectory_sFile); // activity.assetIsDirectory(file);
	bool bIsDirectory = assetIsDirectory_result;
	env->DeleteLocalRef(assetIsDirectory_sFile);

	if (bIsDirectory)
		return true;

	struct stat stFileInfo;
	bool blnReturn;
	int intStat;

	// Attempt to get the file attributes
	intStat = stat(sPath.c_str(), &stFileInfo);
	if(intStat == 0 && S_ISDIR(stFileInfo.st_mode))
		return true;
	else
		return false;
}

void CreateDirectoryNonRecursive(const tstring& sPath)
{
	TUnimplemented();

	mkdir(sPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}

bool CopyFileTo(const tstring& sFrom, const tstring& sTo, bool bOverride)
{
	TUnimplemented();

	int read_fd;
	int write_fd;
	struct stat stat_buf;
	off_t offset = 0;

	read_fd = open(sFrom.c_str(), O_RDONLY);

	if (!read_fd)
		return false;

	fstat(read_fd, &stat_buf);

	write_fd = open(sTo.c_str(), O_WRONLY | O_CREAT, stat_buf.st_mode);
	if (!write_fd)
	{
		close(read_fd);
		return false;
	}

	sendfile(write_fd, read_fd, &offset, stat_buf.st_size);

	close(read_fd);
	close(write_fd);

	return true;
}

tstring FindAbsolutePath(const tstring& sPath)
{
	TUnimplemented();

	char* pszFullPath = realpath(sPath.c_str(), nullptr);
	tstring sFullPath = pszFullPath;
	free(pszFullPath);

	return sFullPath;
}

time_t GetFileModificationTime(const char* pszFile)
{
	TUnimplemented();

	struct stat s;
	if (stat(pszFile, &s) != 0)
		return 0;

	return s.st_mtime;
}

void DebugPrint(const char* pszText)
{
	__android_log_print(ANDROID_LOG_INFO, "TinkerDebug", "%s", pszText);
}

void Exec(const tstring& sLine)
{
	int iSystem = system((tstring("./") + sLine).c_str());
}

// Not worried about supporting these on Linux right now.
int TranslateKeyToQwerty(int iKey)
{
	return iKey;
}

int TranslateKeyFromQwerty(int iKey)
{
	return iKey;
}

void GetScreenDPI(float& xdpi, float& ydpi)
{
	JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
	jobject activity = (jobject)SDL_AndroidGetActivity();

	jclass activity_class = env->GetObjectClass(activity);
	jmethodID activity_class_getResources = env->GetMethodID(activity_class, "getResources", "()Landroid/content/res/Resources;");
	jobject resources = env->CallObjectMethod(activity, activity_class_getResources); // activity.getResources();

	jclass resources_class = env->GetObjectClass(resources);
	jmethodID resources_class_getDisplayMetrics = env->GetMethodID(resources_class, "getDisplayMetrics", "()Landroid/util/DisplayMetrics;");
	jobject display_metrics = env->CallObjectMethod(resources, resources_class_getDisplayMetrics); // resources.getDisplayMetrics();

	jclass display_metrics_class = env->GetObjectClass(display_metrics);
	jfieldID xdpi_field = env->GetFieldID(display_metrics_class, "xdpi", "F");
	jfieldID ydpi_field = env->GetFieldID(display_metrics_class, "ydpi", "F");

	xdpi = env->GetFloatField(display_metrics, xdpi_field);
	ydpi = env->GetFloatField(display_metrics, ydpi_field);
}

void EnableMulticast()
{
	//WifiManager.MulticastLock mclock;
	//WifiManager wifimanager = (WifiManager)getSystemService(Context.WIFI_SERVICE);
	//if (wifimanager != null)
	//{
	//	mclock = wifimanager.createMulticastLock("lock");
	//	mcLock.acquire();
	//}

	JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
	jobject activity = (jobject)SDL_AndroidGetActivity();

	jclass activity_class = env->GetObjectClass(activity);
	jmethodID activity_class_getSystemService = env->GetMethodID(activity_class, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
	jstring getSystemService_wifi = env->NewStringUTF("wifi");
	jobject wifimanager = env->CallObjectMethod(activity, activity_class_getSystemService, getSystemService_wifi); // activity.getSystemService(Context.WIFI_SERVICE);
	env->DeleteLocalRef(getSystemService_wifi);

	if (wifimanager == nullptr)
	{
		DebugPrint("ERROR: Couldn't get WifiManager to enable multicast.\n");
		return;
	}

	jclass wifimanager_class = env->GetObjectClass(wifimanager);
	jmethodID wifimanager_class_createMulticastLock = env->GetMethodID(wifimanager_class, "createMulticastLock", "(Ljava/lang/String;)Landroid/net/wifi/WifiManager$MulticastLock;");
	jstring createMulticastLock_lock = env->NewStringUTF("tinker_multicast_lock");
	jobject mclock = env->CallObjectMethod(wifimanager, wifimanager_class_createMulticastLock, createMulticastLock_lock); // wifimanager.createMulticastLock("lock");
	env->DeleteLocalRef(createMulticastLock_lock);

	if (mclock == nullptr)
	{
		DebugPrint("ERROR: Couldn't get multicast lock object.\n");
		return;
	}

	jobject global_mclock = env->NewGlobalRef(mclock);

	jclass mclock_class = env->GetObjectClass(mclock);
	jmethodID mclock_class_acquire = env->GetMethodID(mclock_class, "acquire", "()V");
	env->CallVoidMethod(mclock, mclock_class_acquire); // mclock.acquire();

	DebugPrint("Acquired multicast lock.\n");
}

void DisableMulticast()
{
	//if (mclock.isHeld())
	//{
	//	mclock.release();
	//}
}



