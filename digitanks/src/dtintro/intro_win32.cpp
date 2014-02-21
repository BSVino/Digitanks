#include <windows.h>

#include <tvector.h>
#include <color.h>

tvector<Color> FullScreenShot(int& iWidth, int& iHeight)
{
	/// Take a screenshot of the user's desktop.
	HDC hdcScreen  = CreateDC(L"DISPLAY", NULL, NULL, NULL);
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
		return tvector<Color>();
	}

	int nCapture = SaveDC(hdcCapture);
	SelectObject(hdcCapture, hbmCapture);
	BitBlt(hdcCapture, 0, 0, nWidth, nHeight, hdcScreen, 0, 0, SRCCOPY);
	RestoreDC(hdcCapture, nCapture);
	DeleteDC(hdcCapture);
	DeleteDC(hdcScreen);

	tvector<Color> aclrScreenshot;
	aclrScreenshot.resize(nWidth * nHeight);
	memcpy(&aclrScreenshot[0], lpCapture, sizeof(Color) * aclrScreenshot.size());

	iWidth = nWidth;
	iHeight = nHeight;
	return aclrScreenshot;
}
