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

// SYSTEM INCLUDES
#include <gtest/gtest.h>
#include <iostream>
#include <sstream>
#include <limits>

// PROJECT INCLUDES
#include "utility/buff.hpp"

namespace btr
{

//------------------------------------------------------------------------------

class BuffTest : public testing::Test
{
public:

  // LIFECYCLE

  BuffTest()
  : buff_(1)
  {
  }

  // ATTRIBUTES

  btr::Buff buff_;
};

//------------------------------------------------------------------------------

TEST_F(BuffTest, reserve)
{
  ASSERT_EQ(uint32_t(1), buff_.capacity());
  ASSERT_EQ(uint32_t(1), buff_.size());
  ASSERT_EQ(uint32_t(0), buff_.available());
  ASSERT_EQ(uint32_t(1), buff_.remaining());

  bool success = buff_.reserve(5);
  ASSERT_EQ(true, success);

  buff_.write_ptr() += 1;
  ASSERT_EQ(uint32_t(5), buff_.capacity());
  ASSERT_EQ(uint32_t(1), buff_.size());
  ASSERT_EQ(uint32_t(1), buff_.available());
  ASSERT_EQ(uint32_t(0), buff_.remaining());

  const uint8_t* data = buff_.data();
  success = buff_.reserve(4097);

  ASSERT_EQ(true, success);
  ASSERT_TRUE(buff_.data() != nullptr);
  ASSERT_TRUE(buff_.data() != data);
  ASSERT_EQ(uint32_t(4097), buff_.capacity());
  ASSERT_EQ(uint32_t(1), buff_.size());
  ASSERT_EQ(uint32_t(1), buff_.available());
  ASSERT_EQ(uint32_t(0), buff_.remaining());

  data = buff_.data();
  success = buff_.reserve(4);

  ASSERT_EQ(true, success);
  ASSERT_TRUE(buff_.data() != nullptr);
  // Data may have moved to a different memory location
  //ASSERT_TRUE(buff_.data() == data);
  ASSERT_EQ(uint32_t(4), buff_.capacity());
  ASSERT_EQ(uint32_t(1), buff_.size());
  ASSERT_EQ(uint32_t(1), buff_.available());
  ASSERT_EQ(uint32_t(0), buff_.remaining());
}

TEST_F(BuffTest, resize)
{
  buff_.reserve(4097);

  ASSERT_EQ(uint32_t(4097), buff_.capacity());
  ASSERT_EQ(uint32_t(1), buff_.size());
  ASSERT_EQ(uint32_t(0), buff_.available());
  ASSERT_EQ(uint32_t(1), buff_.remaining());

  bool success = buff_.resize(5);
  ASSERT_EQ(true, success);

  buff_.write_ptr() += 3;
  ASSERT_EQ(uint32_t(4097), buff_.capacity());
  ASSERT_EQ(uint32_t(5), buff_.size());
  ASSERT_EQ(uint32_t(3), buff_.available());
  ASSERT_EQ(uint32_t(2), buff_.remaining());

  buff_.read_ptr() += 2;
  const uint8_t* data = buff_.data();
  success = buff_.resize(3);

  ASSERT_EQ(true, success);
  ASSERT_TRUE(buff_.data() != nullptr);
  ASSERT_TRUE(buff_.data() == data);
  ASSERT_EQ(uint32_t(4097), buff_.capacity());
  ASSERT_EQ(uint32_t(3), buff_.size());
  ASSERT_EQ(uint32_t(1), buff_.available());
  ASSERT_EQ(uint32_t(0), buff_.remaining());

  data = buff_.data();
  success = buff_.resize(2);

  ASSERT_EQ(true, success);
  ASSERT_TRUE(buff_.data() != nullptr);
  ASSERT_TRUE(buff_.data() == data);
  ASSERT_EQ(uint32_t(4097), buff_.capacity());
  ASSERT_EQ(uint32_t(2), buff_.size());
  ASSERT_EQ(uint32_t(0), buff_.available());
  ASSERT_EQ(uint32_t(0), buff_.remaining());
}

TEST_F(BuffTest, extendAdd)
{
  buff_.reserve(5);

  ASSERT_EQ(uint32_t(5), buff_.capacity());
  ASSERT_EQ(uint32_t(1), buff_.size());
  ASSERT_EQ(uint32_t(0), buff_.available());
  ASSERT_EQ(uint32_t(1), buff_.remaining());

  // If 2nd parameter is false (default is true), extend() would adds bytes to the existing size
  // (not capacity); here extends requests "additional" bytes.
  buff_.extend(3, false, false);

  ASSERT_EQ(uint32_t(0), buff_.available());
  ASSERT_EQ(uint32_t(4), buff_.remaining());
  ASSERT_EQ(uint32_t(4), buff_.size());
  ASSERT_EQ(uint32_t(5), buff_.capacity());

  bool success = buff_.extend(10000, false, false);
  ASSERT_EQ(false, success);

  success = buff_.extend(10000, false, true);
  ASSERT_EQ(true, success);
  ASSERT_EQ(uint32_t(0), buff_.available());
  // The size was 4 prior
  ASSERT_EQ(uint32_t(10000 + 4), buff_.size());
  ASSERT_EQ(uint32_t(10000 + 4), buff_.remaining());
  ASSERT_EQ(uint32_t(10000 + 4), buff_.capacity());
}

TEST_F(BuffTest, extendMinimal)
{
  buff_.reserve(4);

  ASSERT_EQ(uint32_t(4), buff_.capacity());
  ASSERT_EQ(uint32_t(1), buff_.size());
  ASSERT_EQ(uint32_t(0), buff_.available());
  ASSERT_EQ(uint32_t(1), buff_.remaining());

  // Minimal extend (true param) considers remaining() bytes and adds missing bytes to the
  // existing size (not capacity); here extends requests "total" bytes
  buff_.extend(3, true, false);

  ASSERT_EQ(uint32_t(0), buff_.available());
  ASSERT_EQ(uint32_t(3), buff_.remaining());
  ASSERT_EQ(uint32_t(3), buff_.size());

  bool success = buff_.extend(10000, true, false);
  ASSERT_EQ(false, success);

  success = buff_.extend(10000, true, true);
  ASSERT_EQ(true, success);
  ASSERT_EQ(uint32_t(0), buff_.available());
  // The size was 4 prior, but with minimal extend we requested total (not additional) bytes
  ASSERT_EQ(uint32_t(10000), buff_.size());
  ASSERT_EQ(uint32_t(10000), buff_.remaining());
  ASSERT_EQ(uint32_t(10000), buff_.capacity());
}

TEST_F(BuffTest, readWriteSingle)
{
  buff_.reserve(4);

  ASSERT_EQ(uint32_t(4), buff_.capacity());
  ASSERT_EQ(uint32_t(1), buff_.size());

  // These writes auto-extend size (not capacity)
  buff_.write(uint8_t(1));
  buff_.write(uint8_t(2));
  buff_.write(uint8_t(3));

  ASSERT_EQ(uint32_t(4), buff_.capacity());
  ASSERT_EQ(uint32_t(3), buff_.size());
  ASSERT_EQ(uint32_t(3), buff_.available());
  ASSERT_EQ(uint32_t(0), buff_.remaining());

  uint8_t output[3] = { 0 };

  for (uint8_t i = 0; i < 3; i++) {
    ASSERT_EQ(true, buff_.read(&output[i], true));
  }
  for (uint8_t i = 0; i < 3; i++) {
    ASSERT_EQ((i + 1), output[i]);
  }

  ASSERT_EQ(uint32_t(3), buff_.size());
  ASSERT_EQ(uint32_t(0), buff_.available());
  ASSERT_EQ(uint32_t(0), buff_.remaining());
}

TEST_F(BuffTest, readWrite)
{
  buff_.reserve(7);

  uint8_t input1[] = { 1, 2, 3 };
  bool result = buff_.write(input1, sizeof(input1));
  ASSERT_EQ(true, result);

  uint8_t input2[] = { 4, 5, 6 };
  result = buff_.write(input2, sizeof(input2));
  ASSERT_EQ(true, result);

  ASSERT_EQ(uint32_t(6), buff_.available());
  ASSERT_EQ(uint32_t(0), buff_.remaining());
  ASSERT_EQ(uint32_t(6), buff_.size());
  ASSERT_EQ(uint32_t(7), buff_.capacity());

  uint8_t output[3] = { 0, 0, 0 };
  ASSERT_EQ(true, buff_.read(output, sizeof(output), true));

  for (uint8_t i = 0; i < sizeof(output); i++) {
    ASSERT_EQ(input1[i], output[i]);
  }

  ASSERT_EQ(uint32_t(3), buff_.available());
  ASSERT_EQ(uint32_t(0), buff_.remaining());
  ASSERT_EQ(uint32_t(6), buff_.size());

  ASSERT_EQ(true, buff_.read(output, 3, true));

  for (uint8_t i = 0; i < sizeof(output); i++) {
    ASSERT_EQ(input2[i], output[i]);
  }

  ASSERT_EQ(uint32_t(0), buff_.available());
  ASSERT_EQ(uint32_t(0), buff_.remaining());
  ASSERT_EQ(uint32_t(6), buff_.size());
  ASSERT_EQ(uint32_t(7), buff_.capacity());
}

TEST_F(BuffTest, shiftOnWrite)
{
  buff_.reserve(2);

  const uint8_t chunk[] = { '2', '2' };
  bool success = buff_.write(chunk, 2, false);

  // It wasn't successful because it couldn't extend the size (note ASSERT_NE)
  ASSERT_NE(true, success);
  ASSERT_EQ(uint32_t(1), buff_.remaining());
  ASSERT_EQ(uint32_t(1), buff_.size());
  ASSERT_EQ(uint32_t(2), buff_.capacity());

  // Size: 1 + 2 = 3
  success = buff_.extend(2, false, true);
  ASSERT_EQ(true, success);
  success = buff_.write(chunk, 2, false);
  ASSERT_EQ(true, success);

  ASSERT_EQ(uint32_t(1), buff_.remaining());
  ASSERT_EQ(uint32_t(3), buff_.size());
  ASSERT_EQ(uint32_t(3), buff_.capacity());

  buff_.read_ptr() += 2;

  ASSERT_EQ(uint32_t(2), buff_.consumed());
  ASSERT_EQ(uint32_t(0), buff_.available());
  ASSERT_EQ(uint32_t(1), buff_.remaining());
  ASSERT_EQ(uint32_t(3), buff_.size());
  ASSERT_EQ(uint32_t(3), buff_.capacity());

  // Store the pointer to later verify that the memory wasn't moved to new location
  const uint8_t* data = buff_.data();
  const uint8_t chunk3[] = { '3', '3', '3' };
  // During this call, already consumed data will be shifted (discarded)
  success = buff_.write(chunk3, 3, false, false);

  // No extension or reallocation was required
  ASSERT_EQ(true, success);
  ASSERT_EQ(uint32_t(0), buff_.consumed());
  ASSERT_EQ(uint32_t(3), buff_.available());
  ASSERT_EQ(uint32_t(0), buff_.remaining());
  ASSERT_EQ(uint32_t(3), buff_.size());
  ASSERT_EQ(uint32_t(3), buff_.capacity());
  ASSERT_TRUE(data == buff_.data());

  buff_.read_ptr() += 2;
  ASSERT_EQ(uint32_t(2), buff_.consumed());
  ASSERT_EQ(uint32_t(1), buff_.available());

  data = buff_.data();
  uint8_t chunk4[] = { '4', '4', '4', '4' };
  // During this call, total 5 bytes in the buffer is required (4 new + 1 unconsumed). We
  // request to extend minimally (account for consumed 2 bytes) and reserve more memory for
  // the total amount.
  success = buff_.write(chunk4, 4, true, true);

  ASSERT_EQ(true, success);
  ASSERT_EQ(uint32_t(0), buff_.consumed());
  ASSERT_EQ(uint32_t(5), buff_.available()); // '3' (1 item) + '4' (4 items)
  ASSERT_EQ(uint32_t(0), buff_.remaining());
  ASSERT_EQ(uint32_t(5), buff_.size());
  ASSERT_EQ(uint32_t(5), buff_.capacity());
  ASSERT_TRUE(data == buff_.data());
}

} // namespace btr
