#ifndef SMALL_VEC_HPP
#define SMALL_VEC_HPP

#include <cassert>
#include <type_traits>
#include <utility>
#include <memory>
#include <cstddef>
#include <new>
#include <stdexcept>
#include <iterator>
#include <initializer_list>

namespace small_vec {
    template <typename T, std::size_t N>
    class small_vec {
        static_assert(N != 0, "small_vec::small_vec: N must be positive");
    private:
        alignas(T) std::byte stack_data[N * sizeof(T)];
        T* data_ = stack_ptr();
        std::size_t capacity_ = N;
        std::size_t size_ = 0;

        [[nodiscard]] bool is_small() const noexcept {
            return data_== stack_ptr();
        }

        T* stack_ptr() noexcept {
            return reinterpret_cast<T*>(stack_data);
        }

        const T* stack_ptr() const noexcept {
            return reinterpret_cast<const T*>(stack_data);
        }

        void grow(std::size_t new_capacity_) {
            T* new_data = static_cast<T*>(::operator new(new_capacity_ * sizeof(T), std::align_val_t{alignof(T)}));
            std::size_t constructed = 0;
            try {
                for (; constructed < size_; ++constructed) {
                    std::construct_at(new_data + constructed, std::move_if_noexcept(data_[constructed]));
                }
            } catch (...) {
                std::destroy_n(new_data, constructed);
                ::operator delete(new_data, new_capacity_ * sizeof(T), std::align_val_t{alignof(T)});
                throw;
            }

            std::destroy_n(data_, size_);
            if (!is_small()) {
                ::operator delete(data_, capacity_ * sizeof(T), std::align_val_t{alignof(T)});
            }

            data_= new_data;
            capacity_ = new_capacity_;
        }

    public:
        using iterator = T*;
        using const_iterator = const T*;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;
        using value_type = T;
        using pointer = T*;
        using const_pointer = const T*;
        using reference = T&;
        using const_reference = const T&;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;

        small_vec() = default;

        small_vec(std::initializer_list<T> init) {
            if (init.size() > N) {
                grow(init.size());
            }
            if (init.size() > 0) {
                try {
                    std::uninitialized_copy(init.begin(), init.end(), data_);
                } catch (...) {
                    if (!is_small()) {
                        ::operator delete(data_, capacity_ * sizeof(T), std::align_val_t{alignof(T)});
                    }
                    throw;
                }

                size_ = init.size();
            }
        }


        small_vec(const small_vec& other) {
            T* new_data = data_;
            std::size_t new_cap = N;
            if (other.size_> N) {
                new_cap = other.size_;
                new_data= static_cast<T*>(::operator new(new_cap * sizeof(T), std::align_val_t{alignof(T)}));
            }
            try {
                std::uninitialized_copy_n(other.data_, other.size_, new_data);
            } catch (...) {
                if (other.size_> N) {
                    ::operator delete(new_data, new_cap * sizeof(T), std::align_val_t{alignof(T)});
                }
                throw;
            }

            data_= new_data;
            size_= other.size_;
            capacity_ = new_cap;
        }

        small_vec& operator=(const small_vec& other) {
            if (this != &other) {
                small_vec temp(other);
                swap(temp);
            }
            return *this;
        }

        small_vec(small_vec&& other) noexcept(std::is_nothrow_move_constructible_v<T> || std::is_nothrow_copy_constructible_v<T>)
        : capacity_(other.capacity_)
        {
            if (!other.is_small()) {
                data_= other.data_;
                size_ = other.size_;
            } else {
                if constexpr (std::is_nothrow_move_constructible_v<T> || std::is_nothrow_copy_constructible_v<T>) {
                    for (; size_ < other.size_; ++size_) {
                        std::construct_at(data_ + size_, std::move_if_noexcept(other.data_[size_]));
                    }
                    std::destroy_n(other.data_, other.size_);
                } else {
                    std::size_t constructed = 0;
                    try {
                        for (; constructed < other.size_; ++constructed) {
                            std::construct_at(data_ + constructed, std::move_if_noexcept(other.data_[constructed]));
                        }
                    } catch (...) {
                        std::destroy_n(data_, constructed);
                        throw;
                    }
                    size_ = other.size_;
                    std::destroy_n(other.data_, size_);
                }
            }

            other.data_= other.stack_ptr();
            other.size_= 0;
            other.capacity_ = N;
        }

