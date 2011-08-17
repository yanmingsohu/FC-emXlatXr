#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <time.h>
#include <string>
#include "nes_sys.h"
#include "cpu.h"

const int test_command = 800;

class CmdVideo : public Video {
    void drawPixel(int x, int y, T_COLOR color) {
        //printf("::draw pixel:(%02x,%02x)=%08x\n", x, y, color);
    }
};

void testCore() {

    using std::string;
    // "rom/Tennis.nes" "rom/Dr_Mario.nes"
    string filename = "rom/f-1.nes";

    CmdVideo video;
    NesSystem fc(&video);

    switch (fc.load_rom(filename)) {
    case LOAD_ROM_SUCCESS:
        goto _LOAD_SUCCESS;

    case ER_LOAD_ROM_PARM:
        printf("parm not null.\n");
        break;
    case ER_LOAD_ROM_OPEN:
        printf("\ncannot open '%s' file.\n", filename.c_str());
        break;
    case ER_LOAD_ROM_HEAD:
        printf("The file '%s' not nes rom. \n", filename.c_str());
        break;
    case ER_LOAD_ROM_TRAINER:
        printf("The 'trainer' not enough '%s'.\n", filename.c_str());
        break;
    case ER_LOAD_ROM_SIZE:
        printf("cannot read enough rom size.\n");
        break;
    case ER_LOAD_ROM_VSIZE:
        printf("cannot read enough vrom size.\n");
        break;
    case ER_LOAD_ROM_BADMAP:
        printf("unsupport map id\n");
        break;
    default:
        printf("unknow error.\n");
    }

    return;

_LOAD_SUCCESS:

    printf("load '%s' over, start..\n\n", filename.c_str());

    cpu_6502 *cpu = fc.getCpu();
    PPU      *ppu = fc.getPPU();
    memory   *ram = fc.getMem();
    cpu->showCmds(0);

    int c        = 0;
    int frameIrq = 0;
    int skip     = 0;
    char ch;

    clock_t s = clock();

    for (;;) {

        printf("\t\t\t\t\t< step: [ %5d ] | h:print help >\n", c);
        ch = getch();

        if (ch=='0') {
            cpu->process();
            ++c;
            if (frameIrq && (c%frameIrq==0)) ppu->oneFrameOver();
            printf(cpu->cmdInfo());
            printf(cpu->debug());
        }

        else if (ch=='h') {
printf(
"\n| 0 :goto next cpu operator               | n :new frame \
 \n| o :frame over                           | m :display memory \
 \n| s :skip 'n' operator,not display        | r :reset cpu \
 \n| i :set operate to send frame IRQ        | \
 \n| x :exit \n"
);
        }

        else if (ch=='s') {
            printf("input skip number: ");
            fflush(stdin);
            scanf("%d", &skip);

            while (skip-- > 0) {
                cpu->process();
                ++c;
                if (frameIrq && (c%frameIrq==0)) ppu->oneFrameOver();
            }
        }

        else if (ch=='i') {
            printf("input how operate send IRQ: ");
            fflush(stdin);
            scanf("%d", &frameIrq);
        }

        else if (ch=='r') {
            cpu->reset();
            c = 0;
        }
        else if (ch=='n') {
            ppu->startNewFrame();
        }
        else if (ch=='o') {
            ppu->oneFrameOver();
        }
        else if (ch=='x') {
            break;
        }

        else if (ch=='m') {
            word addr = 0;
            word len = 0x20;
            printf("input memort(16): ");
            fflush(stdin);
            scanf("%x", &addr);

            printf("|------|-");
            for (int i=0; i<0x10; ++i) {
                printf(" -%X", (i+addr)%0x10);
            }
            printf(" |");

            for (int i=addr; i<len+addr; ++i) {
                if ((i-addr)%16==0) printf("\n 0x%04X: ", i);
                printf(" %02X", ram->read(i));
            }
            printf("\n|-over-|- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- |\n");
        }
    }

    clock_t e = clock();

    printf("执行 %d 条指令使用了 %lf 毫秒\n", c,
           (double)(e - s)*(CLOCKS_PER_SEC/1000));
}


void testCommand() {
}

int __main()
{
    welcome();

    if (!(0 & CPU_FLAGS_ZERO)) {
        printf("jump\n");
    }
    testCore();
    //testCommand();
    system("pause");
    return 0;
}

