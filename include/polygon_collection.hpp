#ifndef POLYGON_COLLECTION_HPP
#define POLYGON_COLLECTION_HPP

#include "polygons.hpp"

#include <concepts>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <utility>

namespace polygons {
    class PolygonCollection {
    public:
        using value_type = std::unique_ptr<Polygon>;
        using storage_type = small_vec::small_vec<value_type, STACK_POINTS>;
        using iterator = storage_type::iterator;
        using const_iterator = storage_type::const_iterator;

    private:
        storage_type polygons_;

    public:
        PolygonCollection() = default;
        PolygonCollection(const PolygonCollection&) = delete;
        PolygonCollection& operator=(const PolygonCollection&) = delete;
        PolygonCollection(PolygonCollection&&) noexcept = default;
        PolygonCollection& operator=(PolygonCollection&&) noexcept = default;

        void add(value_type polygon) {
            if (!polygon) {
                throw std::invalid_argument("PolygonCollection cannot store a null polygon");
            }
            polygons_.push_back(std::move(polygon));
        }

        template<typename T, typename... Args>
            requires std::derived_from<T, Polygon>
        T& emplace(Args&&... args) {
            auto polygon = std::make_unique<T>(std::forward<Args>(args)...);
            T& result = *polygon;
            add(std::move(polygon));
            return result;
        }

        [[nodiscard]] std::size_t size() const noexcept {
            return polygons_.size();
        }

        [[nodiscard]] bool empty() const noexcept {
            return polygons_.empty();
        }

        [[nodiscard]] double totalMetric() const noexcept {
            double total = 0.0;
            for (const auto& polygon : polygons_) {
                total += polygon->metric();
            }
            return total;
        }

        [[nodiscard]] Polygon& at(std::size_t index) {
            return *polygons_.at(index);
        }

        [[nodiscard]] const Polygon& at(std::size_t index) const {
            return *polygons_.at(index);
        }

        bool erase(std::size_t index) noexcept {
            if (index >= polygons_.size()) {
                return false;
            }
            for (std::size_t i = index + 1; i < polygons_.size(); ++i) {
                polygons_[i - 1] = std::move(polygons_[i]);
            }
            polygons_.resize(polygons_.size() - 1);
            return true;
        }

        void clear() noexcept {
            polygons_.clear();
        }

        iterator begin() noexcept {
            return polygons_.begin();
        }

        const_iterator begin() const noexcept {
            return polygons_.begin();
        }

        iterator end() noexcept {
            return polygons_.end();
        }

        const_iterator end() const noexcept {
            return polygons_.end();
        }
    };
}

#endif
