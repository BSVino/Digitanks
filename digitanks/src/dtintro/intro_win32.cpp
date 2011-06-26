#include <windows.h>

#include <IL/il.h>
#include <IL/ilu.h>

size_t FullScreenShot()
{
	/// Take a screenshot of the user's desktop.
	HDC hdcScreen  = CreateDC(_T("DISPLAY", NULL, NULL, NULL);
	HDC hdcCapture = CreateCompatibleDC(hdcScreen);
	int nWidth     = GetDeviceCaps(hdcScreen, HORZRES),
		nHeight    = GetDeviceCaps(hdcScreen, VERTRES),
		nBPP       = GetDeviceCaps(hdcScreen, BITSPIXEL);

	LPBYTE lpCapture;
	BITMAPINFO bmiCapture = { {
		sizeof(BITMAPINFOHEADER), nWidth, nHeight, 1, nBPP, BI_RGB, 0, 0, 0, 0, 0,
	} };
	HBITMAP hbmCapture = CreateDIBSection(hdcScreen, &bmiCapture,
		DIB_PAL_COLORS, (LPVOID *)&lpCapture, NULL, 0);
	if(!hbmCapture)
	{
		DeleteDC(hdcCapture);
		DeleteDC(hdcScreen);
		return 0;
	}

	int nCapture = SaveDC(hdcCapture);
	SelectObject(hdcCapture, hbmCapture);
	BitBlt(hdcCapture, 0, 0, nWidth, nHeight, hdcScreen, 0, 0, SRCCOPY);
	RestoreDC(hdcCapture, nCapture);
	DeleteDC(hdcCapture);
	DeleteDC(hdcScreen);

	ilInit();

	ILuint iDevILId;
	ilGenImages(1, &iDevILId);
	ilBindImage(iDevILId);

	ILboolean bSuccess = ilTexImage(nWidth, nHeight, 1, 4, IL_BGRA, IL_UNSIGNED_BYTE, lpCapture);

	if (!bSuccess)
		return 0;

	bSuccess = ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE);
	if (!bSuccess)
		return 0;

	ILinfo ImageInfo;
	iluGetImageInfo(&ImageInfo);

	if (ImageInfo.Origin == IL_ORIGIN_UPPER_LEFT)
		iluFlipImage();

	return iDevILId;
}
