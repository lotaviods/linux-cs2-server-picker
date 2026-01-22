cmake_minimum_required(VERSION 3.16)

# This is the key for VSCode IntelliSense
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(CS2ServerPicker VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt modules
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Widgets
    Network
    Concurrent
)

qt_standard_project_setup()

# -----------------------------
# Daemon target
# -----------------------------
qt_add_executable(CS2ServerPickerDaemon
    src/daemon.cpp
    src/firewall/linuxfirewallservice.cpp
    src/firewall/linuxfirewallservice.h
    src/firewall/ifirewallservice.h
)

target_link_libraries(CS2ServerPickerDaemon PRIVATE
    Qt6::Core
    Qt6::Network
    Qt6::Concurrent
)

# -----------------------------
# GUI target
# -----------------------------
qt_add_executable(CS2ServerPicker
    src/main.cpp
    src/mainwindow.h
    src/mainwindow.cpp
    src/serverinfo.h
    src/serverservice.h
    src/serverservice.cpp
    src/pingservice.h
    src/pingservice.cpp
    src/firewall/ifirewallservice.h
    src/firewall/firewallclient.h
    src/firewall/firewallclient.cpp
)

target_link_libraries(CS2ServerPicker PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::Concurrent
)

# -----------------------------
# Packaging (tar.gz)
# -----------------------------
add_custom_target(tar-archive ALL
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/CS2ServerPicker-package
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/CS2ServerPicker-package
    COMMAND ${CMAKE_COMMAND} -E copy CS2ServerPicker ${CMAKE_BINARY_DIR}/CS2ServerPicker-package/
    COMMAND ${CMAKE_COMMAND} -E copy CS2ServerPickerDaemon ${CMAKE_BINARY_DIR}/CS2ServerPicker-package/
    COMMAND ${CMAKE_COMMAND} -E tar cvfz ${CMAKE_BINARY_DIR}/CS2ServerPicker.tar.gz CS2ServerPicker-package
    DEPENDS CS2ServerPicker CS2ServerPickerDaemon
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Creating tar archive with CS2ServerPicker binaries"
)
