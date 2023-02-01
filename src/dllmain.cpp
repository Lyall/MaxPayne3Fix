#include "stdafx.h"
#include "helper.hpp"

using namespace std;

HMODULE baseModule = GetModuleHandle(NULL);

// INI Variables
bool bAspectFix;
bool bMovieFix;
bool bLoadingFix;
bool bCentreHUD;
bool bShadowQuality;
float fAdditionalFOV;
int iCustomResX;
int iCustomResY;
int iInjectionDelay;

// Variables
float fNewX;
float fNewY;
float fNativeAspect = 1.777777791f;
float fPi = 3.14159265358979323846f;
float fNativeUIParent = 0.9300000072f; // Value appears to be hardcoded
float fNewAspect;
string sExeName;
string sGameName;
string sExePath;

// Aspect Ratio Hook
DWORD AspectRatioReturnJMP;
float fAspectRatio;
void __declspec(naked) AspectRatio_CC()
{
    __asm
    {
        fst [fAspectRatio]
        fld dword ptr[esp + 0x08]
        fdiv dword ptr[esp + 0x0C]
        jmp[AspectRatioReturnJMP]
    }
}

// Movie Hook
DWORD MovieFixReturnJMP;
float MovieFix_xmm1;
float two = (float)2;
void __declspec(naked) MovieFix_CC()
{
    __asm
    {
        movss [MovieFix_xmm1], xmm1
        movss xmm1, [esp + 0x24]
        divss xmm1, [esp + 0x3c]
        movss [fAspectRatio], xmm1
        comiss xmm1, [fNativeAspect] // Only jump to movie fix if aspect ratio >1.78. <=1.78 is scaled correctly.
        ja MovieFix
        jmp OriginalCode

    MovieFix:
        movss xmm1, [esp + 0x24] // Move hor res to xmm1
        movss xmm3, [esp + 0x3c] // Move vert res to xmm3
        mulss xmm3, [fNativeAspect] // multiply vert by 1.7778
        subss xmm1, xmm3 // subtract scaled vert from native hor
        divss xmm1, [two] // divide by 2
        movss xmm3, xmm1 // mov offset to xmm3
        movss xmm2, [esp + 0x3c] // mov vert res to xmm2
        subss xmm4, xmm3 // subtract offset
        xorps xmm1, xmm1 // zero xmm1
        movss [esp + 0x70], xmm3
        jmp [MovieFixReturnJMP]

    OriginalCode:
        movss xmm1, [MovieFix_xmm1]
        movss[esp + 0x70], xmm3
        jmp [MovieFixReturnJMP]
    }
}

// Loading UI Hook
DWORD LoadingUIReturnJMP;
float fAspectDivisor;
float LoadingUI_xmm0;
void __declspec(naked) LoadingUI_CC()
{
    __asm
    {
        movss [LoadingUI_xmm0], xmm0
        movss xmm0, [fAspectRatio]
        comiss xmm0, [fNativeAspect]
        ja LoadingUI
        jmp OriginalCode

    LoadingUI :
        fld dword ptr[fAspectRatio]
        fdiv[fNativeAspect]
        fstp[fAspectDivisor]
        fld dword ptr[fNativeUIParent]
        fdiv[fAspectDivisor]
        movss xmm0, [LoadingUI_xmm0]
        jmp[LoadingUIReturnJMP]

    OriginalCode:
        movss xmm0, [LoadingUI_xmm0]
        fld dword ptr[fNativeUIParent]
        jmp[LoadingUIReturnJMP]
    }
}

// HUD Hook
DWORD HUDReturnJMP;
void __declspec(naked) HUD_CC()
{
    __asm
    {
        // Centre HUD
        fld dword ptr[fAspectRatio]
        fdiv[fNativeAspect]
        fstp[fAspectDivisor]
        fld dword ptr[fNativeUIParent]
        fdiv[fAspectDivisor]
        jmp[HUDReturnJMP]
    }
}

// Gameplay FOV Hook
DWORD GameplayFOVReturnJMP;
float GameplayFOVValue;
void __declspec(naked) GameplayFOV_CC()
{
    __asm
    {
            //movss xmm1, [esi + 0x60]
            movss xmm1, [GameplayFOVValue]
            jmp [GameplayFOVReturnJMP]
    }
}

