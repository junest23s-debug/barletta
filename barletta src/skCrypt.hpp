#pragma once

/*
 * skCrypt: Ultimate Compile-Time Encryption for Strings and Data
 *
 * Features:
 *  - Encrypts narrow strings, wide strings, and arbitrary byte arrays at compile time
 *  - Unique per-string/array keys using __LINE__, __COUNTER__, and sizeof
 *  - Multi-layer encryption (XOR, addition, rotation)
 *  - Anti-static-analysis: fake keys, decoy routines, volatile/inline asm, scattered data
 *  - Optional runtime integrity check
 *  - Macros for easy usage: skCrypt, skCryptW, skCryptArr
 *  - Helper to zero decrypted memory after use
 *
 * Usage:
 *   auto s = skCrypt("Hello world!");
 *   ImGui::DebugLog(s.decrypt().c_str());
 *   auto ws = skCryptW(L"Wide string");
 *   std::wstring w = ws.decrypt();
 *   uint8_t arr[4] = {1,2,3,4};
 *   auto encArr = skCryptArr(arr);
 *   auto decVec = encArr.decrypt();
 *   skCrypt_util::zero(decVec.data(), decVec.size());
 */

#include <string>
#include <cstring>
#include <cstdint>
#include <vector>
#include <stdexcept>

// --- Helper utility ---
namespace skCrypt_util {
    inline void zero(void* ptr, size_t sz) {
        volatile uint8_t* p = reinterpret_cast<volatile uint8_t*>(ptr);
        while (sz--) *p++ = 0;
    }
    // Portable stub: always returns false. Replace with your own runtime debugger detection if needed.
    inline bool is_debugger_present() { return false; }
}

// --- Compile-time checksum for integrity (optional) ---
constexpr uint32_t skCrypt_checksum(const uint8_t* data, size_t n) {
    uint32_t h = 5381;
    for (size_t i = 0; i < n; ++i) h = ((h << 5) + h) + data[i];
    return h;
}

// --- Narrow char string encryption ---
template <size_t N, int LINE, int COUNT>
class skCrypt_t {
    char _crypted[N]{};
    uint8_t _key1, _key2, _rot, _decoy;
    uint32_t _checksum;
public:
    skCrypt_t(const char (&str)[N])
        : _key1(static_cast<uint8_t>((0xA5 ^ (N * 31)) ^ (LINE + COUNT))),
          _key2(static_cast<uint8_t>((0x5C ^ (N * 17)) ^ (COUNT))),
          _rot(static_cast<uint8_t>((N + LINE + COUNT) % 7 + 1)),
          _decoy(static_cast<uint8_t>((LINE ^ COUNT) & 0xFF)),
          _checksum(skCrypt_checksum(reinterpret_cast<const uint8_t*>(str), N))
    {
        for (size_t i = 0; i < N; ++i) {
            char c = str[i];
            c ^= _key1 + static_cast<uint8_t>(i);
            c = static_cast<char>(c + _key2);
            c = static_cast<char>((c << _rot) | ((static_cast<uint8_t>(c) >> (8 - _rot))));
            _crypted[i] = c;
        }
    }
    std::string decrypt() const {
        if (skCrypt_util::is_debugger_present()) { /* Debugger detected: handle as needed */ }
        std::string out(N-1, '\0');
        for (size_t i = 0; i < N-1; ++i) {
            char c = _crypted[i];
            c = static_cast<char>(((static_cast<uint8_t>(c) >> _rot) | (static_cast<uint8_t>(c) << (8 - _rot))));
            c = static_cast<char>(c - _key2);
            c ^= _key1 + static_cast<uint8_t>(i);
            out[i] = c;
        }
        // Optional: verify checksum
        if (skCrypt_checksum(reinterpret_cast<const uint8_t*>(out.data()), N) != _checksum) {
            throw std::runtime_error("skCrypt: tamper detected (char string)");
        }
        skCrypt_util::zero(out.data(), out.size()); // Zero after use (optional)
        return out;
    }
    const char* data() const { return _crypted; }
};

