/*
 *      Copyright (C) 2016 Garrett Brown
 *      Copyright (C) 2016 Team Kodi
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
 */
#pragma once

#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace ADDON
{
  class JoystickFeature;
};

namespace JOYSTICK
{
  typedef std::string                            ControllerID;
  typedef std::string                            FeatureName;

  typedef std::vector<ADDON::JoystickFeature>    FeatureVector;
  typedef std::map<ControllerID, FeatureVector>  ButtonMap;

  struct FeatureMapItem
  {
    std::string fromFeature;
    std::string toFeature;

    bool operator<(const FeatureMapItem& other) const
    {
      if (fromFeature < other.fromFeature) return true;
      if (fromFeature > other.fromFeature) return false;

      if (toFeature < other.toFeature) return true;
      if (toFeature > other.toFeature) return false;

      return false;
    }
  };

  typedef std::map<FeatureMapItem, unsigned int> FeatureOccurrences;

  struct ControllerMapItem
  {
    std::string        fromController;
    std::string        toController;

    bool operator<(const ControllerMapItem& other) const
    {
      if (fromController < other.fromController) return true;
      if (fromController > other.fromController) return false;

      if (toController < other.toController) return true;
      if (toController > other.toController) return false;

      return false;
    }
  };

  typedef std::map<ControllerMapItem, FeatureOccurrences> ControllerMap;

  typedef std::string FamilyName;
  typedef std::string JoystickName;

  typedef std::map<FamilyName, std::set<JoystickName>> JoystickFamilyMap;

}
