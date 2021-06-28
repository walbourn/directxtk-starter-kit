//
// Game.cpp
//

#include "pch.h"
#include "Game.h"

extern void ExitGame() noexcept;

using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

Game::Game() noexcept(false) :
    m_retryAudio(false)
{
    m_deviceResources = std::make_unique<DX::DeviceResources>(
        DXGI_FORMAT_R10G10B10A2_UNORM,
        DXGI_FORMAT_D32_FLOAT,
        3,
        D3D_FEATURE_LEVEL_11_0);
    m_deviceResources->RegisterDeviceNotify(this);

    m_hdrScene = std::make_unique<DX::RenderTexture>(DXGI_FORMAT_R11G11B10_FLOAT);
#ifdef BUILD_DX12
    XMVECTORF32 color;
    color.v = XMColorSRGBToRGB(Colors::CornflowerBlue);
    m_hdrScene->SetClearColor(color);
#endif
}

Game::~Game()
{
#ifdef BUILD_DX12
    if (m_deviceResources)
    {
        m_deviceResources->WaitForGpu();
    }
#endif

    if (m_audEngine)
    {
        m_audEngine->Suspend();
    }
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
    // Graphics device
    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

    // Input devices
    m_gamePad = std::make_unique<GamePad>();
    m_keyboard = std::make_unique<Keyboard>();
    m_mouse = std::make_unique<Mouse>();
    m_mouse->SetWindow(window);

    // Audio device
    AUDIO_ENGINE_FLAGS eflags = AudioEngine_Default;
#ifdef _DEBUG
    eflags |= AudioEngine_Debug;
#endif
    m_audEngine = std::make_unique<AudioEngine>(eflags);
}

#pragma region Frame Update
// Executes the basic game loop.
void Game::Tick()
{
    m_timer.Tick([&]()
    {
        Update(m_timer);
    });

    Render();

    // Only update audio once per render
    if (m_retryAudio)
    {
        m_retryAudio = false;
        if (m_audEngine->Reset())
        {
            // TODO: restart any looped sounds here
        }
    }
    else if (!m_audEngine->Update())
    {
        if (m_audEngine->IsCriticalError())
        {
            m_retryAudio = true;
        }
    }
}

// Updates the world.
void Game::Update(DX::StepTimer const&)
{
    PIXBeginEvent(PIX_COLOR_DEFAULT, L"Update");

    // TODO -

    PIXEndEvent();
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void Game::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

#ifdef BUILD_DX12
    m_deviceResources->Prepare();

    auto commandList = m_deviceResources->GetCommandList();
    PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Render");
    m_hdrScene->BeginScene(commandList);

    Clear();
    // TODO -

    m_hdrScene->EndScene(commandList);
    PIXEndEvent(commandList);

    PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Postprocess");
    auto rtvDescriptor = m_deviceResources->GetRenderTargetView();
    commandList->OMSetRenderTargets(1, &rtvDescriptor, FALSE, nullptr);

    ID3D12DescriptorHeap* heaps[] = { m_resourceDescriptors->Heap() };
    commandList->SetDescriptorHeaps(static_cast<UINT>(std::size(heaps)), heaps);

    m_toneMap->Process(commandList);

    PIXEndEvent(commandList);
    PIXBeginEvent(PIX_COLOR_DEFAULT, L"Present");
    m_deviceResources->Present();
    m_graphicsMemory->Commit(m_deviceResources->GetCommandQueue());
    PIXEndEvent();

#else // BUILD_DX11
    Clear();

    auto context = m_deviceResources->GetD3DDeviceContext();

    // TODO -

    auto renderTarget = m_deviceResources->GetRenderTargetView();
    context->OMSetRenderTargets(1, &renderTarget, nullptr);

    m_toneMap->Process(context);

    ID3D11ShaderResourceView* nullsrv[] = { nullptr };
    context->PSSetShaderResources(0, 1, nullsrv);

    m_deviceResources->Present();
#endif
}

