# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,NEC PC-100, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")

project (pc100)

set(LOCAL_LIBS 	   vm_pc100
		   vm_vm
		   common_common
		   qt_debugger
		   qt_pc100
		   qt_gui
                   )

set(VMFILES
		   i286.cpp
		   
		   and.cpp
		   
		   i8251.cpp
		   i8255.cpp
		   i8259.cpp
		   
		   msm58321.cpp
		   upd765a.cpp
		   
		   beep.cpp
		   memory.cpp
		   pcm1bit.cpp
		   
		   disk.cpp
		   event.cpp
		   io.cpp
)

set(BUILD_SHARED_LIBS OFF)
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")

include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

add_definitions(-D_PC100)
set(EXEC_TARGET emupc100)
set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/pc100.qrc)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/pc100)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/pc100)

include(config_commonsource)

add_subdirectory(../../src/vm/pc100 vm/pc100)
add_subdirectory(../../src/qt/pc100 qt/pc100)
add_subdirectory(../../src/qt/common qt/common)
add_subdirectory(../../src/qt/debugger qt/debugger)