# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,SHINKO SANGYOU  YS6464A, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")


project (emuys6464a)

set(EXEC_TARGET emuys6464a)
set(VM_NAME ys6464a)
set(USE_FMGEN OFF)
set(WITH_JOYSTICK OFF)
set(WITH_MOUSE ON)

set(FLAG_USE_Z80 ON)
set(VMFILES_BASE
  i8255.cpp
 
  io.cpp
  memory.cpp

  event.cpp
)


set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(WITH_DEBUGGER OFF CACHE BOOL "Build with debugger.")


include(detect_target_cpu)
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

set(VMFILES ${VMFILES_BASE})

add_definitions(-D_YS6464A)
add_definitions(-DWITHOUT_SOUND)
set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/ys6464a.qrc)

include(config_commonsource)
