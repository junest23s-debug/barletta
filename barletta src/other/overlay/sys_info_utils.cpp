#include "sys_info_utils.hpp"
#include <windows.h>
#include <string>
#include <mmdeviceapi.h>
#include <Functiondiscoverykeys_devpkey.h>
#include <comdef.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <sstream>

// --- USER NAME ---
std::string GetCurrentUserName() {
    char username[256];
    DWORD len = 256;
    if (GetUserNameA(username, &len)) return username;
    return "Unknown User";
}

// --- THREAD COUNT ---
DWORD GetCurrentProcessThreadCount() {
    DWORD pid = GetCurrentProcessId();
    DWORD count = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hSnap != INVALID_HANDLE_VALUE) {
        THREADENTRY32 te;
        te.dwSize = sizeof(te);
        if (Thread32First(hSnap, &te)) {
            do {
                if (te.th32OwnerProcessID == pid) count++;
            } while (Thread32Next(hSnap, &te));
        }
        CloseHandle(hSnap);
    }
    return count;
}

// --- UPTIME ---
std::string GetCurrentProcessUptime() {
    FILETIME create, exit, kernel, user;
    if (GetProcessTimes(GetCurrentProcess(), &create, &exit, &kernel, &user)) {
        ULONGLONG start = (((ULONGLONG)create.dwHighDateTime) << 32) | create.dwLowDateTime;
        ULONGLONG now = 0;
        FILETIME ftNow;
        GetSystemTimeAsFileTime(&ftNow);
        now = (((ULONGLONG)ftNow.dwHighDateTime) << 32) | ftNow.dwLowDateTime;
        ULONGLONG diff100ns = now - start;
        ULONGLONG secs = diff100ns / 10000000ULL;
        int h = (int)(secs / 3600);
        int m = (int)((secs % 3600) / 60);
        int s = (int)(secs % 60);
        char buf[32];
        snprintf(buf, sizeof(buf), "%02d:%02d:%02d", h, m, s);
        return buf;
    }
    return "N/A";
}

// --- MEMORY USAGE ---
std::string GetCurrentProcessMemoryUsage() {
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        double mb = pmc.WorkingSetSize / (1024.0 * 1024.0);
        char buf[32];
        snprintf(buf, sizeof(buf), "%.1f MB", mb);
        return buf;
    }
    return "N/A";
}

// --- BUILD DATE/TIME ---
std::string GetBuildDateTime() {
    std::ostringstream oss;
    oss << __DATE__ << " " << __TIME__;
    return oss.str();
}

// --- INJECTION INFO STUBS ---
std::string GetSourcePID() {
    DWORD pid = GetCurrentProcessId();
    char buf[16];
    snprintf(buf, sizeof(buf), "%lu", pid);
    return buf;
}
std::string GetInjectionSource() {
    char path[MAX_PATH] = {0};
    if (GetModuleFileNameA(NULL, path, MAX_PATH)) {
        std::string full(path);
        size_t pos = full.find_last_of("\\/");
        return (pos != std::string::npos) ? full.substr(pos + 1) : full;
    }
    return "Unknown";
}
std::string GetInjectionMethod() {
    // You can customize this if you know the actual method
    return "Manual";
}
std::string GetInjectionTime() {
    static std::string injectionTime;
    if (injectionTime.empty()) {
        SYSTEMTIME st;
        GetLocalTime(&st);
        char buf[64];
        snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
        injectionTime = buf;
    }
    return injectionTime;
}
std::string GetTargetModuleBase() {
    HMODULE hModule = GetModuleHandle(NULL);
    if (hModule) {
        char buf[32];
        snprintf(buf, sizeof(buf), "0x%p", hModule);
        return buf;
    }
    return "Unknown";
}
std::string GetIntegrityLevel() {
    HANDLE hToken = NULL;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
        return "Unknown";
    DWORD dwLength = 0;
    GetTokenInformation(hToken, TokenIntegrityLevel, NULL, 0, &dwLength);
    PTOKEN_MANDATORY_LABEL pTIL = (PTOKEN_MANDATORY_LABEL)LocalAlloc(0, dwLength);
    if (!GetTokenInformation(hToken, TokenIntegrityLevel, pTIL, dwLength, &dwLength)) {
        CloseHandle(hToken);
        LocalFree(pTIL);
        return "Unknown";
    }
    DWORD dwIntegrityLevel = *GetSidSubAuthority(pTIL->Label.Sid, (DWORD)(*GetSidSubAuthorityCount(pTIL->Label.Sid) - 1));
    std::string level = "Unknown";
    if (dwIntegrityLevel < SECURITY_MANDATORY_MEDIUM_RID)
        level = "Low";
    else if (dwIntegrityLevel < SECURITY_MANDATORY_HIGH_RID)
        level = "Medium";
    else if (dwIntegrityLevel < SECURITY_MANDATORY_SYSTEM_RID)
        level = "High";
    else
        level = "System";
    CloseHandle(hToken);
    LocalFree(pTIL);
    return level;
}
std::string GetSessionId() {
    DWORD sessionId = 0;
    if (ProcessIdToSessionId(GetCurrentProcessId(), &sessionId)) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%lu", sessionId);
        return buf;
    }
    return "Unknown";
}

