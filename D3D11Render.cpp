#include "D3D11Render.h"

#include <iostream>

// C6385: Invalid warning about the readable size
#pragma warning(disable : 6385)

using Microsoft::WRL::ComPtr;

bool D3D11Render::Initialize(HWND hWnd)
{
    // Example at https://docs.microsoft.com/en-us/windows/win32/direct3dgetstarted/work-with-dxgi
    HRESULT hr = S_OK;

    D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_1 };
    D3D_FEATURE_LEVEL featureLevelCreated;

    // This flag adds support for surfaces with a color-channel ordering different
    // from the API default. It is required for compatibility with Direct2D.
    UINT deviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(_DEBUG)
    deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    hr = D3D11CreateDevice(
        nullptr,  // Use default adapter
        D3D_DRIVER_TYPE_HARDWARE,
        0,        // No software rasterizer
        deviceFlags,
        levels, ARRAYSIZE(levels),
        D3D11_SDK_VERSION,
        &d3d11Device,
        &featureLevelCreated,
        &d3d11DeviceContext
    );
    CheckHR(hr);

    DXGI_SWAP_CHAIN_DESC swapChainDesc = { 0 };

    // Flip Sequential improves perf and is required on some platforms. Available starting DXGI 1.2 on Windows 8
    // See also https://docs.microsoft.com/en-us/windows/win32/direct3ddxgi/for-best-performance--use-dxgi-flip-model
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // Most common
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;  // This swap-chain to be used for the display
    swapChainDesc.BufferCount = 2;                                // Minimum for FLIP_SEQUENTIAL is 2
    swapChainDesc.Windowed = true;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.OutputWindow = hWnd;

    /* Below can all be defaulted to 0 */
    // swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    // swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    // swapChainDesc.BufferDesc.Width = 0;
    // swapChainDesc.BufferDesc.Height = 0;
    // swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
    // swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
    // swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // Allow transition in/out of full-screen

    hr = d3d11Device.As(&dxgiDevice3);
    CheckHR(hr);

    hr = dxgiDevice3->GetAdapter(&dxgiAdapter);
    CheckHR(hr);

    hr = dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
    CheckHR(hr);

    hr = dxgiFactory->CreateSwapChain(dxgiDevice3.Get(), &swapChainDesc, &dxgiSwapChain);
    CheckHR(hr, "Failed to create the SwapChain");

    hr = dxgiSwapChain->GetDesc(&swapChainDesc);
    CheckHR(hr);
    this->width = swapChainDesc.BufferDesc.Width;
    this->height = swapChainDesc.BufferDesc.Height;

    hr = dxgiSwapChain->GetBuffer(0, IID_PPV_ARGS(&d3d11Resource));
    CheckHR(hr);

    hr = d3d11Device->CreateRenderTargetView(d3d11Resource.Get(), nullptr, &d3d11RenderTargetView);
    CheckHR(hr);

    this->hWnd = hWnd;
    isInitialized = true;
    return isInitialized;
}

