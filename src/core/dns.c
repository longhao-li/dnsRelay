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

/* 其实是随便选的一个值 */
#define DOMAIN_NAME_FILLER 6

#include "dns.h"
#include "logger.h"
#include "model/list.h"
#include "unidef.h"

#include <arpa/inet.h>
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

uint16_t get_header_info(const void *header, HEADER_ITEM qtype)
{
	if (header == NULL) {
		logger_write(LOGGER_ERROR,
			     "get_header_info(): header is NULL. Abort.");
		exit(1);
	}
	uint16_t *header_ptr = (uint16_t *)header;
	return ntohs(header_ptr[(int)qtype]);
}

void set_header_info(void *header, HEADER_ITEM qtype, uint16_t value)
{
	uint16_t *__header = (uint16_t *)header;
	__header[(int)qtype] = htons(value);
}

static struct __query_meta __parse_query_info(const void *query, size_t qsize)
{
	struct __query_meta result;
	uint8_t *__query = (uint8_t *)query;
	const uint8_t *__query_top = __query + qsize;

	/* Bias sizeof header */
	__query += sizeof(struct __dns_header);

	result.query_begin = __query;
	result.name = (char *)(__query);

	/* Beginning of query domain name */
	++__query;
	while (__query < __query_top && (char)*__query != '\0')
		++__query;

	if (__query == __query_top) {
		memset(&result, 0, sizeof(result));
		return result;
	}
	/* (char)*__query should be '\0' now. */

	++__query;
	result.type_ptr = (uint16_t *)__query;
	__query += 2;
	result.class_ptr = (uint16_t *)__query;
	__query += 2;
	result.query_end = __query;

	/* This is not a standard query. */
	if (__query > __query_top)
		memset(&result, 0, sizeof(result));
	return result;
}

query_meta parse_query(const void *query, size_t qsize)
{
	return __parse_query_info(query, qsize);
}

void *get_query_info(const void *data, QUERY_ITEM qtype, size_t data_size)
{
	query_meta meta_data = parse_query(data, data_size);

	/* Illegal query data */
	if (meta_data.query_end == NULL)
		return NULL;

	switch (qtype) {
	case QUERY_NAME:
		return meta_data.name;
	case QUERY_TYPE:
		return meta_data.type_ptr;
	case QUERY_CLASS:
		return meta_data.class_ptr;
	case QUERY_BEGIN:
		return meta_data.query_begin;
	case QUERY_END:
		return meta_data.query_end;
	default:
		return NULL;
	}
}

BOOL get_query_url(const void *data, size_t data_size, char *domain,
		   size_t domain_size)
{
	query_meta meta_data = parse_query(data, data_size);
	if (meta_data.query_end == NULL)
		return FALSE;

	size_t domain_len = strlen(meta_data.name);
	if (domain_len + 1 >= domain_size)
		return FALSE;

	memcpy(domain, meta_data.name, domain_len + 1);
	for (size_t i = 0; i < domain_len; i++)
		if (iscntrl(domain[i]))
			domain[i] = '.';

	return TRUE;
}

uint16_t get_url(const void *data_begin, void *url, char *dest,
		 size_t dest_size)
{
#ifdef __DEBUG__
	assert(data_begin != NULL);
	assert(url != NULL);
	assert(dest != NULL);
#endif

	if (dest_size == 0) {
		logger_write(
			LOGGER_WARNING,
			"get_url(): Destination has no space to store url.");
		return 0;
	}

	uint16_t bias = 0;
	uint16_t dest_len = 0;
	char *url_begin = (char *)url;

	if ((url_begin[0] & 0xc0) == 0xc0)
		bias = ntohs(*(uint16_t *)(url_begin));

	do {
		bias &= 0x3fff;
		if (bias)
			url_begin = (char *)data_begin + bias;
		bias = 0;

		for (int i = 0; url_begin[i] != '\0' && dest_len < dest_size;
		     i++) {
			if ((url_begin[i] & 0xc0) == 0xc0) {
				bias = ntohs(*(uint16_t *)(url_begin + i));
				break;
			}
			dest[dest_len++] = url_begin[i];
		}
	} while (bias);

	if (dest_len >= dest_size) {
		logger_write(
			LOGGER_WARNING,
			"get_url(): Destination space not enough. Return a broken string.");
		return 0;
	}

	dest[dest_len] = '\0';
	return dest_len;
}

void humanlize_url(char *url)
{
	while (url[0] != '\0') {
		if (iscntrl(url[0]))
			url[0] = '.';
		url++;
	}
}

