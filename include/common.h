// This file can be included by arm7 and arm9.
#pragma once

typedef struct SharedData {
    u8 writePersonalData;
    u8 currentSlot;

    u16 needed1, needed2;
    u16 crc1, crc2;
    u16 help1, help2;
    u8 slot;
} SharedData;

extern volatile SharedData* sharedData;

extern bool __dsimode;
