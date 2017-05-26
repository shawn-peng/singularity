/*
 * Copyright (c) 2016-2017 Yisu Peng
 * Copyright (c) 2014, Nils Christopher Brause
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/** \display_wrapper_t opengles.cpp
 * This is an display_wrapper_t of how to use the Wayland C++ bindings with OpenGL ES.
 */

#include <stdexcept>
#include <iostream>
#include <array>
#include <future>

#include <wayland-util.hpp>
#include <wayland-client.hpp>
#include <wayland-egl.hpp>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <linux/input.h>
#include <wayland-cursor.hpp>

#include "wrapper.hpp"
#include "helper.hpp"

using namespace wayland;

// display_wrapper_t Wayland client

#define WIDTH 800
#define HEIGHT 600


/**
 * 		SHADER SOURCES
 */
static const char vertex_shader_source[] =
"attribute vec2 position;\n"
"//attribute vec2 texcoord;\n"
"varying vec2 v_texcoord;\n"
"void main()\n"
"{\n"
"	gl_Position = vec4(position, 0.0, 1.0);\n"
"	v_texcoord = -position * vec2(0.5) + vec2(0.5);\n"
"   //v_texcoord = texcoord;\n"
"}\n";

static const char fragment_debug[] =
"  gl_FragColor = vec4(0.0, 0.3, 0.0, 0.2) + gl_FragColor * 0.8;\n";

static const char fragment_brace[] =
"}\n";

static const char texture_fragment_shader_rgba[] =
"precision mediump float;\n"
"varying vec2 v_texcoord;\n"
"uniform sampler2D tex;\n"
"void main()\n"
"{\n"
"   gl_FragColor = texture2D(tex, v_texcoord);\n"
;

static const char texture_fragment_shader_rgbx[] =
"precision mediump float;\n"
"varying vec2 v_texcoord;\n"
"uniform sampler2D tex;\n"
"uniform float alpha;\n"
"void main()\n"
"{\n"
"   gl_FragColor.rgb = alpha * texture2D(tex, v_texcoord).rgb\n;"
"   gl_FragColor.a = alpha;\n"
;


static int
compile_shader(GLenum type, int count, const char **sources)
{
	GLuint s;
	char msg[512] = {0};
	GLint status;

	s = glCreateShader(type);
	glShaderSource(s, count, sources, NULL);
	glCompileShader(s);
	glGetShaderiv(s, GL_COMPILE_STATUS, &status);
	if (!status) {
		glGetShaderInfoLog(s, sizeof msg, NULL, msg);
		throw std::runtime_error(msg);
		return GL_NONE;
	}

	return s;
}

gl_shader::gl_shader() {
}

int gl_shader::init() {
	const GLchar *vssrc = vertex_shader_source;
	const GLchar *fragment_source = texture_fragment_shader_rgba;
	const char *fssrcs[3];
	fssrcs[0] = fragment_source;
	//fssrcs[1] = fragment_debug;
	fssrcs[1] = fragment_brace;
	int count = 2;
	GLint status;

	//glActiveTexture(GL_TEXTURE0);
	vertex_shader = compile_shader(GL_VERTEX_SHADER, 1, &vssrc);
	fragment_shader = compile_shader(GL_FRAGMENT_SHADER, count, fssrcs);

	program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glBindAttribLocation(program, 0, "position");
	glBindAttribLocation(program, 1, "texcoord");

	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (!status) {
		char msg[512];
		glGetProgramInfoLog(program, sizeof msg, NULL, msg);
		throw std::runtime_error(msg);
		return -1;
	}

	proj_uniform = glGetUniformLocation(program, "proj");
	tex_uniforms[0] = glGetUniformLocation(program, "tex");
	tex_uniforms[1] = glGetUniformLocation(program, "tex1");
	tex_uniforms[2] = glGetUniformLocation(program, "tex2");
	alpha_uniform = glGetUniformLocation(program, "alpha");
	color_uniform = glGetUniformLocation(program, "color");

	return 0;
}






// global objects
static display_client_t display("wayland-0");
static registry_proxy_t registry;
static compositor_proxy_t compositor;
static shell_proxy_t shell;
static seat_proxy_t seat;
static shm_proxy_t shm;

// local objects
static surface_proxy_t surface;
static shell_surface_proxy_t shell_surface;
static pointer_proxy_t pointer;
static keyboard_proxy_t keyboard;
static callback_proxy_t frame_cb;
static cursor_theme_t cursor_theme;
static cursor_image_t cursor_image;
static buffer_proxy_t cursor_buffer;
static surface_proxy_t cursor_surface;

