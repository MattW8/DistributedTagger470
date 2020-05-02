/**
 * distributed hash table
 *
 */
#include <stdlib.h>

#include "local_storage.h"

int dht_init();

void dht_put(const char *key, long value);
long dht_get(const char *key);
void dht_destroy();

