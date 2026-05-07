# Install script for directory: /mnt/d/Code/flavortown/Before_Effects/external/Qt-Color-Widgets

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/mnt/d/Code/flavortown/Before_Effects/linux_build/external/Qt-Color-Widgets/include/QtColorWidgets/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/mnt/d/Code/flavortown/Before_Effects/linux_build/external/Qt-Color-Widgets/resources/QtColorWidgets/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/mnt/d/Code/flavortown/Before_Effects/linux_build/external/Qt-Color-Widgets/src/QtColorWidgets/cmake_install.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/mnt/d/Code/flavortown/Before_Effects/linux_build/external/Qt-Color-Widgets/libQtColorWidgets.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE DIRECTORY FILES "/mnt/d/Code/flavortown/Before_Effects/external/Qt-Color-Widgets/include/" FILES_MATCHING REGEX "^.*.h$|^.*.hpp$|^.*$" REGEX "CMakeLists.txt" EXCLUDE)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Devel" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/QtColorWidgets" TYPE FILE FILES "/mnt/d/Code/flavortown/Before_Effects/linux_build/external/Qt-Color-Widgets/QtColorWidgets_version.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Devel" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/QtColorWidgets" TYPE FILE FILES "/mnt/d/Code/flavortown/Before_Effects/external/Qt-Color-Widgets/include/QtColorWidgets/colorwidgets_global.hpp")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Devel" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/QtColorWidgets/QtColorWidgets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/QtColorWidgets/QtColorWidgets.cmake"
         "/mnt/d/Code/flavortown/Before_Effects/linux_build/external/Qt-Color-Widgets/CMakeFiles/Export/0ac40480edeb2b2d2e4ffe6bbab4636f/QtColorWidgets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/QtColorWidgets/QtColorWidgets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/QtColorWidgets/QtColorWidgets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/QtColorWidgets" TYPE FILE FILES "/mnt/d/Code/flavortown/Before_Effects/linux_build/external/Qt-Color-Widgets/CMakeFiles/Export/0ac40480edeb2b2d2e4ffe6bbab4636f/QtColorWidgets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^()$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/QtColorWidgets" TYPE FILE FILES "/mnt/d/Code/flavortown/Before_Effects/linux_build/external/Qt-Color-Widgets/CMakeFiles/Export/0ac40480edeb2b2d2e4ffe6bbab4636f/QtColorWidgets-noconfig.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/QtColorWidgets/qtcolorwidgets-targets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/QtColorWidgets/qtcolorwidgets-targets.cmake"
         "/mnt/d/Code/flavortown/Before_Effects/linux_build/external/Qt-Color-Widgets/CMakeFiles/Export/0ac40480edeb2b2d2e4ffe6bbab4636f/qtcolorwidgets-targets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/QtColorWidgets/qtcolorwidgets-targets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/QtColorWidgets/qtcolorwidgets-targets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/QtColorWidgets" TYPE FILE FILES "/mnt/d/Code/flavortown/Before_Effects/linux_build/external/Qt-Color-Widgets/CMakeFiles/Export/0ac40480edeb2b2d2e4ffe6bbab4636f/qtcolorwidgets-targets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^()$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/QtColorWidgets" TYPE FILE FILES "/mnt/d/Code/flavortown/Before_Effects/linux_build/external/Qt-Color-Widgets/CMakeFiles/Export/0ac40480edeb2b2d2e4ffe6bbab4636f/qtcolorwidgets-targets-noconfig.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Devel" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/QtColorWidgets" TYPE FILE FILES
    "/mnt/d/Code/flavortown/Before_Effects/linux_build/external/Qt-Color-Widgets/QtColorWidgets/qtcolorwidgets-config.cmake"
    "/mnt/d/Code/flavortown/Before_Effects/linux_build/external/Qt-Color-Widgets/QtColorWidgets/qtcolorwidgets-config-version.cmake"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/mnt/d/Code/flavortown/Before_Effects/linux_build/external/Qt-Color-Widgets/QtColorWidgets.pc")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/mnt/d/Code/flavortown/Before_Effects/linux_build/external/Qt-Color-Widgets/gallery/cmake_install.cmake")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/mnt/d/Code/flavortown/Before_Effects/linux_build/external/Qt-Color-Widgets/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
