
#include <rfe/graphics/opengl/opengl.h>
#include <iostream>

namespace rfe
{
	namespace graphics
	{
#ifdef _WIN32
#define OPENGL_PROC(p, n) PFNGL##p##PROC gl##n = nullptr
#include <rfe/graphics/opengl/gl_proc_table.h>
#undef OPENGL_PROC
#endif

		namespace opengl
		{
			void init()
			{
#ifdef _WIN32
#define OPENGL_PROC(p, n) if(!(gl##n = (PFNGL##p##PROC)wglGetProcAddress("gl"#n))) std::cerr << "Failed to load 'gl" #n "'."
#include <rfe/graphics/opengl/gl_proc_table.h>
#undef OPENGL_PROC
#else
				glewInit();
#endif
			}
		}
	}
}
