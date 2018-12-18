// Copyright (C) 2018 Bolt Robotics <info@boltrobotics.com>
// License: GNU GPL v3

// SYSTEM INCLUDES
#include <gtest/gtest.h>

// PROJECT INCLUDES
#include "utility/x86/serial_io_boost.hpp"
#include "utility/x86/pseudo_tty.hpp"
#include "utility/buff.hpp"
#include "utility/test_helpers.hpp"

namespace btr
{

#define BAUD 115200

//------------------------------------------------------------------------------

class SerialIOBoostTest : public testing::Test
{
public:

  // LIFECYCLE

  SerialIOBoostTest()
  : tty_(),
    act_serial_(TTY_SIM_0, BAUD, 100),
    sim_serial_(TTY_SIM_1, BAUD, 100),
    wbuff_(),
    rbuff_(),
    success_(),
    timeout_(std::make_error_code(std::errc::operation_canceled))
  {
    reset();
  }

  void reset()
  {
    uint8_t h[] =  { 'h','e','l','l','o' };
    wbuff_.resize(sizeof(h));
    wbuff_.write(h, sizeof(h) / sizeof(uint8_t));
    // Don't expect to receive endline character(s)
    rbuff_.resize(wbuff_.size());
  }

protected:

  // ATTRIBUTES

  PseudoTTY tty_;
  SerialIOBoost act_serial_;
  SerialIOBoost sim_serial_;
  Buff wbuff_;
  Buff rbuff_;
  std::error_code success_;
  std::error_code timeout_;
};

//------------------------------------------------------------------------------

// Tests {

TEST_F(SerialIOBoostTest, ReadWriteOK)
{
  std::error_code e = sim_serial_.send(&wbuff_);
  ASSERT_EQ(success_, e) << " Message: " << e.message();

  e = act_serial_.recv(&rbuff_);

  ASSERT_EQ(success_, e) << " Message: " << e.message();
  ASSERT_EQ(0, memcmp(wbuff_.data(), rbuff_.data(), wbuff_.size())) << TestHelpers::toHex(rbuff_);
}

TEST_F(SerialIOBoostTest, DISABLED_Flush)
{
  std::error_code e = sim_serial_.send(&wbuff_);
  ASSERT_EQ(success_, e) << " Message: " << e.message();

  e = sim_serial_.flush();
  ASSERT_EQ(success_, e) << " Message: " << e.message();

  // FIXME: Flush test: serial_.recv generates an error
  e = act_serial_.recv(&rbuff_);
  ASSERT_EQ(timeout_, e) << " Message: " << e.message();

  reset();

  e = sim_serial_.send(&wbuff_);
  ASSERT_EQ(success_, e) << " Message: " << e.message();

  e = act_serial_.recv(&rbuff_);
  ASSERT_EQ(success_, e) << " Message: " << e.message();
  ASSERT_EQ(0, memcmp(wbuff_.data(), rbuff_.data(), wbuff_.size())) << TestHelpers::toHex(rbuff_);
}

TEST_F(SerialIOBoostTest, ReadTimeout)
{
  std::error_code e = act_serial_.recv(&rbuff_);
  ASSERT_EQ(timeout_, e) << " Message: " << e.message();
}

TEST_F(SerialIOBoostTest, DISABLED_WriteTimeout)
{
  // FIXME: Write time-out simulation doesn't work.
  Buff large_buff;
  large_buff.resize(65536);
  std::error_code e = sim_serial_.send(&large_buff);
  ASSERT_EQ(timeout_, e) << " Message: " << e.message();
}

// } Tests

} // namespace btr
