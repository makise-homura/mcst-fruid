#include "errors.h"
#include "i2c-ops.h"
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <reimu.h>

#define FRUID_I2C_DEFAULT_BUS 3
#define FRUID_I2C_DEFAULT_SLAVE 0x57

struct info_area_def_t
{
    enum
    {
        IADEF_END_OF_LIST = 0,
        IADEF_RAW,
        IADEF_TLV,
        IADEF_24BIT_LE_DATE,
        IADEF_CHASSISTYPE
    } iadef_type;
    const char *iadef_name;
    int iadef_langcode_applies;
};

const struct info_area_def_t board_info_area_def[] =
{
    { IADEF_24BIT_LE_DATE, "mfg_date_time", 0 },
    { IADEF_TLV,           "manufacturer",  1 },
    { IADEF_TLV,           "product_name",  1 },
    { IADEF_TLV,           "serial_number", 0 },
    { IADEF_TLV,           "part_number",   1 },
    { IADEF_TLV,           "fru_file_id",   0 },
    { IADEF_END_OF_LIST,   "",              0 }
};

enum tlb_type_t
{
    TLB_BINARY = 0,
    TLB_BCDPLUS = 1,
    TLB_SIXBITASCII = 2,
    TLB_LANGRELATED = 3
};

static uint8_t fru_cksum(uint8_t *block, size_t size)
{
    uint8_t rv = 0;
    for(size_t i = 0; i < size; ++i) rv += block[i];
    return rv;
}

static int read_tlb_field(struct i2c_desc_t *desc, char **buf, off_t *offset)
{
    int rv = 0;

    uint8_t tlb;
    if((rv = i2c_read(desc, &tlb, *offset, 1)) != 0) return rv;
    ++(*offset);
    if(tlb == 0xc1) return ERR_FRU_EMPTYTLB;

    enum tlb_type_t type = tlb >> 6;

    size_t size = tlb & 0x3f;

    if(size == 0)
    {
        if((*buf = malloc(1)) == NULL) return ERR_ENOMEM;
        **buf = '\0';
        return 0;
    }

    switch(type)
    {
        case TLB_BINARY:
            if((*buf = malloc(size * 6)) == NULL) return ERR_ENOMEM; // Should be freed by calling function even on error
            char *out = *buf;
            for(size_t i = 0; i < size; ++i)
            {
                uint8_t byte;
                if((rv = i2c_read(desc, &byte, *offset, 1)) != 0) return rv;
                ++(*offset);
                if(out != *buf) out += sprintf(out, ", ");
                out += sprintf(out, "0x%02X", byte);
            }
            break;

        case TLB_BCDPLUS:
            // [TODO: Currently we don't support this format, maybe it is up to be implemented in future]
            return ERR_FRU_FORMAT_UNSUPPORTED;

        case TLB_SIXBITASCII:
            // [TODO: Currently we don't support this format, maybe it is up to be implemented in future]
            return ERR_FRU_FORMAT_UNSUPPORTED;

        case TLB_LANGRELATED:
            // [TODO: We expect UTF-8 as encoding of these fields. It may be the wrong assumption!]
            if((*buf = malloc(size + 1)) == NULL) return ERR_ENOMEM; // Should be freed by calling function even on error
            if((rv = i2c_read(desc, *buf, *offset, size)) != 0) return rv;
            (*buf)[size] = '\0';
            *offset += size;
            break;
    }
    return 0;
}

static int dump_fru_iua(struct i2c_desc_t *desc, FILE *f, off_t offset)
{
    int rv = 0;

    if(!offset) return 0;
    offset *= 8;

    uint8_t ver;
    if((rv = i2c_read(desc, &ver, offset++, 1)) != 0) return rv;
    if(ver != 1) return ERR_FRU_VERSION;

    if(fputs("\t<internal_use_area>\n", f) == EOF) return ERR_FRU_FWRITE;

    char *name = NULL;
    char *value = NULL;

    while(!rv && (offset < 0x800))
    {
        rv = read_tlb_field(desc, &name, &offset);
        if(rv == ERR_FRU_EMPTYTLB) { rv = 0; break; }
        if(!strcmp(name, "")) rv = ERR_FRU_EMPTYNAME;

        if(!rv) rv = read_tlb_field(desc, &value, &offset);

        if(!rv)
            if(fprintf(f, "\t\t<%s>%s</%s>\n", name, value, name) < 1) rv = ERR_FRU_FWRITE;

        free(name);
        free(value);
    }
    if(offset >= 0x800) return ERR_FRU_OVERSIZE;

    if(!rv)
        if(fputs("\t</internal_use_area>\n", f) == EOF) rv = ERR_FRU_FWRITE;

    return rv;
}

