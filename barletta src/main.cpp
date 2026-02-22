#include <windows.h>
#include <thread>
#include <string>
#include "MinHook.h"
#include "opus/include/opus.h"
#include "XorString.h"
#include "skCrypt.hpp"
#include "offsets.hpp"
#include "other/overlay/overlay.hpp"

typedef HMODULE(WINAPI* LoadLibraryExWType)(LPCWSTR, HANDLE, DWORD);
typedef void(__fastcall* SplittingFilterType)(__int64*, unsigned __int64, __int64);
typedef void(__fastcall* MatchedFilterType)(__int64, __int64, int, __int64, __int64, int, __int64, int, int, int, int, char);

static LoadLibraryExWType g_OriginalLoadLibraryExW = nullptr;
static SplittingFilterType g_OriginalSplittingFilter = nullptr;
static MatchedFilterType g_OriginalMatchedFilter = nullptr;

void __fastcall SplittingFilterHook(__int64* context, unsigned __int64 param1, __int64 param2) {
    if (param1 && param2 == 2) {
        return;
    }

    if ((param2 & 0xFFFFFFFFFFFFFFFEuLL) != 2) {
        return;
    }

    return g_OriginalSplittingFilter(context, param1, param2);
}

void __fastcall MatchedFilterHook(
    __int64 param1, __int64 param2, int param3, 
    __int64 param4, __int64 param5, int param6,
    __int64 param7, int param8, int param9, 
    int param10, int& param11, char& param12) {
    
    if (param11) param11 = 0;
    if (param12) param12 = 0;
    
    return g_OriginalMatchedFilter(param1, param2, param3, param4, param5, param6, param7, param8, param9, param10, param11, param12);
}

int ReturnZero() {
    return 0;
}

HMODULE WINAPI HookedLoadLibraryExW(LPCWSTR moduleName, HANDLE file, DWORD flags) {
    if (!wcsstr(moduleName, L"discord_voice")) {
        return g_OriginalLoadLibraryExW(moduleName, file, flags);
    }

    HMODULE voiceModule = g_OriginalLoadLibraryExW(moduleName, file, flags);
    if (!voiceModule) {
        return nullptr;
    }

    const uintptr_t voiceBase = reinterpret_cast<uintptr_t>(voiceModule);
    
    XorS(hookStatus, "Hook ");
    const bool hookSuccess = MH_CreateHook(
        reinterpret_cast<LPVOID>(voiceBase + 0x863E90),
        custom_opus_encode,
        nullptr
    ) == MH_OK;

    if (hookSuccess) {
        std::thread([] { utilities::ui::start(); }).detach();
    }
    
    std::string narrowStatus = hookStatus.get();
    std::wstring statusMessage(narrowStatus.begin(), narrowStatus.end());
    statusMessage += (hookSuccess ? L"succeeded" : L"failed");
    OutputDebugStringW(statusMessage.c_str());

    struct FunctionHook {
        uintptr_t offset;
        LPVOID detour;
        LPVOID* original;
        const char* name;
    };

    const FunctionHook hooks[] = {
        {0x46869C, reinterpret_cast<LPVOID>(ReturnZero), nullptr, "High Pass Filter"},
        {0x2EF820, reinterpret_cast<LPVOID>(ReturnZero), nullptr, "ProcessStream AudioFrame"},
        {0x2EDFC0, reinterpret_cast<LPVOID>(ReturnZero), nullptr, "ProcessStream StreamConfig"},
        {0x2F2648, reinterpret_cast<LPVOID>(ReturnZero), nullptr, "SendProcessedData"},
        {0x5D8750, reinterpret_cast<LPVOID>(ReturnZero), nullptr, "Clipping Predictor"},
        {0x2E923C, reinterpret_cast<LPVOID>(SplittingFilterHook), reinterpret_cast<LPVOID*>(&g_OriginalSplittingFilter), "Splitting Filter"},
        {0x2E8F00, reinterpret_cast<LPVOID>(MatchedFilterHook), reinterpret_cast<LPVOID*>(&g_OriginalMatchedFilter), "Matched Filter"}
    };

    for (const auto& hook : hooks) {
        if (MH_CreateHook(
            reinterpret_cast<LPVOID>(voiceBase + hook.offset),
            hook.detour,
            hook.original
        ) != MH_OK) {
            OutputDebugStringA((std::string("Failed to hook: ") + hook.name).c_str());
        }
    }

    MH_EnableHook(MH_ALL_HOOKS);
    return voiceModule;
}

void InitializeHooks() {
    HMODULE kernel32 = GetModuleHandleA("kernel32.dll");
    if (!kernel32) {
        OutputDebugStringA("Failed to get kernel32 handle");
        return;
    }
    
    auto loadLibraryExW = GetProcAddress(kernel32, "LoadLibraryExW");
    if (!loadLibraryExW) {
        OutputDebugStringA("Failed to get LoadLibraryExW address");
        return;
    }

    if (MH_Initialize() != MH_OK) {
        OutputDebugStringA("Failed to initialize MinHook");
        return;
    }

    if (MH_CreateHook(
        loadLibraryExW,
        HookedLoadLibraryExW,
        reinterpret_cast<LPVOID*>(&g_OriginalLoadLibraryExW)
    ) != MH_OK) {
        OutputDebugStringA("Failed to create LoadLibraryExW hook");
        return;
    }

    if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK) {
        OutputDebugStringA("Failed to enable hooks");
    }
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID reserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(module);
        std::thread([] {
            InitializeHooks();
        }).detach();
    }
    return TRUE;
}