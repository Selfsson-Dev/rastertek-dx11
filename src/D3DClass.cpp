﻿#include "D3DClass.h"

D3DClass::D3DClass() {
    m_swapChain          = 0;
    m_device             = 0;
    m_deviceContext      = 0;
    m_renderTargetView   = 0;
    m_depthStencilBuffer = 0;
    m_depthStencilState  = 0;
    m_depthStencilView   = 0;
    m_rasterState        = 0;
}

D3DClass::D3DClass(const D3DClass& other) {
}

D3DClass::~D3DClass() {
}

// we initialize Direct3D here, and we get the refresh rate of the users graphics-card/monitor
// ??? we also set color info here ???
bool D3DClass::initialize(int screenWidth, int screenHeight, bool vsync, HWND hwnd, bool fullscreen, float screenDepth,
                          float screenNear) {
    HRESULT result;
    IDXGIFactory* factory;
    IDXGIAdapter* adapter;
    IDXGIOutput* adapterOutput;
    unsigned int numModes, i, numerator, denominator;
    unsigned long long stringLength;
    DXGI_MODE_DESC* displayModeList;
    DXGI_ADAPTER_DESC adapterDesc;
    int error;
    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    D3D_FEATURE_LEVEL featureLevel;
    ID3D11Texture2D* backBufferPtr;
    D3D11_TEXTURE2D_DESC depthBufferDesc;
    D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
    D3D11_RASTERIZER_DESC rasterDesc;
    float fieldOfView, screenAspect;

    m_vsyncEnabled = vsync;

    // create dx graphics interface factory
    result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
    if (FAILED(result)) {
        return false;
    }

    // use factory to create an adapter for the primary graphics interface (graphics card)
    result = factory->EnumAdapters(0, &adapter);
    if (FAILED(result)) {
        return false;
    }

    // enumerate primary adapter output (monitor)
    result = adapter->EnumOutputs(0, &adapterOutput);
    if (FAILED(result)) {
        return false;
    }

    // get number of modes that fit DXGI_FORMAT_R8G8B8A8_UNORM display format for the adapter output (monitor)
    // TODO read: https://learn.microsoft.com/en-us/windows/win32/api/dxgiformat/ne-dxgiformat-dxgi_format
    result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM,
                                               DXGI_ENUM_MODES_INTERLACED,
                                               &numModes, NULL);
    if (FAILED(result)) {
        return false;
    }

    // create a list with all possible display modes for the monitor/graphics-card combo
    displayModeList = new DXGI_MODE_DESC[numModes];
    if (!displayModeList) {
        return false;
    }

    // fill the display modes list
    result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM,
                                               DXGI_ENUM_MODES_INTERLACED,
                                               &numModes, displayModeList);
    if (FAILED(result)) {
        return false;
    }

    // Now go through all the display modes and find the one that matches the screen width and height.
    // When a match is found store the numerator and denominator of the refresh rate for that monitor.
    for (i = 0; i < numModes; i++) {
        if (displayModeList[i].Width == (unsigned int)screenWidth) {
            if (displayModeList[i].Height == (unsigned int)screenHeight) {
                numerator   = displayModeList[i].RefreshRate.Numerator;
                denominator = displayModeList[i].RefreshRate.Denominator;
            }
        }
    }

    result = adapter->GetDesc(&adapterDesc);
    if (FAILED(result)) {
        return false;
    }

    // store mb of card memory
    m_videoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

    // convert the name of the graphics card to a character array and store it
    error = wcstombs_s(&stringLength, m_videoCardDescription,
                       128,
                       adapterDesc.Description,
                       128);
    if (error != 0) {
        return false;
    }

    // we can release all these structures and interfaces since we only used them to get info
    delete[] displayModeList;
    displayModeList = 0;

    adapterOutput->Release();
    adapterOutput = 0;

    adapter->Release();
    adapter = 0;

    factory->Release();
    factory = 0;

    // directx initialization!
    // a back and front buffer is used to show an image on screen
    // front buffer is what is shown on screen
    // back buffer gets drawn on, then we switch the front and back buffer so that we can show the new drawn image
    // this is called a swap chain

    // initialize swapchain description
    ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));


    // set to a single back buffer
    // TODO check how to create a triple buffer (think its 2 here since we only count nr of back buffers)
    swapChainDesc.BufferCount = 1;

    // set height of buffer
    swapChainDesc.BufferDesc.Width  = screenWidth;
    swapChainDesc.BufferDesc.Height = screenHeight;

    // set 32-bit surface for the back buffer, we want it to be the same as above
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

    // set refresh rate of back buffer to the value of numerator and denominator
    if (m_vsyncEnabled) {
        swapChainDesc.BufferDesc.RefreshRate.Numerator   = numerator;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
    }
    else {
        swapChainDesc.BufferDesc.RefreshRate.Numerator   = 0;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    }

    // set the usage of the back buffer
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

    // set window handle
    swapChainDesc.OutputWindow = hwnd;

    // turn off multisample
    swapChainDesc.SampleDesc.Count   = 1;
    swapChainDesc.SampleDesc.Quality = 0;

    if (fullscreen) {
        swapChainDesc.Windowed = false;
    }
    else {
        swapChainDesc.Windowed = true;
    }

    // scanline ordering and scaling to unspecified
    swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapChainDesc.BufferDesc.Scaling          = DXGI_MODE_SCALING_UNSPECIFIED;

    // discard buffer contents after being presented
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    // ignore advanced flags
    swapChainDesc.Flags = 0;

    // set feature level to dx 11
    featureLevel = D3D_FEATURE_LEVEL_11_0;

    // create the swap chain, Direct3D device and Direct3D device context
    result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL,
                                           0, &featureLevel, 1, D3D11_SDK_VERSION,
                                           &swapChainDesc, &m_swapChain, &m_device,
                                           NULL, &m_deviceContext);
    if (FAILED(result)) {
        return false;
    }

    // get pointer to back buffer
    result = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr);
    if (FAILED(result)) {
        return false;
    }

    // create a render target view with the back buffer pointer we got
    result = m_device->CreateRenderTargetView(backBufferPtr, NULL, &m_renderTargetView);
    if (FAILED(result)) {
        return false;
    }

    // we no longer need back buffer pointer
    backBufferPtr->Release();
    backBufferPtr = 0;

    // initialize the description of the depth buffer
    ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

    depthBufferDesc.Width              = screenWidth;
    depthBufferDesc.Height             = screenHeight;
    depthBufferDesc.MipLevels          = 1;
    depthBufferDesc.ArraySize          = 1;
    depthBufferDesc.Format             = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthBufferDesc.SampleDesc.Count   = 1;
    depthBufferDesc.SampleDesc.Quality = 0;
    depthBufferDesc.Usage              = D3D11_USAGE_DEFAULT;
    //  The stencil buffer can be used to achieve effects such as motion blur, volumetric shadows, and other things.
    depthBufferDesc.BindFlags      = D3D11_BIND_DEPTH_STENCIL;
    depthBufferDesc.CPUAccessFlags = 0;
    depthBufferDesc.MiscFlags      = 0;

    // create the texture for the depth buffer
    // this 2d image will be used to show an image on screen
    result = m_device->CreateTexture2D(&depthBufferDesc, NULL, &m_depthStencilBuffer);
    if (FAILED(result)) {
        return false;
    }

    // control what type of depth test Direct3D will do for each pixel

    // initialize stencil state
    ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

    depthStencilDesc.DepthEnable    = true;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc      = D3D11_COMPARISON_LESS;

    depthStencilDesc.StencilEnable    = true;
    depthStencilDesc.StencilReadMask  = 0xff;
    depthStencilDesc.StencilWriteMask = 0xff;

    // stencil operation if pixel is front facing
    depthStencilDesc.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    depthStencilDesc.FrontFace.StencilPassOp      = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;

    // stencil operation if pixel is back facing
    depthStencilDesc.BackFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    depthStencilDesc.BackFace.StencilPassOp      = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;

    // create the depth stencil state
    result = m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState);
    if (FAILED(result)) {
        return false;
    }

    // set the depth stencil state
    m_deviceContext->OMSetDepthStencilState(m_depthStencilState, 1);

    //  create the description of the view of the depth stencil buffer
    //  so that Direct3D knows to use the depth buffer as a depth stencil texture

    //initialize the depth stencil view
    ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

    // set up the depth stencil view description
    depthStencilViewDesc.Format             = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilViewDesc.ViewDimension      = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;

    // create the depth stencil view
    result = m_device->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilView);
    if (FAILED(result)) {
        return false;
    }

    // binding render target view and depth stencil buffer to the output pipeline
    m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);

    // by default directx already has a rasterizer state set up, and it works like the one we are about to set up
    // but if we don't specify anything we won't be able to control anything about it

    // setup raster description this will determine how and what polygons will be drawn
    rasterDesc.AntialiasedLineEnable = false;
    rasterDesc.CullMode              = D3D11_CULL_BACK;
    rasterDesc.DepthBias             = 0;
    rasterDesc.DepthBiasClamp        = 0;
    rasterDesc.DepthClipEnable       = true;
    rasterDesc.FillMode              = D3D11_FILL_SOLID;
    rasterDesc.FrontCounterClockwise = false;
    rasterDesc.MultisampleEnable     = false;
    rasterDesc.SlopeScaledDepthBias  = 0.0f;

    // create the rasterizer state
    result = m_device->CreateRasterizerState(&rasterDesc, &m_rasterState);
    if (FAILED(result)) {
        return false;
    }

    // set the rasterizer state
    m_deviceContext->RSSetState(m_rasterState);

    // set up the viewport so that Direct3D can map clip space coordinates to the render target space

    // set up the viewport for rendering
    m_viewport.Width    = (float)screenWidth;
    m_viewport.Height   = (float)screenHeight;
    m_viewport.MinDepth = 0.0f;
    m_viewport.MaxDepth = 1.0f;
    m_viewport.TopLeftX = 0.0f;
    m_viewport.TopLeftY = 0.0f;

    // create the viewport
    m_deviceContext->RSSetViewports(1, &m_viewport);

    // set up the projection matrix
    fieldOfView  = 3.141592653f / 4.0f;
    screenAspect = (float)screenWidth / (float)screenHeight;

    // create the projection matrix for 3d rendering
    m_projectionMatrix = XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, screenNear, screenDepth);

    // initialize the word matrix to the identity matrix
    m_worldMatrix = XMMatrixIdentity();

    // create orthographic projection matrix for 2d rendering
    m_orthoMatrix = XMMatrixOrthographicLH((float)screenWidth, (float)screenHeight, screenNear, screenDepth);

    return true;
}

