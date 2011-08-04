#ifndef CPU_H_INCLUDED
#define CPU_H_INCLUDED

#include "type.h"

extern cpu_6502 cpu;

/** 重置cpu状态 */
void        reset_cpu();
/** 向堆栈中压数 */
void        push     (byte d);
/** 从堆栈中取数 */
byte        pop      ();

#endif // CPU_H_INCLUDED
