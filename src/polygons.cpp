#include "polygons.hpp"
#include <algorithm>
#include <numbers>
#include <utility>
#include <cmath>
#include <iostream>


namespace polygons {
    namespace {
        double distance(const Point& a, const Point& b) noexcept {
            return std::hypot(a.x - b.x, a.y - b.y);
        }

        bool nearlyEqual(double a, double b) noexcept {
            return std::fabs(a - b) <= EPS * std::max({1.0, std::fabs(a), std::fabs(b)});
        }

        bool validVertices(std::span<const Point> points, std::size_t expected) noexcept {
            if (points.size() != expected) {
                return false;
            }

            double twiceArea = 0.0;
            for (std::size_t i = 0; i < points.size(); ++i) {
                const Point& current = points[i];
                const Point& next = points[(i + 1) % points.size()];
                if (!std::isfinite(current.x) || !std::isfinite(current.y) || distance(current, next) <= EPS) {
                    return false;
                }
                twiceArea += current.x * next.y - next.x * current.y;
            }
            return std::isfinite(twiceArea) && std::fabs(twiceArea) > EPS;
        }

        int orientation(Point a, Point b, Point c, double eps) noexcept {
            double val = (b.y - a.y) * (c.x - b.x) - (b.x - a.x) * (c.y - b.y);
            if (std::fabs(val) < eps) {
                return 0;
            }
            return (val > 0.0) ? 1 : -1;
        }

        bool onSegment(Point p, Point q, Point r, double eps) noexcept {
            return q.x <= std::max(p.x, r.x) + eps && q.x >= std::min(p.x, r.x) - eps &&
                   q.y <= std::max(p.y, r.y) + eps && q.y >= std::min(p.y, r.y) - eps;
        }

        bool segmentsIntersect(Point a, Point b, Point c, Point d) noexcept {
            int o1 = orientation(a, b, c, EPS);
            int o2 = orientation(a, b, d, EPS);
            int o3 = orientation(c, d, a, EPS);
            int o4 = orientation(c, d, b, EPS);

            if (o1 != o2 && o3 != o4) {
                return true;
            }

            if (o1 == 0 && onSegment(a, c, b, EPS)) return true;
            if (o2 == 0 && onSegment(a, d, b, EPS)) return true;
            if (o3 == 0 && onSegment(c, a, d, EPS)) return true;
            if (o4 == 0 && onSegment(c, b, d, EPS)) return true;

            return false;
        }
    } //ns

    const Polygon::points_vec &Polygon::points() const noexcept {
        return points_;
    }

    bool Polygon::hasSelfIntersections(std::span<const Point> pts) noexcept {
        const std::size_t n = pts.size();
        if (n < 3) {
            return false;
        }

        for (std::size_t i = 0; i < n; ++i) {
            Point a = pts[i];
            Point b = pts[(i + 1) % n];

            for (std::size_t j = i + 1; j < n; ++j) {
                bool isAdjacent = (j == i + 1) || (i == 0 && j == n - 1);
                if (isAdjacent) {
                    Point prev = (i == 0 && j == n - 1) ? pts[n - 1] : pts[i];
                    Point shared = (i == 0 && j == n - 1) ? pts[0] : pts[j];
                    Point next = (i == 0 && j == n - 1) ? pts[1] : pts[(j + 1) % n];
                    if (orientation(prev, shared, next, EPS) == 0 &&
                        (onSegment(prev, next, shared, EPS) || onSegment(shared, prev, next, EPS))) {
                        return true;
                        }
                    continue;
                }

                Point c = pts[j];
                Point d = pts[(j + 1) % n];

                if (segmentsIntersect(a, b, c, d)) {
                    return true;
                }
            }
        }

        return false;
    }


    Polygon::Polygon(points_vec&& points) noexcept : points_(std::move(points)) {}

    double Polygon::metric() const noexcept {
        const size_t n = points_.size();
        if (n < 3) {
            return 0.0;
        }

        double s = 0.0;
        for (size_t i = 0, j = n - 1; i < n; j = i++) {
            s += points_[j].x * points_[i].y - points_[i].x * points_[j].y;
        }

        return std::abs(s) * 0.5;
    }

    std::optional<Point> Polygon::center() const noexcept {
        if (points_.empty()) return std::nullopt;
        Point center = {0, 0};
        for (const auto& p: points_) {
            center += p;
        }
        auto n = static_cast<double>(points_.size());
        center.x /= n;
        center.y /= n;
        return center;
    }

    Polygon::operator double() const noexcept {
        return metric();
    }

    bool Polygon::operator==(const Polygon &other) const noexcept {
        return std::fabs(metric() - other.metric()) < EPS;
    }

    std::ostream &operator<<(std::ostream &stream, const Polygon &polygon) {
        polygon.writeAttributes(stream);
        stream << '\n';
        return stream;
    }

