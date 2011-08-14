#include "mem.h"
#include "cpu.h"
#include "stdio.h"
#include "stdlib.h"

#define SET_FLAGS(x) (parm->cpu->FLAGS |= (x))
#define CLE_FLAGS(x) (parm->cpu->FLAGS &= 0xFF ^ (x))

HELP_FNC void check_decimal(command_6502* cmd, command_parm* parm) {
    if (parm->cpu->FLAGS & CPU_FLAGS_DECIMAL) {
        printf("%s -- 不能执行十进运算\n", parm->cpu->cmdInfo());
        parm->cpu->debug();
    }
}

void cpu_command_XXX(command_6502* cmd, command_parm* parm) {
    printf("! unknow command code[%02x] on %04x exit.\n",
           parm->op, parm->cpu->PC - cmd->len);
}

void cpu_command_NOP(command_6502* cmd, command_parm* parm) {
    /* 不干正事 */
}

/* 跳跃到子过程 */
void cpu_command_JSR(command_6502* cmd, command_parm* parm) {
    cpu_6502 *cpu = parm->cpu;

    cpu->push(cpu->PCH);
    cpu->push(cpu->PCL);

    cpu->PCH = parm->p2;
    cpu->PCL = parm->p1;
}

/* 从子过程返回 */
void cpu_command_RTS(command_6502* cmd, command_parm* parm) {
    cpu_6502 *cpu = parm->cpu;

    cpu->PCL = cpu->pop();
    cpu->PCH = cpu->pop();
}

/* 中断 */
void cpu_command_BRK(command_6502* cmd, command_parm* parm) {
    parm->cpu->jump(0xFFFE);
    SET_FLAGS(CPU_FLAGS_BREAK);
}

/* 从中断过程中返回 */
void cpu_command_RTI(command_6502* cmd, command_parm* parm) {
    parm->cpu->rti();
}

/* value & x == true, 则设置'进位'位 */
#define CARRY_WITH(x) \
    (value & (x)) ? SET_FLAGS(CPU_FLAGS_CARRY) \
                  : CLE_FLAGS(CPU_FLAGS_CARRY);

/* 左移0补位 */
void cpu_command_ASL(command_6502* cmd, command_parm* parm) {
    cpu_6502 *cpu = parm->cpu;
    byte value = 0;
    byte model = parm->op - 0x02;

    switch (parm->op) {
    case 0x0A:
        model += 0x20;
    case 0x0E:
    case 0x06:
    case 0x1E:
    case 0x16:
        value = parm->read(model);
    }

    CARRY_WITH(0x80);

    value <<= 1;

    cpu->checkNZ(value);
    parm->write(model, value);
}

/* 右移0补位 */
void cpu_command_LSR(command_6502* cmd, command_parm* parm) {
    cpu_6502 *cpu = parm->cpu;
    byte value = 0;
    byte model = parm->op - 0x42;

    switch (parm->op) {
    case 0x4A:
        model += 0x20;
    case 0x4E:
    case 0x46:
    case 0x5E:
    case 0x56:
        value = parm->read(model);
    }

    CARRY_WITH(0x01);

    value >>= 1;

    CLE_FLAGS(CPU_FLAGS_NEGATIVE);
    cpu->checkZ(value);
    parm->write(model, value);
}

/* 左移循环补位 */
void cpu_command_ROL(command_6502* cmd, command_parm* parm) {
    cpu_6502 *cpu = parm->cpu;
    byte value = 0;
    byte model = parm->op - 0x22;

    switch (parm->op) {
    case 0x2A:
        model += 0x20;
    case 0x2E:
    case 0x26:
    case 0x3E:
    case 0x36:
        value = parm->read(model);
    }

    byte ro = cpu->FLAGS & CPU_FLAGS_CARRY ? 1 : 0;

    CARRY_WITH(0x80);

    value <<= 1;
    value |= ro;

    cpu->checkNZ(value);
    parm->write(model, value);
}

