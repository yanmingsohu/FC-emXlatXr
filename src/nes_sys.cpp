#include "nes_sys.h";


NesSystem::NesSystem() {
    mmc = new MMC();
    ram = new memory(mmc);
    cpu = new cpu_6502(ram);
}

NesSystem::~NesSystem() {
}
