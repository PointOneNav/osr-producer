/**
 * @brief A simple serial port driver.
 */

#include "serial_port.h"

#include <boost/bind.hpp>

#if 0
#include <glog/logging.h>
#else
static std::ostream null_stream(0);
#define LOG(severity) null_stream
#define VLOG(severity) null_stream
#endif

using namespace boost::asio::ip;
using namespace point_one::applications;

/******************************************************************************/
SerialPort::SerialPort(boost::asio::io_service* io_svs)
    : io_service_(io_svs), port_(*io_svs), shutting_down_(false) {}

/******************************************************************************/
SerialPort::~SerialPort() { Close(); }

/******************************************************************************/
void SerialPort::Close() {
  shutting_down_ = true;
  if (port_.is_open()) {
    port_.close();
  }
}

/******************************************************************************/
bool SerialPort::Open(const std::string& port_name, int baud_rate,
                      const CallbackFn& callback) {
  bool success = Open(port_name, baud_rate);
  if (success) {
    callback_ = callback;
    AsyncReadData();
  }
  return success;
}

/******************************************************************************/
bool SerialPort::Open(const std::string& port_name, int baud_rate) {
  boost::system::error_code error_code;
  this->port_name_ = port_name;
  if (port_.is_open()) {
    LOG(ERROR) << "The port '" << port_name_ << "' is already opened.";
    return false;
  }

  port_.open(port_name_, error_code);
  if (error_code) {
    LOG(ERROR) << "Opening of port " << port_name_
               << " failed: " << error_code.message();
    return false;
  }
  LOG(INFO) << "Connected to serial device '" << port_name << "' @ "
            << baud_rate << ".";

  // Setup The options
  VLOG(1) << "SET BAUD RATE: " << baud_rate;
  if (!SetSpeed(port_, baud_rate)) {
    port_.close();
    return false;
  }
  VLOG(1) << "character_size: " << 8;
  port_.set_option(boost::asio::serial_port_base::character_size(8));
  VLOG(1) << "stop_bits: " << boost::asio::serial_port_base::stop_bits::one;
  port_.set_option(boost::asio::serial_port_base::stop_bits(
      boost::asio::serial_port_base::stop_bits::one));
  VLOG(1) << "parity: " << boost::asio::serial_port_base::parity::none;
  port_.set_option(boost::asio::serial_port_base::parity(
      boost::asio::serial_port_base::parity::none));
  port_.set_option(boost::asio::serial_port_base::flow_control(
      boost::asio::serial_port_base::flow_control::none));

  return true;
}

/******************************************************************************/
void SerialPort::Write(const uint8_t* buf, size_t len) {
  if (!port_.is_open()) return;

  VLOG(1) << "Sending " << len << " bytes to '" << port_name_ << "'.";

  size_t written = 0;
  while (written < len) {
    written += port_.write_some(
        boost::asio::const_buffer(buf + written, len - written));
  }
}

/******************************************************************************/
void SerialPort::Write(const std::string& buf) {
  Write(reinterpret_cast<const uint8_t*>(buf.data()), buf.size());
}

/******************************************************************************/
bool SerialPort::SetSpeed(boost::asio::serial_port& p, unsigned baud_rate_bps) {
  termios t;
  int fd = p.native_handle();

  if (tcgetattr(fd, &t) < 0) {
    LOG(ERROR)
        << "Error querying serial port attributes. Could not set baud rate.";
    return false;
  }
  else if (cfsetspeed(&t, baud_rate_bps) < 0) {
    LOG(ERROR) << "Error setting serial port baud rate value.";
    return false;
  }
  else if (tcsetattr(fd, 0, &t) < 0) {
    LOG(ERROR)
        << "Error setting serial port attributes. Could not set baud rate.";
    return false;
  }
  else {
    return true;
  }
}

/******************************************************************************/
void SerialPort::AsyncReadData() {
  if (!port_.is_open()) return;

  port_.async_read_some(
      boost::asio::buffer(buf_, READ_SIZE),
      boost::bind(&SerialPort::OnReceive, this,
                  boost::asio::placeholders::error,
                  boost::asio::placeholders::bytes_transferred));
}

/******************************************************************************/
void SerialPort::OnReceive(const boost::system::error_code& error_code,
                           size_t bytes_transferred) {
  if (!port_.is_open()) {
    return;
  }
  else if (shutting_down_) {
    return;
  }

  if (error_code) {
    LOG(ERROR) << "Error receiving data on '" << port_name_ << "'; "
               << error_code.message();

    // Attempt to reopen.
    boost::system::error_code error_code_retry;
    port_.close();
    port_.open(port_name_, error_code_retry);
    if (error_code_retry) {
      LOG(ERROR) << "Retry failed on device '" << port_name_ << "'; "
                 << error_code_retry.message();
      return;
    }
    AsyncReadData();
    return;
  }

  VLOG(4) << "Received " << bytes_transferred << " bytes on '" << port_name_
          << "'; " << error_code.message();

  if (callback_) callback_((uint8_t*)buf_, bytes_transferred);

  AsyncReadData();
}
