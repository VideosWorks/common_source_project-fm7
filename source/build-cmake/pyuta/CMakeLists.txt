# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject, TOMY Pyuta, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")


project (emupyuta)

set(LOCAL_LIBS 	   vm_pyuta
		   vm_vm
		   common_common
#		   vm_fmgen
		   qt_pyuta
		   qt_gui
                   )

set(VMFILES_BASE
		   tms9995.cpp
		   sn76489an.cpp
		   tms9918a.cpp
		   
		   datarec.cpp
		   
		   event.cpp
		   memory.cpp
)


set(BUILD_SHARED_LIBS OFF)
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENCL ON CACHE BOOL "Build using OpenCL")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(XM7_VERSION 3)
#set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger.")

include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")


set(VMFILES ${VMFILES_BASE})
add_definitions(-D_PYUTA)
set(EXEC_TARGET emupyuta)

#include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/pyuta)
#include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/fmgen)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/pyuta)


include(config_commonsource)



add_subdirectory(../../src/vm/pyuta vm/pyuta)
#add_subdirectory(../../src/vm/fmgen vm/fmgen)
add_subdirectory(../../src/qt/pyuta qt/pyuta)

     
add_subdirectory(../../src/qt/common qt/common)