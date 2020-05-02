#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dictionary_generator.h"

/*
 * Local hash table function prototypes
 */
void   local_init();
void   local_put(const char *key, long value);
int   local_get(const char *key);
void   local_destroy(FILE *out);
void update_tags(struct tagcounts_t *, int );
int get_highest_frequency(struct tagcounts_t*);

