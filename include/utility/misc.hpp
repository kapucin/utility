/* Copyright (C) 2018 Bolt Robotics <info@boltrobotics.com>
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

#ifndef _btr_Misc_hpp_
#define _btr_Misc_hpp_

// SYSTEM INCLUDES
#include <inttypes.h>

// PROJECT INCLUDES

namespace btr
{

/**
 * Implement miscellaneous functions.
 */
class Misc
{
public:

// ATTRIBUTES

  constexpr static const double PI        = 3.14159;
  constexpr static const double PI_HALF   = PI / 2;
  constexpr static const double PI_TWO    = PI * 2;

// LIFECYCLE

  /**
   * This class uses only static operations.
   */
  Misc() = delete;
  ~Misc() = delete;

// OPERATIONS

  /**
   * Translate a value from one range to the value in another.
   *
   * @param value - the position to translate
   * @param left_min - minimum value of input range
   * @param left_max - maximum value of input range
   * @param right_min - minimum value of output range
   * @param right_max - maximum value of output range
   * @return the translated value
   */
  static double translate(
    double value, double left_min, double left_max, double right_min, double right_max);

  /**
   * Get a sign of the provided value.
   *
   * @param val - the value to get the sign of
   * @return - 1 if the value is positive, -1 otherwise
   */
  static int8_t sign(int16_t val);

  /**
   * Convert angle in degrees to radians.
   *
   * @param degrees - the angle in degrees
   */
  static double toRadians(uint8_t degrees);

  /**
   * Convert angle in radians to degrees.
   *
   * @param radians - the angle in radians
   */
  static double toDegrees(double radians);

  /**
   * Calculate angle delta squared.
   *
   * @param angle1
   * @param angle2
   */
  static double delta(double angle1, double angle2);

  /**
   * Modulo operator to handle negative numbers. The % operator in C is not the
   * modulo but the remainder operator.
   *
   * @param a - left parameter
   * @param b - right parameter
   */
  static int16_t modulo(int16_t a, int16_t b);

  /**
   * Represent the content of buffer in hex.
   *
   * @param data - raw data
   * @param size - the size of raw data
   * @param dst_str - the destination buffer
   * @param dst_size - the size of dst_str
   * @return 0 if converted ok, -1 if strlen(dst_str) is less than size * 3
   */
  static int toHex(const uint8_t* data, uint32_t size, char* dst_str, uint32_t dst_size);

  /**
   * Shift the decimal point in a floating-point number by the specified number of places
   * to the right.
   *
   * @param input - input variable
   * @param decimal_places - the number of places to shift by
   * @return the resulting integer
   */
  static void shiftfint(double input, uint16_t* output, uint8_t decimal_places);

  /**
   * Break input value into integer and fractional parts. Multiply the fractional
   * part by the supplied number. After, cast the resulting values into target types.
   *
   * @param input - input value
   * @param ipart - integer part of the result
   * @param fpart - fractional part of the result
   * @param decimal_places - the number of decimal places in fractional part
   */
  static void modfint(double input, uint8_t* ipart, uint8_t* fpart, uint8_t decimal_places);
  static void modfint(double input, uint16_t* ipart, uint16_t* fpart, uint8_t decimal_places);

private:

// OPERATIONS

  /**
   * Internal implementation.
   */
  template<typename T>
  static T translateImpl(
    double value, double left_min, double left_max, double right_min, double right_max);

  template <typename T>
  static int8_t signImpl(T val);

  template<typename T, typename U>
  static T moduloImpl(T a, U b);

  template<typename InType, typename OutType>
  static void shiftfintImpl(InType input, OutType* output, uint8_t decimal_places);

  template<typename InType, typename OutType>
  static void modfintImpl(InType input, OutType* ipart, OutType* fpart, uint8_t decimal_places);

}; // class Misc

} // namespace btr

#endif  // _btr_Misc_hpp_
