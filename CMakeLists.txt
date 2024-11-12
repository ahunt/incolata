cmake_minimum_required(VERSION 3.16)

project(incolata VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#set(QT_MINIMUM_VERSION 6.0.0)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets LinguistTools Charts)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools Charts)

set(TS_FILES incolata_en_GB.ts)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        exercisesmodel.h
        exercisesmodel.cpp
        fitfactorchartview.h
        fitfactorchartview.cpp
        sequentialseriesscrollingchartview.h
        sequentialseriesscrollingchartview.cpp
        testworker.h
        testworker.cpp
        ${TS_FILES}
)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(incolata
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET incolata APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(incolata SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(incolata
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_compile_options(${PROJECT_NAME} PRIVATE
  -Wall -Wextra -Wpedantic -Werror
)

include(ExternalProject)

ExternalProject_Add(
	libp8020build
	CONFIGURE_COMMAND ""
	INSTALL_COMMAND ""
	BUILD_COMMAND cargo build "$<$<NOT:$<CONFIG:Debug>>:--release>"
	COMMAND_EXPAND_LISTS
	SOURCE_DIR "${CMAKE_SOURCE_DIR}/libp8020"
	BINARY_DIR "${CMAKE_SOURCE_DIR}/libp8020"
	BUILD_BYPRODUCTS ${CMAKE_SOURCE_DIR}/libp8020/target/debug/libp8020.a ${CMAKE_SOURCE_DIR}/libp8020/target/release/libp8020.a)
add_library(libp8020 STATIC IMPORTED)
add_dependencies(libp8020 libp8020build)
set_target_properties(libp8020 PROPERTIES
  IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/libp8020/target/release/libp8020.a"
  IMPORTED_LOCATION_DEBUG "${CMAKE_SOURCE_DIR}/libp8020/target/debug/libp8020.a")

target_link_libraries(incolata
  PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Charts libp8020 udev)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER org.ahunt.incolata)
endif()
set_target_properties(incolata PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS incolata
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(incolata)
endif()
