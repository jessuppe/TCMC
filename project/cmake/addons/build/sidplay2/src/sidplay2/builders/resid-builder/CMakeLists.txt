project(resid-builder)

enable_language(CXX)

cmake_minimum_required(VERSION 2.6)

add_definitions(-D_CRT_SECURE_NO_WARNINGS -DHAVE_MSWINDOWS)

include_directories(${PROJECT_SOURCE_DIR}/../../libsidplay/include
                    ${PROJECT_SOURCE_DIR}/../..
                    ${PROJECT_SOURCE_DIR}/include
                    ${PROJECT_SOURCE_DIR}/include/sidplay/builders)

set(SOURCES src/resid.cpp
            src/resid-builder.cpp)

add_library(resid-builder STATIC ${SOURCES})

set(HEADERS ${PROJECT_SOURCE_DIR}/include/sidplay/builders/resid.h)

install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/sidplay/builders)
install(TARGETS resid-builder DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
