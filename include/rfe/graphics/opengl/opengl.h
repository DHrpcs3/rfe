#pragma once
#ifndef _WIN32
#include <GL/glew.h>
#endif

#ifdef _WIN32
#undef WINAPI_FAMILY
#define WINAPI_FAMILY WINAPI_FAMILY_DESKTOP_APP
#include <Windows.h>
#include <Wingdi.h>

#include <GL/gl.h>
#include <GL/glext.h>

#elif defined(__APPLE__)
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>

#else
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glxext.h>
#endif

namespace rfe
{
	namespace graphics
	{
#ifdef _WIN32
#define OPENGL_PROC(p, n) extern PFNGL##p##PROC gl##n
#include "gl_proc_table.h"
#undef OPENGL_PROC
#endif

		namespace opengl
		{
			void init();
		}
	}
}