#ifndef OPENGL_H

#include <GL/gl.h>

#ifdef _WIN32
#   define CALLING_CONVENTION WINAPI
#   define GL_ARRAY_BUFFER    0x8892
#   define GL_COMPILE_STATUS  0x8B81
#   define GL_FRAGMENT_SHADER 0x8B30
#   define GL_STATIC_DRAW     0x88E4
#   define GL_VERTEX_SHADER   0x8B31
#   define GLchar char
#else
#   define CALLING_CONVENTION
#endif

/* List of OpenGL functions used in the program.
   Follow the format "GL(RETURN_TYPE, FUNCTION_NAME, ARGUMENTS)". */
#define OPENGL_FUNCTION_LIST											\
	GL(void, glAttachShader, (GLuint, GLuint))							\
	GL(void, glBindBuffer, (GLenum, GLuint))							\
	GL(void, glBindVertexArray, (GLuint))								\
	GL(void, glBufferData, (GLenum, GLsizei, const GLvoid *, GLenum))	\
	GL(void, glCompileShader, (GLuint))									\
	GL(GLuint, glCreateShader, (GLenum))								\
	GL(GLuint, glCreateProgram, (void))									\
	GL(void, glDeleteShader, (GLuint))									\
	GL(void, glEnableVertexAttribArray, (GLuint))						\
	GL(void, glGenBuffers, (GLsizei, GLuint *))							\
	GL(void, glGenVertexArrays, (GLsizei, GLuint *))					\
	GL(void, glGetProgramInfoLog, (GLuint, GLsizei, GLsizei *, GLchar *)) \
	GL(void, glGetShaderInfoLog, (GLuint, GLsizei, GLsizei *, GLchar *)) \
	GL(void, glGetProgramiv, (GLuint, GLenum, GLint *))					\
	GL(void, glGetShaderiv, (GLuint, GLenum, GLint *))					\
	GL(void, glLinkProgram, (GLuint))									\
	GL(void, glShaderSource, (GLuint, GLsizei, const GLchar *const *, const GLint *)) \
	GL(void, glUseProgram, (GLuint))									\
	GL(void, glVertexAttribPointer, (GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid *)) \

/* Generate function typedefs. */
#define GL(RETURN_TYPE, FUNCTION_NAME, ARGUMENTS)						\
	typedef RETURN_TYPE(CALLING_CONVENTION *FUNCTION_NAME##Fn)ARGUMENTS;
OPENGL_FUNCTION_LIST
#undef GL

/* Generate function pointer declarations.
   Define OPENGL_LOADER before including this header in one file of the program.  */
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
