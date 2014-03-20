#pragma once

#ifdef __ANDROID__
#ifdef TINKER_OPENGLES_3

#include <GLES3/gl3.h>

// Technically it does support multisample but I'm ignoring it for now.
#define T_PLATFORM_SUPPORTS_MULTISAMPLE false
#define T_PLATFORM_SUPPORTS_HALF_FLOAT true

#define GL_TEXTURE_2D_MULTISAMPLE ~0
#define GL_CLAMP_TO_BORDER GL_CLAMP_TO_EDGE
#define GL_MULTISAMPLE ~0

inline void glTexImage2DMultisample(int, int, int, int, int, int) { TUnimplemented(); }
inline void gluBuild2DMipmaps(GLenum target, GLint components, GLint width, GLint height, GLenum format, GLenum type, const void *data)
{
	glTexImage2D(target, 0, format, width, height, 0, format, type, data);
	glGenerateMipmap(target);
}

#else
#ifndef TINKER_OPENGLES_2
#error Android must define either TINKER_OPENGLES_3 or TINKER_OPENGLES_2
#endif

#include <GLES2/gl2.h>

#define T_PLATFORM_SUPPORTS_MULTISAMPLE false
#define T_PLATFORM_SUPPORTS_HALF_FLOAT false

#define GL_RGBA16F GL_RGBA
#define GL_RGBA8 GL_RGBA4
#define GL_HALF_FLOAT GL_UNSIGNED_BYTE
#define GL_TEXTURE_2D_MULTISAMPLE ~0
#define GL_CLAMP_TO_BORDER GL_CLAMP_TO_EDGE
#define GL_DEPTH_COMPONENT24 GL_DEPTH_COMPONENT16
#define GL_MULTISAMPLE ~0
#define GL_READ_FRAMEBUFFER GL_FRAMEBUFFER
#define GL_DRAW_FRAMEBUFFER GL_FRAMEBUFFER

inline void glTexImage2DMultisample(int, int, int, int, int, int) { TUnimplemented(); }
inline void glRenderbufferStorageMultisample(int, int, int, int, int) { TUnimplemented(); }
inline void glBlitFramebuffer(int, int, int, int, int, int, int, int, int, int) { TUnimplemented(); }
inline void gluBuild2DMipmaps(GLenum target, GLint components, GLint width, GLint height, GLenum format, GLenum type, const void *data)
{
	glTexImage2D(target, 0, format, width, height, 0, format, type, data);
	glGenerateMipmap(target);
}

#endif

#else

#include <GL3/gl3w.h>
#include <GL/glu.h>

#define T_PLATFORM_SUPPORTS_MULTISAMPLE !!glTexImage2DMultisample
#define T_PLATFORM_SUPPORTS_HALF_FLOAT GL_ARB_half_float_vertex

#endif

