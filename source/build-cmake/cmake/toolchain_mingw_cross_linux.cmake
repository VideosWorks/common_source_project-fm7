# the name of the target operating system
SET(CMAKE_SYSTEM_NAME Windows)

# which compilers to use for C and C++
SET(CMAKE_C_COMPILER i686-w64-mingw32-gcc)
SET(CMAKE_CXX_COMPILER i686-w64-mingw32-g++)
SET(CMAKE_RC_COMPILER i686-w64-mingw32-windres)
SET(CMAKE_AR  i686-w64-mingw32-gcc-ar)

#SET(CMAKE_C_COMPILER i686-w64-mingw32-clang)
#SET(CMAKE_CXX_COMPILER i686-w64-mingw32-clang++)
#SET(CMAKE_RC_COMPILER i686-w64-mingw32-windres)
#SET(CMAKE_AR  i686-w64-mingw32-gcc-ar)

#set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
#set(CMAKE_EXE_LINKER_FLAGS "-static-libstdc++")
#set(CMAKE_EXE_LINKER_FLAGS "-flto=12 -fwhole-program")

#SET(CMAKE_C_ARCHIVE_CREATE "<CMAKE_AR> qcs <TARGET> --plugin==$(i686-w64-mingw32-gcc --print-file-name=liblto_plugin.so) <LINK_FLAGS> <OBJECTS>")
#SET(CMAKE_C_ARCHIVE_FINISH   true)
#SET(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> qcs <TARGET> --plugin==$(i686-w64-mingw32-gcc --print-file-name=liblto_plugin.so) <LINK_FLAGS> <OBJECTS>")
#SET(CMAKE_CXX_ARCHIVE_FINISH   true)

set(LIBAV_ROOT_DIR "/usr/local/i586-mingw-msvc/ffmpeg-4.1")

# here is the target environment located
set(USE_SDL2 ON)
if(USE_SDL2)
   SET(CMAKE_FIND_ROOT_PATH  /usr/i686-w64-mingw32 
                          /usr/local/i586-mingw-msvc
                          /usr/local/i586-mingw-msvc/SDL/i686-w64-mingw32
			  /usr/local/i586-mingw-msvc/5.12/mingw_82x
			  )
else()
   SET(CMAKE_FIND_ROOT_PATH  /usr/i686-w64-mingw32 
                          /usr/local/i586-mingw-msvc
                          /usr/local/i586-mingw-msvc/SDL1/
			  /usr/local/i586-mingw-msvc/5.12/mingw_82x
			  )
endif()
SET(CSP_CROSS_BUILD 1)

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search 
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(SDL2_LIBRARIES
                         /usr/local/i586-mingw-msvc/SDL/i686-w64-mingw32/lib/libSDL2.dll.a 
			 /usr/local/i586-mingw-msvc/SDL/i686-w64-mingw32/lib/libSDL2main.a)
set(SDL2_INCLUDE_DIRS /usr/local/i586-mingw-msvc/SDL/i686-w64-mingw32/include/SDL2)

set(SDL_LIBRARIES
                         /usr/local/i586-mingw-msvc/SDL1/lib/libSDL.dll.a 
			 /usr/local/i586-mingw-msvc/SDL1/lib/libSDLmain.a)
set(SDL_INCLUDE_DIRS /usr/local/i586-mingw-msvc/SDL1/include/SDL)

set(SDLMAIN_LIBRARY "")

set(ADDITIONAL_LIBRARIES libwinmm.a)