        small_vec& operator=(small_vec&& other) noexcept(noexcept(swap(other))) {
            if (this != &other) {
                swap(other);
            }
            return *this;
        }

        ~small_vec() {
            std::destroy_n(data_, size_);
            if (!is_small()) {
                ::operator delete(data_, capacity_ * sizeof(T), std::align_val_t{alignof(T)});
            }
        }

        void swap(small_vec& b) noexcept(
            (std::is_nothrow_move_constructible_v<T> || std::is_nothrow_copy_constructible_v<T>)
            && std::is_nothrow_swappable_v<T>)
        {
            if (this == &b) return;
            using std::swap;
            auto& a = *this;
            bool a_small = a.is_small();
            bool b_small = b.is_small();

            if (a_small && b_small) {
                small_vec& less = a.size_> b.size_? b : a;
                small_vec& greater = a.size_<= b.size_? b : a;

                std::size_t max_size_= greater.size_;
                std::size_t min_size= less.size_;

                auto diff = max_size_- min_size;
                for (std::size_t i = 0; i < less.size_; ++i) {
                    swap(less.data_[i], greater.data_[i]);
                }

                std::size_t constructed = 0;
                if constexpr (std::is_nothrow_move_constructible_v<T> || std::is_nothrow_copy_constructible_v<T>) {
                    for (; constructed < diff; ++constructed) {
                        std::construct_at(less.data_ + min_size + constructed,
                                          std::move_if_noexcept(greater.data_[constructed + min_size]));
                    }
                } else {
                    try {
                        for (; constructed < diff; ++constructed) {
                            std::construct_at(less.data_ + min_size + constructed,
                                              std::move_if_noexcept(greater.data_[constructed + min_size]));
                        }
                    } catch (...) {
                        std::destroy_n(less.data_ + min_size, constructed);
                        throw;
                    }
                }

                std::destroy_n(greater.data_+ min_size, diff);
                swap(a.size_, b.size_);
            } else if (!a_small && !b_small) {
                swap(a.data_, b.data_);
                swap(a.size_, b.size_);
                swap(a.capacity_, b.capacity_);
            } else {
                small_vec& small = a_small ? a : b;
                small_vec& heap  = a_small ? b : a;

                T* heap_ptr = heap.data_;
                std::size_t heap_cap = heap.capacity_;

                std::size_t constructed = 0;
                if constexpr (std::is_nothrow_move_constructible_v<T> || std::is_nothrow_copy_constructible_v<T>) {
                    for (; constructed < small.size_; ++constructed) {
                        std::construct_at(heap.stack_ptr() + constructed,
                                          std::move_if_noexcept(small.data_[constructed]));
                    }
                } else {
                    try {
                        for (; constructed < small.size_; ++constructed) {
                            std::construct_at(heap.stack_ptr() + constructed,
                                              std::move_if_noexcept(small.data_[constructed]));
                        }
                    } catch (...) {
                        std::destroy_n(heap.stack_ptr(), constructed);
                        throw;
                    }
                }
                std::destroy_n(small.data_, small.size_);

                heap.data_= heap.stack_ptr();
                small.data_= heap_ptr;

                swap(small.size_, heap.size_);
                small.capacity_ = heap_cap;
                heap.capacity_ = N;
            }
        }

        friend void swap(small_vec& a, small_vec& b) noexcept(noexcept(a.swap(b))) {
            a.swap(b);
        }

        template<typename... Args>
        T& emplace_back(Args&&... args) {
            if (size_ == capacity_) {
                std::size_t new_cap = capacity_ * 2;
                T* new_data= static_cast<T*>(::operator new(new_cap * sizeof(T), std::align_val_t{alignof(T)}));

                try {
                    std::construct_at(new_data+ size_, std::forward<Args>(args)...);
                } catch (...) {
                    ::operator delete(new_data, new_cap * sizeof(T), std::align_val_t{alignof(T)});
                    throw;
                }

                std::size_t constructed = 0;
                try {
                    for (; constructed < size_; ++constructed) {
                        std::construct_at(new_data+ constructed, std::move_if_noexcept(data_[constructed]));
                    }
                } catch (...) {
                    std::destroy_n(new_data, constructed);
                    std::destroy_at(new_data+ size_);
                    ::operator delete(new_data, new_cap * sizeof(T), std::align_val_t{alignof(T)});
                    throw;
                }

                std::destroy_n(data_, size_);
                if (!is_small()) {
                    ::operator delete(data_, capacity_ * sizeof(T), std::align_val_t{alignof(T)});
                }

                data_ = new_data;
                capacity_ = new_cap;
                ++size_;
                return data_[size_ - 1];
            }

            T* ptr = std::construct_at(data_ + size_, std::forward<Args>(args)...);
            ++size_;
            return *ptr;
        }


