#include "polygons.hpp"

#include <array>
#include <cmath>
#include <memory>
#include <sstream>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

namespace {
    using polygons::Point;
    using polygons::Polygon;
    using polygons::Triangle;
    using polygons::Square;
    using polygons::Pentagon;

    const std::array<Point, 3> trianglePoints{{{0.0, 0.0}, {4.0, 0.0}, {2.0, 3.0}}};
    const std::array<Point, 3> equilateralPoints{{{0.0, 0.0}, {2.0, 0.0}, {1.0, std::sqrt(3.0)}}};
    const std::array<Point, 4> squarePoints{{{0.0, 0.0}, {2.0, 0.0}, {2.0, 2.0}, {0.0, 2.0}}};
    const std::array<Point, 4> rectanglePoints{{{0.0, 0.0}, {4.0, 0.0}, {4.0, 2.0}, {0.0, 2.0}}};
    const std::array<Point, 4> bowTiePoints{{{0.0, 0.0}, {2.0, 2.0}, {0.0, 2.0}, {2.0, 0.0}}};
    const std::array<Point, 3> collinearPoints{{{0.0, 0.0}, {1.0, 1.0}, {2.0, 2.0}}};

    std::array<Point, 5> regularPentagonPoints() {
        std::array<Point, 5> points{};
        for (std::size_t i = 0; i < points.size(); ++i) {
            const double angle = 2.0 * std::numbers::pi * static_cast<double>(i) / 5.0;
            points[i] = {std::cos(angle), std::sin(angle)};
        }
        return points;
    }
}

TEST(TriangleTest, ComputesArea) {
    const auto triangle = Polygon::fromVertices<Triangle>(trianglePoints);
    ASSERT_TRUE(triangle.has_value());
    EXPECT_DOUBLE_EQ(triangle->metric(), 6.0);
}

TEST(TriangleTest, ComputesCenterAsVertexAverage) {
    const auto triangle = Polygon::fromVertices<Triangle>(trianglePoints);
    ASSERT_TRUE(triangle.has_value());
    ASSERT_TRUE(triangle->center().has_value());
    EXPECT_NEAR(triangle->center()->x, 2.0, polygons::EPS);
    EXPECT_NEAR(triangle->center()->y, 1.0, polygons::EPS);
}

TEST(TriangleTest, DetectsEquilateralTriangle) {
    const auto triangle = Polygon::fromVertices<Triangle>(equilateralPoints);
    ASSERT_TRUE(triangle.has_value());
    EXPECT_TRUE(triangle->isEquilateral());
}

TEST(SquareTest, ComputesAreaAndSide) {
    const auto square = Polygon::fromVertices<Square>(squarePoints);
    ASSERT_TRUE(square.has_value());
    EXPECT_DOUBLE_EQ(square->metric(), 4.0);
    EXPECT_DOUBLE_EQ(square->side(), 2.0);
}

TEST(SquareTest, RejectsRectangleAsSquare) {
    EXPECT_FALSE(Polygon::fromVertices<Square>(rectanglePoints).has_value());
}

TEST(PentagonTest, DetectsRegularPentagon) {
    const auto vertices = regularPentagonPoints();
    const auto pentagon = Polygon::fromVertices<Pentagon>(vertices);
    ASSERT_TRUE(pentagon.has_value());
    EXPECT_TRUE(pentagon->isRegular());
}

TEST(PentagonTest, ComputesArea) {
    const auto vertices = regularPentagonPoints();
    const auto pentagon = Polygon::fromVertices<Pentagon>(vertices);
    ASSERT_TRUE(pentagon.has_value());
    EXPECT_NEAR(pentagon->metric(), 2.5 * std::sin(2.0 * std::acos(-1.0) / 5.0), polygons::EPS);
}

TEST(FactoryTest, RejectsCollinearTriangle) {
    EXPECT_FALSE(Polygon::fromVertices<Triangle>(collinearPoints).has_value());
}

TEST(FactoryTest, ReportsSelfIntersection) {
    Polygon::points_vec points;
    for (const Point& point : bowTiePoints) {
        points.push_back(point);
    }
    polygons::PolygonError error{};
    const auto square = Polygon::fromVerticesOwnedExplicit<Square>(std::move(points), error);
    EXPECT_FALSE(square.has_value());
    EXPECT_EQ(error, polygons::PolygonError::SelfIntersection);
}

