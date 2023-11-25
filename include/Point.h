#ifndef POINT_H
#define POINT_H
#include <Arduino.h>

/// @brief A class that represents a point in 2D space. Enables basic operations on points,
/// such as addition, subtraction, and distance calculation.
class Point {
private:
    int _x;
    int _y;

public:
    /// @brief Construct a new Point object
    Point() : _x(0), _y(0)                              { }
    /// @brief Construct a new Point object
    /// @param x The x coordinate of the point
    /// @param y The y coordinate of the point
    Point(int x, int y) : _x(x), _y(y)                  { }

    /// @brief Provides the x coordinate of the point
    /// @return the x coordinate of the point
    int x() const                                       { return _x; }

    /// @brief Provides the y coordinate of the point
    /// @return the y coordinate of the point
    int y() const                                       { return _y; }

    /// @brief Sets the the `Point` to be equal to another `Point` object
    /// @param other The `Point` to copy
    Point& operator=(const Point& other)                { _x = other._x; _y = other._y; return *this; }

    /// @brief Equality operator. Determines if two points are equal.
    /// @param other The `Point` to compare to
    /// @return `true` if the points are equal, `false` otherwise.
    bool operator==(const Point& other) const           { return _x == other._x && _y == other._y; }

    /// @brief Inequality operator. Determines if two points are not equal.
    /// @param other The `Point` to compare to
    /// @return `true` if the points are not equal, `false` otherwise.
    bool operator!=(const Point& other) const           { return !(*this == other); }

    /// @brief Addition operator. Adds two points together.
    /// @param other The `Point` to add to this `Point`
    /// @return A new `Point` that is the sum of the two points.
    Point operator+(const Point& other) const           { return Point(_x + other._x, _y + other._y); }

    /// @brief Converts the `Point` to a string for display.
    /// @return A string representation of the `Point`
    operator String() const                             { return String("(") + _x + "," + _y + ")"; }

    /// @brief Calculates the Euclidean distance between this point and another point.
    /// @param other the other point to calculate the distance to
    /// @return the Euclidean distance between the two points
    double distance(const Point& other) const           { return sqrt(pow(_x - other._x, 2) + pow(_y - other._y, 2)); }

    /// @brief Calculates the absolute bearing from this point to another point, in degrees. Absolute bearing is
    /// the angle from this point to the other point, in degrees.
    /// @param other The other point to calculate the absolute bearing to
    /// @return The absolute bearing from this point to the other point, in radians. 0 degrees pointing postively in the y axis, and
    /// the right hand rule is used to determine the angle. 90 degrees radians is pointing negatively in the x axis, 180 degrees
    /// radians is pointing negatively in the y axis, and -90 degrees is pointing positively in the x axis
    double absolute_bearing(const Point& other) const   { return atan2(-(other._x - _x), other._y - _y)*(180.0/PI); }
};

#endif // POINT_H