void D3D11Render::Present()
{
    if (isInitialized)
    {
        const float color[] = { 0.5f, 0.5f, 0.5f, 1.0f };

        // Draw a triagle
        struct Vertex {
            float x;
            float y;
        };

        HRESULT hr = S_OK;
        ComPtr<ID3D11Buffer> d3d11Buffer;
        ComPtr<ID3D11VertexShader> d3d11VertexShader;
        ComPtr<ID3D11PixelShader> d3d11PixelShader;
        ComPtr<ID3DBlob> d3dBlob;
        ComPtr<ID3D11InputLayout> d3d11InputLayout;

        D3D11_BUFFER_DESC desc;
        D3D11_SUBRESOURCE_DATA subData;
        const UINT strides = sizeof(Vertex);
        const UINT offsets = 0;

        // Note: By default vertices must be in a clockwise direction, else will get culled
        const Vertex triangle[] = { {0.0f, 0.5f}, {0.5f, -0.5f}, {-0.5f, -0.5f} };

        // Create the vertex buffer
        desc.ByteWidth = sizeof(triangle);
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        desc.CPUAccessFlags = 0;
        desc.MiscFlags = 0;
        desc.StructureByteStride = sizeof(Vertex);

        subData.pSysMem = triangle;
        subData.SysMemPitch = 0;
        subData.SysMemSlicePitch = 0;

        hr = d3d11Device->CreateBuffer(&desc, &subData, &d3d11Buffer);
        CheckHR(hr);
        d3d11DeviceContext->IASetVertexBuffers(0, 1, d3d11Buffer.GetAddressOf(), &strides, &offsets);

        // Create the pixel shader
        hr = D3DReadFileToBlob(L"shader-pixel.cso", &d3dBlob);
        CheckHR(hr);
        hr = d3d11Device->CreatePixelShader(d3dBlob->GetBufferPointer(), d3dBlob->GetBufferSize(), nullptr, &d3d11PixelShader);
        CheckHR(hr);
        d3d11DeviceContext->PSSetShader(d3d11PixelShader.Get(), nullptr, 0);

        // Create the vertex shader
        hr = D3DReadFileToBlob(L"shader-vertex.cso", &d3dBlob);
        CheckHR(hr);
        hr = d3d11Device->CreateVertexShader(d3dBlob->GetBufferPointer(), d3dBlob->GetBufferSize(), nullptr, &d3d11VertexShader);
        CheckHR(hr);
        d3d11DeviceContext->VSSetShader(d3d11VertexShader.Get(), nullptr, 0);

        // Specify the element layout
        const D3D11_INPUT_ELEMENT_DESC elementDesc[] = {
            {
                "POSITION", /* Semantic name */
                0 /* Semantic Index*/,
                DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT,
                0 /* Input Slots */,
                0 /* AlignedByteOffset */,
                D3D11_INPUT_PER_VERTEX_DATA,
                0 /* InstanceDataStepRate*/
             }
        };

        hr = d3d11Device->CreateInputLayout(elementDesc, 1, d3dBlob->GetBufferPointer(), d3dBlob->GetBufferSize(), &d3d11InputLayout);
        CheckHR(hr);
        d3d11DeviceContext->IASetInputLayout(d3d11InputLayout.Get());

        d3d11DeviceContext->OMSetRenderTargets(1, d3d11RenderTargetView.GetAddressOf(), nullptr);
        d3d11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        D3D11_VIEWPORT viewport;
        viewport.TopLeftX = 0;
        viewport.TopLeftY = 0;
        viewport.Width = (float)this->width;
        viewport.Height = (float)this->height;
        viewport.MinDepth = 0;
        viewport.MaxDepth = 1;

        d3d11DeviceContext->RSSetViewports(1u, &viewport);

        d3d11DeviceContext->ClearRenderTargetView(d3d11RenderTargetView.Get(), color);
        d3d11DeviceContext->Draw(3u, 0u);

        hr = dxgiSwapChain->Present(1, 0);
        CheckHR(hr);
        printf("Rendered a triangle\n");
    }
}

void D3D11Render::OnResize(unsigned flags, int width, int height)
{
    // TODO: Update size of resource such as the swap chain
}

// TODO: Convert to use ComPtr and RAII for safety, and maybe move to a "utilities" file
bool ListDisplayModes(HWND hWnd) {
    HRESULT hr = S_OK;
    IDXGIFactory2* pFactory;
    IDXGIAdapter1* pAdapter;
    IDXGIOutput* pOutput;
    IDXGIOutput1* pOutput1;
    DXGI_ADAPTER_DESC1 adapterDesc;

    hr = CreateDXGIFactory2(0, IID_PPV_ARGS(&pFactory));
    CheckHR(hr);

    hr = pFactory->EnumAdapters1(0, &pAdapter); // Index 0 is the adapter for the primary display
    CheckHR(hr);

    hr = pAdapter->GetDesc1(&adapterDesc);
    CheckHR(hr);

    std::wcout << L"Using adapter: " << adapterDesc.Description << "\n";
    hr = pAdapter->EnumOutputs(0, &pOutput); // Index 0 is the adapter for the primary display
    CheckHR(hr);

    hr = pOutput->QueryInterface(IID_PPV_ARGS(&pOutput1));
    CheckHR(hr);

    UINT num_modes = 0;
    hr = pOutput1->GetDisplayModeList1(DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_ENUM_MODES_STEREO, &num_modes, NULL);
    CheckHR(hr);

    DXGI_MODE_DESC1* pModes = new DXGI_MODE_DESC1[num_modes];
    hr = pOutput1->GetDisplayModeList1(DXGI_FORMAT_B8G8R8A8_UNORM, 0 /* Flags */, &num_modes, pModes);
    CheckHR(hr);

    bool resolutionSupported = false;
    for (UINT i = 0; i < num_modes; ++i) {
        const auto& mode = pModes[i];
        // https://www.nvidia.com/en-us/geforce/forums/shield-tv/9/247789/why-is-59940-hz-recommended-over-60hz/
        if (mode.Height == 1080 && mode.Width == 1920 && mode.RefreshRate.Denominator == 1000 &&
            (mode.RefreshRate.Numerator == 60000 || mode.RefreshRate.Numerator == 59940)) {
            resolutionSupported = true;
        }
        std::cout << " - Height: " << mode.Height << ", Width: " << mode.Width << ", Numerator: " << mode.RefreshRate.Numerator << ", Denominator: " << mode.RefreshRate.Denominator << std::endl;
    }

    // Clean up
    pOutput1->Release();
    pOutput->Release();
    pAdapter->Release();
    pFactory->Release();
    delete[] pModes;

    return resolutionSupported;
}