static struct __response_meta __parse_single_response_info(void *data,
							   void *data_end)
{
	struct __response_meta result;

	if (data == NULL || data_end == NULL) {
		logger_write(
			LOGGER_ERROR,
			"__parse_single_response_info(): Receiving NULL as data begin/end. Abort.");
		exit(1);
	} else if (data == data_end) {
		logger_write(
			LOGGER_WARNING,
			"__parse_single_response_info(): data begin = end, no data to parse.");
		memset(&result, 0, sizeof(result));
		return result;
	}

	result.response_begin = data;
	char *name_ptr = (char *)data;
	if ((((uint8_t *)name_ptr)[0] & 0xc0) == 0xc0) {
		result.name = name_ptr;
		name_ptr += 2;
	} else {
		result.name = name_ptr + 1;
		++name_ptr;
		while (name_ptr < (char *)data_end && (*name_ptr) != '\0')
			++name_ptr;
		/* name_ptr should points to '\0' now. */
		++name_ptr;
	}

	result.type_ptr = (uint16_t *)name_ptr;
	name_ptr += 2;
	result.class_ptr = (uint16_t *)name_ptr;
	name_ptr += 2;
	result.ttl_ptr = (uint32_t *)name_ptr;
	name_ptr += 4;
	result.data_len_ptr = (uint16_t *)name_ptr;
	name_ptr += 2;
	if (name_ptr > (char *)data_end) {
		logger_write(
			LOGGER_WARNING,
			"__parse_single_response_info(): Invalid response size.");
		goto response_parse_result_invalid;
	}

	uint16_t data_len = ntohs(*result.data_len_ptr);
	if (name_ptr + data_len > (char *)data_end) {
		logger_write(
			LOGGER_WARNING,
			"__parse_single_response_info(): Invalid response size. Failed to get data size.");
		goto response_parse_result_invalid;
	}

	result.data_ptr = name_ptr;
	result.response_end = name_ptr + data_len;
	if (result.response_end > data_end) {
		logger_write(
			LOGGER_WARNING,
			"__parse_single_response_info(): Invalid response size. Data overflow.");
		goto response_parse_result_invalid;
	}

	return result;

response_parse_result_invalid:
	logger_write(
		LOGGER_WARNING,
		"__parse_single_response_info(): data length not long enough. This may be fake response.");
	memset(&result, 0, sizeof(result));
	return result;
}

size_t parse_responses(const void *data, size_t data_size,
		       response_meta **meta_list_ptr)
{
	if (meta_list_ptr == NULL) {
		logger_write(
			LOGGER_ERROR,
			"parse_response(): passing NULL as meta_list_ptr. Abort.");
		exit(1);
	} else if (data_size < sizeof(struct __dns_header) + 4) {
		logger_write(
			LOGGER_WARNING,
			"parse_response(): data_size: %zu too small. This may be a fake response. ignored.",
			data_size);
		*meta_list_ptr = NULL;
		return 0;
	}

	uint16_t num_response = get_header_info(data, HEADER_ANSWER) +
				get_header_info(data, HEADER_AUTHORITY);
	query_meta query_info = parse_query(data, data_size);

	if (query_info.query_end == NULL)
		return 0;
	if (num_response == 0) {
		return 0;
	}

	(*meta_list_ptr) =
		(response_meta *)malloc(num_response * sizeof(response_meta));

	uint8_t *response = query_info.query_end;
	uint8_t *data_end = (uint8_t *)data + data_size;

	for (uint16_t i = 0; i < num_response && response < data_end; i++) {
		(*meta_list_ptr)[i] =
			__parse_single_response_info(response, data_end);
		response = (*meta_list_ptr)[i].response_end;
	}
	return num_response;
}

static void __get_a_answer(const void *data, a_answer_t *ans,
			   const response_meta *meta)
{
#ifdef __DEBUG__
	assert(ans != NULL);
	assert(meta != NULL);
#endif
	/* Get domain */
	if (meta->name == meta->response_begin) {
		get_url(data, meta->name, ans->domain, DOMAIN_NAME_MAX_LENGTH);
	} else {
		get_url(data, meta->name, ans->domain, DOMAIN_NAME_MAX_LENGTH);
	}

	ans->atype = GET_TYPE_PTR_TYPE(meta->type_ptr);
	ans->aclass = GET_CLASS_PTR_CLASS(meta->class_ptr);
	ans->ttl = GET_TTL_PTR_TTL(meta->ttl_ptr);
	ans->data_length = GET_DATA_LEN_PTR_DATA_LEN(meta->data_len_ptr);
	ans->ip_addr = *((in_addr_t *)(meta->data_ptr));
}

