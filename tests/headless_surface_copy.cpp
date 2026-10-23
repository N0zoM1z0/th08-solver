#include "modern/linux/d3d8_internal.hpp"
#include "test_support.hpp"
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <d3dx8.h>
#include <iostream>
#include <vector>

namespace {
using th08::test::check;
unsigned bytes_per_pixel(D3DFORMAT format) {
    return format == D3DFMT_R5G6B5 || format == D3DFMT_A4R4G4B4 || format == D3DFMT_X1R5G5B5 ? 2
                                                                                             : 4;
}

// An owned CPU surface adapter lets the production D3DX entry point exercise
// arbitrary row padding and exact change notifications without a Native world.
// Like the headless adapter, distinct objects own disjoint pixel vectors.
class Surface final : public IDirect3DSurface8 {
  public:
    static constexpr unsigned guard = 16;
    Surface(unsigned w, unsigned h, D3DFORMAT f, unsigned padding = 0)
        : width(w), height(h), pitch(w * bytes_per_pixel(f) + padding), format(f),
          storage(2 * guard + std::size_t(pitch) * h, 0xa5) {
        std::fill(storage.begin() + guard, storage.end() - guard, 0x37);
    }
    BYTE *pixels() {
        return storage.data() + guard;
    }
    ULONG AddRef() override {
        return 1;
    }
    ULONG Release() override {
        return 1;
    }
    HRESULT GetDesc(D3DSURFACE_DESC *out) override {
        if (!out)
            return E_INVALIDARG;
        *out = {};
        out->Width = width;
        out->Height = height;
        out->Format = format;
        return S_OK;
    }
    HRESULT LockRect(D3DLOCKED_RECT *, const RECT *, DWORD) override {
        return E_FAIL;
    }
    HRESULT UnlockRect() override {
        return E_FAIL;
    }
    HRESULT GetDC(HDC *) override {
        return E_FAIL;
    }
    HRESULT ReleaseDC(HDC) override {
        return E_FAIL;
    }
    unsigned width, height, pitch;
    D3DFORMAT format;
    std::vector<BYTE> storage;
};

struct Access {
    Surface *surface;
    bool read;
};
std::vector<Access> accesses;
std::vector<Surface *> changes;

HRESULT copy(Surface &destination, const RECT *dr, Surface &source, const RECT *sr,
             DWORD filter = 3, D3DCOLOR key = 0) {
    accesses.clear();
    changes.clear();
    return D3DXLoadSurfaceFromSurface(&destination, nullptr, dr, &source, nullptr, sr, filter, key);
}

void success(Surface &destination, Surface &source) {
    check(changes.size() == 1 && changes.front() == &destination,
          "surface copy lost or duplicated change notification");
    check(accesses.size() >= 2 && accesses.front().surface == &source && accesses.front().read &&
              accesses.back().surface == &destination && !accesses.back().read,
          "surface copy changed source-read/destination-write access order");
}

void fill(Surface &surface) {
    const auto bytes = bytes_per_pixel(surface.format);
    for (unsigned y = 0; y < surface.height; ++y)
        for (unsigned x = 0; x < surface.width; ++x) {
            auto *pixel = surface.pixels() + y * surface.pitch + x * bytes;
            const auto index = y * surface.width + x;
            if (bytes == 2) {
                const auto word = std::uint16_t(index);
                std::memcpy(pixel, &word, 2);
            } else {
                pixel[0] = BYTE(index);
                pixel[1] = BYTE(index * 17 + 3);
                pixel[2] = BYTE(index * 67 + 9);
                pixel[3] = BYTE(index * 41);
            }
        }
}

std::vector<BYTE> identity_expected(const Surface &destination, Surface &source) {
    auto expected = destination.storage;
    for (unsigned y = 0; y < source.height; ++y)
        std::copy_n(source.pixels() + y * source.pitch,
                    source.width * bytes_per_pixel(source.format),
                    expected.begin() + Surface::guard + y * destination.pitch);
    return expected;
}

void identity(D3DFORMAT format, unsigned width, unsigned height) {
    Surface source(width, height, format, 7), destination(width, height, format, 13);
    fill(source);
    const auto source_before = source.storage;
    auto fallback = destination;
    const auto expected = identity_expected(destination, source);
    check(copy(destination, nullptr, source, nullptr) == S_OK, "eligible surface copy failed");
    success(destination, source);
    check(destination.storage == expected, "packed identity copy changed pixels, alpha or padding");
    const RECT full{0, 0, LONG(width), LONG(height)};
    check(copy(fallback, &full, source, &full) == S_OK, "forced original conversion failed");
    success(fallback, source);
    check(fallback.storage == expected && fallback.storage == destination.storage,
          "optimized copy differs from forced original pixel roundtrip");
    check(source.storage == source_before, "surface copy mutated its source");
}

void self_overlap() {
    Surface surface(6, 4, D3DFMT_A8R8G8B8, 9);
    fill(surface);
    const auto snapshot = surface.storage;
    auto expected = snapshot;
    const RECT source{0, 0, 4, 3}, destination{1, 1, 5, 4};
    for (unsigned y = 0; y < 3; ++y)
        std::copy_n(snapshot.begin() + Surface::guard + y * surface.pitch, 16,
                    expected.begin() + Surface::guard + (y + 1) * surface.pitch + 4);
    check(copy(surface, &destination, surface, &source) == S_OK, "overlapping self-copy failed");
    success(surface, surface);
    check(surface.storage == expected, "self-copy lost original source snapshot semantics");
}

void fallbacks() {
    Surface source(7, 3, D3DFMT_A8R8G8B8, 5), destination(7, 3, D3DFMT_A8R8G8B8, 11);
    fill(source);
    const RECT clipped{0, 0, 100, 100};
    const auto expected = identity_expected(destination, source);
    check(copy(destination, &clipped, source, &clipped) == S_OK, "clamped rectangle copy failed");
    success(destination, source);
    check(destination.storage == expected, "rectangle clamping changed");
    const auto before = destination.storage;
    check(copy(destination, nullptr, source, nullptr, 0) == E_INVALIDARG,
          "invalid filter bypassed validation");
    check(changes.empty() && destination.storage == before, "invalid filter wrote destination");
    const RECT negative{-1, 0, 2, 2};
    check(copy(destination, nullptr, source, &negative) == E_INVALIDARG,
          "invalid source rectangle accepted");
    check(changes.empty() && destination.storage == before, "invalid rectangle wrote destination");

    for (unsigned y = 0; y < source.height; ++y)
        for (unsigned x = 0; x < source.width; ++x) {
            BYTE *pixel = source.pixels() + y * source.pitch + x * 4;
            pixel[0] = 0x33;
            pixel[1] = 0x22;
            pixel[2] = 0x11;
            pixel[3] = 0x88;
        }
    auto keyed = identity_expected(destination, source);
    for (unsigned y = 0; y < destination.height; ++y)
        for (unsigned x = 0; x < destination.width; ++x)
            keyed[Surface::guard + y * destination.pitch + x * 4 + 3] = 0;
    check(copy(destination, nullptr, source, nullptr, 3, 0x112233) == S_OK,
          "color-key fallback failed");
    success(destination, source);
    check(destination.storage == keyed, "color key no longer clears alpha");

    Surface small(2, 2, D3DFMT_A8R8G8B8, 7), scaled(5, 3, D3DFMT_A8R8G8B8, 13);
    for (unsigned y = 0; y < small.height; ++y)
        for (unsigned x = 0; x < small.width; ++x) {
            BYTE *pixel = small.pixels() + y * small.pitch + x * 4;
            pixel[0] = pixel[2] = pixel[3] = 255;
            pixel[1] = 0;
        }
    auto scaled_expected = scaled.storage;
    for (unsigned y = 0; y < scaled.height; ++y)
        for (unsigned x = 0; x < scaled.width; ++x) {
            auto offset = Surface::guard + y * scaled.pitch + x * 4;
            scaled_expected[offset] = scaled_expected[offset + 2] = scaled_expected[offset + 3] =
                255;
            scaled_expected[offset + 1] = 0;
        }
    check(copy(scaled, nullptr, small, nullptr) == S_OK, "scaled-copy fallback failed");
    success(scaled, small);
    check(scaled.storage == scaled_expected, "scaled fallback replaced by partial raw copy");

    for (const auto format : {D3DFMT_X8R8G8B8, D3DFMT_X1R5G5B5}) {
        Surface xsource(257, 3, format, 3), xdestination(257, 3, format, 5);
        fill(xsource);
        auto normalized = identity_expected(xdestination, xsource);
        for (unsigned y = 0; y < xdestination.height; ++y)
            for (unsigned x = 0; x < xdestination.width; ++x) {
                auto offset = Surface::guard + y * xdestination.pitch + x * bytes_per_pixel(format);
                if (format == D3DFMT_X8R8G8B8)
                    normalized[offset + 3] = 255;
                else {
                    std::uint16_t word;
                    std::memcpy(&word, normalized.data() + offset, 2);
                    word |= 0x8000;
                    std::memcpy(normalized.data() + offset, &word, 2);
                }
            }
        check(copy(xdestination, nullptr, xsource, nullptr) == S_OK, "X-format copy failed");
        success(xdestination, xsource);
        check(xdestination.storage == normalized, "X-format unused bits stopped normalizing");
    }
    Surface zero_width(0, 3, D3DFMT_A8R8G8B8), zero_height(3, 0, D3DFMT_A8R8G8B8);
    check(copy(destination, nullptr, zero_width, nullptr) == E_INVALIDARG && changes.empty(),
          "zero-width source bypassed failure");
    check(copy(zero_height, nullptr, source, nullptr) == E_INVALIDARG && changes.empty(),
          "zero-height destination bypassed failure");
}
} // namespace

bool th08_linux_surface_access(IDirect3DSurface8 *raw, LinuxSurfaceAccess *out, bool read) {
    auto *surface = dynamic_cast<Surface *>(raw);
    if (!surface || !out)
        return false;
    accesses.push_back({surface, read});
    *out = {surface->width && surface->height ? surface->pixels() : nullptr, surface->width,
            surface->height, surface->pitch, surface->format};
    return true;
}

void th08_linux_surface_changed(IDirect3DSurface8 *raw) {
    auto *surface = dynamic_cast<Surface *>(raw);
    check(surface != nullptr, "surface notification has invalid ownership");
    changes.push_back(surface);
}

int main() {
    identity(D3DFMT_A8R8G8B8, 257, 3);
    identity(D3DFMT_A4R4G4B4, 256, 256); // Every possible packed word, including alpha.
    identity(D3DFMT_R5G6B5, 256, 256);
    self_overlap();
    fallbacks();
    std::cout << "Headless surface copy byte-oracle checks passed\n";
}
