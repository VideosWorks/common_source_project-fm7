cmake_minimum_required (VERSION 2.6)

message("* vm/mz80K")

if(BUILD_MZ80AIF)

add_library(vm_mz80k
	memory.cpp
	keyboard.cpp
	mz80aif.cpp
	mz80k.cpp
	printer.cpp
)

elseif(BUILD_MZ80FIO)
add_library(vm_mz80k
	memory.cpp
	keyboard.cpp
	mz80fio.cpp
	mz80k.cpp
	printer.cpp
)

else()

add_library(vm_mz80k
	memory.cpp
	keyboard.cpp
	mz80k.cpp
	printer.cpp
)
endif()
