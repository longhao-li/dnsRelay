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

#ifndef MODEL_LIST_H_
#define MODEL_LIST_H_

#include <stddef.h>

struct __list;

struct __list_node {
	struct __list_node *pre, *next;
	void *value;
	struct __list *belong_to; /* A List. */
};

typedef struct __list_node *list_iterator;

typedef struct __list {
	struct __list_node *begin, *end;
	size_t size;
} list;

#define foreach_list(key, lst)                                                 \
	for (list_iterator key = (lst)->begin; (key) != (lst)->end;            \
	     (key) = (key)->next)

extern list *create_list(void);

extern list_iterator list_push_back(list *lst, void *val);
extern list_iterator list_push_front(list *lst, void *val);
extern void *list_pop_back(list *lst);
extern void *list_pop_front(list *lst);

extern void *list_first(list *lst);
extern void *list_last(list *lst);

extern list_iterator list_find_by_val(list *lst, void *val);
extern list_iterator list_find_kth(list *lst, size_t k);
extern void *list_remove(list_iterator node);
extern void list_move_to_end(list_iterator node);

extern int list_empty(list *lst);
extern size_t list_size(list *lst);
extern void list_clear(list *lst);

#endif /* MODEL_LIST_H_ */
