#ifndef POLYGONS_HPP
#define POLYGONS_HPP

namespace polygons {
    struct Point {
        double x;
        double y;

        Point(double _x, double _y) : x(_x), y(_y) {}
        Point operator + (const Point &other) const {
            return {x + other.x, y + other.y};
        }
        Point operator - (const Point &other) const {
            return {x - other.x, y - other.y};
        }
    };

    class Polygon {
        pr
    };
}


#endif //POLYGONS_HPP
