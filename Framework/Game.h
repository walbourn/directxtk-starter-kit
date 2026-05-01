//
// Game.h
//

#pragma once

#include "DeviceResources.h"
#include "GameComponents.h"
#include "GameServices.h"
#include "RenderTexture.h"
#include "StepTimer.h"

namespace DX::Framework
{
    class Game : public DX::IDeviceNotify
    {
    public:

        Game() noexcept(false);
        virtual ~Game();

        Game(Game&&) = default;
        Game& operator= (Game&&) = default;

        Game(Game const&) = delete;
        Game& operator= (Game const&) = delete;

        // Initialization and management
        void Initialize(HWND window, int width, int height);
        void LoadContent();
        void UnloadContent();

        // Basic game loop
        void Tick();
        void SuppressDraw() noexcept { m_suppressDraw = true; }

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
        void OnExiting();
        void OnNewAudioDevice() noexcept { m_retryAudio = true; }

        // Properties
        void GetDefaultSize(int& width, int& height) const noexcept;

        template<class T>
        T* GetService()
        {
            return m_services.GetService<T>();
        }

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

        // Audio device.
        std::unique_ptr<DirectX::AudioEngine>   m_audEngine;

        // Input devices.
        std::unique_ptr<DirectX::GamePad>       m_gamePad;
        std::unique_ptr<DirectX::Keyboard>      m_keyboard;
        std::unique_ptr<DirectX::Mouse>         m_mouse;

        // Private state.
        bool                                    m_retryAudio;
        bool                                    m_suppressDraw;

        // Graphics resources.
        std::unique_ptr<DX::RenderTexture>              m_hdrScene;
        std::unique_ptr<DirectX::ToneMapPostProcess>    m_toneMap;
#ifdef BUILD_DX12
        std::unique_ptr<DirectX::GraphicsMemory>        m_graphicsMemory;
        std::unique_ptr<DirectX::DescriptorPile>        m_resourceDescriptors;
        std::unique_ptr<DirectX::DescriptorPile>        m_renderDescriptors;

        enum Descriptors
        {
            SceneTex,
            Reserve,
            Count = 128
        };

        enum RTDescriptors
        {
            HDRScene,
            RTReserve,
            RTCount = 64
        };
#endif

        GameComponentCollection                 m_components;
        GameServiceContainer                    m_services;

    public:
        int Run(_In_ HINSTANCE hInstance, _In_ LPWSTR lpCmdLine, int nCmdShow, _In_z_ LPCWSTR szAppName);
        void Quit() noexcept;
    };
}
