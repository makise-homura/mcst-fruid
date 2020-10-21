#include "errors.h"
#include "spi-ops.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#define MTD_DEVICE_NUMBER 6

int spi_init(struct spi_desc_t *desc)
{
    char dev[50];
    desc->mtd = MTD_DEVICE_NUMBER;
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
