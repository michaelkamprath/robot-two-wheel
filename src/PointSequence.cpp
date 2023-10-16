#include "PointSequence.h"
#include "StringStream.h"

const Point PointSequence::_empty_point = Point(0, 0);

PointSequence::PointSequence(uint16_t initial_capacity) 
    :   _capacity(initial_capacity),
        _size(0) 
{
    _points = new Point[_capacity];
}

PointSequence::~PointSequence() {
    delete[] _points;
}

bool PointSequence::expand(void) {
    if (_size < _capacity) {
        return true;
    }

    Point* new_points = new Point[_capacity * 2];
    if (new_points == NULL) {
        return false;
    }

    memcpy(new_points, _points, sizeof(Point) * _capacity);
    delete[] _points;
    _points = new_points;
    _capacity *= 2;
    return true;
}

bool PointSequence::add(const Point& point) {
    if (_size == _capacity) {
        if (!expand()) {
            return false;
        }
    }
    _points[_size++] = point;
    return true;
}

bool PointSequence::add(const PointSequence& other) {
    for (uint16_t i = 0; i < other.size(); i++) {
        if (!add(other[i])) {
            return false;
        }
    }
    return true;
}

void PointSequence::clear() {
    _size = 0;
}

void PointSequence::write_to_stream(Stream& stream) const {
    stream.print("[");
    for (uint16_t i = 0; i < _size; i++) {
        stream.print(_points[i]);
        if (i < _size - 1) {
            stream.print(",");
        }
    }
    stream.print("]");
}

PointSequence::operator String() const {
    StringStream ss;
    write_to_stream(ss);
    return ss.to_string();
}