/* 右移循环补位 */
void cpu_command_ROR(command_6502* cmd, command_parm* parm) {
    cpu_6502 *cpu = parm->cpu;
    byte value = 0;
    byte model = parm->op - 0x62;

    switch (parm->op) {
    case 0x6A:
        model += 0x20;
    case 0x6E:
    case 0x66:
    case 0x7E:
    case 0x76:
        value = parm->read(model);
    }

    byte ro = cpu->FLAGS & CPU_FLAGS_CARRY ? 0x80 : 0;

    CARRY_WITH(0x01);

    value >>= 1;
    value |= ro;

    cpu->checkNZ(value);
    parm->write(model, value);
}

#undef CARRY_WITH

/* N=M7, V=M6, Z=(执行结果==0) */
void cpu_command_BIT(command_6502* cmd, command_parm* parm) {
    cpu_6502 *cpu = parm->cpu;
    byte a = cpu->A;
    byte b = 0;

    switch (parm->op) {
    case 0x2C:
    case 0x24:
        b = parm->read(parm->op - 0x20);
    }

    cpu->FLAGS &= 0xFF ^ (CPU_FLAGS_NEGATIVE & CPU_FLAGS_OVERFLOW);
    cpu->FLAGS |= b    & (CPU_FLAGS_NEGATIVE & CPU_FLAGS_OVERFLOW);

    a &= b;

    cpu->checkZ(a);
}

/* a<b, n=1; z=0; c=0 *
 * a=b, n=0; z=1; c=1 *
 * a>b, n=0; z=0; c=1 */
HELP_FNC void cmp_op(command_parm* parm, byte a, byte b) {
    if (a>b) {
        CLE_FLAGS(CPU_FLAGS_NEGATIVE & CPU_FLAGS_ZERO);
        SET_FLAGS(CPU_FLAGS_CARRY);
    }
    else if (a<b) {
        CLE_FLAGS(CPU_FLAGS_CARRY & CPU_FLAGS_ZERO);
        SET_FLAGS(CPU_FLAGS_NEGATIVE);
    }
    else {
        CLE_FLAGS(CPU_FLAGS_NEGATIVE);
        SET_FLAGS(CPU_FLAGS_CARRY & CPU_FLAGS_ZERO);
    }
}

void cpu_command_CMP(command_6502* cmd, command_parm* parm) {
    switch (parm->op) {
    case 0xC9:
    case 0xCD:
    case 0xC5:
    case 0xDD:
    case 0xD9:
    case 0xD5:
    case 0xD1:
    case 0xC1:
        byte a = parm->cpu->A;
        byte b = parm->read(parm->op - 0xC1);
        cmp_op(parm, a, b);
    }
}

void cpu_command_CPX(command_6502* cmd, command_parm* parm) {
    byte a = parm->cpu->X;
    byte b = 0;

    switch (parm->op) {
    case 0xE0:
        b = parm->p1;
        break;
    case 0xEC:
    case 0xE4:
        b = parm->read(parm->op - 0xE0);
    }

    cmp_op(parm, a, b);
}

void cpu_command_CPY(command_6502* cmd, command_parm* parm) {
    byte a = parm->cpu->Y;
    byte b = 0;

    switch (parm->op) {
    case 0xC0:
        b = parm->p1;
        break;
    case 0xCC:
    case 0xC4:
        b = parm->read(parm->op - 0xC0);
    }

    cmp_op(parm, a, b);
}

void cpu_command_AND(command_6502* cmd, command_parm* parm) {
    switch (parm->op) {
    case 0x29:
    case 0x2D:
    case 0x25:
    case 0x3D:
    case 0x39:
    case 0x35:
    case 0x31:
    case 0x21:
        cpu_6502 *cpu = parm->cpu;
        cpu->A &= parm->read(parm->op - 0x21);
        cpu->checkNZ(cpu->A);
    }
}

void cpu_command_ORA(command_6502* cmd, command_parm* parm) {
    switch (parm->op) {
    case 0x09:
    case 0x0D:
    case 0x05:
    case 0x1D:
    case 0x19:
    case 0x15:
    case 0x11:
    case 0x01:
        cpu_6502 *cpu = parm->cpu;
        cpu->A &= parm->read(parm->op - 0x01);
        cpu->checkNZ(cpu->A);
    }
}

