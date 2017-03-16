/*
 *      Copyright (C) 2015 Garrett Brown
 *      Copyright (C) 2015 Team XBMC
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "JoystickTranslator.h"

using namespace JOYSTICK;

JOYSTICK_DRIVER_HAT_DIRECTION JoystickTranslator::TranslateHatDir(const std::string& hatDir)
{
  if (hatDir == "up")    return JOYSTICK_DRIVER_HAT_UP;
  if (hatDir == "down")  return JOYSTICK_DRIVER_HAT_DOWN;
  if (hatDir == "right") return JOYSTICK_DRIVER_HAT_RIGHT;
  if (hatDir == "left")  return JOYSTICK_DRIVER_HAT_LEFT;

  return JOYSTICK_DRIVER_HAT_UNKNOWN;
}

const char* JoystickTranslator::TranslateHatDir(JOYSTICK_DRIVER_HAT_DIRECTION hatDir)
{
  switch (hatDir)
  {
    case JOYSTICK_DRIVER_HAT_UP:    return "up";
    case JOYSTICK_DRIVER_HAT_DOWN:  return "down";
    case JOYSTICK_DRIVER_HAT_RIGHT: return "right";
    case JOYSTICK_DRIVER_HAT_LEFT:  return "left";
    default:
      break;
  }
  return "";
}

JOYSTICK_DRIVER_SEMIAXIS_DIRECTION JoystickTranslator::TranslateSemiAxisDir(char axisSign)
{
  switch (axisSign)
  {
  case '+': return JOYSTICK_DRIVER_SEMIAXIS_POSITIVE;
  case '-': return JOYSTICK_DRIVER_SEMIAXIS_NEGATIVE;
  default:
    break;
  }
  return JOYSTICK_DRIVER_SEMIAXIS_UNKNOWN;
}

const char* JoystickTranslator::TranslateSemiAxisDir(JOYSTICK_DRIVER_SEMIAXIS_DIRECTION dir)
{
  switch (dir)
  {
    case JOYSTICK_DRIVER_SEMIAXIS_POSITIVE: return "+";
    case JOYSTICK_DRIVER_SEMIAXIS_NEGATIVE: return "-";
    default:
      break;
  }
  return "";
}
