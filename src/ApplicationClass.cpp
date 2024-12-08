#include "../include/ApplicationClass.h"

ApplicationClass::ApplicationClass() {
    m_Direct3D = 0;
}

ApplicationClass::ApplicationClass(const ApplicationClass& other) {
}

ApplicationClass::~ApplicationClass() {
}

bool ApplicationClass::initialize(int screenWidth, int screenHeight, HWND hwnd) {
    bool result;

    m_Direct3D = new D3DClass;

    result = m_Direct3D->initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH,
                                    SCREEN_NEAR);
    if (!result) {
        MessageBox(hwnd, L"Could not initialize Direct3D", L"Error", MB_OK);
        return false;
    }

    return true;
}

void ApplicationClass::shutdown() {
    if (m_Direct3D) {
        m_Direct3D->shutdown();
        delete m_Direct3D;
        m_Direct3D = 0;
    }
}

bool ApplicationClass::frame() {
    bool result;

    result = render();
    if (!result) {
        return false;
    }

    return true;
}

long long int _frame = 0;
float red            = 0.5f;
float green          = 0.5f;
float blue           = 0.5f;

bool ApplicationClass::render() {
    _frame++;


    if (_frame % 500 == 0) {
        red   = (float)(rand()) / (float)(RAND_MAX);
        green = (float)(rand()) / (float)(RAND_MAX);
        blue  = (float)(rand()) / (float)(RAND_MAX);
    }

    //m_Direct3D->begin_scene(0.5f, 0.5f, 0.5f, 1.0f);
    m_Direct3D->begin_scene(red, green, blue, 1.0f);

    m_Direct3D->end_scene();
    return true;
}
