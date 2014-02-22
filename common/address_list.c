/**
 *  naken_asm assembler.
 *  Author: Michael Kohn
 *   Email: mike@mikekohn.net
 *     Web: http://www.mikekohn.net/
 * License: GPL
 *
 * Copyright 2010-2014 by Michael Kohn
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assembler.h"
#include "address_list.h"
#include "macros.h"

int address_list_init(struct _address_list *address_list)
{
  address_list->memory_pool = NULL;
  address_list->locked = 0;

  return 0;
}

void address_list_free(struct _address_list *address_list)
{
  memory_pool_free(address_list->memory_pool);
  address_list->memory_pool = NULL;
}

int address_list_append(struct _asm_context *asm_context, char *name, int address)
{
int token_len;
struct _address_list *address_list = &asm_context->address_list;
struct _memory_pool *memory_pool = address_list->memory_pool;

  if (address_list->locked ==1) return 0;

  int param_count_temp;
  if (address_list_lookup(address_list, name) != -1 ||
      defines_heap_lookup(&asm_context->defines_heap, name, &param_count_temp) != NULL)
  {
    printf("Error: Label '%s' already defined.\n", name);
    return -1;
  }

  token_len = strlen(name) + 1;

  if (token_len + 4 > ADDRESS_HEAP_SIZE)
  {
    printf("Error: Label '%s' is too big.\n", name);
    return -1;
  }

  if (memory_pool == NULL)
  {
    memory_pool = memory_pool_add((struct _naken_heap *)address_list, ADDRESS_HEAP_SIZE);
  }

  while(1)
  {
     if (memory_pool->ptr + token_len + sizeof(int) < memory_pool->len)
     {
       break;
     }

     if (memory_pool->next == NULL)
     {
       memory_pool->next = memory_pool_add((struct _naken_heap *)address_list, ADDRESS_HEAP_SIZE);
     }

     memory_pool=memory_pool->next;
  }

  address = address / asm_context->bytes_per_address;

  memcpy(memory_pool->buffer + memory_pool->ptr, name, token_len);
  memcpy(memory_pool->buffer + memory_pool->ptr+token_len, &address, sizeof(int));
  memory_pool->ptr += token_len + sizeof(int);

  return 0;
} 

void address_list_lock(struct _address_list *address_list)
{
  address_list->locked = 1;
}

int address_list_lookup(struct _address_list *address_list, char *name)
{
struct _memory_pool *memory_pool = address_list->memory_pool;
int token_len;
int address;
int ptr;

  while(memory_pool != NULL)
  {
    ptr = 0;
    while(ptr < memory_pool->ptr)
    {
      token_len = strlen((char *)memory_pool->buffer + ptr) + 1;
      if (strcmp((char *)memory_pool->buffer+ptr, name) == 0)
      {
        memcpy(&address, memory_pool->buffer + ptr + token_len, sizeof(int));
        return address;
      }
      ptr += token_len + sizeof(int);
    }

    memory_pool = memory_pool->next;
  }

  return -1;
}

int address_list_iterate(struct _address_list *address_list, struct _address_list_iter *iter)
{
struct _memory_pool *memory_pool = address_list->memory_pool;
int token_len;

  if (iter->end_flag == 1) return -1;
  if (iter->memory_pool == NULL)
  {
    iter->memory_pool = address_list->memory_pool;
    iter->ptr = 0;
  }

  while(memory_pool != NULL)
  {
    if(iter->ptr < memory_pool->ptr)
    {
      token_len = strlen((char *)memory_pool->buffer + iter->ptr) + 1;
      memcpy(&iter->address, memory_pool->buffer + iter->ptr + token_len, sizeof(int));
      iter->name = memory_pool->buffer + iter->ptr;
      iter->ptr = iter->ptr+token_len + sizeof(int);
      iter->count++;
      return 0;
    }

    memory_pool = memory_pool->next;
  }

  iter->end_flag = 1;

  return -1;
}

int address_list_print(struct _address_list *address_list)
{
struct _address_list_iter iter;

  memset(&iter, 0, sizeof(iter));

  printf("%30s ADDRESS\n", "LABEL");

  while(address_list_iterate(address_list, &iter) != -1)
  {
    printf("%30s %08x (%d)\n", iter.name, iter.address, iter.address);
  }

  printf("Total %d.\n\n", iter.count);

  return 0;
}

int address_list_count_symbols(struct _address_list *address_list)
{
struct _memory_pool *memory_pool = address_list->memory_pool;
int token_len;
int ptr;
int count = 0;

  while(memory_pool != NULL)
  {
    ptr = 0;
    while(ptr < memory_pool->ptr)
    {
      token_len = strlen((char *)memory_pool->buffer + ptr) + 1;
      ptr = ptr + token_len + sizeof(int);
      count++;
    }

    memory_pool = memory_pool->next;
  }

  return count;
}
