#ifndef WORK_LISTENER_H
#define WORK_LISTENER_H

#include "tstring.h"

class IWorkListener
{
public:
	virtual void			BeginProgress()=0;
	virtual void			SetAction(const tstring& pszAction, size_t iTotalProgress)=0;
	virtual void			WorkProgress(size_t iProgress, bool bForceDraw = false)=0;
	virtual void			EndProgress()=0;
};

#endif
