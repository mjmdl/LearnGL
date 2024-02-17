#include <stdint.h>
#include <stdio.h>

#include <windows.h>
#include <GL/gl.h>

#define OPENGL_LOADER
#include "opengl.h"

/* Where are these defined? */
#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_FLAGS_ARB 0x2094
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126

#define USE_MODERN_OPENGL 1
#define OPENGL_MAJOR 3
#define OPENGL_MINOR 3

#define MAIN_LOOP_HERTZ 60
#define MAIN_LOOP_FRAME_TIME (1.0f / (float)MAIN_LOOP_HERTZ)

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef float f32;
typedef double f64;
typedef size_t usize;

static u64 processor_frequency;

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

static inline u64 get_ticks(void) {
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	return counter.QuadPart;
}

static inline i64 get_cycles(void) {
	i64 cycles = __rdtsc();
	return cycles;
}

static inline f32 get_delta_time(u64 start_ticks, u64 end_ticks) {
	f32 delta = (f32)(end_ticks - start_ticks);
	f32 time = (delta / processor_frequency);
	return time;
}

static LRESULT CALLBACK window_procedure(HWND window, UINT message, WPARAM w_param, LPARAM l_param) {
	switch (message) {
		case WM_CLOSE: {
			DestroyWindow(window);
			return 0;
		}
		case WM_DESTROY: {
			PostQuitMessage(0);
			return 0;
		}
		case WM_SIZE: {
			glViewport(0, 0, LOWORD(l_param), HIWORD(l_param));
			return 0;
		}
		default: {
			return DefWindowProc(window, message, w_param, l_param);
		}
	}
}

