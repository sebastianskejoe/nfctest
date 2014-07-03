/* 

PN532 reads the tag by Arduino mega/Leonardo
command list:

#wake up reader
send: 55 55 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ff 03 fd d4 14 01 17 00
return: 00 00 FF 00 FF 00 00 00 FF 02 FE D5 15 16 00

#get firmware
send: 00 00 FF 02 FE D4 02 2A 00
return: 00 00 FF 00 FF 00 00 00 FF 06 FA D5 03 32 01 06 07 E8 00

#read the tag
send: 00 00 FF 04 FC D4 4A 01 00 E1 00
return: 00 00 FF 00 FF 00 00 00 FF 0C F4 D5 4B 01 01 00 04 08 04 XX XX XX XX 5A 00
XX is tag.

*/

const unsigned char wake[24]={0x55,0x55,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0x03,0xfd,0xd4,0x14,0x01,0x17,0x00};//wake up NFC module
const unsigned char firmware[9]={0x00,0x00,0xFF,0x02,0xFE,0xD4,0x02,0x2A,0x00};//
const unsigned char tag[11]={0x00,0x00,0xFF,0x04,0xFC,0xD4,0x4A,0x01,0x00,0xE1,0x00};//detecting tag command

unsigned char receive_ACK[25];//Command receiving buffer

#include "Arduino.h"
#include <NFC.h>
#include <LiquidCrystal.h>
#include <string.h>

LiquidCrystal LCD(5,6,9,10,11,12);

#define BUTTON_PIN 3

void setup()
{
    // Setup LCD and serial
    LCD.begin(16,2);
    LCD.clear();
    LCD.print("NFC test v0.0!");
    Serial.begin(115200);
    setDebugLCD(&LCD);

    pinMode(BUTTON_PIN, INPUT);
}

int state = 0;

void loop() {
    int bs = digitalRead(BUTTON_PIN), i;
    char *buf;
    GetFirmwareVersionResponse *r;
    InListPassiveTargetResponse *pt;
    if (bs == HIGH) {
        switch (state) {
        case 0:
            wake_card();
            delay(100);
            array_ACK(15);
            dsplay(15);
            break;
        case 1:
            r = GetFirmwareVersion();
            LCD.clear();
            LCD.print("Version: ");
            LCD.print(r->Ver);
            LCD.print(".");
            LCD.print(r->Rev);

            free(r);
            break;
        default:
            pt = InListPassiveTarget(2,0,0);
            LCD.clear();
            LCD.print("Targets: ");

            LCD.print(pt->NbTg);
            delay(500);

            LCD.setCursor(0,1);
            for (i = 0; i < pt->NbTg; i++) {
                LCD.print(pt->TargetData[i*9+5], HEX);
                LCD.print(" ");
                LCD.print(pt->TargetData[i*9+6], HEX);
                LCD.print(" ");
                LCD.print(pt->TargetData[i*9+7], HEX);
                LCD.print(" ");
                LCD.print(pt->TargetData[i*9+8], HEX);
                LCD.print(" ");
            }

            free(pt->TargetData);
            free(pt);
            break;
        }
        state++;
        delay(50);
    }
}

void Send_Byte(unsigned char command_data)
{
  Serial.write(command_data);// command send to device
  Serial.flush();// complete the transmission of outgoing serial data 
} 

void array_ACK(unsigned char temp)
{
    for(unsigned char i=0;i<temp;i++)                                           
    {
       receive_ACK[i]= Serial.read();
       delay(100);
    }
}

void wake_card(void)
{
    unsigned char i;
    for(i=0;i<24;i++) //send command
        Send_Byte(wake[i]);
}

void firmware_version(void)
{
    unsigned char i;
    for(i=0;i<9;i++) //send command
    Send_Byte(firmware[i]);
}

void read_tag(void)
{
    unsigned char i;
    for(i=0;i<11;i++) //send command
    Send_Byte(tag[i]);
}

void dsplay(unsigned char tem)
{
    LCD.clear();
    LCD.setCursor(0,0);
    unsigned char i;
    for(i=0;i<tem;i++) {//send command
      if (i == 7) {
        LCD.setCursor(0,1);
      }
      LCD.print(receive_ACK[i], HEX);
    }
    LCD.setCursor(0,0);      
}

