#pragma once
#include <array>
#include <fstream>
#include <iomanip>
#include <memory>
#include <openssl/evp.h>
#include <sstream>
#include <stdexcept>
#include <string>
namespace wine_shadow {
std::string file_hash(const std::string &path) {
    std::ifstream input(path, std::ios::binary);
    std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)> ctx(EVP_MD_CTX_new(), EVP_MD_CTX_free);
    if (!input || !ctx || EVP_DigestInit_ex(ctx.get(), EVP_sha256(), nullptr) != 1)
        throw std::runtime_error("shadow_identity_failed");
    std::array<char, 65536> bytes;
    while (input) {
        input.read(bytes.data(), bytes.size());
        if (EVP_DigestUpdate(ctx.get(), bytes.data(), std::size_t(input.gcount())) != 1)
            throw std::runtime_error("shadow_identity_failed");
    }
    std::array<unsigned char, 32> digest;
    unsigned length = 0;
    if (!input.eof() || EVP_DigestFinal_ex(ctx.get(), digest.data(), &length) != 1 || length != 32)
        throw std::runtime_error("shadow_identity_failed");
    std::ostringstream output;
    for (auto byte : digest)
        output << std::hex << std::setw(2) << std::setfill('0') << unsigned(byte);
    return output.str();
}
} // namespace wine_shadow
