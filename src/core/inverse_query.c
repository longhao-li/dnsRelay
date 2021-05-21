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

#include "inverse_query.h"
#include "cache.h"
#include "dns.h"
#include "logger.h"

#include <assert.h>
#include <string.h>

static size_t __set_answer_header(const request_data *request, out void *answer)
{
	memcpy(answer, request->data, request->size);
	set_header_info(answer, HEADER_FLAGS, FLAGS_RESPONSE_NO_ERROR);
	return request->size;
}

size_t inverse_query_a(const request_data *request, out void *answer)
{
#ifdef __DEBUG__
	assert(request != NULL);
	assert(answer != NULL);
#endif

	cname_answer_t *cname_answers[100] = { 0 };
	int num_cname_answers = 0;
	char url[512] = { 0 };
	int len_url = 0;
	uint8_t buf[512] = { 0 };
	uint16_t len_buf = 0;
	uint8_t *last_cname_begin = NULL;
	size_t res = __set_answer_header(request, answer);

	get_query_url(request->data, request->size, url, 512);
	len_url = strlen(url);

	logger_write(LOGGER_INFO, "inverse_query_a(): Trying to query url: %s",
		     url);

	while (len_url && num_cname_answers < 100) {
		cname_answers[num_cname_answers] = query_CNAME_record(url);
		if (cname_answers[num_cname_answers] == NULL)
			break;

		logger_write(
			LOGGER_INFO,
			"inverse_query_a(): Get CNAME:\n  Domain: %s\n  CNAME: %s\n",
			url, cname_answers[num_cname_answers]->cname);

		strcpy(url, cname_answers[num_cname_answers]->cname);
		len_url = strlen(url);
		num_cname_answers++;
	}

	if (num_cname_answers == 0) {
		logger_write(LOGGER_INFO,
			     "inverse_query_a(): No CNAME record found.");
	}

	for (int i = 0; i < num_cname_answers; i++) {
		uint16_t bias = 0;
		if (last_cname_begin == NULL) {
			bias = (char *)get_query_info(answer, QUERY_BEGIN,
						      request->size) -
			       (char *)answer;
			last_cname_begin = (uint8_t *)answer + res;
		} else {
			bias = last_cname_begin + 12 - (uint8_t *)answer;
			last_cname_begin = (uint8_t *)answer + res;
		}

		len_buf = generate_single_cname_response(answer, bias,
							 cname_answers[i], buf);
		memcpy((uint8_t *)answer + res, buf, len_buf);
		res += len_buf;
	}

	uint16_t num_a_answers = 0;
	uint16_t bias = 0;
	if (last_cname_begin == NULL)
		bias = (char *)get_query_info(answer, QUERY_BEGIN,
					      request->size) -
		       (char *)answer;
	else
		bias = last_cname_begin + 12 - (uint8_t *)answer;

	logger_write(
		LOGGER_INFO,
		"inverse_query_a(): Query A Record\n  Bias: %u\n  Url: %s\n",
		bias, url);

	list *a_rec_list = query_A_record(url);

	if (a_rec_list == NULL) {
		logger_write(LOGGER_INFO,
			     "inverse_query_a(): Domain name not found.");
		return 0;
	}
	if (list_empty(a_rec_list)) {
		logger_write(LOGGER_INFO, "inverse_query_a(): Empty A record.");
		return 0;
	}

	foreach_list(i, a_rec_list)
	{
		a_answer_t *a_ans = (a_answer_t *)i->value;
		if (answer_timeout(a_ans)) {
			logger_write(
				LOGGER_INFO,
				"inverse_query_a(): Record %s TTL timeout.",
				a_ans->domain);
			return 0;
		}

		len_buf = generate_single_a_response(bias, a_ans,
						     (uint8_t *)answer + res);
		res += len_buf;
		num_a_answers++;
	}

	set_header_info(answer, HEADER_ANSWER,
			(uint16_t)num_cname_answers + num_a_answers);

	logger_write_raw(LOGGER_INFO,
			 "inverse_query_a(): Final Result:", answer, res);
	return res;
}

