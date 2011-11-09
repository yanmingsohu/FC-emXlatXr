/*----------------------------------------------------------------------------*|
|*                                                                            *|
|* FC 模拟器 (Famicom是Nintendo公司在1983年7月15日于日本发售的8位游戏机)      *|
|*                                                                            *|
|* $ C++语言的第一个项目,就用它练手吧                                         *|
|* $ 猫饭写作, 如引用本程序代码需注明出处                                     *|
|* $ 作者对使用本程序造成的后果不负任何责任                                   *|
|* $ 亦不会对代码的工作原理做进一步解释,如有重大问题请拨打119 & 911           *|
|*                                                                            *|
|* > 使用 [Code::Block 10.05] 开发环境                                        *|
|* > 编译器使用 [MinGW 3.81] [gcc 4.4.1]                                      *|
|* > 参考了来自 [http://nesdev.parodius.com] 网站的资料                       *|
|* > 感谢 [Flubba] 设计的测试程序, 有了它开发效率成指数提升                   *|
|*                                                                            *|
|* ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ | CatfoOD |^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ *|
|*                                           | yanming-sohu@sohu.com          *|
|* [ TXT CHARSET WINDOWS-936 / GBK ]         | https://github.com/yanmingsohu *|
|*                                           | qQ:412475540                   *|
|*----------------------------------------------------------------------------*/
#include "mem.h"
#include "cpu.h"
#include "stdio.h"
#include "stdlib.h"

#define NOT_MEM_TIME  parm->mem_time = 0 /* 不需要另加寻址时间 */
#define SET_FLAGS(x) (parm->cpu->FLAGS |= (x))
#define CLE_FLAGS(x) (parm->cpu->FLAGS &= 0xFF ^ (x))

HELP_FNC void check_decimal(command_parm* parm) {
    if (parm->cpu->FLAGS & CPU_FLAGS_DECIMAL) {
        printf("%s -- 不能执行十进运算\n", parm->cpu->cmdInfo());
        parm->cpu->debug();
    }
}

void cpu_command_XXX(command_parm* parm) {
    printf("! unknow command code[%02x] on %04x skip.\n",
           parm->op, parm->cpu->PC - parm->cmd->len);
}

void cpu_command_NOP(command_parm*) {
/* 不干正事 */
}

/* 跳跃到子过程 */
void cpu_command_JSR(command_parm* parm) {
    cpu_6502 *cpu = parm->cpu;

    cpu->PC--;
    cpu->push(cpu->PCH);
    cpu->push(cpu->PCL);

    cpu->PCH = parm->p2;
    cpu->PCL = parm->p1;
}

/* 从子过程返回 */
void cpu_command_RTS(command_parm* parm) {
    cpu_6502 *cpu = parm->cpu;

    cpu->PCL = cpu->pop();
    cpu->PCH = cpu->pop();
    cpu->PC++;
}

/* 中断 */
void cpu_command_BRK(command_parm* parm) {
    parm->cpu->PC++;
    SET_FLAGS(CPU_FLAGS_BREAK);
    parm->cpu->jump(0xFFFE);
}

/* 从中断过程中返回 */
void cpu_command_RTI(command_parm* parm) {
    parm->cpu->rti();
#ifdef NMI_DEBUG
    printf("CPU::中断返回\n");
#endif
}

/* value & x == true, 则设置'进位'位 */
#define CARRY_WITH(x) \
    (value & (x)) ? SET_FLAGS(CPU_FLAGS_CARRY) \
                  : CLE_FLAGS(CPU_FLAGS_CARRY);

/* 左移0补位 */
void cpu_command_ASL(command_parm* parm) {
    cpu_6502 *cpu = parm->cpu;
    byte model = parm->op - 0x02;

    if (parm->op == 0x0A) {
        model += 0x20;
    }

    byte value = parm->read(model);
    CARRY_WITH(0x80);

    value <<= 1;

    cpu->checkNZ(value);
    parm->write(model, value);
    NOT_MEM_TIME;
}

/* 右移0补位 */
void cpu_command_LSR(command_parm* parm) {
    cpu_6502 *cpu = parm->cpu;
    byte model = parm->op - 0x42;

    if (parm->op == 0x4A) {
        model += 0x20;
    }

    byte value = parm->read(model);
    CARRY_WITH(0x01);

    value >>= 1;

    CLE_FLAGS(CPU_FLAGS_NEGATIVE);
    cpu->checkZ(value);
    parm->write(model, value);
    NOT_MEM_TIME;
}

