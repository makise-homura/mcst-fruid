#ifndef DICT_OPS_H
#define DICT_OPS_H

struct dict_t
{
    int key;
    const char *value;
};

const char *dict_search(const struct dict_t *dict, int key);

#endif