TEST(StreamTest, OutputContainsNameVerticesMetricAndCenter) {
    const auto triangle = Polygon::fromVertices<Triangle>(trianglePoints);
    ASSERT_TRUE(triangle.has_value());
    std::ostringstream output;
    output << *triangle;
    EXPECT_NE(output.str().find("Triangle"), std::string::npos);
    EXPECT_NE(output.str().find("(4, 0)"), std::string::npos);
    EXPECT_NE(output.str().find("Metric: 6"), std::string::npos);
    EXPECT_NE(output.str().find("Center:"), std::string::npos);
}

TEST(StreamTest, ReadsCoordinatesAndComputesMetric) {
    const auto initial = Polygon::fromVertices<Triangle>(trianglePoints);
    ASSERT_TRUE(initial.has_value());
    Triangle triangle = *initial;
    std::istringstream input("0 0  2 0  0 2");
    input >> triangle;
    EXPECT_FALSE(input.fail());
    EXPECT_DOUBLE_EQ(triangle.metric(), 2.0);
}

TEST(StreamTest, InvalidInputSetsFailbitAndPreservesPreviousFigure) {
    const auto initial = Polygon::fromVertices<Triangle>(trianglePoints);
    ASSERT_TRUE(initial.has_value());
    Triangle triangle = *initial;
    std::istringstream input("0 0  1 1  2 2");
    input >> triangle;
    EXPECT_TRUE(input.fail());
    EXPECT_DOUBLE_EQ(triangle.metric(), 6.0);
}

TEST(PolymorphismTest, DynamicCastFindsCorrectType) {
    const auto squareValue = Polygon::fromVertices<Square>(squarePoints);
    ASSERT_TRUE(squareValue.has_value());
    std::unique_ptr<Polygon> polygon = std::make_unique<Square>(*squareValue);
    EXPECT_NE(dynamic_cast<Square*>(polygon.get()), nullptr);
    EXPECT_EQ(dynamic_cast<Triangle*>(polygon.get()), nullptr);
}

TEST(PolymorphismTest, DoubleConversionReturnsMetric) {
    const auto triangle = Polygon::fromVertices<Triangle>(trianglePoints);
    ASSERT_TRUE(triangle.has_value());
    EXPECT_DOUBLE_EQ(static_cast<double>(*triangle), triangle->metric());
}

TEST(PolymorphismTest, EqualityComparesMetricWithTolerance) {
    const auto first = Polygon::fromVertices<Triangle>(trianglePoints);
    const std::array<Point, 3> sameAreaPoints{{{0.0, 0.0}, {3.0, 0.0}, {2.0, 4.0}}};
    const std::array<Point, 3> differentAreaPoints{{{0.0, 0.0}, {2.0, 0.0}, {0.0, 2.0}}};
    const auto sameArea = Polygon::fromVertices<Triangle>(sameAreaPoints);
    const auto differentArea = Polygon::fromVertices<Triangle>(differentAreaPoints);
    ASSERT_TRUE(first.has_value());
    ASSERT_TRUE(sameArea.has_value());
    ASSERT_TRUE(differentArea.has_value());
    EXPECT_TRUE(*first == *sameArea);
    EXPECT_FALSE(*first == *differentArea);
}

TEST(PolymorphismTest, BasePointersDispatchMetricVirtually) {
    const auto triangleValue = Polygon::fromVertices<Triangle>(trianglePoints);
    const auto squareValue = Polygon::fromVertices<Square>(squarePoints);
    ASSERT_TRUE(triangleValue.has_value());
    ASSERT_TRUE(squareValue.has_value());
    std::vector<std::unique_ptr<Polygon>> figures;
    figures.push_back(std::make_unique<Triangle>(*triangleValue));
    figures.push_back(std::make_unique<Square>(*squareValue));
    double totalMetric = 0.0;
    for (const auto& figure : figures) {
        totalMetric += figure->metric();
    }
    EXPECT_DOUBLE_EQ(totalMetric, 10.0);
}
