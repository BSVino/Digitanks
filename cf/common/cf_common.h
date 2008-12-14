#ifndef CF_SHAREDDEFS_H
#define CF_SHAREDDEFS_H
#ifdef _WIN32
#pragma once
#endif

#define CF_VERSION "beta1"

#define CF_DECLARE_CLASS( className, baseClassName ) \
	typedef baseClassName BaseClass; \
	typedef className ThisClass; \

#endif