int APIENTRY WinMain(HINSTANCE instance, HINSTANCE previous_instance, LPSTR command_line, int command_show) {
	{
		LARGE_INTEGER performance_frequency;
		QueryPerformanceFrequency(&performance_frequency);
		processor_frequency = performance_frequency.QuadPart;
	}

	u64 last_ticks = get_ticks();
	i64 last_cycles = get_cycles();

	u32 scheduler_ms = 1;
	bool sleep_is_granular = (timeBeginPeriod(scheduler_ms) != TIMERR_NOERROR);	
	
	WNDCLASSEXA window_class = {};
	window_class.cbSize = sizeof(window_class);
	window_class.style = CS_OWNDC;
	window_class.lpfnWndProc = window_procedure;
	window_class.hInstance = instance;
	window_class.lpszClassName = "LearnGL";
	if (!RegisterClassExA(&window_class)) {
		MessageBoxA(nullptr, "Failed to register the window class.", "Windows Error", (MB_ICONERROR | MB_OK));
		return FALSE;
	}

	int screen_width = GetSystemMetrics(SM_CXSCREEN);
	int screen_height = GetSystemMetrics(SM_CYSCREEN);
	HWND window = CreateWindowExA(
		0, window_class.lpszClassName, "Learn GL",
		WS_OVERLAPPEDWINDOW,
		(screen_width / 8), (screen_height / 8),
		(screen_width * 3 / 4), (screen_height * 3 / 4),	
		nullptr, nullptr, instance, nullptr);
	if (!window) {
		MessageBoxA(nullptr, "Failed to create the window.", "Windows Error", (MB_ICONERROR | MB_OK));
		return FALSE;
	}

	HDC device_context = GetDC(window);
	if (!device_context) {
		MessageBoxA(nullptr, "Failed to retrieve the window device context.", "Windows Error", (MB_ICONERROR | MB_OK));
		return FALSE;
	}

	PIXELFORMATDESCRIPTOR pixel_format_desc = {};
	pixel_format_desc.nSize = sizeof(pixel_format_desc);
	pixel_format_desc.nVersion = 1;
	pixel_format_desc.dwFlags = (PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER);
	pixel_format_desc.iPixelType = PFD_TYPE_RGBA;
	pixel_format_desc.cColorBits = 32;
	pixel_format_desc.cDepthBits = 24;
	pixel_format_desc.cAlphaBits = 8;
	pixel_format_desc.cStencilBits = 8;
	int pixel_format = ChoosePixelFormat(device_context, &pixel_format_desc);
	if (!pixel_format) {
		MessageBoxA(nullptr, "Failed to retrieve a valid pixel format.", "Windows Error", (MB_ICONERROR | MB_OK));
		return FALSE;
	}
	if (!SetPixelFormat(device_context, pixel_format, &pixel_format_desc)) {
		MessageBoxA(nullptr, "Failed to set the window pixel format.", "Windows Error", (MB_ICONERROR | MB_OK));
		return FALSE;
	}
	
	/* To create a modern OpenGL context, we first have to create a legacy OpenGL context.
	   Do we also need a dummy window for a modern OpenGL "compatible" pixel format?  */
	HGLRC legacy_render_context = wglCreateContext(device_context);
	if (!legacy_render_context || !wglMakeCurrent(device_context, legacy_render_context)) {
		MessageBoxA(nullptr, "Failed to create the legacy OpenGL context.", "OpenGL Error", (MB_ICONERROR | MB_OK));
		return FALSE;
	}

	/* Load the OpenGL modern creation function.
	   We need the legacy context by now.  */
	typedef HGLRC(WINAPI *ModernOpenGlFn)(HDC, HGLRC, const int *);
	auto wglCreateContextAttribsARB = (ModernOpenGlFn)wglGetProcAddress("wglCreateContextAttribsARB");
	if (!wglCreateContextAttribsARB) {
		MessageBoxA(nullptr, "Failed to load the modern OpenGL context creator function.", "OpenGL Error", (MB_ICONERROR | MB_OK));
		return FALSE;
	}

	const int opengl_attribs[] = {
#if USE_MODERN_OPENGL
		WGL_CONTEXT_MAJOR_VERSION_ARB, OPENGL_MAJOR,
		WGL_CONTEXT_MINOR_VERSION_ARB, OPENGL_MINOR,
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
#endif
		0
	};
	HGLRC render_context = wglCreateContextAttribsARB(device_context, nullptr, opengl_attribs);
	if (!render_context || !wglMakeCurrent(device_context, render_context)) {
		MessageBoxA(nullptr, "Failed to create the modern OpenGL context.", "OpenGL Error", (MB_ICONERROR | MB_OK));
		return FALSE;		
	}
	/* We don't need the legacy context anymore.  */
	wglDeleteContext(legacy_render_context);
	legacy_render_context = nullptr;

	/* Load the OpenGL functions.
	   We must have a context. */
#define GL(RETURN_TYPE, FUNCTION_NAME, ARGUMENTS)						\
	FUNCTION_NAME = (FUNCTION_NAME##Fn)wglGetProcAddress(#FUNCTION_NAME); \
	if (!FUNCTION_NAME) {												\
		MessageBoxA(nullptr, "Failed to load the OpenGL function " #FUNCTION_NAME ".", "OpenGL Error", (MB_ICONERROR | MB_OK)); \
		return FALSE;													\
	}
    OPENGL_FUNCTION_LIST;
#undef GL
	
	/* Clear and show the window.  */
	glClearColor(0.17f, 0.17f, 0.17f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	SwapBuffers(device_context);
	ShowWindow(window, command_show);
	UpdateWindow(window);

	{
		f32 rectangle_vertices[] = {
			// x      y     z     r     g     b
			 0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, // top right
			 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, // bottom right
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom left
 			-0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, // top left
		};
		u32 rectangle_indices[] = {
			0, 1, 3, // first triangle
			1, 2, 3, // second triangle
		};

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
			return FALSE;
		}
		
		u32 fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment_shader, 1, &fragment_shader_source, nullptr);
		glCompileShader(fragment_shader);

		glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compiled);
		if (!compiled) {
			char info_log[512];
			glGetShaderInfoLog(fragment_shader, sizeof(info_log), NULL, info_log);
			OutputDebugStringA(info_log);
			return FALSE;
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
			return FALSE;
		}
		
		glUseProgram(shader_program);
		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);
	}
	
	bool keep_running = true;
	MSG message = {};

	/* Main game loop.  */
	while (keep_running) {
		/* Handle win32 events  */
		while (PeekMessageA(&message, nullptr, 0, 0, PM_REMOVE)) {
			if (message.message == WM_QUIT) {
				/* We received the PostQuitMessage!  */
				keep_running = false;
			} else {
				/* Dispatch to our callback procedure.  */
				TranslateMessage(&message);
				DispatchMessage(&message);
			}
		}
		
		glClear(GL_COLOR_BUFFER_BIT);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		
#if !USE_MODERN_OPENGL
		/* Render RGB triangle  */
		glBegin(GL_TRIANGLES);
		glColor3f(1.0f, 0.0f, 0.0f);
		glVertex2f(0.0f, 0.5f);
		glColor3f(0.0f, 1.0f, 0.0f);
		glVertex2f(-0.5f, -0.5f);
		glColor3f(0.0f, 0.0f, 1.0f);
		glVertex2f(0.5f, -0.5f);
		glEnd();
#endif
		
		SwapBuffers(device_context);

		/* Fix the frame rate.  */
		{
			u64 test_ticks = get_ticks();
			f32 test_delta = get_delta_time(last_ticks, test_ticks);
			f32 work_delta = test_delta;

			/* Wait untill end of frame.  */
			while (work_delta < MAIN_LOOP_FRAME_TIME) {
				if (sleep_is_granular) {
					f32 delta = (MAIN_LOOP_FRAME_TIME - work_delta);
					u32 time_left = (u32)(1000.0f * delta);
					
					/* Sleep while it can  */
					if (time_left > 0) {
						Sleep(time_left);
					}
				}
				
				/* Melt the time left the sleep couldn't wait.  */
				while (work_delta < MAIN_LOOP_FRAME_TIME) {
					u64 ticks = get_ticks();
					work_delta = get_delta_time(last_ticks, ticks);
				}
			}
			
			u64 end_ticks = get_ticks();
			f32 delta_time = get_delta_time(last_ticks, end_ticks);
			f32 frame_ms = (1000.0f * delta_time);
			last_ticks = end_ticks;

			i64 end_cycles = get_cycles();
			i64 delta_cycles = (end_cycles - last_cycles);
			last_cycles = end_cycles;

			f32 second_frames = (1000.0f / frame_ms);
			f32 frame_mhz = ((f32)delta_cycles / 1'000'000.0f);

			char log_buffer[256];
			sprintf_s(log_buffer, sizeof(log_buffer), "%.2f FPS | %.2f MS | %.2f MHz\n", second_frames, frame_ms, frame_mhz);
			OutputDebugStringA(log_buffer);
		}
	}

	return (int)message.wParam;
}