void cpu_command_EOR(command_6502* cmd, command_parm* parm) {
    switch (parm->op) {
    case 0x49:
    case 0x4D:
    case 0x45:
    case 0x5D:
    case 0x59:
    case 0x55:
    case 0x51:
    case 0x41:
        cpu_6502 *cpu = parm->cpu;
        cpu->A ^= parm->read(parm->op - 0x41);
        cpu->checkNZ(cpu->A);
    }
}

/* 无条件跳转指令 */
void cpu_command_JMP(command_6502* cmd, command_parm* parm) {

    cpu_6502* cpu = parm->cpu;

    if (parm->op==0x4C) {
        cpu->PCL = parm->p1;
        cpu->PCH = parm->p2;
    } else {
        word offset = (parm->p2<<8) | parm->p1;
        cpu->PCL = cpu->ram->read( offset   );
        cpu->PCH = cpu->ram->read( offset+1 );
    }
}

#define JUMP_CODE  parm->cpu->PC += ((signed char)parm->p1)

/* 条件跳转 C==0 */
void cpu_command_BCC(command_6502* cmd, command_parm* parm) {
    if (!(parm->cpu->FLAGS & CPU_FLAGS_CARRY)) {
        JUMP_CODE;
    }
}

/* 条件跳转 C==1 */
void cpu_command_BCS(command_6502* cmd, command_parm* parm) {
    if (parm->cpu->FLAGS & CPU_FLAGS_CARRY) {
        JUMP_CODE;
    }
}

/* 条件跳转 Z==1 */
void cpu_command_BEQ(command_6502* cmd, command_parm* parm) {
    if (parm->cpu->FLAGS & CPU_FLAGS_ZERO) {
        JUMP_CODE;
    }
}

/* 条件跳转 Z==0*/
void cpu_command_BNE(command_6502* cmd, command_parm* parm) {
    if (!(parm->cpu->FLAGS & CPU_FLAGS_ZERO)) {
        JUMP_CODE;
    }
}

/* 条件跳转 N==1 */
void cpu_command_BMI(command_6502* cmd, command_parm* parm) {
    if (parm->cpu->FLAGS & CPU_FLAGS_NEGATIVE) {
        JUMP_CODE;
    }
}

/* 条件跳转 N==0 */
void cpu_command_BPL(command_6502* cmd, command_parm* parm) {
    if (!(parm->cpu->FLAGS & CPU_FLAGS_NEGATIVE)) {
        JUMP_CODE;
    }
}

/* 条件跳转 V==1 */
void cpu_command_BVS(command_6502* cmd, command_parm* parm) {
    if (parm->cpu->FLAGS & CPU_FLAGS_OVERFLOW) {
        JUMP_CODE;
    }
}

/* 条件跳转 V==0 */
void cpu_command_BVC(command_6502* cmd, command_parm* parm) {
    if (!(parm->cpu->FLAGS & CPU_FLAGS_OVERFLOW)) {
        JUMP_CODE;
    }
}

#undef JUMP_CODE

/* 加法运算, 没有处理BCD加法 */
void cpu_command_ADC(command_6502* cmd, command_parm* parm) {
    check_decimal(cmd, parm);

    cpu_6502* cpu = parm->cpu;
    cpu->clearV();
    word result = 0;

    switch (parm->op) {
    case 0x69:
    case 0x6D:
    case 0x65:
    case 0x7D:
    case 0x79:
    case 0x75:
    case 0x71:
    case 0x61:
        result = parm->read(parm->op - 0x61);
        break;
    }

    result += cpu->A;
    result += (cpu->FLAGS & CPU_FLAGS_CARRY) ? 1 : 0;

    if (result>0xFF) {
        SET_FLAGS(CPU_FLAGS_CARRY);
    } else {
        CLE_FLAGS(CPU_FLAGS_CARRY);
    }

    cpu->checkV(result, cpu->A);
    cpu->checkNZ((byte)result);
    cpu->A = (byte) result;
}

