#pragma once
// Reuse the tracked CPU D3DX declarations, while keeping real MinGW windows.h.
#include "../../../third_party/th08/src/modern/linux/include/d3dx8.h"
using PDIRECT3D8 = IDirect3D8 *;
using PDIRECT3DDEVICE8 = IDirect3DDevice8 *;
