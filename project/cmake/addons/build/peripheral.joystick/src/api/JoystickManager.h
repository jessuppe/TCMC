/*
 *      Copyright (C) 2014-2015 Garrett Brown
 *      Copyright (C) 2014-2015 Team XBMC
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

#include "JoystickTypes.h"
#include "buttonmapper/ButtonMapTypes.h"

#include "kodi_peripheral_utils.hpp"
#include "p8-platform/threads/mutex.h"

#include <vector>

namespace JOYSTICK
{
  class IJoystickInterface;

  class IScannerCallback
  {
  public:
    virtual ~IScannerCallback(void) { }

    /*!
     * \brief Trigger a scan for joysticks
     */
    virtual void TriggerScan(void) = 0;
  };

  class CJoystickManager
  {
  private:
    CJoystickManager(void);

  public:
    static CJoystickManager& Get(void);
    virtual ~CJoystickManager(void) { Deinitialize(); }

    /*!
     * \brief Initialize the joystick manager
     *
     * \param scanner The callback used to trigger a scan
     */
    bool Initialize(IScannerCallback* scanner);

    /*!
     * \brief Deinitialize the joystick manager
     */
    void Deinitialize(void);

    /*!
     * \brief Scan the available interfaces for joysticks
     *
     * \param joysticks The discovered joysticks; must be deallocated
     *
     * \return true if the scan succeeded (even if there are no joysticks)
     */
    bool PerformJoystickScan(JoystickVector& joysticks);

    JoystickPtr GetJoystick(unsigned int index) const;

    JoystickVector GetJoysticks(const ADDON::Joystick& joystickInfo) const;

    /*!
    * \brief Get all events that have occurred since the last call to GetEvents()
    */
    bool GetEvents(std::vector<ADDON::PeripheralEvent>& events);

    /*!
     * \brief Send an event to a joystick
     *
     * \param event The event
     *
     * \return True if the event was handled
     */
    bool SendEvent(const ADDON::PeripheralEvent& event);

    /*!
     * \brief Process events that have arrived since the last call to ProcessEvents()
     */
    void ProcessEvents();

    /*!
     * \brief Trigger a scan for joysticks through the callback
     */
    void TriggerScan(void);

    /*!
     * \brief Get the button map known to the interface
     *
     * \param provider Name of the joystick interface
     *
     * \return A button map populated with hard-coded features for the interface
     */
    const ButtonMap& GetButtonMap(const std::string& provider);

  private:
    IScannerCallback*                m_scanner;
    std::vector<IJoystickInterface*> m_interfaces;
    JoystickVector                   m_joysticks;
    unsigned int                     m_nextJoystickIndex;
    mutable P8PLATFORM::CMutex         m_interfacesMutex;
    mutable P8PLATFORM::CMutex         m_joystickMutex;
  };
}
