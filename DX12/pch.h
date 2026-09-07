//
// pch.h
// Header for standard system include files.
//

#pragma once

#pragma warning(disable : 4619 4616 26812)
// C4619/4616 #pragma warning warnings
// 26812: The enum type 'x' is unscoped. Prefer 'enum class' over 'enum' (Enum.3).

#include <winsdkver.h>
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif
#include <sdkddkver.h>

// Use the C++ standard templated min/max
#define NOMINMAX

// DirectX apps don't need GDI
#define NODRAWTEXT
#define NOGDI
#define NOBITMAP

// Include <mcx.h> if you need this
#define NOMCX

// Include <winsvc.h> if you need this
#define NOSERVICE

// WinHelp is deprecated
#define NOHELP

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <wrl/client.h>
#include <wrl/event.h>

#include <directx/dxgiformat.h>
#include <directx/d3d12.h>
#include <directx/d3dx12.h>
#include <dxguids/dxguids.h>

#include <dxgi1_6.h>

#include <DirectXMath.h>
#include <DirectXColors.h>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <exception>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <system_error>
#include <tuple>
#include <vector>

#ifdef _DEBUG
#include <dxgidebug.h>
#endif

// WinPixEvent Runtime
#include <pix3.h>

#include "directxtk12/Audio.h"
#include "directxtk12/BufferHelpers.h"
#include "directxtk12/CommonStates.h"
#include "directxtk12/DDSTextureLoader.h"
#include "directxtk12/DescriptorHeap.h"
#include "directxtk12/DirectXHelpers.h"
#include "directxtk12/EffectPipelineStateDescription.h"
#include "directxtk12/Effects.h"
#include "directxtk12/GamePad.h"
#include "directxtk12/GeometricPrimitive.h"
#include "directxtk12/GraphicsMemory.h"
#include "directxtk12/Keyboard.h"
#include "directxtk12/Model.h"
#include "directxtk12/Mouse.h"
#include "directxtk12/PostProcess.h"
#include "directxtk12/PrimitiveBatch.h"
#include "directxtk12/RenderTargetState.h"
#include "directxtk12/ResourceUploadBatch.h"
#include "directxtk12/ScreenGrab.h"
#include "directxtk12/SimpleMath.h"
#include "directxtk12/SpriteBatch.h"
#include "directxtk12/SpriteFont.h"
#include "directxtk12/VertexTypes.h"
#include "directxtk12/WICTextureLoader.h"

#include "config.h"

namespace DX
{
    // Helper class for COM exceptions
    class com_exception : public std::exception
    {
    public:
        com_exception(HRESULT hr) noexcept
            : result(hr)
        {}

        const char* what() const noexcept override
        {
            static char s_str[64] = {};
            sprintf_s(s_str, "Failure with HRESULT of %08X", static_cast<unsigned int>(result));
            return s_str;
        }

    private:
        HRESULT result;
    };

    // Helper utility converts D3D API failures into exceptions.
    inline void ThrowIfFailed(HRESULT hr)
    {
        if (FAILED(hr))
        {
            throw com_exception(hr);
        }
    }
} // namespace DX
