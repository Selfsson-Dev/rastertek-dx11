#include "SystemClass.h"

SystemClass::SystemClass() {
    m_application = 0;
    m_input       = 0;
}

SystemClass::SystemClass(const SystemClass& other) {
    // we dont want this!
}

SystemClass::~SystemClass() {
    // use shutdown instead
}

bool SystemClass::initialize() {
    int screenWidth, screenHeight;
    bool result;

    screenWidth  = 0;
    screenHeight = 0;

    // start windows api
    initializeWindows(screenWidth, screenHeight);

    m_input = new InputClass;
    m_input->initialize();

    m_application = new ApplicationClass;

    result = m_application->initialize(screenWidth, screenHeight, m_hwnd);
    if (!result) {
        return false;
    }

    return true;
}

void SystemClass::shutdown() {
    // maybe change to assert instead?

    if (m_application) {
        m_application->shutdown();
        delete m_application;
        m_application = 0;
    }

    if (m_input) {
        delete m_input;
        m_input = 0;
    }

    shutdownWindows();
}

/*
*	while not done
*		check for Windows system messages
*		process system messages
*		process application loop
*		check if user wanted to quit during the frame processing
*/

void SystemClass::run() {
    MSG msg;
    bool done, result;

    ZeroMemory(&msg, sizeof(MSG));

    done = false;
    while (!done) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            // TODO UNDERSTAND THIS
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (msg.message == WM_QUIT) {
            done = true;
        }
        else {
            result = frame();
            if (!result) {
                done = true;
            }
        }
    }
}

bool SystemClass::frame() {
    bool result;

    if (m_input->is_key_down(VK_ESCAPE)) {
        return false;
    }

    result = m_application->frame(); // let application object do its frame processing
    if (!result) {
        return false;
    }

    return true;
}

/*	here we listen for windows system messages 
*	we are only interested in if we: 
*		press a key 
*		let go of a key.
*	Everything else is handled by default windows handler :) */

LRESULT CALLBACK SystemClass::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam) {
    switch (umsg) {
        case WM_KEYDOWN: {
            m_input->key_down(static_cast<unsigned int>(wparam));
            return 0;
        }

        case WM_KEYUP: {
            m_input->key_up(static_cast<unsigned int>(wparam));
            return 0;
        }

        default: {
            return DefWindowProc(hwnd, umsg, wparam, lparam);
        }
    }
}

void SystemClass::initializeWindows(int& screenWidth, int& screenHeight) {
    WNDCLASSEX wc;
    DEVMODE dmScreenSettings;
    int posX, posY;

    ApplicationHandle = this;
    m_hinstance       = GetModuleHandle(NULL);
    m_applicationName = L"Awesome Engine";

    // TODO UNDERSTAND PARAMETERS WE ARE SETTINGS HERE
    // default windows class settings
    wc.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc   = WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = m_hinstance;
    wc.hIcon         = LoadIcon(NULL, IDI_WINLOGO);
    wc.hIconSm       = wc.hIcon;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = m_applicationName;
    wc.cbSize        = sizeof(WNDCLASSEX);

    RegisterClassEx(&wc);

    // get clients resolution
    screenWidth  = GetSystemMetrics(SM_CXSCREEN);
    screenHeight = GetSystemMetrics(SM_CYSCREEN);

    if (FULL_SCREEN) {
        memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
        dmScreenSettings.dmSize       = sizeof(dmScreenSettings);
        dmScreenSettings.dmPelsWidth  = static_cast<unsigned long>(screenWidth);
        dmScreenSettings.dmPelsHeight = static_cast<unsigned long>(screenHeight);
        dmScreenSettings.dmBitsPerPel = 32;
        dmScreenSettings.dmFields     = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

        // set to fullscreen
        ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

        posX = 0;
        posY = 0;
    }
    screenWidth  = 800;
    screenHeight = 600;

    posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth) / 2;
    posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;

    // create window handle with screen settings
    m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, m_applicationName, m_applicationName,
                            WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
                            posX, posY, screenWidth, screenHeight,
                            NULL, NULL, m_hinstance, NULL);

    // focus new window and bring it to front
    ShowWindow(m_hwnd, SW_SHOW);
    SetForegroundWindow(m_hwnd);
    SetFocus(m_hwnd);

    ShowCursor(false);
}

void SystemClass::shutdownWindows() {
    ShowCursor(true);

    // disable fullscreen
    if (FULL_SCREEN) {
        ChangeDisplaySettings(NULL, 0);
    }

    // remove window
    DestroyWindow(m_hwnd);
    m_hwnd = NULL;

    // remove the application instance
    UnregisterClass(m_applicationName, m_hinstance);
    m_hinstance = NULL;

    ApplicationHandle = NULL;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam) {
    switch (umessage) {
        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        }

        case WM_CLOSE: {
            PostQuitMessage(0);
            return 0;
        }

        default: {
            return ApplicationHandle->MessageHandler(hwnd, umessage, wparam, lparam);
        }
    }
}
