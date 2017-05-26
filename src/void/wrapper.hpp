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

#ifndef __WRAPPER_HPP_
#define __WRAPPER_HPP_

#include <thread>
#include <future>
#include <unordered_map>

#include <GLES2/gl2.h>

struct gl_shader {
	GLuint program;
	GLuint vertex_shader, fragment_shader;
	GLint proj_uniform;
	GLint tex_uniforms[3];
	GLint alpha_uniform;
	GLint color_uniform;                        
	const char *vertex_source, *fragment_source;

	gl_shader();

	int init();
};


//typedef void (*callback_t)(void *owner, void *data);
//
//struct event_notify_t {
//	callback_t *func;
//	void *data;
//};

class display_wrapper_t {
public:

private:
	bool running;
	bool has_pointer;
	bool has_keyboard;

	void *owner;
	void *userdata;

	gl_shader shader;
	promise<gl_shader *> initialized_shader;

	std::thread *td;

	void init_egl();

	//callback_t frame_callback;
	//callback_t quit_callback;
	function<void()> frame_callback;
	function<void()> quit_callback;
	function<void(int32_t,int32_t)> pointer_enter_callback;
	function<void(uint32_t,int32_t,int32_t)> pointer_motion_callback;
	function<void(uint32_t,uint32_t,uint32_t,
			wayland::pointer_button_state,
			function<void()>
			)> pointer_button_callback;
	//std::unordered_map<std::string, std::function> callback_dict;
	//std::unordered_map<std::string, callback_t> callback_dict;

	int width;
	int height;

public:
	display_wrapper_t();
	~display_wrapper_t();

	void attach(void *buffer);
	void draw(uint32_t serial = 0);
	void commit();
	void run();
	void start();
	void stop();
	void join();
	void dispatch();

	void set_owner(void *owner);
	void set_user_data(void *data);

	void *get_frame_buffer();

	decltype(frame_callback) &on_frame();
	decltype(quit_callback) &on_quit();
	decltype(pointer_enter_callback) &on_pointer_enter();
	decltype(pointer_motion_callback) &on_pointer_motion();
	decltype(pointer_button_callback) &on_pointer_button();
	//int register_callback(std::string event, callback_t f);
	// events: frame, quit
	//template <typename... Args>
	//int register_callback(std::string event,
	//		std::function<void(Args...)> f);

	int get_width();
	int get_height();


	gl_shader *get_shader();
};


void gl_print_error();

#endif