// Shadow Quality Hook
DWORD ShadowQualityReturnJMP;
void __declspec(naked) ShadowQuality_CC()
{
    __asm
    {
        mov dword ptr [ecx + 0xE4], 00000004
        mov edx, 00000004
        jmp [ShadowQualityReturnJMP]
    }
}

void Logging()
{
    loguru::add_file("MaxPayne3Fix.log", loguru::Truncate, loguru::Verbosity_MAX);
    loguru::set_thread_name("Main");
}

void ReadConfig()
{
    // Initialize config
    INIReader config("MaxPayne3Fix.ini");

    iInjectionDelay = config.GetInteger("MaxPayne3Fix Parameters", "InjectionDelay", 2000);
    bAspectFix = config.GetBoolean("Fix Aspect", "Enabled", true);
    bMovieFix = config.GetBoolean("Fix FMVs", "Enabled", true);
    bLoadingFix = config.GetBoolean("Fix Loading Screens", "Enabled", true);
    bCentreHUD = config.GetBoolean("Centred 16:9 HUD", "Enabled", false);
    //bShadowQuality = config.GetBoolean("Increase Shadow Quality", "Enabled", false);
    fAdditionalFOV = config.GetFloat("Gameplay FOV", "AdditionalFOV", 0);

    // Grab desktop resolution
    RECT desktop;
    GetWindowRect(GetDesktopWindow(), &desktop);
    fNewX = (float)desktop.right;
    fNewY = (float)desktop.bottom;
    fNewAspect = (float)desktop.right / (float)desktop.bottom;

    // Get game name and exe path
    LPWSTR exePath = new WCHAR[_MAX_PATH];
    GetModuleFileNameW(baseModule, exePath, _MAX_PATH);
    wstring exePathWString(exePath);
    sExePath = string(exePathWString.begin(), exePathWString.end());
    wstring wsGameName = Memory::GetVersionProductName();
    sExeName = sExePath.substr(sExePath.find_last_of("/\\") + 1);
    sGameName = string(wsGameName.begin(), wsGameName.end());

    LOG_F(INFO, "Game Name: %s", sGameName.c_str());
    LOG_F(INFO, "Game Path: %s", sExePath.c_str());

    // Check if config failed to load
    if (config.ParseError() != 0) {
        LOG_F(ERROR, "Can't load config file");
        LOG_F(ERROR, "Parse error: %d", config.ParseError());
    }

    // Log config parse
    LOG_F(INFO, "Config Parse: iInjectionDelay: %dms", iInjectionDelay);
    LOG_F(INFO, "Config Parse: bAspectFix: %d", bAspectFix);
    LOG_F(INFO, "Config Parse: bMovieFix: %d", bMovieFix);
    LOG_F(INFO, "Config Parse: bLoadingFix: %d", bLoadingFix);
    LOG_F(INFO, "Config Parse: bCentreHUD: %d", bCentreHUD);
    //LOG_F(INFO, "Config Parse: bShadowQuality: %d", bShadowQuality);
    LOG_F(INFO, "Config Parse: fAdditionalFOV: %.2f", fAdditionalFOV);
    LOG_F(INFO, "Config Parse: fNewX: %.2f", fNewX);
    LOG_F(INFO, "Config Parse: fNewY: %.2f", fNewY);
    LOG_F(INFO, "Config Parse: fNewAspect: %.4f", fNewAspect);
}

