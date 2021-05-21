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

#ifndef MODEL_TRIE_H_
#define MODEL_TRIE_H_

#include <stddef.h>

#define NUM_DOMAIN_CHARACTER 38

typedef struct __trie_node {
    struct __trie_node *next[NUM_DOMAIN_CHARACTER];
    void *value;
    size_t size;
} trie;

extern trie *create_trie(void);
extern void destroy_trie(trie *t);

extern void trie_insert(trie *t, const char *name, size_t n_siz, void *val);
extern void *trie_remove(trie *t, const char *name, size_t n_siz);
extern void *trie_find(trie *t, const char *name, size_t n_siz);

#endif /* MODEL_TRIE_H_ */
