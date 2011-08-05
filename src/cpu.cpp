#include "mem.h"
#include "cpu.h"
#include "stdio.h"
#include "stdlib.h"

void display_command(command_6502* cmd, command_parm* parm) {
    const char* cc = NULL;

    switch (cmd->len) {
    case 3:
        cc = "%04X.  [%02X]  %s %02X %02X \n";
        break;
    case 2:
        cc = "%04X.  [%02X]  %s %02X \n";
        break;
    default:
        cc = "%04X.  [%02X]  %s \n";
    }

    printf(cc, parm->cpu->PC - cmd->len,
               parm->op, cmd->name, parm->p1, parm->p2);
}

void cpu_command_XXX(command_6502* cmd, command_parm* parm) {
    printf("unknow command %s on %04x exit.",
           cmd->name, parm->cpu->PC - cmd->len);
}

void cpu_command_BRK(command_6502* cmd, command_parm* parm) {

}
void cpu_command_ASL(command_6502* cmd, command_parm* parm) {

}
void cpu_command_AND(command_6502* cmd, command_parm* parm) {

}
void cpu_command_BPL(command_6502* cmd, command_parm* parm) {

}
void cpu_command_BIT(command_6502* cmd, command_parm* parm) {

}
void cpu_command_CLC(command_6502* cmd, command_parm* parm) {

}
void cpu_command_PHP(command_6502* cmd, command_parm* parm) {

}
void cpu_command_PLP(command_6502* cmd, command_parm* parm) {

}
void cpu_command_ROL(command_6502* cmd, command_parm* parm) {

}
void cpu_command_ORA(command_6502* cmd, command_parm* parm) {

}
void cpu_command_JSR(command_6502* cmd, command_parm* parm) {

}
void cpu_command_BMI(command_6502* cmd, command_parm* parm) {

}
void cpu_command_SEC(command_6502* cmd, command_parm* parm) {

}
void cpu_command_RTI(command_6502* cmd, command_parm* parm) {

}
void cpu_command_EOR(command_6502* cmd, command_parm* parm) {

}
void cpu_command_LSR(command_6502* cmd, command_parm* parm) {

}
void cpu_command_PHA(command_6502* cmd, command_parm* parm) {

}

void cpu_command_JMP(command_6502* cmd, command_parm* parm) {

    cpu_6502* cpu = parm->cpu;

    if (parm->op==0x4C) {
        cpu->PCL = parm->p1;
        cpu->PCH = parm->p2;
    } else {
        word offset = (parm->p2<<8) | parm->p1;
        cpu->PCL = cpu->ram->read( offset );
        cpu->PCH = cpu->ram->read( offset+1 );
    }
}

void cpu_command_BVC(command_6502* cmd, command_parm* parm) {

}
void cpu_command_ADC(command_6502* cmd, command_parm* parm) {

}
void cpu_command_CLI(command_6502* cmd, command_parm* parm) {

}
void cpu_command_RTS(command_6502* cmd, command_parm* parm) {

}
void cpu_command_ROR(command_6502* cmd, command_parm* parm) {

}
void cpu_command_PLA(command_6502* cmd, command_parm* parm) {

}
void cpu_command_BVS(command_6502* cmd, command_parm* parm) {

}
void cpu_command_SEI(command_6502* cmd, command_parm* parm) {

}

void cpu_command_STA(command_6502* cmd, command_parm* parm) {


    cpu_6502* cpu = parm->cpu;
    word offset = 0;

    switch (parm->op) {
    case 0x8D:
        offset = parm->abs();
        break;
    case 0x85:
        offset = parm->zpg();
        break;
    case 0x9D:
        offset = parm->absX();
        break;
    case 0x99:
        offset = parm->absY();
        break;
    case 0x95:
        offset = parm->zpgY();
        break;
    case 0x91:
        offset = parm->$zpg$Y();
        break;
    case 0x81:
        offset = parm->$zpgX$();
        break;
    }

    cpu->ram->write(offset, cpu->A);
}

void cpu_command_STY(command_6502* cmd, command_parm* parm) {


    cpu_6502* cpu = parm->cpu;
    word offset = 0;

    switch (parm->op) {
    case 0x8C:
        offset = parm->abs();
        break;
    case 0x84:
        offset = parm->zpg();
        break;
    case 0x94:
        offset = parm->zpgX();
        break;
    }

    cpu->ram->write(offset, cpu->Y);
}

void cpu_command_STX(command_6502* cmd, command_parm* parm) {


    cpu_6502* cpu = parm->cpu;
    word offset = 0;

    switch (parm->op) {
    case 0x8E:
        offset = parm->abs();
        break;
    case 0x86:
        offset = parm->zpg();
        break;
    case 0x96:
        offset = parm->zpgY();
        break;
    }

    cpu->ram->write(offset, cpu->X);
}

void cpu_command_DEY(command_6502* cmd, command_parm* parm) {

}
void cpu_command_TXA(command_6502* cmd, command_parm* parm) {

}
void cpu_command_BCC(command_6502* cmd, command_parm* parm) {

}

