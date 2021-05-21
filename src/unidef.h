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

#ifndef UNIDEF_H_
#define UNIDEF_H_

#define __DEBUG__

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#if (defined(_WIN32) || defined(WIN32))
#include <windows.h>
#else
#include <unistd.h>
#include <arpa/inet.h>
#endif

#ifndef BOOL
#define BOOL int
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/* Mark out params */
#define out

#define DNS_SERVER_MAX(x, y) ((x) > (y) ? (x) : (y))
#define DNS_SERVER_MIN(x, y) ((x) < (y) ? (x) : (y))

#if (defined(_WIN32) || defined(WIN32))
#define sleepms(msec) Sleep(misc)
#else
#define sleepms(msec) usleep((msec)*1000)
#endif

#define RAW_DATA_MAX_SIZE 1024
#define DOMAIN_NAME_MAX_LENGTH 128

#define GET_TYPE_PTR_TYPE(ptr) (ntohs(*(uint16_t *)(ptr)))
#define GET_CLASS_PTR_CLASS(ptr) (ntohs(*(uint16_t *)(ptr)))
#define GET_TTL_PTR_TTL(ptr) (ntohl(*(uint32_t *)(ptr)))
#define GET_DATA_LEN_PTR_DATA_LEN(ptr) (htons(*(uint16_t *)(ptr)))

typedef struct raw_data {
	size_t size;
	uint8_t data[RAW_DATA_MAX_SIZE];
} raw_data;

#ifndef _NETINET6_IN6_H_
typedef struct in6_addr in6_addr_t;
#endif

#endif /* UNIDEF_H_ */