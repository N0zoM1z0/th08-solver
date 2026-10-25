#define CINTERFACE
#include <cstddef>
#include <d3d8.h>
#include <windows.h>
// The wrapped methods must keep their exact SDK slots on the pinned ABI.
static_assert(sizeof(void *) == 4);
static_assert(sizeof(IDirect3DDevice8Vtbl) == 97 * sizeof(void *));
static_assert(offsetof(IDirect3DDevice8Vtbl, Present) == 15 * sizeof(void *));
static_assert(offsetof(IDirect3DDevice8Vtbl, BeginScene) == 34 * sizeof(void *));
static_assert(offsetof(IDirect3DDevice8Vtbl, EndScene) == 35 * sizeof(void *));
