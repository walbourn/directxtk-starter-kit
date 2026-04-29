This provides a helper class for managing <abbr title="Multisample Anti-aliasing">MSAA</abbr> render target/depth-stencil buffers.

[MSAAHelper.h](https://github.com/Microsoft/DirectXTK12/wiki/MSAAHelper.h)

```cpp
class MSAAHelper
{
public:
    MSAAHelper(DXGI_FORMAT backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM,
        DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D32_FLOAT,
        unsigned int sampleCount = 4);

    void SetDevice(ID3D12Device* device);

    void SizeResources(size_t width, size_t height);

    void ReleaseDevice();

    void Prepare(ID3D12GraphicsCommandList* commandList,
        D3D12_RESOURCE_STATES beforeState = D3D12_RESOURCE_STATE_RESOLVE_SOURCE);

    void Resolve(ID3D12GraphicsCommandList* commandList, ID3D12Resource* backBuffer,
        D3D12_RESOURCE_STATES beforeState = D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATES afterState = D3D12_RESOURCE_STATE_PRESENT);

    void SetWindow(const RECT& rect);

    ID3D12Resource* GetMSAARenderTarget() const { return m_msaaRenderTarget.Get(); }
    ID3D12Resource* GetMSAADepthStencil() const { return m_msaaDepthStencil.Get(); }

    D3D12_CPU_DESCRIPTOR_HANDLE GetMSAARenderTargetView() const
    {
        return m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    }
    D3D12_CPU_DESCRIPTOR_HANDLE GetMSAADepthStencilView() const
    {
        return m_dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    }

    void SetClearColor(DirectX::FXMVECTOR color)
    {
        DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(m_clearColor), color);
    }

    DXGI_FORMAT GetBackBufferFormat() const { return m_backBufferFormat; }
    DXGI_FORMAT GetDepthBufferFormat() const { return m_depthBufferFormat; }
    unsigned int GetSampleCount() const { return m_sampleCount; }

private:
    Microsoft::WRL::ComPtr<ID3D12Device>                m_device;
    Microsoft::WRL::ComPtr<ID3D12Resource>              m_msaaRenderTarget;
    Microsoft::WRL::ComPtr<ID3D12Resource>              m_msaaDepthStencil;
    float                                               m_clearColor[4];

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>        m_rtvDescriptorHeap;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>        m_dsvDescriptorHeap;

    DXGI_FORMAT                                         m_backBufferFormat;
    DXGI_FORMAT                                         m_depthBufferFormat;
    unsigned int                                        m_sampleCount;
    unsigned int                                        m_targetSampleCount;

    size_t                                              m_width;
    size_t                                              m_height;
};
```

[MSAAHelper.cpp](https://github.com/Microsoft/DirectXTK12/wiki/MSAAHelper.cpp)

```cpp
#include "MSAAHelper.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

MSAAHelper::MSAAHelper(DXGI_FORMAT backBufferFormat,
    DXGI_FORMAT depthBufferFormat,
    unsigned int sampleCount) :
        m_clearColor{},
        m_rtvDescriptorHeap{},
        m_dsvDescriptorHeap{},
        m_backBufferFormat(backBufferFormat),
        m_depthBufferFormat(depthBufferFormat),
        m_sampleCount(0),
        m_targetSampleCount(sampleCount),
        m_width(0),
        m_height(0)
{
    if (sampleCount < 2 || sampleCount > D3D12_MAX_MULTISAMPLE_SAMPLE_COUNT)
    {
        throw std::out_of_range("MSAA sample count invalid.");
    }
}


void MSAAHelper::SetDevice(ID3D12Device* device)
{
    if (device == m_device.Get())
        return;

    if (m_device)
    {
        ReleaseDevice();
    }

    {
        D3D12_FEATURE_DATA_FORMAT_SUPPORT formatSupport = { m_backBufferFormat, D3D12_FORMAT_SUPPORT1_NONE, D3D12_FORMAT_SUPPORT2_NONE };
        if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatSupport, sizeof(formatSupport))))
        {
            throw std::exception();
        }

        UINT required = D3D12_FORMAT_SUPPORT1_RENDER_TARGET | D3D12_FORMAT_SUPPORT1_MULTISAMPLE_RESOLVE | D3D12_FORMAT_SUPPORT1_MULTISAMPLE_RENDERTARGET;
        if ((formatSupport.Support1 & required) != required)
        {
            throw std::exception();
        }
    }

    {
        D3D12_FEATURE_DATA_FORMAT_SUPPORT formatSupport = { m_depthBufferFormat, D3D12_FORMAT_SUPPORT1_NONE, D3D12_FORMAT_SUPPORT2_NONE };
        if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatSupport, sizeof(formatSupport))))
        {
            throw std::exception();
        }

        UINT required = D3D12_FORMAT_SUPPORT1_DEPTH_STENCIL | D3D12_FORMAT_SUPPORT1_MULTISAMPLE_RENDERTARGET;
        if ((formatSupport.Support1 & required) != required)
        {
            throw std::exception();
        }
    }

    for (m_sampleCount = m_targetSampleCount; m_sampleCount > 1; m_sampleCount--)
    {
        D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS levels = { m_backBufferFormat, m_sampleCount, D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE, 0u };
        if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &levels, sizeof(levels))))
            continue;

        if (levels.NumQualityLevels > 0)
            break;
    }

    if (m_sampleCount < 2)
    {
        throw std::exception();
    }

    // Create descriptor heaps for render target views and depth stencil views.
    D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc = {};
    rtvDescriptorHeapDesc.NumDescriptors = 1;
    rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

    ThrowIfFailed(device->CreateDescriptorHeap(&rtvDescriptorHeapDesc,
        IID_PPV_ARGS(m_rtvDescriptorHeap.ReleaseAndGetAddressOf())));

    if (m_depthBufferFormat != DXGI_FORMAT_UNKNOWN)
    {
        D3D12_DESCRIPTOR_HEAP_DESC dsvDescriptorHeapDesc = {};
        dsvDescriptorHeapDesc.NumDescriptors = 1;
        dsvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

        ThrowIfFailed(device->CreateDescriptorHeap(&dsvDescriptorHeapDesc,
            IID_PPV_ARGS(m_dsvDescriptorHeap.ReleaseAndGetAddressOf())));
    }

    m_device = device;
}


void MSAAHelper::SizeResources(size_t width, size_t height)
{
    if (width == m_width && height == m_height)
        return;

    if (m_width > UINT32_MAX || m_height > UINT32_MAX)
    {
        throw std::out_of_range("Invalid width/height");
    }

    if (!m_device)
        return;

    m_width = m_height = 0;

    CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);

    DXGI_FORMAT msaaFormat = m_backBufferFormat;

    // Create an MSAA render target
    D3D12_RESOURCE_DESC msaaRTDesc = CD3DX12_RESOURCE_DESC::Tex2D(
        msaaFormat,
        static_cast<UINT64>(width),
        static_cast<UINT>(height),
        1, // This render target view has only one texture.
        1, // Use a single mipmap level
        m_sampleCount
    );
    msaaRTDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    D3D12_CLEAR_VALUE msaaOptimizedClearValue = {};
    msaaOptimizedClearValue.Format = m_backBufferFormat;
    memcpy(msaaOptimizedClearValue.Color, m_clearColor, sizeof(float) * 4);

    ThrowIfFailed(m_device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &msaaRTDesc,
        D3D12_RESOURCE_STATE_RESOLVE_SOURCE,
        &msaaOptimizedClearValue,
        IID_PPV_ARGS(m_msaaRenderTarget.ReleaseAndGetAddressOf())
    ));

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.Format = m_backBufferFormat;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;

    m_device->CreateRenderTargetView(
        m_msaaRenderTarget.Get(), &rtvDesc,
        m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

    if (m_depthBufferFormat != DXGI_FORMAT_UNKNOWN)
    {
        // Create an MSAA depth stencil view
        D3D12_RESOURCE_DESC depthStencilDesc = CD3DX12_RESOURCE_DESC::Tex2D(
            m_depthBufferFormat,
            static_cast<UINT64>(width),
            static_cast<UINT>(height),
            1, // This depth stencil view has only one texture.
            1, // Use a single mipmap level.
            m_sampleCount
        );
        depthStencilDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

        D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
        depthOptimizedClearValue.Format = m_depthBufferFormat;
        depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
        depthOptimizedClearValue.DepthStencil.Stencil = 0;

        ThrowIfFailed(m_device->CreateCommittedResource(
            &heapProperties,
            D3D12_HEAP_FLAG_NONE,
            &depthStencilDesc,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &depthOptimizedClearValue,
            IID_PPV_ARGS(m_msaaDepthStencil.ReleaseAndGetAddressOf())
        ));

        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.Format = m_depthBufferFormat;
        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;

        m_device->CreateDepthStencilView(
            m_msaaDepthStencil.Get(), &dsvDesc,
            m_dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
    }

    m_width = width;
    m_height = height;
}


void MSAAHelper::ReleaseDevice()
{
    m_rtvDescriptorHeap.Reset();
    m_dsvDescriptorHeap.Reset();

    m_msaaRenderTarget.Reset();
    m_msaaDepthStencil.Reset();

    m_device.Reset();

    m_width = m_height = 0;
}


void MSAAHelper::Prepare(ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES beforeState)
{
    D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_msaaRenderTarget.Get(),
        beforeState,
        D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList->ResourceBarrier(1, &barrier);
}


void MSAAHelper::Resolve(ID3D12GraphicsCommandList* commandList, ID3D12Resource* backBuffer,
    D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState)
{
    D3D12_RESOURCE_BARRIER barriers[2] =
    {
        CD3DX12_RESOURCE_BARRIER::Transition(m_msaaRenderTarget.Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_RESOLVE_SOURCE),
        CD3DX12_RESOURCE_BARRIER::Transition(backBuffer,
            beforeState,
            D3D12_RESOURCE_STATE_RESOLVE_DEST)
    };

    commandList->ResourceBarrier((beforeState != D3D12_RESOURCE_STATE_RESOLVE_DEST) ? 2u : 1u, barriers);

    commandList->ResolveSubresource(backBuffer, 0, m_msaaRenderTarget.Get(), 0, m_backBufferFormat);

    if (afterState != D3D12_RESOURCE_STATE_RESOLVE_DEST)
    {
        D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            backBuffer,
            D3D12_RESOURCE_STATE_RESOLVE_DEST,
            afterState);
        commandList->ResourceBarrier(1, &barrier);
    }
}


void MSAAHelper::SetWindow(const RECT& output)
{
    // Determine the render target size in pixels.
    auto width = size_t(std::max<LONG>(output.right - output.left, 1));
    auto height = size_t(std::max<LONG>(output.bottom - output.top, 1));

    SizeResources(width, height);
}
```

# Example

In your **Game.h** header file, add:

```cpp
#include "MSAAHelper.h"
```

And then add a variable declaration to the private section of your **Game** class:

```cpp
std::unique_ptr<DX::MSAAHelper> m_msaaHelper;
```

For MSAA rendering, you typically use a MSAA depth/stencil buffer. Therefore, you should create the [[DeviceResources]] instance without a depth buffer (which is non-MSAA) by passing ``DXGI_FORMAT_UNKNOWN`` for the depth-buffer format. In the **Game** constructor modify:

```cpp
// Depth-buffer managed by MSAAHelper.
m_deviceResources = std::make_unique<DX::DeviceResources>(
    DXGI_FORMAT_B8G8R8A8_UNORM,
    DXGI_FORMAT_UNKNOWN);
m_deviceResources->RegisterDeviceNotify(this);
```

Then add (where ``MSAA_COUNT`` is your target sample count)

```
m_msaaHelper = std::make_unique<DX::MSAAHelper>(
    m_deviceResources->GetBackBufferFormat(),
    DXGI_FORMAT_D32_FLOAT,
    MSAA_COUNT);

m_msaaHelper->SetClearColor(Colors::CornflowerBlue);
```

In the **CreateDeviceDependentResources** method, add:

```cpp
// Set the MSAA device. Note this updates GetSampleCount.
m_msaaHelper->SetDevice(device);
```

In the **CreateWindowSizeDependentResources** method, call:

```cpp
auto size = m_deviceResources->GetOutputSize();

// Set window size for MSAA.
m_msaaHelper->SetWindow(size);
```

Make sure to update your render target state when creating any <abbr title="Pipeline State Objects">PSOs</abbr> for rendering as MSAA:

```cpp
RenderTargetState rtState(m_deviceResources->GetBackBufferFormat(),
    m_msaaHelper->GetDepthBufferFormat());
rtState.sampleDesc.Count = m_msaaHelper->GetSampleCount();
```

Since you are rendering to the MSAA render target & depth/stencil buffer rather than to the standard DeviceResources instances, update your **Clear** method as follows:

```cpp
// Clear the views.
...

auto rtvDescriptor = m_msaaHelper->GetMSAARenderTargetView();
auto dsvDescriptor = m_msaaHelper->GetMSAADepthStencilView();
```

The **Render** function should work as written, but the Prepare and Present calls will need updated:

```cpp
// Prepare the command list to render a new frame.
m_deviceResources->Prepare();
auto commandList = m_deviceResources->GetCommandList();
m_msaaHelper->Prepare(commandList);

...
// Show the new frame.
m_msaaHelper->Resolve(commandList, m_deviceResources->GetRenderTarget());
m_deviceResources->Present(D3D12_RESOURCE_STATE_PRESENT);
m_graphicsMemory->Commit(m_deviceResources->GetCommandQueue());
```

Be sure to add to your **OnDeviceLost**:

```cpp
// Release MSAA resources.
m_msaaHelper->ReleaseDevice();
```

# Remarks

This helper class uses the 'default' quality for simplicity.

If the requested sample count isn't supported, it will use the largest value that is supported. For example, if you request 8, but the device only supports 4 it will use 4x. If no MSAA sample count is valid for the given formats, it will throw a C++ exception.

You can provide the before state for the backbuffer resource to both Prepare and Resolve as an optional parameter, as well as the after state for the backbuffer buffer when calling Resolve.  Combined with the state parameters on DeviceResources Prepare and Present, you can optimize various MSAA and post-processing state transitions as needed.
