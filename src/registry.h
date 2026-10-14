
#ifndef REGISTRY_H
#define REGISTRY_H

#include <cstddef>
#include <vector>
#include <utility>
#include <cassert>

template <typename T>
class Registry {
private:
    struct Slot{
        T value;
    };
    std::vector<Slot> data_;
    std::vector<bool> alive_; // if tickets are activate
    std::vector<size_t> free_; // freed tickets
    size_t count_ = 0; //alive count

public:
    // Return the number of active components
    size_t size() const {
        return count_;
    }

    bool valid(size_t id) const{
        return id < alive_.size() && alive_[id];
    }

    template <typename U> size_t store(U&& value){
        size_t id;
        if (! free_.empty()){
            id = free_.back();
            free_.pop_back();
            data_[id].value = std::forward<U>(value);
            alive_[id] = true;
        }
        else {
            id = data_.size();
            data_.push_back(Slot{std::forward<U>(value)});
            alive_.push_back(true);
        }
        ++ count_;
        return id;
    }

    void remove(size_t id){
        assert(valid(id));
        data_[id].value = T{};
        alive_[id] = false;
        free_.push_back(id);
        --count_;
    }

    T& operator[](size_t id) {
        assert(valid(id));
        return data_[id].value;
    }

    const T& operator[](size_t id) const {
        assert(valid(id));
        return data_[id].value;
    }

    void reserve(size_t capacity){
        data_.reserve(capacity);
        alive_.reserve(capacity);
    }
};

#endif  // REGISTRY_H
