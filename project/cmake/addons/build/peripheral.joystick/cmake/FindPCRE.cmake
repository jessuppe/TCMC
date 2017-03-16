# - Try to find pcrecpp
# Once done this will define
#
# PCRE_FOUND - system has libpcrecpp
# PCRE_INCLUDE_DIRS - the libpcrecpp include directory
# PCRE_LIBRARIES - The libpcrecpp libraries
# PCRE_DEFINITIONS - required definitions

if(PKG_CONFIG_FOUND)
  pkg_check_modules (PCRE libpcrecpp)
  if(NOT PCRE_INCLUDE_DIRS)
    set(PCRE_INCLUDE_DIRS ${PCRE_INCLUDEDIR})
  endif()
else()
  find_path(PCRE_INCLUDE_DIRS pcrecpp.h)
  find_library(PCRECPP_LIBRARY NAMES pcrecpp pcrecppd)
  find_library(PCRE_LIBRARY NAMES pcre pcred)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PCRE DEFAULT_MSG PCRE_INCLUDE_DIRS PCRECPP_LIBRARY PCRE_LIBRARY)
mark_as_advanced(PCRE_INCLUDE_DIRS PCRECPP_LIBRARY PCRE_LIBRARY)
if(PCRE_FOUND)
  set(PCRE_LIBRARIES ${PCRECPP_LIBRARY} ${PCRE_LIBRARY})
  if(WIN32)
    set(PCRE_DEFINITIONS "-DPCRE_STATIC=1")
  endif()
endif()

