/*---------------------------------------------------------------------------------

  This is file derived from:

	default ARM7 core

		Copyright (C) 2005 - 2010
		Michael Noland (joat)
		Jason Rogers (dovoto)
		Dave Murphy (WinterMute)

	This software is provided 'as-is', without any express or implied
	warranty.  In no event will the authors be held liable for any
	damages arising from the use of this software.

	Permission is granted to anyone to use this software for any
	purpose, including commercial applications, and to alter it and
	redistribute it freely, subject to the following restrictions:

	1.	The origin of this software must not be misrepresented; you
		must not claim that you wrote the original software. If you use
		this software in a product, an acknowledgment in the product
		documentation would be appreciated but is not required.

	2.	Altered source versions must be plainly marked as such, and
		must not be misrepresented as being the original software.

	3.	This notice may not be removed or altered from any source
		distribution.

---------------------------------------------------------------------------------*/
#include <nds.h>
#include "common.h"
#include <stdlib.h>
#include <string.h>
#include "data.h"

u8 extraData[0x100];

volatile SharedData* sharedData;

void VblankHandler(void) {
    //Wifi_Update();
}

void VcountHandler() {
    inputGetAndSend();
}

volatile bool exitflag = false;

void powerButtonCB() {
    exitflag = true;
}

//---------------------------------------------------------------------------------
void readUserSettingsMine() {
//---------------------------------------------------------------------------------

	PERSONAL_DATA slot1;
	PERSONAL_DATA slot2;

	short slot1count, slot2count;
	short slot1CRC, slot2CRC;

	uint32 userSettingsBase;
	readFirmware( 0x20, &userSettingsBase,2);
	
	uint32 slot1Address = userSettingsBase * 8;
	uint32 slot2Address = userSettingsBase * 8 + 0x100;
	
	readFirmware( slot1Address , &slot1, sizeof(PERSONAL_DATA));
	readFirmware( slot2Address , &slot2, sizeof(PERSONAL_DATA));
	readFirmware( slot1Address + 0x70, &slot1count, 2);
	readFirmware( slot2Address + 0x70, &slot2count, 2);
	readFirmware( slot1Address + 0x72, &slot1CRC, 2);
	readFirmware( slot2Address + 0x72, &slot2CRC, 2);

	// default to slot 1 user Settings
	void *currentSettings = &slot1;
	
	short calc1CRC = swiCRC16( 0xffff, &slot1, 0x70);
	short calc2CRC = swiCRC16( 0xffff, &slot2, 0x70);

	// bail out if neither slot is valid
	if ( calc1CRC != slot1CRC && calc2CRC != slot2CRC) {
        sharedData->currentSlot = 3;
        sharedData->needed1 = calc1CRC;
        sharedData->needed2 = calc2CRC;
        sharedData->crc1 = slot1CRC;
        sharedData->crc2 = slot2CRC;
        sharedData->help1 = swiCRC16( 0xffff, &slot1, 0x70);
        sharedData->help2 = swiCRC16( 0xffff, &slot2, 0x70);
        return;
    }
	
	// if both slots are valid pick the most recent
	if ( calc1CRC == slot1CRC && calc2CRC == slot2CRC ) { 
		currentSettings = (slot2count == (( slot1count + 1 ) & 0x7f) ? &slot2 : &slot1);
	} else {
		if ( calc2CRC == slot2CRC )
			currentSettings = &slot2;
	}
	
    if (currentSettings == (void*)&slot1)
        sharedData->currentSlot = 1;
    else
        sharedData->currentSlot = 2;

	memcpy ( PersonalData, currentSettings, sizeof(PERSONAL_DATA));
}