void AspectFix()
{
    if (bAspectFix)
    {
        uint8_t* AspectRatioScanResult = Memory::PatternScan(baseModule, "E8 ?? ?? ?? ?? D9 44 ?? ?? D8 74 ?? ?? 83 EC ?? 8B ??");
        if (AspectRatioScanResult)
        {
            DWORD AspectRatioAddress = ((uintptr_t)AspectRatioScanResult + 0x5);
            int AspectRatioHookLength = Memory::GetHookLength((char*)AspectRatioAddress, 6);
            AspectRatioReturnJMP = AspectRatioAddress + AspectRatioHookLength;
            Memory::DetourFunction32((void*)AspectRatioAddress, AspectRatio_CC, AspectRatioHookLength);

            LOG_F(INFO, "Aspect Ratio Check: Hook length is %d bytes", AspectRatioHookLength);
            LOG_F(INFO, "Aspect Ratio Check: Hook address is 0x%" PRIxPTR, (uintptr_t)AspectRatioAddress);
        }
        else if (!AspectRatioScanResult)
        {
            LOG_F(INFO, "Aspect Ratio Check: Pattern scan failed.");
        }

        uint8_t* AspectFixScanResult = Memory::PatternScan(baseModule, "76 ?? 6A ?? 6A ?? 8B ?? E8 ?? ?? ?? ?? F3 0F ?? ?? ?? ?? ?? ??");
        if (AspectFixScanResult)
        {
            Memory::PatchBytes((uintptr_t)AspectFixScanResult, "\xEB", 1);

            LOG_F(INFO, "Aspect Fix: Patched byte(s) at 0x%" PRIxPTR, (uintptr_t)AspectFixScanResult);
        }
        else if (!AspectFixScanResult)
        {
            LOG_F(INFO, "Aspect Fix: Pattern scan failed.");
        }

        // Thank you to Rose @ WSGF Discord for finding and sharing this fix!
        uint8_t* FullscreenBugScanResult = Memory::PatternScan(baseModule, "D9 ?? ?? ?? D9 ?? ?? ?? DC ?? DF ?? DD ?? 76 ?? F3 0F ?? ?? ?? ??");
        if (FullscreenBugScanResult)
        {
            Memory::PatchBytes(((uintptr_t)FullscreenBugScanResult + 0xB), "\xF0", 1);

            LOG_F(INFO, "Fullscreen Bug: Patched byte(s) at 0x%" PRIxPTR, (uintptr_t)FullscreenBugScanResult);
        }
        else if (!FullscreenBugScanResult)
        {
            LOG_F(INFO, "Fullscreen Bug: Pattern scan failed.");
        }
    }
}

void MovieFix()
{
    if (bMovieFix)
    {
        uint8_t* MovieFixScanResult = Memory::PatternScan(baseModule, "52 8D ?? ?? ?? F3 0F ?? ?? F3 0F ?? ?? 50 8B ?? F3 0F ?? ?? ?? ??");
        if (MovieFixScanResult)
        {
            DWORD MovieFixAddress = ((uintptr_t)MovieFixScanResult + 0x10);
            int MovieFixHookLength = Memory::GetHookLength((char*)MovieFixAddress, 5);
            MovieFixReturnJMP = MovieFixAddress + MovieFixHookLength;
            Memory::DetourFunction32((void*)MovieFixAddress, MovieFix_CC, MovieFixHookLength);

            LOG_F(INFO, "Movie Fix: Hook length is %d bytes", MovieFixHookLength);
            LOG_F(INFO, "Movie Fix: Hook address is 0x%" PRIxPTR, (uintptr_t)MovieFixAddress);
        }
        else if (!MovieFixScanResult)
        {
            LOG_F(INFO, "Movie Fix: Pattern scan failed.");
        }
    }
}

void GameplayFOV()
{
    if (fAdditionalFOV > (float)0)
    {
        uint8_t* GameplayFOVScanResult = Memory::PatternScan(baseModule, "F3 0F ?? ?? ?? 32 ?? 8B ?? ?? ?? ?? ?? 80 ?? ?? 30 ?? ?? ?? ?? ?? 8A");
        if (GameplayFOVScanResult)
        {
            GameplayFOVValue = (float)45 + fAdditionalFOV;

            DWORD GameplayFOVAddress = ((uintptr_t)GameplayFOVScanResult);
            int GameplayFOVHookLength = Memory::GetHookLength((char*)GameplayFOVAddress, 4);
            GameplayFOVReturnJMP = GameplayFOVAddress + GameplayFOVHookLength;
            Memory::DetourFunction32((void*)GameplayFOVAddress, GameplayFOV_CC, GameplayFOVHookLength);

            LOG_F(INFO, "Gameplay FOV: Hook length is %d bytes", GameplayFOVHookLength);
            LOG_F(INFO, "Gameplay FOV: Hook address is 0x%" PRIxPTR, (uintptr_t)GameplayFOVAddress);
        }
        else if (!GameplayFOVScanResult)
        {
            LOG_F(INFO, "Gameplay FOV: Pattern scan failed.");
        }
    }
}

