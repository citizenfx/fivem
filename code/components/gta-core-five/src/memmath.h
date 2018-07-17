#pragma once

#include <type_traits>
#include <cstdint>
#include <cstddef>

namespace mem {
    class pointer {
    protected:
        uintptr_t value_;

    public:
        pointer()                                                       noexcept;
        pointer(const std::nullptr_t null)                              noexcept;
        pointer(const uintptr_t value)                                  noexcept;
        pointer(const void* const value)                                noexcept;

        bool null()                                               const noexcept;

        pointer add(const pointer value)                          const noexcept;
        pointer sub(const pointer value)                          const noexcept;

        size_t dist(const pointer value)                          const noexcept;

        pointer shift(const pointer from, const pointer to)       const noexcept;

        pointer rip(const ptrdiff_t offset)                       const;

        pointer& deref()                                          const noexcept;

        pointer operator+(const pointer value)                    const noexcept;
        pointer operator-(const pointer value)                    const noexcept;

        pointer& operator+=(const pointer value)                        noexcept;
        pointer& operator-=(const pointer value)                        noexcept;

        pointer& operator++()                                           noexcept;
        pointer& operator--()                                           noexcept;

        pointer operator++(int)                                         noexcept;
        pointer operator--(int)                                         noexcept;

        bool operator==(const pointer value)                      const noexcept;
        bool operator!=(const pointer value)                      const noexcept;

        bool operator<(const pointer value)                       const noexcept;
        bool operator>(const pointer value)                       const noexcept;

        bool operator<=(const pointer value)                      const noexcept;
        bool operator>=(const pointer value)                      const noexcept;

        template <typename T>
        std::enable_if_t<std::is_same<T, pointer>::value, T> as() const noexcept;

        template <typename T>
        std::enable_if_t<std::is_integral<T>::value, T>      as() const noexcept;

        template <typename T>
        std::enable_if_t<std::is_pointer<T>::value, T>       as() const noexcept;

        template <typename T>
        std::enable_if_t<
            std::is_lvalue_reference<T>::value, T>           as() const noexcept;

        template <typename T>
        std::enable_if_t<std::is_array<T>::value,
            std::add_lvalue_reference_t<T>>                  as() const noexcept;
    };

    inline pointer::pointer() noexcept
        : value_()
    { }

    inline pointer::pointer(std::nullptr_t) noexcept
        : value_()
    { }

    inline pointer::pointer(const uintptr_t p) noexcept
        : value_(p)
    { }

    inline pointer::pointer(const void* const p) noexcept
        : value_(reinterpret_cast<uintptr_t>(p))
    { }

    inline bool pointer::null() const noexcept {
        return !value_;
    }

    inline pointer pointer::add(const pointer value) const noexcept {
        return value_ + value.value_;
    }

    inline pointer pointer::sub(const pointer value) const noexcept {
        return value_ - value.value_;
    }

    inline size_t pointer::dist(const pointer value) const noexcept {
        return value.sub(*this).as<size_t>();
    }

    inline pointer pointer::shift(const pointer from, const pointer to) const noexcept {
        return sub(from).add(to);
    }

    inline pointer pointer::rip(const ptrdiff_t offset) const
    {
        return add(offset).add(as<int32_t&>());
    }

    inline pointer& pointer::deref() const noexcept {
        return as<pointer&>();
    }

    inline pointer pointer::operator+(const pointer value) const noexcept {
        return add(value);
    }

    inline pointer pointer::operator-(const pointer value) const noexcept {
        return sub(value);
    }

    inline pointer& pointer::operator+=(const pointer value) noexcept {
        return (*this) = (*this) + value;
    }

    inline pointer& pointer::operator-=(const pointer value) noexcept {
        return (*this) = (*this) - value;
    }

    inline pointer& pointer::operator++() noexcept {
        return (*this) = (*this) + 1;
    }

    inline pointer& pointer::operator--() noexcept {
        return (*this) = (*this) - 1;
    }

    inline pointer pointer::operator++(int) noexcept {
        pointer result = (*this);

        ++(*this);

        return result;
    }

    inline pointer pointer::operator--(int) noexcept {
        pointer result = (*this);

        ++(*this);

        return result;
    }

    inline bool pointer::operator==(const pointer value) const noexcept {
        return value_ == value.value_;
    }

    inline bool pointer::operator!=(const pointer value) const noexcept {
        return value_ != value.value_;
    }

    inline bool pointer::operator<(const pointer value) const noexcept {
        return value_ < value.value_;
    }

    inline bool pointer::operator>(const pointer value) const noexcept {
        return value_ > value.value_;
    }

    inline bool pointer::operator<=(const pointer value) const noexcept {
        return value_ <= value.value_;
    }

    inline bool pointer::operator>=(const pointer value) const noexcept {
        return value_ >= value.value_;
    }

    template <typename T>
    inline std::enable_if_t<std::is_same<T, pointer>::value, T> pointer::as() const noexcept {
        return (*this);
    }

    template <typename T>
    inline std::enable_if_t<std::is_integral<T>::value, T> pointer::as() const noexcept {
        static_assert(std::is_same<std::make_unsigned_t<T>, uintptr_t>::value, "Invalid Integer Type");

        return static_cast<T>(value_);
    }

    template <typename T>
    inline std::enable_if_t<std::is_pointer<T>::value, T> pointer::as() const noexcept {
        return reinterpret_cast<T>(value_);
    }

    template <typename T>
    inline std::enable_if_t<std::is_lvalue_reference<T>::value, T> pointer::as() const noexcept {
        return *as<std::add_pointer_t<T>>();
    }

    template <typename T>
    inline std::enable_if_t<std::is_array<T>::value, std::add_lvalue_reference_t<T>> pointer::as() const noexcept {
        return as<std::add_lvalue_reference_t<T>>();
    }
}
