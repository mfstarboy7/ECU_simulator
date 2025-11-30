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
    
    # Engine & Physics
    src/engine/FuelControl.cpp
    src/engine/FuelControl.h
    src/engine/EnginePhysics.h   # <-- NEW (Lesson 7)

    # Diagnostics
    src/dtc/DTCManager.cpp
    src/dtc/DTCManager.h
    src/dtc/DTC.h

    # CAN Bus Simulation
    src/can/CANBus.cpp           # <-- NEW (Lesson 6)
    src/can/CANBus.h             # <-- NEW (Lesson 6)
    src/can/CANMessage.h         # <-- NEW (Lesson 6)
)

add_subdirectory(src/scheduler)


# cmake_minimum_required(VERSION 3.20)


# project(ECU_simulator VERSION 1.0
#         DESCRIPTION "Automotive ECU Simulation Project"
#         LANGUAGES CXX)

# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

# add_executable(ECU_simulator
#     src/main.cpp
#     src/sensors/SensorModule.cpp
#     src/sensors/SensorModule.h
#     src/filters/Filter.h
#     src/engine/FuelControl.cpp
#     src/engine/FuelControl.h
#     src/dtc/DTCManager.cpp
#     src/dtc/DTCManager.h
#     src/dtc/DTC.h
#     src/can/CANBus.cpp
#     src/can/CANBus.h


# )

# add_subdirectory(src/scheduler)
