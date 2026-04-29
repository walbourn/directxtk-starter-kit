This provides a helper class for managing an offscreen render target.

[RenderTexture.h](https://github.com/Microsoft/DirectXTK12/wiki/RenderTexture.h)

```cpp
class RenderTexture
{
public:
    RenderTexture(DXGI_FORMAT format);

    void SetDevice(ID3D12Device* device,
        D3D12_CPU_DESCRIPTOR_HANDLE srvDescriptor,
        D3D12_CPU_DESCRIPTOR_HANDLE rtvDescriptor);

    void SizeResources(size_t width, size_t height);

    void ReleaseDevice();

    void TransitionTo(ID3D12GraphicsCommandList* commandList,
        D3D12_RESOURCE_STATES afterState);

    void BeginScene(ID3D12GraphicsCommandList* commandList)
    {
        TransitionTo(commandList, D3D12_RESOURCE_STATE_RENDER_TARGET);
    }

    void EndScene(ID3D12GraphicsCommandList* commandList)
    {
        TransitionTo(commandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    }

    void Clear(ID3D12GraphicsCommandList* commandList)
    {
        commandList->ClearRenderTargetView(m_rtvDescriptor, m_clearColor, 0, nullptr);
    }

    void SetClearColor(DirectX::FXMVECTOR color)
    {
        DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(m_clearColor), color);
    }

    ID3D12Resource* GetResource() const { return m_resource.Get(); }
    D3D12_RESOURCE_STATES GetCurrentState() const { return m_state; }

    void UpdateState(D3D12_RESOURCE_STATES state) { m_state = state; }
        // Use when a state transition was applied to the resource directly

    void SetWindow(const RECT& rect);

    DXGI_FORMAT GetFormat() const { return m_format; }

private:
    Microsoft::WRL::ComPtr<ID3D12Device> m_device;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_resource;
    D3D12_RESOURCE_STATES m_state;
    D3D12_CPU_DESCRIPTOR_HANDLE m_srvDescriptor;
    D3D12_CPU_DESCRIPTOR_HANDLE m_rtvDescriptor;
    float m_clearColor[4];

    DXGI_FORMAT m_format;

    size_t m_width;
    size_t m_height;
};
```

[RenderTexture.cpp](https://github.com/Microsoft/DirectXTK12/wiki/RenderTexture.cpp)

```cpp
#include "RenderTexture.h"

using namespace DirectX;

using Microsoft::WRL::ComPtr;

RenderTexture::RenderTexture(DXGI_FORMAT format) :
    m_state(D3D12_RESOURCE_STATE_COMMON),
    m_srvDescriptor{},
    m_rtvDescriptor{},
    m_clearColor{},
    m_format(format),
    m_width(0),
    m_height(0)
{
}

void RenderTexture::SetDevice(ID3D12Device* device,
    D3D12_CPU_DESCRIPTOR_HANDLE srvDescriptor,
    D3D12_CPU_DESCRIPTOR_HANDLE rtvDescriptor)
{
    if (device == m_device.Get()
        && srvDescriptor.ptr == m_srvDescriptor.ptr
        && rtvDescriptor.ptr == m_rtvDescriptor.ptr)
        return;

    if (m_device)
    {
        ReleaseDevice();
    }

    {
        D3D12_FEATURE_DATA_FORMAT_SUPPORT formatSupport = { m_format, D3D12_FORMAT_SUPPORT1_NONE, D3D12_FORMAT_SUPPORT2_NONE };
        if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatSupport, sizeof(formatSupport))))
        {
            throw std::runtime_error("CheckFeatureSupport");
        }

        UINT required = D3D12_FORMAT_SUPPORT1_TEXTURE2D | D3D12_FORMAT_SUPPORT1_RENDER_TARGET;
        if ((formatSupport.Support1 & required) != required)
        {
            throw std::runtime_error("RenderTexture");
        }
    }

    if (!srvDescriptor.ptr || !rtvDescriptor.ptr)
    {
        throw std::runtime_error("Invalid descriptors");
    }

    m_device = device;

    m_srvDescriptor = srvDescriptor;
    m_rtvDescriptor = rtvDescriptor;
}

void RenderTexture::SizeResources(size_t width, size_t height)
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

    auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

    D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(m_format,
        static_cast<UINT64>(width),
        static_cast<UINT>(height),
        1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

    D3D12_CLEAR_VALUE clearValue = { m_format, {} };
    memcpy(clearValue.Color, m_clearColor, sizeof(clearValue.Color));

    m_state = D3D12_RESOURCE_STATE_RENDER_TARGET;

    // Create a render target
    ThrowIfFailed(
        m_device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
            &desc,
            m_state, &clearValue,
            IID_PPV_ARGS(m_resource.ReleaseAndGetAddressOf()))
    );

    // Create RTV.
    m_device->CreateRenderTargetView(m_resource.Get(), nullptr, m_rtvDescriptor);

    // Create SRV.
    m_device->CreateShaderResourceView(m_resource.Get(), nullptr, m_srvDescriptor);

    m_width = width;
    m_height = height;
}

void RenderTexture::ReleaseDevice()
{
    m_resource.Reset();
    m_device.Reset();

    m_state = D3D12_RESOURCE_STATE_COMMON;
    m_width = m_height = 0;

    m_srvDescriptor.ptr = m_rtvDescriptor.ptr = 0;
}

void RenderTexture::TransitionTo(ID3D12GraphicsCommandList* commandList,
    D3D12_RESOURCE_STATES afterState)
{
    TransitionResource(commandList, m_resource.Get(), m_state, afterState);
    m_state = afterState;
}

void RenderTexture::SetWindow(const RECT& output)
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
#include "RenderTexture.h"
```

And then add to the private section of your **Game** class:

```cpp
std::unique_ptr<DX::RenderTexture> m_renderTexture;

std::unique_ptr<DirectX::DescriptorHeap> m_resourceDescriptors;
std::unique_ptr<DirectX::DescriptorHeap> m_renderDescriptors;

enum Descriptors
{
    SceneTex,
    Count
};

enum RTDescriptors
{
    OffScreenRT,
    RTCount
};
```

In the **Game** constructor add:

```cpp
m_renderTexture = std::make_unique<DX::RenderTexture>(
    m_deviceResources->GetBackBufferFormat());

// Set optimized clear color.
m_renderTexture->SetClearColor(Colors::CornflowerBlue);
```

> For this example, our render texture is the same size & format as the swapchain, and we are using the depth/stencil buffer created by [[DeviceResources]].

In the **CreateDeviceDependentResources** method, add:

```cpp
m_resourceDescriptors = std::make_unique<DescriptorHeap>(device,
    Descriptors::Count);

m_renderDescriptors = std::make_unique<DescriptorHeap>(device,
    D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
    D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
    RTDescriptors::RTCount);

m_renderTexture->SetDevice(device,
    m_resourceDescriptors->GetCpuHandle(Descriptors::SceneTex),
    m_renderDescriptors->GetCpuHandle(RTDescriptors::OffScreenRT));
```

In the **CreateWindowSizeDependentResources** method, call:

```cpp
auto size = m_deviceResources->GetOutputSize();
m_renderTexture->SetWindow(size);
```

Be sure to add to your **OnDeviceLost**:

```cpp
m_renderTexture->ReleaseDevice();
m_resourceDescriptors.reset();
m_renderDescriptors.reset();
```

Rendering to the offscreen rendering texture, update the **Clear** method as follows:

```cpp
// Clear the views.
...
auto rtvDescriptor = m_renderDescriptors->GetCpuHandle(
    RTDescriptors::OffScreenRT);
auto dsvDescriptor = m_deviceResources->GetDepthStencilView();

commandList->OMSetRenderTargets(1, &rtvDescriptor, FALSE, &dsvDescriptor);
m_renderTexture->Clear(commandList);
commandList->ClearDepthStencilView(dsvDescriptor, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
```

In the **Render** function, our scene is rendered to the offscreen texture. To see it, we need to render using it to the swapchain. For example, you could use a [[SpriteBatch]]:

```cpp
m_renderTexture->BeginScene(commandList);

Clear();

// Render scene

m_renderTexture->EndScene(commandList);

...

auto rtvDescriptor = m_deviceResources->GetRenderTargetView();
commandList->OMSetRenderTargets(1, &rtvDescriptor, FALSE, nullptr);

ID3D12DescriptorHeap* heaps[] = { m_resourceDescriptors->Heap() };
commandList->SetDescriptorHeaps(static_cast<UINT>(std::size(heaps)), heaps);

m_spriteBatch->Begin(commandList);

m_spriteBatch->Draw(
    m_resourceDescriptors->GetGpuHandle(Descriptors::SceneTex),
    GetTextureSize(m_renderTexture->GetResource())
    m_deviceResources->GetOutputSize());

m_spriteBatch->End();

// Show the new frame.
m_deviceResources->Present();
```

You can also use [[PostProcess]]:

```cpp
m_renderTexture->BeginScene(commandList);

Clear();

// Render scene

m_renderTexture->EndScene(commandList);

...

auto rtvDescriptor = m_deviceResources->GetRenderTargetView();
commandList->OMSetRenderTargets(1, &rtvDescriptor, FALSE, nullptr);

ID3D12DescriptorHeap* heaps[] = { m_resourceDescriptors->Heap() };
commandList->SetDescriptorHeaps(static_cast<UINT>(std::size(heaps)), heaps);

m_basicPostProcess->SetSourceTexture(
    m_resourceDescriptors->GetGpuHandle(Descriptors::SceneTex));
m_basicPostProcess->Process(commandList);

// Show the new frame.
m_deviceResources->Present();
```

# Applications

* The primary use of **RenderTexture** is for post-processing and/or tone-mapping the scene before presenting for viewing.

* In addition to post-processing and tone-mapping, you can use **RenderTexture** to resize and/or format-convert textures with the GPU. Note that you will need to call **SetViewport** to the target size of the render texture before rendering.

# Remarks

The **SetWindow** method is a simple wrapper for **SizeResources** which makes it easier to use with ``RECT`` like ``m_deviceResources->GetOutputSize();``. For specific sizes, you can just call **SizeResources** directly.