static int dump_fru_mra(struct i2c_desc_t *desc __attribute__((unused)), FILE *f __attribute__((unused)), off_t offset __attribute__((unused)))
{
    if(!offset) return 0;

    // [TODO: Currently we don't support decoding Multi-Record area, maybe it is up to be implemented in future]
    return ERR_FRU_FORMAT_UNSUPPORTED;
}

static int dump_fru_xia(struct i2c_desc_t *desc, FILE *f, off_t offset, const char *tagname, const struct info_area_def_t *info_area_def)
{
    if(!offset) return 0;
    offset *= 8;

    if(info_area_def == NULL) return ERR_FRU_FORMAT_UNSUPPORTED; // pass NULL for currently unsupported area types

    if(fprintf(f, "\t<%s>\n", tagname) < 0) return ERR_FRU_FWRITE;

    int rv;
    int extra_field_num = -1; // should become zero for first extra field

    uint8_t ver;
    if((rv = i2c_read(desc, &ver, offset++, 1)) != 0) return rv;
    if(ver != 1) return ERR_FRU_VERSION;

    uint8_t len;
    if((rv = i2c_read(desc, &len, offset++, 1)) != 0) return rv;
    off_t end = (off_t)len * 8 + offset;

    uint8_t lc;
    if((rv = i2c_read(desc, &lc, offset++, 1)) != 0) return rv; // unused for now

    for(;;)
    {
        if(offset >= end) return ERR_FRU_OVERSIZE;

        int iadef_real_type;
        char iadef_name_buf[50];
        const char *iadef_real_name = iadef_name_buf;
        if(info_area_def->iadef_type == IADEF_END_OF_LIST)
        {
            iadef_real_type = IADEF_TLV;
            snprintf(iadef_name_buf, 50, "extra_field_%d", ++extra_field_num);
        }
        else
        {
            iadef_real_type = info_area_def->iadef_type;
            iadef_real_name = info_area_def->iadef_name;
        }

        switch(iadef_real_type)
        {
            case IADEF_24BIT_LE_DATE:
            {
                char buf[256];
                uint8_t date[3];
                if((rv = i2c_read(desc, &date, offset, 3)) != 0) return rv;
                offset += 3;
                time_t t = 820454400 + ((time_t)date[0] + (((time_t)date[1]) << 8) + (((time_t)date[2]) << 16)) * 60; // 820454400 is 1/1/96
                if(ctime_r (&t, buf) == NULL) return ERR_FRU_FORMAT_UNSUPPORTED;
                if(buf[strlen(buf) - 1] == '\n') buf[strlen(buf) - 1] = '\0';
                if(fprintf(f, "\t\t<%s type=\"timestamp\">%ld</%s>\n\t\t<%s type=\"string\">%s</%s>\n", iadef_real_name, t, iadef_real_name, iadef_real_name, buf, iadef_real_name) < 0) return ERR_FRU_FWRITE;
                break;
            }

            case IADEF_TLV:
            {
                char *buf = NULL;
                // [TODO: we should somehow check iadef_langcode_applies and lc here]
                int rv = read_tlb_field(desc, &buf, &offset);
                if(rv == ERR_FRU_EMPTYTLB)
                {
                    free(buf);
                    if(fprintf(f, "\t</%s>\n", tagname) < 0) return ERR_FRU_FWRITE;
                    return 0;
                }
                if(fprintf(f, "\t\t<%s>%s</%s>\n", iadef_real_name, buf, iadef_real_name) < 0) rv = ERR_FRU_FWRITE;
                free(buf);
                if(rv) return rv;
                break;
            }

            default:
                // [TODO: Currently we don't support any of formats except specified above, maybe it is up to be implemented in future]
                return ERR_FRU_FORMAT_UNSUPPORTED;
        }

        if(info_area_def->iadef_type != IADEF_END_OF_LIST) ++info_area_def;
    }
}