static void __get_aaaa_answer(const void *data, aaaa_answer_t *ans,
			      const response_meta *meta)
{
#ifdef __DEBUG__
	assert(ans != NULL);
	assert(meta != NULL);
#endif
	/* Get domain */
	if (meta->name == meta->response_begin) {
		get_url(data, meta->name, ans->domain, DOMAIN_NAME_MAX_LENGTH);
	} else {
		get_url(data, meta->name, ans->domain, DOMAIN_NAME_MAX_LENGTH);
	}

	ans->atype = GET_TYPE_PTR_TYPE(meta->type_ptr);
	ans->aclass = GET_CLASS_PTR_CLASS(meta->class_ptr);
	ans->ttl = GET_TTL_PTR_TTL(meta->ttl_ptr);
	ans->data_length = GET_DATA_LEN_PTR_DATA_LEN(meta->data_len_ptr);
	ans->ip_addr = *(in6_addr_t *)(meta->data_ptr);
}

static void __get_cname_answer(const void *data, cname_answer_t *ans,
			       const response_meta *meta)
{
#ifdef __DEBUG__
	assert(data != NULL);
	assert(ans != NULL);
	assert(meta != NULL);
#endif

	/* Get domain */
	if (meta->name == meta->response_begin) {
		get_url(data, meta->name, ans->domain, DOMAIN_NAME_MAX_LENGTH);
	} else {
		get_url(data, meta->name, ans->domain, DOMAIN_NAME_MAX_LENGTH);
	}

	ans->atype = GET_TYPE_PTR_TYPE(meta->type_ptr);
	ans->aclass = GET_CLASS_PTR_CLASS(meta->class_ptr);
	ans->ttl = GET_TTL_PTR_TTL(meta->ttl_ptr);
	ans->data_length = GET_DATA_LEN_PTR_DATA_LEN(meta->data_len_ptr);
	get_url(data, meta->data_ptr, ans->cname, DOMAIN_NAME_MAX_LENGTH);
}

size_t get_answers(const void *data, size_t data_size, answer_t **answer_list)
{
	response_meta *metas = NULL;
	size_t num_meta = parse_responses(data, data_size, &metas);
	if (num_meta == 0) {
		*answer_list = NULL;
		return 0;
	}

	size_t num_answer = 0;
	*answer_list = (answer_t *)malloc(num_meta * sizeof(answer_t));

	for (size_t i = 0; i < num_meta; i++) {
		if (metas[i].response_end == NULL)
			continue;

		uint16_t i_type = GET_TYPE_PTR_TYPE(metas[i].type_ptr);
		if (i_type == TYPE_A) {
			(*answer_list)[num_answer].type = TYPE_A;
			a_answer_t *ans =
				(a_answer_t *)malloc(sizeof(a_answer_t));

			(*answer_list)[num_answer].answer = ans;

			__get_a_answer(data, ans, &metas[i]);
			num_answer++;
		} else if (i_type == TYPE_AAAA) {
			(*answer_list)[num_answer].type = TYPE_AAAA;
			aaaa_answer_t *ans =
				(aaaa_answer_t *)malloc(sizeof(aaaa_answer_t));

			(*answer_list)[num_answer].answer = ans;

			__get_aaaa_answer(data, ans, &metas[i]);
			num_answer++;
		} else if (i_type == TYPE_CNAME) {
			(*answer_list)[num_answer].type = TYPE_CNAME;
			cname_answer_t *ans = (cname_answer_t *)malloc(
				sizeof(cname_answer_t));

			(*answer_list)[num_answer].answer = ans;

			__get_cname_answer(data, ans, &metas[i]);
			num_answer++;
		}
	}
#ifdef __DEBUG__
	assert(metas != NULL);
#endif
	free(metas);
	return num_answer;
}

size_t generate_no_name_response(const void *query, size_t q_size,
				 void *response, size_t response_size)
{
	if (response_size < q_size) {
		logger_write(
			LOGGER_WARNING,
			"generate_no_response(): Response buffer size is not big enough for one entire response. Ignored.");
		return 0;
	}

	memcpy(response, query, q_size);
	set_header_info(response, HEADER_FLAGS, FLAGS_RESPONSE_NO_SUCH_NAME);
	return q_size;
}

static BOOL __is_equal_character(char a, char b)
{
	if (a == b)
		return TRUE;

	if (iscntrl(a) && iscntrl(b))
		return TRUE;

	if (iscntrl(a) && b == '.')
		return TRUE;

	if (a == '.' && iscntrl(b))
		return TRUE;

	return FALSE;
}

