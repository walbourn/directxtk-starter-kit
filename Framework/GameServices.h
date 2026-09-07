//
// GameServices.h
//

#pragma once

#include <typeinfo>
#include <typeindex>
#include <unordered_map>

namespace DX::Framework
{
    // A class for handling the collection of game services
    //
    // Note: This class does *not* take ownership of the provider object
    class GameServiceContainer
    {
    public:
        GameServiceContainer() = default;

        GameServiceContainer(GameServiceContainer&&)            = default;
        GameServiceContainer& operator=(GameServiceContainer&&) = default;

        GameServiceContainer(const GameServiceContainer&)            = delete;
        GameServiceContainer& operator=(const GameServiceContainer&) = delete;

        ~GameServiceContainer() { Clear(); }

        void AddService(const std::type_info& type, _In_ void* provider);

        template<class T>
        void AddService(T* provider)
        {
            AddService(typeid(T), provider);
        }

        void RemoveService(const std::type_info& type);

        template<class T>
        void RemoveService(T*)
        {
            RemoteService(typeid(T));
        }

        void Clear();

        void* GetService(const std::type_info& type) const;

        template<class T>
        T* GetService()
        {
            return static_cast<T*>(GetService(typeid(T)));
        }

    private:
        std::unordered_map<std::type_index, void*> mServices;
    };

} // namespace DX::Framework
