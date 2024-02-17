#include "learngl.hpp"
#include "opengl.h"

static const char *vertex_shader_source = R"glsl(
#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 color;
out vec3 frag_color;

void main() {
	gl_Position = vec4(pos.x, pos.y, pos.z, 1.0f);
	frag_color = color;
}
)glsl";

static const char *fragment_shader_source = R"glsl(
#version 330 core

in vec3 frag_color;
out vec4 final_color;

void main() {
	final_color = vec4(frag_color.xyz, 1.0f);
}
)glsl";

static f32 rectangle_vertices[] = {
	// x      y     z     r     g     b
	0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, // top right
	0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, // bottom right
	-0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom left
	-0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, // top left
};

static u32 rectangle_indices[] = {
	0, 1, 3, // first triangle
	1, 2, 3, // second triangle
};

static bool render_initialize()
{
	u32 vertex_array;
	glGenVertexArrays(1, &vertex_array);
	glBindVertexArray(vertex_array);
		
	u32 vertex_buffer;
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rectangle_vertices), rectangle_vertices, GL_STATIC_DRAW);

	u32 element_array;
	glGenBuffers(1, &element_array);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_array);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(rectangle_indices), rectangle_indices, GL_STATIC_DRAW);
		
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (6 * sizeof(f32)), (void *)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (6 * sizeof(f32)), (void *)(3 * sizeof(f32)));

	u32 vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_shader_source, nullptr);
	glCompileShader(vertex_shader);
		
	int compiled;
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled) {
		char info_log[512];
		glGetShaderInfoLog(vertex_shader, sizeof(info_log), NULL, info_log);
		OutputDebugStringA(info_log);
		return false;
	}
		
	u32 fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_shader_source, nullptr);
	glCompileShader(fragment_shader);

	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled) {
		char info_log[512];
		glGetShaderInfoLog(fragment_shader, sizeof(info_log), NULL, info_log);
		OutputDebugStringA(info_log);
		return false;
	}

	u32 shader_program = glCreateProgram();
	glAttachShader(shader_program, vertex_shader);
	glAttachShader(shader_program, fragment_shader);
	glLinkProgram(shader_program);

	int linked;
	glGetProgramiv(shader_program, GL_COMPILE_STATUS, &linked);
	if (!linked) {
		char info_log[512];
		glGetProgramInfoLog(shader_program, sizeof(info_log), NULL, info_log);
		OutputDebugStringA(info_log);
		return false;
	}
		
	glUseProgram(shader_program);
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	return true;
}

static void render_draw()
{
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}
