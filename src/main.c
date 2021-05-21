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
#define _GNU_SOURCE

#include "core/cache.h"
#include "core/dns.h"
#include "core/host.h"
#include "core/inverse_query.h"
#include "core/logger.h"
#include "core/request_cache.h"
#include "core/socket.h"
#include "test.h"
#include "unidef.h"

#include <assert.h>
#include <ctype.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static const char *const remote_dns[] = { "119.29.29.29", "180.76.76.76",
					  "114.114.114.114", "1.1.1.1" };
static SOCKET rmdns_sks[4];
static SOCKADDR_IN rmdns_info[4];

static void program_start();
static void handle_request(unsigned char id);

static BOOL handle_in_host(request_data *request);
static BOOL handle_in_cache(request_data *request);
static void handle_in_remote_server(unsigned char id, request_data *request,
				    raw_data *remote_data);

int main()
{
	program_start();
	pthread_t hts[4];

	for (int i = 0; i < 4; i++) {
		pthread_create(&hts[i], NULL, (void *)handle_request,
			       (void *)(unsigned long long)i);
		pthread_detach(hts[i]);
	}
	pthread_exit(NULL);
	return 0;
}

static void program_start()
{
	logger_init("./info.log", LOGGER_INFO, LOGGER_TARGET_CONSOLE);

	socket_init();
	request_cache_init();
	init_cache_pools();
	read_host("./host");

	for (size_t i = 0; i < 4; i++) {
		rmdns_sks[i] = socket(AF_INET, SOCK_DGRAM, 0);
		rmdns_info[i].sin_family = AF_INET;
		rmdns_info[i].sin_port = htons(53);
		rmdns_info[i].sin_addr.s_addr = inet_addr(remote_dns[i]);
		logger_write(LOGGER_DEBUG,
			     "Initialize Remote Socket %s succeeded.",
			     remote_dns[i]);
	}
}

static void handle_request(const unsigned char id)
{
	request_data *request;
	raw_data recv_buf;

	logger_write(LOGGER_DEBUG,
		     "handle_request(%u): Create thread succeeded.", id);

	while (1) {
		request = get_request();
		if (request == NULL) {
			sleepms(10);
			continue;
		}

		if (request->size < sizeof(dns_header)) {
			logger_write(
				LOGGER_DEBUG,
				"handle_request(): Request data smaller than dns_header. This may be a fake request. Ignored.");
			free(request);
			continue;
		}

		if (handle_in_host(request)) {
			free(request);
			continue;
		} else if (handle_in_cache(request)) {
			free(request);
			continue;
		} else {
			handle_in_remote_server(id, request, &recv_buf);

			if (recv_buf.size)
				update_cache(&recv_buf);
		}

		free(request);
	}
}

static void handle_in_remote_server(unsigned char id, request_data *request,
				    raw_data *recv_buf)
{
	uint16_t oid = 0, gid = rand();

	oid = get_header_info(request->data, HEADER_ID);
	set_header_info(request->data, HEADER_ID, gid);

	send_to(rmdns_sks[id], &rmdns_info[id], request->data, request->size);
	recv_buf->size = listen_to_async(rmdns_sks[id], &rmdns_info[id],
					 recv_buf->data, 1024, 1);

	if (!recv_buf->size)
		return;

	set_header_info(recv_buf->data, HEADER_ID, oid);

	send_to(get_local_socket(), &request->info, recv_buf->data,
		recv_buf->size);
}

/**
 * inverse query
 */
static BOOL handle_in_cache(request_data *request)
{
#ifdef __DEBUG__
	assert(request != NULL);
#endif

	uint16_t *qtype_ptr =
		get_query_info(request->data, QUERY_TYPE, request->size);

	if (qtype_ptr == NULL) {
		logger_write(
			LOGGER_WARNING,
			"handle_in_cache(): Failed to get query type. This may be a fake query request.");
		return TRUE;
	}

	uint16_t qtype = GET_TYPE_PTR_TYPE(qtype_ptr);
	uint8_t reply[512] = { 0 };
	size_t reply_size = 0;

	if (qtype == TYPE_A) {
		reply_size = inverse_query_a(request, reply);
	} else if (qtype == TYPE_AAAA) {
		reply_size = inverse_query_aaaa(request, reply);
	} else {
		return FALSE;
	}

	if (!reply_size)
		return FALSE;

	send_to(get_local_socket(), &request->info, reply, reply_size);
	return TRUE;
}

static BOOL handle_in_host(request_data *request)
{
#ifdef __DEBUG__
	assert(request != NULL);
#endif
	uint16_t *qtype_ptr =
		get_query_info(request->data, QUERY_TYPE, request->size);

	if (qtype_ptr == NULL) {
		logger_write(
			LOGGER_WARNING,
			"handle_in_cache(): Failed to get query type. This may be a fake query request.");
		return TRUE;
	}

	uint16_t qtype = GET_TYPE_PTR_TYPE(qtype_ptr);

	if (qtype != TYPE_A)
		return FALSE;

	char url[512] = { 0 };
	uint8_t reply[512] = { 0 };
	size_t reply_size = 0;

	if (!get_query_url(request->data, request->size, url, 512)) {
		logger_write(
			LOGGER_WARNING,
			"handle_in_cache(): Failed to get query type. This may be a bad query request.");
		return TRUE;
	}

	char *url_begin = url;
	if (url_begin[0] != '\0' &&
	    (iscntrl(url_begin[0]) || url_begin[0] == '.'))
		url_begin++;

	a_answer_t *ans = host_query(url_begin);

	if (ans == NULL) {
		logger_write(LOGGER_INFO, "handle_in_host(): Url %s not found.",
			     url_begin);
		return FALSE;
	}

	if (ans->ip_addr == 0) {
		logger_write(LOGGER_INFO,
			     "handle_in_host(): Url %s in black list.",
			     url_begin);
		reply_size = generate_no_name_response(
			request->data, request->size, reply, 512);
	} else {
		logger_write(
			LOGGER_INFO,
			"handle_in_host(): Url %s found. ip_addr: %u.%u.%u.%u",
			url, ((uint8_t *)(&ans->ip_addr))[0],
			((uint8_t *)(&ans->ip_addr))[1],
			((uint8_t *)(&ans->ip_addr))[2],
			((uint8_t *)(&ans->ip_addr))[3]);
		memcpy(reply, request->data, request->size);
		reply_size = request->size;

		set_header_info(reply, HEADER_FLAGS, FLAGS_RESPONSE_NO_ERROR);
		set_header_info(reply, HEADER_ANSWER, 1);

		reply_size += generate_single_a_response(0xc00c, ans,
							 reply + reply_size);
	}

	logger_write_raw(LOGGER_INFO, "Query in host(): Url -- %s", reply,
			 reply_size);
	send_to(get_local_socket(), &request->info, reply, reply_size);
	return TRUE;
}
