#include "errors.h"
#include "fruid-ops.h"

int main()
{
    int rv = get_fruid("/var/volatile/motherboard_info.xml");
    if (rv) print_err(rv, "while reading FRU ID information");
    return rv;
}
