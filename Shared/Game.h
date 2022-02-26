//
// Game.h
//

#pragma once

#include "DeviceResources.h"
#include "GameComponents.h"
#include "RenderTexture.h"
#include "StepTimer.h"


class Game final : public DX::IDeviceNotify
{
public:

    Game() noexcept(false);
    ~Game();

    Game(Game&&) = default;
    Game& operator= (Game&&) = default;

    Game(Game const&) = delete;
    Game& operator= (Game const&) = delete;

    // Initialization and management
    void Initialize(HWND window, int width, int height);

    // Basic game loop
    void Tick();

    // IDeviceNotify
    void OnDeviceLost() override;
    void OnDeviceRestored() override;

    // Messages
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowMoved();
    void OnDisplayChange();
    void OnWindowSizeChanged(int width, int height);
    void OnNewAudioDevice() noexcept { m_retryAudio = true; }

    // Properties
    void GetDefaultSize( int& width, int& height ) const noexcept;

    // Globally shared resources
#ifdef BUILD_DX12
    std::unique_ptr<DirectX::GraphicsMemory>        m_graphicsMemory;
    std::unique_ptr<DirectX::DescriptorHeap>        m_resourceDescriptors;
    std::unique_ptr<DirectX::DescriptorHeap>        m_renderDescriptors;
#endif
    std::unique_ptr<DirectX::AudioEngine>           m_audEngine;
    std::unique_ptr<DirectX::GamePad>               m_gamePad;
    std::unique_ptr<DirectX::Keyboard>              m_keyboard;
    std::unique_ptr<DirectX::Mouse>                 m_mouse;

    std::unique_ptr<DX::RenderTexture>              m_hdrScene;
    std::unique_ptr<DirectX::ToneMapPostProcess>    m_toneMap;

private:

    void Update(DX::StepTimer const& timer);
    void Render();

    void Clear();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

    // Device resources.
    std::unique_ptr<DX::DeviceResources>    m_deviceResources;

    // Rendering loop timer.
    DX::StepTimer                           m_timer;

    // Audio control
    bool                                    m_retryAudio;

#ifdef BUILD_DX12
    enum Descriptors
    {
        SceneTex,
        Count
    };

    enum RTDescriptors
    {
        HDRScene,
        RTCount
    };
#endif

    GameComponentCollection                 m_components;
};
