#include "dict-ops.h"

const char *dict_search(const struct dict_t *dict, int key)
{
    for(;dict->key;++dict) if (dict->key == key) break;

    return dict->value;
}