/* 左移循环补位 */
void cpu_command_ROL(command_parm* parm) {
    cpu_6502 *cpu = parm->cpu;
    byte model = parm->op - 0x22;

    if (parm->op == 0x2A) {
        model += 0x20;
    }

    byte value = parm->read(model);
    byte ro = cpu->FLAGS & CPU_FLAGS_CARRY ? 1 : 0;

    CARRY_WITH(0x80);

    value <<= 1;
    value |= ro;

    cpu->checkNZ(value);
    parm->write(model, value);
    NOT_MEM_TIME;
}

/* 右移循环补位 */
void cpu_command_ROR(command_parm* parm) {
    cpu_6502 *cpu = parm->cpu;
    byte model = parm->op - 0x62;

    if (parm->op == 0x6A) {
        model += 0x20;
    }

    byte value = parm->read(model);
    byte ro = cpu->FLAGS & CPU_FLAGS_CARRY ? 0x80 : 0;

    CARRY_WITH(0x01);

    value >>= 1;
    value |= ro;

    cpu->checkNZ(value);
    parm->write(model, value);
    NOT_MEM_TIME;
}

#undef CARRY_WITH

/* N=M7, V=M6, Z=(执行结果==0) */
void cpu_command_BIT(command_parm* parm) {
    cpu_6502 *cpu = parm->cpu;
    byte b = parm->read(parm->op - 0x20);

    cpu->FLAGS &= 0xFF ^ (CPU_FLAGS_NEGATIVE | CPU_FLAGS_OVERFLOW);
    cpu->FLAGS |= b    & (CPU_FLAGS_NEGATIVE | CPU_FLAGS_OVERFLOW);

    cpu->checkZ(cpu->A & b);
}

/* a<b, *n=1; z=0; c=0 *
 * a=b,  n=0; z=1; c=1 *
 * a>b, *n=0; z=0; c=1 */
HELP_FNC void cmp_op(command_parm* parm, byte a, byte b) {
    word c = a - b;

    if (c<0x100) {
        SET_FLAGS(CPU_FLAGS_CARRY);
    } else {
        CLE_FLAGS(CPU_FLAGS_CARRY);
    }

    parm->cpu->checkNZ(c);
}

void cpu_command_CMP(command_parm* parm) {
    byte b = parm->read(parm->op - 0xC1);
    cmp_op(parm, parm->cpu->A, b);
}

void cpu_command_CPX(command_parm* parm) {
    byte b;

    if (parm->op == 0xE0) {
        b = parm->p1;
    } else {
        b = parm->read(parm->op - 0xE0);
    }

    cmp_op(parm, parm->cpu->X, b);
}

void cpu_command_CPY(command_parm* parm) {
    byte b;

    if (parm->op == 0xC0) {
        b = parm->p1;
    } else {
        b = parm->read(parm->op - 0xC0);
    }

    cmp_op(parm, parm->cpu->Y, b);
}

void cpu_command_AND(command_parm* parm) {
    cpu_6502 *cpu = parm->cpu;
    cpu->A &= parm->read(parm->op - 0x21);
    cpu->checkNZ(cpu->A);
}

void cpu_command_ORA(command_parm* parm) {
    cpu_6502 *cpu = parm->cpu;
    cpu->A |= parm->read(parm->op - 0x01);
    cpu->checkNZ(cpu->A);
}

void cpu_command_EOR(command_parm* parm) {
    cpu_6502 *cpu = parm->cpu;
    cpu->A ^= parm->read(parm->op - 0x41);
    cpu->checkNZ(cpu->A);
}

/* 无条件跳转指令 */
void cpu_command_JMP(command_parm* parm) {
    cpu_6502* cpu = parm->cpu;

    if (parm->op==0x4C) {
        cpu->PCL = parm->p1;
        cpu->PCH = parm->p2;
    } else {
        word offH = parm->p2<<8; /* 模拟跨页bug */
        cpu->PCL = cpu->ram->read( offH | parm->p1           );
        cpu->PCH = cpu->ram->read( offH | byte(parm->p1 + 1) );
    }
}

#define JUMP_CODE  word base = parm->cpu->PC;                       \
                   word hi = base & 0xFF00;                         \
                   base += ((signed char)parm->p1);                 \
                   parm->mem_time = ((0xFF00 & base) ^ hi) ? 2 : 1; \
                   parm->cpu->PC = base

/* 条件跳转 C==0 */
void cpu_command_BCC(command_parm* parm) {
    if (!(parm->cpu->FLAGS & CPU_FLAGS_CARRY)) {
        JUMP_CODE;
    }
}

