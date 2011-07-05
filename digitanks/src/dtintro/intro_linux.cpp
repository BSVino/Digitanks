#include <stdlib.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <IL/il.h>
#include <IL/ilu.h>

size_t FullScreenShot()
{
	ilInit();

	Display* pDisplay = XOpenDisplay(NULL);
	int iScreen = DefaultScreen(pDisplay);
	Window oRootWindow = RootWindow(pDisplay, iScreen);

	XWindowAttributes oAttributes;
	XGetWindowAttributes(pDisplay, oRootWindow, &oAttributes);

	XImage* pImage = XGetImage(pDisplay, oRootWindow, 0, 0, oAttributes.width, oAttributes.height, AllPlanes, ZPixmap);

	ILuint iDevILId;
	ilGenImages(1, &iDevILId);
	ilBindImage(iDevILId);

	ILboolean bSuccess = ilTexImage(pImage->width, pImage->height, 1, 4, IL_BGRA, IL_UNSIGNED_BYTE, (void*)(&(pImage->data[0])));

	if (!bSuccess)
		return 0;

	bSuccess = ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE);
	if (!bSuccess)
		return 0;

	ILinfo ImageInfo;
	iluGetImageInfo(&ImageInfo);

	iluFlipImage();

	XDestroyImage(pImage);

	XCloseDisplay(pDisplay);

	return iDevILId;
}