    bool Polygon::readPoints(std::istream& stream, std::size_t count) {
        using std::swap;
        points_vec new_points;
        new_points.reserve(count);

        for (std::size_t i = 0; i < count; ++i) {
            double x, y;
            if (!(stream >> x >> y)) {
                return false;
            }
            new_points.emplace_back(x, y);
        }
        swap(points_, new_points);
        if (!isValid()) {
            swap(points_, new_points);
            stream.setstate(std::ios::failbit);
            return false;
        }
        return true;
    }

    std::istream& operator>>(std::istream& stream, Polygon& polygon) {
        return polygon.read(stream);
    }

    Triangle::Triangle(points_vec&& points) noexcept : Polygon(std::move(points)) {}

    bool Triangle::isValid() const noexcept {
        const auto& vertices = points();
        return validVertices(std::span(vertices.data(), vertices.size()), 3) &&
               !hasSelfIntersections(std::span(vertices.data(), vertices.size()));
    }

    bool Triangle::isEquilateral() const noexcept {
        if (!isValid()) {
            return false;
        }
        const auto& vertices = points();
        const double a = distance(vertices[0], vertices[1]);
        const double b = distance(vertices[1], vertices[2]);
        const double c = distance(vertices[2], vertices[0]);
        return nearlyEqual(a, b) && nearlyEqual(b, c);
    }

    void Triangle::writeAttributes(std::ostream& stream) const {
        for (std::size_t i = 0; i < points().size(); ++i) {
            if (i != 0) stream << ' ';
            stream << points()[i].x << ' ' << points()[i].y;
        }
    }

    std::istream& Triangle::read(std::istream& stream) {
        readPoints(stream, 3);
        return stream;
    }

    Square::Square(points_vec&& points) noexcept : Polygon(std::move(points)) {}

    bool Square::isValid() const noexcept {
        const auto& vertices = points();
        const std::span pts(vertices.data(), vertices.size());
        if (!validVertices(pts, 4) || hasSelfIntersections(pts)) {
            return false;
        }

        double sides[4];
        for (std::size_t i = 0; i < 4; ++i) {
            sides[i] = distance(vertices[i], vertices[(i + 1) % 4]);
            if (!nearlyEqual(sides[0], sides[i])) {
                return false;
            }
        }

        for (std::size_t i = 0; i < 4; ++i) {
            const Point& previous = vertices[(i + 3) % 4];
            const Point& current = vertices[i];
            const Point& next = vertices[(i + 1) % 4];
            const double dot = (previous.x - current.x) * (next.x - current.x) +
                               (previous.y - current.y) * (next.y - current.y);
            if (std::fabs(dot) > EPS * std::max(1.0, sides[i] * sides[(i + 3) % 4])) {
                return false;
            }
        }
        return true;
    }

    double Square::side() const noexcept {
        if (points().size() != 4) {
            return 0.0;
        }
        return distance(points()[0], points()[1]);
    }

    void Square::writeAttributes(std::ostream& stream) const {
        for (std::size_t i = 0; i < points().size(); ++i) {
            if (i != 0) stream << ' ';
            stream << points()[i].x << ' ' << points()[i].y;
        }
    }

    std::istream& Square::read(std::istream& stream) {
        readPoints(stream, 4);
        return stream;
    }

    Pentagon::Pentagon(points_vec&& points) noexcept : Polygon(std::move(points)) {}

    bool Pentagon::isValid() const noexcept {
        const auto& vertices = points();
        const std::span pts(vertices.data(), vertices.size());
        return validVertices(pts, 5) && !hasSelfIntersections(pts);
    }

    bool Pentagon::isRegular() const noexcept {
        if (!isValid()) {
            return false;
        }

        const auto& vertices = points();
        const double firstSide = distance(vertices[0], vertices[1]);
        const double regularAngleCosine = std::cos(3.0 * std::numbers::pi / 5.0);
        for (std::size_t i = 0; i < 5; ++i) {
            const Point& previous = vertices[(i + 4) % 5];
            const Point& current = vertices[i];
            const Point& next = vertices[(i + 1) % 5];
            const double side = distance(current, next);
            if (!nearlyEqual(firstSide, side)) {
                return false;
            }

            const double ax = previous.x - current.x;
            const double ay = previous.y - current.y;
            const double bx = next.x - current.x;
            const double by = next.y - current.y;
            const double cosine = (ax * bx + ay * by) / (distance(previous, current) * side);
            if (!nearlyEqual(cosine, regularAngleCosine)) {
                return false;
            }
        }
        return true;
    }

    void Pentagon::writeAttributes(std::ostream& stream) const {
        for (std::size_t i = 0; i < points().size(); ++i) {
            if (i != 0) stream << ' ';
            stream << points()[i].x << ' ' << points()[i].y;
        }
    }

    std::istream& Pentagon::read(std::istream& stream) {
        readPoints(stream, 5);
        return stream;
    }

} //ns polygons
