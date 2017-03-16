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

#include "VFSDirectoryUtils.h"
#include "utils/CommonIncludes.h" // for libXBMC_addon.h

#include <assert.h>

using namespace JOYSTICK;

CVFSDirectoryUtils::CVFSDirectoryUtils(ADDON::CHelper_libXBMC_addon* frontend)
  : m_frontend(frontend)
{
  assert(m_frontend);
}

bool CVFSDirectoryUtils::Create(const std::string& path)
{
  return m_frontend->CreateDirectory(path.c_str());
}

bool CVFSDirectoryUtils::Exists(const std::string& path)
{
  return m_frontend->DirectoryExists(path.c_str());
}

bool CVFSDirectoryUtils::Remove(const std::string& path)
{
  return m_frontend->RemoveDirectory(path.c_str());
}

bool CVFSDirectoryUtils::GetDirectory(const std::string& path, const std::string& mask, std::vector<ADDON::CVFSDirEntry>& items)
{
  return ADDON::VFSUtils::GetDirectory(m_frontend, path, mask, items);
}
