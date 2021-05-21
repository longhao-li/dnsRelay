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

#include "answer.h"

#include "core/dns.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>

a_answer_t *create_a_answer(const char *domain, uint32_t TTL, const char *ip_addr)
{
    a_answer_t *ans = (a_answer_t *)malloc(sizeof(a_answer_t));
    memset(ans, 0, sizeof(a_answer_t));

    strcpy(ans->domain, domain);
    ans->ttl = TTL;
    ans->atype = TYPE_A;
    ans->aclass = CLASS_IN;
    ans->data_length = 4;
    ans->ip_addr = inet_addr(ip_addr);
    ans->last_update = time(NULL);

    return ans;
}