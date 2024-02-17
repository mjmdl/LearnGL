#ifndef OPENGL_H

/* IMPORTANT: List of OpenGL functions used in the program. */
/* NOTE: Follow the format GL(RETURN_TYPE, FUNCTION_NAME, ARGUMENTS) */
#define OPENGL_FUNCTION_LIST					\
	GL(void, glAttachShader, (GLuint, GLuint))	\
	GL(GLuint, glCreateProgram, (void))			\
	GL(GLuint, glCreateShader, (GLenum))

/* NOTE: Generate function typedefs */
#define GL(RETURN_TYPE, FUNCTION_NAME, ARGUMENTS)				\
	typedef RETURN_TYPE(WINAPI *FUNCTION_NAME##Fn)ARGUMENTS;
OPENGL_FUNCTION_LIST
#undef GL

/* NOTE: Generate function pointer declarations. */
/* IMPORTANT: Define OPENGL_LOADER before including this header in
   one file of the program. */
#ifdef OPENGL_LOADER
#   define GL(RETURN_TYPE, FUNCTION_NAME, ARGUMENTS)	\
	FUNCTION_NAME##Fn FUNCTION_NAME;
#else
#   define GL(RETURN_TYPE, FUNCTION_NAME, ARGUMENTS)	\
	extern FUNCTION_NAME##Fn FUNCTION_NAME;
#endif
OPENGL_FUNCTION_LIST
#undef GL

#define OPENGL_H
#endif
