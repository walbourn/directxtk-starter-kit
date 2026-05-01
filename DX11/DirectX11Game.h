//
// DirectX11Game.h
//

#pragma once

#include "Game.h"

class DirectX11Game final : public DX::Framework::Game
{
public:

    DirectX11Game() noexcept(false);
    ~DirectX11Game();

    DirectX11Game(DirectX11Game&&) = default;
    DirectX11Game& operator= (DirectX11Game&&) = default;

    DirectX11Game(DirectX11Game const&) = delete;
    DirectX11Game& operator= (DirectX11Game const&) = delete;
};
