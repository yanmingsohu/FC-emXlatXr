#include   <stdio.h>
#include  <stdlib.h>
#include   <conio.h>
#include    <time.h>
#include     "cpu.h"
#include "nes_sys.h"
#include     "pad.h"

static void cpu_debug_help() {
    printf(
"------------------------------------------------------------------------------+\
\n| 0 :goto next cpu operator             | n :new frame \
\n| o :frame over                         | m :display memory \
\n| s :skip 'n' operator,not display      | r :reset cpu \
\n| i :set operate to send frame IRQ      | d :draw a frame \
\n| g :goto XXXX                          | b :breakpoint \
\n| p :push key                           | c :clear screen \
\n| x :exit \
\n------------------------------------------------------------------------------&\n"
    );
}

#define CPU_IS(o, x, y)        \
         ( (parm->op==(0x##o)) \
         &&(parm->p1==(0x##x)) \
         &&(parm->p2==(0x##y)) )
static void condition_code(NesSystem *fc) { /* 实现可能不准确 */

    cpu_6502     *cpu  = fc->getCpu();
    PPU          *ppu  = fc->getPPU();
    command_parm *parm = &cpu->prev_parm;

    if (CPU_IS(8D, 00, 20)) {
        printf("DBG::修改PPU窗口坐标高位,滚动方式,字库偏移,精灵大小,VB中断\n");
    } else

    if (CPU_IS(8D, 01, 20)) {
        printf("DBG::修改PPU左列状态,显示屏蔽,背景色\n");
    } else

    if (CPU_IS(8D, 03, 20)) {
        printf("DBG::修改PPU精灵内存指针\n");
    } else

    if (CPU_IS(8D, 04, 20)) {
        printf("DBG::向PPU精灵内存写数据\n");
    } else

    if (CPU_IS(8D, 05, 20)) {
        int x, y;
        ppu->getWindowPos(&x, &y);
        printf("DBG::修改PPU窗口坐标 x:%d,y:%d\n", x, y);
    } else

    if (CPU_IS(8D, 06, 20)) {
        printf("DBG::修改PPU指针:%04x\n", ppu->getVRamPoint());
    } else

    if (CPU_IS(8D, 07, 20)) {
        printf("DBG::向PPU写数据 地址:%04X\n", ppu->getVRamPoint());
    } else

    if (CPU_IS(AD, 02, 20)) {
        printf("DBG::读取2002数据,(精灵溢出[20],碰撞[40],VBlank[80])\n");
    } else

    if (CPU_IS(AD, 07, 20)) {
        printf("DBG::读取PPU显存数据 地址:%04X\n", ppu->getVRamPoint()-1);
    }
}
#undef CPU_IS

void debugCpu(NesSystem *fc) {

    cpu_6502 *cpu = fc->getCpu();
    PPU      *ppu = fc->getPPU();
    memory   *ram = fc->getMem();

    cpu->showDebug(1);

    int c           = 0;
    int frameIrq    = 0;
    int skip        = 0;
    int breakpoint  = -1;
    char ch;

    clock_t s = clock();

    for (;;) {

        printf("\t\t\t\t\t< step: [ %5d ] | h:print help >\n", c);
        ch = getch();

        if (ch=='0') {
            cpu->process();
            ++c;
            if (frameIrq && (c%frameIrq==0)) ppu->oneFrameOver();
            condition_code(fc);
            printf(cpu->cmdInfo());
            printf(cpu->debug());
        }

        else if (ch=='h') {
            cpu_debug_help();
        }
        else if (ch=='c') {
            system("cls");
        }

        else if (ch=='b') {
            printf("XX input PC(16) when stop: ");
            fflush(stdin);
            scanf("%x", &breakpoint);
        }
        else if (ch=='p') {
            int key = 0;
            printf("push key (0-7): ");
            fflush(stdin);
            scanf("%d", &key);
            fc->getPad()->pushKey(FC_PAD_KEY(key), 0);
        }

        else if (ch=='g') {
            printf("input PC(16) to jump: ");
            fflush(stdin);
            scanf("%x", &cpu->PC);
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
            cpu->RES = 1;
            c = 0;
        }
        else if (ch=='n') {
            ppu->startNewFrame();
        }
        else if (ch=='o') {
            ppu->oneFrameOver();
        }
        else if (ch=='d') {
            fc->drawFrame();
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
        else if (ch=='x') {
            break;
        }
    }

    clock_t e = clock();
    cpu->showDebug(0);

    printf("执行 %d 条指令使用了 %lf 毫秒\n", c,
           (double)(e - s)*(CLOCKS_PER_SEC/1000));
}
