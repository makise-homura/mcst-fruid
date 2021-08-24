#include "errors.h"
#include "devtree-ops.h"

int main()
{
    int rv = get_devtree("/var/volatile/motherboard_devtree.dtb");
    if (rv) print_err(rv, "while reading device tree");
}
