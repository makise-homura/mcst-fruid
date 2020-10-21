#include "errors.h"
#include "spi-ops.h"
#include <endian.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#define DTB_SIGNATURE   0xff000003
#define DTB_DEFAULTADDR 0x00700000
#define DTB_DEFAULTSIZE 0x00100000

typedef union
{
    uint32_t crc32;
    uint32_t stribog[8];
} cksum256_t;

static int get_pt_offset(struct spi_desc_t *desc, off_t *offset)
{
    int rv = 0;
    off_t flashsize = 0;
    size_t flashsector = 0;
    spi_get_sectorsize(desc, &flashsector); // For now it never fails
    if ((rv = spi_get_size(desc, &flashsize)) != 0) return rv;
    *offset = ((flashsize > 0x1000000) ? 0x1000000 : flashsize) - flashsector;
    return 0;
}

static int get_devtree_address_from_parttable(struct spi_desc_t *desc, off_t *dt_address, size_t *dt_size)
{

    struct __attribute__((__packed__)) pt_header_t
    {
        char pt_signature[4];
        uint32_t pt_version;
        uint32_t pt_n_entries;
        cksum256_t pt_cksum;
        uint8_t padding[20];
    } pt_header;

    int rv;
    off_t pt_offset;
    if((rv = get_pt_offset(desc, &pt_offset)) != 0) return rv;
    if((rv = spi_read(desc, &pt_header, pt_offset, sizeof(pt_header))) != 0) return rv;
    uint32_t crc = pt_header.pt_cksum.crc32;
    memset(&pt_header.pt_cksum, 0, sizeof(pt_header.pt_cksum));
//    if (crc != crc32(crc32(0L, Z_NULL, 0), (Bytef *)&pt_header, sizeof(pt_header))) return ERR_FPT_CKSUM;
    // [TODO: Shall check header version here]

    for(uint32_t i = 0; i < pt_header.pt_n_entries; ++i)
    {
        struct __attribute__((__packed__)) pt_entry_t
        {
            uint32_t pte_signature;
            uint32_t pte_offset;
            uint32_t pte_size;
            cksum256_t pte_cksum;
            uint8_t padding[20];
        } pt_entry;

        if((rv = spi_read(desc, &pt_entry, pt_offset + sizeof(pt_header) + i * sizeof(pt_entry), sizeof(pt_entry))) != 0) return rv;
        uint32_t crc = pt_entry.pte_cksum.crc32;
        memset(&pt_entry.pte_cksum, 0, sizeof(pt_entry.pte_cksum));
//        if (crc != crc32(crc32(0L, Z_NULL, 0), (Bytef *)&pt_entry, sizeof(pt_entry))) return ERR_FPT_PTE_CKSUM;

        if (pt_entry.pte_signature == DTB_SIGNATURE)
        {
            *dt_address = pt_entry.pte_offset;
            *dt_size = pt_entry.pte_size;
            return 0;
        }
    }
    return ERR_FPT_OVERRUN;
}

static int check_dtb_consistency(struct spi_desc_t *desc, off_t dtb_addr, size_t *dtb_size)
{
    int rv;
    uint32_t dtb_magic, dtb_realsize;

    if((rv = spi_read(desc, &dtb_magic, dtb_addr, sizeof(uint32_t))) != 0) return rv;
    if(be32toh(dtb_magic) != 0xd00dfeed) return ERR_DTB_MAGIC;

    if((rv = spi_read(desc, &dtb_realsize, dtb_addr + sizeof(uint32_t), sizeof(uint32_t))) != 0) return rv;
    dtb_realsize = be32toh(dtb_realsize);
    if (dtb_realsize > *dtb_size) return ERR_DTB_OVERSIZE;

    *dtb_size = dtb_realsize;
    return 0;
}

static int get_devtree_dtb(void **dtb_data, size_t *dtb_size) // Note: dtb_data should be freed even on error
{
    int rv;

    struct spi_desc_t desc;
    if((rv = spi_init(&desc)) != 0) return rv;

    off_t dtb_addr = DTB_DEFAULTADDR;
    *dtb_size = DTB_DEFAULTSIZE;

    rv = get_devtree_address_from_parttable(&desc, &dtb_addr, dtb_size);
    if(rv == ERR_FPT_CKSUM) rv = 0; // Fallback to default

    if(!rv) rv = check_dtb_consistency(&desc, dtb_addr, dtb_size);

    if(!rv) if((*dtb_data = malloc(*dtb_size)) == NULL) rv = ERR_ENOMEM;
    if(!rv) rv = spi_read(&desc, *dtb_data, dtb_addr, *dtb_size);
    spi_deinit(&desc);
    return rv;
}

int get_devtree(const char *outputfile)
{
    void *dtb_data = NULL;
    size_t dtb_size;
    FILE *f = NULL;
    int rv = 0;

    rv = get_devtree_dtb(&dtb_data, &dtb_size);

    if (!rv) if ((f = fopen(outputfile, "w")) == NULL) rv = ERR_DTB_FILE;
    if (!rv) if (fwrite(dtb_data, dtb_size, 1, f) != 1) rv = ERR_DTB_FWRITE;

    free(dtb_data);
    if(f) fclose(f);

    return rv;
}
