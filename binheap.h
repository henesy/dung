#ifndef BINHEAP_H
#define BINHEAP_H

struct binheap_node;
typedef struct binheap_node binheap_node_t;

typedef struct binheap {
  binheap_node_t **array;
  u32int size;
  u32int array_size;
  s32int (*compare)(const void *key, const void *with);
  void (*datum_delete)(void *);
} binheap_t;

void binheap_init(binheap_t *h,
                  s32int (*compare)(const void *key, const void *with),
                  void (*datum_delete)(void *));
void binheap_init_from_array(binheap_t *h,
                             void *array,
                             u32int size,
                             u32int nmemb,
                             s32int (*compare)(const void *key,
                                                const void *with),
                             void (*datum_delete)(void *));
void binheap_delete(binheap_t *h);
binheap_node_t *binheap_insert(binheap_t *h, void *v);
void *binheap_peek_min(binheap_t *h);
void *binheap_remove_min(binheap_t *h);
void binheap_decrease_key(binheap_t *h, binheap_node_t *n);
u32int binheap_is_empty(binheap_t *h);

#endif
