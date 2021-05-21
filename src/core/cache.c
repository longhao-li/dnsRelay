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

#include "cache.h"
#include "dns.h"
#include "logger.h"
#include "model/trie.h"
#include "unidef.h"

#include <assert.h>
#include <pthread.h>
#include <string.h>

static pthread_rwlock_t a_rec_pool_lock;
static pthread_rwlock_t cname_rec_pool_lock;
static pthread_rwlock_t aaaa_rec_pool_lock;

static trie a_rec_pool;
static trie cname_rec_pool;
static trie aaaa_rec_pool;

void init_cache_pools(void)
{
	pthread_rwlock_init(&a_rec_pool_lock, NULL);
	pthread_rwlock_init(&cname_rec_pool_lock, NULL);
	pthread_rwlock_init(&aaaa_rec_pool_lock, NULL);
	logger_write(LOGGER_DEBUG, "cache(): Cache initializetion finished.");
}

static void update_A_record(const char *domain, list *records)
{
	size_t len = strlen(domain);
	list *ori_list = trie_remove(&a_rec_pool, domain, len);
	trie_insert(&a_rec_pool, domain, len, records);

	/* 删除之前的记录 */
	if (ori_list != NULL) {
		foreach_list(i, ori_list) free(i->value);

		while (list_empty(ori_list))
			list_pop_back(ori_list);
		free(ori_list);
	}
}

static list *__query_A_record_no_lock(const char *domain)
{
	size_t len = strlen(domain);
	list *res = (list *)trie_find(&a_rec_pool, domain, len);
	return res;
}

list *query_A_record(const char *domain)
{
	pthread_rwlock_rdlock(&a_rec_pool_lock);
	size_t len = strlen(domain);
	list *res = (list *)trie_find(&a_rec_pool, domain, len);
	pthread_rwlock_unlock(&a_rec_pool_lock);
	return res;
}

static void update_CNAME_record(const char *domain, cname_answer_t *records)
{
	size_t len = strlen(domain);
	cname_answer_t *ori_rec = trie_remove(&cname_rec_pool, domain, len);
	trie_insert(&cname_rec_pool, domain, len, records);

	free(ori_rec);
}

cname_answer_t *query_CNAME_record(const char *domain)
{
	pthread_rwlock_rdlock(&cname_rec_pool_lock);
	size_t len = strlen(domain);
	cname_answer_t *res = (cname_answer_t *)trie_find(&cname_rec_pool, domain, len);
	pthread_rwlock_unlock(&cname_rec_pool_lock);
	return res;
}

static void update_AAAA_record(const char *domain, list *records)
{
	size_t len = strlen(domain);
	list *ori_list = trie_remove(&aaaa_rec_pool, domain, len);
	trie_insert(&aaaa_rec_pool, domain, len, records);

	/* 删除之前的记录 */
	if (ori_list != NULL) {
		foreach_list(i, ori_list) free(i->value);

		while (list_empty(ori_list))
			list_pop_back(ori_list);
		free(ori_list);
	}
}

static list *__query_AAAA_record_no_lock(const char *domain)
{
	size_t len = strlen(domain);
	list *res = (list *)trie_find(&aaaa_rec_pool, domain, len);
	return res;
}

list *query_AAAA_record(const char *domain)
{
	pthread_rwlock_rdlock(&aaaa_rec_pool_lock);
	size_t len = strlen(domain);
	list *res = (list *)trie_find(&aaaa_rec_pool, domain, len);
	pthread_rwlock_unlock(&aaaa_rec_pool_lock);
	return res;
}


static void try_update_cname_cache(answer_t *answers, size_t num_answer)
{
#ifdef __DEBUG__
	assert(answers != NULL);
#endif
	if (num_answer == 0)
		return;

	pthread_rwlock_wrlock(&cname_rec_pool_lock);

	for (size_t i = 0; i < num_answer; i++) {
		if (answers[i].type == TYPE_CNAME) {
			cname_answer_t *rec =
				(cname_answer_t *)answers[i].answer;
			rec->last_update = time(NULL);
			update_CNAME_record(rec->domain, rec);
		}
	}

	pthread_rwlock_unlock(&cname_rec_pool_lock);
}

