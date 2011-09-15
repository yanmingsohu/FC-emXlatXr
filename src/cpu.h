/*----------------------------------------------------------------------------*|
|*                                                                            *|
|* FC 模拟器 (Famicom是Nintendo公司在1983年7月15日于日本发售的8位游戏机)      *|
|*                                                                            *|
|* $ C++语言的第一个项目,就用它练手吧                                         *|
|* $ 猫饭写作, 如引用本程序代码需注明出处                                     *|
|* $ 作者对使用本程序造成的后果不负任何责任                                   *|
|* $ 亦不会对代码的工作原理做进一步解释,如有重大问题请拨打119                 *|
|*                                                                            *|
|* > 使用 [Code::Block 10.05] 开发环境                                        *|
|* > 编译器使用 [MinGW 3.81] [gcc 4.4.1]                                      *|
|* > 参考了来自 [http://nesdev.parodius.com] 网站的资料                       *|
|* > 感谢 [Flubba,blargg] 设计的测试程序, 有了它开发效率成指数提升            *|
|*                                                                            *|
|* ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ | CatfoOD |^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ *|
|*                                           | yanming-sohu@sohu.com          *|
|* [ TXT CHARSET WINDOWS-936 / GBK ]         | https://github.com/yanmingsohu *|
|*                                           | qQ:412475540                   *|
|*----------------------------------------------------------------------------*/
#ifndef CPU_H_INCLUDED
#define CPU_H_INCLUDED

#include "type.h"
#include "mem.h"
#include "ppu.h"

#define CPU_NTSC      1789772.5  /* Hz */
#define CPU_PAL       1773447

struct command_6502;
struct command_parm;
struct cpu_6502;

/* cpu寻址方式 */
enum CPU_ADDRESSING_MODE {

    ADD_MODE_$zpgX$  = 0x00,
    ADD_MODE_zpg     = 0x04,
    ADD_MODE_imm     = 0x08,
    ADD_MODE_abs     = 0x0C,
    ADD_MODE_$zpg$Y  = 0x10,
    ADD_MODE_zpgX    = 0x14,
    ADD_MODE_zpgY    = 0x15,
    ADD_MODE_absY    = 0x18,
    ADD_MODE_absX    = 0x1C,
    ADD_MODE_acc     = 0x28, /* 使用A的值               */
    ADD_MODE_rel     = 0xEF, /* 相对寻址                */
    ADD_MODE_not     = 0xFF, /* 无寻址                  */

    ADD_MODE_inX     = 0x00, /* $zpgX$别名              */
    ADD_MODE_inY     = 0x10, /* $zpgX$别名              */
    ADD_MODE_zpX     = 0x14, /* zpgX别名                */
    ADD_MODE_zpY     = 0x15, /* zpgY别名                */
    ADD_MODE_abY     = 0x18, /* absX别名                */
    ADD_MODE_abX     = 0x1C, /* absX别名                */
};

/* 向命令处理函数传递参数 */
struct command_parm {

    command_6502 *cmd;
    memory       *ram;
    cpu_6502     *cpu;
    int          mem_time;   /* 寻址附加时间,不会自动复位  */

    byte op;                 /* 命令的代码                 */
    byte p1;                 /* 第一个参数(如果有)         */
    byte p2;                 /* 第二个参数(如果有)         */
    word addr;               /* 当前指令的地址             */

    /* 按照内存寻址方式返回该地址的上的数据            *
     * 不同寻址类型的相同指令编码品偏移量相同(也有例外)*
     * 于是,可以用cpu指令减去偏移得到该指令的寻址类型  */
    inline byte read(const byte addressing_mode);

    /* 按照内存寻址方式把value写入到该地址             */
    inline void write(const byte addressing_mode, byte value);

    /* 按照内存寻址方式读取该地址的数据                */
    inline word getAddr(const byte addressing_mode);

