/**
 * MIT License
 *
 * Copyright (c) 2021 qwqllh
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#define _POSIX_C_SOURCE 200809L

#include "socket.h"
#include "logger.h"
#include "unidef.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>
#include <signal.h>
#include <unistd.h>

static SOCKET local_socket;

static void bind_local_port(const unsigned short port)
{
	local_socket = socket(AF_INET, SOCK_DGRAM, 0);

	SOCKADDR_IN info;
	info.sin_family = AF_INET;
	info.sin_port = htons(port);
	info.sin_addr.s_addr = INADDR_ANY;

	if (bind(local_socket, (SOCKADDR *)&info, sizeof(info))) {
		logger_write(
			LOGGER_ERROR,
			"FATAL: bind_local_port(): Failed to bind local port %u",
			port);
		exit(1);
	} else {
		logger_write(
			LOGGER_DEBUG,
			"bind_local_port(): Listening to 127.0.0.1 port %u.",
			port);
	}
}

void socket_init()
{
	bind_local_port(DNS_PORT);
	logger_write(LOGGER_DEBUG,
		     "socket_init(): Socket initialization finished");
}

void send_to(const SOCKET sock_id, const SOCKADDR_IN *sock_info,
	     const unsigned char *data, const size_t size)
{
	if (sock_info == NULL) {
		logger_write(LOGGER_ERROR,
			     "FATAL: send_to(): NULL socket info. Abort.");
		exit(1);
	}
	if (data == NULL) {
		logger_write(LOGGER_ERROR,
			     "FATAL: send_to(): NULL data. Abort.");
		exit(1);
	}

	if (sendto(sock_id, data, size, 0, (SOCKADDR *)sock_info,
		   sizeof(SOCKADDR_IN)) == SOCKET_ERROR) {
		logger_write(LOGGER_WARNING,
			     "send_to(): Failed to send data. ERROR CODE: %d",
			     errno);
	} else {
		unsigned char *addr =
			(unsigned char *)&(sock_info->sin_addr.s_addr);
		logger_write(LOGGER_DEBUG,
			     "send_to(): Send data to %u.%u.%u.%u succeeded.",
			     addr[0], addr[1], addr[2], addr[3]);
	}
}

size_t listen_to(const SOCKET sock_id, SOCKADDR_IN *sock_info,
		 unsigned char *buffer, const size_t buf_size)
{
	socklen_t temp = sizeof(SOCKADDR_IN);
	size_t res = recvfrom(sock_id, buffer, buf_size, 0,
			      (SOCKADDR *)sock_info, &temp);

	unsigned char *addr = (unsigned char *)&(sock_info->sin_addr.s_addr);
	if (res == SOCKET_ERROR) {
		logger_write(LOGGER_WARNING,
			     "listen_to(): Failed to listen to %u.%u.%u.%u.",
			     addr[0], addr[1], addr[2], addr[3]);
		return 0;
	} else {
		logger_write(LOGGER_DEBUG,
			     "listen_to(): Received data from %u.%u.%u.%u.",
			     addr[0], addr[1], addr[2], addr[3]);
		return res;
	}
}

static void *listen_to_middleware(void *args)
{
	struct {
		SOCKET socket;
		SOCKADDR_IN *sock_info;
		unsigned char *buffer;
		size_t buf_size;
		size_t res;
	} *arg_ptr = args;
	arg_ptr->res = listen_to(arg_ptr->socket, arg_ptr->sock_info,
				 arg_ptr->buffer, arg_ptr->buf_size);
	return NULL;
}

static void *listen_to_timer(void *args)
{
	struct {
		unsigned int sec;
		pthread_t main_thread;
	} *arg_ptr = args;
	sleep(arg_ptr->sec);
	if (!pthread_kill(arg_ptr->main_thread, 0)) {
		logger_write(LOGGER_DEBUG, "Listen to server timeout. skip.");
		pthread_cancel(arg_ptr->main_thread);
	}
	return NULL;
}

size_t listen_to_async(const SOCKET sock_id, SOCKADDR_IN *sock_info,
		       unsigned char *buffer, const size_t buf_size,
		       const unsigned int sec)
{
	pthread_t listen_thread, timer_thread;
	struct {
		SOCKET socket;
		SOCKADDR_IN *sock_info;
		unsigned char *buffer;
		size_t buf_size;
		size_t res;
	} *arg_ptr = malloc(sizeof(struct {
		SOCKET socket;
		SOCKADDR_IN *sock_info;
		unsigned char *buffer;
		size_t buf_size;
		size_t res;
	}));

	arg_ptr->socket = sock_id;
	arg_ptr->sock_info = sock_info;
	arg_ptr->buffer = buffer;
	arg_ptr->buf_size = buf_size;
	arg_ptr->res = 0;

	pthread_create(&listen_thread, NULL, listen_to_middleware, arg_ptr);

	struct {
		unsigned int sec;
		pthread_t main_thread;
	} *timer_arg_ptr = malloc(sizeof(struct {
		unsigned int sec;
		pthread_t main_thread;
	}));

	timer_arg_ptr->main_thread = listen_thread;
	timer_arg_ptr->sec = sec;

	pthread_create(&timer_thread, NULL, listen_to_timer, timer_arg_ptr);
	pthread_detach(timer_thread);
	pthread_join(listen_thread, NULL);

	// Kill timer if received data from remote DNS Server
	if (!pthread_kill(timer_thread, 0)) {
		pthread_cancel(timer_thread);
	}

	size_t res = arg_ptr->res;

	free(arg_ptr);
	free(timer_arg_ptr);
	return res;
}

size_t listen_to_local(SOCKADDR_IN *sock_info, unsigned char *buffer,
		       const size_t buf_size)
{
	return listen_to(local_socket, sock_info, buffer, buf_size);
}

SOCKET get_local_socket()
{
	return local_socket;
}
