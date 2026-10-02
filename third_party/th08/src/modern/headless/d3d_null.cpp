// CPU resource storage for the imported ANM loader. Rendering has no gameplay
// effect here; ANM scripts, sprite dimensions and their lifetimes still execute.
// This adapter never initializes SDL video, OpenGL, a window or an audio device.
#include "modern/linux/d3d8_internal.hpp"
#include <algorithm>
#include <cstring>
#include <vector>

namespace {
UINT pixel_bytes(D3DFORMAT format) {
    switch (format) {
    case D3DFMT_R8G8B8:
        return 3;
    case D3DFMT_R5G6B5:
    case D3DFMT_X1R5G5B5:
    case D3DFMT_A1R5G5B5:
    case D3DFMT_A4R4G4B4:
        return 2;
    default:
        return 4;
    }
}
class Surface final : public IDirect3DSurface8 {
  public:
    Surface(UINT w, UINT h, D3DFORMAT f)
        : width(w), height(h), pitch(w * pixel_bytes(f)), format(f),
          pixels(std::size_t(pitch) * h) {}
    ULONG AddRef() override {
        return ++refs;
    }
    ULONG Release() override {
        auto n = --refs;
        if (!n)
            delete this;
        return n;
    }
    HRESULT GetDesc(D3DSURFACE_DESC *d) override {
        if (!d)
            return E_INVALIDARG;
        *d = {};
        d->Format = format;
        d->Type = D3DRTYPE_SURFACE;
        d->Pool = D3DPOOL_SYSTEMMEM;
        d->Size = pixels.size();
        d->Width = width;
        d->Height = height;
        return S_OK;
    }
    HRESULT LockRect(D3DLOCKED_RECT *out, const RECT *r, DWORD) override {
        if (!out)
            return E_INVALIDARG;
        const RECT area = r ? *r : RECT{0, 0, LONG(width), LONG(height)};
        if (area.left < 0 || area.top < 0 || area.right <= area.left || area.bottom <= area.top ||
            UINT(area.right) > width || UINT(area.bottom) > height)
            return E_INVALIDARG;
        out->Pitch = pitch;
        out->pBits = pixels.data() + area.top * pitch + area.left * pixel_bytes(format);
        return S_OK;
    }
    HRESULT UnlockRect() override {
        return S_OK;
    }
    HRESULT GetDC(HDC *dc) override {
        if (!dc)
            return E_INVALIDARG;
        *dc = CreateCompatibleDC(nullptr);
        return *dc ? S_OK : E_FAIL;
    }
    HRESULT ReleaseDC(HDC dc) override {
        return DeleteDC(dc) ? S_OK : E_FAIL;
    }
    ULONG refs = 1;
    UINT width, height, pitch;
    D3DFORMAT format;
    std::vector<BYTE> pixels;
};
class Texture final : public IDirect3DTexture8 {
  public:
    Texture(UINT w, UINT h, D3DFORMAT f) : surface(new Surface(w, h, f)) {}
    ~Texture() override {
        surface->Release();
    }
    ULONG AddRef() override {
        return ++refs;
    }
    ULONG Release() override {
        auto n = --refs;
        if (!n)
            delete this;
        return n;
    }
    DWORD SetPriority(DWORD p) override {
        auto old = priority;
        priority = p;
        return old;
    }
    void PreLoad() override {}
    HRESULT GetLevelDesc(UINT l, D3DSURFACE_DESC *d) override {
        return l ? E_INVALIDARG : surface->GetDesc(d);
    }
    HRESULT GetSurfaceLevel(UINT l, IDirect3DSurface8 **s) override {
        if (l || !s)
            return E_INVALIDARG;
        surface->AddRef();
        *s = surface;
        return S_OK;
    }
    HRESULT LockRect(UINT l, D3DLOCKED_RECT *r, const RECT *area, DWORD f) override {
        return l ? E_INVALIDARG : surface->LockRect(r, area, f);
    }
    HRESULT UnlockRect(UINT l) override {
        return l ? E_INVALIDARG : surface->UnlockRect();
    }
    ULONG refs = 1;
    DWORD priority = 0;
    Surface *surface;
};
class VertexBuffer final : public IDirect3DVertexBuffer8 {
  public:
    explicit VertexBuffer(UINT n) : bytes(n) {}
    ULONG AddRef() override {
        return ++refs;
    }
    ULONG Release() override {
        auto n = --refs;
        if (!n)
            delete this;
        return n;
    }
    HRESULT Lock(UINT offset, UINT size, BYTE **out, DWORD) override {
        if (!out || offset > bytes.size() || size > bytes.size() - offset)
            return E_INVALIDARG;
        *out = bytes.data() + offset;
        return S_OK;
    }
    HRESULT Unlock() override {
        return S_OK;
    }
    ULONG refs = 1;
    std::vector<BYTE> bytes;
};
class Device final : public IDirect3DDevice8 {
  public:
    Device() : backbuffer(new Surface(640, 480, D3DFMT_X8R8G8B8)) {}
    ~Device() override {
        backbuffer->Release();
    }
    ULONG AddRef() override {
        return ++refs;
    }
    ULONG Release() override {
        auto n = --refs;
        if (!n)
            delete this;
        return n;
    }
    HRESULT TestCooperativeLevel() override {
        return S_OK;
    }
    HRESULT Reset(D3DPRESENT_PARAMETERS *) override {
        return S_OK;
    }
    HRESULT Present(const RECT *, const RECT *, HWND, const RGNDATA *) override {
        return S_OK;
    }
    HRESULT GetBackBuffer(UINT index, D3DBACKBUFFER_TYPE, IDirect3DSurface8 **out) override {
        if (index || !out)
            return E_INVALIDARG;
        backbuffer->AddRef();
        *out = backbuffer;
        return S_OK;
    }
    HRESULT CreateTexture(UINT w, UINT h, UINT, DWORD, D3DFORMAT f, D3DPOOL,
                          IDirect3DTexture8 **out) override {
        if (!out || !w || !h || w > 8192 || h > 8192)
            return E_INVALIDARG;
        *out = new Texture(w, h, f);
        return S_OK;
    }
    HRESULT CreateVertexBuffer(UINT n, DWORD, DWORD, D3DPOOL,
                               IDirect3DVertexBuffer8 **out) override {
        if (!out)
            return E_INVALIDARG;
        *out = new VertexBuffer(n);
        return S_OK;
    }
    HRESULT CreateRenderTarget(UINT w, UINT h, D3DFORMAT f, D3DMULTISAMPLE_TYPE, BOOL,
                               IDirect3DSurface8 **out) override {
        return CreateImageSurface(w, h, f, out);
    }
    HRESULT CreateImageSurface(UINT w, UINT h, D3DFORMAT f, IDirect3DSurface8 **out) override {
        if (!out || !w || !h || w > 8192 || h > 8192)
            return E_INVALIDARG;
        *out = new Surface(w, h, f);
        return S_OK;
    }
    HRESULT CopyRects(IDirect3DSurface8 *source, const RECT *rects, UINT n, IDirect3DSurface8 *dest,
                      const POINT *points) override {
        auto *s = dynamic_cast<Surface *>(source);
        auto *d = dynamic_cast<Surface *>(dest);
        if (!s || !d || s->format != d->format)
            return E_INVALIDARG;
        if (!rects)
            n = 1;
        for (UINT i = 0; i < n; ++i) {
            RECT r = rects ? rects[i] : RECT{0, 0, LONG(s->width), LONG(s->height)};
            POINT p = points ? points[i] : POINT{0, 0};
            D3DLOCKED_RECT src{}, dst{};
            RECT target{p.x, p.y, p.x + r.right - r.left, p.y + r.bottom - r.top};
            if (FAILED(s->LockRect(&src, &r, 0)) || FAILED(d->LockRect(&dst, &target, 0)))
                return E_INVALIDARG;
            for (LONG y = 0; y < r.bottom - r.top; ++y)
                std::memmove(static_cast<BYTE *>(dst.pBits) + y * dst.Pitch,
                             static_cast<BYTE *>(src.pBits) + y * src.Pitch,
                             (r.right - r.left) * pixel_bytes(s->format));
        }
        return S_OK;
    }
    HRESULT BeginScene() override {
        return S_OK;
    }
    HRESULT EndScene() override {
        return S_OK;
    }
    HRESULT Clear(DWORD, const D3DRECT *, DWORD, D3DCOLOR, float, DWORD) override {
        return S_OK;
    }
    HRESULT SetTransform(D3DTRANSFORMSTATETYPE, const D3DMATRIX *) override {
        return S_OK;
    }
    HRESULT SetViewport(const D3DVIEWPORT8 *v) override {
        if (!v)
            return E_INVALIDARG;
        viewport = *v;
        return S_OK;
    }
    HRESULT GetViewport(D3DVIEWPORT8 *v) override {
        if (!v)
            return E_INVALIDARG;
        *v = viewport;
        return S_OK;
    }
    HRESULT SetRenderState(D3DRENDERSTATETYPE, DWORD) override {
        return S_OK;
    }
    HRESULT SetTexture(DWORD, IDirect3DTexture8 *) override {
        return S_OK;
    }
    HRESULT SetTextureStageState(DWORD, D3DTEXTURESTAGESTATETYPE, DWORD) override {
        return S_OK;
    }
    HRESULT SetVertexShader(DWORD) override {
        return S_OK;
    }
    HRESULT SetStreamSource(UINT, IDirect3DVertexBuffer8 *, UINT) override {
        return S_OK;
    }
    HRESULT DrawPrimitive(D3DPRIMITIVETYPE, UINT, UINT) override {
        return S_OK;
    }
    HRESULT DrawPrimitiveUP(D3DPRIMITIVETYPE, UINT, const void *, UINT) override {
        return S_OK;
    }
    HRESULT GetDeviceCaps(D3DCAPS8 *c) override {
        if (!c)
            return E_INVALIDARG;
        *c = {};
        c->MaxTextureWidth = c->MaxTextureHeight = 8192;
        return S_OK;
    }
    HRESULT ResourceManagerDiscardBytes(DWORD) override {
        return S_OK;
    }
    ULONG refs = 1;
    Surface *backbuffer;
    D3DVIEWPORT8 viewport{0, 0, 640, 480, 0, 1};
};
class Direct3D final : public IDirect3D8 {
  public:
    ULONG AddRef() override {
        return ++refs;
    }
    ULONG Release() override {
        auto n = --refs;
        if (!n)
            delete this;
        return n;
    }
    HRESULT GetAdapterDisplayMode(UINT, D3DDISPLAYMODE *m) override {
        if (!m)
            return E_INVALIDARG;
        *m = {640, 480, 60, D3DFMT_X8R8G8B8};
        return S_OK;
    }
    HRESULT CheckDeviceFormat(UINT, D3DDEVTYPE, D3DFORMAT, DWORD, D3DRESOURCETYPE,
                              D3DFORMAT) override {
        return S_OK;
    }
    HRESULT CreateDevice(UINT, D3DDEVTYPE, HWND, DWORD, D3DPRESENT_PARAMETERS *,
                         IDirect3DDevice8 **out) override {
        if (!out)
            return E_INVALIDARG;
        *out = new Device();
        return S_OK;
    }
    ULONG refs = 1;
};
} // namespace
extern "C" IDirect3D8 *Direct3DCreate8(UINT) {
    return new Direct3D();
}
bool th08_linux_surface_access(IDirect3DSurface8 *raw, LinuxSurfaceAccess *out, bool) {
    auto *s = dynamic_cast<Surface *>(raw);
    if (!s || !out)
        return false;
    *out = {s->pixels.data(), s->width, s->height, s->pitch, s->format};
    return true;
}
void th08_linux_surface_changed(IDirect3DSurface8 *) {}
// Pixel/render audits require a rendered framebuffer and are unsupported here.
bool th08_linux_texture_region_stats(IDirect3DTexture8 *, float, float, float, float, D3DCOLOR,
                                     LinuxTextureRegionStats *) {
    return false;
}
bool th08_linux_begin_framebuffer_probe(IDirect3DDevice8 *, int, int, int, int) {
    return false;
}
bool th08_linux_end_framebuffer_probe(IDirect3DDevice8 *, LinuxFramebufferDeltaStats *) {
    return false;
}
