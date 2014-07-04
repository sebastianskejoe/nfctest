#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "NFC.h"

#define HDR_LENGTH 6

#ifndef TEST
#include <Arduino.h>
void setDebugLCD(LiquidCrystal *lcd) {
    _dbgLCD = lcd;
}
#endif

void requestHeader(unsigned char *request, unsigned char length) {
    request[0] = 0x00;
    request[1] = 0x00;
    request[2] = 0xFF;
    request[3] = length+1;
    request[4] = 0xFF-length;
    request[5] = 0xD4;
}

unsigned char requestDCS(unsigned char *data, unsigned char length) {
    unsigned char ret = 0xFF;
    int i;
    ret -= 0xD4;
    for (i = 0; i < length; i++) {
        ret -= data[i];
    }
    ret += 1;
    return ret;
}

void sendRequest(char *name, unsigned char code, char *signature, ...) {
    int i;
    unsigned char length = 0, u;
    unsigned char *array;
    va_list args;

    unsigned char requestdata[255];
    unsigned char data[255];
    memset(requestdata, 0, 255);
    memset(data, 0, 255);

    // Clear display for debugging
    DBG(name);

    va_start(args, signature);
    data[length] = code;
    length += sizeof(code);
    for (i = 0; i < strlen(signature); i++) {
        switch(signature[i]) {
        case 'u':
            u = (unsigned char)va_arg(args, int);
            data[length] = u;
            length += sizeof(u);
            break;
        case 'a':
            array = va_arg(args, unsigned char*);
            if(array != 0) {
                memcpy(&data[length], array, strlen((const char*)array));
                length += strlen((const char *)array);
            }
            break;
        }
    }
    va_end(args);

    // Create request header in requestData
    requestHeader(requestdata, length);

    // Copy data to requestData and add DCS
    memcpy(&requestdata[HDR_LENGTH], &data[0], length);
    requestdata[HDR_LENGTH+length] = requestDCS(data, length);

    // Write data to serial
    for (i = 0; i < length+HDR_LENGTH+1; i++) {
        DBGHEX(requestdata[i]);
        #ifndef TEST
        Serial.write(requestdata[i]);
        #endif
    }
    #ifndef TEST
    Serial.flush();
    #endif
}

void * nfcReadResponse(char *signature, ...) {

    unsigned char *buf = (unsigned char *)malloc(255);
    unsigned char cur, last = 0xFF;
    unsigned char keepalive = 20;

    // Frame data
    unsigned char len, lcs, tfi, code, dcs;
    unsigned char acked = 0;

    // For constructing a return array
    va_list vargs;
    unsigned char a, i, n, siglen = strlen(signature), sum = 0, el, pos = 0;
    unsigned char *ret = (unsigned char *)malloc(siglen*4), *addr;

    // Special codes
    char ACK[2] = {0xFF, 0x00};
    char NACK[2] = {0x00, 0x00};

    // Read until we get a frame
    va_start(vargs, signature);
    while (true) {
        cur = getByte();

        if (!keepalive) {
            DBG("kill");
            break;
        }

        // Check if it is the start of a frame
        if (last == 0x00 && cur == 0xFF) {

            // Get the length of the packet
            len = getByte();
            
            // Check for special cases
            switch (len) {
            // ACK
            case 0x00:
                // Check if we have a 0xFF 0x00 next
                getBytes(buf, 2);
                int res;
                if (memcmp(buf, ACK, 2) == 0) {
                    acked = 1;
                }

                break;

            // NACK or extended
            case 0xFF:
                getBytes(buf, 2);

                // Is it a NACK
                if (memcmp(buf, NACK, 2)) {
                    free(buf);
                    va_end(vargs);

                    memset(ret, 0, siglen*4);
                    return ret;
                } else if (buf[0] == 0xFF) {
                    // It is an extended frame
                    //return readExtendedFrame(buf[1]);
                }
                break;

            // A normal frame
            default:
                lcs = getByte();

                // Make sure it is a response
                tfi = getByte();
                if (tfi != 0xD5) {
                    return 0;
                }

                code = getByte();

                // Read data
                getBytes(buf, len-2);

                // Data check sum
                dcs = getByte();

                // Empty postamble
                getByte();

                // Get length of argsa
                for (i = 0; i < siglen; i++) {
                    switch (signature[i]) {
                    case 'u':
                        ret[sum] = buf[pos];
                        sum += 1;
                        pos += 1;
                        break;
                    case 'a':
                        addr = va_arg(vargs, unsigned char*);

                        // Last argument
                        if (i == siglen-1) {
                            el = len-2-pos;
                        }

                        // Check if size is specified
                        for (a = 1; a < siglen-i; i++) {
                            n = signature[i+a];

                            // If n isn't a number, length is specified
                            if (n < '0' || n > '9') {
                                break;
                            }

                            el *= 10;
                            el += n-'0';
                        }

                        // No size specified and not last argument
                        // Guess that length is remaining length - remaining number of args
                        // This assumes that there are no more arrays
                        if (el == 0) {
                            el = len-2-(siglen-1-i)-pos;
                        }

                        // Copy it
                        memcpy(addr, &buf[pos], el);
                        sum += sizeof(unsigned char*);
                        pos += el;

                        break;
                    }
                }
                ret[sum] = acked;

                va_end(vargs);
                free(buf);
                return ret;
                break;
            }
        }

        last = cur;
        keepalive--;
    }

    free(buf);

    memset(ret, 0, siglen*4);
    return ret;
}