void writeEnable() {
    // Write enable command
    REG_SPICNT = SPI_ENABLE | SPI_BYTE_MODE | SPI_CONTINUOUS | SPI_DEVICE_FIRMWARE;
    REG_SPIDATA = FIRMWARE_WREN;
	while (REG_SPICNT & SPI_BUSY);
    REG_SPICNT = 0;
}
//---------------------------------------------------------------------------------
void writeFirmware(u32 address, void * src, u32 size) {
//---------------------------------------------------------------------------------
	int oldIME=enterCriticalSection();
	u8 *buffer = src;

    while (REG_SPICNT & SPI_BUSY);

    writeEnable();

	// Write command
	REG_SPICNT = SPI_ENABLE | SPI_BYTE_MODE | SPI_CONTINUOUS | SPI_DEVICE_FIRMWARE;
	REG_SPIDATA = FIRMWARE_PW;
	while (REG_SPICNT & SPI_BUSY);

	// Set the address
	REG_SPIDATA = (address>>16) & 0xFF;
	while (REG_SPICNT & SPI_BUSY);
	REG_SPIDATA = (address>>8) & 0xFF;
	while (REG_SPICNT & SPI_BUSY);
	REG_SPIDATA = (address) & 0xFF;
	while (REG_SPICNT & SPI_BUSY);

	u32 i;
	// Write the data
	for(i=0;i<size;i++) {
		REG_SPIDATA = buffer[i];
		while (REG_SPICNT & SPI_BUSY);
	}

	REG_SPICNT = 0;
	leaveCriticalSection(oldIME);
}

