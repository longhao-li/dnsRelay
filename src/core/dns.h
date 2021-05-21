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

#ifndef CORE_DNS_H_
#define CORE_DNS_H_

#include "model/answer.h"
#include "model/list.h"
#include "model/meta.h"
#include "model/record.h"
#include "unidef.h"

#include <stdlib.h>

#define TYPE_A 1
#define TYPE_NS 2
#define TYPE_MD 3
#define TYPE_MF 4
#define TYPE_CNAME 5
#define TYPE_SOA 6
#define TYPE_MB 7
#define TYPE_MG 8
#define TYPE_MR 9
#define TYPE_NULL 10
#define TYPE_WKS 11
#define TYPE_PTR 12
#define TYPE_HINFO 13
#define TYPE_MINFO 14
#define TYPE_MX 15
#define TYPE_TXT 16
#define TYPE_AAAA 28
#define TYPE_HTTPS 65

#define CLASS_IN 1

enum HEADER_FLAGS {
	FLAGS_QUERY_STANDARD_QUERY = 0x0100,
	FLAGS_RESPONSE_NO_ERROR = 0x8180,
	FLAGS_RESPONSE_NO_SUCH_NAME = 0x8183,
	FLAGS_RESPONSE_NOT_IMPLEMENTED = 0x8184
};

typedef enum HEADER_ITEM {
	HEADER_ID = 0,
	HEADER_FLAGS = 1,
	HEADER_QUESTION = 2,
	HEADER_ANSWER = 3,
	HEADER_AUTHORITY = 4,
	HEADER_ADDITIONAL = 5
} HEADER_ITEM;

typedef enum QUERY_ITEM {
	QUERY_NAME = 0,
	QUERY_TYPE = 1,
	QUERY_CLASS = 2,
	QUERY_BEGIN = 3, /* The beginning of query. */
	QUERY_END = 4, /* The end of query */
} QUERY_ITEM;

/**
 * Value returned will perform ntohs automatically.
 */
extern uint16_t get_header_info(const void *header, HEADER_ITEM qtype);

/**
 * @param value New value. htons will be performed automatically.
 */
extern void set_header_info(void *header, HEADER_ITEM qtype, uint16_t value);

/**
 * Parse query.
 * @return query_meta::response_begin is NULL if data is illegal query.
 */
extern query_meta parse_query(const void *query, size_t qsize);

/**
 * @param data_size Used to ensure safety. Max size of data.
 * @return Return the pointer to query data. return NULL if data is not valid.
 */
extern void *get_query_info(const void *data, QUERY_ITEM qtype,
			    size_t data_size);

/**
 * Get url from query, and store in domain.
 * Unknown characters in domain will be replaced to '.'.
 */
extern BOOL get_query_url(const void *data, size_t data_size, out char *domain,
			  size_t domain_size);

/**
 * @param data_begin The beginning location of the whole data. Used for bias format(11xx xxxx).
 */
extern uint16_t get_url(const void *data_begin, void *url, out char *dest,
			size_t dest_size);

/**
 * Change cntrl character to '.'.
 * @param url Must ends with '\0'.
 */
extern void humanlize_url(char *url);

extern size_t parse_responses(const void *data, size_t data_size,
			      out response_meta **meta_list_ptr);

extern size_t get_answers(const void *data, size_t data_size,
			  out answer_t **answer_list);

extern size_t generate_no_name_response(const void *query, size_t q_size,
					out void *response,
					size_t response_size);

extern size_t generate_whole_a_response(const void *query, size_t q_size,
					a_answer_t *answer, out void *dest);

extern size_t generate_single_a_response(uint16_t bias, a_answer_t *answer,
					 out void *dest);

extern size_t generate_single_aaaa_response(uint16_t bias,
					    aaaa_answer_t *answer,
					    out void *dest);

extern size_t generate_single_cname_response(const void *data, uint16_t bias,
					     cname_answer_t *answer,
					     out void *dest);

#endif /* CORE_DNS_H_ */