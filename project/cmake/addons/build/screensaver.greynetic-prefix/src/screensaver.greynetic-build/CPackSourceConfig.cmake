# This file will be configured to contain variables for CPack. These variables
# should be set in the CMake list file of the project before CPack module is
# included. The list of available CPACK_xxx variables and their associated
# documentation may be obtained using
#  cpack --help-variable-list
#
# Some variables are common to all generators (e.g. CPACK_PACKAGE_NAME)
# and some are specific to a generator
# (e.g. CPACK_NSIS_EXTRA_INSTALL_COMMANDS). The generator specific variables
# usually begin with CPACK_<GENNAME>_xxxx.


SET(CPACK_ARCHIVE_COMPONENT_INSTALL "ON")
SET(CPACK_BINARY_7Z "")
SET(CPACK_BINARY_BUNDLE "")
SET(CPACK_BINARY_CYGWIN "")
SET(CPACK_BINARY_DEB "")
SET(CPACK_BINARY_DRAGNDROP "")
SET(CPACK_BINARY_IFW "")
SET(CPACK_BINARY_NSIS "")
SET(CPACK_BINARY_OSXX11 "")
SET(CPACK_BINARY_PACKAGEMAKER "")
SET(CPACK_BINARY_RPM "")
SET(CPACK_BINARY_STGZ "")
SET(CPACK_BINARY_TBZ2 "")
SET(CPACK_BINARY_TGZ "")
SET(CPACK_BINARY_TXZ "")
SET(CPACK_BINARY_TZ "")
SET(CPACK_BINARY_WIX "")
SET(CPACK_BINARY_ZIP "")
SET(CPACK_CMAKE_GENERATOR "NMake Makefiles")
SET(CPACK_COMPONENTS_ALL "screensaver.greynetic-1.1.0")
SET(CPACK_COMPONENTS_ALL_SET_BY_USER "TRUE")
SET(CPACK_COMPONENTS_IGNORE_GROUPS "1")
SET(CPACK_COMPONENT_UNSPECIFIED_HIDDEN "TRUE")
SET(CPACK_COMPONENT_UNSPECIFIED_REQUIRED "TRUE")
SET(CPACK_GENERATOR "7Z;ZIP")
SET(CPACK_IGNORE_FILES "/CVS/;/\\.svn/;/\\.bzr/;/\\.hg/;/\\.git/;\\.swp\$;\\.#;/#")
SET(CPACK_INCLUDE_TOPLEVEL_DIRECTORY "OFF")
SET(CPACK_INSTALLED_DIRECTORIES "D:/Projects/Viqsoft/XBMC/arcko-xbmc/project/cmake/addons/build/screensaver.greynetic;/")
SET(CPACK_INSTALL_CMAKE_PROJECTS "")
SET(CPACK_INSTALL_PREFIX "D:/Projects/Viqsoft/XBMC/arcko-xbmc/project/Win32BuildSetup/BUILD_WIN32/addons")
SET(CPACK_MODULE_PATH "D:/Projects/Viqsoft/XBMC/arcko-xbmc/project/cmake/addons/build/screensaver.greynetic;D:/Projects/Viqsoft/XBMC/arcko-xbmc/project/cmake/addons/output/lib/kodi;D:/Projects/Viqsoft/XBMC/arcko-xbmc/project/cmake/addons/output/share/kodi/cmake")
SET(CPACK_NSIS_DISPLAY_NAME "screensaver.greynetic 0.1.1")
SET(CPACK_NSIS_INSTALLER_ICON_CODE "")
SET(CPACK_NSIS_INSTALLER_MUI_ICON_CODE "")
SET(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES")
SET(CPACK_NSIS_PACKAGE_NAME "screensaver.greynetic 0.1.1")
SET(CPACK_OUTPUT_CONFIG_FILE "D:/Projects/Viqsoft/XBMC/arcko-xbmc/project/cmake/addons/build/screensaver.greynetic-prefix/src/screensaver.greynetic-build/CPackConfig.cmake")
SET(CPACK_PACKAGE_DEFAULT_LOCATION "/")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "C:/Program Files/CMake/share/cmake-3.6/Templates/CPack.GenericDescription.txt")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "screensaver.greynetic built using CMake")
SET(CPACK_PACKAGE_DIRECTORY "C:/Users/arcko/AppData/Local/Temp")
SET(CPACK_PACKAGE_FILE_NAME "screensaver.greynetic-0.1.1-Source")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "screensaver.greynetic 0.1.1")
SET(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "screensaver.greynetic 0.1.1")
SET(CPACK_PACKAGE_NAME "screensaver.greynetic")
SET(CPACK_PACKAGE_RELOCATABLE "true")
SET(CPACK_PACKAGE_VENDOR "Humanity")
SET(CPACK_PACKAGE_VERSION "0.1.1")
SET(CPACK_PACKAGE_VERSION_MAJOR "0")
SET(CPACK_PACKAGE_VERSION_MINOR "1")
SET(CPACK_PACKAGE_VERSION_PATCH "1")
SET(CPACK_RESOURCE_FILE_LICENSE "C:/Program Files/CMake/share/cmake-3.6/Templates/CPack.GenericLicense.txt")
SET(CPACK_RESOURCE_FILE_README "C:/Program Files/CMake/share/cmake-3.6/Templates/CPack.GenericDescription.txt")
SET(CPACK_RESOURCE_FILE_WELCOME "C:/Program Files/CMake/share/cmake-3.6/Templates/CPack.GenericWelcome.txt")
SET(CPACK_SET_DESTDIR "OFF")
SET(CPACK_SOURCE_7Z "ON")
SET(CPACK_SOURCE_CYGWIN "")
SET(CPACK_SOURCE_GENERATOR "7Z;ZIP")
SET(CPACK_SOURCE_IGNORE_FILES "/CVS/;/\\.svn/;/\\.bzr/;/\\.hg/;/\\.git/;\\.swp\$;\\.#;/#")
SET(CPACK_SOURCE_INSTALLED_DIRECTORIES "D:/Projects/Viqsoft/XBMC/arcko-xbmc/project/cmake/addons/build/screensaver.greynetic;/")
SET(CPACK_SOURCE_OUTPUT_CONFIG_FILE "D:/Projects/Viqsoft/XBMC/arcko-xbmc/project/cmake/addons/build/screensaver.greynetic-prefix/src/screensaver.greynetic-build/CPackSourceConfig.cmake")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "screensaver.greynetic-0.1.1-Source")
SET(CPACK_SOURCE_TBZ2 "")
SET(CPACK_SOURCE_TGZ "")
SET(CPACK_SOURCE_TOPLEVEL_TAG "win32-Source")
SET(CPACK_SOURCE_TXZ "")
SET(CPACK_SOURCE_TZ "")
SET(CPACK_SOURCE_ZIP "ON")
SET(CPACK_STRIP_FILES "")
SET(CPACK_SYSTEM_NAME "win32")
SET(CPACK_TOPLEVEL_TAG "win32-Source")
SET(CPACK_WIX_SIZEOF_VOID_P "4")

if(NOT CPACK_PROPERTIES_FILE)
  set(CPACK_PROPERTIES_FILE "D:/Projects/Viqsoft/XBMC/arcko-xbmc/project/cmake/addons/build/screensaver.greynetic-prefix/src/screensaver.greynetic-build/CPackProperties.cmake")
endif()

if(EXISTS ${CPACK_PROPERTIES_FILE})
  include(${CPACK_PROPERTIES_FILE})
endif()
