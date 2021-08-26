#include "errors.h"
#include "devtree-ops.h"
#include "fruid-ops.h"
#include "dict-ops.h"
#include <stdlib.h>

int main()
{
    int rv1, rv2, rv3 = 0;

    rv2 = get_devtree("/var/volatile/motherboard_devtree.dtb");
    if (rv2)
    {
        print_err(rv2, "while reading device tree");
    }
    else
    {
        rv3 = system("dtc -I dtb -O dts -o /var/volatile/motherboard_devtree.dts /var/volatile/motherboard_devtree.dtb");
    }

    rv1 = get_fruid("/var/volatile/motherboard_info.xml");
    if (rv1)
    {
        print_err(rv1, "while reading FRU ID information");
    }

    return rv1 + rv2 + rv3;
}
