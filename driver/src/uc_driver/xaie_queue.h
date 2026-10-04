/******************************************************************************
* Copyright (C) 2024 AMD, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file queue.h
* @{
*
* This file contains queue implementation.
******************************************************************************/

#ifndef XAIE_QUEUE_H
#define XAIE_QUEUE_H

#include <stddef.h>
#include <stdbool.h>

#include "xaie_utils.h"

#define INVALID_QUEUE_ENTRY         0xFFFFFFFFU

typedef struct
{
  unsigned int front;
  unsigned int rear;
  unsigned int size;
}
queue_t;

static inline void queue_init(queue_t *q, const unsigned int max_size)
{
  q->front = 0;
  q->rear = max_size - 1U;
  q->size = 0;
}

static inline int queue_empty(queue_t *q)
{
  return (q->size == 0U) ? 1 : 0;
}

static inline int queue_full(queue_t *q, const unsigned int max_size)
{
  return (q->size == max_size) ? 1 : 0;
}

static inline unsigned int queue_size(queue_t *q)
{
  return q->size;
}

static inline unsigned int queue_push(queue_t *q, const unsigned int max_size)
{
  if (UNLIKELY(queue_full(q, max_size) != 0))
  {
    return INVALID_QUEUE_ENTRY;
  }

  q->rear = (q->rear + 1U) % max_size;
  ++q->size;

  return q->rear;
}

static inline unsigned int queue_push_front(queue_t *q, const unsigned int max_size)
{
  if (UNLIKELY(queue_full(q, max_size) != 0))
  {
    return INVALID_QUEUE_ENTRY;
  }

  q->front = (q->front != 0U) ? (q->front - 1U) : (max_size - 1U);
  ++q->size;

  return q->front;
}

static inline unsigned int queue_push_front_nocheck(queue_t *q, const unsigned int max_size)
{
  q->front = (q->front != 0U) ? (q->front - 1U) : (max_size - 1U);
  ++q->size;

  return q->front;
}

static inline unsigned int queue_push_nocheck(queue_t *q, const unsigned int max_size)
{
  q->rear = (q->rear + 1U) % max_size;
  ++q->size;

  return q->rear;
}

static inline unsigned int queue_pop(queue_t *q, const unsigned int max_size)
{
  if (UNLIKELY(queue_empty(q) != 0))
  {
    return INVALID_QUEUE_ENTRY;
  }

  const unsigned int res = q->front;

  q->front = (q->front + 1U) % max_size;
  --q->size;

  return res;
}

#endif

/** @} */