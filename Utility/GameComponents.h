//
// GameComponents.h
//

#pragma once

#include "StepTimer.h"

#include <cassert>
#include <utility>
#include <vector>


class Game;

// An abstract base class for game components
class IGameComponent
{
public:
    IGameComponent(IGameComponent&&) = default;
    IGameComponent& operator= (IGameComponent&&) = default;

    IGameComponent(IGameComponent const&) = default;
    IGameComponent& operator= (IGameComponent const&) = default;

    virtual ~IGameComponent() = default;

    virtual void Initialize() = 0;

    virtual void Update(DX::StepTimer const& timer) = 0;

    // Properties
    bool m_enabled;
    int m_order;

    void SetGame(_In_ Game* game) noexcept { m_game = game; }

protected:
    Game* m_game;

    IGameComponent() :
        m_enabled(true),
        m_order(0),
        m_game(nullptr)
    {
    }
};


// An abstract base class for game components that can be drawn
class IDrawableGameComponent : public IGameComponent
{
public:
    IDrawableGameComponent(IDrawableGameComponent&&) = default;
    IDrawableGameComponent& operator= (IDrawableGameComponent&&) = default;

    IDrawableGameComponent(IDrawableGameComponent const&) = default;
    IDrawableGameComponent& operator= (IDrawableGameComponent const&) = default;

#ifdef BUILD_DX12
    virtual void Draw(_In_ ID3D12GraphicsCommandList* commandList) = 0;
#else
    virtual void Draw(_In_ ID3D11DeviceContext* context) = 0;
#endif

    virtual void LoadGraphicsContent() {}

    virtual void UnloadGraphicsContent() {}

    // Properties
    bool m_hidden;

protected:
    IDrawableGameComponent() :
        IGameComponent(),
        m_hidden(false)
    {
    }
};


// A class for handling the collection of game components
class GameComponentCollection
{
public:
    GameComponentCollection() :
        mGame(nullptr),
        mDirtyOrder(false)
    {
    }

    GameComponentCollection(GameComponentCollection&&) = default;
    GameComponentCollection& operator=(GameComponentCollection&&) = default;

    GameComponentCollection(const GameComponentCollection&) = delete;
    GameComponentCollection& operator=(const GameComponentCollection&) = delete;

    ~GameComponentCollection() { Clear(); }

    IGameComponent*& operator[](const size_t index) { return mGameComponents[index]; }

    void Add(_In_ IGameComponent* item);
    void Add(_In_ IDrawableGameComponent* item);

    template<class T, class... Args>
    void Add(Args&&... args)
    {
        auto* ptr = new T(std::forward<Args>(args)...);
        Add(ptr);
    }

    void Remove(_In_ IGameComponent* item);
    void Remove(_In_ IDrawableGameComponent* item);

    bool Contains(_In_ IGameComponent* item) const noexcept;

    void Clear();

    void OrderChanged() noexcept { mDirtyOrder = true; }

    void Initialize();
    void Update(DX::StepTimer const& timer);

    template<class T>
    void Draw(T* list)
    {
        for (auto i : mDrawableComponents)
        {
            if (i->m_hidden)
                continue;

            i->Draw(list);
        }
    }

    void LoadContent();
    void UnloadContent();

    // Properties
    size_t Count() const noexcept { return mGameComponents.size(); }

    void SetGame(_In_ Game* game) noexcept { mGame = game; }

private:
    Game* mGame;
    bool mDirtyOrder;

    std::vector<IGameComponent*> mGameComponents;
    std::vector<IDrawableGameComponent*> mDrawableComponents;
};
