//
// GameComponents.h
//

#pragma once

#include "DeviceResources.h"
#include "StepTimer.h"

#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <utility>
#include <vector>


// An abstract base class for game components
class IGameComponent
{
public:
    virtual ~IGameComponent() {}

    virtual void Initialize(DX::DeviceResources& deviceResources) = 0;

    virtual void Update(DX::StepTimer const& timer) = 0;

    bool m_enabled;
    int m_order;

protected:
    IGameComponent() : m_enabled(true), m_order(0) {}
};


// An abstract base class for game components that can be drawn
class IDrawableGameComponent : public IGameComponent
{
public:
    virtual void Draw() = 0;

    virtual void OnDeviceLost() {}

    virtual void OnDeviceRestored(DX::DeviceResources& deviceResources) { UNREFERENCED_PARAMETER(deviceResources); }

    bool m_hidden;

protected:
    IDrawableGameComponent() : IGameComponent(), m_hidden(false) {}
};


// A class for handling the collection of game components
class GameComponentCollection
{
public:
    GameComponentCollection() : mDirtyOrder(false) {}

    GameComponentCollection(GameComponentCollection&&) = default;
    GameComponentCollection& operator=(GameComponentCollection&&) = default;

    GameComponentCollection(const GameComponentCollection&) = delete;
    GameComponentCollection& operator=(const GameComponentCollection&) = delete;

    ~GameComponentCollection() { Clear(); }

    size_t Count() const noexcept { return mGameComponents.size(); }

    IGameComponent*& operator[](const size_t index) { return mGameComponents[index]; }

    void Add(IGameComponent* item)
    {
        for (auto i : mGameComponents)
        {
            if (item == i)
            {
                throw std::runtime_error("Cannot add the same game component more than once");
            }
        }

        mGameComponents.push_back(item);
        mDirtyOrder = true;
    }

    void Add(IDrawableGameComponent* item)
    {
        for (auto i : mDrawableComponents)
        {
            if (item == i)
            {
                throw std::runtime_error("Cannot add the same game component more than once");
            }
        }

        mGameComponents.push_back(item);
        mDrawableComponents.push_back(item);
        mDirtyOrder = true;
    }

    template<class T, class... Args>
    void Add(Args&&... args)
    {
        auto* ptr = new T(std::forward<Args>(args)...);
        Add(ptr);
    }

    void Remove(IGameComponent* item)
    {
        auto i = std::find(mGameComponents.begin(), mGameComponents.end(), item);

        if (i != mGameComponents.cend())
        {
            auto ptr = *i;
            mGameComponents.erase(i);
            delete ptr;
        }
    }

    void Remove(IDrawableGameComponent* item)
    {
        auto i = std::find(mGameComponents.begin(), mGameComponents.end(), item);

        if (i != mGameComponents.cend())
        {
            auto ptr = *i;
            mGameComponents.erase(i);

            auto j = std::find(mDrawableComponents.begin(), mDrawableComponents.end(), item);
            if (j != mDrawableComponents.cend())
            {
                mDrawableComponents.erase(j);
            }

            delete ptr;
        }
    }

    bool Contains(IGameComponent* item) const noexcept
    {
        auto i = std::find(mGameComponents.cbegin(), mGameComponents.cend(), item);
        return (i != mGameComponents.cend());
    }

    void Clear()
    {
        for (auto i : mGameComponents)
        {
            delete i;
        }

        mGameComponents.clear();
        mDrawableComponents.clear();
    }

    void OrderChanged() noexcept { mDirtyOrder = true; }

    void OnInitialize(DX::DeviceResources& deviceResources)
    {
        for (auto i : mGameComponents)
        {
            i->Initialize(deviceResources);
        }
    }

    void OnUpdate(DX::StepTimer const& timer)
    {
        if (mDirtyOrder)
        {
            mDirtyOrder = false;

            std::stable_sort(mGameComponents.begin(), mGameComponents.end(),
                [](const IGameComponent* item1, const IGameComponent* item2) -> bool
                {
                    return item1->m_order < item2->m_order;
                });
        }

        for (auto i : mGameComponents)
        {
            if (i->m_enabled)
                i->Update(timer);
        }
    }

    void OnDraw()
    {
        for (auto i : mDrawableComponents)
        {
            if (i->m_hidden)
                continue;

            i->Draw();
        }
    }

    void OnDeviceLost()
    {
        for (auto i : mDrawableComponents)
        {
            i->OnDeviceLost();
        }
    }

    void OnDeviceRestored(DX::DeviceResources& deviceResources)
    {
        for (auto i : mDrawableComponents)
        {
            i->OnDeviceRestored(deviceResources);
        }
    }

private:
    bool mDirtyOrder;

    std::vector<IGameComponent*> mGameComponents;
    std::vector<IDrawableGameComponent*> mDrawableComponents;
};
