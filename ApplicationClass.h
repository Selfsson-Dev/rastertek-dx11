#pragma once
#include "D3DClass.h"

constexpr bool FULL_SCREEN   = false;
constexpr bool VSYNC_ENABLED = true;
constexpr float SCREEN_DEPTH = 1000.0f;
constexpr float SCREEN_NEAR  = 0.3f;

class ApplicationClass
{
public:
    ApplicationClass();
    ApplicationClass(const ApplicationClass&);
    ~ApplicationClass();

    bool initialize(int, int, HWND);
    void shutdown();
    bool frame();

private:
    bool render();

    D3DClass* m_Direct3D;
};