//---------------------------------------------------------------------------------
int main() {
    ledBlink(0);

    irqInit();
    // Start the RTC tracking IRQ
    initClockIRQ();

    fifoInit();

    while (!fifoCheckValue32(FIFO_USER_03));
    sharedData = (volatile SharedData*)(fifoGetValue32(FIFO_USER_03) | 0x02000000);
    readUserSettingsMine();
    //sharedData->currentSlot = aoeuSlot;

    //---------------------------------------------------------------------------------
    // Find the slot being used

	PERSONAL_DATA slot1;
	PERSONAL_DATA slot2;

	short slot1count, slot2count;
	short slot1CRC, slot2CRC;

	uint32 userSettingsBase;
	readFirmware( 0x20, &userSettingsBase,2);
	
	uint32 slot1Address = userSettingsBase * 8;
	uint32 slot2Address = userSettingsBase * 8 + 0x100;
	
	readFirmware( slot1Address , &slot1, sizeof(PERSONAL_DATA));
	readFirmware( slot2Address , &slot2, sizeof(PERSONAL_DATA));
	readFirmware( slot1Address + 0x70, &slot1count, 2);
	readFirmware( slot2Address + 0x70, &slot2count, 2);
	readFirmware( slot1Address + 0x72, &slot1CRC, 2);
	readFirmware( slot2Address + 0x72, &slot2CRC, 2);

	// default to slot 1 user Settings
    u32 currentSettings = slot1Address;
	
	short calc1CRC = swiCRC16( 0xffff, &slot1, sizeof(PERSONAL_DATA));
	short calc2CRC = swiCRC16( 0xffff, &slot2, sizeof(PERSONAL_DATA));

	// bail out if neither slot is valid
	//if ( calc1CRC != slot1CRC && calc2CRC != slot2CRC) return;
	
	// if both slots are valid pick the most recent
	if ( calc1CRC == slot1CRC && calc2CRC == slot2CRC ) { 
		currentSettings = (slot2count == (( slot1count + 1 ) & 0x7f) ? slot2Address : slot1Address);
	} else {
		if ( calc2CRC == slot2CRC )
			currentSettings = slot2Address;
	}

    SetYtrigger(80);

    //installWifiFIFO();
    installSoundFIFO();

    installSystemFIFO();

    irqSet(IRQ_VCOUNT, VcountHandler);
    irqSet(IRQ_VBLANK, VblankHandler);

    irqEnable( IRQ_VBLANK | IRQ_VCOUNT | IRQ_NETWORK);

    setPowerButtonCB(powerButtonCB);   

    memset(extraData, 0xaa, 0x100);

    while (!exitflag) {
        if (sharedData->writePersonalData == 1) {
            int i;
            for (i=0; i<2; i++) {
                u32 address = 0x1fe00 + i*0x100;

                u16 crc = swiCRC16(0xffff, data+0x200+i*0x100, 0x70);
                u16 crc2 = swiCRC16(0xffff, data+0x200+i*0x100+0x74, 0x8a);
                if (i == 1) {
                    data[0x72+0x200+i*0x100] = crc&0xff;
                    data[0x73+0x200+i*0x100] = crc>>8;

                    data[0xfe + 0x200+i*0x100] = crc2&0xff;
                    data[0xff + 0x200+i*0x100] = crc2>>8;
                }
            }
            data[0x70+0x300] = ((data[0x70+0x200] + 1)&0x7f);
            data[0x71+0x300] = 0;
            int offset=0;
            int j;
            for (j=0; j<3; j++) {
                i = -1;
                for (i=j*0x100; i<(j+1)*0x100; i++) {
                    if ((data[i] | (data[i+1]<<8)) == 0x6b28) {
                        offset = i;
                        break;
                    }
                }
                if (i == -1)
                    continue;
                int newVal = -(offset-0x2d0)+0x36b28-0x100;
                data[offset] = newVal&0xff;
                data[offset+1] = (newVal>>8)&0xff;
                data[offset+2] = (newVal>>16)&0xff;
                data[offset+3] = (newVal>>24)&0xff;
            }
            /*
            for (i=0; i<30; i++)
                swiWaitForVBlank();
            writeFirmware(0x1fc00, data+0x000, 0x100);
            for (i=0; i<30; i++)
                swiWaitForVBlank();
            writeFirmware(0x1fd00, data+0x100, 0x100);
            for (i=0; i<30; i++)
                swiWaitForVBlank();

            writeFirmware(0x1f400, extraData, 0x100);
            for (i=0; i<30; i++)
                swiWaitForVBlank();
            writeFirmware(0x1f500, extraData, 0x100);
            for (i=0; i<30; i++)
                swiWaitForVBlank();
            writeFirmware(0x1f600, extraData, 0x100);
            for (i=0; i<30; i++)
                swiWaitForVBlank();
            writeFirmware(0x1f700, extraData, 0x100);
            for (i=0; i<30; i++)
                swiWaitForVBlank();
            writeFirmware(0x1f800, extraData, 0x100);
            for (i=0; i<30; i++)
                swiWaitForVBlank();
            writeFirmware(0x1f900, extraData, 0x100);
            for (i=0; i<30; i++)
                swiWaitForVBlank();
            writeFirmware(0x1fa00, extraData, 0x100);
            for (i=0; i<30; i++)
                swiWaitForVBlank();
            writeFirmware(0x1fb00, extraData, 0x100);
            for (i=0; i<30; i++)
                swiWaitForVBlank();
            writeFirmware(0x1fc00, extraData, 0x100);
            for (i=0; i<30; i++)
                swiWaitForVBlank();
            writeFirmware(0x1fd00, extraData, 0x100);
            for (i=0; i<30; i++)
                swiWaitForVBlank();
                */
            writeFirmware(0x1fe00, data+0x200, 0x100);
            for (i=0; i<30; i++)
                swiWaitForVBlank();
            writeFirmware(0x1ff00, data+0x300, 0x100);
            sharedData->writePersonalData = false;
        }
        else if (sharedData->writePersonalData == 2) {
            u8 buffer[0x100];
            memset(buffer, 0xff, 0x100);
            memcpy(buffer, PersonalData, 0x70);
            u16 crc = 0;
            buffer[0x72] = crc&0xff;
            buffer[0x73] = crc>>8;
            buffer[0x70] = 0;
            buffer[0x71] = 0;
            writeFirmware(slot1Address, buffer, 0x100);
            writeFirmware(slot2Address, buffer, 0x100);

            sharedData->writePersonalData = false;
        }
        if ( 0 == (REG_KEYINPUT & (KEY_SELECT | KEY_START | KEY_L | KEY_R))) {
            exitflag = true;
        }
        swiWaitForVBlank();
    }
    return 0;
}
