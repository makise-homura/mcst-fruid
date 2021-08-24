#ifndef ERRORS_H
#define ERRORS_H

enum
{
    ERR_NONE = 0,    // Generic no error
    ERR_EOF,         // Generic end of file
    ERR_ENOMEM,      // Generic no memory

    ERR_FRU_EMPTYNAME,            // Empty user parameter name
    ERR_FRU_OVERSIZE,             // Oversized FRU information
    ERR_FRU_CKSUM,                // FRU data checksum is bad
    ERR_FRU_VERSION,              // Illegal FRU data version
    ERR_FRU_FORMAT_UNSUPPORTED,   // FRU data string format is unsupported
    ERR_FRU_FILE,                 // Can't open FRU data XML file
    ERR_FRU_FWRITE,               // Can't write FRU data XML file
    ERR_FRU_EMPTYTLB,             // Type-Length Byte for empty field

    ERR_I2C_TRAVERSE,  // Can't traverse I2C bus to find EEPROM
    ERR_I2C_OPEN,      // Can't open I2C bus
    ERR_I2C_CREATE,    // Can't create EEPROM instance on I2C bus
    ERR_I2C_SLAVE,     // Can't select EEPROM slave device on I2C bus
    ERR_I2C_SEEK,      // Can't seek I2C EEPROM
    ERR_I2C_READ,      // Can't read I2C EEPROM
    ERR_I2C_CLOSE,     // Can't close I2C bus

    ERR_SPI_NODEV,     // Can't find SPI flash device
    ERR_SPI_OPEN,      // Can't open SPI flash device
    ERR_SPI_DETECT,    // Can't detect ID of SPI flash device
    ERR_SPI_SIZE,      // Can't determine size of SPI flash device
    ERR_SPI_SEEK,      // Can't seek SPI flash device
    ERR_SPI_READ,      // Can't read SPI flash device
    ERR_SPI_CLOSE,     // Can't close SPI flash device

    ERR_FPT_MAGIC,     // Flash partition table not detected
    ERR_FPT_VERSION,   // Unsupported flash partition table version
    ERR_FPT_OVERRUN,   // No suitable partition table entry found

    ERR_DTB_FILE,      // Can't open DTB file
    ERR_DTB_FWRITE,    // Can't write DTB file
    ERR_DTB_MAGIC,     // DTB data magic number is bad
    ERR_DTB_OVERSIZE,  // Oversized DTB information
};

void print_err(int code, const char *where);

#endif
