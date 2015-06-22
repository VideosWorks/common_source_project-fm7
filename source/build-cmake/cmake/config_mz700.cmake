# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

set(LOCAL_LIBS 	   vm_mz700
		   vm_vm
		   common_common
		   vm_fmgen
#		   common_scaler-generic
		   qt_mz700
		   qt_gui
                   )

set(VMFILES_BASE
		   z80.cpp
		   beep.cpp
		   i8255.cpp
		   i8253.cpp
		   pcm1bit.cpp
		   
		   datarec.cpp
		   
		   and.cpp

		   event.cpp
		   io.cpp
		   memory.cpp
)

set(VMFILES_MZ800 ${VMFILES_BASE}
		   mb8877.cpp
		   disk.cpp
		   
		   not.cpp
		   sn76489an.cpp
		   z80pio.cpp
		   z80sio.cpp
)

set(VMFILES_MZ1500 ${VMFILES_MZ800}
		   ym2203.cpp
)
		   


set(BUILD_MZ700 OFF CACHE BOOL "Build EMU-MZ800")
set(BUILD_MZ800 OFF CACHE BOOL "Build EMU-MZ800")
set(BUILD_MZ1500 OFF CACHE BOOL "Build EMU-MZ1500")
set(USE_CMT_SOUND OFF CACHE BOOL "Using sound with CMT")

set(BUILD_SHARED_LIBS OFF)
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENCL ON CACHE BOOL "Build using OpenCL")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger.")

include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")


if(BUILD_MZ1500)

set(VMFILES ${VMFILES_MZ1500})
add_definitions(-D_MZ1500)
set(EXEC_TARGET emumz1500)

elseif(BUILD_MZ800)

set(VMFILES ${VMFILES_MZ800})
add_definitions(-D_MZ800)
set(EXEC_TARGET emumz800)

else()

set(VMFILES ${VMFILES_BASE})
add_definitions(-D_MZ700)
set(EXEC_TARGET emumz700)

endif()

if(USE_CMT_SOUND)
add_definitions(-DDATAREC_SOUND)
endif()


#include_directories(${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/mz700)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/fmgen)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/mz700)

include(config_commonsource)