/* 条件跳转 C==1 */
void cpu_command_BCS(command_parm* parm) {
    if (parm->cpu->FLAGS & CPU_FLAGS_CARRY) {
        JUMP_CODE;
    }
}

/* 条件跳转 Z==1 */
void cpu_command_BEQ(command_parm* parm) {
    if (parm->cpu->FLAGS & CPU_FLAGS_ZERO) {
        JUMP_CODE;
    }
}

/* 条件跳转 Z==0*/
void cpu_command_BNE(command_parm* parm) {
    if (!(parm->cpu->FLAGS & CPU_FLAGS_ZERO)) {
        JUMP_CODE;
    }
}

/* 条件跳转 N==1 */
void cpu_command_BMI(command_parm* parm) {
    if (parm->cpu->FLAGS & CPU_FLAGS_NEGATIVE) {
        JUMP_CODE;
    }
}

/* 条件跳转 N==0 */
void cpu_command_BPL(command_parm* parm) {
    if (!(parm->cpu->FLAGS & CPU_FLAGS_NEGATIVE)) {
        JUMP_CODE;
    }
}

/* 条件跳转 V==1 */
void cpu_command_BVS(command_parm* parm) {
    if (parm->cpu->FLAGS & CPU_FLAGS_OVERFLOW) {
        JUMP_CODE;
    }
}

/* 条件跳转 V==0 */
void cpu_command_BVC(command_parm* parm) {
    if (!(parm->cpu->FLAGS & CPU_FLAGS_OVERFLOW)) {
        JUMP_CODE;
    }
}

#undef JUMP_CODE

/* 加法运算, 没有处理BCD加法 */
void cpu_command_ADC(command_parm* parm) {
    check_decimal(parm);

    cpu_6502* cpu = parm->cpu;
    cpu->clearV();

    byte b = parm->read(parm->op - 0x61);
    word temp = cpu->A + b;

    if (cpu->FLAGS & CPU_FLAGS_CARRY) {
        ++temp;
    }

    if (temp>0xFF) {
        SET_FLAGS(CPU_FLAGS_CARRY);
    } else {
        CLE_FLAGS(CPU_FLAGS_CARRY);
    }

    cpu->setV( !((cpu->A ^ b) & 0x80) && ((cpu->A ^ temp) & 0x80) );
    cpu->checkNZ((byte)temp);
    cpu->A = (byte)temp;
}

/* 减法运算 */
void cpu_command_SBC(command_parm* parm) {
    check_decimal(parm);

    cpu_6502* cpu = parm->cpu;
    cpu->clearV();

    byte b = parm->read(parm->op - 0xE1);
    word temp = cpu->A - b;

    if (!(cpu->FLAGS & CPU_FLAGS_CARRY)) {
        --temp;
    }

    if (temp<0x100) {
        SET_FLAGS(CPU_FLAGS_CARRY);
    } else {
        CLE_FLAGS(CPU_FLAGS_CARRY);
    }

    temp &= 0xFF;
    cpu->checkNZ((byte)temp);
    cpu->setV( ((cpu->A ^ temp ) & 0x80) && ((cpu->A ^ b) & 0x80) );
    cpu->A = (byte)temp;
}

/* --X */
void cpu_command_DEX(command_parm* parm) {
    cpu_6502 *cpu = parm->cpu;
    cpu->X--;
    cpu->checkNZ(cpu->X);
}

/* ++X */
void cpu_command_INX(command_parm* parm) {
    cpu_6502 *cpu = parm->cpu;
    cpu->X++;
    cpu->checkNZ(cpu->X);
}

/* --Y */
void cpu_command_DEY(command_parm* parm) {
    cpu_6502 *cpu = parm->cpu;
    cpu->Y--;
    cpu->checkNZ(cpu->Y);
}

/* ++Y */
void cpu_command_INY(command_parm* parm) {
    cpu_6502 *cpu = parm->cpu;
    cpu->Y++;
    cpu->checkNZ(cpu->Y);
}

/* --内存 */
void cpu_command_DEC(command_parm* parm) {
    memory *ram = parm->ram;

    word addr = parm->getAddr(parm->op - 0xC2);
    byte value = ram->read(addr) - 1;
    ram->write(addr, value);
    parm->cpu->checkNZ(value);

    NOT_MEM_TIME;
}