unsigned char getByte() {
    #ifdef TEST
    return test_response[test_resp_i++];
    #else
    delay(100);
    return Serial.read();
    #endif
}

void getBytes(unsigned char *buf, int n) {
    int i;
    for (i = 0; i < n; i++) {
        buf[i] = getByte();
    }
}

// BEGIN generated code
DiagnoseResponse * Diagnose(unsigned char NumTst, unsigned char * InParam) {
    sendRequest((char *)"Diagnose", 0x00, (char *)"ua", NumTst, InParam);

    #ifndef TEST
    delay(100);
    #endif
    
    unsigned char *aOutParam = (unsigned char *)malloc(255);
    DiagnoseResponse *resp = (DiagnoseResponse *)nfcReadResponse((char *)"a", aOutParam);
    
    resp->OutParam = aOutParam;
    

    #ifndef TEST
    _dbgLCD->setCursor(0,1);
    _dbgLCD->print("foo");
    delay(500);
    #endif
    return resp;
}

GetFirmwareVersionResponse * GetFirmwareVersion() {
    sendRequest((char *)"GetFirmwareVersion", 0x02, (char *)"");

    #ifndef TEST
    delay(100);
    #endif
    
    GetFirmwareVersionResponse *resp = (GetFirmwareVersionResponse *)nfcReadResponse((char *)"uuuu");
    

    #ifndef TEST
    _dbgLCD->setCursor(0,1);
    _dbgLCD->print("foo");
    delay(500);
    #endif
    return resp;
}

GetGeneralStatusResponse * GetGeneralStatus() {
    sendRequest((char *)"GetGeneralStatus", 0x04, (char *)"");

    #ifndef TEST
    delay(100);
    #endif
    
    unsigned char *aTgData = (unsigned char *)malloc(255);
    GetGeneralStatusResponse *resp = (GetGeneralStatusResponse *)nfcReadResponse((char *)"uuuau", aTgData);
    
    resp->TgData = aTgData;
    

    #ifndef TEST
    _dbgLCD->setCursor(0,1);
    _dbgLCD->print("foo");
    delay(500);
    #endif
    return resp;
}

ReadRegisterResponse * ReadRegister(unsigned char * Adr) {
    sendRequest((char *)"ReadRegister", 0x06, (char *)"a", Adr);

    #ifndef TEST
    delay(100);
    #endif
    
    unsigned char *aVal = (unsigned char *)malloc(255);
    ReadRegisterResponse *resp = (ReadRegisterResponse *)nfcReadResponse((char *)"a", aVal);
    
    resp->Val = aVal;
    

    #ifndef TEST
    _dbgLCD->setCursor(0,1);
    _dbgLCD->print("foo");
    delay(500);
    #endif
    return resp;
}

void WriteRegister(unsigned char * AdrVal) {
    sendRequest((char *)"WriteRegister", 0x08, (char *)"a", AdrVal);
}

ReadGPIOResponse * ReadGPIO() {
    sendRequest((char *)"ReadGPIO", 0x0C, (char *)"");

    #ifndef TEST
    delay(100);
    #endif
    
    ReadGPIOResponse *resp = (ReadGPIOResponse *)nfcReadResponse((char *)"uuu");
    

    #ifndef TEST
    _dbgLCD->setCursor(0,1);
    _dbgLCD->print("foo");
    delay(500);
    #endif
    return resp;
}

