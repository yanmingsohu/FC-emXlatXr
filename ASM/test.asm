; Displays a message on the screen. Demonstrates how to set up PPU
; and nametable

.segment "HEADER"
 .byte "NES", 26, 2, 1 
; CHR ROM data
.segment "CHARS"

.segment "VECTORS"
 .word 0, 0, 0, nmi, reset, irq 

.segment "STARTUP"

.segment "CODE" 

; ===============================================
; NES Registers
; ===============================================
PPUCTRL = $2000 ; 这两个寄存器用来控制PPU的各种行为
PPUMASK = $2001
PPUSTATUS = $2002 ; 用来读取PPU当前状态
PPUSCROLL = $2005 ; 设置背景卷轴的X/Y坐标
PPUADDR = $2006 ; 设置PPU中的VRAM地址
PPUDATA = $2007 ; 往当前VRAM地址中写入数据

; ===============================================
; 程序入口 - 开机或Reset的时候会跳到这里
; ===============================================
reset:
 ; 初始化NES硬件
 ldx #$FF ; 重置栈顶指针到$FF (255)
 txs
 sei ; 禁用IRQ中断
 lda #0
 sta PPUCTRL ; 关闭NMI(将PPUCTRL置零)
 sta PPUMASK ; 关闭PPU渲染(将PPUMASK置零)
 
 ; 等待PPU预热（一共等待2次，第一次VBlank发生当作预热完成，第二次才正式当作VBlank）
 @wait1: bit PPUSTATUS ; 循环等待PPUSTATUS最高位置位(位的高低从右到左是从低到高，最高位即最左端D7，一旦置位，表示VBlank发生)
 bpl @wait1
 ; 读取PPUSTATUS也会清除最高位
 ; 所以到这里D7位已经清0了
 @wait2: bit PPUSTATUS ; 再次等待PPUSTATUS的D7置位
 bpl @wait2
 
 ; 设置头四个调色板
 lda #$3F ; 设置PPU地址为调色板RAM（$3F00）
 sta PPUADDR
 lda #0
 sta PPUADDR
 lda #$51 ; 设置背景色为黑色
 sta PPUDATA
 lda #$FF ; 设置3个前景色为白色
 sta PPUDATA
 sta PPUDATA
 sta PPUDATA
 
 ; 显示前等待VBlank
 bit PPUSTATUS
 @wait3: bit PPUSTATUS
 bpl @wait3
 
 ; 启用背景显示
 lda #%00001000 ; 启用背景
 sta PPUMASK
 lda #0 ; 滚动背景到最左上角(即$2000处的nametable)
 sta PPUCTRL
 sta PPUSCROLL
 sta PPUSCROLL
 
 ; 不断循环(什么都不做，保持当前PPU状态，不断显示蓝色背景)
 forever:
 jmp forever
  

; ===============================================
; 中断处理
; ===============================================
irq:
 rti
nmi:
 rti