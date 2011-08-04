#include <string.h>
#include "mem.h"

memory ram;

void reset_ram() {
    memset(&ram, 0, sizeof(ram));
}
