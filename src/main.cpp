#include "../include/SystemClass.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow) {
    SystemClass* System;
    bool result;

    System = new SystemClass;

    result = System->initialize();

    if (result) {
        System->run();
    }

    System->shutdown();
    delete System;
    System = 0;

    return 0;
}