#pragma once
#include <windows.h>
#include <string>

std::string GetWindowsVersionString();
std::string GetDefaultAudioDeviceName();

// System/Process/User/Audio info helpers
std::string GetCurrentUserName();
DWORD GetCurrentProcessThreadCount();
std::string GetCurrentProcessUptime();
std::string GetCurrentProcessMemoryUsage();
std::string GetBuildDateTime();

// Stub functions for injection info (implement if possible, else return "N/A")
std::string GetSourcePID();
std::string GetInjectionSource();
std::string GetInjectionMethod();
std::string GetInjectionTime();
std::string GetTargetModuleBase();
std::string GetIntegrityLevel();
std::string GetSessionId();

// Audio info helpers (implement as needed)
std::string GetAudioSampleRate();
std::string GetAudioChannels();
std::string GetAudioBufferSize();
std::string GetAudioLastError();

// Expose encoder frame size globals
extern int opusFrameSizeIdx;
extern int opusFrameSize;
