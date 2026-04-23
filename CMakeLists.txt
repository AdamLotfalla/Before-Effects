cmake_minimum_required(VERSION 3.16)

project(Before_Effects VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Svg Core)
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


target_link_libraries(Before_Effects PRIVATE Qt${QT_VERSION_MAJOR}::Widgets 
    Qt6::Widgets
    Qt6::Core
    Qt6::Svg
    QtColorWidgets
)

set_target_properties(Before_Effects PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE          # hides the console window on Windows, comment to enable qDebug()
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

include(GNUInstallDirs)
install(TARGETS Before_Effects
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
