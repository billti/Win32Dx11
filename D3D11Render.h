#pragma once

#include "framework.h"

#include <d3d11_2.h>
#include <dxgi1_2.h>
#include <d3dcompiler.h>

#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "DXGI.lib")
#pragma comment(lib, "D3DCompiler.lib")

bool ListDisplayModes(HWND hWnd);

class D3D11Render
{
public:
    bool Initialize(HWND);
    void Present();
    void OnResize(unsigned flags, int width, int height);
    bool isInitialized = false;
private:
    HWND hWnd = nullptr;
    UINT width = 0, height = 0;
    Microsoft::WRL::ComPtr<IDXGIFactory> dxgiFactory;
    Microsoft::WRL::ComPtr<ID3D11Device> d3d11Device;
    Microsoft::WRL::ComPtr<IDXGIDevice3> dxgiDevice3;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> d3d11DeviceContext;
    Microsoft::WRL::ComPtr<IDXGIAdapter> dxgiAdapter;
    Microsoft::WRL::ComPtr<IDXGISwapChain> dxgiSwapChain;
    Microsoft::WRL::ComPtr<ID3D11Resource> d3d11Resource;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> d3d11RenderTargetView;
};
