# CMakeList.txt : CMake project for Keyb_Emu, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.4)

# Add source to this project's executable.
add_executable (Keyb_Emu "Keyb_Emu.cpp" "Keyb_Emu.h")

set_target_properties(${PROJECT_NAME} PROPERTIES
LINK_FLAGS /SUBSYSTEM:CONSOLE
)

# TODO: Add tests and install targets if needed.