/* 减法运算 */
void cpu_command_SBC(command_6502* cmd, command_parm* parm) {
    check_decimal(cmd, parm);

    cpu_6502* cpu = parm->cpu;
    cpu->clearV();
    word result = 0;

    switch (parm->op) {
    case 0xE9:
    case 0xED:
    case 0xE5:
    case 0xFD:
    case 0xF9:
    case 0xF5:
    case 0xF1:
    case 0xE1:
        result = parm->read(parm->op - 0xE1);
        break;
    }

    result -= cpu->A;
    result -= (cpu->FLAGS & CPU_FLAGS_CARRY) ? 1 : 0;

    if (((signed short int)result)>=0) {
        SET_FLAGS(CPU_FLAGS_CARRY);
    } else {
        CLE_FLAGS(CPU_FLAGS_CARRY);
    }

    cpu->checkV(result, cpu->A);
    cpu->checkNZ((byte)result);
    cpu->A = (byte) result;
}

/* --X */
void cpu_command_DEX(command_6502* cmd, command_parm* parm) {
    cpu_6502 *cpu = parm->cpu;
    cpu->X--;
    cpu->checkNZ(cpu->X);
}

/* ++X */
void cpu_command_INX(command_6502* cmd, command_parm* parm) {
    cpu_6502 *cpu = parm->cpu;
    cpu->X++;
    cpu->checkNZ(cpu->X);
}

/* --Y */
void cpu_command_DEY(command_6502* cmd, command_parm* parm) {
    cpu_6502 *cpu = parm->cpu;
    cpu->Y--;
    cpu->checkNZ(cpu->Y);
}

/* ++Y */
void cpu_command_INY(command_6502* cmd, command_parm* parm) {
    cpu_6502 *cpu = parm->cpu;
    cpu->Y++;
    cpu->checkNZ(cpu->Y);
}

/* --内存 */
void cpu_command_DEC(command_6502* cmd, command_parm* parm) {
    switch (parm->op) {
    case 0xCE:
    case 0xC6:
    case 0xDE:
    case 0xD6:
        memory *ram = parm->ram;
        word addr = parm->getAddr(parm->op - 0xC2);
        byte value = ram->read(addr) - 1;
        ram->write(addr, value);
        parm->cpu->checkNZ(value);
    }
}

/* ++内存 */
void cpu_command_INC(command_6502* cmd, command_parm* parm) {
    switch (parm->op) {
    case 0xEE:
    case 0xE6:
    case 0xFE:
    case 0xF6:
        memory *ram = parm->ram;
        word addr = parm->getAddr(parm->op - 0xE2);
        byte value = ram->read(addr) + 1;
        ram->write(addr, value);
        parm->cpu->checkNZ(value);
    }
}

/* 把A寄存器存入内存 */
void cpu_command_STA(command_6502* cmd, command_parm* parm) {
    switch (parm->op) {
    case 0x8D:
    case 0x85:
    case 0x9D:
    case 0x99:
    case 0x95:
    case 0x91:
    case 0x81:
        parm->write(parm->op - 0x81, parm->cpu->A);
    }
}

/* 把Y寄存器存入内存 */
void cpu_command_STY(command_6502* cmd, command_parm* parm) {
    switch (parm->op) {
    case 0x8C:
    case 0x84:
    case 0x94:
        parm->write(parm->op - 0x80, parm->cpu->Y);
    }
}

/* 把X寄存器存入内存 */
void cpu_command_STX(command_6502* cmd, command_parm* parm) {
    switch (parm->op) {
    case 0x8E:
    case 0x86:
    case 0x96:
        parm->write(parm->op - 0x82, parm->cpu->X);
    }
}

/* 由内存载入Y寄存器 */
void cpu_command_LDY(command_6502* cmd, command_parm* parm) {

    cpu_6502* cpu = parm->cpu;

    switch (parm->op) {
    case 0xA0:
        cpu->Y = parm->p1;
        break;
    case 0xAC:
    case 0xA4:
    case 0xBC:
    case 0xB4:
        cpu->Y = parm->read(parm->op - 0xA0);
        break;
    }
    cpu->checkNZ(cpu->Y);
}

/* 由内存载入A寄存器 */
void cpu_command_LDA(command_6502* cmd, command_parm* parm) {

    cpu_6502* cpu = parm->cpu;

    switch (parm->op) {
    case 0xA9:
    case 0xAD:
    case 0xA5:
    case 0xBD:
    case 0xB9:
    case 0xB5:
    case 0xB1:
    case 0xA1:
        cpu->A = parm->read(parm->op - 0xA1);
        break;
    }

    cpu->checkNZ(cpu->A);
}

