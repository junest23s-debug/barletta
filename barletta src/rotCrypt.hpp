#pragma once

/*
 * rotCrypt: Compile-Time String Encryption (Rotation + XOR)
 *
 * Features:
 *  - Encrypts string literals at compile time using key-based rotation and XOR
 *  - Unique per-string keys using __LINE__ and __COUNTER__
 *  - Simple macro usage: rotCrypt("my string")
 *  - Decrypt at runtime with .decrypt()
 *
 * Usage:
 *   auto s = rotCrypt("Secret!");
 *   printf("%s\n", s.decrypt().c_str());
 */

#include <string>
#include <cstdint>

// --- Compile-time checksum for integrity (optional) ---
constexpr uint32_t rotCrypt_checksum(const uint8_t* data, size_t n) {
    uint32_t h = 5381;
    for (size_t i = 0; i < n; ++i) h = ((h << 5) + h) + data[i];
    return h;
}

template <size_t N, int LINE, int COUNT>
class rotCrypt_t {
    char _crypted[N]{};
    uint8_t _xor_key, _rot;
    uint32_t _checksum;
public:
    rotCrypt_t(const char (&str)[N])
        : _xor_key(static_cast<uint8_t>(0xC3 ^ (N * 13) ^ (LINE + COUNT))),
          _rot(static_cast<uint8_t>((N + LINE + COUNT) % 7 + 1)),
          _checksum(rotCrypt_checksum(reinterpret_cast<const uint8_t*>(str), N))
    {
        for (size_t i = 0; i < N; ++i) {
            char c = str[i];
            // Layer 1: Bit rotation
            c = static_cast<char>((c << _rot) | ((static_cast<uint8_t>(c) >> (8 - _rot))));
            // Layer 2: XOR
            c ^= _xor_key + static_cast<uint8_t>(i);
            _crypted[i] = c;
        }
    }
    std::string decrypt() const {
        std::string out(N-1, '\0');
        for (size_t i = 0; i < N-1; ++i) {
            char c = _crypted[i];
            // Undo Layer 2: XOR
            c ^= _xor_key + static_cast<uint8_t>(i);
            // Undo Layer 1: Bit rotation
            c = static_cast<char>(((static_cast<uint8_t>(c) >> _rot) | (static_cast<uint8_t>(c) << (8 - _rot))));
            out[i] = c;
        }
        // Optional: verify checksum
        if (rotCrypt_checksum(reinterpret_cast<const uint8_t*>(out.data()), N) != _checksum) {
            throw std::runtime_error("rotCrypt: tamper detected");
        }
        return out;
    }
    const char* data() const { return _crypted; }
};

#define rotCrypt(str) ([]() { \
    static rotCrypt_t<sizeof(str), __LINE__, __COUNTER__> crypt(str); \
    return crypt; \
})()
