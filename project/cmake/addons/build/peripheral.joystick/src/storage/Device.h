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
 *
 */
#pragma once

#include "kodi_peripheral_utils.hpp"

namespace JOYSTICK
{
  /*!
   * \brief Record for devices in the button map database
   *
   * Device properties are inherited from ADDON::Joystick. These are:
   *
   *   - Device type
   *   - Name                     [ or "" if unknown ]
   *   - USB Vendor ID/Product ID [ or 00:00 if unknown ]
   *   - Driver index
   *   - Provider (driver name)   [ required ]
   *   - Requested Port           [ or -1 for no port requested ]
   *   - Button count
   *   - Hat count
   *   - Axis count
   *   - Motor count
   *
   * This class contains logic for comparing, sorting and combining device
   * records, as well as utility functions.
   *
   * The number of motors is hard-coded in the api section of the code and is
   * not used in the handling logic.
   */
  class CDevice : public ADDON::Joystick
  {
  public:
    CDevice(void) { }
    CDevice(const ADDON::Joystick& joystick);

    /*!
     * \brief Define a comparison operator for driver records
     */
    bool operator==(const CDevice& rhs) const;
    bool operator!=(const CDevice& rhs) const { return !(*this == rhs); }

    /*!
     * \brief Define a total order for driver records
     */
    bool operator<(const CDevice& rhs) const;

    /*!
     * \brief Define a similarity metric for driver records
     */
    bool SimilarTo(const CDevice& rhs) const;

    /*!
     * \brief Define a validity test for driver records
     */
    bool IsValid(void) const;

    /*!
     * \brief Merge valid (known) properties of given record into this record
     */
    void MergeProperties(const CDevice& record);
  };
}
