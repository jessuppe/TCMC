#pragma once
/*
 *      Copyright (C) 2005-2013 Team XBMC
 *      http://xbmc.org
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

#include "Addon.h"
#include "DllAddon.h"
#include "AddonManager.h"
#include "addons/interfaces/AddonInterfaces.h"
#include "utils/XMLUtils.h"

namespace ADDON
{
  class CAddonDll : public CAddon
  {
  public:
    CAddonDll(AddonProps props);

    //FIXME: does shallow pointer copy. no copy assignment op
    CAddonDll(const CAddonDll &rhs);
    virtual ~CAddonDll();
    virtual ADDON_STATUS GetStatus();

    // addon settings
    virtual void SaveSettings();
    virtual std::string GetSetting(const std::string& key);

    ADDON_STATUS Create(ADDON_TYPE type, void* funcTable, void* info);
    void Destroy();

    bool DllLoaded(void) const;

    /*!
     * @brief Function to create a addon instance class
     *
     * @param[in] instanceType The wanted instance type class to open on addon
     * @param[in] instanceID The from Kodi used ID string of active instance
     * @param[in] instance Pointer where the interface functions from addon
     *                     becomes stored during his instance creation.
     * @param[in] parentInstance In case the instance class is related to another
     *                           addon instance class becomes with the pointer
     *                           given to addon. Is optional and most addon types
     *                           not use it.
     * @return The status of addon after the creation.
     */
    ADDON_STATUS CreateInstance(ADDON_TYPE instanceType, const std::string& instanceID, KODI_HANDLE instance, KODI_HANDLE parentInstance = nullptr);

    /*!
     * @brief Function to destroy a on addon created instance class
     *
     * @param[in] instanceID The from Kodi used ID string of active instance
     */
    void DestroyInstance(const std::string& instanceID);

  protected:
    bool Initialized() { return m_initialized; }
    static uint32_t GetChildCount() { static uint32_t childCounter = 0; return childCounter++; }
    CAddonInterfaces* m_pHelpers;
    bool m_bIsChild;
    std::string m_parentLib;

  private:
    /*!
     * @brief Main addon creation call function
     *
     * This becomes called only one time before a addon instance becomes created.
     * If another instance becomes requested is this Create no more used. To see
     * like a "int main()" on exe.
     *
     * @param[in] firstKodiInstance The first instance who becomes used.
     *                              In case addon supports only one instance
     *                              and not multiple together can addon use
     *                              only one complete class for everything.
     *                              This is used then to interact on interface.
     * @return The status of addon after the creation.
     */
    ADDON_STATUS Create(KODI_HANDLE firstKodiInstance);

    bool CheckAPIVersion(int type);

    DllAddon* m_pDll;
    bool m_initialized;
    bool LoadDll();
    bool m_needsavedsettings;
    std::map<std::string, std::pair<ADDON_TYPE, KODI_HANDLE>> m_usedInstances;

    virtual ADDON_STATUS TransferSettings();

    static void AddOnStatusCallback(void *userData, const ADDON_STATUS status, const char* msg);
    static bool AddOnGetSetting(void *userData, const char *settingName, void *settingValue);
    static void AddOnOpenSettings(const char *url, bool bReload);
    static void AddOnOpenOwnSettings(void *userData, bool bReload);

    /// addon to kodi basic callbacks below
    //@{

    /*!
     * This structure, which is fixed to the addon headers, makes use of the at
     * least supposed parts for the interface.
     * This structure is defined in:
     * /xbmc/addons/kodi-addon-dev-kit/include/kodi/AddonBase.h
     */
    AddonGlobalInterface m_interface;

    inline bool InitInterface(KODI_HANDLE firstKodiInstance);
    inline void DeInitInterface();

    static char* get_addon_path(void* kodiBase);
    static char* get_base_user_path(void* kodiBase);
    static void addon_log_msg(void* kodiBase, const int addonLogLevel, const char* strMessage);
    static bool get_setting(void* kodiBase, const char* settingName, void* settingValue);
    static bool set_setting(void* kodiBase, const char* settingName, const char* settingValue);
    static void free_string(void* kodiBase, char* str);
    //@}
  };

}; /* namespace ADDON */

