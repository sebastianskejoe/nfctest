#ifndef __NFC_H
#define __NFC_H

#ifndef TEST
#include <LiquidCrystal.h>
static LiquidCrystal *_dbgLCD = 0;
void setDebugLCD(LiquidCrystal *lcd);
#endif

#ifndef TEST
#define DBG(msg) if (_dbgLCD) _dbgLCD->print(msg);
#define DBGHEX(msg) if (_dbgLCD) _dbgLCD->print(msg, HEX);
#else
#define DBG(msg) printf("%s", msg);
#define DBGHEX(msg) printf("%x ", msg);
#endif

#ifdef TEST
static int test_resp_i = 0;
static char test_response[] = {0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0x00,
                       0x00, 0xFF, 0x06, 0xFA, 0xD5, 0x03, 0x32,
                       0x01, 0x06, 0x07, 0xE8, 0x00, 0x00, 0x00,
                       0xFF, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
                       0x0C, 0xF4, 0xD5, 0x4B, 0x01, 0x01, 0x00,
                       0x04, 0x08, 0x04, 0xFF, 0xFF, 0xFF, 0xFF,
                       0x5A, 0x00};
#endif

void requestHeader(unsigned char *request, unsigned char length);
unsigned char requestDCS(unsigned char *data, unsigned char length);
void sendRequest(char *name, unsigned char code, char *signature, ...);

void * nfcReadResponse(char * signature, ...);
unsigned char getByte();
void getBytes(unsigned char *buf, int n);

// BEGIN generated response structs

typedef struct { 
    unsigned char * OutParam;
    unsigned char acked;
} DiagnoseResponse;

typedef struct { 
    unsigned char IC;
    unsigned char Ver;
    unsigned char Rev;
    unsigned char Support;
    unsigned char acked;
} GetFirmwareVersionResponse;

typedef struct { 
    unsigned char Err;
    unsigned char Field;
    unsigned char NbTg;
    unsigned char * TgData;
    unsigned char SAMStatus;
    unsigned char acked;
} GetGeneralStatusResponse;

typedef struct { 
    unsigned char * Val;
    unsigned char acked;
} ReadRegisterResponse;

typedef struct { 
    unsigned char P3;
    unsigned char P7;
    unsigned char IOI1;
    unsigned char acked;
} ReadGPIOResponse;

typedef struct { 
    unsigned char Status;
    unsigned char acked;
} PowerDownResponse;

typedef struct { 
    unsigned char Status;
    unsigned char Tg;
    unsigned char * NFCID3t;
    unsigned char DIDt;
    unsigned char BSt;
    unsigned char BRt;
    unsigned char TO;
    unsigned char PPt;
    unsigned char * Gt;
    unsigned char acked;
} InJumpForDEPResponse;

typedef struct { 
    unsigned char Status;
    unsigned char Tg;
    unsigned char * NFCID3t;
    unsigned char DIDt;
    unsigned char BSt;
    unsigned char BRt;
    unsigned char TO;
    unsigned char PPt;
    unsigned char * Gt;
    unsigned char acked;
} InJumpForPSLResponse;

typedef struct { 
    unsigned char NbTg;
    unsigned char * TargetData;
    unsigned char acked;
} InListPassiveTargetResponse;

// END generated response structs


// BEGIN generated requests
DiagnoseResponse * Diagnose(unsigned char NumTst, unsigned char * InParam);
GetFirmwareVersionResponse * GetFirmwareVersion();
GetGeneralStatusResponse * GetGeneralStatus();
ReadRegisterResponse * ReadRegister(unsigned char * Adr);
void WriteRegister(unsigned char * AdrVal);
ReadGPIOResponse * ReadGPIO();
void WriteGPIO(unsigned char P3, unsigned char P7);
void SetSerialBaudRate(unsigned char BR);
void SetParameters(unsigned char Flags);
void SAMConfiguration(unsigned char Mode, unsigned char Timeout, unsigned char IRQ = 0x01);
PowerDownResponse * PowerDown(unsigned char WakeUpEnable, unsigned char GenerateIRQ);
void RFConfiguration(unsigned char CfgItem, unsigned char * ConfigurationData);
void RFRegulationTest(unsigned char TxMode);
InJumpForDEPResponse * InJumpForDEP(unsigned char ActPass, unsigned char BR, unsigned char Next, unsigned char * PassiveInitiatorData = 0, unsigned char * NFCID3i = 0, unsigned char * Gi = 0);
InJumpForPSLResponse * InJumpForPSL(unsigned char ActPass, unsigned char BR, unsigned char Next, unsigned char * PassiveInitiatorData = 0, unsigned char * NFCID3i = 0, unsigned char * Gi = 0);
InListPassiveTargetResponse * InListPassiveTarget(unsigned char MaxTg, unsigned char BrTy, unsigned char * InitiatorData);
// END generated requests
#endif