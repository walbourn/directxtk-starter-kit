//
// DirectX12Game.cpp
//

#include "pch.h"
#include "DirectX12Game.h"

#include <memory>

#if defined(USING_D3D12_AGILITY_SDK)
extern "C" {
    // Used to enable the "Agility SDK" components
    __declspec(dllexport) extern const UINT  D3D12SDKVersion;
    __declspec(dllexport) extern const char* D3D12SDKPath;

    const UINT  D3D12SDKVersion = D3D12_SDK_VERSION;
    const char* D3D12SDKPath    = u8".\\D3D12\\";
}
#endif

LPCWSTR g_szAppName = L"DirectXTKStarter Kit (DX12)";

DirectX12Game::DirectX12Game() noexcept(false) {}

DirectX12Game::~DirectX12Game() {}

// Entry point
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    auto game = std::make_unique<DirectX12Game>();
    return game->Run(hInstance, lpCmdLine, nCmdShow, g_szAppName);
}
