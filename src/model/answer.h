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

#ifndef MODEL_ANSWER_H_
#define MODEL_ANSWER_H_

#include "unidef.h"

#include <arpa/inet.h>
#include <stdint.h>
#include <time.h>

typedef struct answer {
	uint16_t type;
	void *answer;
} answer_t;

typedef struct a_answer {
	char domain[DOMAIN_NAME_MAX_LENGTH];
	time_t last_update;
	uint16_t atype;
	uint16_t aclass;
	uint32_t ttl;
	uint16_t data_length;
	in_addr_t ip_addr;
} a_answer_t;

typedef struct aaaa_answer {
	char domain[DOMAIN_NAME_MAX_LENGTH];
	time_t last_update;
	uint16_t atype;
	uint16_t aclass;
	uint32_t ttl;
	uint16_t data_length;
	in6_addr_t ip_addr;
} aaaa_answer_t;

typedef struct cname_answer {
	char domain[DOMAIN_NAME_MAX_LENGTH];
	time_t last_update;
	uint16_t atype;
	uint16_t aclass;
	uint32_t ttl;
	uint16_t data_length;
	char cname[DOMAIN_NAME_MAX_LENGTH];
} cname_answer_t;

#define answer_timeout(ans) (time(NULL) - ((ans)->last_update) >= ((ans)->ttl))
#define answer_ttl(ans) (((ans)->ttl) - (time(NULL) - ((ans)->last_update)))

extern a_answer_t *create_a_answer(const char *domain, uint32_t TTL, const char *ip_addr);

#endif /* MODEL_ANSWER_H_ */