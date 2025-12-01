cmake_minimum_required(VERSION 3.20)

project(ECU_simulator VERSION 1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- 1. Fetch Dependencies (GUI Libraries) ---
include(FetchContent)

# Download GLFW (Window Manager)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.3.8
)
FetchContent_MakeAvailable(glfw)

# Download Dear ImGui (The GUI Library)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.89.9
)
FetchContent_MakeAvailable(imgui)

# Suppress warnings for external libraries
if(MSVC)
  add_compile_options(/wd5287)
endif()

# --- 2. Create the Executable ---
add_executable(ECU_simulator
    src/main.cpp
    
    # Scheduler
    src/scheduler/Scheduler.cpp
    src/scheduler/Scheduler.h

    # Sensors & Engine
    src/sensors/SensorModule.cpp
    src/sensors/SensorModule.h
    src/Filters/Filter.h          
    src/engine/FuelControl.cpp
    src/engine/FuelControl.h
    src/engine/EnginePhysics.h

    # Diagnostics & Memory
    src/dtc/DTCManager.cpp
    src/dtc/DTCManager.h
    src/dtc/DTC.h
    src/memory/FlashMemory.cpp
    src/memory/FlashMemory.h

    # Comms & Logging
    src/can/CANBus.cpp
    src/can/CANBus.h
    src/can/CANMessage.h
    src/logging/Logger.cpp
    src/logging/Logger.h
    
    # ImGui Sources
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

# --- 3. Link Libraries ---
target_include_directories(ECU_simulator PRIVATE 
    ${imgui_SOURCE_DIR} 
    ${imgui_SOURCE_DIR}/backends
    src/scheduler
)

# FIXED: Explicitly find and link OpenGL using standard CMake package
find_package(OpenGL REQUIRED)

if(WIN32)
    # Link OpenGL + standard Windows networking (ws2_32) just in case
    target_link_libraries(ECU_simulator PRIVATE glfw opengl32)
else()
    target_link_libraries(ECU_simulator PRIVATE glfw OpenGL::GL)
endif()