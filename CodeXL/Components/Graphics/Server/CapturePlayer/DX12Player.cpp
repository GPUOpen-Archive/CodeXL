#include <d3d12.h>
#include <dxgi1_4.h>
#include <stdio.h>

#include "DX12Player.h"

/// Helper function to detect for failure cases
inline void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        throw;
    }
}

/// The application-defined function that processes messages sent to a window.Main message handler
/// \param hWnd A handle to the window.
/// \param uMsg The message.
/// \param wParam Additional message information. The contents of this parameter depend on the value of the uMsg parameter.
/// \param lParam Additional message information. The contents of this parameter depend on the value of the uMsg parameter.
/// \return The return value is the result of the message processing and depends on the message sent.
LRESULT CALLBACK DX12WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // Handle destroy/shutdown messages.
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    // Handle any messages the switch statement didn't.
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

/// Initialize a render window for Windows.
/// \param hInstance Application instance
/// \param windowWidth The width of the player window
/// \param windowHeight The height of the player window
/// \return True if success, false if failure
bool DX12Player::InitializeWindow(HINSTANCE hInstance, UINT windowWidth, UINT windowHeight)
{
    m_pPlayerWindow = new WindowsWindow(windowWidth, windowHeight, hInstance, DX12WindowProc);

    if (m_pPlayerWindow == nullptr)
    {
        return false;
    }

    bool bWindowInitialied = m_pPlayerWindow->Initialize();

    if (bWindowInitialied == false)
    {
        return false;
    }

    bool bOpenAndUpdated = m_pPlayerWindow->OpenAndUpdate(SW_MINIMIZE);
    if (bOpenAndUpdated == false)
    {
        return false;
    }

    return bOpenAndUpdated;
}

/// Overriden in derived class to initialize the graphics required for a render loop. The render loop acts as a message pump to the user clients.
/// \return True if success, false if failure
bool DX12Player::InitializeGraphics()
{
    ID3D12Device* graphicsDevice = nullptr;

    UINT frameCount = 2;

    // Initialize all pipeline components necessary to render a frame.
    /// Invoking these calls will allow the DXGI/DX12Server plugins to be injected into our player application.

    // @TODO: In the future, the following commands will invoked by loaded a capture file,
    // initializing, and executing all captured calls. Spinning on the target frame will beat
    // the DX12Server's message loop, allowing communicate with GPUPerfServer.
    IDXGIFactory4* factory = nullptr;
    ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&factory)));

    ThrowIfFailed(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&graphicsDevice)));

    // Describe and create the command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    ID3D12CommandQueue* commandQueue = nullptr;
    ThrowIfFailed(graphicsDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)));

    // Describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferCount = frameCount;
    swapChainDesc.BufferDesc.Width = m_pPlayerWindow->GetWindowWidth();
    swapChainDesc.BufferDesc.Height = m_pPlayerWindow->GetWindowHeight();
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    // The window handle must be set before being used here.
    swapChainDesc.OutputWindow = m_pPlayerWindow->GetWindowHandle();
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.Windowed = TRUE;

    HRESULT res = factory->CreateSwapChain(commandQueue, &swapChainDesc, (IDXGISwapChain**)&m_swapchain);

    ThrowIfFailed(res);

    if (res != S_OK)
    {
        return false;
    }
    else
    {
        return true;
    }

}

/// Perfrom any cleanup here
void DX12Player::Destroy()
{
    // Cleanup here
    if (m_pPlayerWindow != nullptr)
    {
        delete m_pPlayerWindow;
        m_pPlayerWindow = nullptr;
    }
}

/// Implement the render loop.
void DX12Player::RenderLoop()
{
    // Main sample loop.
    MSG msg = { 0 };

    for (;;)
    {
        // Process any messages in the queue.
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_QUIT)
            {
                break;
            }
        }

        ThrowIfFailed(m_swapchain->Present(0, 0));
    }
}