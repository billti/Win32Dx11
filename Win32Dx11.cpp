#include "Win32Dx11.h"
#include "D3D11Render.h"

#include <shellapi.h>

#define MAX_LOADSTRING 100

const bool show_menu = false;

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
D3D11Render d3d11Render;                        // Does all the Direct3D magic

// Forward declarations of functions included in this code module:
ATOM                RegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Make the diagnostics console optional
    FILE* stream;
    if (!AttachConsole(ATTACH_PARENT_PROCESS)) AllocConsole();
    freopen_s(&stream, "CON", "r", stdin);
    freopen_s(&stream, "CON", "w", stdout);
    freopen_s(&stream, "CON", "w", stderr);
    printf("Launching app\n");

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WIN32DX11, szWindowClass, MAX_LOADSTRING);
    RegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        printf("InitInstance failed\n");
        return -1;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WIN32DX11));

    // Main message loop:
    BOOL msgResult;
    MSG msg = { 0 };
    msgResult = PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
    while (WM_QUIT != msg.message) {
        msgResult = PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
        if (msgResult) {
            if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        } else {
            d3d11Render.Present();
        }
    }

    return (int) msg.wParam;
}

ATOM RegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = 0; // CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WIN32DX11));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = NULL; // We'll paint our own background
    wcex.lpszMenuName   = show_menu ? MAKEINTRESOURCEW(IDC_WIN32DX11) : nullptr;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable
   RECT rect = { 0 };
   rect.left = 100;
   rect.top = 100;
   rect.right = rect.left + 1600;
   rect.bottom = rect.top + 900;
   AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, show_menu);

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      200, 100, rect.right - rect.left, rect.bottom - rect.top, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      printf("CreateWindowW failed\n");
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   if (!d3d11Render.Initialize(hWnd)) return FALSE;
   d3d11Render.Present();
   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                // Open the project page in the default browser
                ShellExecute(NULL, nullptr, L"https://github.com/billti/Win32Dx11", nullptr, nullptr, SW_SHOW);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) {
            printf("Escape key pressed\n");
        }
        if (wParam == 'Q') {
            printf("'Q' key pressed\n");
        }
        break;
    case WM_LBUTTONDOWN:
        {
            const POINTS pt = MAKEPOINTS(lParam);
            printf("Mouse clicked at {x: %d, y: %d}\n", pt.x, pt.y);
            break;
        }
    case WM_PAINT:
        {
            printf("Received WM_PAINT\n");
            d3d11Render.Present();

            // Need to validate the entire region, else WM_PAINT will keep coming forever
            ValidateRgn(hWnd, NULL);
            break;
        }
    case WM_SIZE:
        {
            int width = LOWORD(lParam);  // Macro to get the low-order word.
            int height = HIWORD(lParam); // Macro to get the high-order word.

            printf("Resizing to width: %d, height: %d\n", width, height);
            d3d11Render.OnResize((UINT)wParam, width, height);
            break;
        }
    case WM_DESTROY:
        printf("About to quit!\n");
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
