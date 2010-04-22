#ifndef WORK_LISTENER_H
#define WORK_LISTENER_H

class IWorkListener
{
public:
	virtual void			BeginProgress()=0;
	virtual void			SetAction(wchar_t* pszAction, size_t iTotalProgress)=0;
	virtual void			WorkProgress(size_t iProgress, bool bForceDraw = false)=0;
	virtual void			EndProgress()=0;
};

#endif
