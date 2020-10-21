#include "errors.h"
#include "devtree-ops.h"
#include "fruid-ops.h"
#include "dict-ops.h"
#include <stdlib.h>

int main()
{
    int rv;

    rv = get_fruid("/var/volatile/motherboard_info.xml");
    if (rv)
    {
        print_err(rv, "while reading FRU ID information");
        return rv;
    }

    rv = get_devtree("/var/volatile/motherboard_devtree.dtb");
    if (rv)
    {
        print_err(rv, "while reading device tree");
        return rv;
    }

    return 0;//system("dtc -I dtb -O dts -o /var/volatile/motherboard_devtree.dts /var/volatile/motherboard_devtree.dtb");
}
