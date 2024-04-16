#include "errors.h"
#include "spi-ops.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#define SPIDEV_CONTROLLER 0
#define SPIDEV_CS_LINE 0

#define SPI_BLOCKSIZE 64

struct spi_ioc_transfer spi_msg[2] =
{
    {
        .tx_buf = (__u64)NULL,
        .rx_buf = (__u64)NULL,
        .len = 0,
        .speed_hz = 12500000,
        .delay_usecs = 0,
        .bits_per_word = 8,
        .cs_change = 0
    },
    {
        .rx_buf = (__u64)NULL,
        .tx_buf = (__u64)NULL,
        .len = 0,
        .speed_hz = 12500000,
        .delay_usecs = 0,
        .bits_per_word = 8,
        .cs_change = 0
    }
};


static int spidev_read_block(struct spi_desc_t *desc, void *buf, off_t offset, size_t len)
{
    char readcmd[4] =
    {
        0x03, // Read command
        (char)(offset >> 16),
        (char)((offset >> 8) & 0xFF),
        (char)(offset & 0xFF)
    };

    spi_msg[0].tx_buf = (__u64)readcmd;
    spi_msg[0].len = 4;
    spi_msg[1].rx_buf = (__u64)buf;
    spi_msg[1].len = len;

    if (ioctl(desc->fd, SPI_IOC_MESSAGE(2), spi_msg) == -1) return ERR_SPI_READ;
    return 0;
}

int spi_init(struct spi_desc_t *desc)
{
    char dev[50];
    snprintf(dev, 50, "/dev/spidev%d.%d", SPIDEV_CONTROLLER, SPIDEV_CS_LINE);
    if((desc->fd = open(dev, O_RDWR)) == -1) return ERR_SPI_OPEN;
    desc->seek = 0;
    return 0;
}

int spi_read(struct spi_desc_t *desc, void *buf, off_t offset, ssize_t size)
{
    desc->seek = offset;
    while (size > 0)
    {
        size_t readsize = (size > SPI_BLOCKSIZE) ? SPI_BLOCKSIZE : size;
        if (spidev_read_block(desc, buf, desc->seek, readsize) != 0) return ERR_SPI_READ;
        buf += readsize;
        desc->seek += readsize;
        size -= readsize;
    }
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

    struct __attribute__((packed))
    {
        uint8_t manufacturer;
        uint8_t type;
        uint8_t capacity;
    } id;

    char readcmd = 0x9f; // Read ID command

    spi_msg[0].tx_buf = (__u64)&readcmd;
    spi_msg[0].len = 1;
    spi_msg[1].rx_buf = (__u64)&id;
    spi_msg[1].len = 3;

    if (ioctl(desc->fd, SPI_IOC_MESSAGE(2), spi_msg) == -1) return ERR_SPI_READ;

    switch(id.capacity)
    {
        case 0x15:
        case 0x16:
        case 0x17:
        case 0x18:
        case 0x19:
        case 0x1a:
            *size = (1L << id.capacity); return 0;
        default:
            return ERR_SPI_DETECT;
    }
}
