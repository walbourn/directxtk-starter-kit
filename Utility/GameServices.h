//
// GameServices.h
//

#pragma once

#if !defined(_CPPRTTI) && !defined(__GXX_RTTI)
#error GameServices requires RTTI
#endif

#include <cassert>
#include <stdexcept>
#include <typeinfo>
#include <typeindex>
#include <unordered_map>


class Game;

// A class for handling the collection of game services
//
// Note: This class does *not* take ownership of the provider object
class GameServiceContainer
{
public:
    GameServiceContainer() = default;

    GameServiceContainer(GameServiceContainer&&) = default;
    GameServiceContainer& operator=(GameServiceContainer&&) = default;

    GameServiceContainer(const GameServiceContainer&) = delete;
    GameServiceContainer& operator=(const GameServiceContainer&) = delete;

    ~GameServiceContainer() { Clear(); }

    void AddService(const std::type_info& type, _In_ void* provider)
    {
        assert(provider != nullptr);

        auto const& ti = std::type_index(type);

        if (mServices.find(ti) != mServices.end())
        {
            throw std::runtime_error("Can't add the same service more than once");
        }

        mServices[ti] = provider;
    }

    template<class T>
    void AddService(T* provider)
    {
        AddService(typeid(T), provider);
    }

    void RemoveService(const std::type_info& type)
    {
        auto it = mServices.find(std::type_index(type));
        mServices.erase(it);
    }

    void* GetService(const std::type_info& type) const
    {
        auto it = mServices.find(std::type_index(type));
        if (it == mServices.end())
            return nullptr;

        return it->second;
    }

    template<class T>
    T* GetService()
    {
        return static_cast<T*>(GetService(typeid(T)));
    }

    void Clear()
    {
        mServices.clear();
    }

private:
    std::unordered_map<std::type_index, void*> mServices;
};
