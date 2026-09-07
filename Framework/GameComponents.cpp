//
// GameComponents.cpp
//
//

#include "pch.h"
#include "GameComponents.h"

#include <algorithm>
#include <stdexcept>

using namespace DX::Framework;

void GameComponentCollection::Add(_In_ IGameComponent* item)
{
    assert(item != nullptr);

    item->SetGame(mGame);

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

void GameComponentCollection::Add(_In_ IDrawableGameComponent* item)
{
    assert(item != nullptr);

    item->SetGame(mGame);

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

void GameComponentCollection::Remove(_In_ IGameComponent* item)
{
    assert(item != nullptr);

    auto i = std::find(mGameComponents.begin(), mGameComponents.end(), item);

    if (i != mGameComponents.cend())
    {
        auto ptr = *i;
        mGameComponents.erase(i);
        delete ptr;
    }
}

void GameComponentCollection::Remove(_In_ IDrawableGameComponent* item)
{
    assert(item != nullptr);

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

bool GameComponentCollection::Contains(_In_ IGameComponent* item) const noexcept
{
    assert(item != nullptr);
    auto i = std::find(mGameComponents.cbegin(), mGameComponents.cend(), item);
    return (i != mGameComponents.cend());
}

void GameComponentCollection::Clear()
{
    for (auto i : mGameComponents)
    {
        delete i;
    }

    mGameComponents.clear();
    mDrawableComponents.clear();
}

void GameComponentCollection::Initialize()
{
    for (auto i : mGameComponents)
    {
        i->Initialize();
    }

    for (auto i : mDrawableComponents)
    {
        i->LoadGraphicsContent();
    }
}

void GameComponentCollection::Update(DX::StepTimer const& timer)
{
    if (mDirtyOrder)
    {
        mDirtyOrder = false;

        std::stable_sort(mGameComponents.begin(),
            mGameComponents.end(),
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

void GameComponentCollection::LoadContent()
{
    for (auto i : mDrawableComponents)
    {
        i->LoadGraphicsContent();
    }
}

void GameComponentCollection::UnloadContent()
{
    for (auto i : mDrawableComponents)
    {
        i->UnloadGraphicsContent();
    }
}
