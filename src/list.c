/*
 * list.c
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

#include "list.h"

static void LIST_NODE_free(LIST_NODE* node)
{ free(node); }

LIST* LIST_alloc(void)
{
    LIST* list = malloc(sizeof(LIST));
    if(!list)
        return NULL;

    list->first = NULL;
    list->last = NULL;
    return list;
}

void LIST_free(LIST* list)
{
    if(!list)
        return;

    LIST_NODE* node = list->first;
    while(node)
    {
        LIST_NODE* temp = node;
        node = node->next;
        LIST_NODE_free(temp);
    }
    free(list);
}

LIST_NODE* LIST_push_front(LIST* list, void* data)
{
    LIST_NODE* node = malloc(sizeof(LIST_NODE));
    if(!node)
        return NULL;

    node->next = list->first;
    node->prev = NULL;
    node->data = data;

    if(!list->first)    // The list is empty
        list->last = node;
    else
        list->first->prev = node;

    list->first = node;
    return node;
}

LIST_NODE* LIST_push_back(LIST* list, void* data)
{
    LIST_NODE* node = malloc(sizeof(LIST_NODE));
    if(!node)
        return NULL;

    node->next = NULL;
    node->prev = list->last;
    node->data = data;

    if(!list->first)    // The list is empty
        list->first = node;
    else
        list->last->next = node;

    list->last = node;
    return node;
}

void* LIST_pop_front(LIST* list)
{
    void* data = list->first->data;
    LIST_NODE* node = list->first;
    list->first = list->first->next;
    if(!list->first)
        list->last = NULL;
    else
        list->first->prev = NULL;
    LIST_NODE_free(node);
    return data;
}

void* LIST_pop_back(LIST* list)
{
    void* data = list->last->data;
    LIST_NODE* node = list->last;
    list->last = list->last->prev;
    if(!list->last)
        list->first = NULL;
    else
        list->last->next = NULL;
    LIST_NODE_free(node);
    return data;
}

void LIST_remove(LIST* list, LIST_NODE* node, int free_node)
{
    if(node->prev)  // node has a predecessor
        node->prev->next = node->next;
    else    // node is the first node
        list->first = node->next;

    if(node->next)  // node has a successor
        node->next->prev = node->prev;
    else    // node is the last node
        list->last = node->prev;

    if(free_node)
        LIST_NODE_free(node);
}
