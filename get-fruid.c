#include "errors.h"
#include "fruid-ops.h"
#include <stdlib.h>
#include <reimu.h>

int main(int argc, char *argv[])
{
    int bus = -1, slave = -1;
    char *path = "/var/volatile/motherboard_info.xml";

    if (argc == 4)
    {
        char *pb, *ps;
        bus = strtol(argv[1], &pb, 0);
        slave = strtol(argv[2], &ps, 0);
        path = argv[3];
        if (*pb || *ps) reimu_cancel(64, "Incorrect arguments (bus and slave address should be correct numbers)\n");
    }
    else if (argc != 1) reimu_cancel(63, "Incorrect arguments (use without any arguments or with bus and slave address)\n");

    int rv = get_fruid(path, bus, slave);
    if (rv) print_err(rv, "while reading FRU ID information");
    return rv;
}