        void push_back(const T& value) {
            emplace_back(value);
        }

        void push_back(T&& value) {
            emplace_back(std::move(value));
        }

        void reserve(std::size_t new_cap) {
            if (new_cap > capacity_) {
                grow(new_cap);
            }
        }

        void resize(std::size_t new_size) {
            if (new_size < size_) {
                std::destroy_n(data_+ new_size, size_ - new_size);
            } else if (new_size > size_) {
                if (new_size > capacity_) {
                    size_t new_cap = (new_size > capacity_ * 2) ? new_size : capacity_ * 2;
                    grow(new_cap);
                }
                std::size_t constructed = 0;
                try {
                    for (std::size_t i = size_; i < new_size; ++i, ++constructed) {
                        std::construct_at(data_ + i);
                    }
                } catch (...) {
                    std::destroy_n(data_ + size_, constructed);
                    throw;
                }

            }
            size_ = new_size;
        }

        void pop_back() noexcept {
            assert(size_ > 0);
            std::destroy_at(data_ + --size_);
        }

        const T& back() const noexcept {
            assert(size_ > 0);
            return data_[size_ - 1];
        }

        T& back() noexcept {
            assert(size_ > 0);
            return data_[size_ - 1];
        }

        const T& front() const noexcept {
            assert(size_ > 0);
            return data_[0];
        }

        T& front() noexcept {
            assert(size_ > 0);
            return data_[0];
        }

        void clear() noexcept {
            std::destroy_n(data_, size_);
            size_ = 0;
        }

        const T& operator[](std::size_t index) const noexcept {
            assert(index < size_);
            return data_[index];
        }

        T& operator[](std::size_t index) noexcept {
            assert(index < size_);
            return data_[index];
        }

        T& at(std::size_t index) {
            if (size_ <= index) {
                throw std::out_of_range("small_vec::at: index out of range");
            }
            return data_[index];
        }

        const T& at(std::size_t index) const {
            if (size_ <= index) {
                throw std::out_of_range("small_vec::at: index out of range");
            }
            return data_[index];
        }



        T* data() noexcept {
            return data_;
        }

        const T* data() const noexcept {
            return data_;
        }

        iterator begin() noexcept {
            return data_;
        }
        const_iterator begin() const noexcept {
            return data_;
        }
        const_iterator cbegin() const noexcept {
            return data_;
        }

        reverse_iterator rbegin() noexcept {
            return std::reverse_iterator(end());
        }

        const_reverse_iterator rbegin() const noexcept {
            return std::reverse_iterator(end());
        }

        const_reverse_iterator crbegin() const noexcept {
            return std::reverse_iterator(cend());
        }


        iterator end() noexcept {
            return data_ + size_;
        }
        const_iterator end() const noexcept {
            return data_ + size_;
        }
        const_iterator cend() const noexcept {
            return data_ + size_;
        }

        reverse_iterator rend() noexcept {
            return reverse_iterator(begin());
        }
        const_reverse_iterator rend() const noexcept {
            return const_reverse_iterator(begin());
        }
        const_reverse_iterator crend() const noexcept {
            return const_reverse_iterator(cbegin());
        }

        [[nodiscard]] std::size_t size() const noexcept {
            return size_;
        }

        [[nodiscard]] bool empty() const noexcept {
            return size_ == 0;
        }

        [[nodiscard]] std::size_t capacity() const noexcept {
            return capacity_;
        }

        bool operator==(const small_vec& rhs) const {
            if (size_ != rhs.size_) return false;

            for (std::size_t i = 0; i < size_; ++i) {
                if (data_[i] != rhs.data_[i]) return false;
            }
            return true;
        }
    };
}

#endif //SMALL_VEC_HPP
