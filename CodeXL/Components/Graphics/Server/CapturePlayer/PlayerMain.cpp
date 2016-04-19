//==============================================================================
/// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file Main.cpp
/// \brief The main entrypoint for the CapturePlayer application. This small
/// application is responsible for loading a Capture file, and executing
/// playback of all captured calls. The player is able to respond to commands
/// issued by HTTP requests.
///
/// @TODO: THERE SHOULD NOT BE ANY API-SPECIFIC CODE WITHIN THE PLAYER'S MAIN.
/// Instead, load a Capture file from disk, and execute it. Doing so will load
/// our Server plugins, which we can use to communicate with the player process.
///
/// @TODO: Must update the CapturePlayer project file to remove unwanted .props.
///
//==============================================================================

#include <d3d12.h>
#include <dxgi1_4.h>
#include <stdio.h>
#include "ReplayWindow.h"

inline void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        throw;
    }
}

// Main message handler for the sample.
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // Handle destroy/shutdown messages.
    switch (message)
    {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    // Handle any messages the switch statement didn't.
    return DefWindowProc(hWnd, message, wParam, lParam);
}

ReplayWindow* replayWindow = NULL;
const UINT windowWidth = 800;
const UINT windowHeight = 600;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    replayWindow = new ReplayWindow(windowWidth, windowHeight);

    // Start the ecapture player minimized. We will need to revert this when we are playing back a frame capture as in this case we will want to see the window.
    bool bWindowInitialied = replayWindow->Initialize(hInstance, nCmdShow, WindowProc);

    if (bWindowInitialied)
    {
        // Open the window in the system's UI after creating it.
        bool bWindowReady = replayWindow->OpenAndUpdate(SW_MINIMIZE);

        if (bWindowReady)
        {
            //////////////////////////////////////////////////////////////////////////
            // @TODO: THIS CODE WAS STOLEN FROM THE "D3D12MULTITHREADING" SDK SAMPLE!
            //////////////////////////////////////////////////////////////////////////
            ID3D12Device* graphicsDevice = NULL;

            UINT frameCount = 2;

            // Initialize all pipeline components necessary to render a frame.
            /// Invoking these calls will allow the DXGI/DX12Server plugins to be injected into our player application.

            // @TODO: In the future, the following commands will invoked by loaded a capture file,
            // initializing, and executing all captured calls. Spinning on the target frame will beat
            // the DX12Server's message loop, allowing communicate with GPUPerfServer.
            IDXGIFactory4* factory = NULL;
            ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&factory)));

            ThrowIfFailed(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&graphicsDevice)));

            // Describe and create the command queue.
            D3D12_COMMAND_QUEUE_DESC queueDesc = {};
            queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
            queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

            ID3D12CommandQueue* commandQueue = NULL;
            ThrowIfFailed(graphicsDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)));

            // Describe and create the swap chain.
            DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
            swapChainDesc.BufferCount = frameCount;
            swapChainDesc.BufferDesc.Width = windowWidth;
            swapChainDesc.BufferDesc.Height = windowHeight;
            swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
            swapChainDesc.OutputWindow = replayWindow->GetWindowHandle();
            swapChainDesc.SampleDesc.Count = 1;
            swapChainDesc.Windowed = TRUE;

            IDXGISwapChain3* swapchain = NULL;
            ThrowIfFailed(factory->CreateSwapChain(commandQueue, &swapChainDesc, (IDXGISwapChain**)&swapchain));

            // Main sample loop.
            MSG msg = { 0 };

            for (;;)
            {
                // Process any messages in the queue.
                if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);

                    if (msg.message == WM_QUIT)
                    {
                        break;
                    }
                }

                ThrowIfFailed(swapchain->Present(0, 0));
            }
        }
    }

    return 0;
}