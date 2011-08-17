#include "type.h"
#include <stdio.h>

void welcome() {
    printf(
"|*----------------------------------------------------------------------|-_-|-*|" \
"|*--| FC simulate              |----------------------------------------------*|" \
"|---| CatfoOD '2011            |---| yanming-sohu@sohu.com |-------------------|" \
"|---| is Free 'soFtwarE        |---| 412475540             |-------------------|" \
"|*----------------------------------------------------------------------|+_+|-*|" \
    "\n");
}

void printArr(byte* arr, int offset, int len) {

    for (int i=offset; i<len+offset; ++i) {
        if ((i-offset)%16==0) printf("\n0x%04X: ", i);
        printf(" %02X", arr[i]);
    }

    printf("\n");
}
