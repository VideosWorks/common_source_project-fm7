# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,SEGA SC-3000, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")

project (sc3000)

set(LOCAL_LIBS 	   vm_sc3000
		   vm_vm
		   common_common
		   qt_debugger
		   qt_sc3000
		   qt_gui
                   )

set(VMFILES
		   z80.cpp
		   
		   i8251.cpp
		   i8255.cpp
		   
		   sn76489an.cpp
		   tms9918a.cpp
		   upd765a.cpp
		   
		   datarec.cpp
		   disk.cpp
		   
		   io.cpp
		   event.cpp
)

set(BUILD_SHARED_LIBS OFF)
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")

include(detect_target_cpu)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

add_definitions(-D_SC3000)
set(EXEC_TARGET emusc3000)
set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/sc3000.qrc)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/sc3000)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/sc3000)

include(config_commonsource)

add_subdirectory(../../src/vm/sc3000 vm/sc3000)
add_subdirectory(../../src/qt/sc3000 qt/sc3000)
add_subdirectory(../../src/qt/common qt/common)
add_subdirectory(../../src/qt/debugger qt/debugger)