//
// DirectX12Game.h
//

#pragma once

#include "Game.h"

class DirectX12Game final : public DX::Framework::Game
{
public:
    DirectX12Game() noexcept(false);
    ~DirectX12Game();

    DirectX12Game(DirectX12Game&&)            = default;
    DirectX12Game& operator=(DirectX12Game&&) = default;

    DirectX12Game(DirectX12Game const&)            = delete;
    DirectX12Game& operator=(DirectX12Game const&) = delete;
};
