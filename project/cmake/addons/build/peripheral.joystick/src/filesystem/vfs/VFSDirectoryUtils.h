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
#pragma once

#include "filesystem/IDirectoryUtils.h"

namespace ADDON { class CHelper_libXBMC_addon; }

namespace JOYSTICK
{
  class CVFSDirectoryUtils : public IDirectoryUtils
  {
  public:
    CVFSDirectoryUtils(ADDON::CHelper_libXBMC_addon* frontend);

    virtual ~CVFSDirectoryUtils(void) { }

    // implementation of IDirectoryUtils
    virtual bool Create(const std::string& path) override;
    virtual bool Exists(const std::string& path) override;
    virtual bool Remove(const std::string& path) override;
    virtual bool GetDirectory(const std::string& path, const std::string& mask, std::vector<ADDON::CVFSDirEntry>& items) override;

  private:
    ADDON::CHelper_libXBMC_addon* const m_frontend;
  };
}
