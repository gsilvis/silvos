#ifndef __SILVOS_LIST_H
#define __SILVOS_LIST_H

#include <stddef.h>

struct list_head {
  struct list_head *next, *prev;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name)  struct list_head name = LIST_HEAD_INIT(name)

static inline int list_empty (struct list_head *head) {
  return head == head->next;
}

static inline void __list_add (struct list_head *to_add,
                               struct list_head *prev,
                               struct list_head *next) {
  next->prev = to_add;
  to_add->next = next;
  to_add->prev = prev;
  prev->next = to_add;
}

static inline void list_push_front (struct list_head *to_add,
                                    struct list_head *head) {
  __list_add(to_add, head, head->next);
}

static inline void list_push_back (struct list_head *to_add,
                                   struct list_head *head) {
  __list_add(to_add, head->prev, head);
}

static inline void list_remove (struct list_head *to_remove) {
  to_remove->next->prev = to_remove->prev;
  to_remove->prev->next = to_remove->next;
  to_remove->prev = NULL;
  to_remove->next = NULL;
}

static inline struct list_head *list_pop_front (struct list_head *head) {
  if (list_empty(head)) {
    return NULL;
  }
  struct list_head *result = head->next;
  list_remove(result);
  return result;
}

static inline struct list_head *list_pop_back (struct list_head *head) {
  if (list_empty(head)) {
    return NULL;
  }
  struct list_head *result = head->prev;
  list_remove(result);
  return result;
}

#endif
