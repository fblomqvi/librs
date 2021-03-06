/*
 * list.h
 * Copyright (C) 2019 Ferdinand Blomqvist
 *
 * This file is part of librs.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 2.1 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef FB_CONTAINERS_LIST_H
#define FB_CONTAINERS_LIST_H

#include <stdlib.h>

typedef struct fb_containers_list_node_VOID_PTR LIST_NODE;

struct fb_containers_list_node_VOID_PTR
{
    void* data;
    LIST_NODE* next;
    LIST_NODE* prev;
};

typedef struct fb_containers_list_VOID_PTR
{
    LIST_NODE* first;
    LIST_NODE* last;
} LIST;

/*! \brief Allocates a new LIST.
 *
 *  \returns A pointer to the newly allocated LIST or a null
 * pointer on failure.
 */
LIST* LIST_alloc(void);

/*! \brief Frees the given LIST. */
void LIST_free(LIST* list);

LIST_NODE* LIST_push_front(LIST* list, void* data);
LIST_NODE* LIST_push_back(LIST* list, void* data);
void* LIST_pop_front(LIST* list);
void* LIST_pop_back(LIST* list);

void LIST_remove(LIST* list, LIST_NODE* node, int free_node);

static inline int LIST_empty(LIST* list)
{ return !list->first; }

static inline void* LIST_front(LIST* list)
{ return list->first->data; }

static inline void* LIST_back(LIST* list)
{ return list->last->data; }

static inline LIST_NODE* LIST_first(LIST* list)
{ return list->first; }

static inline LIST_NODE* LIST_last(LIST* list)
{ return list->last; }

static inline LIST_NODE* LIST_next(LIST_NODE* node)
{ return node->next; }

static inline LIST_NODE* LIST_prev(LIST_NODE* node)
{ return node->prev; }

#endif
