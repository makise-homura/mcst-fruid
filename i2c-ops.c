#include "errors.h"
#include "i2c-ops.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>

#define I2C_DEV_TYPE "24c128"
#define I2C_BLOCKSIZE 64

static int i2c_create_device(int bus, int slave)
{
    char newdevfile[256];
    snprintf(newdevfile, 256, "/sys/class/i2c-adapter/i2c-%d/new_device", bus);

    FILE *f;
    int rv = 0;
    if((f = fopen(newdevfile, "w")) == NULL) return ERR_I2C_CREATE;
    if(fprintf(f, "%s 0x%02x\n", I2C_DEV_TYPE, slave) < 0) rv = ERR_I2C_CREATE;
    if(fclose(f)) return ERR_I2C_CREATE;
    return rv;
}

int i2c_init(struct i2c_desc_t *desc, int bus, int slave)
{
    char dev[256];
    snprintf(dev, 256, "/sys/class/i2c-adapter/i2c-%d/%d-%04x/eeprom", bus, bus, slave);

    struct stat st;
    if(stat(dev, &st) == -1)
    {
        if(errno == ENOENT)
        {
            int rv = i2c_create_device(bus, slave);
            if (rv) return rv;
        }
        else return ERR_I2C_SLAVE;
    }

    if((desc->fd = open(dev, O_RDONLY)) == -1) return ERR_I2C_OPEN;
    desc->seek = 0;
    return 0;
}

int i2c_read(struct i2c_desc_t *desc, void *buf, off_t offset, size_t size)
{
    if (desc->seek != offset)
    {
        if(lseek(desc->fd, offset, SEEK_SET) != offset) return ERR_I2C_SEEK;
        desc->seek = offset;
    }

    char *cbuf = buf;
    while (size > 0)
    {
        size_t readsize = (size > I2C_BLOCKSIZE) ? I2C_BLOCKSIZE : size;
        if (read(desc->fd, cbuf, readsize) < 0) return ERR_I2C_READ;
        cbuf += readsize;
        desc->seek += readsize;
        size -= readsize;
    }
    return 0;
}

int i2c_deinit(struct i2c_desc_t *desc)
{
    return close(desc->fd) ? ERR_I2C_CLOSE : 0;
}
