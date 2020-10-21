#ifndef SPI_OPS_H
#define SPI_OPS_H

#include <sys/types.h>

struct spi_desc_t
{
    int fd;
    int mtd;
    size_t seek;
};

int spi_init(struct spi_desc_t *desc);
int spi_read(struct spi_desc_t *desc, void *buf, off_t offset, size_t size);
int spi_deinit(struct spi_desc_t *desc);
int spi_get_sectorsize(struct spi_desc_t *desc, size_t *size);
int spi_get_size(struct spi_desc_t *desc, size_t *size);

#endif