void cpu_command_LDY(command_6502* cmd, command_parm* parm) {


    cpu_6502* cpu = parm->cpu;
    word offset = 0;

    switch (parm->op) {
    case 0xA0:
        cpu->Y = parm->p1;
        return;
    case 0xAC:
        offset = parm->abs();;
        break;
    case 0xA4:
        offset = parm->zpg();;
        break;
    case 0xBC:
        offset = parm->absX();;
        break;
    case 0xB4:
        offset = parm->zpgX();;
        break;
    }

    cpu->Y = cpu->ram->read( offset );
    cpu->checkNZ(cpu->Y);
}

void cpu_command_LDA(command_6502* cmd, command_parm* parm) {


    cpu_6502* cpu = parm->cpu;
    word offset = 0;

    switch (parm->op) {
    case 0xA9:
        cpu->A = parm->p1;
        return;
    case 0xAD:
        offset = parm->abs();
        break;
    case 0xA5:
        offset = parm->zpg();
        break;
    case 0xBD:
        offset = parm->absX();
        break;
    case 0xB9:
        offset = parm->absY();
        break;
    case 0xB5:
        offset = parm->zpgX();
        break;
    case 0xB1:
        offset = parm->$zpg$Y();
        break;
    case 0xA1:
        offset = parm->$zpgX$();
        break;
    }

    cpu->A = cpu->ram->read( offset );
    cpu->checkNZ(cpu->A);
}

void cpu_command_LDX(command_6502* cmd, command_parm* parm) {


    cpu_6502* cpu = parm->cpu;
    word offset = 0;

    switch (parm->op) {
    case 0xA2:
        cpu->X = parm->p1;
        return;
    case 0xAE:
        offset = parm->abs();
        break;
    case 0xA6:
        offset = parm->zpg();
        break;
    case 0xBE:
        offset = parm->absY();
        break;
    case 0xB6:
        offset = parm->zpgY();
        break;
    }

    cpu->X = cpu->ram->read( offset );
    cpu->checkNZ(cpu->X);
}

void cpu_command_TAY(command_6502* cmd, command_parm* parm) {

}
void cpu_command_TYA(command_6502* cmd, command_parm* parm) {

}
void cpu_command_TXS(command_6502* cmd, command_parm* parm) {

}
void cpu_command_TAX(command_6502* cmd, command_parm* parm) {

}
void cpu_command_BSC(command_6502* cmd, command_parm* parm) {

}
void cpu_command_CLV(command_6502* cmd, command_parm* parm) {

}
void cpu_command_TSX(command_6502* cmd, command_parm* parm) {

}
void cpu_command_CPY(command_6502* cmd, command_parm* parm) {

}
void cpu_command_CMP(command_6502* cmd, command_parm* parm) {

}
void cpu_command_DEC(command_6502* cmd, command_parm* parm) {

}
void cpu_command_DEX(command_6502* cmd, command_parm* parm) {

}
void cpu_command_BNE(command_6502* cmd, command_parm* parm) {

}
void cpu_command_BCS(command_6502* cmd, command_parm* parm) {

}
void cpu_command_INY(command_6502* cmd, command_parm* parm) {

}
void cpu_command_CLD(command_6502* cmd, command_parm* parm) {

}
void cpu_command_CPX(command_6502* cmd, command_parm* parm) {

}
void cpu_command_SBC(command_6502* cmd, command_parm* parm) {

}
void cpu_command_INC(command_6502* cmd, command_parm* parm) {

}
void cpu_command_INX(command_6502* cmd, command_parm* parm) {

}
void cpu_command_NOP(command_6502* cmd, command_parm* parm) {

}
void cpu_command_SED(command_6502* cmd, command_parm* parm) {

}
void cpu_command_BEQ(command_6502* cmd, command_parm* parm) {

}

/****************************************************************************/

#define Op(n, t, l, p)   {#n, t, l, command_6502::vt_op_##p, cpu_command_##n}
#define  L               Op(XXX, 0, 1, not)
#define  N               L,
#define  O(n, t, l, p)   Op(n, t, l, p),

/* 索引既是6502命令的首字节 */
static command_6502 command_list_6502[] = {
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
        : ram(ram)
{
}

void cpu_6502::reset() {
    A      = 0;
    Y      = 0;
    X      = 0;

    PC     = 0;
    SP     = 0x01FF;
    FLAGS  = 0x20;

    PCH    = ram->read(0xFFFD);
    PCL    = ram->read(0xFFFC);
}

void cpu_6502::push(byte d) {
    ram->write(SP | 0x0100, d);
    SP--;
}

byte cpu_6502::pop() {
    SP++;
    return ram->read(SP);
}

byte cpu_6502::process() {
    static command_parm parm;

    parm.op = ram->readPro(PC);
    command_6502 *opp = &command_list_6502[parm.op];

    switch (opp->len) {
    case 3:
        parm.p2 = ram->readPro(PC+2);
    case 2:
        parm.p1 = ram->readPro(PC+1);
    }

    PC += opp->len;
    parm.cpu = this;
    display_command(opp, &parm);
    opp->op_func(opp, &parm);

    return opp->time;
}

void cpu_6502::debug() {
    char buf[9];
    itoa(FLAGS, buf, 2);

    printf("CPU >\t A: %04x\t X: %04x\t Y: %04x\n"
           "    >\tPC: %04x\tSP: %04x\tFG: %08s\n",
           A, X, Y, PC, SP, buf);
}