/* ++内存 */
void cpu_command_INC(command_parm* parm) {
    memory *ram = parm->ram;

    word addr = parm->getAddr(parm->op - 0xE2);
    byte value = ram->read(addr) + 1;
    ram->write(addr, value);
    parm->cpu->checkNZ(value);

    NOT_MEM_TIME;
}

/* 把A寄存器存入内存 */
void cpu_command_STA(command_parm* parm) {
    parm->write(parm->op - 0x81, parm->cpu->A);
    NOT_MEM_TIME;
}

/* 把Y寄存器存入内存 */
void cpu_command_STY(command_parm* parm) {
    parm->write(parm->op - 0x80, parm->cpu->Y);
}

/* 把X寄存器存入内存 */
void cpu_command_STX(command_parm* parm) {
    byte mode;
    if (parm->op == 0x96) {
        mode = ADD_MODE_zpgY;
    } else {
        mode = parm->op - 0x82;
    }

    parm->write(mode, parm->cpu->X);
}

/* 由内存载入A寄存器 */
void cpu_command_LDA(command_parm* parm) {
    cpu_6502* cpu = parm->cpu;
    cpu->A = parm->read(parm->op - 0xA1);
    cpu->checkNZ(cpu->A);
}

/* 由内存载入Y寄存器 */
void cpu_command_LDY(command_parm* parm) {
    cpu_6502* cpu = parm->cpu;

    if (parm->op == 0xA0) {
        cpu->Y = parm->p1;
    } else {
        cpu->Y = parm->read(parm->op - 0xA0);
    }

    cpu->checkNZ(cpu->Y);
}

/* 由内存载入X寄存器 */
void cpu_command_LDX(command_parm* parm) {
    cpu_6502* cpu = parm->cpu;

    if (parm->op == 0xA2) {
        cpu->X = parm->p1;
    } else {
        byte mode;

        if (parm->op == 0xB6) {
            mode = ADD_MODE_zpgY;
        } else {
            mode = parm->op - 0xA2;
        }

        cpu->X = parm->read(mode);
    }

    cpu->checkNZ(cpu->X);
}

/* 累加器入栈 */
void cpu_command_PHA(command_parm* parm) {
    cpu_6502* cpu = parm->cpu;
    cpu->push(cpu->A);
}

/* 出栈入累加器 */
void cpu_command_PLA(command_parm* parm) {
    cpu_6502* cpu = parm->cpu;
    cpu->A = cpu->pop();
    cpu->checkNZ(cpu->A);
}

/* FLAGS入栈 */
void cpu_command_PHP(command_parm* parm) {
    cpu_6502* cpu = parm->cpu;
    cpu->push(cpu->FLAGS);
}

/* 出栈入FLAGS */
void cpu_command_PLP(command_parm* parm) {
    cpu_6502* cpu = parm->cpu;
    cpu->FLAGS = cpu->pop() | 0x30;
}

/* a = b */
#define _TR(b,a) cpu_6502 *cpu = parm->cpu; \
                 cpu->a = cpu->b

void cpu_command_TAY(command_parm* parm) {
    _TR(A, Y);
    cpu->checkNZ(cpu->A);
}

void cpu_command_TAX(command_parm* parm) {
    _TR(A, X);
    cpu->checkNZ(cpu->A);
}

void cpu_command_TXA(command_parm* parm) {
    _TR(X, A);
    cpu->checkNZ(cpu->X);
}

void cpu_command_TYA(command_parm* parm) {
    _TR(Y, A);
    cpu->checkNZ(cpu->Y);
}

void cpu_command_TSX(command_parm* parm) {
    _TR(SP, X);
    cpu->checkNZ(cpu->SP);
}

void cpu_command_TXS(command_parm* parm) {
    _TR(X, SP);
}

#undef _TR

/* 设置为禁止中断 */
void cpu_command_SEI(command_parm* parm) {
    SET_FLAGS(CPU_FLAGS_INTERDICT);
}

/* 清除禁止中断位 */
void cpu_command_CLI(command_parm* parm) {
    CLE_FLAGS(CPU_FLAGS_INTERDICT);
}

/* 设置为十进制(BCD)算术运算 */
void cpu_command_SED(command_parm* parm) {
    SET_FLAGS(CPU_FLAGS_DECIMAL);
}

/* 还原为二进制算术运算 */
void cpu_command_CLD(command_parm* parm) {
    CLE_FLAGS(CPU_FLAGS_DECIMAL);
}

/* 设置进位标志 */
void cpu_command_SEC(command_parm* parm) {
    SET_FLAGS(CPU_FLAGS_CARRY);
}

