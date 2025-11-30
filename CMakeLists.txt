cmake_minimum_required(VERSION 3.20)

project(ECU_simulator VERSION 1.0
        DESCRIPTION "Automotive ECU Simulation Project"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ECU_simulator
    src/main.cpp
    src/sensors/SensorModule.cpp
    src/sensors/SensorModule.h
    src/filters/Filter.h
    src/engine/FuelControl.cpp
    src/engine/FuelControl.h
)

add_subdirectory(src/scheduler)