/* 由内存载入X寄存器 */
void cpu_command_LDX(command_6502* cmd, command_parm* parm) {

    cpu_6502* cpu = parm->cpu;

    switch (parm->op) {
    case 0xA2:
        cpu->X = parm->p1;
        return;
    case 0xAE:
    case 0xA6:
    case 0xBE:
    case 0xB6:
        cpu->X = parm->read(parm->op - 0xA2);
        break;
    }

    cpu->checkNZ(cpu->X);
}

/* 累加器入栈 */
void cpu_command_PHA(command_6502* cmd, command_parm* parm) {
    cpu_6502* cpu = parm->cpu;
    cpu->push(cpu->A);
}

/* 出栈入累加器 */
void cpu_command_PLA(command_6502* cmd, command_parm* parm) {
    cpu_6502* cpu = parm->cpu;
    cpu->A = cpu->pop();
    cpu->checkNZ(cpu->A);
}

/* FLAGS入栈 */
void cpu_command_PHP(command_6502* cmd, command_parm* parm) {
    cpu_6502* cpu = parm->cpu;
    cpu->push(cpu->FLAGS);
}

/* 出栈入FLAGS */
void cpu_command_PLP(command_6502* cmd, command_parm* parm) {
    cpu_6502* cpu = parm->cpu;
    cpu->FLAGS = cpu->pop() | 0x30;
}

/* a = b */
#define _TR(b,a) cpu_6502 *cpu = parm->cpu; \
                 cpu->a = cpu->b

void cpu_command_TAY(command_6502* cmd, command_parm* parm) {
    _TR(A, Y);
    cpu->checkNZ(cpu->A);
}

void cpu_command_TAX(command_6502* cmd, command_parm* parm) {
    _TR(A, X);
    cpu->checkNZ(cpu->A);
}

void cpu_command_TXA(command_6502* cmd, command_parm* parm) {
    _TR(X, A);
    cpu->checkNZ(cpu->A);
}

void cpu_command_TYA(command_6502* cmd, command_parm* parm) {
    _TR(Y, A);
    cpu->checkNZ(cpu->A);
}

void cpu_command_TSX(command_6502* cmd, command_parm* parm) {
    _TR(SP, X);
    cpu->checkNZ(cpu->X);
}

void cpu_command_TXS(command_6502* cmd, command_parm* parm) {
    _TR(X, SP);
}

#undef _TR

/* 设置为禁止中断 */
void cpu_command_SEI(command_6502* cmd, command_parm* parm) {
    SET_FLAGS(CPU_FLAGS_INTERDICT);
}

/* 清除禁止中断位 */
void cpu_command_CLI(command_6502* cmd, command_parm* parm) {
    CLE_FLAGS(CPU_FLAGS_INTERDICT);
}

/* 设置为十进制(BCD)算术运算 */
void cpu_command_SED(command_6502* cmd, command_parm* parm) {
    SET_FLAGS(CPU_FLAGS_DECIMAL);
}

/* 还原为二进制算术运算 */
void cpu_command_CLD(command_6502* cmd, command_parm* parm) {
    CLE_FLAGS(CPU_FLAGS_DECIMAL);
}

/* 设置进位标志 */
void cpu_command_SEC(command_6502* cmd, command_parm* parm) {
    SET_FLAGS(CPU_FLAGS_CARRY);
}

/* 清除进位标志 */
void cpu_command_CLC(command_6502* cmd, command_parm* parm) {
    CLE_FLAGS(CPU_FLAGS_CARRY);
}

/* 清除溢出位 */
void cpu_command_CLV(command_6502* cmd, command_parm* parm) {
    parm->cpu->clearV();
}

#undef SET_FLAGS
#undef CLE_FLAGS

/****************************************************************************/

#define Op(n, t, l, p)   {#n, t, l, command_6502::vt_op_##p, cpu_command_##n}
#define  L               Op(XXX, 0, 1, not)
#define  N               L,
#define  O(n, t, l, p)   Op(n, t, l, p),

