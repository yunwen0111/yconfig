#include "yconfig.h"

#include <stdio.h>


int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("  Usage: %s config-file config-file2 ...\n", argv[0]);
        return 1;
    }

    yconfig_t *yc = yconfig_init();
    for (int i = 1; i != argc; ++i)
        yconfig_parse(yc, argv[i]);
    // Query here
    yconfig_destroy(yc);

    return 0;
}
