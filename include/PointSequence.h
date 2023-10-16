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
    PointSequence(uint16_t initial_capacity = 10);
    virtual ~PointSequence();

    uint16_t size() const                               { return _size; }
    uint16_t capacity() const                           { return _capacity; }

    bool add(const Point& point);
    bool add(int x, int y)                              { return add(Point(x, y)); }
    bool add(const PointSequence& other);
    
    void clear();

    const Point& operator[](uint16_t index) const       { return index < _size ? _points[index] : _empty_point; }

    void write_to_stream(Stream& stream) const;
    operator String() const;
};

#endif // __POINTSEQUENCE_H__