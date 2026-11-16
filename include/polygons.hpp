#ifndef POLYGONS_HPP
#define POLYGONS_HPP
#include "small_vec.hpp"
#include <cmath>
#include <optional>
#include <iostream>
#include <span>
#include <concepts>

namespace polygons {
    inline constexpr size_t STACK_POINTS = 5;
    inline constexpr double EPS = 1e-9;

    enum class PolygonError {
        SelfIntersection,
        TooFewPoints,
        ValidationError,
        None
    };

    struct Point {
        double x;
        double y;

        Point() = default;
        Point(double _x, double _y) : x(_x), y(_y) {}
        Point operator + (const Point &other) const {
            return {x + other.x, y + other.y};
        }
        Point operator - (const Point &other) const {
            return {x - other.x, y - other.y};
        }

        Point& operator+=(const Point & point) {
            x += point.x;
            y += point.y;
            return *this;
        }

        bool operator==(const Point& other) const {
            return std::fabs(other.x - x) < EPS && std::fabs(other.y - y) < EPS;
        }

        friend std::ostream& operator<<(std::ostream &os, const Point &point) {
            os << "(" << point.x << ", " << point.y << ")";
            return os;
        }
    };

    class Polygon {
    public:
        using points_vec = small_vec::small_vec<Point, STACK_POINTS>;

    private:
         points_vec points_{};
    protected:
        Polygon(const Polygon&) = default;
        Polygon& operator=(const Polygon&) = default;
        Polygon(Polygon&&) noexcept = default;
        Polygon& operator=(Polygon&&) noexcept = default;
        explicit Polygon(points_vec&& points) noexcept;

        virtual void writeAttributes(std::ostream& stream) const = 0;
        virtual std::istream& read(std::istream& stream) = 0;
        bool readPoints(std::istream& stream, std::size_t count);
        [[nodiscard]] const points_vec& points() const noexcept;
        [[nodiscard]] static bool hasSelfIntersections(std::span<const Point>) noexcept;
    public:
        virtual ~Polygon() = default;

        template<typename T>
        [[nodiscard]] static std::optional<T> fromVerticesOwnedExplicit(points_vec&& vertices, PolygonError& error)
        noexcept(std::is_nothrow_constructible_v<T, points_vec&&> && std::is_nothrow_move_constructible_v<T>)
        requires std::derived_from<T, Polygon>
        {
            if (vertices.size() < 3) {
                error = PolygonError::TooFewPoints;
                return std::nullopt;
            }
            if (hasSelfIntersections(vertices)) {
                error = PolygonError::SelfIntersection;
                return std::nullopt;
            }

            T poly(std::move(vertices));
            if (!poly.isValid()) {
                error = PolygonError::ValidationError;
                return std::nullopt;
            }

            error = PolygonError::None;
            return poly;
        }

        template<typename T>
        [[nodiscard]] static std::optional<T> fromVerticesOwned(points_vec&& vertices)
        noexcept(std::is_nothrow_constructible_v<T, points_vec&&> && std::is_nothrow_move_constructible_v<T>)
        {
            PolygonError ignored{};
            return fromVerticesOwnedExplicit<T>(std::move(vertices), ignored);
        }

        template<typename T>
        [[nodiscard]] static std::optional<T> fromVerticesExplicit(std::span<const Point> vertices, PolygonError& error) {
            points_vec vec;
            vec.reserve(vertices.size());
            for (const Point& v : vertices) {
                vec.push_back(v);
            }
            return fromVerticesOwnedExplicit<T>(std::move(vec), error);
        }

        template<typename T>
        [[nodiscard]] static std::optional<T> fromVertices(std::span<const Point> vertices) {
            PolygonError ignored{};
            return fromVerticesExplicit<T>(vertices, ignored);
        }

        [[nodiscard]] virtual double metric() const noexcept;

        [[nodiscard]] std::optional<Point> center() const noexcept;

        [[nodiscard]] virtual bool isValid() const noexcept = 0;

        explicit operator double() const noexcept;

        bool operator==(const Polygon &other) const noexcept;

        friend std::ostream& operator<<(std::ostream& stream, const Polygon &polygon);
        friend std::istream& operator>>(std::istream& stream, Polygon &polygon);
    };

    class Triangle final : public Polygon {
        friend class Polygon;
    protected:
        void writeAttributes(std::ostream& stream) const override;
        std::istream& read(std::istream& stream) override;
    private:
        explicit Triangle(points_vec&& points) noexcept;
    public:
        [[nodiscard]] bool isValid() const noexcept override;
        [[nodiscard]] bool isEquilateral() const noexcept;
    };

    class Square final : public Polygon {
        friend class Polygon;
    protected:
        void writeAttributes(std::ostream& stream) const override;
        std::istream& read(std::istream& stream) override;
    private:
        explicit Square(points_vec&& points) noexcept;
    public:
        [[nodiscard]] bool isValid() const noexcept override;
        [[nodiscard]] double side() const noexcept;
    };

    class Pentagon final : public Polygon {
        friend class Polygon;
    protected:
        void writeAttributes(std::ostream& stream) const override;
        std::istream& read(std::istream& stream) override;
    private:
        explicit Pentagon(points_vec&& points) noexcept;
    public:
        [[nodiscard]] bool isValid() const noexcept override;
        [[nodiscard]] bool isRegular() const noexcept;
    };
}


#endif //POLYGONS_HPP
