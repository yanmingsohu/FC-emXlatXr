#ifndef DEBUG_H_INCLUDED
#define DEBUG_H_INCLUDED

/**
 * 接管系统的执行,直到返回,期间可以通过命令调试
 * 整个系统
 */
void debugCpu(NesSystem *fc);

#endif // DEBUG_H_INCLUDED
