#ifndef ENTITY_MANAGER_H
#define ENTITY_MANAGER_H

#include <cstddef>  // size_t
#include <array>
#include <cassert>
#include <limits>
#include <tuple>
#include <type_traits>
#include <vector>

#include "pack.h"  // Pack from prep
#include "registry.h"

using EntityID = size_t;

template <typename... ComponentTypes>
class EntityManager {
    using Components = Pack<ComponentTypes...>;
    static constexpr size_t INVALID_IDX = std::numeric_limits<size_t>::max();

    template <typename C> static constexpr size_t componentIndex() {
        static_assert(Components::template contains<C>, "Not a valid component type");
        return static_cast<size_t>(Components::template index<C>);
    }
private:
    std::tuple<Registry<ComponentTypes>...> registries_;
    std::array<std::vector<size_t>, sizeof...(ComponentTypes)> componentIdx_;
    std::vector<bool> alive_;
    std::vector<EntityID> freeIds_;
public:
    EntityID createEntity() {
        EntityID id;
        if (!freeIds_.empty()) { //fill out the blank
            id = freeIds_.back();
            freeIds_.pop_back();
            alive_[id] = true;
            for (auto& v : componentIdx_) {
                v[id] = INVALID_IDX;
            }
        } else {
            id = alive_.size();
            alive_.push_back(true);
            for (auto& v : componentIdx_) {
                v.push_back(INVALID_IDX);
            }
        }
        return id;
    }

    bool validEntity(EntityID id) const {
        return id < alive_.size() && alive_[id];
    }

    template <typename ComponentType> bool hasComponent(EntityID id) {
        assert(validEntity(id));
        return componentIdx_[componentIndex<ComponentType>()][id] != INVALID_IDX;
    }

    template <typename ComponentType> ComponentType& getComponent(EntityID id){
        assert(hasComponent<ComponentType>(id));
        size_t index = componentIdx_[componentIndex<ComponentType>()][id];
        return std::get<Registry<ComponentType>>(registries_)[index];
    }

    template <typename ComponentType> void addComponent(EntityID id, ComponentType&& component) {
        using T = std::remove_cv_t<std::remove_reference_t<ComponentType>>;
        assert(validEntity(id));
        assert(!hasComponent<T>(id));
        size_t ticket = std::get<Registry<T>>(registries_).store(std::forward<ComponentType>(component));
        componentIdx_[componentIndex<T>()][id] = ticket;
    }
};

#endif  // ENTITY_MANAGER_H
