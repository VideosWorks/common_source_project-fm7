# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,National JR-800, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")


project (emujr800)

set(EXEC_TARGET emujr800)
set(VM_NAME jr800)
set(USE_FMGEN OFF)
set(WITH_JOYSTICK ON)
set(WITH_MOUSE OFF)

set(VMFILES_BASE
		memory.cpp
		event.cpp
)

set(VMFILES_LIB 
		datarec.cpp
		hd44102.cpp
		hd6301.cpp
		mc6801.cpp
		mc6800.cpp
		noise.cpp
		pcm1bit.cpp
)

set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger.")

include(detect_target_cpu)
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

set(VMFILES ${VMFILES_BASE})
add_definitions(-D_JR800)
set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/jr800.qrc)
include(config_commonsource)
