#pragma once

#include "targetver.h"

// Windows Header Files (exclude rarely used stuff)
#define NOBITMAP
#define NOCLIPBOARD
#define NODRAWTEXT
#define NOCOMM
#define NOHELP
#define NOKANJI
#define NOMCX
#define NOMINMAX
#define NONLS
#define NOPROFILER
#define NOSCROLL
#define NOSERVICE
#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <objbase.h>
#include <wrl/client.h>  // See https://github.com/microsoft/DirectXTK/wiki/ComPtr

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <stdio.h>
#include <stdexcept>

inline void CheckHR(HRESULT hr, char const * const msg = nullptr) {
    if (FAILED(hr)) {
        if (msg) printf("FATAL ERROR: %s\n", msg);
        terminate();
    }
}
