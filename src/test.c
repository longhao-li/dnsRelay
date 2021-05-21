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

#include "test.h"

#include "core/dns.h"
#include "unidef.h"

#include <stdio.h>

extern void test_header_parse(void *data, size_t size)
{
	fprintf(stderr,
		"TEST HEADER:\n  ID: %04x\n  Flags: %04x\n  Question: %u\n  Answer: %u\n  Authority: %u\n  Additional: %u\n",
		get_header_info(data, HEADER_ID),
		get_header_info(data, HEADER_FLAGS),
		get_header_info(data, HEADER_QUESTION),
		get_header_info(data, HEADER_ANSWER),
		get_header_info(data, HEADER_AUTHORITY),
		get_header_info(data, HEADER_ADDITIONAL));
}

extern void test_query_parse(void *data, size_t size)
{
	query_meta qmeta = parse_query(data, size);
	char name[512];

	get_query_url(data, size, name, 512);

	fprintf(stderr, "QUERY:\n  Domain Name: %s\n  Type: %u\n  Class: %u\n",
		name, ntohs(*qmeta.type_ptr), ntohs(*qmeta.class_ptr));
}

extern void test_response_parse(void *data, size_t size)
{
	response_meta *metas = NULL;
	// print_raw_data(data, size);
	size_t num = parse_responses(data, size, &metas);

	for (size_t i = 0; i < num; i++) {
		if (metas->response_end == NULL) {
			fprintf(stderr,
				"**** test_response_parse(): i = %zu, This may a fake response. Raw data:\n",
				i);
			// print_raw_data(metas->response_begin, (char *)metas->response_end - (char *)metas->response_begin);
			continue;
		}

		fprintf(stderr,
			"Response Meta:\n  Begin: %zu\n  Name: %zu\n  Type_ptr: %zu\n  Class_ptr: %zu\n  TTL_ptr: %zu\n  Data_len_ptr: %zu\n  Data_ptr: %zu\n  End: %zu\n",
			(uint8_t *)metas[i].response_begin - (uint8_t *)data,
			(uint8_t *)metas[i].name - (uint8_t *)data,
			(uint8_t *)metas[i].type_ptr - (uint8_t *)data,
			(uint8_t *)metas[i].class_ptr - (uint8_t *)data,
			(uint8_t *)metas[i].ttl_ptr - (uint8_t *)data,
			(uint8_t *)metas[i].data_len_ptr - (uint8_t *)data,
			(uint8_t *)metas[i].data_ptr - (uint8_t *)data,
			(uint8_t *)metas[i].response_end - (uint8_t *)data);

		fprintf(stderr, "Response:\n  Name: ");
		/* Begin with 0xc0 ? */
		if ((uint8_t)(metas[i].name[0]) == 0xc0) {
			fprintf(stderr, "%02x %02x\n",
				(uint8_t)metas[i].name[0],
				(uint8_t)metas[i].name[1]);
		} else {
			fprintf(stderr, "%s\n", metas[i].name);
		}

		fprintf(stderr,
			"  Type: %u\n  Class: %u\n  TTL: %u\n  Data Length: %u\n",
			ntohs(*(metas[i].type_ptr)),
			ntohs(*(metas[i].class_ptr)),
			ntohl(*(metas[i].ttl_ptr)),
			ntohs(*(metas[i].data_len_ptr)));

		if (ntohs(*(metas[i].type_ptr)) == TYPE_A) {
			fprintf(stderr, "  IPv4 Address: %u.%u.%u.%u\n",
				((uint8_t *)metas[i].data_ptr)[0],
				((uint8_t *)metas[i].data_ptr)[1],
				((uint8_t *)metas[i].data_ptr)[2],
				((uint8_t *)metas[i].data_ptr)[3]);
		} else if (ntohs(*(metas[i].type_ptr)) == TYPE_CNAME) {
			char domain[512];
			// uint16_t bias = get_url(metas[i].data_ptr, ntohs(*(metas[i].data_len_ptr)), domain, 512);
			get_url(data, metas[i].data_ptr, domain, 512);
			fprintf(stderr, "  CNAME Domain: %s\n", domain);
			// if (bias) {
			//     fprintf(stderr, "%s\n", ((char *)data) + (bias & 0x3fff));
			//     fprintf(stderr, "  CNAME END Bias: %04x\n", bias);
			// } else {
			//     fprintf(stderr, "\n");
			// }
		} else if (ntohs(*(metas[i].type_ptr)) == TYPE_AAAA) {
			fprintf(stderr,
				"  IPv6 Address: %u:%u:%u:%u:%u:%u:%u:%u\n",
				ntohs(((uint16_t *)metas[i].data_ptr)[0]),
				ntohs(((uint16_t *)metas[i].data_ptr)[1]),
				ntohs(((uint16_t *)metas[i].data_ptr)[2]),
				ntohs(((uint16_t *)metas[i].data_ptr)[3]),
				ntohs(((uint16_t *)metas[i].data_ptr)[4]),
				ntohs(((uint16_t *)metas[i].data_ptr)[5]),
				ntohs(((uint16_t *)metas[i].data_ptr)[6]),
				ntohs(((uint16_t *)metas[i].data_ptr)[7]));
		}
	}

	if (num)
		free(metas);
}