/* 索引既是6502命令的首字节 */
command_6502 command_list_6502[] = {
/* --------------------------------------| 0x00 - 0x0F |--- */
    O(BRK, 7, 1, not)   O(ORA, 6, 2, ind)   N
    N                   N                   O(ORA, 3, 2, zpg)
    O(ASL, 5, 2, zpg)   N                   O(PHP, 3, 1, not)
    O(ORA, 2, 2, imm)   O(ASL, 2, 1, not)   N
    N                   O(ORA, 4, 3, abs)   O(ASL, 6, 3, abs)
    N
/* --------------------------------------| 0x10 - 0x1F |--- */
    O(BPL, 2, 2, rel)   O(ORA, 5, 2, ind)   N
    N                   N                   O(ORA, 4, 2, zpg)
    O(ASL, 6, 2, zpg)   N                   O(CLC, 2, 1, not)
    O(ORA, 4, 3, abs)   N                   N
    N                   O(ORA, 4, 3, abs)   O(ASL, 6, 3, abs)
    N
/* --------------------------------------| 0x20 - 0x2F |--- */
    O(JSR, 6 ,3, rel)   O(AND, 6, 2, ind)   N
    N                   O(BIT, 3, 2, zpg)   O(AND, 3, 2, zpg)
    O(ROL, 5, 2, zpg)   N                   O(PLP, 4, 1, not)
    O(AND, 2, 2, imm)   O(ROL, 2, 1, not)   N
    O(BIT, 4, 3, abs)   O(AND, 4, 3, abs)   O(ROL, 6, 3, abs)
    N
/* --------------------------------------| 0x30 - 0x3F |--- */
    O(BMI, 2, 2, rel)   O(AND, 5, 2, ind)   N
    N                   N                   O(AND, 4, 2, zpg)
    O(ROL, 6, 2, zpg)   N                   O(SEC, 2, 1, not)
    O(AND, 4, 3, abs)   N                   N
    N                   O(AND, 4, 3, abs)   O(ROL, 6, 3, abs)
    N
/* --------------------------------------| 0x40 - 0x4F |--- */
    O(RTI, 6, 1, not)   O(EOR, 6, 2, ind)   N
    N                   N                   O(EOR, 3, 2, zpg)
    O(LSR, 5, 2, zpg)   N                   O(PHA, 3, 1, not)
    O(EOR, 2, 2, imm)   O(LSR, 2, 1, not)   N
    O(JMP, 3, 3, abs)   O(EOR, 4, 3, abs)   O(LSR, 6, 3, abs)
    N
/* --------------------------------------| 0x50 - 0x5F |--- */
    O(BVC, 2, 2, rel)   O(EOR, 5, 2, ind)   N
    N                   N                   O(EOR, 4, 2, zpg)
    O(LSR, 6, 2, zpg)   N                   O(CLI, 2, 1, not)
    O(EOR, 4, 3, abs)   N                   N
    N                   O(EOR, 4, 3, abs)   O(LSR, 6, 3, abs)
    N
/* --------------------------------------| 0x60 - 0x6F |--- */
    O(RTS, 6, 1, not)   O(ADC, 6, 2, ind)   N
    N                   N                   O(ADC, 3, 2, zpg)
    O(ROR, 5, 2, zpg)   N                   O(PLA, 4, 1, not)
    O(ADC, 2, 2, imm)   O(ROR, 2, 1, not)   N
    O(JMP, 6, 3, ind)   O(ADC, 4, 3, abs)   O(ROR, 6, 3, abs)
    N
/* --------------------------------------| 0x70 - 0x7F |--- */
    O(BVS, 2, 2, rel)   O(ADC, 5, 2, ind)   N
    N                   N                   O(ADC, 4, 2, zpg)
    O(ROR, 6, 2, zpg)   N                   O(SEI, 2, 1, not)
    O(ADC, 4, 3, abs)   N                   N
    N                   O(ADC, 4, 3, abs)   O(ROR, 6, 3, abs)
    N
/* --------------------------------------| 0x80 - 0x8F |--- */
    N                   O(STA, 6, 2, ind)   N
    N                   O(STY, 3, 2, zpg)   O(STA, 3, 2, zpg)
    O(STX, 3, 2, zpg)   N                   O(DEY, 2, 1, not)
    N                   O(TXA, 2, 1, not)   N
    O(STY, 4, 3, abs)   O(STA, 4, 3, abs)   O(STX, 4, 3, abs)
    N
/* --------------------------------------| 0x90 - 0x9F |--- */
    O(BCC, 2, 2, rel)   O(STA, 6, 2, ind)   N
    N                   O(STY, 4, 2, zpg)   O(STA, 4, 2, zpg)
    O(STX, 4, 2, zpg)   N                   O(TYA, 2, 1, not)
    O(STA, 5, 3, abs)   O(TXS, 2, 1, not)   N
    N                   O(STA, 4, 3, abs)   N
    N
/* --------------------------------------| 0xA0 - 0xAF |--- */
    O(LDY, 2, 2, imm)   O(LDA, 6, 2, ind)   O(LDX, 2, 2, imm)
    N                   O(LDY, 3, 2, zpg)   O(LDA, 3, 2, zpg)
    O(LDX, 3, 2, zpg)   N                   O(TAY, 2, 1, not)
    O(LDA, 2, 2, imm)   O(TAX, 2, 1, not)   N
    O(LDY, 4, 3, abs)   O(LDA, 4, 3, abs)   O(LDX, 4, 3, abs)
    N
/* --------------------------------------| 0xB0 - 0xBF |--- */
    O(BCS, 2, 2, rel)   O(LDA, 5, 2, ind)   N
    N                   O(LDY, 4, 2, zpg)   O(LDA, 4, 2, zpg)
    O(LDX, 4, 2, zpg)   N                   O(CLV, 2, 1, not)
    O(LDA, 4, 3, abs)   O(TSX, 2, 1, not)   N
    O(LDY, 4, 3, abs)   O(LDA, 4, 3, abs)   O(LDX, 4, 3, abs)
    N
/* --------------------------------------| 0xC0 - 0xCF |--- */
    O(CPY, 2, 2, imm)   O(CMP, 6, 2, ind)   N
    N                   O(CPY, 3, 2, zpg)   O(CMP, 3, 2, zpg)
    O(DEC, 5, 2, zpg)   N                   O(INY, 2, 1, not)
    O(CMP, 2, 2, imm)   O(DEX, 2, 1, not)   N
    O(CPY, 4, 3, abs)   O(CMP, 4, 3, abs)   O(DEC, 6, 3, abs)
    N
/* --------------------------------------| 0xD0 - 0xDF |--- */
    O(BNE, 2, 2, rel)   O(CMP, 5, 2, ind)   N
    N                   N                   O(CMP, 4, 2, zpg)
    O(DEC, 6, 2, zpg)   N                   O(CLD, 2, 1, not)
    O(CMP, 4, 3, abs)   N                   N
    N                   O(CMP, 4, 3, abs)   O(DEC, 6, 3, abs)
    N
/* --------------------------------------| 0xE0 - 0xEF |--- */
    O(CPX, 2, 2, imm)   O(SBC, 6, 2, ind)   N
    N                   O(CPX, 3, 2, zpg)   O(SBC, 3, 2, zpg)
    O(INC, 5, 2, zpg)   N                   O(INX, 2, 1, not)
    O(SBC, 2, 2, imm)   O(NOP, 2, 1, not)   N
    O(CPX, 4, 3, abs)   O(SBC, 4, 3, abs)   O(INC, 6, 3, abs)
    N
/* --------------------------------------| 0xF0 - 0xFF |--- */
    O(BEQ, 2, 2, rel)   O(SBC, 5, 2, ind)   N
    N                   N                   O(SBC, 4, 2, zpg)
    O(INC, 6, 2, zpg)   N                   O(SED, 2, 1, not)
    O(SBC, 4, 3, abs)   N                   N
    N                   O(SBC, 4, 3, abs)   O(INC, 6, 3, abs)
    N
/* --------------------------------------| over        |--- */
    L
};

