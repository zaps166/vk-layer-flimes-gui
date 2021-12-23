cmake_minimum_required(VERSION 3.13)
project(vk-layer-flimes-gui LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 17)

option(BUILD_QT6 "Build with Qt6" ON)
option(BUILD_QT5 "Build with Qt5" ON)

if(BUILD_QT6 AND NOT BUILD_QT5)
    set(QT6_MAYBE_REQUIRED REQUIRED)
else()
    unset(QT6_MAYBE_REQUIRED)
endif()

if(BUILD_QT6)
    find_package(Qt6 COMPONENTS Widgets ${QT6_MAYBE_REQUIRED})
endif()
if(BUILD_QT5 AND NOT Qt6_FOUND)
    find_package(Qt5 COMPONENTS Widgets X11Extras REQUIRED)
endif()

find_package(PkgConfig REQUIRED)

pkg_check_modules(XCB REQUIRED xcb)
pkg_check_modules(UDEV REQUIRED libudev)

include(GNUInstallDirs)

file(GLOB PROJECT_FILES
    "src/*.cpp"
    "src/*.hpp"
)
add_executable(${PROJECT_NAME}
    ${PROJECT_FILES}
)

target_compile_definitions(${PROJECT_NAME}
    PUBLIC
    -DVK_LAYER_FLIMES_GUI_NAME="vk-layer-flimes-gui"
    -DVK_LAYER_FLIMES_GUI_VERSION="1.5.0"
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${XCB_INCLUDE_DIRS}
    ${UDEV_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    Qt::Widgets
    ${XCB_LINK_LIBRARIES}
    ${UDEV_LINK_LIBRARIES}
)
if (Qt5_FOUND)
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
        Qt::X11Extras
    )
elseif(Qt6_FOUND)
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
        Qt::GuiPrivate
    )
endif()

install(TARGETS ${PROJECT_NAME}
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(FILES "${CMAKE_SOURCE_DIR}/data/vk-layer-flimes-gui.desktop"
    DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications"
)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/data/hicolor"
    DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons"
)