size_t inverse_query_aaaa(const request_data *request, out void *answer)
{
#ifdef __DEBUG__
	assert(request != NULL);
	assert(answer != NULL);
#endif
	cname_answer_t *cname_answers[100] = { 0 };
	int num_cname_answers = 0;
	char url[512] = { 0 };
	int len_url = 0;
	uint8_t buf[512] = { 0 };
	uint16_t len_buf = 0;
	uint8_t *last_cname_begin = NULL;
	size_t res = __set_answer_header(request, answer);

	get_query_url(request->data, request->size, url, 512);
	len_url = strlen(url);

	logger_write(LOGGER_INFO,
		     "inverse_query_aaaa(): Trying to query url: %s", url);

	while (len_url && num_cname_answers < 100) {
		cname_answers[num_cname_answers] = query_CNAME_record(url);
		if (cname_answers[num_cname_answers] == NULL)
			break;

		logger_write(
			LOGGER_INFO,
			"inverse_query_aaaa(): Get CNAME:\n  Domain: %s\n  CNAME: %s\n",
			url, cname_answers[num_cname_answers]->cname);

		strcpy(url, cname_answers[num_cname_answers]->cname);
		len_url = strlen(url);
		num_cname_answers++;
	}

	if (num_cname_answers == 0) {
		logger_write(LOGGER_INFO,
			     "inverse_query_aaaa(): No CNAME record found.");
	}

	for (int i = 0; i < num_cname_answers; i++) {
		uint16_t bias = 0;
		if (last_cname_begin == NULL) {
			bias = (char *)get_query_info(answer, QUERY_BEGIN,
						      request->size) -
			       (char *)answer;
			last_cname_begin = (uint8_t *)answer + res;
		} else {
			bias = last_cname_begin + 12 - (uint8_t *)answer;
			last_cname_begin = (uint8_t *)answer + res;
		}

		len_buf = generate_single_cname_response(answer, bias,
							 cname_answers[i], buf);
		memcpy((uint8_t *)answer + res, buf, len_buf);
		res += len_buf;
	}

	uint16_t num_aaaa_answers = 0;
	uint16_t bias = 0;
	if (last_cname_begin == NULL)
		bias = (char *)get_query_info(answer, QUERY_BEGIN,
					      request->size) -
		       (char *)answer;
	else
		bias = last_cname_begin + 12 - (uint8_t *)answer;

	logger_write(
		LOGGER_INFO,
		"inverse_query_aaaa(): Query AAAA Record\n  Bias: %u\n  Url: %s\n",
		bias, url);

	list *aaaa_rec_list = query_AAAA_record(url);

	if (aaaa_rec_list == NULL) {
		logger_write(LOGGER_INFO,
			     "inverse_query_aaaa(): Domain name not found.");
		return 0;
	}
	if (list_empty(aaaa_rec_list)) {
		logger_write(LOGGER_INFO,
			     "inverse_query_aaaa(): Empty AAAA record.");
		return 0;
	}

	foreach_list(i, aaaa_rec_list)
	{
		aaaa_answer_t *aaaa_ans = (aaaa_answer_t *)i->value;
		if (answer_timeout(aaaa_ans)) {
			logger_write(
				LOGGER_INFO,
				"inverse_query_aaaa(): Record %s TTL timeout.",
				aaaa_ans->domain);
			return 0;
		}

		len_buf = generate_single_aaaa_response(
			bias, aaaa_ans, (uint8_t *)answer + res);
		res += len_buf;
		num_aaaa_answers++;
	}

	set_header_info(answer, HEADER_ANSWER,
			(uint16_t)num_cname_answers + num_aaaa_answers);

	logger_write_raw(LOGGER_INFO,
			 "inverse_query_aaaa(): Final Result:", answer, res);
	return res;
}
