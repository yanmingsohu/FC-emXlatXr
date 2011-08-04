#ifndef CPU_H_INCLUDED
#define CPU_H_INCLUDED

#include "type.h"
#include "mem.h"

struct cpu_6502 {

private:
/* NTSC制式机型运行频率为1.7897725 MHz
 * PAL制式机型运行频率为1.773447 MHz                       */
#define CPU_NTSC    1.7897725
#define CPU_PAL     1.773447

    byte A; 	    /* 累加器                              */
    byte Y;	    	/* 索引暂存器                          */
    byte X;	    	/* 索引暂存器                          */

    union {
        struct { byte PCH; byte PCL; };
        word PC;
    };  			/* 程序计数器,指向下一个要执行的指令   */

    word SP;		/* 堆叠指示器 0x0100-0x01FF
                     * 递减的, 指向'空', 每次2字节         */

#define CPU_FLAGS_NEGATIVE    (1<<7)  /* 负值              */
#define CPU_FLAGS_OVERFLOW    (1<<6)  /* 有溢出            */
#define CPU_FLAGS_BREAK       (1<<4)  /* 有中断            */
#define CPU_FLAGS_DECIMAL     (1<<3)  /* 十进制            */
#define CPU_FLAGS_INTERDICT   (1<<2)  /* 禁止中断          */
#define CPU_FLAGS_ZERO        (1<<1)  /* 结果为零          */
#define CPU_FLAGS_CARRY       1       /* 有进位            */

    byte FLAGS;	    /* 状态暂存器  N V 1 B D I Z C
					 * N负值 	V溢出		B中断命令
					 * D十进制	I插断禁能	Z零值
					 * C进位                               */
    memory *ram;    /* 内存                                */

public:
    cpu_6502(memory* ram);

    void    reset();            /* 重置cpu状态             */
    void    push(byte d);       /* 向堆栈中压数            */
    byte    pop();              /* 从堆栈中取数            */
    void    debug();            /* 打印cpu状态             */
    void    process();          /* 处理当前命令并指向下一条命令 */
};

struct command_6502 {
    char name[5];
    byte time;
    byte len;
};

command_6502 command_list_6502[] = {
    {"BRK", 7, 1}
};

#endif // CPU_H_INCLUDED
