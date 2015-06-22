# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")

set(LOCAL_LIBS 	   vm_mz2500
		   vm_vm
		   common_common
		   qt_mz2500
		   qt_gui
                   )

set(VMFILES_2500
		   z80.cpp
		   z80pio.cpp
		   z80sio.cpp
		   
		   datarec.cpp
		   disk.cpp
		   
		   i8255.cpp
		   i8253.cpp
		   io.cpp
 
		   pcm1bit.cpp
		   mb8877.cpp
		   rp5c01.cpp
		   ls393.cpp
		   w3100a.cpp
		   
		   ym2203.cpp
		   
		   event.cpp
		   memory.cpp
)
set(VMFILES_BASE
		   event.cpp
		   memory.cpp
		   io.cpp
		   disk.cpp
		   
		   datarec.cpp
		   i8253.cpp
		   i8255.cpp
		   mb8877.cpp
		   pcm1bit.cpp
		   z80.cpp
		   z80pio.cpp
		   
		   )
set(VMFILES_QD
		   z80sio.cpp
		   )

set(VMFILES_16BIT
		   i286.cpp
		   i8259.cpp
		   )


set(BUILD_MZ2500 OFF CACHE BOOL "Build EMU-MZ2500")
set(BUILD_MZ2200 OFF CACHE BOOL "Build EMU-MZ2200")
set(BUILD_MZ2000 OFF CACHE BOOL "Build EMU-MZ2000")
set(BUILD_MZ80B OFF CACHE BOOL "Build EMU-MZ80B")
set(USE_CMT_SOUND ON CACHE BOOL "Using sound with CMT")

set(BUILD_SHARED_LIBS OFF)
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENCL ON CACHE BOOL "Build using OpenCL")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(XM7_VERSION 3)
set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger.")

include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

if(BUILD_MZ2500)

set(VMFILES ${VMFILES_2500})
set(LOCAL_LIBS ${LOCAL_LIBS} vm_fmgen)
add_definitions(-D_MZ2500)
set(EXEC_TARGET emumz2500)
set(USE_SOCKET ON)

elseif(BUILD_MZ2000)
set(VMFILES ${VMFILES_BASE} ${VMFILES_QD} ${VMFILES_16BIT})
add_definitions(-D_MZ2000)
set(EXEC_TARGET emumz2000)

elseif(BUILD_MZ2200)
set(VMFILES ${VMFILES_BASE} ${VMFILES_QD} ${VMFILES_16BIT})
set(LOCAL_LIBS ${LOCAL_LIBS})
add_definitions(-D_MZ2200)
set(EXEC_TARGET emumz2200)

elseif(BUILD_MZ80B)
set(VMFILES ${VMFILES_BASE})
set(LOCAL_LIBS ${LOCAL_LIBS})
add_definitions(-D_MZ80B)
set(EXEC_TARGET emumz80b)

endif()

if(BUILD_MZ80A)
set(VMFILES ${VMFILES}
            mb8877.cpp
	    disk.cpp
	    io.cpp )
#add_definitions(-DSUPPORT_MZ80AIF)
endif()

if(USE_CMT_SOUND)
add_definitions(-DDATAREC_SOUND)
endif()


#include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/mz2500)
if(NUILD_MZ2500)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/fmgen)
endif()
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/mz2500)

include(config_commonsource)

if(USE_SSE2)
#  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/fm7/vram/sse2)
#  add_subdirectory(../../src/vm/fm7/vram/sse2 vm/fm7/vram/sse2)
endif()


if(USE_SSE2)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/common/scaler/sse2)
endif()