// --- AUDIO INFO STUBS ---
std::string GetAudioSampleRate() { return "48000 Hz"; } // Replace with real value if available
std::string GetAudioChannels() {
    extern int channels;
    std::string label = (channels == 1) ? "1 (Mono)" : "2 (Stereo)";
    return label;
}
extern int opusFrameSize;
extern int channels;
std::string GetAudioBufferSize() {
    int bytesPerSample = 2; // opus_int16
    int totalBytes = opusFrameSize * channels * bytesPerSample;
    std::string channelLabel = (channels == 1) ? "Mono" : "Stereo";
    char buf[64];
    snprintf(buf, sizeof(buf), "%d samples (%d bytes, %s)", opusFrameSize, totalBytes, channelLabel.c_str());
    return std::string(buf);
} // Replace with real value if available
std::string GetAudioLastError() { return "None"; }

typedef LONG (WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);

std::string GetWindowsVersionString() {
    HMODULE hMod = ::GetModuleHandleW(L"ntdll.dll");
    if (hMod) {
        RtlGetVersionPtr fxPtr = (RtlGetVersionPtr)::GetProcAddress(hMod, "RtlGetVersion");
        if (fxPtr != nullptr) {
            RTL_OSVERSIONINFOW rovi = { 0 };
            rovi.dwOSVersionInfoSize = sizeof(rovi);
            if (fxPtr(&rovi) == 0) {
                char buf[128];
                std::string name = "Windows";
                if (rovi.dwMajorVersion == 10 && rovi.dwMinorVersion == 0) {
                    // Windows 11 is build 22000+ (consumer), Windows 10 is below
                    if (rovi.dwBuildNumber >= 22000 && rovi.dwBuildNumber < 30000) name = "Windows 11";
                    else if (rovi.dwBuildNumber >= 30000) name = "Windows 12 or later";
                    else name = "Windows 10";
                } else if (rovi.dwMajorVersion == 6 && rovi.dwMinorVersion == 3) name = "Windows 8.1";
                else if (rovi.dwMajorVersion == 6 && rovi.dwMinorVersion == 2) name = "Windows 8";
                else if (rovi.dwMajorVersion == 6 && rovi.dwMinorVersion == 1) name = "Windows 7";
                else if (rovi.dwMajorVersion == 6 && rovi.dwMinorVersion == 0) name = "Windows Vista";
                else if (rovi.dwMajorVersion == 5 && rovi.dwMinorVersion == 1) name = "Windows XP";
                else if (rovi.dwMajorVersion > 10) name = "Windows (Future Version)";
                snprintf(buf, sizeof(buf), "%s (Build %lu)", name.c_str(), rovi.dwBuildNumber);
                return buf;
            }
        }
    }
    return "Unknown Windows Version";
}

std::string GetDefaultAudioDeviceName() {
    std::string deviceName = "Unknown Audio Device";
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    IMMDeviceEnumerator* pEnumerator = nullptr;
    IMMDevice* pDevice = nullptr;
    IPropertyStore* pProps = nullptr;
    
    if (SUCCEEDED(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, IID_PPV_ARGS(&pEnumerator)))) {
        if (SUCCEEDED(pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice))) {
            if (SUCCEEDED(pDevice->OpenPropertyStore(STGM_READ, &pProps))) {
                PROPVARIANT varName;
                PropVariantInit(&varName);
                if (SUCCEEDED(pProps->GetValue(PKEY_Device_FriendlyName, &varName))) {
                    // Convert wide string to UTF-8
                    _bstr_t b(varName.pwszVal);
                    deviceName = (const char*)b;
                    PropVariantClear(&varName);
                }
                pProps->Release();
            }
            pDevice->Release();
        }
        pEnumerator->Release();
    }
    if (hr == S_OK) CoUninitialize();
    return deviceName;
}
