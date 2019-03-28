// Copyright (C) 2019 Bolt Robotics <info@boltrobotics.com>
// License: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>

// SYSTEM INCLUDES
#include <gtest/gtest.h>

// PROJECT INCLUDES
#include "utility/defines.hpp"

//================================ TEST FIXTURES ===============================

//=================================== TESTS ====================================

namespace btr
{

TEST(StatusTest, set_status)
{
  uint32_t s1 = 0xFF000000;
  uint32_t s2 = 0x00FF0000;
  uint32_t vv = 0x0000FFFF;
  uint32_t status = vv;
  set_status(&status, s1);
  set_status(&status, s2);
  set_status(&status, 0);

  ASSERT_EQ((s1 | s2 | vv), status);
  ASSERT_EQ(false, is_ok(status));
  ASSERT_EQ(false, is_ok(&status));
  ASSERT_EQ(true, is_err(status));
  ASSERT_EQ(true, is_err(&status));

  status &= 0xFFFF0000;
  ASSERT_EQ((s1 | s2), status);
  ASSERT_EQ(false, is_ok(status));
  ASSERT_EQ(false, is_ok(&status));
  ASSERT_EQ(true, is_err(status));
  ASSERT_EQ(true, is_err(&status));

}

TEST(StatusTest, clear_status)
{
  uint32_t s1 = 0xFF000000;
  uint32_t s2 = 0x00FF0000;
  uint32_t vv = 0x0000FFFF;
  uint32_t status = vv;
  set_status(&status, s1);
  set_status(&status, s2);
  set_status(&status, 0);

  clear_status(&status);
  ASSERT_EQ(0x0000FFFF, status);
  ASSERT_EQ(true, is_ok(status));
  ASSERT_EQ(true, is_ok(&status));
  ASSERT_EQ(false, is_err(status));
  ASSERT_EQ(false, is_err(&status));
}

} // namespace btr