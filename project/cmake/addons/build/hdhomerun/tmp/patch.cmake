file(COPY D:/Projects/Viqsoft/XBMC/arcko-xbmc/project/cmake/addons/build/pvr.hdhomerun/depends/common/hdhomerun/CMakeLists.txt
                     DESTINATION D:/Projects/Viqsoft/XBMC/arcko-xbmc/project/cmake/addons/build/hdhomerun/src/hdhomerun)
execute_process(COMMAND "D:/Projects/Viqsoft/XBMC/arcko-xbmc/project/cmake/addons/output/bin/patch.exe" --binary -p1 -i "D:/Projects/Viqsoft/XBMC/arcko-xbmc/project/cmake/addons/build/pvr.hdhomerun/depends/common/hdhomerun/1.patch")
