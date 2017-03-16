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

namespace JOYSTICK
{
  /*!
   * \brief Model for how controllers map to each other
   */
  class CControllerModel
  {
  public:
    /*!
     * \brief Add data to the model
     *
     * \param controllerMapping  The from and to controllers
     * \param featureMapping     The from and to features
     */
    void AddFeatureMapping(const ControllerMapItem& controllerMapping, const FeatureMapItem& featuremapping);

    /*!
     * \brief Get a translation map for the specified form and to controllers
     *
     * \param needle The from and to controllers
     *
     * \return A map whose keys are from-to feature pairs and whose values are all 1
     */
    const FeatureOccurrences& GetNormalizedFeatures(const ControllerMapItem& needle);

  private:
    void Normalize(const ControllerMapItem& needle, bool bSwap);

    static void NormalizeFeatures(const FeatureOccurrences& feature, FeatureOccurrences& result, bool bSwap);

    ControllerMap m_map;
    ControllerMap m_reducedMap;
  };
}
