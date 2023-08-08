#pragma once

#include <cstdlib>
#include <utility>

template <typename Type>
class ArrayPtr {
public:
    ArrayPtr() = default;

    explicit ArrayPtr(size_t size) {
        raw_ptr_ = size == 0 ? nullptr : new Type[size]();
    }

    explicit ArrayPtr(Type* raw_ptr) noexcept
            : raw_ptr_(raw_ptr)
    {
    }

    ArrayPtr(const ArrayPtr&) = delete;
    ArrayPtr& operator=(const ArrayPtr&) = delete;

    ArrayPtr(ArrayPtr&& other) {
        raw_ptr_ = std::exchange(other.raw_ptr_, nullptr);
    }

    ArrayPtr& operator=(ArrayPtr&& rhs) {
        raw_ptr_ = std::exchange(rhs.raw_ptr_, nullptr);
        return *this;
    }

    ~ArrayPtr() {
        delete[] raw_ptr_;
    }

    // Прекращает владением массивом в памяти, возвращает значение адреса массива
    // После вызова метода указатель на массив должен обнулиться
    [[nodiscard]] Type* Release() noexcept {
        return std::exchange(raw_ptr_, nullptr);
    }

    Type& operator[](size_t index) noexcept {
        return *(raw_ptr_ + index);
    }

    const Type& operator[](size_t index) const noexcept {
        return *(raw_ptr_ + index);
    }

    explicit operator bool() const {
        return raw_ptr_ != nullptr;
    }

    Type* Get() const noexcept {
        return raw_ptr_;
    }

    // Обменивается значениями указателя на массив с объектом other
    void swap(ArrayPtr& other) noexcept {
        std::swap(other.raw_ptr_, raw_ptr_);
    }

private:
    Type* raw_ptr_ = nullptr;
};

