message("* qt/osd")
SET(THIS_LIB_VERSION 2.19.0)

set(s_qt_osd_headers
	osd_base.h
)
#include(PrecompiledHeader)

if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_osd_headers_MOC ${s_qt_osd_headers})
else()
  QT4_WRAP_CPP(s_qt_osd_headers_MOC ${s_qt_osd_headers})
endif()

SET(s_qt_osd_srcs
#	  pch.cpp
      osd_base.cpp
	  osd_screen.cpp
	  osd_console.cpp
	  osd_input.cpp
	  osd_printer.cpp
	  osd_socket.cpp
	  osd_sound.cpp
	  osd_video.cpp
	  ${s_qt_osd_headers_MOC}

)
add_definitions(-D__LIBOSD_VERSION=\"libCSPosd.${THIS_LIB_VERSION}\")

if(WIN32)
#add_library(qt_osd
#    ${s_qt_osd_srcs}
#    ${s_qt_osd_headers_MOC}
#    )
#set_property(TARGET qt_osd PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
include (GenerateExportHeader)
  add_library(CSPosd SHARED
    ${s_qt_osd_srcs}
    ${s_qt_osd_headers_MOC}
  )

target_link_libraries(CSPosd PUBLIC
          PRIVATE  ${CMAKE_SOURCE_DIR}/../../build-cmake/bin-win32/libCSPemu_utils.dll.a
#          ${CMAKE_SOURCE_DIR}/../../build-cmake/bin-win32/libCSPgui.dll.a
#          ${CMAKE_SOURCE_DIR}/../../build-cmake/bin-win32/libqt_avio.a
          PUBLIC
	  Qt5::Core
	  Qt5::Gui
	  Qt5::OpenGL
	  Qt5::Widgets
	  ${SDL2_LIBRARIES}
	  )
generate_export_header(CSPosd
       BASE_NAME CSPosd
       EXPORT_MACRO_NAME CSPosd_EXPORT
       EXPORT_FILE_NAME CSPosd_Export.h
       STATIC_DEFINE CSPgosd_BUILT_AS_STATIC
)
  set_target_properties(CSPosd PROPERTIES 
     SOVERSION ${THIS_LIB_VERSION}
     VERSION ${THIS_LIB_VERSION}
  )

else()
	#if(USE_DEVICES_SHARED_LIB)
	  add_library(CSPosd SHARED
		${s_qt_osd_srcs}
		${s_qt_osd_headers_MOC}
#		pch.cpp
		)
#	  add_precompiled_header(CSPosd pch_list
#		SOURCE_CXX pch.cpp FORCEINCLUDE
#		)
	  target_link_libraries(CSPosd PUBLIC
		Qt5::Core
		Qt5::Gui
		Qt5::Widgets
		${SDL2_LIBRARIES}
		)
	  set_target_properties(CSPosd PROPERTIES 
		SOVERSION ${THIS_LIB_VERSION}
		VERSION ${THIS_LIB_VERSION}
		)
	  INSTALL(TARGETS CSPosd DESTINATION ${LIBCSP_INSTALL_DIR})
	#else()
	#  add_library(osd STATIC
	#	${s_qt_osd_srcs}
	#	${s_qt_osd_headers_MOC}
	#	)
	#endif()
endif()


