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

#include "FilesystemTypes.h"
#include "IFile.h"
#include "IFileUtils.h"

#include <string>

namespace ADDON { class CHelper_libXBMC_addon; }

namespace JOYSTICK
{
  class CFileUtils
  {
  public:
    static bool Initialize(ADDON::CHelper_libXBMC_addon* frontend);
    static void Deinitialize(void);

    static bool Exists(const std::string& url);
    static bool Stat(const std::string& url, STAT_STRUCTURE& buffer);
    static bool Rename(const std::string& url, const std::string& newUrl);
    static bool Delete(const std::string& url);
    static bool SetHidden(const std::string& url, bool bHidden);

  private:
    /*!
     * \brief Create a file utilities instance to handle the specified URL
     *
     * \return The file utilities instance, or empty if no file utility
     *         implementations can handle the URL
     */
    static FileUtilsPtr CreateFileUtils(const std::string& url);

    static ADDON::CHelper_libXBMC_addon* m_frontend;
  };
}