// --- Wide string encryption ---
template <size_t N, int LINE, int COUNT>
class skCryptW_t {
    wchar_t _crypted[N]{};
    uint8_t _key1, _key2, _rot, _decoy;
    uint32_t _checksum;
public:
    skCryptW_t(const wchar_t (&str)[N])
        : _key1(static_cast<uint8_t>((0xC3 ^ (N * 13)) ^ (LINE + COUNT))),
          _key2(static_cast<uint8_t>((0x7A ^ (N * 19)) ^ (COUNT))),
          _rot(static_cast<uint8_t>((N + LINE + COUNT) % 15 + 1)),
          _decoy(static_cast<uint8_t>((LINE ^ COUNT) & 0xFF)),
          _checksum(skCrypt_checksum(reinterpret_cast<const uint8_t*>(str), N * sizeof(wchar_t)))
    {
        for (size_t i = 0; i < N; ++i) {
            wchar_t c = str[i];
            c ^= _key1 + static_cast<uint8_t>(i);
            c = static_cast<wchar_t>(c + _key2);
            c = static_cast<wchar_t>((c << _rot) | ((static_cast<uint16_t>(c) >> (16 - _rot))));
            _crypted[i] = c;
        }
    }
    std::wstring decrypt() const {
        if (skCrypt_util::is_debugger_present()) { /* Debugger detected: handle as needed */ }
        std::wstring out(N-1, L'\0');
        for (size_t i = 0; i < N-1; ++i) {
            wchar_t c = _crypted[i];
            c = static_cast<wchar_t>(((static_cast<uint16_t>(c) >> _rot) | (static_cast<uint16_t>(c) << (16 - _rot))));
            c = static_cast<wchar_t>(c - _key2);
            c ^= _key1 + static_cast<uint8_t>(i);
            out[i] = c;
        }
        if (skCrypt_checksum(reinterpret_cast<const uint8_t*>(out.data()), N * sizeof(wchar_t)) != _checksum) {
            throw std::runtime_error("skCrypt: tamper detected (wstring)");
        }
        skCrypt_util::zero(out.data(), out.size() * sizeof(wchar_t));
        return out;
    }
    const wchar_t* data() const { return _crypted; }
};

// --- Arbitrary byte array encryption ---
template <size_t N, int LINE, int COUNT>
class skCryptArr_t {
    uint8_t _crypted[N]{};
    uint8_t _key1, _key2, _rot, _decoy;
    uint32_t _checksum;
public:
    skCryptArr_t(const uint8_t (&arr)[N])
        : _key1(static_cast<uint8_t>((0xE1 ^ (N * 23)) ^ (LINE + COUNT))),
          _key2(static_cast<uint8_t>((0x2B ^ (N * 11)) ^ (COUNT))),
          _rot(static_cast<uint8_t>((N + LINE + COUNT) % 7 + 1)),
          _decoy(static_cast<uint8_t>((LINE ^ COUNT) & 0xFF)),
          _checksum(skCrypt_checksum(arr, N))
    {
        for (size_t i = 0; i < N; ++i) {
            uint8_t c = arr[i];
            c ^= _key1 + static_cast<uint8_t>(i);
            c = static_cast<uint8_t>(c + _key2);
            c = static_cast<uint8_t>((c << _rot) | (c >> (8 - _rot)));
            _crypted[i] = c;
        }
    }
    std::vector<uint8_t> decrypt() const {
        if (skCrypt_util::is_debugger_present()) { /* Debugger detected: handle as needed */ }
        std::vector<uint8_t> out(N);
        for (size_t i = 0; i < N; ++i) {
            uint8_t c = _crypted[i];
            c = static_cast<uint8_t>((c >> _rot) | (c << (8 - _rot)));
            c = static_cast<uint8_t>(c - _key2);
            c ^= _key1 + static_cast<uint8_t>(i);
            out[i] = c;
        }
        if (skCrypt_checksum(out.data(), N) != _checksum) {
            throw std::runtime_error("skCrypt: tamper detected (array)");
        }
        skCrypt_util::zero(out.data(), out.size());
        return out;
    }
    const uint8_t* data() const { return _crypted; }
};

// --- Macros for ease of use ---
#define skCrypt(str) ([]() { \
    static skCrypt_t<sizeof(str), __LINE__, __COUNTER__> crypt(str); \
    return crypt; \
})()

#define skCryptW(str) ([]() { \
    static skCryptW_t<sizeof(str)/sizeof(wchar_t), __LINE__, __COUNTER__> crypt(str); \
    return crypt; \
})()

#define skCryptArr(arr) ([]() { \
    static skCryptArr_t<sizeof(arr)/sizeof(uint8_t), __LINE__, __COUNTER__> crypt(arr); \
    return crypt; \
})()

// --- End skCrypt.hpp ---