/* 清除进位标志 */
void cpu_command_CLC(command_parm* parm) {
    CLE_FLAGS(CPU_FLAGS_CARRY);
}

/* 清除溢出位 */
void cpu_command_CLV(command_parm* parm) {
    parm->cpu->clearV();
}

#undef SET_FLAGS
#undef CLE_FLAGS
#undef NOT_MEM_TIME

/****************************************************************************/

#define Op(n, t, l, p)   {#n, t, l, ADD_MODE_##p, cpu_command_##n}
#define  L               Op(XXX, 0, 1, not)
#define  N               L,
#define  O(n, t, l, p)   Op(n, t, l, p),

/* 索引既是6502命令的首字节 */
command_6502 command_list_6502[] = {
/* --------------------------------------| 0x00 - 0x0F |--- */
    O(BRK, 7, 1, not)   O(ORA, 6, 2, inX)   N
    N                   N                   O(ORA, 3, 2, zpg)
    O(ASL, 5, 2, zpg)   N                   O(PHP, 3, 1, not)
    O(ORA, 2, 2, imm)   O(ASL, 2, 1, not)   N
    N                   O(ORA, 4, 3, abs)   O(ASL, 6, 3, abs)
    N
/* --------------------------------------| 0x10 - 0x1F |--- */
    O(BPL, 2, 2, rel)   O(ORA, 5, 2, inY)   N
    N                   N                   O(ORA, 4, 2, zpX)
    O(ASL, 6, 2, zpX)   N                   O(CLC, 2, 1, not)
    O(ORA, 4, 3, abY)   N                   N
    N                   O(ORA, 4, 3, abX)   O(ASL, 7, 3, abX)
    N
/* --------------------------------------| 0x20 - 0x2F |--- */
    O(JSR, 6 ,3, rel)   O(AND, 6, 2, inY)   N
    N                   O(BIT, 3, 2, zpg)   O(AND, 3, 2, zpg)
    O(ROL, 5, 2, zpg)   N                   O(PLP, 4, 1, not)
    O(AND, 2, 2, imm)   O(ROL, 2, 1, not)   N
    O(BIT, 4, 3, abs)   O(AND, 4, 3, abs)   O(ROL, 6, 3, abs)
    N
/* --------------------------------------| 0x30 - 0x3F |--- */
    O(BMI, 2, 2, rel)   O(AND, 5, 2, inY)   N
    N                   N                   O(AND, 4, 2, zpX)
    O(ROL, 6, 2, zpX)   N                   O(SEC, 2, 1, not)
    O(AND, 4, 3, abY)   N                   N
    N                   O(AND, 4, 3, abX)   O(ROL, 6, 3, abX)
    N
/* --------------------------------------| 0x40 - 0x4F |--- */
    O(RTI, 6, 1, not)   O(EOR, 6, 2, inX)   N
    N                   N                   O(EOR, 3, 2, zpg)
    O(LSR, 5, 2, zpg)   N                   O(PHA, 3, 1, not)
    O(EOR, 2, 2, imm)   O(LSR, 2, 1, not)   N
    O(JMP, 3, 3, abs)   O(EOR, 4, 3, abs)   O(LSR, 6, 3, abs)
    N
/* --------------------------------------| 0x50 - 0x5F |--- */
    O(BVC, 2, 2, rel)   O(EOR, 5, 2, inY)   N
    N                   N                   O(EOR, 4, 2, zpX)
    O(LSR, 6, 2, zpX)   N                   O(CLI, 2, 1, not)
    O(EOR, 4, 3, abY)   N                   N
    N                   O(EOR, 4, 3, abX)   O(LSR, 6, 3, abX)
    N
/* --------------------------------------| 0x60 - 0x6F |--- */
    O(RTS, 6, 1, not)   O(ADC, 6, 2, inX)   N
    N                   N                   O(ADC, 3, 2, zpg)
    O(ROR, 5, 2, zpg)   N                   O(PLA, 4, 1, not)
    O(ADC, 2, 2, imm)   O(ROR, 2, 1, not)   N
    O(JMP, 6, 3, not)   O(ADC, 4, 3, abs)   O(ROR, 6, 3, abs)
    N
/* --------------------------------------| 0x70 - 0x7F |--- */
    O(BVS, 2, 2, rel)   O(ADC, 5, 2, inY)   N
    N                   N                   O(ADC, 4, 2, zpX)
    O(ROR, 6, 2, zpX)   N                   O(SEI, 2, 1, not)
    O(ADC, 4, 3, abY)   N                   N
    N                   O(ADC, 4, 3, abX)   O(ROR, 6, 3, abX)
    N
/* --------------------------------------| 0x80 - 0x8F |--- */
    N                   O(STA, 6, 2, inX)   N
    N                   O(STY, 3, 2, zpg)   O(STA, 3, 2, zpg)
    O(STX, 3, 2, zpg)   N                   O(DEY, 2, 1, not)
    N                   O(TXA, 2, 1, not)   N
    O(STY, 4, 3, abs)   O(STA, 4, 3, abs)   O(STX, 4, 3, abs)
    N
/* --------------------------------------| 0x90 - 0x9F |--- */
    O(BCC, 2, 2, rel)   O(STA, 6, 2, inY)   N
    N                   O(STY, 4, 2, zpX)   O(STA, 4, 2, zpX)
    O(STX, 4, 2, zpY)   N                   O(TYA, 2, 1, not)
    O(STA, 5, 3, abY)   O(TXS, 2, 1, not)   N
    N                   O(STA, 4, 3, abX)   N
    N
/* --------------------------------------| 0xA0 - 0xAF |--- */
    O(LDY, 2, 2, imm)   O(LDA, 6, 2, inX)   O(LDX, 2, 2, imm)
    N                   O(LDY, 3, 2, zpg)   O(LDA, 3, 2, zpg)
    O(LDX, 3, 2, zpg)   N                   O(TAY, 2, 1, not)
    O(LDA, 2, 2, imm)   O(TAX, 2, 1, not)   N
    O(LDY, 4, 3, abs)   O(LDA, 4, 3, abs)   O(LDX, 4, 3, abs)
    N
/* --------------------------------------| 0xB0 - 0xBF |--- */
    O(BCS, 2, 2, rel)   O(LDA, 5, 2, inY)   N
    N                   O(LDY, 4, 2, zpX)   O(LDA, 4, 2, zpX)
    O(LDX, 4, 2, zpY)   N                   O(CLV, 2, 1, not)
    O(LDA, 4, 3, abY)   O(TSX, 2, 1, not)   N
    O(LDY, 4, 3, abX)   O(LDA, 4, 3, abX)   O(LDX, 4, 3, abY)
    N
/* --------------------------------------| 0xC0 - 0xCF |--- */
    O(CPY, 2, 2, imm)   O(CMP, 6, 2, inX)   N
    N                   O(CPY, 3, 2, zpg)   O(CMP, 3, 2, zpg)
    O(DEC, 5, 2, zpg)   N                   O(INY, 2, 1, not)
    O(CMP, 2, 2, imm)   O(DEX, 2, 1, not)   N
    O(CPY, 4, 3, abs)   O(CMP, 4, 3, abs)   O(DEC, 6, 3, abs)
    N
/* --------------------------------------| 0xD0 - 0xDF |--- */
    O(BNE, 2, 2, rel)   O(CMP, 5, 2, inY)   N
    N                   N                   O(CMP, 4, 2, zpX)
    O(DEC, 6, 2, zpX)   N                   O(CLD, 2, 1, not)
    O(CMP, 4, 3, abY)   N                   N
    N                   O(CMP, 4, 3, abX)   O(DEC, 6, 3, abX)
    N
/* --------------------------------------| 0xE0 - 0xEF |--- */
    O(CPX, 2, 2, imm)   O(SBC, 6, 2, inX)   N
    N                   O(CPX, 3, 2, zpg)   O(SBC, 3, 2, zpg)
    O(INC, 5, 2, zpg)   N                   O(INX, 2, 1, not)
    O(SBC, 2, 2, imm)   O(NOP, 2, 1, not)   N
    O(CPX, 4, 3, abs)   O(SBC, 4, 3, abs)   O(INC, 6, 3, abs)
    N
/* --------------------------------------| 0xF0 - 0xFF |--- */
    O(BEQ, 2, 2, rel)   O(SBC, 5, 2, inY)   N
    N                   N                   O(SBC, 4, 2, zpX)
    O(INC, 6, 2, zpX)   N                   O(SED, 2, 1, not)
    O(SBC, 4, 3, abY)   N                   N
    N                   O(SBC, 4, 3, abX)   O(INC, 6, 3, abX)
    N
/* --------------------------------------| over        |--- */
    L
};

