file(COPY D:/Projects/Viqsoft/XBMC/arcko-xbmc/project/cmake/addons/build/pvr.stalker/depends/common/libxml2/CMakeLists.txt
                     DESTINATION D:/Projects/Viqsoft/XBMC/arcko-xbmc/project/cmake/addons/build/libxml2/src/libxml2)
execute_process(COMMAND "D:/Projects/Viqsoft/XBMC/arcko-xbmc/project/cmake/addons/output/bin/patch.exe" --binary -p1 -i "D:/Projects/Viqsoft/XBMC/arcko-xbmc/project/cmake/addons/build/pvr.stalker/depends/common/libxml2/win32_io.patch")