size_t generate_single_a_response(uint16_t bias, a_answer_t *answer,
				  out void *dest)
{
#ifdef __DEBUG__
	assert(answer != NULL);
	assert(dest != NULL);
#endif
	bias |= 0xc000;

	uint16_t *a_ptr = (uint16_t *)dest;
	a_ptr[0] = htons(bias);
	a_ptr[1] = htons(answer->atype);
	a_ptr[2] = htons(answer->aclass);
	((uint32_t *)(a_ptr + 3))[0] = htonl(answer_ttl(answer));
	a_ptr[5] = htons(answer->data_length);
	((in_addr_t *)(a_ptr + 6))[0] = answer->ip_addr;
	return 16;
}

size_t generate_whole_a_response(const void *query, size_t q_size,
				 a_answer_t *answer, out void *dest)
{
#ifdef __DEBUG__
	assert(query != NULL);
	assert(answer != NULL);
	assert(dest != NULL);
#endif
	char *name_begin = get_query_info(query, QUERY_NAME, q_size);

	if (name_begin == NULL) {
		logger_write(
			LOGGER_WARNING,
			"generate_whole_a_response(): Received a bad request.");
		return 0;
	}

	size_t res = q_size;
	uint16_t bias = name_begin - (char *)query;
	uint8_t *response_begin = dest;

	memcpy(dest, query, q_size);
	response_begin += q_size;

	set_header_info(dest, HEADER_FLAGS, FLAGS_RESPONSE_NO_ERROR);
	set_header_info(dest, HEADER_ANSWER, 1);

	res += generate_single_a_response(bias, answer, response_begin);
	return res;
}

size_t generate_single_aaaa_response(uint16_t bias, aaaa_answer_t *answer,
				     out void *dest)
{
#ifdef __DEBUG__
	assert(answer != NULL);
	assert(dest != NULL);
#endif
	bias |= 0xc000;

	uint16_t *a_ptr = (uint16_t *)dest;

	a_ptr[0] = htons(bias);
	a_ptr[1] = htons(answer->atype);
	a_ptr[2] = htons(answer->aclass);

	if (answer->ttl == 0xffffffff)
		((uint32_t *)(a_ptr + 3))[0] = htonl(86400);
	else
		((uint32_t *)(a_ptr + 3))[0] = htonl(answer_ttl(answer));
	a_ptr[5] = htons(answer->data_length);
	((in6_addr_t *)(a_ptr + 6))[0] = answer->ip_addr;

	return 28;
}

size_t generate_single_cname_response(const void *data, uint16_t bias,
				      cname_answer_t *answer, out void *dest)
{
#ifdef __DEBUG__
	assert(data != NULL);
	assert(answer != NULL);
	assert(dest != NULL);
#endif
	bias |= 0xc0;
	uint16_t *a_ptr = (uint16_t *)dest;
	a_ptr[0] = htons(bias);
	bias &= 0x3fff;
	a_ptr[1] = htons(answer->atype);
	a_ptr[2] = htons(answer->aclass);

	((uint32_t *)(a_ptr + 3))[0] = htonl(answer_ttl(answer));

	a_ptr[5] = htons(0);

	char *cname = (char *)(a_ptr + 6);
	size_t ans_cname_len = strlen(answer->cname);

	char ori_url[512] = { 0 };
	size_t ori_url_len = get_url(data, (char *)data + bias, ori_url, 512);

	int i = 0,
	    top = ans_cname_len < ori_url_len ? ans_cname_len : ori_url_len;
	for (i = 0; i < top; i++) {
		if (!__is_equal_character(answer->cname[ans_cname_len - i - 1],
					  ori_url[ori_url_len - i - 1]))
			break;
	}

	int ori_cur = ori_url_len - i - 1, cname_cur = ans_cname_len - i - 1;

	while (ori_cur < ori_url_len && !iscntrl(ori_url[ori_cur]) &&
	       ori_url[ori_cur] != '.') {
		ori_cur++;
		cname_cur++;
	}

	if (ori_cur >= ori_url_len) {
		memcpy(cname, answer->cname, ans_cname_len + 1);
		a_ptr[5] = htons((uint16_t)(ans_cname_len + 1));
	} else {
		memcpy(cname, answer->cname, cname_cur);
		cname += cname_cur;
		a_ptr[5] = htons((uint16_t)cname_cur + 2);

		char *ori_ch = (char *)data + bias;

		for (int j = 0; j < ori_cur; j++) {
			if ((ori_ch[0] & 0xc0) == 0xc0) {
				uint16_t ori_bias = ntohs(*(uint16_t *)ori_ch);
				ori_bias &= 0x3fff;
				ori_ch = (char *)data + ori_bias;
			}
			ori_ch++;
		}

		uint16_t ori_bias = htons((ori_ch - (char *)data) | 0xc000);
		*(uint16_t *)cname = ori_bias;
	}
	return 12 + ntohs(a_ptr[5]);
}