#undef N
#undef O
#undef L
#undef Op

/****************************************************************************/

byte cpu_6502::process() {
    byte cpu_cyc = reset();

    prev_parm.op = ram->read(PC);
    command_6502 *opp = &command_list_6502[prev_parm.op];

    switch (opp->len) {
    case 3:
        prev_parm.p2 = ram->read(PC+2);
    case 2:
        prev_parm.p1 = ram->read(PC+1);
    }

    prev_parm.cmd = opp;
    prev_parm.addr = PC;
    PC += opp->len;

#ifdef SHOW_CPU_OPERATE
    if (m_showDebug) printf(cmdInfo());
#endif

    prev_parm.mem_time = 0;
    opp->op_func(&prev_parm);

    cpu_cyc += opp->time;
    cpu_cyc += prev_parm.mem_time;
    cpu_cyc += nmi();
    cpu_cyc += irq();
    return cpu_cyc;
}

cpu_6502::cpu_6502(memory* ram)
        : NMI_idle(1), m_showDebug(0)
        , NMI(0), IRQ(0), RES(1), ram(ram)
{
    prev_parm.cpu = this;
    prev_parm.ram = ram;
    prev_parm.mem_time = 0;
}

char* cpu_6502::debug() {
    static char abuf[256];
    char cbuf[9];
    itoa(FLAGS, cbuf, 2);

    sprintf(abuf,
            "CPU >   A: %04X   X: %04X   Y: %04X  (NV1BDIZC)\n"
            "    >  PC: %04X  SP: %04X       FG:   %08s\n",
            A, X, Y, PC, 0x0100 | SP, cbuf);

    return abuf;
}

