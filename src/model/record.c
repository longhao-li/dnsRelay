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

#include "record.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

A_RECORD *create_A_RECORD(const char *ip, uint32_t TTL)
{
	A_RECORD *result = (A_RECORD *)malloc(sizeof(A_RECORD));
	result->ip_addr = inet_addr(ip);
	if (result->ip_addr == INADDR_NONE) {
		free(result);
		return NULL;
	}

	result->TTL = TTL;
	result->last_update = time(NULL);
	return result;
}

CNAME_RECORD *create_CNAME_RECORD(char *domain, size_t len, uint32_t TTL)
{
	CNAME_RECORD *result = (CNAME_RECORD *)malloc(sizeof(CNAME_RECORD));
	memcpy(result->domain, domain, len);
	result->domain_len = (uint16_t)len;
	result->domain[len + 1] = '\0';
	result->TTL = TTL;
	return result;
}

AAAA_RECORD *create_AAAA_RECORD(const char *ip, uint32_t TTL)
{
	AAAA_RECORD *result = (AAAA_RECORD *)malloc(sizeof(AAAA_RECORD));
	if (!inet_pton(AF_INET6, ip, &(result->ip_addr))) {
		free(result);
		return NULL;
	}

	result->TTL = TTL;
	result->last_update = time(NULL);
	return result;
}

BOOL A_record_timeout(const A_RECORD *record)
{
	if (record == NULL)
		return FALSE;

	time_t time_pass = time(NULL) - record->last_update;
	if (time_pass > record->TTL)
		return TRUE;
	return FALSE;
}

BOOL CNAME_record_timeout(const CNAME_RECORD *record)
{
	if (record == NULL)
		return FALSE;

	time_t time_pass = time(NULL) - record->last_update;
	if (time_pass > record->TTL)
		return TRUE;
	return FALSE;
}

BOOL AAAA_record_timeout(const AAAA_RECORD *record)
{
	if (record == NULL)
		return FALSE;

	time_t time_pass = time(NULL) - record->last_update;
	if (time_pass > record->TTL)
		return TRUE;
	return FALSE;
}

uint32_t get_A_current_TTL(const A_RECORD *record)
{
	if (record == NULL)
		return 0;
	time_t time_pass = time(NULL) - record->last_update;
	return record->TTL > time_pass ? record->TTL - time_pass : 0;
}

uint32_t get_CNAME_current_TTL(const CNAME_RECORD *record)
{
	if (record == NULL)
		return 0;
	time_t time_pass = time(NULL) - record->last_update;
	return record->TTL > time_pass ? record->TTL - time_pass : 0;
}

uint32_t get_AAAA_current_TTL(const AAAA_RECORD *record)
{
	if (record == NULL)
		return 0;
	time_t time_pass = time(NULL) - record->last_update;
	return record->TTL > time_pass ? record->TTL - time_pass : 0;
}
