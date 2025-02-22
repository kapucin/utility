// Copyright (C) 2018 Sergey Kapustin <kapucin@gmail.com>

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// SYSTEM INCLUDES
#include <gtest/gtest.h>

// PROJECT INCLUDES
#include "utility/common/sorters.hpp"

//================================ TEST FIXTURES ===============================

//=================================== TESTS ====================================

namespace btr
{

template<typename T, size_t size>
::testing::AssertionResult ArraysMatch(const T (&expected)[size], const T (&actual)[size])
{
  for (size_t i = 0; i < size; ++i) {
    if (expected[i] != actual[i]) {
      return ::testing::AssertionFailure()
        << "actual[" << i << "] (" << actual[i] << ") != expected["
        << i << "] (" << expected[i] << ")";
    }
  }
  return ::testing::AssertionSuccess();
}

TEST(SortersTest, testInsertionSort)
{
  const uint16_t data_odd_sorted[] = { 2, 4, 5, 7, 16 };
  uint16_t data_odd[] = { 7, 5, 2, 16, 4 };
  Sorters::insertionSort(data_odd, sizeof(data_odd_sorted) / sizeof(uint16_t));
  ASSERT_TRUE(ArraysMatch(data_odd_sorted, data_odd));

  const uint16_t data_even_sorted[] = { 2, 4, 5, 5, 7, 16 };
  uint16_t data_even[] = { 7, 5, 2, 16, 4, 5 };
  Sorters::insertionSort(data_even, sizeof(data_even_sorted) / sizeof(uint16_t));
  ASSERT_TRUE(ArraysMatch(data_even_sorted, data_even));
}

} // namespace btr
