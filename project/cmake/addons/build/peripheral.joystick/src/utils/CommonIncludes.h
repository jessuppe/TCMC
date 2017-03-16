/*
 *      Copyright (C) 2016 Garrett Brown
 *      Copyright (C) 2016 Team XBMC
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
 *  along with this Program; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */
#pragma once

// This must be #defined before libXBMC_addon.h to fix compile
#if !defined(_WIN32) && !defined(TARGET_DARWIN)
  #include <sys/stat.h>
  #define __stat64 stat64
#endif

#if defined(TARGET_DARWIN)
#include "posix/os-types.h"
#endif

#include "libXBMC_addon.h"
