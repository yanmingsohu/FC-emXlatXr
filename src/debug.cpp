#include   <stdio.h>
#include  <stdlib.h>
#include   <conio.h>
#include    <time.h>
#include     "cpu.h"
#include "nes_sys.h"

static void cpu_debug_help() {
printf(
"------------------------------------------------------------------------------+\
\n| 0 :goto next cpu operator               | n :new frame \
\n| o :frame over                           | m :display memory \
\n| s :skip 'n' operator,not display        | r :reset cpu \
\n| i :set operate to send frame IRQ        | d :draw a from \
\n| g : goto XXXX  \
\n| x :exit \
\n------------------------------------------------------------------------------&\n"
);
}

void debugCpu(NesSystem *fc) {

    cpu_6502 *cpu = fc->getCpu();
    PPU      *ppu = fc->getPPU();
    memory   *ram = fc->getMem();

    cpu->showDebug(1);

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
            cpu_debug_help();
        }
        else if (ch=='d') {
            fc->drawFrame();
        }

        else if (ch=='g') {
            printf("input IP(16): ");
            fflush(stdin);
            scanf("%x", &cpu->FLAGS);
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
    cpu->showDebug(0);

    printf("执行 %d 条指令使用了 %lf 毫秒\n", c,
           (double)(e - s)*(CLOCKS_PER_SEC/1000));
}
