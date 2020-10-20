cmake_minimum_required(VERSION 3.13)

project(vk-layer-flimes-gui LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_STANDARD 17)

find_package(QT NAMES Qt6 Qt5)
find_package(Qt${QT_VERSION_MAJOR}
    COMPONENTS
    Widgets
    X11Extras
    REQUIRED
)

find_package(PkgConfig REQUIRED)

pkg_check_modules(XCB REQUIRED xcb xcb-keysyms)
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
    -DVK_LAYER_FLIMES_GUI_VERSION="1.1.0"
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${XCB_INCLUDE_DIRS}
    ${UDEV_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::X11Extras
    ${XCB_LINK_LIBRARIES}
    ${UDEV_LINK_LIBRARIES}
)

install(TARGETS ${PROJECT_NAME}
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(FILES "${CMAKE_SOURCE_DIR}/data/vk-layer-flimes-gui.desktop"
    DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications"
)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/data/hicolor"
    DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons"
)
