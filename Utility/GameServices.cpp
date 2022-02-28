//
// GameServices.cpp
//

#include "pch.h"
#include "GameServices.h"

#if !defined(_CPPRTTI) && !defined(__GXX_RTTI)
#error GameServices requires RTTI
#endif

#include <cassert>
#include <stdexcept>


void GameServiceContainer::AddService(const std::type_info& type, _In_ void* provider)
{
    assert(provider != nullptr);

    auto const& ti = std::type_index(type);

    if (mServices.find(ti) != mServices.end())
    {
        throw std::runtime_error("Can't add the same service more than once");
    }

    mServices[ti] = provider;
}

void GameServiceContainer::RemoveService(const std::type_info& type)
{
    auto it = mServices.find(std::type_index(type));
    if (it != mServices.end())
    {
        mServices.erase(it);
    }
}

void GameServiceContainer::Clear()
{
    mServices.clear();
}

void* GameServiceContainer::GetService(const std::type_info& type) const
{
    auto it = mServices.find(std::type_index(type));
    if (it == mServices.end())
        return nullptr;

    return it->second;
}
