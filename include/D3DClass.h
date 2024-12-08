#pragma once

// link libraries
#pragma comment(lib, "d3d11.lib")       // drawing lib
#pragma comment(lib, "dxgi.lib")        // hardware interface
#pragma comment(lib, "d3dcompiler.lib") // shader compiling

// d3d11.h contains windows.h
#include <d3d11.h>
#include <DirectXMath.h>
using namespace DirectX;

class D3DClass
{
public:
    D3DClass();
    D3DClass(const D3DClass&);
    ~D3DClass();

    bool initialize(int, int, bool, HWND, bool, float, float);
    void shutdown();

    void begin_scene(float, float, float, float);
    void end_scene();

    ID3D11Device* get_device();
    ID3D11DeviceContext* get_device_context();

    void get_projection_matrix(XMMATRIX&);
    void get_world_matrix(XMMATRIX&);
    void get_ortho_matrix(XMMATRIX&);

    void get_video_card_info(char*, int&);

    void set_back_buffer_render_target();
    void reset_viewport();

private:
    bool m_vsyncEnabled;
    int m_videoCardMemory;
    char m_videoCardDescription[128];
    IDXGISwapChain* m_swapChain;
    ID3D11Device* m_device;
    ID3D11DeviceContext* m_deviceContext;
    ID3D11RenderTargetView* m_renderTargetView;
    ID3D11Texture2D* m_depthStencilBuffer;
    ID3D11DepthStencilState* m_depthStencilState;
    ID3D11DepthStencilView* m_depthStencilView;
    ID3D11RasterizerState* m_rasterState;
    XMMATRIX m_projectionMatrix;
    XMMATRIX m_worldMatrix;
    XMMATRIX m_orthoMatrix;
    D3D11_VIEWPORT m_viewport;
};