void print_raw_data(const unsigned char *data, const size_t size)
{
	for (size_t i = 0; i < size; i++) {
		if (i % 16 == 0)
			fprintf(stderr, "\n");
		fprintf(stderr, "%02x ", data[i]);
	}
	fprintf(stderr, "\n");
}

void test_answer_response_parse(void *data, size_t size)
{
	answer_t *answer_list;
	size_t num_ans = get_answers(data, size, &answer_list);
	if (num_ans == 0) {
		fprintf(stderr, "TEST ANSWER: No answer.\n");
		return;
	}

	for (size_t i = 0; i < num_ans; i++) {
		if (answer_list[i].type == TYPE_A) {
			a_answer_t *ans = (a_answer_t *)answer_list[i].answer;
			humanlize_url(ans->domain);

			printf("A Answer:\n  Domain: %s\n  Type: %u\n  Class: %u\n  TTL: %u\n  Data Length: %u\n  Ip Addr: %u.%u.%u.%u\n",
			       ans->domain, ans->atype, ans->aclass, ans->ttl,
			       ans->data_length,
			       ((uint8_t *)(&(ans->ip_addr)))[0],
			       ((uint8_t *)(&(ans->ip_addr)))[1],
			       ((uint8_t *)(&(ans->ip_addr)))[2],
			       ((uint8_t *)(&(ans->ip_addr)))[3]);
		} else if (answer_list[i].type == TYPE_CNAME) {
			cname_answer_t *ans =
				(cname_answer_t *)answer_list[i].answer;
			humanlize_url(ans->domain);
			humanlize_url(ans->cname);

			printf("CNAME Answer:\n  Domain: %s\n  Type: %u\n  Class: %u\n  TTL: %u\n  Data Length: %u\n  CNAME: %s\n",
			       ans->domain, ans->atype, ans->aclass, ans->ttl,
			       ans->data_length, ans->cname);
		} else if (answer_list[i].type == TYPE_CNAME) {
			aaaa_answer_t *ans =
				(aaaa_answer_t *)answer_list[i].answer;
			humanlize_url(ans->domain);
			printf("AAAA Answer:\n  Domain: %s\n  Type: %u\n  Class: %u\n  TTL: %u\n  Data Length: %u\n  Ip Addr: %x:%x:%x:%x:%x:%x:%x:%x\n",
			       ans->domain, ans->atype, ans->aclass, ans->ttl,
			       ans->data_length,
			       ntohs(((uint16_t *)(&(ans->ip_addr)))[0]),
			       ntohs(((uint16_t *)(&(ans->ip_addr)))[1]),
			       ntohs(((uint16_t *)(&(ans->ip_addr)))[2]),
			       ntohs(((uint16_t *)(&(ans->ip_addr)))[3]),
			       ntohs(((uint16_t *)(&(ans->ip_addr)))[4]),
			       ntohs(((uint16_t *)(&(ans->ip_addr)))[5]),
			       ntohs(((uint16_t *)(&(ans->ip_addr)))[6]),
			       ntohs(((uint16_t *)(&(ans->ip_addr)))[7]));
		}
	}
}
