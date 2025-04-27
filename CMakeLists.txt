cmake_minimum_required(VERSION 3.14)

project(FCFSSchedulerSimulation VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required Qt packages
find_package(Qt6 COMPONENTS Core Widgets Charts REQUIRED)
# Fallback to Qt5 if Qt6 not found
if (NOT Qt6_FOUND)
    find_package(Qt5 COMPONENTS Core Widgets Charts REQUIRED)
endif()

# Configure Qt options
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

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
    Qt::Core
    Qt::Widgets
    Qt::Charts
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