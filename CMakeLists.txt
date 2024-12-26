cmake_minimum_required(VERSION 3.16)
include(ExternalProject)

project(incolata VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QT_MINIMUM_VERSION 6.0.0)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets LinguistTools Charts)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools Charts)

set(TS_FILES incolata_en_GB.ts)

configure_file(config.h.in include/config.h)
include_directories(${PROJECT_BINARY_DIR}/include)

set(PROJECT_SOURCES
        main.cpp
        application.h
        application.cpp
        mainwindow.h
        mainwindow.cpp
        mainwindow.ui
        connectiondialog.h
        connectiondialog.cpp
        connectiondialog.ui
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

IF (APPLE)
  add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/resources/incolata_icon.icns"
    COMMAND iconutil --convert icns --output "${CMAKE_CURRENT_BINARY_DIR}/resources/incolata_icon.icns" "${CMAKE_SOURCE_DIR}/resources/incolata_icon.iconset"
    DEPENDS "${CMAKE_SOURCE_DIR}/resources/incolata_icon.iconset/")

  set(MACOSX_BUNDLE_ICON_FILE incolata_icon.icns)
  set(app_icon_macos "${CMAKE_CURRENT_BINARY_DIR}/resources/incolata_icon.icns")
  set_source_files_properties(${app_icon_macos} PROPERTIES
    MACOSX_PACKAGE_LOCATION "Resources")
ENDIF()

qt_add_executable(incolata
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
    resources/resources.qrc
    ${app_icon_macos}
)

qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})

target_compile_options(${PROJECT_NAME} PRIVATE
  -Wall -Wextra -Wpedantic -Werror
)

ExternalProject_Add(
	libp8020build
	CONFIGURE_COMMAND ""
	INSTALL_COMMAND ""
	BUILD_COMMAND cargo build "$<$<NOT:$<CONFIG:Debug>>:--release>"
	COMMAND_EXPAND_LISTS
	SOURCE_DIR "${CMAKE_SOURCE_DIR}/libp8020"
	BINARY_DIR "${CMAKE_SOURCE_DIR}/libp8020"
	BUILD_ALWAYS true
	BUILD_BYPRODUCTS ${CMAKE_SOURCE_DIR}/libp8020/target/debug/libp8020.a ${CMAKE_SOURCE_DIR}/libp8020/target/release/libp8020.a)
add_library(libp8020 STATIC IMPORTED)
add_dependencies(libp8020 libp8020build)
set_target_properties(libp8020 PROPERTIES
  IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/libp8020/target/release/libp8020.a"
  IMPORTED_LOCATION_DEBUG "${CMAKE_SOURCE_DIR}/libp8020/target/debug/libp8020.a")

IF (UNIX AND NOT APPLE)
  set(AdditionalLibs udev)
ENDIF()

target_link_libraries(incolata
  PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Charts libp8020 ${AdditionalLibs})

set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER org.ahunt.incolata)

set_target_properties(incolata PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_NAME "Incolata"
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

qt_finalize_executable(incolata)
