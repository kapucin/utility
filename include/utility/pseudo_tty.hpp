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

#ifndef _btr_PseudoTTY_hpp__
#define _btr_PseudoTTY_hpp__

// SYSTEM INCLUDES

using namespace std::chrono_literals;

namespace btr
{

#define PRG "socat"
#define TTY_SIM_0 "/tmp/ttySIM0"
#define TTY_SIM_1 "/tmp/ttySIM1"
#define PTY0 "PTY,link=" TTY_SIM_0 ",raw,echo=0"
#define PTY1 "PTY,link=" TTY_SIM_1 ",raw,echo=0"

/**
 * The class opens send and receive interfaces to a serial port upon constructing an instance.
 * The serial port is to close when the instance gets destroyed.
 */
class PseudoTTY
{
public:

// LIFECYCLE

  /**
   * Forks a child process.
   */
  PseudoTTY();

  /**
   * Terminates child process and reaps its PID
   */
  ~PseudoTTY();

private:

// ATTRIBUTES

  pid_t child_pid_;
};

} // namespace btr

#endif // _btr_PseudoTTY_hpp__
