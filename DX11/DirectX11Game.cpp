//
// DirectX11Game.cpp
//

#include "pch.h"
#include "DirectX11Game.h"

#include <memory>

// Indicates to hybrid graphics systems to prefer the discrete part by default
extern "C" {
    __declspec(dllexport) DWORD NvOptimusEnablement                  = 0x00000001;
    __declspec(dllexport) int   AmdPowerXpressRequestHighPerformance = 1;
}

LPCWSTR g_szAppName = L"DirectXTKStarter Kit (DX11)";

DirectX11Game::DirectX11Game() noexcept(false) {}

DirectX11Game::~DirectX11Game() {}

// Entry point
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    auto game = std::make_unique<DirectX11Game>();
    return game->Run(hInstance, lpCmdLine, nCmdShow, g_szAppName);
}
