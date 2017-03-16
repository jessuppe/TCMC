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

#include "FileUtils.h"
#include "filesystem/vfs/VFSFileUtils.h"
#include "utils/CommonIncludes.h" // for libXBMC_addon.h

using namespace JOYSTICK;

ADDON::CHelper_libXBMC_addon* CFileUtils::m_frontend = NULL;

bool CFileUtils::Initialize(ADDON::CHelper_libXBMC_addon* frontend)
{
  m_frontend = frontend;

  return true;
}

void CFileUtils::Deinitialize(void)
{
  m_frontend = NULL;
}

bool CFileUtils::Exists(const std::string& url)
{
  // Create file utils
  FileUtilsPtr fileUtils = CreateFileUtils(url);
  if (fileUtils)
    return fileUtils->Exists(url);

  return false;
}

bool CFileUtils::Stat(const std::string& url, STAT_STRUCTURE& buffer)
{
  // Create file utils
  FileUtilsPtr fileUtils = CreateFileUtils(url);
  if (fileUtils)
    return fileUtils->Stat(url, buffer);

  return false;
}

bool CFileUtils::Rename(const std::string& url, const std::string& newUrl)
{
  // Create file utils
  FileUtilsPtr fileUtils = CreateFileUtils(url);
  if (fileUtils)
    return fileUtils->Rename(url, newUrl);

  return false;
}

bool CFileUtils::Delete(const std::string& url)
{
  // Create file utils
  FileUtilsPtr fileUtils = CreateFileUtils(url);
  if (fileUtils)
    return fileUtils->Delete(url);

  return false;
}

bool CFileUtils::SetHidden(const std::string& url, bool bHidden)
{
  // Create file utils
  FileUtilsPtr fileUtils = CreateFileUtils(url);
  if (fileUtils)
    return fileUtils->SetHidden(url, bHidden);

  return false;
}

FileUtilsPtr CFileUtils::CreateFileUtils(const std::string& url)
{
  FileUtilsPtr fileUtils;

  if (m_frontend)
    fileUtils = FileUtilsPtr(new CVFSFileUtils(m_frontend));

  return fileUtils;
}
