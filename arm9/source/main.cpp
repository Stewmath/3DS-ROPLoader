#include <nds.h>
#include <fat.h>
#include <stdio.h>
#include "common.h"

volatile SharedData* sharedData;

void processInput();

int main(void)
{
	//irqInit();
	irqEnable(IRQ_VBLANK);
    fatInitDefault();


    sharedData = (volatile SharedData*)memUncached(malloc(sizeof(SharedData)));
    sharedData->writePersonalData = false;
    sharedData->currentSlot = 0;
    //memset((void*)sharedData->data[0], 0, 0x100);
    //memset((void*)sharedData->data[1], 0, 0x100);
    // It might make more sense to use "fifoSendAddress" here.
    // However there may have been something wrong with it in dsi mode.
    fifoSendValue32(FIFO_USER_03, ((u32)sharedData)&0x00ffffff);


	consoleDemoInit();
	consoleDebugInit(DebugDevice_CONSOLE);
    char nickname[10];
    for (int i=0; i<7; i++)
        nickname[i] = PersonalData->name[i];
    nickname[7] = '\0';
    printf("Nickname is %s\n", nickname);
    printf("Press A to install hax\n");
    /*
    printf("%d\n", sizeof(PERSONAL_DATA));

    for (int i=0; i<10; i++)
        PersonalData->name[i] = 'B';
    PersonalData->nameLen = 10;
    */
    //PersonalData->stuff |= BIT(6); // Autoboot


    bool printed=false;
    while(1) {
        swiWaitForVBlank();
        processInput();
        if (!printed && sharedData->currentSlot != 0) {
            //printf("Using slot %d\n", sharedData->currentSlot);
            /*
            printf("CRC1: %.4x, wanted %.4x\n", sharedData->crc1, sharedData->needed1);
            printf("CRC2: %.4x, wanted %.4x\n", sharedData->crc2, sharedData->needed2);
            printf("%.4x, %.4x\n", sharedData->help1, sharedData->help2);
            */
            printed = true;
        }
    }

	return 0;
}

void processInput()
{
	scanKeys();

	int keys = keysDown();

    if ((keys & KEY_A) || (keys & KEY_B)) {
        sharedData->slot = 1;
        if (keys & KEY_A)
            sharedData->slot = 0;
        printf("Writing data...\n");
        sharedData->writePersonalData = 1;

        while (sharedData->writePersonalData);
        printf("Done.\n");
    }

    /*
    if (keys & KEY_X) {
        sharedData->writePersonalData = 2;
        printf("Nuked.\n");
    }
    */
}
