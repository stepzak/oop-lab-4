#include "polygon_collection.hpp"

#include <array>
#include <iomanip>
#include <iostream>
#include <numbers>

int main() {
    using namespace polygons;

    const std::array<Point, 3> trianglePoints{{{0.0, 0.0}, {4.0, 0.0}, {2.0, 3.0}}};
    const std::array<Point, 4> squarePoints{{{5.0, 0.0}, {7.0, 0.0}, {7.0, 2.0}, {5.0, 2.0}}};
    std::array<Point, 5> pentagonPoints{};
    for (std::size_t i = 0; i < pentagonPoints.size(); ++i) {
        const double angle = 2.0 * std::numbers::pi * static_cast<double>(i) / 5.0;
        pentagonPoints[i] = {3.0 + std::cos(angle), 5.0 + std::sin(angle)};
    }

    PolygonCollection figures;
    figures.emplace<Triangle>(trianglePoints);
    figures.emplace<Square>(squarePoints);
    figures.emplace<Pentagon>(pentagonPoints);

    std::size_t index = 1;
    for (const auto& figure : figures) {
        std::cout << "Figure " << index++ << ":\n"
                  << "Vertices: " << *figure
                  << "Area: " << figure->metric() << '\n';

        if (const auto center = figure->center()) {
            std::cout << "Center: (" << center->x << ", " << center->y << ")\n";
        }

        if (const auto* triangle = dynamic_cast<const Triangle*>(figure.get())) {
            std::cout << "Equilateral: " << (triangle->isEquilateral() ? "yes" : "no") << '\n';
        } else if (const auto* square = dynamic_cast<const Square*>(figure.get())) {
            std::cout << "Side: " << square->side() << '\n';
        } else if (const auto* pentagon = dynamic_cast<const Pentagon*>(figure.get())) {
            std::cout << "Regular: " << (pentagon->isRegular() ? "yes" : "no") << '\n';
        }

        std::cout << '\n';
    }

    std::cout << "Total area: " << figures.totalMetric() << '\n';
    return 0;
}
