cmake_minimum_required(VERSION 3.16)

project(Before_Effects VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTORCC ON)

# Fix for copy relocations against protected Qt6 symbols
# Required for glibc 2.39+ (Arch Linux, Ubuntu 24.04+)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Enforce minimum compiler versions to avoid ABI issues with Qt6
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "12.0")
        message(FATAL_ERROR
            "GCC 12+ is required to avoid copy relocation issues with Qt6 on modern glibc.\n"
            "Found: GCC ${CMAKE_CXX_COMPILER_VERSION}\n"
            "On Ubuntu 22.04: sudo apt install gcc-12 g++-12"
        )
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "14.0")
        message(FATAL_ERROR
            "Clang 14+ is required to avoid copy relocation issues with Qt6 on modern glibc.\n"
            "Found: Clang ${CMAKE_CXX_COMPILER_VERSION}"
        )
    endif()
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets Svg Core SvgWidgets)
qt_standard_project_setup()

set(QT_VERSION_MAJOR 6)
add_subdirectory(external/Qt-Color-Widgets)

qt_add_executable(Before_Effects
    main.cpp
    mainwindow.cpp mainwindow.h
    timeline.cpp timeline.h
    mainwindow.ui
    resources.qrc
    viewPort.cpp viewPort.h
)

# Apply -fPIC explicitly to this target (belt-and-suspenders alongside CMAKE_POSITION_INDEPENDENT_CODE)
target_compile_options(Before_Effects PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-fPIC>
)

target_link_libraries(Before_Effects PRIVATE
    Qt6::Widgets
    Qt6::Core
    Qt6::Svg
    Qt6::SvgWidgets
    QtColorWidgets
)

set_target_properties(Before_Effects PROPERTIES
    MACOSX_BUNDLE TRUE
    # WIN32_EXECUTABLE TRUE          # hides the console window on Windows, comment to enable qDebug()
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

include(GNUInstallDirs)
install(TARGETS Before_Effects
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)