/**
 * @brief A simple serial port driver.
 */

#pragma once

#include <termios.h>
#include <atomic>
#include <string>

#include <boost/asio.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>

namespace point_one {
namespace applications {

class SerialPort {
 public:
  typedef std::function<void(const uint8_t*, size_t)> CallbackFn;

  SerialPort() = delete;

  SerialPort(boost::asio::io_service* io_svs);

  ~SerialPort();

  void Close();

  bool Open(const std::string& port_name, int baud_rate,
            const CallbackFn& callback);

  bool Open(const std::string& port_name, int baud_rate);

  void Write(const uint8_t* buf, size_t len);

  void Write(const std::string& buf);

 private:
  boost::asio::io_service* io_service_;
  boost::asio::serial_port port_;
  std::string port_name_;
  CallbackFn callback_;

  static const int READ_SIZE = 1024;
  char buf_[READ_SIZE];

  std::atomic<bool> shutting_down_;

  bool SetSpeed(boost::asio::serial_port& p, unsigned baud_rate_bps);

  void AsyncReadData();

  void OnReceive(const boost::system::error_code& error_code,
                 size_t bytes_transferred);
};

} // namespace applications
} // namespace point_one
