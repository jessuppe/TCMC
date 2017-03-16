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

#include "storage/ButtonMap.h"

#include <string>

class TiXmlElement;

namespace ADDON
{
  struct DriverPrimitive;
  class JoystickFeature;
}

namespace JOYSTICK
{
  class CAnomalousTrigger;
  class CButtonMap;

  class CButtonMapXml : public CButtonMap
  {
  public:
    CButtonMapXml(const std::string& strResourcePath);
    CButtonMapXml(const std::string& strResourcePath, const DevicePtr& device);

    virtual ~CButtonMapXml(void) { }

  protected:
    // implementation of CButtonMap
    virtual bool Load(void) override;
    virtual bool Save(void) const override;

  private:
    bool SerializeButtonMaps(TiXmlElement* pElement) const;
    bool SerializeTriggers(TiXmlElement* pElement) const;

    static bool Serialize(const FeatureVector& features, TiXmlElement* pElement);
    static bool Deserialize(const TiXmlElement* pElement, FeatureVector& features);

    static bool IsValid(const ADDON::JoystickFeature& feature);
    static bool SerializeTrigger(TiXmlElement* pElement, const CAnomalousTrigger* trigger);
    static bool SerializeFeature(TiXmlElement* pElement, const ADDON::DriverPrimitive& primitive, const char* tagName);
    static bool SerializePrimitiveTag(TiXmlElement* pElement, const ADDON::DriverPrimitive& primitive, const char* tagName);
    static void SerializePrimitive(TiXmlElement* pElement, const ADDON::DriverPrimitive& primitive);
    static bool DeserializePrimitive(const TiXmlElement* pElement, ADDON::DriverPrimitive& primitive, const std::string& featureName);
  };
}