void WriteGPIO(unsigned char P3, unsigned char P7) {
    sendRequest((char *)"WriteGPIO", 0x0E, (char *)"uu", P3, P7);
}

void SetSerialBaudRate(unsigned char BR) {
    sendRequest((char *)"SetSerialBaudRate", 0x10, (char *)"u", BR);
}

void SetParameters(unsigned char Flags) {
    sendRequest((char *)"SetParameters", 0x12, (char *)"u", Flags);
}

void SAMConfiguration(unsigned char Mode, unsigned char Timeout, unsigned char IRQ) {
    sendRequest((char *)"SAMConfiguration", 0x14, (char *)"uuu", Mode, Timeout, IRQ);
}

PowerDownResponse * PowerDown(unsigned char WakeUpEnable, unsigned char GenerateIRQ) {
    sendRequest((char *)"PowerDown", 0x16, (char *)"uu", WakeUpEnable, GenerateIRQ);

    #ifndef TEST
    delay(100);
    #endif
    
    PowerDownResponse *resp = (PowerDownResponse *)nfcReadResponse((char *)"u");
    

    #ifndef TEST
    _dbgLCD->setCursor(0,1);
    _dbgLCD->print("foo");
    delay(500);
    #endif
    return resp;
}

void RFConfiguration(unsigned char CfgItem, unsigned char * ConfigurationData) {
    sendRequest((char *)"RFConfiguration", 0x32, (char *)"ua", CfgItem, ConfigurationData);
}

void RFRegulationTest(unsigned char TxMode) {
    sendRequest((char *)"RFRegulationTest", 0x58, (char *)"u", TxMode);
}

InJumpForDEPResponse * InJumpForDEP(unsigned char ActPass, unsigned char BR, unsigned char Next, unsigned char * PassiveInitiatorData, unsigned char * NFCID3i, unsigned char * Gi) {
    sendRequest((char *)"InJumpForDEP", 0x56, (char *)"uuuaaa", ActPass, BR, Next, PassiveInitiatorData, NFCID3i, Gi);

    #ifndef TEST
    delay(100);
    #endif
    
    unsigned char *aNFCID3t = (unsigned char *)malloc(10);
    unsigned char *aGt = (unsigned char *)malloc(255);
    InJumpForDEPResponse *resp = (InJumpForDEPResponse *)nfcReadResponse((char *)"uua10uuuuua", aNFCID3t, aGt);
    
    resp->NFCID3t = aNFCID3t;
    
    resp->Gt = aGt;
    

    #ifndef TEST
    _dbgLCD->setCursor(0,1);
    _dbgLCD->print("foo");
    delay(500);
    #endif
    return resp;
}

InJumpForPSLResponse * InJumpForPSL(unsigned char ActPass, unsigned char BR, unsigned char Next, unsigned char * PassiveInitiatorData, unsigned char * NFCID3i, unsigned char * Gi) {
    sendRequest((char *)"InJumpForPSL", 0x46, (char *)"uuuaaa", ActPass, BR, Next, PassiveInitiatorData, NFCID3i, Gi);

    #ifndef TEST
    delay(100);
    #endif
    
    unsigned char *aNFCID3t = (unsigned char *)malloc(10);
    unsigned char *aGt = (unsigned char *)malloc(255);
    InJumpForPSLResponse *resp = (InJumpForPSLResponse *)nfcReadResponse((char *)"uua10uuuuua", aNFCID3t, aGt);
    
    resp->NFCID3t = aNFCID3t;
    
    resp->Gt = aGt;
    

    #ifndef TEST
    _dbgLCD->setCursor(0,1);
    _dbgLCD->print("foo");
    delay(500);
    #endif
    return resp;
}

InListPassiveTargetResponse * InListPassiveTarget(unsigned char MaxTg, unsigned char BrTy, unsigned char * InitiatorData) {
    sendRequest((char *)"InListPassiveTarget", 0x4A, (char *)"uua", MaxTg, BrTy, InitiatorData);

    #ifndef TEST
    delay(100);
    #endif
    
    unsigned char *aTargetData = (unsigned char *)malloc(255);
    InListPassiveTargetResponse *resp = (InListPassiveTargetResponse *)nfcReadResponse((char *)"ua", aTargetData);
    
    resp->TargetData = aTargetData;
    

    #ifndef TEST
    _dbgLCD->setCursor(0,1);
    _dbgLCD->print("foo");
    delay(500);
    #endif
    return resp;
}

// END generated code
