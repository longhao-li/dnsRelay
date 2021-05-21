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

#ifndef MODEL_RECORD_H_
#define MODEL_RECORD_H_

#include "unidef.h"

#include <arpa/inet.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

typedef struct A_RECORD {
	in_addr_t ip_addr;
	time_t last_update;
	uint32_t TTL;
} A_RECORD;

typedef struct CNAME_RECORD {
	char domain[128];
	uint16_t domain_len;
	time_t last_update;
	uint32_t TTL;
} CNAME_RECORD;

typedef struct AAAA_RECORD {
	in6_addr_t ip_addr;
	time_t last_update;
	uint32_t TTL;
} AAAA_RECORD;

extern A_RECORD *create_A_RECORD(const char *ip, uint32_t TTL);
extern CNAME_RECORD *create_CNAME_RECORD(char *domain, size_t len,
					 uint32_t TTL);
extern AAAA_RECORD *create_AAAA_RECORD(const char *ip, uint32_t TTL);

extern BOOL A_record_timeout(const A_RECORD *record);
extern BOOL CNAME_record_timeout(const CNAME_RECORD *record);
extern BOOL AAAA_record_timeout(const AAAA_RECORD *record);

extern uint32_t get_A_current_TTL(const A_RECORD *record);
extern uint32_t get_CNAME_current_TTL(const CNAME_RECORD *record);
extern uint32_t get_AAAA_current_TTL(const AAAA_RECORD *record);

#endif /* MODEL_RECORD_H_ */
