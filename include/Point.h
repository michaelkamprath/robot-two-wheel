#ifndef POINT_H
#define POINT_H
#include <Arduino.h>

class Point {
private:
    int _x;
    int _y;

public:
    Point() : _x(0), _y(0)                              { }
    Point(int x, int y) : _x(x), _y(y)                  { }

    int x() const                                       { return _x; }
    int y() const                                       { return _y; }

    Point& operator=(const Point& other)                { _x = other._x; _y = other._y; return *this; }
    bool operator==(const Point& other) const           { return _x == other._x && _y == other._y; }
    bool operator!=(const Point& other) const           { return !(*this == other); }
    Point operator+(const Point& other) const           { return Point(_x + other._x, _y + other._y); }

    operator String() const                             { return String("(") + _x + "," + _y + ")"; }
    double distance(const Point& other) const           { return sqrt(pow(_x - other._x, 2) + pow(_y - other._y, 2)); }

    // absolute bearing is the angle from this point to the other point, in degrees. 0 degrees pointing postively in 
    // the y axis, and the right hand rule is used to determine the angle. 90 degrees is pointing negatively in 
    // the x axis, 180 degrees is pointing negatively in the y axis, and -90 degrees is pointing positively in
    // the x axis.
    double absolute_bearing(const Point& other) const   { return atan2(-(other._x - _x), other._y - _y)*(180.0/PI); }
};

#endif // POINT_H