// Helper method to clear the back buffers.
void Game::Clear()
{
#ifdef BUILD_DX12
    // Clear the views.
    auto rtvDescriptor = m_renderDescriptors->GetCpuHandle(RTDescriptors::HDRScene);
    auto dsvDescriptor = m_deviceResources->GetDepthStencilView();

    auto commandList = m_deviceResources->GetCommandList();
    PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Clear");
    commandList->OMSetRenderTargets(1, &rtvDescriptor, FALSE, &dsvDescriptor);
    m_hdrScene->Clear(commandList);
    commandList->ClearDepthStencilView(dsvDescriptor, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // Set the viewport and scissor rect.
    auto viewport = m_deviceResources->GetScreenViewport();
    auto scissorRect = m_deviceResources->GetScissorRect();
    commandList->RSSetViewports(1, &viewport);
    commandList->RSSetScissorRects(1, &scissorRect);
#else
    // Clear the views.
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTarget = m_hdrScene->GetRenderTargetView();
    auto depthStencil = m_deviceResources->GetDepthStencilView();

    XMVECTORF32 color;
    color.v = XMColorSRGBToRGB(Colors::CornflowerBlue);
    context->ClearRenderTargetView(renderTarget, color);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    // Set the viewport.
    auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);
#endif
}
#pragma endregion

#pragma region Message Handlers
// Message handlers
void Game::OnActivated()
{
}

void Game::OnDeactivated()
{
}

void Game::OnSuspending()
{
    m_gamePad->Suspend();
    m_audEngine->Suspend();
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();
    m_audEngine->Resume();
    m_gamePad->Resume();
}

void Game::OnWindowMoved()
{
    auto r = m_deviceResources->GetOutputSize();
    m_deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void Game::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();
}

// Properties
void Game::GetDefaultSize(int& width, int& height) const noexcept
{
    width = 1280;
    height = 720;
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Game::CreateDeviceDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();

#ifdef BUILD_DX12
    // Check Shader Model 6 support
    D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = { D3D_SHADER_MODEL_6_0 };
    if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel)))
        || (shaderModel.HighestShaderModel < D3D_SHADER_MODEL_6_0))
    {
#ifdef _DEBUG
        OutputDebugStringA("ERROR: Shader Model 6.0 is not supported!\n");
#endif
        throw std::runtime_error("Shader Model 6.0 is not supported!");
    }

    m_graphicsMemory = std::make_unique<GraphicsMemory>(device);

    m_resourceDescriptors = std::make_unique<DescriptorHeap>(device,
        Descriptors::Count);

    m_renderDescriptors = std::make_unique<DescriptorHeap>(device,
        D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
        D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
        RTDescriptors::RTCount);

    m_hdrScene->SetDevice(device,
        m_resourceDescriptors->GetCpuHandle(Descriptors::SceneTex),
        m_renderDescriptors->GetCpuHandle(RTDescriptors::HDRScene));

    RenderTargetState rtState(m_deviceResources->GetBackBufferFormat(),
        DXGI_FORMAT_UNKNOWN);

    // Set tone-mapper as 'pass-through' for now...
    m_toneMap = std::make_unique<ToneMapPostProcess>(device,
        rtState,
        ToneMapPostProcess::None, ToneMapPostProcess::SRGB);
#else
    m_hdrScene->SetDevice(device);

    m_toneMap = std::make_unique<ToneMapPostProcess>(device);

    // Set tone-mapper as 'pass-through' for now...
    m_toneMap->SetOperator(ToneMapPostProcess::None);
    m_toneMap->SetTransferFunction(ToneMapPostProcess::SRGB);
#endif
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    auto size = m_deviceResources->GetOutputSize();
    m_hdrScene->SetWindow(size);

#ifdef BUILD_DX12
    auto sceneTex = m_resourceDescriptors->GetGpuHandle(Descriptors::SceneTex);
    m_toneMap->SetHDRSourceTexture(sceneTex);
#else
    m_toneMap->SetHDRSourceTexture(m_hdrScene->GetShaderResourceView());
#endif
}

void Game::OnDeviceLost()
{
#ifdef BUILD_DX12
    m_graphicsMemory.reset();
    m_resourceDescriptors.reset();
    m_renderDescriptors.reset();
#endif
    m_hdrScene->ReleaseDevice();
    m_toneMap.reset();
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion
