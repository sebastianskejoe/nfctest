#include "NFC.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    // GetFWVer
    GetFirmwareVersionResponse *gfw = GetFirmwareVersion();
    printf("Ver: %d, Rev: %d\n", gfw->Ver, gfw->Rev);
    free(gfw);

    InListPassiveTargetResponse *pt = InListPassiveTarget(2, 0, 0);
    printf("NbTg: %d\n", pt->NbTg);
    printf("Id: %x %x %x %x\n", pt->TargetData[5], pt->TargetData[6], pt->TargetData[7], pt->TargetData[8]);
    free(pt->TargetData);
    free(pt);
}
