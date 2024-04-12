cmake_minimum_required(VERSION 3.5)

project(brain_N5000 VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets PrintSupport)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        ${RES_FILES}
        icon.rc
        ${RES_FILES}

)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(brain_N5000
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        protocol.h protocol.cpp
        json.hpp
        NirsN5000_protocol.hpp
        circular_queue.h

        configparam.h configparam.cpp configparam.ui
        pic.qrc
        devtable.h devtable.cpp
        qcustomplot.cpp qcustomplot.h
        waveplot.h waveplot.cpp
        vftflow.h vftflow.cpp vftflow.ui
        launch.h launch.cpp launch.ui
        mywindow.h mywindow.cpp
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET brain_N5000 APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(brain_N5000 SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(brain_N5000
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(brain_N5000 PRIVATE Qt${QT_VERSION_MAJOR}::Widgets ws2_32 Qt${QT_VERSION_MAJOR}::PrintSupport)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.brain_N5000)
endif()
set_target_properties(brain_N5000 PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS brain_N5000
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(brain_N5000)
endif()
