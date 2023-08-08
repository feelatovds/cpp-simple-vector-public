#pragma once

#include <cstdlib>
#include <initializer_list>
#include <stdexcept>
#include <algorithm>
#include <cassert>
#include <utility>

#include "array_ptr.h"


class ReserveProxyObj {
public:
    ReserveProxyObj(size_t ctr)
            : reserve_capacity_(ctr)
    {
    }

    size_t GetCapacity() const {
        return reserve_capacity_;
    }

private:
    size_t reserve_capacity_ = 0;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    explicit SimpleVector(size_t size)
            : items_(size)
            , size_(size)
            , capacity_(size)
    {
        std::fill(begin(), end(), std::move(Type()));
    }

    SimpleVector(size_t size, const Type& value)
            : items_(size)
            , size_(size)
            , capacity_(size)
    {
        std::fill(begin(), end(), value);
    }

    SimpleVector(std::initializer_list<Type> init)
            : items_(init.size())
            , size_(init.size())
            , capacity_(init.size())
    {
        int i = 0;
        for (auto& val : init) {
            items_[i++] = std::move(val);
        }
    }

    SimpleVector(const SimpleVector& other)
            : SimpleVector(other.size_)
    {
        ArrayPtr<Type> tmp(other.size_);
        std::copy(other.begin(), other.end(), &tmp[0]);
        tmp.swap(items_);
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        assert(&rhs.items_[0] != &items_[0]); // запрещаем самоприсваивание

        ArrayPtr<Type> tmp(rhs.size_);
        std::copy(rhs.begin(), rhs.end(), &tmp[0]);
        tmp.swap(items_);
        size_ = rhs.size_;
        capacity_ = rhs.capacity_;
        return *this;
    }

    SimpleVector(SimpleVector&& other) {
        items_ = std::move(other.items_);
        size_ = std::exchange(other.size_, 0);
        capacity_ = std::exchange(other.capacity_, 0);
    }

    SimpleVector& operator=(SimpleVector&& rhs) {
        items_ = std::move(rhs.items_);
        size_ = std::exchange(rhs.size_, 0);
        capacity_ = std::exchange(rhs.capacity_, 0);
        return *this;
    }

    SimpleVector(const ReserveProxyObj& obj)
            : SimpleVector()
    {
        Reserve(obj.GetCapacity());
    }

    size_t GetSize() const noexcept {
        return size_;
    }

    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    Type& operator[](size_t index) noexcept {
        assert(index < size_);
        return items_[index];
    }

    const Type& operator[](size_t index) const noexcept {
        assert(index < size_);
        return items_[index];
    }

    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Method At(index): index >= size");
        }
        return items_[index];
    }

    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Method At(index): index >= size");
        }
        return items_[index];
    }

    void Clear() noexcept {
        size_ = 0;
    }

    void Resize(size_t new_size) {
        if (new_size <= size_) {
            size_ = new_size;
            return;
        }

        if (size_ < new_size && new_size <= capacity_) {
            size_ = new_size;
            return;
        }

        if (new_size > capacity_) {
            size_t new_capacity = std::max(new_size, 2 * capacity_);
            ArrayPtr<Type> tmp(new_capacity);
            std::copy(std::make_move_iterator(begin()), std::make_move_iterator(end()), &tmp[0]);
            tmp.swap(items_);
            size_ = new_size;
            capacity_ = new_capacity;
            return;
        }
    }

    Iterator begin() noexcept {
        return size_ == 0 ? nullptr : &items_[0];
    }

    Iterator end() noexcept {
        return size_ == 0 ? nullptr : &items_[0] + size_;
    }

    ConstIterator begin() const noexcept {
        return size_ == 0 ? nullptr : &items_[0];
    }

    ConstIterator end() const noexcept {
        return size_ == 0 ? nullptr : &items_[0] + size_;
    }

    ConstIterator cbegin() const noexcept {
        return begin();
    }

    ConstIterator cend() const noexcept {
        return end();
    }

    void PushBack(Type item) {
        if (size_ != capacity_) {
            items_[size_++] = std::move(item);
            return;
        }

        const size_t new_capacity = std::max(2 * capacity_, size_t(1));
        ArrayPtr<Type> tmp(new_capacity);
        std::copy(std::make_move_iterator(begin()), std::make_move_iterator(end()), &tmp[0]);
        tmp[size_] = std::move(item);

        tmp.swap(items_);
        ++size_;
        capacity_ = new_capacity;
    }

    Iterator Insert(ConstIterator pos, Type value) {
        assert(pos >= begin() && pos <= end());
        if (size_ != capacity_) {
            auto p = pos - begin();
            std::copy_backward(std::make_move_iterator(begin() + p), std::make_move_iterator(end()), end() + 1);
            items_[p] = std::move(value);
            ++size_;
            return begin() + p;
        }

        auto p = pos - begin();
        size_t new_capacity = std::max(2 * capacity_, size_t(1));
        ArrayPtr<Type> tmp(new_capacity);
        std::copy(std::make_move_iterator(begin()), std::make_move_iterator(begin() + p), &tmp[0]);
        tmp[p] = std::move(value);

        std::copy_backward(std::make_move_iterator(begin() + p), std::make_move_iterator(end()), &tmp[size_] + 1);
        tmp.swap(items_);
        capacity_ = new_capacity;
        ++size_;
        return begin() + p;
    }

    void PopBack() noexcept {
        assert(size_ != 0);
        --size_;
    }

    Iterator Erase(ConstIterator pos) {
        assert(pos >= begin() && pos < end());
        auto p = pos - begin();
        std::copy(std::make_move_iterator(begin() + p + 1), std::make_move_iterator(end()), begin() + p);
        --size_;
        return begin() + p;
    }

    void swap(SimpleVector& other) noexcept {
        items_.swap(other.items_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity <= capacity_) {
            return;
        }

        ArrayPtr<Type> tmp(new_capacity);
        std::copy(begin(), end(), &tmp[0]);
        tmp.swap(items_);
        capacity_ = new_capacity;
    }

private:
    ArrayPtr<Type> items_;

    size_t size_ = 0;
    size_t capacity_ = 0;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return lhs.GetSize() == rhs.GetSize() && std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(rhs < lhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}

