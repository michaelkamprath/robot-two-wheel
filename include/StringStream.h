#ifndef __STRINGSTREAM_H__
#define __STRINGSTREAM_H__
#include <Arduino.h>

class StringStream : public Stream {
private:
    String _buffer;
    unsigned int _position;

public:
    StringStream();
    virtual ~StringStream();

    void clear();
    void set_buffer(const String& buffer);

    operator String() const                     { return _buffer; }
    const String& to_string() const             { return _buffer; }

    // Stream interface
    virtual int available() override;
    virtual int read() override;
    virtual int peek() override;
    virtual size_t write(uint8_t) override;
};

#endif // __STRINGSTREAM_H__

