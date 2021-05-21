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

#include "trie.h"

#include <assert.h>
#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

const static int16_t __char_ref_table[] = {
	0,  0,	0,  0,	0,  0,	0,  0,	0,  0,	0,  0,	0,  0,	0,  0,
	0,  0,	0,  0,	0,  0,	0,  0,	0,  0,	0,  0,	0,  0,	0,  0,
	0,  0,	0,  0,	0,  0,	0,  0,	0,  0,	0,  0,	0,  0,	0,  0,
	1,  2,	3,  4,	5,  6,	7,  8,	9,  10, 0,  0,	0,  0,	0,  0,
	0,  11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
	26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 0,	0,  0,	0,  37,
	0,  11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
	26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 0,	0,  0,	0,  0
};

static int16_t __get_char_ref(char ch)
{
	if (ch < 0)
		return -1;

	return __char_ref_table[(int)ch];
}

trie *create_trie(void)
{
	trie *result = (trie *)malloc(sizeof(trie));
	memset(result, 0, sizeof(trie));
	return result;
}

static struct __trie_node *__create_trie_node(void)
{
	trie *result = (trie *)malloc(sizeof(trie));
	memset(result, 0, sizeof(trie));
	return result;
}

void destroy_trie(trie *t)
{
	if (t == NULL)
		return;

	for (int i = 0; i < NUM_DOMAIN_CHARACTER; i++)
		if (t->next[i] != NULL) {
			destroy_trie(t->next[i]);
			t->next[i] = NULL;
		}
	free(t);
}

void trie_insert(trie *t, const char *name, size_t n_siz, void *val)
{
	if (t == NULL || name == NULL || val == NULL) {
		assert("trie_insert: Trying to insert NULL to NULL.");
	}
	if (n_siz == 0) {
		return;
	}

	/**
     * 逆序插入是因为域名的后半部分经常重复，尽可能复用trie节点以减少内存占用
     **/
	for (int i = (int)n_siz - 1; i >= 0; i--) {
		t->size += 1;
		uint16_t ch = __get_char_ref(name[i]);
		if (t->next[ch] == NULL)
			t->next[ch] = __create_trie_node();
		t = t->next[ch];
	}

	t->size += 1;
	t->value = val;
}

static void *__trie_remove(trie *t, const char *name, int cur)
{
	if (t == NULL || name == NULL) {
		assert("trie_remove: Trying to remove from NULL.");
	}

	void *res = NULL;

	if (cur == -1) {
		res = t->value;
		t->size -= 1;
		t->value = NULL;
		return res;
	}

	uint16_t ch = __get_char_ref(name[cur]);
	if (t->next[ch] == NULL)
		return NULL;

	res = __trie_remove(t->next[ch], name, cur - 1);
	if (res != NULL)
		t->size -= 1;

	if (t->next[ch]->size == 0) {
		free(t->next[ch]);
		t->next[ch] = NULL;
	}

	return res;
}

void *trie_remove(trie *t, const char *name, size_t n_siz)
{
	if (t == NULL || name == NULL) {
		assert("trie_remove: Trying to remove from NULL.");
	}
	if (n_siz == 0) {
		return NULL;
	}

	return __trie_remove(t, name, (int)n_siz - 1);
}

void *trie_find(trie *t, const char *name, size_t n_siz)
{
	if (t == NULL || name == NULL) {
		assert("trie_find: Trying to find something in NULL.");
	}
	if (n_siz == 0) {
		return NULL;
	}

	for (int i = (int)n_siz - 1; i >= 0; i--) {
		uint16_t ch = __get_char_ref(name[i]);
		if (t->next[ch] == NULL)
			return NULL;
		t = t->next[ch];
	}

	return t->value;
}
