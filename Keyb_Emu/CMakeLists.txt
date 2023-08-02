# CMakeList.txt : CMake project for Keyb_Emu, include source and define
# project specific logic here.
#
project(Keyb_Emu)
cmake_minimum_required (VERSION 3.4)

# Add source to this project's executable.
add_executable (Keyb_Emu "Keyb_Emu.cpp" "Keyb_Emu.h")

if (MSVC)
set_target_properties(${PROJECT_NAME} PROPERTIES
LINK_FLAGS /SUBSYSTEM:CONSOLE
)
endif (MSVC)

if (UNIX)
TARGET_LINK_LIBRARIES(Keyb_Emu X11 Xtst)
target_compile_features(Keyb_Emu PUBLIC cxx_std_17)
endif (UNIX)

# TODO: Add tests and install targets if needed.
