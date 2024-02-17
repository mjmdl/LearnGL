#include <windows.h>
#include <GL/gl.h>

#define OPENGL_LOADER
#include "opengl.h"

// STUDY: Where are these defined?
#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_FLAGS_ARB 0x2094
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126

#define USE_MODERN_OPENGL 1

namespace windows {
	static constexpr const char *WindowName = "Learn OpenGL";
	static constexpr const char *WindowClassName = "LearnGL";

	static LRESULT CALLBACK window_callback(
			HWND window, UINT message,
			WPARAM w_param, LPARAM l_param);
}
	
static LRESULT CALLBACK windows::window_callback(
		HWND window, UINT message,
		WPARAM w_param, LPARAM l_param) {
	switch (message) {
	case WM_CLOSE:
		DestroyWindow(window);
		return 0;		
		
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
			
	default:
		return DefWindowProc(window, message, w_param, l_param);
	}
}

int APIENTRY WinMain(
		HINSTANCE instance, HINSTANCE previous_instance,
		LPSTR command_line, int command_show) {
	WNDCLASSEXA window_class = {};
	window_class.cbSize = sizeof(window_class);
	window_class.style = CS_OWNDC;
	window_class.lpfnWndProc = windows::window_callback;
	window_class.hInstance = instance;
	window_class.lpszClassName = windows::WindowClassName;
	if (!RegisterClassExA(&window_class)) {
		MessageBoxA(
				nullptr,
				"Failed to register the window class.",
				"Windows Error",
				(MB_ICONERROR | MB_OK));
		return FALSE;
	}

	int screen_width = GetSystemMetrics(SM_CXSCREEN);
	int screen_height = GetSystemMetrics(SM_CYSCREEN);
	HWND window = CreateWindowExA(
			0, windows::WindowClassName, windows::WindowName,
			WS_OVERLAPPEDWINDOW,
			(screen_width / 8), (screen_height / 8),
			(screen_width * 3 / 4), (screen_height * 3 / 4),	
			nullptr, nullptr, instance, nullptr);
	if (!window) {
		MessageBoxA(
				nullptr, "Failed to create the window.",
				"Windows Error", (MB_ICONERROR | MB_OK));
		return FALSE;
	}

	HDC device_context = GetDC(window);
	if (!device_context) {
		MessageBoxA(
				nullptr, "Failed to retrieve the window device context.",
				"Windows Error", (MB_ICONERROR | MB_OK));
		return FALSE;
	}

	PIXELFORMATDESCRIPTOR pixel_format_desc = {};
	pixel_format_desc.nSize = sizeof(pixel_format_desc);
	pixel_format_desc.nVersion = 1;
	pixel_format_desc.dwFlags = (
			PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER);
	pixel_format_desc.iPixelType = PFD_TYPE_RGBA;
	pixel_format_desc.cColorBits = 32;
	pixel_format_desc.cDepthBits = 24;
	pixel_format_desc.cAlphaBits = 8;
	pixel_format_desc.cStencilBits = 8;
	int pixel_format = ChoosePixelFormat(device_context, &pixel_format_desc);
	if (!pixel_format) {
		MessageBoxA(
				nullptr, "Failed to retrieve a valid pixel format.",
				"Windows Error", (MB_ICONERROR | MB_OK));
		return FALSE;
	}
	if (!SetPixelFormat(device_context, pixel_format, &pixel_format_desc)) {
		MessageBoxA(
				nullptr, "Failed to set the window pixel format.",
				"Windows Error", (MB_ICONERROR | MB_OK));
		return FALSE;
	}

	/* NOTE: To create a modern OpenGL context, we first have to create
	   a legacy OpenGL context. */
	/* STUDY: Do we also need a dummy window for a modern OpenGL
	   "compatible" pixel format? */
	HGLRC legacy_render_context = wglCreateContext(device_context);
	if (!legacy_render_context ||
			!wglMakeCurrent(device_context, legacy_render_context)) {
		MessageBoxA(
				nullptr, "Failed to create the legacy OpenGL context.",
				"OpenGL Error", (MB_ICONERROR | MB_OK));
		return FALSE;
	}

	/* NOTE: Load the OpenGL modern creation function. */
	/* NOTE: We need the legacy context by now. */
	typedef HGLRC(WINAPI *ModernOpenGlFn)(HDC, HGLRC, const int *);
	auto wglCreateContextAttribsARB = (ModernOpenGlFn)wglGetProcAddress(
			"wglCreateContextAttribsARB");
	if (!wglCreateContextAttribsARB) {
		MessageBoxA(
				nullptr, "Failed to load the modern OpenGL context creator function.",
				"OpenGL Error", (MB_ICONERROR | MB_OK));
		return FALSE;
	}

	const int OpenGlAttribs[] = {
#if USE_MODERN_OPENGL
		WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
		WGL_CONTEXT_MINOR_VERSION_ARB, 3,
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
#endif
		0
	};
	HGLRC render_context = wglCreateContextAttribsARB(
			device_context,
			nullptr,
			OpenGlAttribs);
	if (!render_context || !wglMakeCurrent(device_context, render_context)) {
		MessageBoxA(
				nullptr, "Failed to create the modern OpenGL context.",
				"OpenGL Error", (MB_ICONERROR | MB_OK));
		return FALSE;		
	}
	/* NOTE: We don't need the legacy context anymore. */
	wglDeleteContext(legacy_render_context);
	legacy_render_context = nullptr;

	/* NOTE: Load the OpenGL functions. */
	/* NOTE: We must have a context. */
#define GL(RETURN_TYPE, FUNCTION_NAME, ARGUMENTS)						\
	FUNCTION_NAME = (FUNCTION_NAME##Fn)wglGetProcAddress(#FUNCTION_NAME); \
	if (!FUNCTION_NAME) {												\
		MessageBoxA(													\
				nullptr, "Failed to load the OpenGL function " #FUNCTION_NAME ".", \
				"OpenGL Error", (MB_ICONERROR | MB_OK));				\
		return FALSE;													\
	}
	OPENGL_FUNCTION_LIST;
#undef GL
	
	/* NOTE: Clear and show the window. */
	glClearColor(0.17f, 0.17f, 0.17f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	SwapBuffers(device_context);
	ShowWindow(window, command_show);
	UpdateWindow(window);
	
	bool keep_running = true;
	MSG message = {};

	/* NOTE: Main game loop. */
	while (keep_running) {
		/* NOTE: Handle win32 events */
		while (PeekMessageA(&message, nullptr, 0, 0, PM_REMOVE)) {
			if (message.message == WM_QUIT) {
				/* NOTE: We received the PostQuitMessage! */
				keep_running = false;
			} else {
				/* NOTE: Dispatch to our callback procedure. */
				TranslateMessage(&message);
				DispatchMessage(&message);
			}
		}
		
		glClear(GL_COLOR_BUFFER_BIT);

#if !USE_MODERN_OPENGL
		/* NOTE: Render RGB triangle */
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
	}

	return (int)message.wParam;
}

