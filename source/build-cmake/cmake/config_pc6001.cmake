# Build Common Sourcecode Project, Agar.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of , but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")
set(VM_NAME pc6001)
set(USE_FMGEN ON)
set(WITH_JOYSTICK ON)
set(WITH_MOUSE ON)

set(VMFILES
		   i8255.cpp
		   event.cpp
		   io.cpp
		   memory.cpp
)

set(VMFILES_LIB
		noise.cpp
		datarec.cpp
		pc6031.cpp
		pc80s31k.cpp
		upd765a.cpp

		prnfile.cpp
		disk.cpp
)
set(FLAG_USE_MCS48 ON)
set(FLAG_USE_Z80 ON)

set(BUILD_SHARED_LIBS OFF)

set(BUILD_PC6001 OFF CACHE BOOL "Build on PC6001")
set(BUILD_PC6001MK2 OFF CACHE BOOL "Build on PC6001mk2")
set(BUILD_PC6001MK2SR OFF CACHE BOOL "Build on PC6001mk2SR")
set(BUILD_PC6601 OFF CACHE BOOL "Build on PC6601")
set(BUILD_PC6601SR OFF CACHE BOOL "Build on PC6601SR")

set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(WITH_DEBUGGER ON CACHE BOOL "Build witn Debugger.")

include(detect_target_cpu)
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

if(BUILD_PC6001)
   add_definitions(-D_PC6001)
   set(EXEC_TARGET emupc6001)
   set(VMFILES ${VMFILES}
       mc6847.cpp
   )
   set(VMFILES_LIB ${VMFILES_LIB}
       ay_3_891x.cpp
       mc6847_base.cpp
   )
   set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/pc6001.qrc)
elseif(BUILD_PC6001MK2)
   add_definitions(-D_PC6001MK2)
   set(EXEC_TARGET emupc6001mk2)
   set(VMFILES_LIB ${VMFILES_LIB}
       upd7752.cpp
   )
   set(VMFILES_LIB ${VMFILES_LIB}
       ay_3_891x.cpp
   )
   set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/pc6001mk2.qrc)
elseif(BUILD_PC6001MK2SR)
   add_definitions(-D_PC6001MK2SR)
   set(EXEC_TARGET emupc6001mk2sr)
   set(VMFILES_LIB ${VMFILES_LIB}
       upd7752.cpp
       ym2203.cpp
   )
   set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/pc6001mk2sr.qrc)
elseif(BUILD_PC6601)
   add_definitions(-D_PC6601)
   set(EXEC_TARGET emupc6601)
   set(VMFILES_LIB ${VMFILES_LIB}
       ay_3_891x.cpp
       upd7752.cpp
   )
   set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/pc6601.qrc)
elseif(BUILD_PC6601SR)
   add_definitions(-D_PC6601SR)
   set(EXEC_TARGET emupc6601sr)
   set(VMFILES_LIB ${VMFILES_LIB}
       upd7752.cpp
       ym2203.cpp
   )
   set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/pc6601sr.qrc)
endif()

include(config_commonsource)

