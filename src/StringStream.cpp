#include "StringStream.h"

StringStream::StringStream() : _buffer(), _position(0) {
}

StringStream::~StringStream() {
}

void StringStream::clear() {
    _buffer = "";
    _position = 0;
}

void StringStream::set_buffer(const String& buffer) {
    _buffer = buffer;
    _position = 0;
}

int StringStream::available() {
    return _buffer.length() - _position;
}

int StringStream::read() {
    if (_position >= _buffer.length()) {
        return -1;
    }
    return _buffer[_position++];
}

int StringStream::peek() {
    if (_position >= _buffer.length()) {
        return -1;
    }
    return _buffer[_position];
}

size_t StringStream::write(uint8_t c) {
    _buffer += (char) c;
    _position++;
    return 1;
}

