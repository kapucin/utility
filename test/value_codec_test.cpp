// Copyright (C) 2018 Sergey Kapustin <kapucin@gmail.com>

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// SYSTEM INCLUDES
#include <gtest/gtest.h>
#include <iostream>
#include <limits>

// PROJECT INCLUDES
#include "utility/common/value_codec.hpp"
#include "utility/common/buff.hpp"
#include "utility/common/test_helpers.hpp"

namespace btr
{

//================================ TEST FIXTURES ===============================

/**
 * Test fixture for testing functionality in ValueCodec class.
 */
class ValueCodecTest : public testing::Test
{
public:

  // LIFECYCLE

  ValueCodecTest()
    :
      buff_(32)
  {
  }

  // ATTRIBUTES

  Buff buff_;
};

//=================================== TESTS ====================================

TEST_F(ValueCodecTest, decodeFixedIntMsbOK)
{
  uint8_t data[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

  buff_.write(data);
  int64_t val64 = 0;
  int rc = ValueCodec::decodeFixedInt(&buff_, &val64, sizeof(val64), true);

  ASSERT_EQ(0, rc);
  ASSERT_EQ(int64_t(-1), val64);
  ASSERT_EQ(uint32_t(0), buff_.available());

  buff_.write(data);
  int32_t val32 = 0;
  rc = ValueCodec::decodeFixedInt(&buff_, &val32, sizeof(val32), true);

  ASSERT_EQ(0, rc);
  ASSERT_EQ(int64_t(-1), val32);
  ASSERT_EQ(uint32_t(4), buff_.available());

  uint8_t uval8 = 0;
  rc = ValueCodec::decodeFixedInt(&buff_, &uval8, sizeof(uval8), true);

  ASSERT_EQ(0, rc);
  ASSERT_EQ(uint8_t(255), uval8);
  ASSERT_EQ(uint32_t(3), buff_.available());

  uint16_t uval16 = 0;
  rc = ValueCodec::decodeFixedInt(&buff_, &uval16, sizeof(uval16), true);

  ASSERT_EQ(0, rc);
  ASSERT_EQ(uint16_t(65535), uval16);
  ASSERT_EQ(uint32_t(1), buff_.available());
}

TEST_F(ValueCodecTest, decodeFixedIntLsbOK)
{
  uint8_t data[] = {0xFF, 0xFE, 0xFD, 0xFC, 0xFB, 0xFA, 0xF9, 0xF8};

  buff_.write(data);
  int64_t val64 = 0;
  int rc = ValueCodec::decodeFixedInt(&buff_, &val64, sizeof(int64_t), false);

  ASSERT_EQ(0, rc);
  ASSERT_EQ(int64_t(-506097522914230529), val64);
  ASSERT_EQ(uint32_t(0), buff_.available());

  buff_.write(data);
  int32_t val32 = 0;
  rc = ValueCodec::decodeFixedInt(&buff_, &val32, sizeof(int32_t), false);

  ASSERT_EQ(0, rc);
  ASSERT_EQ(int64_t(-50462977), val32);
  ASSERT_EQ(uint32_t(4), buff_.available());

  uint8_t uval8 = 0;
  rc = ValueCodec::decodeFixedInt(&buff_, &uval8, sizeof(uint8_t), false);

  ASSERT_EQ(0, rc);
  ASSERT_EQ(uint8_t(251), uval8);
  ASSERT_EQ(uint32_t(3), buff_.available());

  uint16_t uval16 = 0;
  rc = ValueCodec::decodeFixedInt(&buff_, &uval16, sizeof(uint16_t), false);

  ASSERT_EQ(0, rc);
  ASSERT_EQ(uint16_t(63994), uval16);
  ASSERT_EQ(uint32_t(1), buff_.available());
}

TEST_F(ValueCodecTest, decodeFixedIntBad)
{
  uint8_t data[] = {0xFF, 0xFE, 0xFD, 0xFC};

  buff_.write(data);
  int64_t val64 = 0;
  int rc = ValueCodec::decodeFixedInt(&buff_, &val64, sizeof(int64_t), false);
  ASSERT_EQ(-1, rc);
  ASSERT_EQ(ERANGE, errno);
  ASSERT_EQ(uint32_t(4), buff_.available());

  int16_t val16 = 0;
  rc = ValueCodec::decodeFixedInt(&buff_, &val16, sizeof(val16) + 1, true);
  ASSERT_EQ(ERANGE, errno);
  ASSERT_EQ(uint32_t(4), buff_.available());
}

TEST_F(ValueCodecTest, littleEndian)
{
  int16_t number = 0x0001;
  char* ptr = (char*) &number;
  bool expected = (ptr[0] == 1);

  ASSERT_EQ(expected, ValueCodec::isLittleEndian());
}

TEST_F(ValueCodecTest, swap)
{
  uint8_t data[] = {0xFF, 0xFE, 0xFD, 0xFC};

  char* p = reinterpret_cast<char*>(data);
  uint32_t val = *reinterpret_cast<uint32_t*>(p);
  ValueCodec::swap(&val);

  uint8_t* data2 = reinterpret_cast<uint8_t*>(&val);

  for (int i = 0; i < 4; i++)  {
    ASSERT_EQ(data[i], data2[3 - i]) << "Index: " << i <<
      ", data2: " << TestHelpers::toHex(data2, sizeof(val));
  }
}

TEST_F(ValueCodecTest, varIntNBits)
{
  // Bits are all 1s
  //
  buff_.write((uint8_t) 0xFF);
  uint8_t v1 = 0;
  ValueCodec::decodeVarIntNBits(&buff_, &v1, ARRAY(0x86), true);
  ASSERT_EQ(0b111111, v1);
  ASSERT_EQ(uint32_t(0), buff_.available());

  buff_.write((uint8_t) 0xFF);
  uint8_t v2 = 0;
  ValueCodec::decodeVarIntNBits(&buff_, &v2, ARRAY(0x2), true);
  ASSERT_EQ(uint8_t(0b11), v2);
  ASSERT_EQ(uint32_t(0), buff_.available());

  buff_.write(ARRAY(0x1F, 0xFF));
  uint16_t v3 = 0;
  ValueCodec::decodeVarIntNBits(&buff_, &v3, ARRAY(0x05, 0x88), true);
  ASSERT_EQ(uint16_t(0b1111111111111), v3);
  ASSERT_EQ(uint32_t(0), buff_.available());

  // Mixed bits
  //
  buff_.write((uint8_t) 0xAC);
  ValueCodec::decodeVarIntNBits(&buff_, &v1, ARRAY(0x86), true);
  ASSERT_EQ(uint8_t(0b101011), v1);
  ASSERT_EQ(uint32_t(0), buff_.available());

  buff_.write((uint8_t) 0xAE);
  ValueCodec::decodeVarIntNBits(&buff_, &v2, ARRAY(0x2), true);
  ASSERT_EQ(uint8_t(0b10), v2);
  ASSERT_EQ(uint32_t(0), buff_.available());

  buff_.write(ARRAY(0x17, 0x19));
  ValueCodec::decodeVarIntNBits(&buff_, &v3, ARRAY(0x05, 0x88), true);
  ASSERT_EQ(uint16_t(0b1011100011001), v3);
  ASSERT_EQ(uint32_t(0), buff_.available());

  // Second by has high bit clear. Result should be identical to the previous
  // because all 8 bits are requested.
  //
  buff_.write(ARRAY(0x17, 0x19));
  ValueCodec::decodeVarIntNBits(&buff_, &v3, ARRAY(0x05, 0x08), true);
  ASSERT_EQ(uint16_t(0b1011100011001), v3);
  ASSERT_EQ(uint32_t(0), buff_.available());
}

TEST_F(ValueCodecTest, varInt7Bits)
{
  buff_.write(ARRAY(0x0F));
  uint8_t v1 = 0;
  int rc = ValueCodec::decodeVarInt7Bits(&buff_, &v1);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(uint8_t(0xF), v1);
  ASSERT_EQ(uint32_t(0), buff_.available());

  buff_.write(ARRAY(0x8F, 0x7F));
  uint16_t v2 = 0;
  rc = ValueCodec::decodeVarInt7Bits(&buff_, &v2);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(uint16_t(0x7FF), v2);
  ASSERT_EQ(uint32_t(0), buff_.available());

  buff_.write(ARRAY(0x82, 0x88, 0x1, 0x3));
  uint16_t v3 = 0;
  rc = ValueCodec::decodeVarInt7Bits(&buff_, &v3);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(uint16_t(0x421), v3);
  ASSERT_EQ(uint32_t(1), buff_.available());
}

TEST_F(ValueCodecTest, varInt7BitsBad)
{
  // Buffer is small and high bit is set.
  buff_.write(ARRAY(0x82));
  uint16_t v1 = 0;
  int rc = ValueCodec::decodeVarInt7Bits(&buff_, &v1);
  ASSERT_EQ(-1, rc);
  ASSERT_EQ(ERANGE, errno);
  ASSERT_EQ(uint32_t(0), buff_.available());

  // High bit is not set.
  buff_.write(ARRAY(0x82, 0x88, 0x81, 0x3));
  uint16_t v2 = 0;
  rc = ValueCodec::decodeVarInt7Bits(&buff_, &v2);
  ASSERT_EQ(-1, rc);
  ASSERT_EQ(ERANGE, errno);
  ASSERT_EQ(uint32_t(1), buff_.available());
}

TEST_F(ValueCodecTest, encodeFixedIntPositive)
{
  const uint16_t num = 0x0402;
  uint16_t val = 0;

  // If this host is little-endian, the number is encoded in lSB, 0x0204
  buff_.write(num);
  // Decode in MSB
  int rc = ValueCodec::decodeFixedInt(&buff_, &val, sizeof(val), true);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(uint16_t(0x0204), val);

  // If this host is little-endian, the number is encoded in LSB, 0x0204
  buff_.write(num);
  // Decode in LSB
  rc = ValueCodec::decodeFixedInt(&buff_, &val, sizeof(val), false);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(num, val);

  // If this host is little-endian, the number is encoded in MSB, 0x0402
  ValueCodec::encodeFixedInt(&buff_, num, true);
  // Decode in MSB
  rc = ValueCodec::decodeFixedInt(&buff_, &val, sizeof(val), true);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(num, val);

  // If this host is little-endian, the number is encoded in LSB, 0x0204
  ValueCodec::encodeFixedInt(&buff_, num, false);
  // Decode in MSB
  rc = ValueCodec::decodeFixedInt(&buff_, &val, sizeof(val), true);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(uint16_t(0x0204), val);

  // If this host is little-endian, the number is encoded in LSB, 0x0204
  ValueCodec::encodeFixedInt(&buff_, num, false);
  // Decode in MSB
  rc = ValueCodec::decodeFixedInt(&buff_, &val, sizeof(val), false);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(num, val);
}

TEST_F(ValueCodecTest, encodeFixedIntRawBuff)
{
  const uint16_t num = 0x0402;
  uint16_t val = 0;
  uint8_t* buffc = buff_.data();

  // If this host is little-endian, the number is encoded in MSB, 0x0402
  ValueCodec::encodeFixedInt(buffc, num, true);
  // Decode in MSB
  ValueCodec::decodeFixedInt(buffc, &val, sizeof(val), true);
  ASSERT_EQ(num, val);

  // If this host is little-endian, the number is encoded in LSB, 0x0204
  ValueCodec::encodeFixedInt(buffc, num, false);
  // Decode in MSB
  ValueCodec::decodeFixedInt(buffc, &val, sizeof(val), true);
  ASSERT_EQ(uint16_t(0x0204), val);

  // If this host is little-endian, the number is encoded in LSB, 0x0204
  ValueCodec::encodeFixedInt(buffc, num, false);
  // Decode in MSB
  ValueCodec::decodeFixedInt(buffc, &val, sizeof(val), false);
  ASSERT_EQ(num, val);

  const int16_t num_neg = -0x0402;
  int16_t val_neg = 0;

  // If this host is little-endian, the number is encoded in MSB, -0x0402
  ValueCodec::encodeFixedInt(buffc, num_neg, true);
  ValueCodec::decodeFixedInt(buffc, &val_neg, sizeof(val_neg), true);
  ASSERT_EQ(num_neg, val_neg);

  // If this host is little-endian, the number is encoded in LSB, -0x0204
  ValueCodec::encodeFixedInt(buffc, num_neg, false);
  // Decode in MSB
  ValueCodec::decodeFixedInt(buffc, &val_neg, sizeof(val_neg), true);
  ASSERT_EQ(int16_t(-0x0105), val_neg);

  // If this host is little-endian, the number is encoded in LSB, -0x0204
  ValueCodec::encodeFixedInt(buffc, num_neg, false);
  // Decode in MSB
  ValueCodec::decodeFixedInt(buffc, &val_neg, sizeof(val_neg), false);
  ASSERT_EQ(num_neg, val_neg);
}

TEST_F(ValueCodecTest, encodeFixedIntNegative)
{
  const int16_t num = -0x0402;
  int16_t val = 0;

  // If this host is little-endian, the number is encoded in lSB, -0x0204
  buff_.write(num);
  // Decode in MSB
  int rc = ValueCodec::decodeFixedInt(&buff_, &val, sizeof(val), true);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(int16_t(-0x0105), val);

  // If this host is little-endian, the number is encoded in LSB, -0x0204
  buff_.write(num);
  // Decode in LSB
  rc = ValueCodec::decodeFixedInt(&buff_, &val, sizeof(val), false);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(num, val);

  // If this host is little-endian, the number is encoded in MSB, -0x0402
  ValueCodec::encodeFixedInt(&buff_, num, true);
  // Decode in MSB
  rc = ValueCodec::decodeFixedInt(&buff_, &val, sizeof(val), true);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(num, val);

  // If this host is little-endian, the number is encoded in LSB, -0x0204
  ValueCodec::encodeFixedInt(&buff_, num, false);
  // Decode in MSB
  rc = ValueCodec::decodeFixedInt(&buff_, &val, sizeof(val), true);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(int16_t(-0x0105), val);

  // If this host is little-endian, the number is encoded in LSB, -0x0204
  ValueCodec::encodeFixedInt(&buff_, num, false);
  // Decode in MSB
  rc = ValueCodec::decodeFixedInt(&buff_, &val, sizeof(val), false);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(num, val);
}

TEST_F(ValueCodecTest, encodeFloatToInt)
{
  double v = Misc::PI;

  ValueCodec::encodeFloatToInt(&buff_, sizeof(uint16_t), v, 2, true);
  uint8_t expected[] = { 0x1, 0x3A };

  for (uint8_t i = 0; i < 2; i++) {
    ASSERT_EQ(expected[i], buff_.data()[i]);
  }

  buff_.reset();
  v = 2.1;
  uint8_t* buff = buff_.data();
  ValueCodec::encodeFloatToInt(buff, sizeof(uint16_t), v, 3, true);
  uint8_t expected2[] = { 0x8, 0x34 };

  for (uint8_t i = 0; i < 2; i++) {
    ASSERT_EQ(expected2[i], buff[i]) << TestHelpers::toHex(buff_);
  }
}

TEST_F(ValueCodecTest, decodeIntToFloat)
{
  double v = 0;

  uint8_t raw[] = { 0x1, 0x3A }; // 314
  ValueCodec::decodeIntToFloat<int16_t>(raw, 2, &v, 2, true);
  ASSERT_EQ(3.14, v);

  uint8_t raw2[] = { 0x8, 0x34 }; // 2100
  ValueCodec::decodeIntToFloat<int16_t>(raw2, 2, &v, 3, true);
  ASSERT_EQ(2.1, v);

  uint8_t raw3[] = { 0x80, 0x00 };
  ValueCodec::decodeIntToFloat<int16_t>(raw3, 2, &v, 5, true);
  ASSERT_EQ(-0.32768, v);
}

template<typename OutType>
void decodeIntPartsToFloat(Buff* buff, OutType ipart_expected, OutType fpart_expected, int line)
{
  // Decode integer part
  OutType ipart_actual = 0;
  int rc = ValueCodec::decodeFixedInt(buff, &ipart_actual, sizeof(OutType), true);
  ASSERT_EQ(0, rc) << "Line: " << line;
  ASSERT_EQ(ipart_expected, ipart_actual) << "Line: " << line;

  // Decode fractional part
  OutType fpart_actual = 0;
  rc = ValueCodec::decodeFixedInt(buff, &fpart_actual, sizeof(OutType), true);
  ASSERT_EQ(0, rc) << "Line: " << line;
  ASSERT_EQ(fpart_expected, fpart_actual) << "Line: " << line;
}

TEST_F(ValueCodecTest, encodeFloatToIntParts)
{
  double val = Misc::PI;

  ValueCodec::encodeFloatToIntParts(&buff_, sizeof(uint8_t), val, 2, true);
  decodeIntPartsToFloat<uint8_t>(&buff_, 3, 14, __LINE__);

  ValueCodec::encodeFloatToIntParts(&buff_, sizeof(uint8_t), val, 1, true);
  decodeIntPartsToFloat<uint8_t>(&buff_, 3, 1, __LINE__);

  ValueCodec::encodeFloatToIntParts(&buff_, sizeof(uint8_t), val, 0, true);
  decodeIntPartsToFloat<uint8_t>(&buff_, 3, 0, __LINE__);

  val = 65535.65536;
  ValueCodec::encodeFloatToIntParts(&buff_, sizeof(uint8_t), val, 5, true);
  decodeIntPartsToFloat<uint8_t>(&buff_, 255, 0, __LINE__);

#if 0
  // Look for: "why-does-converting-an-large-double-to-a-uint16-t-with-optimizations-on-gcc-and"
  // You'll get 0 with clang, and 65535 on gcc when optimizations are on:
  //std::cout << (static_cast<uint16_t>(65537.0)) << std::endl;

  val = 65536.65535;
  ValueCodec::encodeFloatToIntParts(&buff_, sizeof(uint16_t), val, 5, true);
  decodeIntPartsToFloat<uint16_t>(&buff_, 0, 65535, __LINE__);
#endif
}

} // namespace btr