void UIFix()
{
    if (bLoadingFix)
    {
        uint8_t* LoadingUIScanResult = Memory::PatternScan(baseModule, "D9 05 ?? ?? ?? ?? D9 1C ?? FF ?? C3 CC ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? CC 56 8B ?? E8 ?? ?? ?? ??");
        if (LoadingUIScanResult)
        {
            DWORD LoadingUIAddress = ((uintptr_t)LoadingUIScanResult);
            int LoadingUIHookLength = Memory::GetHookLength((char*)LoadingUIAddress, 4);
            LoadingUIReturnJMP = LoadingUIAddress + LoadingUIHookLength;
            Memory::DetourFunction32((void*)LoadingUIAddress, LoadingUI_CC, LoadingUIHookLength);

            LOG_F(INFO, "Loading UI: Hook length is %d bytes", LoadingUIHookLength);
            LOG_F(INFO, "Loading UI: Hook address is 0x%" PRIxPTR, (uintptr_t)LoadingUIAddress);
        }
        else if (!LoadingUIScanResult)
        {
            LOG_F(INFO, "Loading UI: Pattern scan failed.");
        }
    }

    if (bCentreHUD)
    {
        uint8_t* HUDScanResult = Memory::PatternScan(baseModule, "D9 05 ?? ?? ?? ?? D9 1C ?? FF D2 C3 CC ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? CC 56 6A");
        if (HUDScanResult)
        {
            DWORD HUDAddress = ((uintptr_t)HUDScanResult);
            int HUDHookLength = Memory::GetHookLength((char*)HUDAddress, 4);
            HUDReturnJMP = HUDAddress + HUDHookLength;
            Memory::DetourFunction32((void*)HUDAddress, HUD_CC, HUDHookLength);

            LOG_F(INFO, "HUD: Hook length is %d bytes", HUDHookLength);
            LOG_F(INFO, "HUD: Hook address is 0x%" PRIxPTR, (uintptr_t)HUDAddress);
        }
        else if (!HUDScanResult)
        {
            LOG_F(INFO, "HUD: Pattern scan failed.");
        }
    }
}

void GraphicsTweaks()
{
    // TODO: Fix this. It's broken right now.
    if (bShadowQuality)
    {
        uint8_t* ShadowQuality2ScanResult = Memory::PatternScan(baseModule, "83 ?? ?? ?? ?? ?? 03 7C ?? 8D ?? ?? 83 ?? ?? 7E ??");  
        uint8_t* ShadowQualityScanResult = Memory::PatternScan(baseModule, "8B 89 ?? ?? ?? ?? ?? 01 00 00 00 D3 ?? 89 15 ?? ?? ?? ?? 8B 87 ?? ?? ?? ??");        
        if (ShadowQualityScanResult && ShadowQuality2ScanResult)
        {
            Memory::PatchBytes(((uintptr_t)ShadowQuality2ScanResult + 0x6), "\x04", 1);

            DWORD ShadowQualityAddress = (((uintptr_t)ShadowQualityScanResult - 0x6A));
            int ShadowQualityHookLength = Memory::GetHookLength((char*)ShadowQualityAddress, 4);
            ShadowQualityReturnJMP = ShadowQualityAddress + ShadowQualityHookLength;
            Memory::DetourFunction32((void*)ShadowQualityAddress, ShadowQuality_CC, ShadowQualityHookLength);

            LOG_F(INFO, "Shadow Quality: Hook length is %d bytes", ShadowQualityHookLength);
            LOG_F(INFO, "Shadow Quality: Hook address is 0x%" PRIxPTR, (uintptr_t)ShadowQualityAddress);
        }
        else if (!ShadowQualityScanResult && !ShadowQuality2ScanResult)
        {
            LOG_F(INFO, "Shadow Quality: Pattern scan failed.");
        }
    } 
}

DWORD __stdcall Main(void*)
{
    Logging();
    ReadConfig();
    AspectFix();
    MovieFix();
    GameplayFOV();
    UIFix();
    //GraphicsTweaks();
    return true; // end thread
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        CreateThread(NULL, 0, Main, 0, NULL, 0);
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