static void try_update_a_cache(answer_t *answers, size_t num_answer)
{
#ifdef __DEBUG__
	assert(answers != NULL);
#endif
	if (num_answer == 0)
		return;

	pthread_rwlock_wrlock(&a_rec_pool_lock);

	for (size_t i = 0; i < num_answer; i++) {
		if (answers[i].type == TYPE_A) {
			a_answer_t *rec = (a_answer_t *)answers[i].answer;
			list *lst = __query_A_record_no_lock(rec->domain);
			list_clear(lst);
		}
	}

	for (size_t i = 0; i < num_answer; i++) {
		if (answers[i].type == TYPE_A) {
			a_answer_t *rec = (a_answer_t *)answers[i].answer;
			rec->last_update = time(NULL);
			list *lst = __query_A_record_no_lock(rec->domain);

			if (lst == NULL) {
				lst = create_list();
				update_A_record(rec->domain, lst);
			} else {
				list_clear(lst);
			}

			list_push_back(lst, rec);

			logger_write(
				LOGGER_INFO,
				"try_update_a_cache(): Updated cache:\n  Domain: %s\n  ip_addr: %u.%u.%u.%u",
				rec->domain, ((uint8_t *)(&rec->ip_addr))[0],
				((uint8_t *)(&rec->ip_addr))[1],
				((uint8_t *)(&rec->ip_addr))[2],
				((uint8_t *)(&rec->ip_addr))[3]);
		}
	}

	pthread_rwlock_unlock(&a_rec_pool_lock);
}

static void try_update_aaaa_cache(answer_t *answers, size_t num_answer)
{
#ifdef __DEBUG__
	assert(answers != NULL);
#endif
	if (num_answer == 0)
		return;

	pthread_rwlock_wrlock(&aaaa_rec_pool_lock);

	for (size_t i = 0; i < num_answer; i++) {
		if (answers[i].type == TYPE_AAAA) {
			aaaa_answer_t *rec = (aaaa_answer_t *)answers[i].answer;
			list *lst = __query_AAAA_record_no_lock(rec->domain);
			list_clear(lst);
		}
	}

	for (size_t i = 0; i < num_answer; i++) {
		if (answers[i].type == TYPE_AAAA) {
			aaaa_answer_t *rec = (aaaa_answer_t *)answers[i].answer;
			rec->last_update = time(NULL);
			list *lst = __query_AAAA_record_no_lock(rec->domain);

			if (lst == NULL) {
				lst = create_list();
				update_AAAA_record(rec->domain, lst);
			} else {
				list_clear(lst);
			}

			list_push_back(lst, rec);

			logger_write(
				LOGGER_INFO,
				"try_update_aaaa_cache(): Update cache:\n  Domain: %s\n  ip_addr: %x:%x:%x:%x:%x:%x:%x:%x",
				rec->domain,
				ntohs(((uint16_t *)(&rec->ip_addr))[0]),
				ntohs(((uint16_t *)(&rec->ip_addr))[1]),
				ntohs(((uint16_t *)(&rec->ip_addr))[2]),
				ntohs(((uint16_t *)(&rec->ip_addr))[3]),
				ntohs(((uint16_t *)(&rec->ip_addr))[4]),
				ntohs(((uint16_t *)(&rec->ip_addr))[5]),
				ntohs(((uint16_t *)(&rec->ip_addr))[6]),
				ntohs(((uint16_t *)(&rec->ip_addr))[7]));
		}
	}

	pthread_rwlock_unlock(&aaaa_rec_pool_lock);
}

void update_cache(raw_data *remote_data)
{
#ifdef __DEBUG__
	assert(remote_data != NULL);
#endif
	answer_t *answers = NULL;

	size_t num_ans =
		get_answers(remote_data->data, remote_data->size, &answers);
	if (answers == NULL)
		return;

	logger_write(LOGGER_INFO, "update_cache(): Trying to update a cache.");
	try_update_a_cache(answers, num_ans);

	logger_write(LOGGER_INFO, "update_cache(): Trying to update cname cache.");
	try_update_cname_cache(answers, num_ans);

	logger_write(LOGGER_INFO, "update_cache(): Trying to update aaaa cache.");
	try_update_aaaa_cache(answers, num_ans);
	logger_write(LOGGER_INFO, "update_cache(): Update Cache Finished.");

	free(answers);
}
