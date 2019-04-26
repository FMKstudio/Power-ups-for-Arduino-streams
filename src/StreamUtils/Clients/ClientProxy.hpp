// StreamUtils - github.com/bblanchon/ArduinoStreamUtils
// Copyright Benoit Blanchon 2019
// MIT License

#pragma once

#include <Client.h>

#include "../Configuration.hpp"
#include "../Streams/StreamProxy.hpp"

namespace StreamUtils {

template <typename ReadPolicy, typename WritePolicy, typename ConnectPolicy>
class ClientProxy : public Client {
 public:
  explicit ClientProxy(Client &upstream, ReadPolicy reader, WritePolicy writer,
                       ConnectPolicy connection)
      : _target(upstream),
        _reader(reader),
        _writer(writer),
        _connection(connection) {}

  ClientProxy(const ClientProxy &other)
      : _target(other._target),
        _reader(other._reader),
        _writer(other._writer),
        _connection(other._connection) {}

  ~ClientProxy() {
    _writer.detach(_target);
  }

  // --- Print ---

  size_t write(const uint8_t *buffer, size_t size) override {
    return _writer.write(_target, buffer, size);
  }

  size_t write(uint8_t data) override {
    return _writer.write(_target, data);
  }

  using Print::write;

  // --- Stream ---

  int available() override {
    return _reader.available(_target);
  }

  int read() override {
    return _reader.read(_target);
  }

  int peek() override {
    return _reader.peek(_target);
  }

  void flush() override {
    _writer.flush(_target);
  }

  // WARNING: we cannot use "override" because most cores don't define this
  // function as virtual
  virtual size_t readBytes(char *buffer, size_t size) {
    return _reader.readBytes(_target, buffer, size);
  }

  // --- Client ---

#if STREAMUTILS_CLIENT_CONNECT_TAKE_CONST_REF
  int connect(const IPAddress &ip, uint16_t port) override {
#else
  int connect(IPAddress ip, uint16_t port) override {
#endif
    return _connection.connect(_target, ip, port);
  }

  int connect(const char *ip, uint16_t port) override {
    return _connection.connect(_target, ip, port);
  }

  uint8_t connected() override {
    return _connection.connected(_target);
  }

#if STREAMUTILS_CLIENT_STOP_TAKES_TIMEOUT
  bool stop(unsigned timeout) override {
    return _connection.stop(_target, timeout);
  }
#else
  void stop() override {
    _connection.stop(_target);
  }
#endif

  operator bool() override {
    return _connection.operator_bool(_target);
  }

  int read(uint8_t *buf, size_t size) override {
    return _reader.read(_target, buf, size);
  }

#if STREAMUTILS_CLIENT_FLUSH_TAKES_TIMEOUT
  bool flush(unsigned timeout) override {
    return _writer.flush(_target, timeout);
  }
#endif

 private:
  Client &_target;
  ReadPolicy _reader;
  WritePolicy _writer;
  ConnectPolicy _connection;
};

}  // namespace StreamUtils