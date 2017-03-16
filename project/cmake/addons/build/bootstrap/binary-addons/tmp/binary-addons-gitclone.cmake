if("master" STREQUAL "")
  message(FATAL_ERROR "Tag for git checkout should not be empty.")
endif()

set(run 0)

if("D:/Projects/Viqsoft/XBMC/arcko-xbmc/project/cmake/addons/build/bootstrap/binary-addons/src/binary-addons-stamp/binary-addons-gitinfo.txt" IS_NEWER_THAN "D:/Projects/Viqsoft/XBMC/arcko-xbmc/project/cmake/addons/build/bootstrap/binary-addons/src/binary-addons-stamp/binary-addons-gitclone-lastrun.txt")
  set(run 1)
endif()

if(NOT run)
  message(STATUS "Avoiding repeated git clone, stamp file is up to date: 'D:/Projects/Viqsoft/XBMC/arcko-xbmc/project/cmake/addons/build/bootstrap/binary-addons/src/binary-addons-stamp/binary-addons-gitclone-lastrun.txt'")
  return()
endif()

execute_process(
  COMMAND ${CMAKE_COMMAND} -E remove_directory "D:/Projects/Viqsoft/XBMC/arcko-xbmc/project/cmake/addons/build/bootstrap/binary-addons/src/binary-addons"
  RESULT_VARIABLE error_code
  )
if(error_code)
  message(FATAL_ERROR "Failed to remove directory: 'D:/Projects/Viqsoft/XBMC/arcko-xbmc/project/cmake/addons/build/bootstrap/binary-addons/src/binary-addons'")
endif()

set(git_options)

# disable cert checking if explicitly told not to do it
set(tls_verify "")
if(NOT "x" STREQUAL "x" AND NOT tls_verify)
  list(APPEND git_options
    -c http.sslVerify=false)
endif()

set(git_clone_options)

set(git_shallow "")
if(git_shallow)
  list(APPEND git_clone_options --depth 1 --no-single-branch)
endif()

# try the clone 3 times incase there is an odd git clone issue
set(error_code 1)
set(number_of_tries 0)
while(error_code AND number_of_tries LESS 3)
  execute_process(
    COMMAND "D:/DevTools/SCM/Git/cmd/git.exe" ${git_options} clone ${git_clone_options} --origin "origin" "https://github.com/xbmc/repo-binary-addons.git" "binary-addons"
    WORKING_DIRECTORY "D:/Projects/Viqsoft/XBMC/arcko-xbmc/project/cmake/addons/build/bootstrap/binary-addons/src"
    RESULT_VARIABLE error_code
    )
  math(EXPR number_of_tries "${number_of_tries} + 1")
endwhile()
if(number_of_tries GREATER 1)
  message(STATUS "Had to git clone more than once:
          ${number_of_tries} times.")
endif()
if(error_code)
  message(FATAL_ERROR "Failed to clone repository: 'https://github.com/xbmc/repo-binary-addons.git'")
endif()

execute_process(
  COMMAND "D:/DevTools/SCM/Git/cmd/git.exe" ${git_options} checkout master
  WORKING_DIRECTORY "D:/Projects/Viqsoft/XBMC/arcko-xbmc/project/cmake/addons/build/bootstrap/binary-addons/src/binary-addons"
  RESULT_VARIABLE error_code
  )
if(error_code)
  message(FATAL_ERROR "Failed to checkout tag: 'master'")
endif()

execute_process(
  COMMAND "D:/DevTools/SCM/Git/cmd/git.exe" ${git_options} submodule init 
  WORKING_DIRECTORY "D:/Projects/Viqsoft/XBMC/arcko-xbmc/project/cmake/addons/build/bootstrap/binary-addons/src/binary-addons"
  RESULT_VARIABLE error_code
  )
if(error_code)
  message(FATAL_ERROR "Failed to init submodules in: 'D:/Projects/Viqsoft/XBMC/arcko-xbmc/project/cmake/addons/build/bootstrap/binary-addons/src/binary-addons'")
endif()

execute_process(
  COMMAND "D:/DevTools/SCM/Git/cmd/git.exe" ${git_options} submodule update --recursive --init 
  WORKING_DIRECTORY "D:/Projects/Viqsoft/XBMC/arcko-xbmc/project/cmake/addons/build/bootstrap/binary-addons/src/binary-addons"
  RESULT_VARIABLE error_code
  )
if(error_code)
  message(FATAL_ERROR "Failed to update submodules in: 'D:/Projects/Viqsoft/XBMC/arcko-xbmc/project/cmake/addons/build/bootstrap/binary-addons/src/binary-addons'")
endif()

# Complete success, update the script-last-run stamp file:
#
execute_process(
  COMMAND ${CMAKE_COMMAND} -E copy
    "D:/Projects/Viqsoft/XBMC/arcko-xbmc/project/cmake/addons/build/bootstrap/binary-addons/src/binary-addons-stamp/binary-addons-gitinfo.txt"
    "D:/Projects/Viqsoft/XBMC/arcko-xbmc/project/cmake/addons/build/bootstrap/binary-addons/src/binary-addons-stamp/binary-addons-gitclone-lastrun.txt"
  WORKING_DIRECTORY "D:/Projects/Viqsoft/XBMC/arcko-xbmc/project/cmake/addons/build/bootstrap/binary-addons/src/binary-addons"
  RESULT_VARIABLE error_code
  )
if(error_code)
  message(FATAL_ERROR "Failed to copy script-last-run stamp file: 'D:/Projects/Viqsoft/XBMC/arcko-xbmc/project/cmake/addons/build/bootstrap/binary-addons/src/binary-addons-stamp/binary-addons-gitclone-lastrun.txt'")
endif()