/* 返回的字符串长23字符 */
char* cpu_6502::cmdInfo() {

    static char buf[64];
    const char* cc = NULL;
    const char* tp = NULL;
    command_6502 *cmd = &command_list_6502[prev_parm.op];

    switch (cmd->len) {
    case 3:
        cc = "%04X.  >%s  [%02X] %s %02X %02X \n"; break;
    case 2:
        cc = "%04X.  >%s  [%02X] %s %02X -- \n";   break;
    default:
        cc = "%04X.  >%s  [%02X] %s -- -- \n";
    }

    switch (cmd->type) {

    case ADD_MODE_abs:
        tp = "abs "; break;
    case ADD_MODE_absY:
        tp = "absY"; break;
    case ADD_MODE_absX:
        tp = "absX"; break;

    case ADD_MODE_$zpg$Y:
        tp = "indY"; break;
    case ADD_MODE_$zpgX$:
        tp = "indX"; break;

    case ADD_MODE_imm:
        tp = "imm "; break;

    case ADD_MODE_rel:
        tp = "rel "; break;

    case ADD_MODE_zpgX:
        tp = "zpgX"; break;
    case ADD_MODE_zpgY:
        tp = "zpgY"; break;
    case ADD_MODE_zpg:
        tp = "zpg "; break;

    case ADD_MODE_acc:
    case ADD_MODE_not:
        tp = "----"; break;
    default:
        tp = "!!!?"; break;
    }

    sprintf(buf, cc, prev_parm.addr, tp,
                 prev_parm.op, cmd->name, prev_parm.p1, prev_parm.p2);

    return buf;
}

byte cpu_6502::reset() {
    if (RES) {
        IRQ    = 0;
        NMI    = 0;
        RES    = 0;
        FLAGS  = CPU_FLAGS_CONST | CPU_FLAGS_INTERDICT;

        ram->soft_reset();
        PCH    = ram->read(0xFFFD);
        PCL    = ram->read(0xFFFC);

        NMI_idle = 1;
        return CPU_RESET_CYC;
    }
    return 0;
}

inline byte cpu_6502::irq() {
    if (NMI_idle && IRQ && (~FLAGS & CPU_FLAGS_INTERDICT)) {
        IRQ = 0;
        jump(0xFFFE);
        return CPU_INTERRUPT_CYC;
    }
    return 0;
}

inline byte cpu_6502::nmi() {
    if (NMI && NMI_idle) {
        //NMI = 0;
        NMI_idle = 0;
        jump(0xFFFA);
#ifdef NMI_DEBUG
        printf("CPU::NMI中断\n");
        printf(debug());
#endif
        return CPU_NMI_CYC;
    }
    return 0;
}

inline void cpu_6502::rti() {
    FLAGS = pop();
    PCL   = pop();
    PCH   = pop();
    NMI   = 0;
    NMI_idle = 1;
}

inline void cpu_6502::jump(word addr) {
    push(PCH);
    push(PCL);
    push(FLAGS);
    PCH    = ram->read(addr+1);
    PCL    = ram->read(addr  );
    FLAGS |= CPU_FLAGS_INTERDICT;
}

