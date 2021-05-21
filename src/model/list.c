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

#include "list.h"

#include <stdlib.h>

list_iterator __new_list_node(list *lst)
{
	list_iterator result =
		(list_iterator)malloc(sizeof(struct __list_node));
	result->pre = result->next = result->value = NULL;
	result->belong_to = lst;
	return result;
}

list *create_list(void)
{
	list *result = (list *)malloc(sizeof(list));
	result->begin = __new_list_node(result);
	result->end = result->begin;
	result->size = 0;
	return result;
}

list_iterator list_push_back(list *lst, void *val)
{
	if (lst == NULL)
		return NULL;

	list_iterator result = lst->end;

	lst->end->value = val;

	lst->end->next = __new_list_node(lst);
	lst->end->next->pre = lst->end;

	lst->end = lst->end->next;
	lst->size += 1;
	return result;
}

list_iterator list_push_front(list *lst, void *val)
{
	if (lst == NULL)
		return NULL;

	list_iterator result = __new_list_node(lst);
	result->value = val;
	result->next = lst->begin;

	lst->begin->pre = result;

	lst->begin = result;
	lst->size += 1;
	return result;
}

void *list_pop_back(list *lst)
{
	if (lst == NULL || list_empty(lst))
		return NULL;

	return list_remove(lst->end->pre);
}

void *list_pop_front(list *lst)
{
	if (lst == NULL || list_empty(lst))
		return NULL;

	return list_remove(lst->begin);
}

void *list_first(list *lst)
{
	if (lst == NULL || list_empty(lst))
		return NULL;
	return lst->begin->value;
}

void *list_last(list *lst)
{
	if (lst == NULL || list_empty(lst))
		return NULL;

	return lst->end->pre->value;
}

list_iterator list_find_by_val(list *lst, void *val)
{
	if (lst == NULL)
		return NULL;

	foreach_list(i, lst)
	{
		if (i->value == val)
			return i;
	}
	return NULL;
}

list_iterator list_find_kth(list *lst, size_t k)
{
	if (lst == NULL)
		return NULL;

	if (k >= lst->size)
		return lst->end;

	size_t counter = 0;
	foreach_list(i, lst)
	{
		if (counter >= k)
			return i;
		++counter;
	}
	return lst->end;
}

void *list_remove(list_iterator node)
{
	if (node == NULL || node == node->belong_to->end)
		return NULL;

	list *lst = node->belong_to;
	void *res = NULL;

	if (node->pre != NULL)
		node->pre->next = node->next;
	if (node->next != NULL)
		node->next->pre = node->pre;

	if (node == lst->begin) {
		lst->begin = lst->begin->next;
	}

	res = node->value;
	lst->size -= 1;
	free(node);

	return res;
}

void list_move_to_end(list_iterator node)
{
	if (node == NULL || node == node->belong_to->end ||
	    node == node->belong_to->end->pre)
		return;

	list *lst = node->belong_to;
	void *val = list_remove(node);
	list_push_back(lst, val);
}

int list_empty(list *lst)
{
	return lst->begin == lst->end;
}

size_t list_size(list *lst)
{
	return lst->size;
}

void list_clear(list *lst)
{
	if (lst == NULL)
		return;
	while (!list_empty(lst)) {
		void *ptr = list_pop_back(lst);
		free(ptr);
	}
}
