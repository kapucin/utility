/* Copyright (C) 2017 Bolt Robotics <info@boltrobotics.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#ifndef _btr_SerialIOTermios_hpp__
#define _btr_SerialIOTermios_hpp__

// SYSTEM INCLUDES
#include <termios.h>
#include <fcntl.h>
#include <sys/ioctl.h>

// PROJECT INCLUDES
#include "utility/buff.hpp"

namespace btr
{

/**
 * The class provides a send/receive interface to a serial port.
 */
class SerialIOTermios
{
public:

  // LIFECYCLE

  /**
   * Ctor.
   *
   * @param port - serial IO port
   * @param baud_rate - baud rate. It must be one of values specified by in termios.h
   *  @see http://man7.org/linux/man-pages/man3/termios.3.html
   * @param timeout - serial operation timeout in milliseconds
   */
  SerialIOTermios(const std::string& port, int baud_rate = 57600, int timeout = 0);

  /**
   * Dtor.
   */
  ~SerialIOTermios();

  // OPERATIONS

  /**
   * Close and open the port.
   */
  void reset();

  /**
   * Flush not-transmitted and non-read data on the serial port.
   */
  std::error_code flush();

  /**
   * @return bytes available on the serial port
   */
  uint32_t available();

  /**
   * Set the number of bytes to read.
   *
   * @param bytes - the byte count
   */
  void setReadMinimum(uint32_t bytes);

  /**
   * Read data from serial port.
   *
   * @param buff - the buffer to read into
   * @param bytes - the number of bytes to read
   */
  std::error_code recv(Buff* buff, uint32_t bytes);

  /**
   * Write data to serial port. The function increments buffer->read_ptr() by
   * the amount of buff->available() bytes on successful operation.
   *
   * @param data - the data to send
   */
  std::error_code send(Buff* buff);

private:

  // OPERATIONS

  /**
   * Convert numeric BAUD rate to termios-specific one.
   *
   * @param num - the number baud rate
   * @return termios baud rate
   */
  static int getNativeBaud(int num);

  // ATTRIBUTES

  std::string port_name_;
  int baud_rate_;
  int timeout_millis_;
  int port_;

}; // class SerialIOTermios

////////////////////////////////////////////////////////////////////////////////
// INLINE OPERATIONS
////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////// PUBLIC ///////////////////////////////////

//=================================== LIFECYCLE ================================

inline SerialIOTermios::SerialIOTermios(
    const std::string& port_name, int baud_rate, int timeout_millis)
: port_name_(port_name),
  baud_rate_(baud_rate),
  timeout_millis_(timeout_millis),
  port_(-1)
{
  baud_rate_ = getNativeBaud(baud_rate);
  reset();
}

inline SerialIOTermios::~SerialIOTermios()
{
  close(port_);
}

//=================================== OPERATIONS ===============================

inline void SerialIOTermios::reset()
{
  close(port_);

  port_ = open(port_name_.c_str(), O_RDWR | O_NOCTTY);

  if (port_ < 0) {
      throw "Failed to open " + port_name_;
  }

  struct termios options;
  tcgetattr(port_, &options);

  cfmakeraw(&options);
  cfsetospeed(&options, baud_rate_);
  cfsetispeed(&options, baud_rate_);

  //options.c_cflag = CS8 | CREAD | CLOCAL | baud_rate_;
  //options.c_iflag = IGNPAR | IGNCR;
  //options.c_lflag &= ~ICANON;
  options.c_cc[VTIME] = timeout_millis_ / 100; // VTIME is in tenths of seconds
  options.c_cc[VMIN] = 0;

  tcflush(port_, TCIOFLUSH);
  tcsetattr(port_, TCSANOW, &options);
}

inline std::error_code SerialIOTermios::flush()
{
  errno = 0;

  if (0 == tcflush(port_, TCIOFLUSH)) {
    return std::error_code(0, std::generic_category());
  } else {
    return std::error_code(errno, std::generic_category());
  }
}

inline uint32_t SerialIOTermios::available()
{
  uint32_t bytes_available;
  ioctl(port_, FIONREAD, &bytes_available);
  return bytes_available;
}

inline void SerialIOTermios::setReadMinimum(uint32_t bytes)
{
  struct termios options;
  tcgetattr(port_, &options);
  options.c_cc[VTIME] = 1;
  options.c_cc[VMIN] = bytes;
  tcsetattr(port_, TCSANOW, &options);
}

inline std::error_code SerialIOTermios::recv(Buff* buff, uint32_t bytes)
{
  if (buff->remaining() < bytes) {
    return std::error_code(ENOBUFS, std::generic_category());
  }

  std::error_code err(0, std::generic_category());

  do {
    errno = 0;
    ssize_t count = read(port_, buff->write_ptr(), bytes);

    if (count > 0) {
      // At least 1 byte is received, continue reading until received requested bytes
      buff->write_ptr() += count;
      bytes -= count;
    } else if (count == 0) {
      // If count is 0, we reached EOF or the call has timed out.
      err = std::error_code(ETIME, std::generic_category());
      break;
    } else {
      // -1 is received on error.
      err = std::error_code(errno, std::generic_category());
      break;
    }
  } while (bytes > 0);

  return err;
}

inline std::error_code SerialIOTermios::send(Buff* buff)
{
  errno = 0;
  int count = write(port_, buff->read_ptr(), buff->available());

  if (count >= 0) {
    return std::error_code(0, std::generic_category());
  } else {
    return std::error_code(errno, std::generic_category());
  }
}

///////////////////////////////////// PRIVATE //////////////////////////////////

//=================================== OPERATIONS ===============================

inline int SerialIOTermios::getNativeBaud(int num)
{
  int baud = B57600;

  switch (num) {
    case 9600:
      baud = B9600;
      break;
    case 38400:
      baud = B38400;
      break;
    case 57600:
      baud = B57600;
      break;
    case 115200:
      baud = B115200;
      break;
    default:
      break;
  };

  return baud;
}

} // namespace btr

#endif // _btr_SerialIOTermios_hpp__