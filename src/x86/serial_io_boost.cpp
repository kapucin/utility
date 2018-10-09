/* Copyright (C) 2018 Bolt Robotics <info@boltrobotics.com> */

// SYSTEM INCLUDES
#include <boost/bind.hpp>

// PROJECT INCLUDES
#include "utility/x86/serial_io_boost.hpp"  // class implemented
#include "utility/buff.hpp"

namespace btr
{

/////////////////////////////////////////////// PUBLIC /////////////////////////////////////////////

//============================================= LIFECYCLE ==========================================

SerialIOBoost::SerialIOBoost(const std::string& port, int baud_rate, int timeout)
  :
  io_service_(),
  serial_(io_service_, port),
  timer_(io_service_),
  err_(),
  expected_bytes_(0),
  timeout_(timeout)
{
  serial_.set_option(bio::serial_port::baud_rate(baud_rate));
}

SerialIOBoost::~SerialIOBoost()
{
  serial_.close();
}

//============================================= OPERATIONS =========================================

std::error_code SerialIOBoost::flush()
{
  if (0 == tcflush(serial_.lowest_layer().native_handle(), TCIOFLUSH)) {
    err_.clear();
  } else {
    err_ = std::error_code(errno, std::generic_category());
  }
  return err_;
}

std::error_code SerialIOBoost::recv(Buff* buff)
{
  io_service_.reset();
  expected_bytes_ = buff->remaining();

  bio::async_read(
      serial_,
      bio::buffer(buff->write_ptr(), expected_bytes_),
      boost::bind(
        &SerialIOBoost::onOprComplete,
        this,
        bio::placeholders::error,
        bio::placeholders::bytes_transferred));

  timeAsyncOpr();

  if (!err_) {
    buff->write_ptr() += expected_bytes_;
  }
  return err_;
}

std::error_code SerialIOBoost::send(Buff* buff)
{
  io_service_.reset();
  expected_bytes_ = buff->available();

  bio::async_write(
      serial_,
      bio::buffer(buff->read_ptr(), expected_bytes_),
      boost::bind(&SerialIOBoost::onOprComplete,
        this,
        bio::placeholders::error,
        bio::placeholders::bytes_transferred));

  timeAsyncOpr();

  if (!err_) {
    buff->read_ptr() += expected_bytes_;
  }

  return err_;
}

/////////////////////////////////////////////// PROTECTED //////////////////////////////////////////

//============================================= OPERATIONS =========================================

/////////////////////////////////////////////// PRIVATE ////////////////////////////////////////////

//============================================= OPERATIONS =========================================

void SerialIOBoost::timeAsyncOpr()
{
  timer_.expires_from_now(boost::posix_time::milliseconds(timeout_));
  timer_.async_wait(boost::bind(&SerialIOBoost::onTimeout, this, bio::placeholders::error));
  io_service_.run();
}

void SerialIOBoost::onOprComplete(const boost::system::error_code& err, size_t bytes_transferred)
{
  if (err) {
    err_ = std::make_error_code(static_cast<std::errc>(err.value()));
  } else if (bytes_transferred != expected_bytes_) {
    err_ = std::make_error_code(std::errc::message_size);
  } else {
    err_.clear();
  }

  timer_.cancel();
}

void SerialIOBoost::onTimeout(const boost::system::error_code& error)
{
  // When the timer is cancelled, the error is generated.
  //
  if (!error) {
    serial_.cancel(); // timed out
  }
}

} // namespace btr