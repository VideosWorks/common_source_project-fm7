# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,EPSON HC 80, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")


project (emuhc80)
set(VM_NAME hc80)
set(USE_FMGEN OFF)

set(EXEC_TARGET emuhc80)
set(WITH_JOYSTICK OFF)
set(WITH_MOUSE OFF)

set(FLAG_USE_Z80 ON)
set(VMFILES_BASE
      event.cpp
)
set(VMFILES_LIB
      beep.cpp
      disk.cpp
      i8251.cpp
	  noise.cpp
      ptf20.cpp
)

set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger.")

include(detect_target_cpu)
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

set(VMFILES ${VMFILES_BASE})
add_definitions(-D_HC80)
set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/hc80.qrc)

include(config_commonsource)
