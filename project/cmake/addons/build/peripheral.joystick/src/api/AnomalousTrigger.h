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
 */
#pragma once

#include "IJoystickAxisFilter.h"

namespace JOYSTICK
{
  /*!
   * \brief Filter for anomalous triggers
   *
   * Most gamepad triggers report a value in the interval [0.0, 1.0], where 0.0
   * is depressed and 1.0 is fully pressed.
   *
   * Anomalous triggers, when depressed, have a value of -1.0 or 1.0. When
   * fully pressed, the value can either be zero or the depressed value's
   * opposite.
   *
   * Some APIs, such as cocoa, are event-driven and do not report an initial
   * value. We assume that the triggers are depressed when the controller is
   * detected and that the initial perturbation is small. An anomaly is
   * detected if the initial value is close to -1.0 or 1.0.
   *
   * At first, the trigger is expected to range between its center value and
   * zero. When a value is encountered that lies in the opposite semiaxis, the
   * range is updated to include the full interval [-1.0, 1.0].
   *
   * Triggers centered about -1.0 are transformed to travel from zero to 1.0.
   *
   * Triggers centered about 1.0 are transformed to travel from zero to -1.0.
   */
  class CAnomalousTrigger : public IJoystickAxisFilter
  {
  public:
    CAnomalousTrigger(unsigned int axisIndex);

    // implementation of IJoystickAxisFilter
    virtual float Filter(float value) override;

    /*!
     * \brief Has this axis been detected as an anomalous trigger
     */
    bool IsAnomalousTrigger(void) const;

    unsigned int AxisIndex(void) const { return m_axisIndex; }

    int Center(void) const { return GetCenter(m_center); }

    unsigned int Range(void) const { return GetRange(m_range); }

  private:
    enum AXIS_STATE
    {
      /*!
       * \brief Axis properties are unknown
       */
      STATE_UNKNOWN,

      /*!
       * \brief Axis is a dpad with discrete values -1.0, 0.0, 1.0
       */
      STATE_DISCRETE_DPAD,

      /*!
       * \brief Axis is not a discrete dpad, but is otherwise unknown
       */
      STATE_NOT_DISCRETE_DPAD,

      /*!
       * \brief Center has been determined (see AXIS_CENTER)
       */
      STATE_CENTER_KNOWN,

      /*!
       * \brief Range has been determined (see TRIGGER_RANGE)
       */
      STATE_RANGE_KNOWN,
    };

    enum AXIS_CENTER
    {
      CENTER_ZERO,
      CENTER_NEGATIVE_ONE,
      CENTER_POSITIVE_ONE,
    };

    enum TRIGGER_RANGE
    {
      TRIGGER_RANGE_HALF, // trigger value is in the interval [-1.0, 0.0] or [0.0, 1.0]
      TRIGGER_RANGE_FULL, // trigger value is in the interval [-1.0, 1.0]
    };

    /*!
     * \brief Helper functions
     */
    static int GetCenter(AXIS_CENTER center);
    static unsigned int GetRange(TRIGGER_RANGE range);

    const unsigned int m_axisIndex;
    AXIS_STATE         m_state;
    AXIS_CENTER        m_center;
    TRIGGER_RANGE      m_range;
    bool               m_bCenterSeen;
    bool               m_bPositiveOneSeen;
    bool               m_bNegativeOneSeen;
  };
}
