# - asplib config mode
#
# Defines the following variables:
#  asplib_FOUND        - true
#  asplib_VERSION      - version of the asplib library found, e.g. 0.2
#  asplib_INCLUDE_DIRS - header directories with which to compile
#  asplib_LINKER_FLAGS - flags that must be passed to the linker
#  asplib_LIBRARIES    - names of the libraries with which to link
#  asplib_LIBRARY_DIRS - directories in which the libraries are situated

# propagate these properties from one build system to the other
set (asplib_VERSION "0.3")
set (asplib_INCLUDE_DIRS D:/Projects/Viqsoft/XBMC/arcko-xbmc/project/cmake/addons/build/asplib/src/asplib;D:/Projects/Viqsoft/XBMC/arcko-xbmc/project/cmake/addons/build/asplib/src/asplib/Biquads;D:/Projects/Viqsoft/XBMC/arcko-xbmc/project/cmake/addons/output/include/asplib D:/Projects/Viqsoft/XBMC/arcko-xbmc/project/cmake/addons/output/include)
set (asplib_LIBRARY_DIRS "")
set (asplib_LINKER_FLAGS "")
set (asplib_CONFIG_VARS "")

# libraries come from the build tree where this file was generated
if(WIN32)
  set (asplib_LIBRARY "D:/Projects/Viqsoft/XBMC/arcko-xbmc/project/cmake/addons/output/lib/asplib.lib")
else(WIN32)
  set (asplib_LIBRARY "-LD:/Projects/Viqsoft/XBMC/arcko-xbmc/project/cmake/addons/output/lib -lasplib")
endif(WIN32)
set (asplib_LIBRARIES ${asplib_LIBRARY} "")
mark_as_advanced (asplib_LIBRARY)

# add the library as a target, so that other things in the project including
# this file may depend on it and get rebuild if this library changes.
add_library (asplib UNKNOWN IMPORTED)
set_property (TARGET asplib PROPERTY IMPORTED_LOCATION "${asplib_LIBRARY}")
