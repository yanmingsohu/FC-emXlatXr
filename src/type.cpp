#include  <stdio.h>
#include <stdarg.h>
#include   "type.h"
#include    "rom.h"

void welcome() {
    printf(
"|*----------------------------------------------------------------------|-_-|-*|\n" \
"|*--| FC simulate              |----------------------------------------------*|\n" \
"|*--| CatfoOD '2011            |---| yanming-sohu@sohu.com |------------------*|\n" \
"|*--| is Free 'soFtwarE        |---| 412475540             |------------------*|\n" \
"|*----------------------------------------------------------------------|+_+|-*|\n" \
    "\n");
}

void printArr(byte* arr, int offset, int len) {

    for (int i=offset; i<len+offset; ++i) {
        if ((i-offset)%16==0) printf("\n0x%04X: ", i);
        printf(" %02X", arr[i]);
    }

    printf("\n");
}

char* parseOpenError(int load_err_code) {
    static char STR[64];

    switch (load_err_code) {

    case LOAD_ROM_SUCCESS:
        sprintf(STR, "load success.\n");
        break;

    case ER_LOAD_ROM_PARM:
        sprintf(STR, "parm not null.\n");
        break;

    case ER_LOAD_ROM_OPEN:
        sprintf(STR, "\ncannot open rom file.\n");
        break;

    case ER_LOAD_ROM_HEAD:
        sprintf(STR, "The file not nes rom. \n");
        break;

    case ER_LOAD_ROM_TRAINER:
        sprintf(STR, "The 'trainer' not enough.\n");
        break;

    case ER_LOAD_ROM_SIZE:
        sprintf(STR, "cannot read enough rom size.\n");
        break;

    case ER_LOAD_ROM_VSIZE:
        sprintf(STR, "cannot read enough vrom size.\n");
        break;

    case ER_LOAD_ROM_BADMAP:
        sprintf(STR, "unsupport map id\n");
        break;

    default:
        sprintf(STR, "unknow error.\n");
    }

    return STR;
}