#undef N
#undef O
#undef L
#undef Op

/****************************************************************************/

cpu_6502::cpu_6502(memory* ram)
        : NMI_idle(1), NMI(0), IRQ(0), RES(1), ram(ram)
{
}

inline void cpu_6502::push(byte d) {
    ram->write(SP | 0x0100, d);
    SP--;
}

inline byte cpu_6502::pop() {
    SP++;
    return ram->read(SP | 0x0100);
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
    command_6502 *cmd = &command_list_6502[prev_parm->op];

    switch (cmd->len) {
    case 3:
        cc = "%04X.  >%s  [%02X] %s %02X %02X \n"; break;
    case 2:
        cc = "%04X.  >%s  [%02X] %s %02X -- \n";   break;
    default:
        cc = "%04X.  >%s  [%02X] %s -- -- \n";
    }

    switch (cmd->type) {
    case command_6502::vt_op_abs:
        tp = "abs"; break;
    case command_6502::vt_op_imm:
        tp = "imm"; break;
    case command_6502::vt_op_ind:
        tp = "ind"; break;
    case command_6502::vt_op_rel:
        tp = "rel"; break;
    case command_6502::vt_op_zpg:
        tp = "zpg"; break;
    default:
        tp = "---"; break;
    }

    sprintf(buf, cc, prev_parm->addr, tp,
                 prev_parm->op, cmd->name, prev_parm->p1, prev_parm->p2);

    return buf;
}

