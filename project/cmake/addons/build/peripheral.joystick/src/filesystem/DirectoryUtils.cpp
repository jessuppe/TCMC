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

#include "DirectoryUtils.h"
#include "filesystem/vfs/VFSDirectoryUtils.h"
#include "utils/CommonIncludes.h" // for libXBMC_addon.h

using namespace JOYSTICK;

ADDON::CHelper_libXBMC_addon* CDirectoryUtils::m_frontend = NULL;

bool CDirectoryUtils::Initialize(ADDON::CHelper_libXBMC_addon* frontend)
{
  m_frontend = frontend;

  return true;
}

void CDirectoryUtils::Deinitialize(void)
{
  m_frontend = NULL;
}

// --- Directory operations ---------------------------------------------------------

bool CDirectoryUtils::Create(const std::string& path)
{
  // Create directory utils
  DirectoryUtilsPtr dirUtils = CreateDirectoryUtils(path);
  if (dirUtils)
    return dirUtils->Create(path);

  return false;
}

bool CDirectoryUtils::Exists(const std::string& path)
{
  // Create directory utils
  DirectoryUtilsPtr dirUtils = CreateDirectoryUtils(path);
  if (dirUtils)
    return dirUtils->Exists(path);

  return false;
}

bool CDirectoryUtils::Remove(const std::string& path)
{
  // Create directory utils
  DirectoryUtilsPtr dirUtils = CreateDirectoryUtils(path);
  if (dirUtils)
    return dirUtils->Remove(path);

  return false;
}

bool CDirectoryUtils::GetDirectory(const std::string& path, const std::string& mask, std::vector<ADDON::CVFSDirEntry>& items)
{
  // Create directory utils
  DirectoryUtilsPtr dirUtils = CreateDirectoryUtils(path);
  if (dirUtils)
    return dirUtils->GetDirectory(path, mask, items);

  return false;
}

DirectoryUtilsPtr CDirectoryUtils::CreateDirectoryUtils(const std::string& url)
{
  DirectoryUtilsPtr dirUtils;

  if (m_frontend)
    dirUtils = DirectoryUtilsPtr(new CVFSDirectoryUtils(m_frontend));

  return dirUtils;
}
