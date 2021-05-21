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

#include "host.h"

#include "logger.h"
#include "model/answer.h"
#include "model/trie.h"
#include "unidef.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static trie __host_tree;

static void host_add_record(const char *domain, const char *ipaddr)
{
	a_answer_t *record = create_a_answer(domain, 0xffffffff, ipaddr);

	size_t domain_len = strlen(domain);
	if (trie_find(&__host_tree, domain, domain_len) == NULL) {
		trie_insert(&__host_tree, domain, domain_len, record);
	} else {
		logger_write(
			LOGGER_WARNING,
			"host_add_record(): Record %s already exists. The older one will be covered.",
			domain);
		a_answer_t *temp = trie_remove(&__host_tree, domain, domain_len);
		free(temp);
		trie_insert(&__host_tree, domain, domain_len, record);
	}
}

void read_host(const char *path)
{
	FILE *host = fopen(path, "r");
	if (host == NULL) {
		logger_write(LOGGER_ERROR,
			     "read_host(): Failed to open host: %s.", path);
		return;
	}
	char buf[1024], ip_buf[1024];

	while (!feof(host)) {
		fscanf(host, "%s%s", ip_buf, buf);
		host_add_record(buf, ip_buf);
		logger_write(LOGGER_DEBUG, "read_host(): adding host %s %s.",
			     buf, ip_buf);
	}
}

a_answer_t *host_query(const char *domain)
{
	a_answer_t *result = trie_find(&__host_tree, domain, strlen(domain));
	return result;
}
