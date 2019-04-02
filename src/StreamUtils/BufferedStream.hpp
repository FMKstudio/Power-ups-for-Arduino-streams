// ArduinoJson - arduinojson.org
// Copyright Benoit Blanchon 2019
// MIT License

#pragma once

namespace StreamUtils {

class BufferedStream : public Stream {
 public:
  explicit BufferedStream(Stream &upstream, char *buffer, size_t capacity)
      : _upstream(upstream),
        _buffer(buffer),
        _capacity(capacity),
        _begin(buffer),
        _end(buffer) {}

  size_t write(const uint8_t *buffer, size_t size) override {
    return _upstream.write(buffer, size);
  }

  size_t write(uint8_t data) override {
    return _upstream.write(data);
  }

  int available() override {
    return _upstream.available() + _end - _begin;
  }

  int read() override {
    reloadIfEmpty();
    return isEmpty() ? -1 : *_begin++;
  }

  int peek() override {
    return isEmpty() ? _upstream.peek() : *_begin;
  }

  void flush() override {
    _upstream.flush();
  }

  // WARNING: we cannot use "override" because most cores don't define this
  // function as virtual
  virtual size_t readBytes(char *buffer, size_t size) {
    size_t result = 0;

    if (!isEmpty()) {
      size_t bytesInBuffer = _end - _begin;
      memcpy(buffer, _begin, bytesInBuffer);
      _begin += bytesInBuffer;
      result += bytesInBuffer;
      buffer += bytesInBuffer;
      size -= bytesInBuffer;
    }

    // at this point, the buffer is empty (or size is 0)
    if (size < _capacity) {
      reload();
      memcpy(buffer, _begin, size);
      _begin += size;
      result += size;
    } else {
      result += _upstream.readBytes(buffer, size);
    }
    return result;
  }

 private:
  bool isEmpty() const {
    return _begin >= _end;
  }

  void reloadIfEmpty() {
    if (!isEmpty()) return;
    reload();
  }

  void reload() {
    _begin = _buffer;
    _end = _begin + _upstream.readBytes(_buffer, _capacity);
  }

  Stream &_upstream;
  char *_buffer;
  size_t _capacity;
  char *_begin;
  char *_end;
};

template <size_t capacity>
class StaticBufferedStream : public BufferedStream {
 public:
  explicit StaticBufferedStream(Stream &upstream)
      : BufferedStream(upstream, _buffer, capacity) {}

 private:
  char _buffer[capacity];
};

template <typename Stream>
StaticBufferedStream<64> bufferizeInput(Stream &upstream) {
  return StaticBufferedStream<64>{upstream};
}

}  // namespace StreamUtils