cmake_minimum_required(VERSION 3.20)

# Add option to force lower CMake version if needed (addresses the specific error)
option(QT_FORCE_MIN_CMAKE_VERSION_FOR_USING_QT "Force a lower minimum CMake version for Qt" "3.20")
if(QT_FORCE_MIN_CMAKE_VERSION_FOR_USING_QT)
    set(QT_FORCE_MIN_CMAKE_VERSION_FOR_USING_QT ${QT_FORCE_MIN_CMAKE_VERSION_FOR_USING_QT} CACHE STRING "Force minimum CMake version for Qt" FORCE)
endif()

project(FCFSSchedulerSimulation VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configure Qt options
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find required Qt packages
find_package(Qt6 COMPONENTS Core Widgets Charts REQUIRED)

# Define source files
set(SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Charts
)

# Set include directories
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Install directives
install(TARGETS ${PROJECT_NAME} DESTINATION bin)

# Copy Qt DLLs to build directory (Windows only)
if(WIN32)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/DeployQt.cmake OPTIONAL)
    if(COMMAND deploy_qt)
        deploy_qt(${PROJECT_NAME})
    endif()
endif()