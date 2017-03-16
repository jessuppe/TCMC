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

#include "DeviceXml.h"
#include "ButtonMapDefinitions.h"
#include "storage/Device.h"
#include "storage/StorageUtils.h"
#include "log/Log.h"

#include "tinyxml.h"

#include <cstdlib>

using namespace JOYSTICK;

bool CDeviceXml::Serialize(const CDevice& record, TiXmlElement* pElement)
{
  pElement->SetAttribute(BUTTONMAP_XML_ATTR_DEVICE_NAME, record.Name());
  pElement->SetAttribute(BUTTONMAP_XML_ATTR_DEVICE_PROVIDER, record.Provider());
  if (record.IsVidPidKnown())
  {
    pElement->SetAttribute(BUTTONMAP_XML_ATTR_DEVICE_VID, CStorageUtils::FormatHexString(record.VendorID()));
    pElement->SetAttribute(BUTTONMAP_XML_ATTR_DEVICE_PID, CStorageUtils::FormatHexString(record.ProductID()));
  }
  if (record.ButtonCount() != 0)
    pElement->SetAttribute(BUTTONMAP_XML_ATTR_DEVICE_BUTTONCOUNT, record.ButtonCount());
  if (record.HatCount() != 0)
    pElement->SetAttribute(BUTTONMAP_XML_ATTR_DEVICE_HATCOUNT, record.HatCount());
  if (record.AxisCount() != 0)
    pElement->SetAttribute(BUTTONMAP_XML_ATTR_DEVICE_AXISCOUNT, record.AxisCount());
  if (record.Index() != 0)
    pElement->SetAttribute(BUTTONMAP_XML_ATTR_DEVICE_INDEX, record.Index());
  return true;
}

bool CDeviceXml::Deserialize(const TiXmlElement* pElement, CDevice& record)
{
  const char* name = pElement->Attribute(BUTTONMAP_XML_ATTR_DEVICE_NAME);
  if (!name)
  {
    esyslog("<%s> tag has no \"%s\" attribute", BUTTONMAP_XML_ELEM_DEVICE, BUTTONMAP_XML_ATTR_DEVICE_NAME);
    return false;
  }
  record.SetName(name);

  const char* provider = pElement->Attribute(BUTTONMAP_XML_ATTR_DEVICE_PROVIDER);
  if (!provider)
  {
    esyslog("<%s> tag has no \"%s\" attribute", BUTTONMAP_XML_ELEM_DEVICE, BUTTONMAP_XML_ATTR_DEVICE_PROVIDER);
    return false;
  }
  record.SetProvider(provider);

  const char* vid = pElement->Attribute(BUTTONMAP_XML_ATTR_DEVICE_VID);
  if (vid)
    record.SetVendorID(CStorageUtils::HexStringToInt(vid));

  const char* pid = pElement->Attribute(BUTTONMAP_XML_ATTR_DEVICE_PID);
  if (pid)
    record.SetProductID(CStorageUtils::HexStringToInt(pid));

  const char* buttonCount = pElement->Attribute(BUTTONMAP_XML_ATTR_DEVICE_BUTTONCOUNT);
  if (buttonCount)
    record.SetButtonCount(std::atoi(buttonCount));

  const char* hatCount = pElement->Attribute(BUTTONMAP_XML_ATTR_DEVICE_HATCOUNT);
  if (hatCount)
    record.SetHatCount(std::atoi(hatCount));

  const char* axisCount = pElement->Attribute(BUTTONMAP_XML_ATTR_DEVICE_AXISCOUNT);
  if (axisCount)
    record.SetAxisCount(std::atoi(axisCount));

  const char* index = pElement->Attribute(BUTTONMAP_XML_ATTR_DEVICE_INDEX);
  if (index)
    record.SetIndex(std::atoi(index));

  return true;
}
