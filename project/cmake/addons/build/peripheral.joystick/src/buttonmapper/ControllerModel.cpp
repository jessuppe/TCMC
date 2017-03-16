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
 *
 */

#include "ControllerModel.h"

using namespace JOYSTICK;

void CControllerModel::AddFeatureMapping(const ControllerMapItem& controllerMapping, const FeatureMapItem& featureMapping)
{
  ++m_map[controllerMapping][featureMapping];

  // Reset the reduced model for the controller mapping
  m_reducedMap[controllerMapping].clear();
}

const FeatureOccurrences& CControllerModel::GetNormalizedFeatures(const ControllerMapItem& needle)
{
  const bool bSwap = (needle.fromController >= needle.toController);

  FeatureOccurrences& result = m_reducedMap[needle];
  if (result.empty())
    NormalizeFeatures(m_map[needle], result, bSwap);

  return result;
}

void CControllerModel::NormalizeFeatures(const FeatureOccurrences& features, FeatureOccurrences& result, bool bSwap)
{
  // First pass, calculate max occurrence counts
  std::map<std::string, unsigned int> maxFeatureCounts;
  for (auto it = features.begin(); it != features.end(); ++it)
  {
    const std::string& fromFeature = it->first.fromFeature;
    const std::string& toFeature = it->first.toFeature;
    const std::string& lookupFeature = bSwap ? toFeature : fromFeature;
    const unsigned int count = it->second;

    unsigned int& maxCount = maxFeatureCounts[lookupFeature];
    if (count > maxCount)
      maxCount = count;
  }

  // Second pass, assign features with max occurrences to result
  for (auto it = features.begin(); it != features.end(); ++it)
  {
    const std::string& fromFeature = it->first.fromFeature;
    const std::string& toFeature = it->first.toFeature;
    const std::string& lookupFeature = bSwap ? toFeature : fromFeature;
    const unsigned int count = it->second;

    unsigned int& targetCount = maxFeatureCounts[lookupFeature];
    if (targetCount == count)
    {
      FeatureMapItem featurePair = { fromFeature, toFeature };
      result[featurePair] = 1;
      targetCount = 0;
    }
  }
}