    /*---------------| 寻址算法定义, 函数返回地址 |----*/
    word  abs   ();
    word  absX  ();
    word  absY  ();
    word  zpg   ();
    word  zpgX  ();
    word  zpgY  ();
    word  $zpg$Y();
    word  $zpgX$();
    word  $ind$ (byte x);
};

struct cpu_6502 {

private:
    byte NMI_idle;               /* NMI空闲则为1                     */
    byte m_showDebug;            /* SHOW_CPU_MEMORY_ADDRESSING,
                                  * SHOW_CPU_OPERATE 启用后,仍需设置 */

public:
#define CPU_INTERRUPT_CYC   8    /* 中断命令的执行周期               */
#define CPU_RESET_CYC       6    /* 复位命令执行周期                 */
#define CPU_NMI_CYC        17    /* 不可屏蔽中断执行周期             */

    byte A; 	                 /* 累加器                           */
    byte Y;	    	             /* 索引暂存器                       */
    byte X;	    	             /* 索引暂存器                       */

    byte SP;		             /* 堆叠指示器 0x0100-0x01FF
                                  * 递减的, 指向'空', 每次1字节      */
    union {
        struct { byte PCL; byte PCH; };
        word PC;                 /* 程序计数器,指向下一个要执行的指令*/
    };

#define CPU_FLAGS_NEGATIVE    (1<<7)  /* 负值                        */
#define CPU_FLAGS_OVERFLOW    (1<<6)  /* 有溢出                      */
#define CPU_FLAGS_CONST       (1<<5)  /* 始终为1                     */
#define CPU_FLAGS_BREAK       (1<<4)  /* 有中断                      */
#define CPU_FLAGS_DECIMAL     (1<<3)  /* 十进制                      */
#define CPU_FLAGS_INTERDICT   (1<<2)  /* 禁止中断                    */
#define CPU_FLAGS_ZERO        (1<<1)  /* 结果为零                    */
#define CPU_FLAGS_CARRY        1      /* 有进位                      */

    byte FLAGS;	    /* 状态暂存器  N V 1 B D I Z C                   *
					 * N负值 	V溢出		B中断命令                *
					 * D十进制	I插断禁能	Z零值                    *
					 * C进位                                         */
    byte NMI;       /* 如果>0则执行不可屏蔽之中断                    */
    byte IRQ;       /* 如果>0则执行可屏蔽之中断                      */
    byte RES;       /* 如果>0则执行复位动作                          */

    memory       *ram;          /* 内存                              */
    command_parm  prev_parm;    /* 前一个命令的参数                  */

    cpu_6502(memory* ram);

    byte    reset();            /* 重置cpu状态,返回使用周期(soft)    */
    void    push(byte d);       /* 向堆栈中压数                      */
    byte    pop();              /* 从堆栈中取数                      */
    char*   debug();            /* 返回cpu状态字符串                 */
    byte    process();          /* 处理当前命令并指向下一条命
                                 * 令,返回使用的处理器周期           */
    byte    irq();              /* 处理器执行中断,返回使用的
                                 * 命令周期                          */
    void    rti();              /* 从中断过程中返回                  */
    byte    nmi();              /* 处理不可屏蔽的中断                */
    void    jump(word addr);    /* 跳转到addr内存指向的地址(addr, addr+1)
                                 * 并保存之前的PC和FLAGS             */
    char*   cmdInfo();          /* 返回上一条指令的描述              */


    void    checkNZ(byte value);/* 如果value最高位为1 则N=1,否则为0, *
                                 * 如果value==0 则Z=1,否则为0        */
    void    checkZ(byte value);
    void    setV(bool set);     /* set==true 则设置溢出否则清除      */
    void    clearV();

    void showDebug(byte show) { /* 是否在命令结束后显示相关信息      */
        m_showDebug = show;
    }

    byte isShowDebug() {
        return m_showDebug;
    }
};

struct command_6502 {

    char    name[4];
    byte    time;
    byte    len;

    CPU_ADDRESSING_MODE type;

    /* 指向处理函数的指针 */
    void (*op_func)(command_parm* parm);
};


#endif // CPU_H_INCLUDED