byte cpu_6502::process() {

    static command_parm parm;
    prev_parm = &parm;

    byte cpu_cyc = reset();

    parm.op = ram->readPro(PC);
    command_6502 *opp = &command_list_6502[parm.op];

    switch (opp->len) {
    case 3:
        parm.p2 = ram->readPro(PC+2);
    case 2:
        parm.p1 = ram->readPro(PC+1);
    }

    parm.addr = PC;
    parm.cpu = this;
    parm.ram = parm.cpu->ram;
    PC += opp->len;

    opp->op_func(opp, &parm);

    cpu_cyc += nmi();
    cpu_cyc += irq();
    cpu_cyc += opp->time;

    return cpu_cyc;
}

inline byte cpu_6502::reset() {
    if (RES) {
        IRQ    = 0;
        NMI    = 0;
        RES    = 0;
        FLAGS  = CPU_FLAGS_CONST | CPU_FLAGS_INTERDICT;

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
if (!NMI_idle) {
    //printf(cmdInfo());
}
    if (!NMI) {
        NMI_idle = 1;
    }
    if (NMI && NMI_idle) {
        //NMI = 0;
        NMI_idle = 0;
        jump(0xFFFA);
printf("NMI中断\n");
printf(debug());
        return CPU_NMI_CYC;
    }
    return 0;
}

inline void cpu_6502::rti() {
    FLAGS = pop();
    PCL   = pop();
    PCH   = pop();
}

inline void cpu_6502::jump(word addr) {
    push(PCH);
    push(PCL);
    push(FLAGS);
    PCH    = ram->read(addr+1);
    PCL    = ram->read(addr  );
    FLAGS |= CPU_FLAGS_INTERDICT;
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

inline void cpu_6502::checkV(word value, byte beforeOper) {
    if (value & beforeOper & 0x80) {
        FLAGS |= CPU_FLAGS_OVERFLOW;
    } else {
        FLAGS &= 0xFF ^ CPU_FLAGS_OVERFLOW;
    }
}

inline void cpu_6502::clearV() {
    FLAGS &= 0xFF ^ CPU_FLAGS_OVERFLOW;
}

/****************************************************************************/

inline word command_parm::abs() {
    return (p2<<8) | p1;
}

inline word command_parm::absX() {
    return abs() + cpu->X;
}

inline word command_parm::absY() {
    return abs() + cpu->Y;
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
    return $$$(0, cpu->Y);
}

inline word command_parm::$zpgX$() {
    return $$$(cpu->X, 0);
}

inline word command_parm::$$$(byte x, byte y) {
    word offset = (p1 + x) & 0x00FF;
    byte l = ram->read( offset   );
    byte h = ram->read( offset+1 );
    return (h<<8 | l) + y;
}

inline byte command_parm::read(const byte addressing_mode) {
    if (addressing_mode==ADD_MODE_imm) {
        return p1;
    } else
    if (addressing_mode==ADD_MODE_acc) {
        return cpu->A;
    }
    return ram->read( getAddr(addressing_mode) );
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
    cpu->ram->write(getAddr(addressing_mode), value);
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

    case ADD_MODE_$zpg$Y:
        return $zpg$Y();

    case ADD_MODE_$zpgX$:
        return $zpgX$();
    }

    printf("!无效的寻址代码: %d\n", addressing_mode);
    return 0;
}