// release all pointers
void D3DClass::shutdown() {
    // set to windowed mode so we don't get exception while releasing the swap chain
    if (m_swapChain) {
        m_swapChain->SetFullscreenState(false, NULL);
    }

    if (m_rasterState) {
        m_rasterState->Release();
        m_rasterState = 0;
    }

    if (m_depthStencilView) {
        m_depthStencilView->Release();
        m_depthStencilView = 0;
    }

    if (m_depthStencilState) {
        m_depthStencilState->Release();
        m_depthStencilState = 0;
    }

    if (m_depthStencilBuffer) {
        m_depthStencilBuffer->Release();
        m_depthStencilBuffer = 0;
    }

    if (m_renderTargetView) {
        m_renderTargetView->Release();
        m_renderTargetView = 0;
    }

    if (m_deviceContext) {
        m_deviceContext->Release();
        m_deviceContext = 0;
    }

    if (m_device) {
        m_device->Release();
        m_device = 0;
    }

    if (m_swapChain) {
        m_swapChain->Release();
        m_swapChain = 0;
    }

    return;
}

// called in the beginning to clear the back buffer so it's ready to be drawn to
void D3DClass::begin_scene(float red, float blue, float green, float alpha) {
    float color[4] = { red, blue, green, alpha };

    // clear back buffer to specified color
    m_deviceContext->ClearRenderTargetView(m_renderTargetView, color);

    // clear the depth buffer
    m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

    return;
}