inline void cpu_6502::push(byte d) {
    ram->write(SP | 0x0100, d);
    SP--;
}

inline byte cpu_6502::pop() {
    SP++;
    return ram->read(SP | 0x0100);
}

inline void cpu_6502::checkNZ(byte value) {
    if (value & 0x80) {
        FLAGS |= CPU_FLAGS_NEGATIVE;
    } else {
        FLAGS &= 0xFF ^ CPU_FLAGS_NEGATIVE;
    }

    checkZ(value);
}

inline void cpu_6502::checkZ(byte value) {
    if (value) {
        FLAGS &= 0xFF ^ CPU_FLAGS_ZERO;
    } else {
        FLAGS |= CPU_FLAGS_ZERO;
    }
}

inline void cpu_6502::setV(bool set) {
    if ( set ) {
        FLAGS |= CPU_FLAGS_OVERFLOW;
    } else {
        FLAGS &= 0xFF ^ CPU_FLAGS_OVERFLOW;
    }
}

inline void cpu_6502::clearV() {
    FLAGS &= 0xFF ^ CPU_FLAGS_OVERFLOW;
}

/****************************************************************************/

/* a+b 超过页界限则寻址时间+1, 返回 a+b */
static inline word CHECK_PAGE_BOUND(word a, word b, int *mem_time) {
    word base = a;
    word hi = base & 0xFF00;
    base += b;
    if ( (0xFF00 & base) ^ hi ) *mem_time = 1;
    return base;
}

inline word command_parm::abs() {
    return (p2<<8) | p1;
}

inline word command_parm::absX() {
    return CHECK_PAGE_BOUND(abs(), cpu->X, &mem_time);
}

inline word command_parm::absY() {
    return CHECK_PAGE_BOUND(abs(), cpu->Y, &mem_time);
}

inline word command_parm::zpg() {
    return p1 & 0x00FF;
}

inline word command_parm::zpgX() {
    return (p1 + cpu->X) & 0x00FF;
}

inline word command_parm::zpgY() {
    return (p1 + cpu->Y) & 0x00FF;
}

inline word command_parm::$zpg$Y() {
    return CHECK_PAGE_BOUND($ind$(0), cpu->Y, &mem_time);
}

inline word command_parm::$zpgX$() {
    return $ind$(cpu->X);
}

inline word command_parm::$ind$(byte x) {
    word offset = (p1 + x) & 0x00FF;
    byte l = ram->read( offset   );
    byte h = ram->read( offset+1 );
    return (h<<8 | l);
}

inline byte command_parm::read(const byte addressing_mode) {
    if (addressing_mode==ADD_MODE_imm) {
        return p1;
    } else
    if (addressing_mode==ADD_MODE_acc) {
        return cpu->A;
    }
#ifdef SHOW_CPU_MEMORY_ADDRESSING
    word addr = getAddr(addressing_mode);
    byte value = ram->read( addr );
    cpu->isShowDebug()
        && printf("CPU::read address:%04X value:%X\n", addr, value);
    return value;
#endif
#ifndef SHOW_CPU_MEMORY_ADDRESSING
    return ram->read( getAddr(addressing_mode) );
#endif
}

inline void command_parm::write(const byte addressing_mode, byte value) {
    if (addressing_mode==ADD_MODE_imm) {
        printf("!不能向立即数中写数据");
        return;
    } else
    if (addressing_mode==ADD_MODE_acc) {
        cpu->A = value;
        return;
    }
    word addr = getAddr(addressing_mode);
#if defined SHOW_CPU_MEMORY_ADDRESSING
    cpu->isShowDebug()
        && printf("CPU::write address:%04X value:%X\n", addr, value);
#endif
    cpu->ram->write(addr, value);
}

inline word command_parm::getAddr(const byte addressing_mode) {
    switch (addressing_mode) {

    case ADD_MODE_abs:
        return abs();

    case ADD_MODE_zpg:
        return zpg();

    case ADD_MODE_absX:
        return absX();

    case ADD_MODE_absY:
        return absY();

    case ADD_MODE_zpgX:
        return zpgX();

    case ADD_MODE_zpgY:
        return zpgY();

    case ADD_MODE_$zpg$Y:
        return $zpg$Y();

    case ADD_MODE_$zpgX$:
        return $zpgX$();
    }

    printf("!无效的寻址代码: %d\n", addressing_mode);
    return 0;
}
