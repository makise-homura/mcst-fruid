#ifndef I2C_OPS_H
#define I2C_OPS_H

#include <sys/types.h>

struct i2c_desc_t
{
    int fd;
    size_t seek;
};

int i2c_init(struct i2c_desc_t *desc, int bus, int slave);
int i2c_read(struct i2c_desc_t *desc, void *buf, off_t offset, size_t size);
int i2c_deinit(struct i2c_desc_t *desc);

#endif