void D3DClass::end_scene() {
    // present the back buffer to the screen since rendering is complete
    if (m_vsyncEnabled) {
        // lock screen to refresh rate
        m_swapChain->Present(1, 0);
    }
    else {
        // swap as soon as done
        m_swapChain->Present(0, 0);
    }

    return;
}

ID3D11DeviceContext* D3DClass::get_device_context() {
    return m_deviceContext;
}

void D3DClass::get_projection_matrix(XMMATRIX& projectionMatrix) {
    projectionMatrix = m_orthoMatrix;
    return;
}

void D3DClass::get_world_matrix(XMMATRIX& worldMatrix) {
    worldMatrix = m_worldMatrix;
    return;
}

void D3DClass::get_ortho_matrix(XMMATRIX& orthoMatrix) {
    orthoMatrix = m_orthoMatrix;
    return;
}

void D3DClass::get_video_card_info(char* cardName, int& memory) {
    strcpy_s(cardName, 128, m_videoCardDescription);
    memory = m_videoCardMemory;
    return;
}

void D3DClass::set_back_buffer_render_target() {
    // bind the render target view and depth stencil buffer to the output render pipeline
    m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);

    return;
}

void D3DClass::reset_viewport() {
    // set the viewport
    m_deviceContext->RSSetViewports(1, &m_viewport);
    return;
}




