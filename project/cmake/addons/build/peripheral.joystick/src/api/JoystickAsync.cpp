/*
 *      Copyright (C) 2014-2015 Garrett Brown
 *      Copyright (C) 2014-2015 Team XBMC
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
 */

#include "JoystickAsync.h"

using namespace JOYSTICK;
using namespace P8PLATFORM;

CJoystickAsync::CJoystickAsync(const std::string& strProvider)
  : CJoystick(strProvider)
{
}

bool CJoystickAsync::GetEvents(std::vector<ADDON::PeripheralEvent>& events)
{
  CLockObject lock(m_mutex);
  return CJoystick::GetEvents(events);
}

void CJoystickAsync::SetButtonValue(unsigned int buttonIndex, JOYSTICK_STATE_BUTTON buttonValue)
{
  CLockObject lock(m_mutex);
  CJoystick::SetButtonValue(buttonIndex, buttonValue);
}

void CJoystickAsync::SetHatValue(unsigned int hatIndex, JOYSTICK_STATE_HAT hatValue)
{
  CLockObject lock(m_mutex);
  CJoystick::SetHatValue(hatIndex, hatValue);
}

void CJoystickAsync::SetAxisValue(unsigned int axisIndex, JOYSTICK_STATE_AXIS axisValue)
{
  CLockObject lock(m_mutex);
  CJoystick::SetAxisValue(axisIndex, axisValue);
}
