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

#include "request_cache.h"
#include "logger.h"
#include "unidef.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>

typedef struct request_node {
	request_data *data;
	struct request_node *next;
} request_node;

typedef struct request_queue {
	size_t size;
	request_node *begin, *end;
} request_queue;

pthread_mutex_t request_cache_mutex;
pthread_t request_cache_thread;
request_queue request_cache_pool;
int request_cache_pool_inited;

static request_node *new_node()
{
	request_node *res = (request_node *)malloc(sizeof(request_node));
	res->data = NULL;
	res->next = NULL;
	return res;
}

static request_data *listen_to_client()
{
	request_data *res = (request_data *)malloc(sizeof(request_data));
	res->size = listen_to_local(&res->info, res->data, REQUEST_BUF_SIZE);
	return res;
}

static void push_back(request_data *data)
{
	request_cache_pool.end->data = data;
	request_cache_pool.end->next = new_node();
	request_cache_pool.end = request_cache_pool.end->next;
}

_Noreturn static void *listener_thread(void *_)
{
	while (1) {
		request_data *request = listen_to_client();
		pthread_mutex_lock(&request_cache_mutex);
		push_back(request);
		pthread_mutex_unlock(&request_cache_mutex);
	}
}

/**
 * 初始化request cache pool，并创建线程持续监听请求。调用request_cache其他相关函数前必须调用该函数
 */
void request_cache_init()
{
	pthread_mutex_init(&request_cache_mutex, NULL);
	request_cache_pool.size = 0;
	request_cache_pool.begin = new_node();
	request_cache_pool.end = request_cache_pool.begin;
	pthread_create(&request_cache_thread, NULL, listener_thread, NULL);
	request_cache_pool_inited = 1;
	logger_write(
		LOGGER_DEBUG,
		"request_cache_init(): Request Cache Pool initialization finished.");
}

/**
 * 检查请求池是否为空
 * @return 若请求池为空则返回true，否则返回false
 */
int no_request()
{
	if (!request_cache_pool_inited) {
		logger_write(
			LOGGER_ERROR,
			"FATAL: no_request(): Request Cache Pool not initialized.");
		exit(1);
	}
	return (request_cache_pool.begin == request_cache_pool.end);
}

/**
 * 从请求池获取一个请求。调用该函数后，请求的指针会被从请求池中移除。
 * 因此，在完全释放指针之前，请调用free释放其空间。
 * @return 若请求池为空则返回NULL，否则返回一个请求的指针
 */
request_data *get_request()
{
	if (!request_cache_pool_inited) {
		logger_write(
			LOGGER_ERROR,
			"FATAL: get_request(): Request Cache Pool not initialized.");
		exit(1);
	}

	pthread_mutex_lock(&request_cache_mutex);
	if (no_request()) {
		// logger_write(LOGGER_DEBUG, "Request Cache Pool Empty.");
		pthread_mutex_unlock(&request_cache_mutex);
		return NULL;
	}
	request_data *res = request_cache_pool.begin->data;
	request_node *temp = request_cache_pool.begin;
	request_cache_pool.begin = request_cache_pool.begin->next;
	free(temp);
	pthread_mutex_unlock(&request_cache_mutex);
	return res;
}