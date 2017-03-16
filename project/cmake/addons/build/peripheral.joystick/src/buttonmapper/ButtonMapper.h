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
#pragma once

#include "ButtonMapTypes.h"
#include "storage/StorageTypes.h"

#include <memory>
#include <string>

namespace ADDON
{
  class CHelper_libKODI_peripheral;
  class Joystick;
};

namespace JOYSTICK
{
  class CControllerTransformer;
  class CJoystickFamilyManager;
  class IDatabaseCallbacks;

  class CButtonMapper
  {
  public:
    CButtonMapper(ADDON::CHelper_libKODI_peripheral* peripheralLib);
    ~CButtonMapper();

    bool Initialize(CJoystickFamilyManager& familyManager);
    void Deinitialize();

    IDatabaseCallbacks* GetCallbacks();

    bool GetFeatures(const ADDON::Joystick& joystick, const std::string& strDeviceId, FeatureVector& features);

    void RegisterDatabase(const DatabasePtr& database);
    void UnregisterDatabase(const DatabasePtr& database);

  private:
    ButtonMap GetButtonMap(const ADDON::Joystick& joystick) const;
    static void MergeButtonMap(ButtonMap& knownMap, const ButtonMap& newFeatures);
    static void MergeFeatures(FeatureVector& features, const FeatureVector& newFeatures);
    bool GetFeatures(const ADDON::Joystick& joystick, ButtonMap&& buttonMap, const std::string& controllerId, FeatureVector& features);
    void DeriveFeatures(const ADDON::Joystick& joystick, const std::string& toController, const ButtonMap& buttonMap, FeatureVector& transformedFeatures);

    DatabaseVector    m_databases;
    std::unique_ptr<CControllerTransformer> m_controllerTransformer;

    ADDON::CHelper_libKODI_peripheral* m_peripheralLib;
  };
}
