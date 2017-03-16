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

#include "ButtonMapper.h"
#include "ControllerTransformer.h"
#include "storage/IDatabase.h"

#include "kodi_peripheral_utils.hpp"
#include "libKODI_peripheral.h"

#include <algorithm>
#include <iterator>

using namespace JOYSTICK;

CButtonMapper::CButtonMapper(ADDON::CHelper_libKODI_peripheral* peripheralLib) :
  m_peripheralLib(peripheralLib)
{
}

CButtonMapper::~CButtonMapper()
{
}

bool CButtonMapper::Initialize(CJoystickFamilyManager& familyManager)
{
  m_controllerTransformer.reset(new CControllerTransformer(familyManager));
  return true;
}

void CButtonMapper::Deinitialize()
{
  m_controllerTransformer.reset();
  m_databases.clear();
}

IDatabaseCallbacks* CButtonMapper::GetCallbacks()
{
  return m_controllerTransformer.get();
}

bool CButtonMapper::GetFeatures(const ADDON::Joystick& joystick,
                                const std::string& strControllerId,
                                FeatureVector& features)
{
  // Accumulate available button maps for this device
  ButtonMap accumulatedMap = GetButtonMap(joystick);

  GetFeatures(joystick, std::move(accumulatedMap), strControllerId, features);

  return !features.empty();
}

ButtonMap CButtonMapper::GetButtonMap(const ADDON::Joystick& joystick) const
{
  ButtonMap accumulatedMap;

  for (DatabaseVector::const_iterator it = m_databases.begin(); it != m_databases.end(); ++it)
  {
    const ButtonMap& buttonMap = (*it)->GetButtonMap(joystick);
    MergeButtonMap(accumulatedMap, buttonMap);
  }

  return accumulatedMap;
}

void CButtonMapper::MergeButtonMap(ButtonMap& knownMap, const ButtonMap& newFeatures)
{
  for (auto it = newFeatures.begin(); it != newFeatures.end(); ++it)
  {
    const std::string& controllerId = it->first;
    const FeatureVector& features = it->second;

    MergeFeatures(knownMap[controllerId], features);
  }
}

void CButtonMapper::MergeFeatures(FeatureVector& features, const FeatureVector& newFeatures)
{
  for (const ADDON::JoystickFeature& newFeature : newFeatures)
  {
    // Check for duplicate feature name
    bool bFound = std::find_if(features.begin(), features.end(),
      [&newFeature](const ADDON::JoystickFeature& feature)
      {
        return feature.Name() == newFeature.Name();
      }) != features.end();

    // Check for duplicate driver primitives
    if (!bFound)
    {
      const auto& newPrimitives = newFeature.Primitives();

      bFound = std::find_if(features.begin(), features.end(),
        [&newPrimitives](const ADDON::JoystickFeature& feature)
        {
          for (const auto& primitive : feature.Primitives())
          {
            if (primitive.Type() == JOYSTICK_DRIVER_PRIMITIVE_TYPE_UNKNOWN)
              continue;

            if (std::find(newPrimitives.begin(), newPrimitives.end(), primitive) != newPrimitives.end())
              return true; // Found primitive
          }
          return false; // Didn't find primitive
        }) != features.end();
    }

    if (!bFound)
      features.push_back(newFeature);
  }
}

bool CButtonMapper::GetFeatures(const ADDON::Joystick& joystick, ButtonMap&& buttonMap, const std::string& controllerId, FeatureVector& features)
{
  // Try to get a button map for the specified controller profile
  auto itController = buttonMap.find(controllerId);
  if (itController != buttonMap.end())
    features.swap(itController->second);

  bool bNeedsFeatures = false;

  if (features.empty())
    bNeedsFeatures = true;

  if (m_peripheralLib)
  {
    unsigned int featureCount = m_peripheralLib->FeatureCount(controllerId);
    if (featureCount > 0)
      bNeedsFeatures = (features.size() < featureCount);
  }

  // Try to derive a button map from relations between controller profiles
  if (bNeedsFeatures)
  {
    FeatureVector derivedFeatures;
    DeriveFeatures(joystick, controllerId, buttonMap, derivedFeatures);
    MergeFeatures(features, derivedFeatures);
  }

  return !features.empty();
}

void CButtonMapper::DeriveFeatures(const ADDON::Joystick& joystick, const std::string& toController, const ButtonMap& buttonMap, FeatureVector& transformedFeatures)
{
  if (!m_controllerTransformer)
    return;

  // Search the button map for the controller with the highest count of features defined
  unsigned int maxFeatures = 0;
  auto maxFeaturesIt = buttonMap.end();

  for (auto it = buttonMap.begin(); it != buttonMap.end(); ++it)
  {
    const unsigned int featureCount = it->second.size();
    if (featureCount > maxFeatures)
    {
      maxFeatures = featureCount;
      maxFeaturesIt = it;
    }
  }

  if (maxFeaturesIt != buttonMap.end())
  {
    // Transform the controller profile with the most features to the specified controller
    const std::string& fromController = maxFeaturesIt->first;
    const FeatureVector& features = maxFeaturesIt->second;

    m_controllerTransformer->TransformFeatures(joystick, fromController, toController, features, transformedFeatures);
  }
}

void CButtonMapper::RegisterDatabase(const DatabasePtr& database)
{
  if (std::find(m_databases.begin(), m_databases.end(), database) == m_databases.end())
    m_databases.push_back(database);
}

void CButtonMapper::UnregisterDatabase(const DatabasePtr& database)
{
  m_databases.erase(std::remove(m_databases.begin(), m_databases.end(), database), m_databases.end());
}
