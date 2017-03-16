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
 */

#include "PeripheralScanner.h"

#include "libKODI_peripheral.h"

using namespace ADDON;
using namespace JOYSTICK;

CPeripheralScanner::CPeripheralScanner(CHelper_libKODI_peripheral* peripheralLib)
  : m_peripheralLib(peripheralLib)
{
}

void CPeripheralScanner::TriggerScan(void)
{
  if (m_peripheralLib)
    m_peripheralLib->TriggerScan();
}
