#include "errors.h"
#include "spi-ops.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MTD_DEVICE_NAME "sys"

static int detect_device(const char *mtdname)
{
    char link[50], *path, *path_s, *path_e;
    snprintf(link, 50, "/dev/mtd/%s", mtdname);
    if((path = realpath(link, NULL)) == NULL) return -1;

    int rv = -2;
    if(!strncmp(path, "/dev/mtd", 8))
    {
        path_s = path + 8;
        rv = strtol(path_s, &path_e, 10);
        if (path_e == path_s) rv = -3;
    }
    free(path);
    return rv;
}

int spi_init(struct spi_desc_t *desc)
{
    char dev[50];
    if((desc->mtd = detect_device(MTD_DEVICE_NAME)) < 0) return ERR_SPI_NODEV;
    snprintf(dev, 50, "/dev/mtdblock%d", desc->mtd);
    if((desc->fd = open(dev, O_RDWR)) == -1) return ERR_SPI_OPEN;
    desc->seek = 0;
    return 0;
}

int spi_read(struct spi_desc_t *desc, void *buf, off_t offset, size_t size)
{
    if (desc->seek != offset)
    {
        if(lseek(desc->fd, offset, SEEK_SET) != offset) return ERR_SPI_SEEK;
        desc->seek = offset;
    }
    if (read(desc->fd, buf, size) != size) return ERR_SPI_READ;
    desc->seek += size;
    return 0;
}

int spi_deinit(struct spi_desc_t *desc)
{
    return close(desc->fd) ? ERR_SPI_CLOSE : 0;
}

int spi_get_sectorsize(struct spi_desc_t *desc, size_t *size)
{
    // Currently we are hardware-bound to flash chips with 4k sector.
    // There's no way to found it out from BMC side.
    *size = 4096;
    return 0;
}

int spi_get_size(struct spi_desc_t *desc, size_t *size)
{
    int rv = 0;
    FILE *f;

    char sizefile[100];
    snprintf(sizefile, 100, "/sys/class/mtd/mtd%d/size", desc->mtd);
    if((f = fopen(sizefile, "r")) == NULL) return ERR_SPI_SIZE;

    long lsize;
    if (fscanf(f, "%ld", &lsize) == EOF) rv = ERR_SPI_SIZE;
    *size = lsize;

    return rv;
}