struct i2c_addr_t { int detected; int bus; int slave; };

static int find_i2c_dev(enum cancel_type_t unused __attribute__((unused)), const char *pcompatible, int unused2 __attribute__((unused)), int bus, int reg, const char *label, const void *data)
{
    const void *pdata = *(void * const *)data;
    struct i2c_addr_t *i2c_addr = (struct i2c_addr_t *)pdata;
    if (!strcmp(pcompatible, "24c128") && !strcmp(label, "FRUID"))
    {
        i2c_addr->detected = 1;
        i2c_addr->bus = bus;
        i2c_addr->slave = reg;
    }
    return 0;
}

int get_fruid(const char *filename, int bus, int slave)
{
    FILE *f;
    int rv = 0;
    struct i2c_desc_t desc;

    struct __attribute__((__packed__)) fru_common_header_t
    {
        uint8_t format_version;
        uint8_t offset_iua;
        uint8_t offset_cia;
        uint8_t offset_bia;
        uint8_t offset_pia;
        uint8_t offset_mra;
        uint8_t pad;
        uint8_t cksum;
    } fru_common_header;

    struct i2c_addr_t i2c_addr = { .detected = 0, .bus = FRUID_I2C_DEFAULT_BUS, .slave = FRUID_I2C_DEFAULT_SLAVE };

    if (bus < 0)
    {
        const void* paddr = &i2c_addr;

        reimu_message(stderr, "Searching FRU ID EEPROM in DTB: ");
        if((rv = reimu_traverse_all_i2c(&paddr, find_i2c_dev, JUST_PRINT_ERROR)) > 2) return ERR_I2C_TRAVERSE;
        if(!rv) reimu_message(stderr, "Finished.\n");

        if (i2c_addr.detected) reimu_message(stdout, "FRU ID EEPROM detected at bus %d, addr 0x%02x\n", i2c_addr.bus, i2c_addr.slave);
        else reimu_message(stdout, "FRU ID EEPROM not detected, using defaults: bus %d, addr 0x%02x\n", i2c_addr.bus, i2c_addr.slave);
    }
    else
    {
        i2c_addr.bus = bus;
        i2c_addr.slave = slave;
        reimu_message(stdout, "FRU ID EEPROM forced address: bus %d, addr 0x%02x\n", i2c_addr.bus, i2c_addr.slave);
    }

    if((rv = i2c_init(&desc, i2c_addr.bus, i2c_addr.slave)) != 0) return rv;

    rv = i2c_read(&desc, &fru_common_header, 0, 8);
    if(!rv) if(fru_cksum((uint8_t *)&fru_common_header, 8) != 0) rv = ERR_FRU_CKSUM;
    if(!rv) if(fru_common_header.format_version != 1) rv = ERR_FRU_VERSION;

    if(!rv) if((f = fopen(filename, "w")) == NULL) rv = ERR_FRU_FILE;
    if(!rv) if(fputs("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<fruinfo>\n", f) == EOF) rv = ERR_FRU_FWRITE;

    if(!rv) rv = dump_fru_iua(&desc, f, fru_common_header.offset_iua);
    // [TODO: Currently we don't support decoding Chassis/Product Info areas, maybe it is up to be implemented in future]
    if(!rv) rv = dump_fru_xia(&desc, f, fru_common_header.offset_cia, "chassis_info_area", NULL);
    if(!rv) rv = dump_fru_xia(&desc, f, fru_common_header.offset_bia, "board_info_area", board_info_area_def);
    if(!rv) rv = dump_fru_xia(&desc, f, fru_common_header.offset_pia, "product_info_area", NULL);
    if(!rv) rv = dump_fru_mra(&desc, f, fru_common_header.offset_mra);

    if(!rv) if(fputs("</fruinfo>\n", f) == EOF) rv = ERR_FRU_FWRITE;

    if(f) fclose(f);
    i2c_deinit(&desc);
    if(rv) unlink(filename);
    return rv;
}