// EGL
static egl_window_t egl_window;
static EGLDisplay egldisplay;
static EGLSurface eglsurface;
static EGLContext eglcontext;


void gl_print_error() {
	GLenum err(glGetError());
	while (err != GL_NO_ERROR) {
		string error;
		switch (err) {
			case GL_INVALID_OPERATION:
				error = "INVALID_OPERATION";
				break;
			case GL_INVALID_ENUM:
				error = "INVALID_ENUM";
				break;
			case GL_INVALID_VALUE:
				error = "INVALID_VALUE";
				break;
			case GL_OUT_OF_MEMORY:
				error = "OUT_OF_MEMORY";
				break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:
				error = "INVALID_FRAMEBUFFER_OPERATION";
				break;
			default:
				error = "Unknown error";
				break;
		}
		std::cout << "GL_" << error;
		err = glGetError();
	}
}

void display_wrapper_t::init_egl() {
	egldisplay = eglGetDisplay(display);
	if(egldisplay == EGL_NO_DISPLAY)
		throw std::runtime_error("eglGetDisplay");

	EGLint major, minor;
	if(eglInitialize(egldisplay, &major, &minor) == EGL_FALSE)
		throw std::runtime_error("eglInitialize");
	if(!((major == 1 && minor >= 4) || major >= 2))
		throw std::runtime_error("EGL version too old");

	//if(eglBindAPI(EGL_OPENGL_API) == EGL_FALSE)
	if(eglBindAPI(EGL_OPENGL_ES_API) == EGL_FALSE)
		throw std::runtime_error("eglBindAPI");

	std::array<EGLint, 13> config_attribs = {{
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
			EGL_RED_SIZE, 8,
			EGL_GREEN_SIZE, 8,
			EGL_BLUE_SIZE, 8,
			EGL_ALPHA_SIZE, 8,
			//EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
			EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
			EGL_NONE
	}
	};

	EGLConfig config;
	EGLint num;
	if(eglChooseConfig(egldisplay, config_attribs.data(), &config, 1, &num) == EGL_FALSE || num == 0) {
		throw std::runtime_error("eglChooseConfig");
	}

	std::array<EGLint, 3> context_attribs = {{
		EGL_CONTEXT_CLIENT_VERSION, 2,
			EGL_NONE
	}
	};

	eglcontext = eglCreateContext(egldisplay, config, EGL_NO_CONTEXT, context_attribs.data());
	if(eglcontext == EGL_NO_CONTEXT)
		throw std::runtime_error("eglCreateContext");

	eglsurface = eglCreateWindowSurface(egldisplay, config, egl_window, NULL);
	if(eglsurface == EGL_NO_SURFACE)
		throw std::runtime_error("eglCreateWindowSurface");

	if(eglMakeCurrent(egldisplay, eglsurface, eglsurface, eglcontext) == EGL_FALSE)
		throw std::runtime_error("eglMakeCurrent");
}

void display_wrapper_t::draw(uint32_t serial) {
	// draw stuff
	glClearColor(0, 0, 0, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);


	// schedule next draw
	frame_cb = surface.frame();
	frame_cb.on_done() = bind_mem_fn(&display_wrapper_t::draw, this);

	//callback_t func = callback_dict["frame"];
	//if (func) {
	//	func(owner, userdata);
	//}
	frame_callback();

	// swap buffers
	if(eglSwapBuffers(egldisplay, eglsurface) == EGL_FALSE)
		throw std::runtime_error("eglSwapBuffers");
}


