#ifndef __POINTSEQUENCE_H__
#define __POINTSEQUENCE_H__
#include <Arduino.h>
#include "Point.h"

class PointSequence {
private:
    Point* _points;
    uint16_t _capacity;
    uint16_t _size;

protected:
    bool expand(void);

    static const Point _empty_point;
public:
    /// @brief Construct a new PointSequence object
    /// @param initial_capacity The initial capacity of the sequence. If the sequence grows beyond this capacity,
    /// the sequence's storage will be doubled.
    PointSequence(uint16_t initial_capacity = 10);
    virtual ~PointSequence();

    /// @brief Provides the number of points in the sequence.
    /// @return the number of points in the sequence.
    uint16_t size() const                               { return _size; }

    /// @brief Provides the currently allocated capacity of this container.
    /// @return the currently allocated capacity of this container.
    uint16_t capacity() const                           { return _capacity; }

    /// @brief Adds a point to the sequence.
    /// @param point The point to add.
    /// @return true if the point was added, false otherwise.
    bool add(const Point& point);

    /// @brief Adds a point to the sequence.
    /// @param x The x coordinate of the point to add.
    /// @param y The y coordinate of the point to add.
    /// @return true if the point was added, false otherwise.
    bool add(int x, int y)                              { return add(Point(x, y)); }

    /// @brief Adds a point to the sequence. The order of the points is maintained.
    /// @param other The `PointSequence` to add.
    /// @return true if the points were added, false otherwise.
    bool add(const PointSequence& other);

    /// @brief Removes all points from the sequence. Capcity is not changed.
    void clear();

    /// @brief Provides access to a point in the sequence.
    /// @param index The index of the point to access.
    /// @return The point at the specified index. If the index is out of range, an empty point is returned.
    const Point& operator[](uint16_t index) const       { return index < _size ? _points[index] : _empty_point; }

    /// @brief Writes a debug rerpesentations of the contents of the sequence to a stream.
    /// @param stream The `Stream` to write to.
    void write_to_stream(Stream& stream) const;

    /// @brief Converts the sequence to a string.
    /// @return A string representation of the sequence.
    operator String() const;
};

#endif // __POINTSEQUENCE_H__