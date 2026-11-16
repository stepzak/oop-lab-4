#include "polygons.hpp"
#include <utility>
#include <cmath>
#include <iostream>


namespace polygons {
    namespace {
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
        polygon.printName(stream);
        stream << '\n';
        polygon.printAttributes(stream);
        auto center = polygon.center();
        stream << '\n' << "Metric: " << polygon.metric() << '\n' << "Center: ";
        if (center.has_value()) {
            stream << center.value();
        } else {
            stream << "none";
        }
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

} //ns polygons