display_wrapper_t::display_wrapper_t() {
	width = WIDTH;
	height = HEIGHT;

	display = display_client_t(std::string("wayland-0"));
	// retrieve global objects
	registry = display.get_registry();
	registry.on_global() = [&](uint32_t name, std::string interface, uint32_t version) {
		if(interface == "wl_compositor")
			registry.bind(name, compositor, version);
		else if(interface == "wl_shell")
			registry.bind(name, shell, version);
		else if(interface == "wl_seat")
			registry.bind(name, seat, version);
		else if(interface == "wl_shm")
			registry.bind(name, shm, version);
	};
	display.dispatch();

	seat.on_capabilities() = [&](seat_capability capability) {
		has_keyboard = capability & seat_capability::keyboard;
		has_pointer = capability & seat_capability::pointer;
	};
	display.dispatch();

	if(!has_keyboard)
		throw std::runtime_error("No keyboard found.");
	if(!has_pointer)
		throw std::runtime_error("No pointer found.");

	// create a surface
	surface = compositor.create_surface();
	shell_surface = shell.get_shell_surface(surface);

	shell_surface.on_ping() = [&](uint32_t serial) {
		shell_surface.pong(serial);
	};
	shell_surface.set_title("Window");
	shell_surface.set_toplevel();

	// Get input devices
	pointer = seat.get_pointer();
	keyboard = seat.get_keyboard();

	// load cursor theme
	cursor_theme = cursor_theme_t("default", 16, shm);
	cursor_t cursor = cursor_theme.get_cursor("arrow");
	cursor_image = cursor.image(0);
	cursor_buffer = cursor_image.get_buffer();

	// create cursor surface
	cursor_surface = compositor.create_surface();

	// draw cursor
	pointer.on_enter() = [&](uint32_t serial, surface_proxy_t surf_proxy, fixed_t x, fixed_t y) {
		cursor_surface.attach(cursor_buffer, 0, 0);
		cursor_surface.damage(0, 0, cursor_image.width(), cursor_image.height());
		cursor_surface.commit();
		pointer.set_cursor(serial, cursor_surface, 0, 0);

		//pointer_enter_callback(owner, serial, surf_proxy, x, y);
	};

	// window movement
	pointer.on_button() = [&](uint32_t serial, uint32_t time, uint32_t button, pointer_button_state state) {
		auto wrapper_on_buttion = [&]() {
			if(button == BTN_LEFT && state == pointer_button_state::pressed) {
				shell_surface.move(seat, serial);
			}
		};
		if (pointer_button_callback) {
			pointer_button_callback(serial, time, button, state, wrapper_on_buttion);
		} else {
			wrapper_on_buttion();
		}
	};

	pointer.on_motion() = [&](uint32_t time, fixed_t surface_x, fixed_t surface_y) {
		pointer_motion_callback(time, surface_x, surface_y);
	};

	// press 'q' to exit
	keyboard.on_key() = [&](uint32_t, uint32_t, uint32_t key, keyboard_key_state state) {
		if(key == KEY_Q && state == keyboard_key_state::pressed) {
			running = false;
		}
	};

	// intitialize egl
	//egl_window = egl_window_t(surface, 320, 240);
	//init_egl();

	// draw stuff
	//draw();
}

display_wrapper_t::~display_wrapper_t() {
	// finialize EGL
	//if(eglDestroyContext(egldisplay, eglcontext) == EGL_FALSE)
	//	throw std::runtime_error("eglDestroyContext");
	//if(eglTerminate(egldisplay) == EGL_FALSE)
	//	throw std::runtime_error("eglTerminate");
	eglDestroyContext(egldisplay, eglcontext);
	eglTerminate(egldisplay);
}

void display_wrapper_t::start() {
	td = new std::thread(&display_wrapper_t::run, this);
}

void display_wrapper_t::stop() {
	running = false;
	//td->stop();
}

void display_wrapper_t::join() {
	td->join();
}

void display_wrapper_t::run() {
	// intitialize egl
	egl_window = egl_window_t(surface, width, height);
	init_egl();

	shader.init();
	initialized_shader.set_value(&shader);

	// draw stuff
	draw();

	// event loop
	running = true;
	while(running)
		display.dispatch();
}

void display_wrapper_t::dispatch() {
	display.dispatch();
}

void display_wrapper_t::set_owner(void *owner) {
	this->owner = owner;
}

void display_wrapper_t::set_user_data(void *data) {
	this->userdata = data;
}

void *display_wrapper_t::get_frame_buffer() {
	return NULL;
}

//int display_wrapper_t::register_callback(std::string event, callback_t f) {
//	callback_dict[event] = f;
//}
//int display_wrapper_t::register_callback(std::string event, std::function f) {
//	callback_dict[event] = f;
//}

//auto display_wrapper_t::on_frame() {
decltype(display_wrapper_t::frame_callback) &
display_wrapper_t::on_frame() {
	return frame_callback;
}
decltype(display_wrapper_t::quit_callback) &
display_wrapper_t::on_quit() {
	return quit_callback;
}
decltype(display_wrapper_t::pointer_enter_callback) &
display_wrapper_t::on_pointer_enter() {
	return pointer_enter_callback;
}
decltype(display_wrapper_t::pointer_motion_callback) &
display_wrapper_t::on_pointer_motion() {
	return pointer_motion_callback;
}
decltype(display_wrapper_t::pointer_button_callback) &
display_wrapper_t::on_pointer_button() {
	return pointer_button_callback;
}

gl_shader *display_wrapper_t::get_shader() {
	std::future<gl_shader *> futp = initialized_shader.get_future();
	return futp.get();
}

int display_wrapper_t::get_width() {
	return width;
}

int display_wrapper_t::get_height() {
	return height;
}




